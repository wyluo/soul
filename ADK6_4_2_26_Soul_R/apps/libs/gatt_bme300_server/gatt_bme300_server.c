/* Copyright (c) 2017 Qualcomm Technologies International, Ltd. */
/*  */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "gatt_bme300_server_private.h"
#include "pb_encode.h"
#include "pb_decode.h"
#include "bme_protocol.pb.h"
#include "bme300_if.h"

#ifdef DEBUG_PRINT_ENABLED
#define PRINT(x) printf x
#else
#define PRINT(x)  {}
#endif

extern void BME_calibrateGaitSensorForward(uint8 duration);
extern void BME_calibrateGaitSensorDown(uint8 duration);
extern void BME_getGaitSensorCalibrationValue(float *cal);
extern void BME_setGaitSensorCalibrationValue(float *cal);
extern void BME_startGaitSensor(uint8 cycleTime, float height);

extern virtSensorOutputGait_t gaitParameters;
extern uint8 isGaitParametersUpdated;

uint8_t bleRxBuf[120];
uint8_t bleTxBuf[120];
bool isGaitSensorRunning = FALSE;
float gaitSensorCal[BME_PARAM_GAIT_SENSOR_CAL_LENGTH];

static bmePacketIns_t bmePacketIns;

void setBmePacketInstance(bmePacketIns_t bmeIns)
{
     bmePacketIns = bmeIns;
}

static size_t bleProtoSendCalibrationResponse(pb_type_t type, bool success, pb_byte_t *calVal)
{
    Beflex_BiomechEngine_Message message = Beflex_BiomechEngine_Message_init_zero;
    message.which_typeName = Beflex_BiomechEngine_Message_calibrationResponse_tag;
    message.typeName.calibrationResponse.type = (Beflex_BiomechEngine_Calibration_Type)type;
    message.typeName.calibrationResponse.success = success;
    if (calVal != NULL) {
        message.typeName.calibrationResponse.calValue.size = 36;
        memcpy(message.typeName.calibrationResponse.calValue.bytes, calVal, 36);
    }
    memset(bleTxBuf, 0, sizeof(bleTxBuf));
    pb_ostream_t stream = pb_ostream_from_buffer(bleTxBuf, sizeof(bleTxBuf));
    bool status = pb_encode_delimited(&stream, Beflex_BiomechEngine_Message_fields, &message);
    size_t len = stream.bytes_written;

    if (!status) {
        //Serial.print("[PB\t] encoding failed : ");
        //Serial.println(PB_GET_ERROR(&stream));
        return 0;
    }
    //bleSerial.write(bleTxBuf, len);
    
    return (len);
}

static size_t bleProtoSendParameterResponse(pb_type_t type, bool success)
{
    Beflex_BiomechEngine_Message message = Beflex_BiomechEngine_Message_init_zero;
    message.which_typeName = Beflex_BiomechEngine_Message_parameterResponse_tag;
    message.typeName.parameterResponse.type = (Beflex_BiomechEngine_Parameter_Type)type;
    message.typeName.parameterResponse.success = success;
    memset(bleTxBuf, 0, sizeof(bleTxBuf));
    pb_ostream_t stream = pb_ostream_from_buffer(bleTxBuf, sizeof(bleTxBuf));
    bool status = pb_encode_delimited(&stream, Beflex_BiomechEngine_Message_fields, &message);
    size_t len = stream.bytes_written;

    if (!status) {
        //Serial.print("[PB\t] encoding failed : ");
        //Serial.println(PB_GET_ERROR(&stream));
        return 0;
    }

    //bleSerial.write(bleTxBuf, len);
    
    return (len);
}

