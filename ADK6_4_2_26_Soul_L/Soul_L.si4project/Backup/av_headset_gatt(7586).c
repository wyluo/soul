/*!
\copyright  Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
            All Rights Reserved.
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       av_headset_gatt.c
\brief      Application support for GATT, GATT Server and GAP Server
*/

#include "av_headset.h"

#ifdef INCLUDE_GATT

#include "av_headset_db.h"
#include "av_headset_log.h"

#include "av_headset_test_le.h"


#include <gatt.h>
#include <gatt_manager.h>
#include <gatt_server.h>
#include <gatt_gap_server.h>
#include <gaia.h>
#include <panic.h>

#ifdef SOUL_DEVICE_INFO
#include "av_headset_gatt_dis.h"
#endif

#ifdef INCLUDE_PTEK
#include "av_headset_config.h"
#include <gatt_heart_rate_server.h>
#endif
#ifdef INCLUDE_BME300
#include <gatt_bme300_server.h>
#include <bme300.h>
extern uint8_t bleRxBuf[120];
extern uint8_t bleTxBuf[120];

#define MAKE_BME300_SERVER_MESSAGE(TYPE) TYPE##_T* msg = (TYPE##_T *)PanicNull(calloc(1,sizeof(TYPE##_T)));

#define SET_BME300_PARCE_TIMER(ms) \
    do { \
  accelTaskData *accel = appGetAccelerometer(); \
  uint16* msg = (uint16 *)PanicNull(calloc(1,sizeof(uint16))); \
  *msg = (ms);\
  MessageCancelAll(&accel->task, ACCELEROMETER_MESSAGE_BME_PARCER_TIMER); \
  MessageSend(&accel->task, ACCELEROMETER_MESSAGE_BME_PARCER_TIMER, msg); \
    } while (0)

#endif

/*! Earbud GATT database, for the required GATT and GAP servers. */
extern const uint16 gattDatabase[];

gattGattServerInfo *appGattGetInstanceFromConnId(uint16 conn_id)
{
    int gatt_instance;

    for (gatt_instance = 0; gatt_instance < appConfigBleGetGattServerInstances(); gatt_instance++)
    {
        gattGattServerInfo *instance = appGetGattInstance(gatt_instance);
        if (instance->conn_id == conn_id)
        {
            return instance;
        }
    }
    return (gattGattServerInfo*)NULL;
}

static gattGattServerInfo *appGattCreateInstanceFromConnId(uint16 conn_id)
{
    int gatt_instance;

    for (gatt_instance = 0; gatt_instance < appConfigBleGetGattServerInstances(); gatt_instance++)
    {
        gattGattServerInfo *instance = appGetGattInstance(gatt_instance);
        if (instance->conn_id == 0)
        {
            instance->conn_id = conn_id;
            MessageSend(appGetSmTask(),APP_GATT_CONNECTION_MADE,NULL);
            return instance;
        }
    }
    return (gattGattServerInfo*)NULL;
}

static bool appGattDeleteInstanceByConnId(uint16 conn_id)
{
    int gatt_instance;

    for (gatt_instance = 0; gatt_instance < appConfigBleGetGattServerInstances(); gatt_instance++)
    {
        gattGattServerInfo *instance = appGetGattInstance(gatt_instance);
        if (instance->conn_id == conn_id)
        {
            instance->conn_id = 0;
            MessageSend(appGetSmTask(),APP_GATT_CONNECTION_DROPPED,NULL);
            return TRUE;
        }
    }
    return FALSE;
}


/*! @brief Reply with our MTU. */
static void appGattHandleGattExchangeMtuInd(GATT_EXCHANGE_MTU_IND_T* ind)
{
#if 1 // jim
    GattExchangeMtuResponse(ind->cid, appConfigBleGattMtuMin());
#else
    GattExchangeMtuResponse(ind->cid, appConfigBleGattMtuMin(ind->mtu));
#endif    
}

/*! @brief Handle confirmation about GATT Manager registration. 

    And set-up for resolvable addressing.
*/
static void appGattHandleGattManRegisterWithGattCfm(GATT_MANAGER_REGISTER_WITH_GATT_CFM_T* cfm)
{
    UNUSED(cfm);

    DEBUG_LOGF("APP:GATT: appGattHandleGattManRegisterWithGattCfm");

    if (appConfigBleUseResolvablePrivateAddress())
    {
        ConnectionDmBleConfigureLocalAddressAutoReq(ble_local_addr_generate_static,NULL,appConfigBleAddressChangeTime());
    }
    else
    {
        MessageSend(appGetAppTask(), APP_GATT_INIT_CFM, NULL);
    }
}

static void appGattDisconnectAllBle(void)
{
    int gatt_instance;
    int count = 0;

    for (gatt_instance = 0; gatt_instance < appConfigBleGetGattServerInstances(); gatt_instance++)
    {
        gattGattServerInfo *instance = appGetGattInstance(gatt_instance);
        uint16 connection_id = instance->conn_id;
        if (connection_id)
        {
            GattManagerDisconnectRequest(connection_id);
            count ++;
        }
    }

    if (count)
    {
        DEBUG_LOGF("appGattDisconnectAllBle requested disconnect on %d links",count);
    }
}

bool appGattGoConnectable(void)
{
    ble_adv_params_t    adv_params;
    advManagerAdvert   *advert;

    if (appGattHasBleConnection())
    {
        DEBUG_LOGF("appGattGoConnectable. Disallowed as have a connection");
        return FALSE;
    }
    DEBUG_LOGF("appGattGoConnectable");

    PanicNotNull(appGetGatt()->advert_settings);

    advert = appAdvManagerNewAdvert();
    appGetGatt()->advert_settings = advert;

    appAdvManagerUseLocalName(advert);
    appAdvManagerSetDiscoverableMode(advert, avHeadsetBleDiscoverableModeGeneral);
    appAdvManagerSetReadNameReason(advert, avHeadsetBleGapReadNameGapServer);
#ifdef INCLUDE_GATT_BATTERY_SERVER
    appAdvManagerSetService(advert, GATT_SERVICE_UUID_BATTERY_SERVICE);
#endif

#ifdef SOUL_DEVICE_INFO
    appAdvManagerSetService(advert, GATT_SERVICE_UUID_DEVICE_INFORMATION);
#endif

#ifdef INCLUDE_PTEK
    appAdvManagerSetService(advert, GATT_SERVICE_UUID_HEART_RATE);
#endif

    adv_params.undirect_adv.filter_policy = ble_filter_none;
    appConfigBleGetAdvertisingRate(appGetGatt()->advertising_mode, 
                                    &adv_params.undirect_adv.adv_interval_min,
                                    &adv_params.undirect_adv.adv_interval_max);
    appAdvManagerSetAdvertParams(advert, &adv_params);

    appAdvManagerSetAdvertisingType(advert, ble_adv_ind);
    appAdvManagerSetAdvertisingChannels(advert, BLE_ADV_CHANNEL_ALL);

    /* Select random address - this uses Resolvable Private Address if configured */
    appAdvManagerSetUseOwnRandomAddress(advert, appConfigBleUseResolvablePrivateAddress());

    appAdvManagerSetAdvertData(advert,appGetGattTask());

    return TRUE;
}

bool appGattAllowBleConnections(bool allow)
{
    bool have_connection = appGattHasBleConnection();
    bool trying_connection = appGattAttemptingBleConnection();

    DEBUG_LOGF("appGattAllowBleConnections(%d). Have %d. Trying %d.",allow,have_connection,trying_connection);

    if (allow && !have_connection && !trying_connection)
    {
        appGattGoConnectable();
    }
    else if (!allow && have_connection)
    {
        appGattDisconnectAllBle();
    }
    else if (!allow && trying_connection)
    {
        GattManagerCancelWaitForRemoteClient();
    }
    else
    {
        return FALSE;
    }
    return TRUE;
}


static void appGattDeleteCurrentAdvert(void)
{
    gattTaskData *gatt = appGetGatt();

    advManagerAdvert *advert = gatt->advert_settings;
    if (advert)
    {
        appAdvManagerDeleteAdvert(advert);
        gatt->advert_settings = NULL;
    }
}
void Pre_ptekEnable(void)
{
	ptekEnable((const ptekConfig *)appConfigPtek());
}
void Pre_ptekDisable(void)
{
	#ifdef INCLUDE_PTEK
	ptekDisable((const ptekConfig *)appConfigPtek());
	#endif
}

static void appGattHandleGattManRemoteClientConnectCfm(GATT_MANAGER_REMOTE_CLIENT_CONNECT_CFM_T *cfm)
{
    appGattDeleteCurrentAdvert();

    if (cfm->status == gatt_status_success)
    {
        if (appGattCreateInstanceFromConnId(cfm->cid))
        {
            DEBUG_LOGF("appGattHandleGattManRemoteClientConnectCfm. Added cid %d",cfm->cid);
            #ifdef INCLUDE_PTEK
            //ptekEnable((const ptekConfig *)appConfigPtek());
            #endif

            GaiaConnectGatt(cfm->cid);

            appConnRulesSetEvent(appGetSmTask(), RULE_EVENT_BLE_CONNECTABLE_CHANGE);

//          sinkBleSlaveConnIndEvent(cfm->cid);
//          gattClientAdd(cfm->cid,ble_gap_role_peripheral);  Does discovery. Not obvs useful.
        }
        else
        {   
            DEBUG_LOGF("appGattHandleGattManRemoteClientConnectCfm. Unable to add new GATT client");
            GattManagerDisconnectRequest(cfm->cid);
        }
    }
    else
    {
        DEBUG_LOGF("appGattHandleGattManRemoteClientConnectCfm. Failure. Status:%d.",cfm->status);
    }

}

static void appGattHandleGattManRemoteClientConnectInd(GATT_MANAGER_REMOTE_CLIENT_CONNECT_IND_T *ind)
{
    #ifdef SOUL_BLADE
    GattManagerRemoteClientConnectResponse(ind->cid, ind->flags, TRUE);
    #else
    /* Reject GATT connection request over BR/EDR */
    GattManagerRemoteClientConnectResponse(ind->cid, ind->flags, FALSE);
    #endif
}

static void appGattHandleCancelRemoteClientConnectCfm(GATT_MANAGER_CANCEL_REMOTE_CLIENT_CONNECT_CFM_T *cfm)
{
    DEBUG_LOGF("appGattHandleCancelRemoteClientConnectCfm. sts %d",cfm->status);

    if (cfm->status == gatt_status_success)
    {
        appGattDeleteCurrentAdvert();

        appConnRulesSetEvent(appGetSmTask(), RULE_EVENT_BLE_CONNECTABLE_CHANGE);
    }
}