static size_t bleProtoSendGaitParameters(void)
{
    Beflex_BiomechEngine_Message message = Beflex_BiomechEngine_Message_init_zero;
    message.which_typeName = Beflex_BiomechEngine_Message_parameterGait_tag;
    message.typeName.parameterGait.state = gaitParameters.state;
    message.typeName.parameterGait.elapsed = gaitParameters.elapsed;
    message.typeName.parameterGait.stepCount = gaitParameters.stepCount;
    message.typeName.parameterGait.cadence = gaitParameters.cadence;
    message.typeName.parameterGait.stepWidth = gaitParameters.stepWidth;
    message.typeName.parameterGait.headAngle = gaitParameters.headAngle;
    message.typeName.parameterGait.wssl_rgtl = gaitParameters.wssl_rgtl;
    message.typeName.parameterGait.wssr_rgtr = gaitParameters.wssr_rgtr;
    message.typeName.parameterGait.wdsl_rftl = gaitParameters.wdsl_rftl;
    message.typeName.parameterGait.wdsr_rgtr = gaitParameters.wdsr_rgtr;
    message.typeName.parameterGait.wfpl_rmll = gaitParameters.wfpl_rmll;
    message.typeName.parameterGait.wfpr_rmlr = gaitParameters.wfpr_rmlr;
    message.typeName.parameterGait.shock = gaitParameters.shock;
    message.typeName.parameterGait.symmetry = gaitParameters.symmetry;
    message.typeName.parameterGait.vo = gaitParameters.vo;
    message.typeName.parameterGait.legStiffness = gaitParameters.legStiffness;
    message.typeName.parameterGait.consistency = gaitParameters.consistency;
    message.typeName.parameterGait.speed = gaitParameters.speed;
    memset(bleTxBuf, 0, sizeof(bleTxBuf));
    pb_ostream_t stream = pb_ostream_from_buffer(bleTxBuf, sizeof(bleTxBuf));
    bool status = pb_encode_delimited(&stream, Beflex_BiomechEngine_Message_fields, &message);
    size_t len = stream.bytes_written;

    if (!status) {
        //Serial.print("[PB\t] encoding failed : ");
        //Serial.println(PB_GET_ERROR(&stream));
        return 0;
    }
    //bleSerial.write(bleTxBuf, len);
    
    return (len);
}

void bleProtoProc(const GBMES_T *bmes,uint16 cid)
{
    Beflex_BiomechEngine_Message bmeMessage = Beflex_BiomechEngine_Message_init_zero;
    pb_istream_t stream = pb_istream_from_buffer(bleRxBuf, sizeof(bleRxBuf));
    bool status = pb_decode_delimited(&stream, Beflex_BiomechEngine_Message_fields, &bmeMessage);
    size_t rLen = 0;

    if (!status) {
        //Serial.print("[PB\t] decoding failed : ");
        //Serial.println(PB_GET_ERROR(&stream));
        return;
    }
    
    MAKE_BME300_SERVER_MESSAGE(GATT_BME300_SERVER_WRITE_READY_IND);
    message->bmes = bmes;
    message->cid = cid;
    MessageSend(bmes->app_task, GATT_BME300_SERVER_WRITE_READY_IND, message);

    if (bmeMessage.which_typeName == Beflex_BiomechEngine_Message_calibrationRequest_tag) {
        uint8_t duration;

        switch (bmeMessage.typeName.calibrationRequest.type) {
            case Beflex_BiomechEngine_Calibration_Type_LOOK_FORWARD:
                duration = bmeMessage.typeName.calibrationRequest.period;
                //Serial.println("[PB] calibration look forward for " + String(duration) + "sec");
                BME_calibrateGaitSensorForward(duration);
                break;
            case Beflex_BiomechEngine_Calibration_Type_LOOK_DOWN:
                duration = bmeMessage.typeName.calibrationRequest.period;
                //Serial.println("[PB] calibration look down for " + String(duration) + "sec");
                BME_calibrateGaitSensorDown(duration);
                break;
            case Beflex_BiomechEngine_Calibration_Type_FINISH:
                //Serial.println("[PB] calibration finish");
                BME_getGaitSensorCalibrationValue(gaitSensorCal);
                rLen = bleProtoSendCalibrationResponse(Beflex_BiomechEngine_Calibration_Type_FINISH, TRUE, (pb_byte_t *)gaitSensorCal);
                break;
        }
    }
    else if (bmeMessage.which_typeName == Beflex_BiomechEngine_Message_parameterRequest_tag) {
        uint8_t cycleTime;
        float height;

        switch (bmeMessage.typeName.parameterRequest.type) {
            case Beflex_BiomechEngine_Parameter_Type_START:
                cycleTime = bmeMessage.typeName.parameterRequest.period;
                height = bmeMessage.typeName.parameterRequest.height;
                //Serial.print("[PB] parameter request. duration: " + String(cycleTime) + "sec, height: " + String(height));
                if (bmeMessage.typeName.parameterRequest.calValue.size == sizeof(gaitSensorCal)) {
                    memcpy((pb_byte_t *)gaitSensorCal, bmeMessage.typeName.parameterRequest.calValue.bytes, sizeof(gaitSensorCal));

                    BME_setGaitSensorCalibrationValue(gaitSensorCal);
                    BME_startGaitSensor(cycleTime, height);
                    isGaitSensorRunning = TRUE;
                    rLen = bleProtoSendParameterResponse(Beflex_BiomechEngine_Parameter_Type_START, TRUE);
                }
                else {
                    //Serial.println(", calVal: incorrect size");
                    rLen = bleProtoSendParameterResponse(Beflex_BiomechEngine_Parameter_Type_START, FALSE);
                }
                break;
            case Beflex_BiomechEngine_Parameter_Type_FINISH:
                // stopping gait sensor is done by ble disconnect callback function..
                // this routine is not used for now
                break;
        }
    }
    else {
        //Serial.println("[PB\t] invalid message type");
        return;
    }
    
    if(rLen)
    {
        GATT_BME300_SERVER_WRITE_CLIENT_CONFIG_IND_T* msg = (GATT_BME300_SERVER_WRITE_CLIENT_CONFIG_IND_T *)PanicNull(calloc(1,sizeof(GATT_BME300_SERVER_WRITE_CLIENT_CONFIG_IND_T)));
        msg->bmes = bmes;
        msg->cid = cid;
        msg->bmeIns = bmePacketIns;
        msg->size_value = (uint16)rLen;
        MessageSend(bmes->app_task, GATT_BME300_SERVER_WRITE_CLIENT_CONFIG_IND, msg);
    }
}