static void appGattHandleGattManDisconnectInd(GATT_MANAGER_DISCONNECT_IND_T *ind)
{
    appGattDeleteCurrentAdvert();

    if (appGattDeleteInstanceByConnId(ind->cid))
    {
        DEBUG_LOGF("appGattHandleGattManDisconnectInd. Disconnected conn id %d, sts %d", ind->cid, ind->status);
        #ifdef INCLUDE_PTEK
        ptekDisable((const ptekConfig *)appConfigPtek());
        #endif
        
        #ifdef INCLUDE_BME300
        SET_BME300_PARCE_TIMER(300);
        BME_stopGaitSensor();
        StreamAttSourceRemoveAllHandles(ind->cid);
        #endif

        GaiaDisconnectGatt(ind->cid);

        appConnRulesSetEvent(appGetSmTask(), RULE_EVENT_BLE_CONNECTABLE_CHANGE);
    }
    else
    {
        DEBUG_LOGF("appGattHandleGattManDisconnectInd. Unable to disconnect requested conn id %d, sts %d",ind->cid, ind->status);
    }
}




static void appGattHandleWriteClientConfigInd(const GATT_SERVER_WRITE_CLIENT_CONFIG_IND_T *config_ind)
{
    gattGattServerInfo *instance;

    DEBUG_LOGF("appGattHandleWriteClientConfigInd");

    instance = (gattGattServerInfo *)PanicNull(appGattGetInstanceFromConnId(config_ind->cid));

    instance->config.gatt = config_ind->config_value;
    DEBUG_LOGF("  client_config[0x%x]\n", config_ind->config_value);
}

#ifdef INCLUDE_GATT_BATTERY_SERVER

static void appGattHandleBattServerReadLevelInd(const GATT_BATTERY_SERVER_READ_LEVEL_IND_T * ind)
{
    uint8 battery_percent;
    uint16 battery_level_left;
    uint16 battery_level_right;

    gattGattServerInfo *instance;

    DEBUG_LOGF("appGattHandleBattServerReadLevelInd bas=[0x%p] cid=[0x%x]\n", (void *)ind->battery_server, ind->cid);

    if (appConfigIsLeft())
    {
        appPeerSyncGetPeerBatteryLevel(&battery_level_left,&battery_level_right);
    }
    else
    {
        appPeerSyncGetPeerBatteryLevel(&battery_level_right,&battery_level_left);
    }

    instance = (gattGattServerInfo *)PanicNull(appGattGetInstanceFromConnId(ind->cid));

    if (ind->battery_server == &instance->battery_server_left)
    {
        battery_percent = appBatteryConvertLevelToPercentage(battery_level_left);
    }
    else
    {
        battery_percent = appBatteryConvertLevelToPercentage(battery_level_right);
    }

    DEBUG_LOGF("    Return: level=[%u]\n", battery_percent);

    /* Return requested battery level */
    GattBatteryServerReadLevelResponse(ind->battery_server, ind->cid, battery_percent);
}

#define NAMESPACE_BLUETOOTH_SIG             0x01        /* Bluetooth SIG Namespace */
        /* Values taken from GATT Namespace Descriptors, in Assigned Numbers */
#define DESCRIPTION_BATTERY_UNKNOWN         0x0000      /*!< Bluetooth SIG description "unknown" */
#define DESCRIPTION_BATTERY_LEFT            0x010D      /*!< Bluetooth SIG description "left" */
#define DESCRIPTION_BATTERY_RIGHT           0x010E      /*!< Bluetooth SIG description "right" */

static void appGattHandleBatteryServerReadPresentationInd(const GATT_BATTERY_SERVER_READ_PRESENTATION_IND_T * ind)
{
    uint16 description = DESCRIPTION_BATTERY_UNKNOWN;
    gattGattServerInfo *instance;

    instance = (gattGattServerInfo *)PanicNull(appGattGetInstanceFromConnId(ind->cid));

    if (ind->battery_server == &instance->battery_server_left)
    {
        description = DESCRIPTION_BATTERY_LEFT;
    }
    else if (ind->battery_server == &instance->battery_server_right)
    {
        description = DESCRIPTION_BATTERY_RIGHT;
    }
    else
    {
        Panic();
    }

    DEBUG_LOGF("appGattHandleBatteryServerReadPresentationInd bas=[0x%p] cid=[0x%x]\n", 
                    ind->battery_server,ind->cid);

    GattBatteryServerReadPresentationResponse(ind->battery_server,
                                              ind->cid,
                                              NAMESPACE_BLUETOOTH_SIG,
                                              description);

    DEBUG_LOGF("   Return: desc=[0x%x]\n", description);
}

static void appGattHandleBatteryServerReadClientConfig(const GATT_BATTERY_SERVER_READ_CLIENT_CONFIG_IND_T * ind)
{
    uint16 client_config = 0;
    gattGattServerInfo *instance;

    instance = (gattGattServerInfo *)PanicNull(appGattGetInstanceFromConnId(ind->cid));

    /* Return the current value of the client configuration descriptor for the device */
    DEBUG_LOGF("appGattHandleBatteryServerReadClientConfig bas=[0x%p] cid=[0x%x]\n", ind->battery_server, ind->cid);

    if (ind->battery_server == &instance->battery_server_left)
    {
        client_config = instance->config.battery_left;
    }
    else
    {
        client_config = instance->config.battery_right;
    }

    GattBatteryServerReadClientConfigResponse(ind->battery_server, ind->cid, client_config);
    DEBUG_LOGF("  client_config=[0x%x]\n", client_config);
}
#endif /* INCLUDE_GATT_BATTERY_SERVER */


static void appGattHandleGattGapReadDeviceNameInd(const GATT_GAP_SERVER_READ_DEVICE_NAME_IND_T *ind)
{
    uint8 *name = appAdvManagerGetLocalName();
    uint16 namelen;

    DEBUG_LOGF("appGattHandleGattGapReadDeviceNameInd");

    /* It is (barely) possible that there is not a local name set, 
       in which case use a fallback */
    if (!name)
    {
        static uint8 fallback_name[] = "Earbud";
        name = fallback_name;
    }

    namelen = strlen((char *)name);

    /* The indication can request a portion of our name by specifying the start offset */
    if (ind->name_offset)
    {
        /* Check that we haven't been asked for an entry off the end of the name */
        if (ind->name_offset >= namelen)
        {
            namelen = 0;
            name = NULL;
        }
        else
        {
            namelen -= ind->name_offset;
            name += ind->name_offset;
        }
    }

    GattGapServerReadDeviceNameResponse(appGetGattGapServerInst(0), ind->cid,
                                        namelen, name);
}


static void appGattHandlerAdvMgrAdvertSetDataCfm(const APP_ADVMGR_ADVERT_SET_DATA_CFM_T *cfm)
{
    DEBUG_LOGF("appGattHandlerAdvMgrAdvertSetDataCfm Status %d",cfm->status);

    PanicNotZero(cfm->status);

    GattManagerWaitForRemoteClient(appGetGattTask(), NULL, gatt_connection_ble_slave_undirected);
    MessageSend(appGetSmTask(),APP_GATT_CONNECTABLE,NULL);
}

static void appGattMessageHandler(Task task, MessageId id, Message message)
{
    UNUSED(task);

    DEBUG_LOGF("appGattMessageHandler id:%d 0x%x", id, id);

    switch (id)
    {
        /******************** GATT LIB MESSAGES ******************/
        case GATT_EXCHANGE_MTU_IND:
            appGattHandleGattExchangeMtuInd((GATT_EXCHANGE_MTU_IND_T*)message);
            break;

        /************ GATT MANAGER LIB MESSAGES ******************/
        case GATT_MANAGER_REGISTER_WITH_GATT_CFM:
            appGattHandleGattManRegisterWithGattCfm((GATT_MANAGER_REGISTER_WITH_GATT_CFM_T*)message);
            break;

        case GATT_MANAGER_REMOTE_CLIENT_CONNECT_CFM:
            appGattHandleGattManRemoteClientConnectCfm((GATT_MANAGER_REMOTE_CLIENT_CONNECT_CFM_T*)message);
            break;

        case GATT_MANAGER_REMOTE_CLIENT_CONNECT_IND:
            appGattHandleGattManRemoteClientConnectInd((GATT_MANAGER_REMOTE_CLIENT_CONNECT_IND_T*)message);
            break;

        case GATT_MANAGER_CANCEL_REMOTE_CLIENT_CONNECT_CFM:
            appGattHandleCancelRemoteClientConnectCfm((GATT_MANAGER_CANCEL_REMOTE_CLIENT_CONNECT_CFM_T *)message);
            break;

        case GATT_MANAGER_DISCONNECT_IND:
            appGattHandleGattManDisconnectInd((GATT_MANAGER_DISCONNECT_IND_T*)message);
            break;

        case GATT_MANAGER_SERVER_ACCESS_IND:
            DEBUG_LOGF("appGattMessageHandler. GATT_MANAGER_SERVER_ACCESS_IND. Handle 0x%x",
                       ((GATT_MANAGER_SERVER_ACCESS_IND_T *)message)->handle);
            break;

        /******************** GAP MESSAGES ******************/
        case GATT_GAP_SERVER_READ_DEVICE_NAME_IND:
            appGattHandleGattGapReadDeviceNameInd((const GATT_GAP_SERVER_READ_DEVICE_NAME_IND_T*)message);
            break;

        /************ GATT SERVER LIB MESSAGES ******************/
        case GATT_SERVER_READ_CLIENT_CONFIG_IND:
            DEBUG_LOGF("appGattMessageHandler ##UNHANDLED## GATT_SERVER_READ_CLIENT_CONFIG_IND");
            break;

        case GATT_SERVER_WRITE_CLIENT_CONFIG_IND:
            appGattHandleWriteClientConfigInd((const GATT_SERVER_WRITE_CLIENT_CONFIG_IND_T *)message);
            break;

        case GATT_SERVER_SERVICE_CHANGED_INDICATION_CFM:
            DEBUG_LOGF("appGattMessageHandler ##UNHANDLED## GATT_SERVER_SERVICE_CHANGED_INDICATION_CFM");
            break;

        /************ ADVERTISING MANAGER MESSAGES ******************/
        case APP_ADVMGR_ADVERT_SET_DATA_CFM:
            appGattHandlerAdvMgrAdvertSetDataCfm((const APP_ADVMGR_ADVERT_SET_DATA_CFM_T *)message);
            break;

        default:
            DEBUG_LOGF("appGattMessageHandler. Unhandled message id:0x%x", id);
            break;
    }
}