void updateGaitPram(const GBMES_T *bmes,uint16 cid)
{
    size_t rLen = 0;
    
    if (isGaitParametersUpdated) {
    
        uint8_t mode = gaitParameters.header & 0xFF;
        uint8_t valid = (gaitParameters.header >> 8) * 0xFF;
        
        isGaitParametersUpdated = FALSE;
        
        PRINT(("mode: 0x%x valid : 0x%x",mode,valid));
    
        if (valid) {
            switch (mode) {
                case BME_PARAM_GAIT_SENSOR_MODE_GAIT:
                {
                    PRINT(("mode: gait"));
                    rLen = bleProtoSendGaitParameters();
                }
                break;
                case BME_PARAM_GAIT_SENSOR_MODE_CAL_FORWARD:
                {
                    PRINT(("mode : calibration forward"));
                    rLen = bleProtoSendCalibrationResponse(Beflex_BiomechEngine_Calibration_Type_LOOK_FORWARD, true, NULL);
                }
                break;
                case BME_PARAM_GAIT_SENSOR_MODE_CAL_DOWN:
                {
                    PRINT(("mode : calibration down"));
                    rLen = bleProtoSendCalibrationResponse(Beflex_BiomechEngine_Calibration_Type_LOOK_DOWN, true, NULL);
                }
                break;
            }
            
            if (rLen)
            {
                /* On a Read, ask the app for current client config value */
                MAKE_BME300_SERVER_MESSAGE(GATT_BME300_SERVER_WRITE_CLIENT_CONFIG_IND);
                message->bmes = bmes;
                message->cid = cid;
                message->bmeIns = bmePacketIns;
                message->size_value = (uint16)rLen;
                MessageSend(bmes->app_task, GATT_BME300_SERVER_WRITE_CLIENT_CONFIG_IND, message);
            }
            
        }
        else {
            //Serial.println("invalid gait data!");
        }
    }
}

/* Macros */
#define sendBme300ServerAccessErrorRsp(bmes, access_ind, error) sendBme300ServerAccessRsp((Task)&bmes->lib_task, access_ind->cid, access_ind->handle, error, 0, NULL);


/***************************************************************************
NAME
    sendBme300ServerAccessRsp

DESCRIPTION
    Send an access response to the GATT Manager library.
*/
static void sendBme300ServerAccessRsp(Task task, uint16 cid, uint16 handle, uint16 result, uint16 size_value, const uint8 *value)
{
    if (!GattManagerServerAccessResponse(task, cid, handle, result, size_value, value))
    {
        /* The GATT Manager should always know how to send this response */
        PRINT(("Couldn't send GATT access response\n"));
    }
}