/*! Handle CL_DM_BLE_CONFIGURE_LOCAL_ADDRESS_CFM message

    This completes the library initialisation, so inform the application 
 */
static void appGattHandleDmBleCconfigureLocalAddressCfm(CL_DM_BLE_CONFIGURE_LOCAL_ADDRESS_CFM_T *cfm)
{
    if (appConfigBleUseResolvablePrivateAddress())
    {
        DEBUG_LOGF("appGattHandleDmBleCconfigureLocalAddressCfm sts:%d First resolvable %04X:%02X:%06X",
                    cfm->status,
                    cfm->random_taddr.addr.nap,cfm->random_taddr.addr.uap,cfm->random_taddr.addr.lap);

        MessageSend(appGetAppTask(), APP_GATT_INIT_CFM, NULL);
    }
}


bool appGattHandleConnectionLibraryMessages(MessageId id, Message message, bool already_handled)
{
    DEBUG_LOGF("appGattHandleConnectionLibraryMessages id : 0x%x  0x%p", id, message);
    switch (id)
    {
    case CL_DM_BLE_CONFIGURE_LOCAL_ADDRESS_CFM:
        appGattHandleDmBleCconfigureLocalAddressCfm((CL_DM_BLE_CONFIGURE_LOCAL_ADDRESS_CFM_T *)message);
        return TRUE;
        default:
            DEBUG_LOGF("appGattHandleConnectionLibraryMessages. Unhandled message id:0x%X 0x%p", id, message);
            break;
    }
    return already_handled;
}


#ifdef INCLUDE_GATT_BATTERY_SERVER
static void appGattBatteryMessageHandler(Task task, MessageId id, Message message)
{
    UNUSED(task);

    DEBUG_LOGF("appGattBatteryMessageHandler id:%d 0x%x", id, id);

    switch (id)
    {
        case GATT_BATTERY_SERVER_READ_LEVEL_IND:
            appGattHandleBattServerReadLevelInd((const GATT_BATTERY_SERVER_READ_LEVEL_IND_T *) message);
            break;

        case GATT_BATTERY_SERVER_READ_CLIENT_CONFIG_IND:
            appGattHandleBatteryServerReadClientConfig((const GATT_BATTERY_SERVER_READ_CLIENT_CONFIG_IND_T *) message);
            break;

        case GATT_BATTERY_SERVER_WRITE_CLIENT_CONFIG_IND:
            DEBUG_LOGF("appGattMessageHandler ##UNHANDLED## GATT_BATTERY_SERVER_WRITE_CLIENT_CONFIG_IND");
            break;

        case GATT_BATTERY_SERVER_READ_PRESENTATION_IND:
            appGattHandleBatteryServerReadPresentationInd((const GATT_BATTERY_SERVER_READ_PRESENTATION_IND_T *)message);
            break;

        default:
            DEBUG_LOGF("appGattBatteryMessageHandler. Unhandled message id:0x%x", id);
            break;
    }
}

#endif /* INCLUDE_GATT_BATTERY_SERVER */

#ifdef INCLUDE_PTEK
static void appGattHrsMessageHandler(Task task, MessageId id, Message message)
{
    UNUSED(task);

    DEBUG_LOGF("appGattHrsMessageHandler id:%d 0x%x", id, id);

    switch (id)
    {
        /************ GATT HEART RATE SERVER LIB MESSAGES ******************/
        case GATT_HR_SERVER_READ_CLIENT_CONFIG_IND:    
            DEBUG_LOGF("<<<<< MINI >>>>>>> GATT_HR_SERVER_READ_CLIENT_CONFIG_IND");
            handleReadHrMeasurementClientConfig((GATT_HR_SERVER_READ_CLIENT_CONFIG_IND_T*)message);
            break;
        case GATT_HR_SERVER_WRITE_CLIENT_CONFIG_IND:
            DEBUG_LOGF("<<<<< MINI >>>>>>> GATT_HR_SERVER_WRITE_CLIENT_CONFIG_IND");
            handleWriteHrMeasurementClientConfig((GATT_HR_SERVER_WRITE_CLIENT_CONFIG_IND_T*)message);
            break;

        case GATT_HR_SERVER_TIMER_IND:
            DEBUG_LOGF("<<<<< MINI >>>>>>> GATT_HR_SERVER_TIMER_IND");
            appBleHRSensorInContact((GATT_HR_SERVER_TIMER_IND_T*)message);
            
            break;

        default:
            DEBUG_LOGF("appGattHrsMessageHandler. Unhandled message id:0x%x", id);
            break;
    }
}

#endif /* INCLUDE_PTEK */