/***************************************************************************
NAME
    handleBme300ServerAccessInd

DESCRIPTION
    Handles the GATT_MANAGER_ACCESS_IND message that was sent to the library.
*/
static void handleBme300ServerAccessInd(GBMES_T *const bmes, const GATT_MANAGER_SERVER_ACCESS_IND_T *access_ind)
{
    PRINT((" Function:handleBme300ServerAccessInd(), Handle = %x \n",access_ind->handle));

    switch (access_ind->handle)
    {
        case HANDLE_BME300_TX_CLIENT_CONFIG:
        {
            if (!GattManagerServerAccessResponse(&bmes->lib_task, access_ind->cid, HANDLE_BME300_TX_CLIENT_CONFIG, gatt_status_success,  0, NULL))
            {
                /* The GATT Manager should always know how to send this response */
                PRINT(("Couldn't send GATT access response\n"));
            }
            bmePacketIns.bmeSrc = NULL;
            bmePacketIns.bmeSnk = NULL;
            bmePacketIns.handle_data_endpoint = 0;
            bmePacketIns.handle_response_endpoint= 0;
 
            /* On a Read, ask the app for current client config value */
            MAKE_BME300_SERVER_MESSAGE(GATT_BME300_SERVER_READ_CLIENT_CONFIG_IND);
            message->bmes = bmes;
            message->cid = access_ind->cid;
            message->bmeIns = bmePacketIns;
            MessageSend(bmes->app_task, GATT_BME300_SERVER_READ_CLIENT_CONFIG_IND, message);
         }
        break;

        case HANDLE_BME300_UART_RX:
        {
            if (!GattManagerServerAccessResponse(&bmes->lib_task, access_ind->cid, HANDLE_BME300_UART_RX, gatt_status_success,  0, NULL))
            {
                /* The GATT Manager should always know how to send this response */
                PRINT(("Couldn't send GATT access response\n"));
            } 
            memset(bleRxBuf, 0, sizeof(bleRxBuf));
            memcpy(bleRxBuf, access_ind->value, access_ind->size_value);
            
            bleProtoProc(bmes,access_ind->cid);
        }
        break;

        case HANDLE_BME300_RX_CLIENT_CONFIG:
        {
            if (!GattManagerServerAccessResponse(&bmes->lib_task, access_ind->cid, HANDLE_BME300_RX_CLIENT_CONFIG, gatt_status_success,  0, NULL))
            {
                 PRINT(("Couldn't send GATT access response\n"));
            }
          }
        break;

        default:
        {
            /* Respond to invalid handles */
            sendBme300ServerAccessErrorRsp(bmes, access_ind,gatt_status_invalid_handle);
         }
        break;
    }
}

/****************************************************************************/
static void bme300ServerMsgHandler(Task task, MessageId id, Message msg)
{
    GBMES_T *bmes = (GBMES_T*)task;

    switch (id)
    {
        case GATT_MANAGER_SERVER_ACCESS_IND:
        {
            /* Read/write access to characteristic */
            handleBme300ServerAccessInd(bmes, (const GATT_MANAGER_SERVER_ACCESS_IND_T *)msg);
        }
        break;
        
        case GATT_MANAGER_DISCONNECT_IND:
        {
            GATT_MANAGER_DISCONNECT_IND_T *gattDisconn = (GATT_MANAGER_DISCONNECT_IND_T *)msg;
            
            MAKE_BME300_SERVER_MESSAGE(GATT_BME300_SERVER_DISCONNECT_IND);
            message->bmes = bmes;
            message->cid = gattDisconn->cid;
            message->status = gattDisconn->status;
            MessageSend(bmes->app_task, GATT_BME300_SERVER_DISCONNECT_IND, message);
        }
        break;

        default:
        {
            /* Unrecognized GATT Manager message */
            PRINT(("BMES: GATT Manager Server Msg 0x%x not handled\n",id));
        }
        break;
    }
}


/****************************************************************************/
uint8 GattBme300ServerInit(Task appTask, GBMES_T *const bmes, uint16 start_handle, uint16 end_handle)
{   
    /*Registration parameters for TDS library to GATT manager  */
    gatt_manager_server_registration_params_t reg_params;
    
    /* validate the input parameters */
    if ((appTask == NULL) || (bmes == NULL ))
    {
        PRINT(("GHRS: Invalid Initialization parameters"));
    }

    /* Reset all the service library memory */
    memset(bmes, 0, sizeof(GBMES_T));
    
    /* Set up the library task handler for external messages
     * TDS library receives GATT manager messages here
     */
    bmes->lib_task.handler = bme300ServerMsgHandler;
    /*Store application message handler as application messages need to be posted here */
    bmes->app_task = appTask;
    /* Fill in the registration parameters */
    reg_params.start_handle = start_handle;
    reg_params.end_handle   = end_handle;
    reg_params.task = &bmes->lib_task;
    /* Try to register this instance of BME300 library to GATT manager */
    if (GattManagerRegisterServer(&reg_params) == gatt_manager_status_success)
    {
        /* Mark Lib init is done */
       return TRUE;
    }
    return FALSE;
}