#ifdef INCLUDE_BME300
#define HANDLE_SIZE 2
static uint16 uartSendPacket(Sink uartSink, uint8 *data, uint16 length, uint16 handle_endpoint)
{
    uint8 *sinkPtr;
    uint16 offset;
    uint16 bytesAvailable;
    uint16 sendSize = length+HANDLE_SIZE; /*Data Length + Handel Addr Size */
    uint16 remainSize = 0;
    
    sinkPtr = SinkMap(uartSink);

    if(sinkPtr)
    {
        DEBUG_LOGF("UART:uartSendPacket available\n");
        bytesAvailable = SinkSlack(uartSink);
        
        if(sendSize > bytesAvailable)
        {
            sendSize = bytesAvailable;
            remainSize = length - (sendSize - HANDLE_SIZE);
        }
        
        offset = SinkClaim(uartSink, sendSize);
        
        DEBUG_LOGF("UART: bytesAvailable 0x%x offset %d", bytesAvailable, offset);

        if(offset != 0xffff)
        {
            sinkPtr[offset++] = (handle_endpoint & 0xFF);
            sinkPtr[offset++] = (handle_endpoint >> 8);
            memcpy(&sinkPtr[offset],data,sendSize);
            SinkFlush(uartSink, sendSize);
            DEBUG_LOGF("UART: sinkPtr[0] 0x%x 0x%x 0x%x", sinkPtr[0], sinkPtr[1], sinkPtr[2]);
            DEBUG_LOGF("UART: sinkPtr[3] 0x%x 0x%x 0x%x 0x%x", sinkPtr[3], sinkPtr[4], sinkPtr[5], sinkPtr[6]);
        }
    }
    else
    {
        DEBUG_LOGF("UART: no sink\n");
    }

    return remainSize;
}


static void handleBme300ServerDisconnectInd(GATT_BME300_SERVER_DISCONNECT_IND_T *ind)
{
    appGattDeleteCurrentAdvert();

    if (appGattDeleteInstanceByConnId(ind->cid))
    {
        DEBUG_LOGF("handleBme300ServerDisconnectInd. Disconnected conn id %d, sts %d", ind->cid, ind->status);
        
        SET_BME300_PARCE_TIMER(300);
        BME_stopGaitSensor();
        StreamAttSourceRemoveAllHandles(ind->cid);
        appConnRulesSetEvent(appGetSmTask(), RULE_EVENT_BLE_CONNECTABLE_CHANGE);
    }
    else
    {
        DEBUG_LOGF("handleBme300ServerDisconnectInd. Unable to disconnect requested conn id %d, sts %d",ind->cid, ind->status);
    }
}


static void appGattBmeMessageHandler(Task task, MessageId id, Message message)
{
    UNUSED(task);

    //DEBUG_LOGF("appGattBmeMessageHandler id:%d 0x%x", id, id);

    switch (id)
    {
        /************ GATT HEART RATE SERVER LIB MESSAGES ******************/
        case GATT_BME300_SERVER_READ_CLIENT_CONFIG_IND:    
        {
            GATT_BME300_SERVER_READ_CLIENT_CONFIG_IND_T *ind =(GATT_BME300_SERVER_READ_CLIENT_CONFIG_IND_T*)message;
            DEBUG_LOGF("<<<<< MINI >>>>>>> GATT_BME300_SERVER_READ_CLIENT_CONFIG_IND");
    
            /* Obtain source from Enhanced ATT streams */
            if(ind->bmeIns.bmeSrc == NULL)
            {
                
                StreamAttSourceRemoveAllHandles(ind->cid);
                
                ind->bmeIns.bmeSrc = StreamAttServerSource(ind->cid);
            
                if(ind->bmeIns.bmeSrc)
                {
                    bmePacketIns_t bmeIns;
                    
                    DEBUG_LOG("ATT Stream Source valid\n");
                    /* Get the adjusted handle from GattServer Database */
                    ind->bmeIns.handle_data_endpoint = GattManagerGetServerDatabaseHandle(&ind->bmes->lib_task, HANDLE_BME300_UART_RX);
                    PanicFalse(StreamAttAddHandle(ind->bmeIns.bmeSrc, ind->bmeIns.handle_data_endpoint));
                    DEBUG_LOG("GAIA Task registered for recieving the messages\n");
                    /* Configure the source for getting all the messages */
                    SourceConfigure(ind->bmeIns.bmeSrc, VM_SOURCE_MESSAGES, VM_MESSAGES_SOME);
                    /* Associate the task with the stream source */
                    MessageStreamTaskFromSink(StreamSinkFromSource(ind->bmeIns.bmeSrc), &ind->bmes->lib_task);
                    /* Obtaining the sink from the ATT streams and storing the same */
                    ind->bmeIns.bmeSnk = StreamAttServerSink(ind->cid);
                    /* Configure the sink such that messages are not received */
                    SinkConfigure(ind->bmeIns.bmeSnk, VM_SINK_MESSAGES, VM_MESSAGES_NONE);
                    ind->bmeIns.handle_response_endpoint = GattManagerGetServerDatabaseHandle(&ind->bmes->lib_task, HANDLE_BME300_UART_TX);
                    DEBUG_LOGF("<<< MINI >>> handle_data_endpoint 0x%x, response_endpoint 0x%x",ind->bmeIns.handle_data_endpoint, ind->bmeIns.handle_response_endpoint);
                    
                    bmeIns.bmeSrc = ind->bmeIns.bmeSrc;
                    bmeIns.bmeSnk = ind->bmeIns.bmeSnk;
                    bmeIns.handle_data_endpoint = ind->bmeIns.handle_data_endpoint;
                    bmeIns.handle_response_endpoint = ind->bmeIns.handle_response_endpoint;
                    
                    setBmePacketInstance(bmeIns);
                }
            }
            
            SET_BME300_PARCE_TIMER(10);
        }
            break;
        case GATT_BME300_SERVER_WRITE_CLIENT_CONFIG_IND:
        {
            GATT_BME300_SERVER_WRITE_CLIENT_CONFIG_IND_T *ind =(GATT_BME300_SERVER_WRITE_CLIENT_CONFIG_IND_T*)message;
            bmePacketIns_t bmeIns = ind->bmeIns;

            
            DEBUG_LOGF("<<<<< MINI >>>>>>> GATT_BME300_SERVER_WRITE_CLIENT_CONFIG_IND");

            /* Obtain source from Enhanced ATT streams */
            if(ind->bmeIns.bmeSrc == NULL)
            {
                StreamAttSourceRemoveAllHandles(ind->cid);
                ind->bmeIns.bmeSrc = StreamAttServerSource(ind->cid);

                if(ind->bmeIns.bmeSrc)
                {
                    DEBUG_LOG("ATT Stream Source valid\n");
                    /* Get the adjusted handle from GattServer Database */
                    ind->bmeIns.handle_data_endpoint = GattManagerGetServerDatabaseHandle(&ind->bmes->lib_task, HANDLE_BME300_UART_RX);
                    PanicFalse(StreamAttAddHandle(ind->bmeIns.bmeSrc, ind->bmeIns.handle_data_endpoint));
                    DEBUG_LOG("GAIA Task registered for recieving the messages\n");
                    /* Configure the source for getting all the messages */
                    SourceConfigure(ind->bmeIns.bmeSrc, VM_SOURCE_MESSAGES, VM_MESSAGES_SOME);
                    /* Associate the task with the stream source */
                    MessageStreamTaskFromSink(StreamSinkFromSource(ind->bmeIns.bmeSrc), &ind->bmes->lib_task);
                    /* Obtaining the sink from the ATT streams and storing the same */
                    ind->bmeIns.bmeSnk = StreamAttServerSink(ind->cid);
                    /* Configure the sink such that messages are not received */
                    SinkConfigure(ind->bmeIns.bmeSnk, VM_SINK_MESSAGES, VM_MESSAGES_NONE);
                    ind->bmeIns.handle_response_endpoint = GattManagerGetServerDatabaseHandle(&ind->bmes->lib_task, HANDLE_BME300_UART_TX);
                    DEBUG_LOGF("<<< MINI >>> handle_data_endpoint 0x%x, response_endpoint 0x%x",ind->bmeIns.handle_data_endpoint, ind->bmeIns.handle_response_endpoint);

                    bmeIns.bmeSrc = ind->bmeIns.bmeSrc;
                    bmeIns.bmeSnk = ind->bmeIns.bmeSnk;
                    bmeIns.handle_data_endpoint = ind->bmeIns.handle_data_endpoint;
                    bmeIns.handle_response_endpoint = ind->bmeIns.handle_response_endpoint;
                
                    setBmePacketInstance(bmeIns);
                    DEBUG_LOGF("<<< MINI 2>>> ind->bmeIns.bmeSnk 0x%x, response_endpoint 0x%x",ind->bmeIns.bmeSnk, ind->bmeIns.handle_response_endpoint);
                }
            }
            if(ind->bmeIns.bmeSnk)
            {
                uint16 remainSize = uartSendPacket(ind->bmeIns.bmeSnk,bleTxBuf, ind->size_value, HANDLE_BME300_UART_TX);
                if( remainSize )
                {
                    MAKE_BME300_SERVER_MESSAGE(GATT_BME300_SERVER_WRITE_MORE_DATA_IND);
                    msg->bmes = ind->bmes;
                    msg->cid =  ind->cid;
                    msg->bmeIns = bmeIns;
                    msg->dataOffset = (uint16)(ind->size_value - remainSize);
                    msg->remainSize = remainSize ;
                    MessageSend(appGetGattBmeTask(), GATT_BME300_SERVER_WRITE_MORE_DATA_IND, msg);
                }
            }
        }
            break;

        case GATT_BME300_SERVER_WRITE_MORE_DATA_IND:
            {
                GATT_BME300_SERVER_WRITE_MORE_DATA_IND_T *ind =(GATT_BME300_SERVER_WRITE_MORE_DATA_IND_T*)message;
                DEBUG_LOGF("<<<<< MINI >>>>>>> GATT_BME300_SERVER_WRITE_MORE_DATA_IND");
                if(ind->bmeIns.bmeSnk)
                {
                    uint16 remainSize = uartSendPacket(ind->bmeIns.bmeSnk,&bleTxBuf[ind->dataOffset],ind->remainSize, HANDLE_BME300_UART_TX);
                    if( remainSize )
                    {
                        MAKE_BME300_SERVER_MESSAGE(GATT_BME300_SERVER_WRITE_MORE_DATA_IND);
                        msg->bmes = ind->bmes;
                        msg->cid =  ind->cid;
                        msg->bmeIns = ind->bmeIns;
                        msg->dataOffset = (uint16)(ind->dataOffset + ind->remainSize - remainSize);
                        msg->remainSize = remainSize ;
                        MessageSend(appGetGattBmeTask(), GATT_BME300_SERVER_WRITE_MORE_DATA_IND, msg);
                    }
                }
            }
            break;
            
        
        case GATT_BME300_SERVER_WRITE_READY_IND:
        {
            GATT_BME300_SERVER_WRITE_READY_IND_T *ind =(GATT_BME300_SERVER_WRITE_READY_IND_T*)message;
            //DEBUG_LOGF("<<<<< MINI >>>>>>> GATT_BME300_SERVER_WRITE_READY_IND");

            updateGaitPram(ind->bmes,ind->cid);

            MAKE_BME300_SERVER_MESSAGE(GATT_BME300_SERVER_WRITE_READY_IND);
            msg->bmes = ind->bmes;
            msg->cid =    ind->cid;
            MessageSendLater(appGetGattBmeTask(), GATT_BME300_SERVER_WRITE_READY_IND, msg, 5);
        }
        break; 

        case GATT_BME300_SERVER_DISCONNECT_IND:
        {
            DEBUG_LOGF("<<<<< MINI >>>>>>> GATT_BME300_SERVER_DISCONNECT_IND");    
            handleBme300ServerDisconnectInd((GATT_BME300_SERVER_DISCONNECT_IND_T*)message);
            break;
        }

        default:
            DEBUG_LOGF("appGattBmeMessageHandler. Unhandled message id:0x%x", id);
            break;
    }
}

#endif /* INCLUDE_BME300 */


void appGattSetAdvertisingMode(appConfigBleAdvertisingMode mode)
{
    gattTaskData *gatt = appGetGatt();

    DEBUG_LOGF("appGattSetAdvertisingMode Mode set to %d (was %d)",mode,gatt->advertising_mode);

    /* Set the advertising mode immediately, as it won't affect a current advert. */
    gatt->advertising_mode = mode;
}


/*! @brief Initialise the GATT component. */
void appGattInit(void)
{
#ifdef INCLUDE_GATT_BATTERY_SERVER
    gatt_battery_server_init_params_t battery_server_params = {.enable_notifications = TRUE};
#endif
    gattTaskData *gatt = appGetGatt();
    int gatt_instance;

    memset(gatt,0,sizeof(*gatt));

    /* setup the GATT tasks */
    gatt->gatt_task.handler = appGattMessageHandler;
#ifdef INCLUDE_GATT_BATTERY_SERVER
    gatt->gatt_battery_task.handler = appGattBatteryMessageHandler;
#endif
    
#ifdef INCLUDE_PTEK
        gatt->gatt_hrs_task.handler = appGattHrsMessageHandler;
#endif

#ifdef INCLUDE_BME300
        gatt->gatt_bme_task.handler = appGattBmeMessageHandler;
#endif

    /* Initialise the GATT Manager */
    if (!GattManagerInit(appGetGattTask()))
    {
        DEBUG_LOGF("appGattInit. Failed to initialise GattManager");
        Panic();
    }

    if (!GattManagerRegisterConstDB(&gattDatabase[0], GattGetDatabaseSize()/sizeof(uint16)))
    {
        DEBUG_LOGF("appGattInit. Failed to register GATT database");
        Panic();
    }

#if appConfigBleGetGattServerInstances() != 1
#error More than one GATT instance
// Not sure on correct behaviour if two instances. There is only one database (at present)
// But if two connections are supported there are two structures. It looks like much/all of
// this information would be duplicated.
#endif
    for (gatt_instance = 0; gatt_instance < appConfigBleGetGattServerInstances(); gatt_instance++)
    {
        if (GattServerInit(appGetGattServerInst(gatt_instance), appGetGattTask(), 
                                        HANDLE_GATT_SERVICE, HANDLE_GATT_SERVICE_END) 
                            != gatt_server_status_success)
        {
            DEBUG_LOGF("APP:GATT: GATT server init failed");
            Panic();
        }

        if (GattGapServerInit(appGetGattGapServerInst(gatt_instance), appGetGattGapTask(),
                                        HANDLE_GAP_SERVICE, HANDLE_GAP_SERVICE_END) 
                            != gatt_gap_server_status_success)
        {
            DEBUG_LOGF("APP:GATT: GAP server init failed");
            Panic();
        }

#ifdef INCLUDE_GATT_BATTERY_SERVER
        if (!GattBatteryServerInit(appGetGattBatteryServerLeft(gatt_instance),
                                        appGetGattBatteryTask(), &battery_server_params, 
                                        HANDLE_BATTERY_SERVICE1, HANDLE_BATTERY_SERVICE1_END))
        {
            DEBUG_LOGF("APP:GATT: GATT battery server init (left) failed");
            Panic();
        }

        if (!GattBatteryServerInit(appGetGattBatteryServerRight(gatt_instance),
                                        appGetGattBatteryTask(), &battery_server_params, 
                                        HANDLE_BATTERY_SERVICE2, HANDLE_BATTERY_SERVICE2_END))
        {
            DEBUG_LOGF("APP:GATT: GATT battery server init (right) failed");
            Panic();
        }
#endif /* INCLUDE_GATT_BATTERY_SERVER */

#ifdef INCLUDE_GATT_GAIA_SERVER
        GaiaStartGattServer(HANDLE_GAIA_SERVICE, HANDLE_GAIA_SERVICE_END);
#endif

#ifdef SOUL_DEVICE_INFO
        appGattDeviceInfoServerInitialise(gatt_instance);
#endif

#ifdef INCLUDE_PTEK
        GattHrServerInit(appGetGattHrsTask(), appGetGattHRServerInst(gatt_instance), HANDLE_HEART_RATE_SERVICE, HANDLE_HEART_RATE_SERVICE_END);
#endif
    
#ifdef INCLUDE_BME300
        GattBme300ServerInit(appGetGattBmeTask(), appGetGattBmeServerInst(gatt_instance), HANDLE_BME300_SERVICE, HANDLE_BME300_SERVICE_END);
#endif

    }
    /* complete registration of servers */
    GattManagerRegisterWithGatt();

}

#endif /* INCLUDE_GATT */
