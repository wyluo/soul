/*!
\copyright  Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
            All Rights Reserved.
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version
\file       av_headset_gatt_ptek.c
\brief      Application support for PTEK, HR Service
*/

#include "av_headset.h"

#ifdef INCLUDE_PTEK
#include <system_clock.h>
#include <bitserial_api.h>
#include "av_headset_db.h"
#include "av_headset_log.h"
#include "av_headset_gatt.h"

#include "av_headset_test_le.h"


#include <gatt.h>
#include <gatt_manager.h>
#include <gatt_server.h>
#include <pio.h>
#include <pio_common.h>
#include <panic.h>
#include <util.h>
#include <gatt_heart_rate_server.h>

#define Ptek_debug   0

/*! Macro to make a message. */
#define MAKE_MESSAGE(TYPE) TYPE##_T *message = PanicUnlessNew(TYPE##_T);

/*
    The simulated heart rate measurement notification size.
       -------------------------------------
       | Flags    |   HR meas Value |  RR Interval
       ------------------------------------
       Flags:            1 byte
       HR Meas value:    1 byte
       RR interval:      2 bytes
*/
#define HEART_RATE_NOTIFICATION_SIZE    4

#define PIO2BANK(pio) ((uint16)((pio) / 32))
/*! \brief Returns the PIO bit position mask within a bank.
    \param pio The pio.
*/
#define PIO2MASK(pio) (1UL << ((pio) % 32))
bitserial_handle gPtekHandle = BITSERIAL_HANDLE_ERROR;

#define I2C_BUFFER_LEN 8
#define START_CMD_LEN 5 // 6
#define SET_CMD_LEN 20 //21
#define GET_CMD_LEN 6 //7
#define READ_MAX_CNT 10

static bool ptekStartCmd(bitserial_handle handle);
static bool ptekStopCmd(bitserial_handle handle);

static void delay_ms(uint16 v_msec_16)
{
    /** add one to the delay to make sure we don't return early */
    uint32 v_delay = SystemClockGetTimerTime() + (v_msec_16 * 1000) + 1;

    while (((int32)(SystemClockGetTimerTime() - v_delay)) < 0);
}

static int getReadSize(bitserial_handle handle)
{
    bitserial_result result = BITSERIAL_RESULT_TIMEOUT;
    int i, read_cnt=0, read_size = 0;
    uint8 offset_rsp_len = PTEK_RSP_LENGTH_OFFSET;
    uint8 value[2] = {0,0};

    for(i = 0;i <READ_MAX_CNT;i++)
    {
        DEBUG_LOG("result = BitserialWrite==========3");
        result = BitserialWrite(handle,
                                BITSERIAL_NO_MSG,
                                &offset_rsp_len, 1,
                                 BITSERIAL_FLAG_BLOCK);
                                /*BITSERIAL_FLAG_NO_STOP);*/
        DEBUG_LOG("result = BitserialWrite==========4");

        if (result == BITSERIAL_RESULT_SUCCESS){
            result = BitserialRead(handle,
                                    BITSERIAL_NO_MSG,
                                    value, 2,
                                    BITSERIAL_FLAG_BLOCK);
            //DEBUG_LOG("getReadSize = 0x%x 0x%x", value[0], value[1]);
        }
        DEBUG_LOG("result = BitserialWrite==========5");

        if(value[0])
        {
            read_cnt++;
            if(read_size == value[0])
            {
                read_size = value[0];
                break;
            }
            else
            {
                read_size = value[0];
            }
        }
        //delay_ms(1);
    }

    return read_size;

}

static bitserial_result getReadOffset(bitserial_handle handle, uint8* value, uint16 len)
{
    bitserial_result result = BITSERIAL_RESULT_TIMEOUT;
    uint8 offset_reg = PTEK_READ_OFFSET;
#if 1
    //DEBUG_LOG("getReadOffset==========1");

    result = BitserialWrite(handle,
                            BITSERIAL_NO_MSG,
                            &offset_reg, sizeof(offset_reg),
                            BITSERIAL_FLAG_NO_STOP);
    //DEBUG_LOG("getReadOffset==========2");

    if (result == BITSERIAL_RESULT_SUCCESS){
        result = BitserialRead(handle,
                                BITSERIAL_NO_MSG,
                                value, len,
                                BITSERIAL_FLAG_BLOCK);
    }
#else
    result = BitserialTransfer(handle, BITSERIAL_NO_MSG, &offset_reg, 1, value, len);
#endif

    return result;

}



/*----------------------------------------------------------------------------*
 *  NAME
 *     OffSkin sensor Reading
 *
 *  DESCRIPTION
 *      This function read offskin sensor
 *
 *  RETURNS
 *      None
 *
 *---------------------------------------------------------------------------*/
#if 0
static void ReadOffSkin(bitserial_handle handle)
{
#ifdef Jim
uint8 offskin_get_cmd[GET_CMD_LEN] = {0x00, 0x5, PTEK_PKT_START_CHAR, 0x02, PTEK_CMD_GET, PTEK_OFFSKIN};
#else
uint8 offskin_get_cmd[GET_CMD_LEN] = {PTEK_I2C_READ_ADDR, 0x00, 0x5, PTEK_PKT_START_CHAR, 0x02, PTEK_CMD_GET, PTEK_OFFSKIN};
#endif
bitserial_result result = BITSERIAL_RESULT_TIMEOUT;
int read_size = 0;
#ifdef USE_MALLOC
    uint8 *offSkin_value;
#else
    uint8 offSkin_value[6];
#endif


    if(handle == BITSERIAL_HANDLE_ERROR)
        return;

    result = BitserialWrite(handle,
                            BITSERIAL_NO_MSG,
                            offskin_get_cmd, sizeof(offskin_get_cmd),
                            BITSERIAL_FLAG_BLOCK);

    /*  Read 0x7C */
    read_size = getReadSize(handle);
    DEBUG_LOG("ReadOffSkin result= %d read %d ", result, read_size);

    if(read_size==0) /* Read data must be at least 3 bytes. */
    {
        return;
    }

#ifdef USE_MALLOC
    offSkin_value = malloc(read_size);

    if(!offSkin_value)
        return;
#else
    memset(offSkin_value,0,6);
#endif

    /*  Read Offset 0x7F */
    if (BITSERIAL_RESULT_SUCCESS == getReadOffset(handle,offSkin_value, (uint16)read_size)){
        /* Min : 40bpm (rest:25bpm), Max : 225bpm */
        if(read_size>=2)
            DEBUG_LOG("ReadOffSkin RxData[0] = 0x%x 0x%x",offSkin_value[0], offSkin_value[1]);
        if(read_size>=4)
            DEBUG_LOG("ReadOffSkin RxData[2] = 0x%x 0x%x", offSkin_value[2], offSkin_value[3]);
        if(read_size >=6)
            DEBUG_LOG("ReadOffSkin RxData[4] = 0x%x 0x%x",offSkin_value[4], offSkin_value[5]);
    }

#ifdef USE_MALLOC
    free(offSkin_value);
#endif

}

#endif
/*----------------------------------------------------------------------------*
 *  NAME
 *     RMeasReading
 *
 *  DESCRIPTION
 *      This function formulates HR Measurement data in a format
 *      given by HR service specification. It simulates static
 *      HR measurement reading with RR interval and no enerygy expended field.
 *
 *  RETURNS
 *      None
 *
 *---------------------------------------------------------------------------*/
bool NotSendHR_Flag;
bool BleHR_Flag;

static void HRMeasReading(bitserial_handle handle, uint8 *p_hr_meas, uint8 *p_length, uint8 sensor_status)
{
    uint8  *p_hr_meas_flags = NULL;
    bitserial_result result = BITSERIAL_RESULT_TIMEOUT;
    #ifdef Jim
    uint8 get_cmd[GET_CMD_LEN] = { 0x00, 0x04, PTEK_PKT_START_CHAR, 0x02, PTEK_CMD_GET, PTEK_HEART_RATE};
#else
    uint8 get_cmd[GET_CMD_LEN] = {PTEK_I2C_READ_ADDR, 0x00, 0x05, PTEK_PKT_START_CHAR, 0x02, PTEK_CMD_GET, PTEK_HEART_RATE};
#endif
    int read_size = 0;
#ifdef USE_MALLOC
    uint8 *value;
#else
    uint8 value[6];
#endif

    /*
     *  --------------------------------------
     *  | Flags      |   HR meas Value |  RR Intervals -----------
     *  --------------------------------------
     *      Flags field:
     *              Bit 0: Heart Rate Value Format bit, 0 - UINT8 Format, 1 - UINT16 Format
     *              Bit 1: Sensor Contact Status bit, 0 - No or Poor Contact, 1 - In contact
     *              Bit 2: Sensor Contact Support bit, 0 - Sensor Contact feature is supported, 1 - Sensor Contact feature not supported
     *              Bit 3: Energy Expended Status bit, 0 - Energy Expended field not present, 1 - Energy Expended field is present
     *              Bit 4: RR-Interval bit, 0 - RR-Interval values are not present, 1 - RR-Interval values present
     *              Bits 5-7: RFU (Resereved for future use must be set to 0)
     *
     *      Heart Rate Measurement Value Field:
     *              <= 255 bpm if Heart Rate Value Format bit set to 0, >255 bpm Heart Rate Value Format bit set to 1
     *
     *      RR-Interval Field:
     *              One or mor RR-Interval values if  RR-Interval bit set to 1.
     */

    p_hr_meas_flags = &p_hr_meas[(*p_length)++];
    *p_hr_meas_flags = SENSOR_MEASUREVAL_FORMAT_UINT8 |sensor_status;

    if(handle == BITSERIAL_HANDLE_ERROR)
        goto Failed;

    result = BitserialWrite(handle,
                            BITSERIAL_NO_MSG,
                            get_cmd, sizeof(get_cmd),
                            BITSERIAL_FLAG_BLOCK);

    /*  Read 0x7C */
    read_size = getReadSize(handle);
    DEBUG_LOG("HRMeasReading result= %d read %d ", result, read_size);

    if(read_size==0) /* Read data must be at least 3 bytes. */
    {
        goto Failed;
    }

#ifdef USE_MALLOC
    value = malloc(read_size);

    if(!value)
        goto Failed;
#else
    memset(value,0,6);
#endif

    /*  Read Offset 0x7F */
    if (BITSERIAL_RESULT_SUCCESS == getReadOffset(handle,value,(uint16)read_size)){
        /* Min : 40bpm (rest:25bpm), Max : 225bpm */
        if(read_size == 2)
        {
            p_hr_meas[(*p_length)++] = value[1];
        }
        else if(read_size == 6)
        {
            p_hr_meas[(*p_length)++] = value[5];
        }

        //if(read_size>=2)DEBUG_LOG("HRMeasReading read_size = %d RxData[0] = 0x%x 0x%x",read_size, value[0], value[1]);
        //if(read_size>=4)DEBUG_LOG("HRMeasReading RxData[2] = 0x%x 0x%x", value[2], value[3]);
        //if(read_size >=6)DEBUG_LOG("HRMeasReading RxData[4] = 0x%x 0x%x",value[4], value[5]);
    }
    else
    {
        p_hr_meas[(*p_length)++] = 0;
        p_hr_meas[(*p_length)++] = 0;
    }

#if 0 //RR-Interval
    *p_hr_meas_flags |= RR_INTERVAL_PRESENT;

    /* 480 +/- 32 */
    p_hr_meas[(*p_length)++] = LE8_L(RR_INTERVAL_IN_MS) + (32 - (int32)UtilRandom() % 16);
    p_hr_meas[(*p_length)++] = LE8_H(RR_INTERVAL_IN_MS);
#endif


#ifdef USE_MALLOC
    free(value);
#endif

    return;

  Failed:
    {
      p_hr_meas[(*p_length)++] = 0;
      p_hr_meas[(*p_length)++] = 0;
      //ptekReset((const ptekConfig *)appConfigPtek());
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      sinkBleStopHrSimulation
 *
 *  DESCRIPTION
 *      This function sends handles EventSysBleHrSensorNotInContact and sends final
 *      HR Measurement data with Sensor Contact Bit set to "NOT IN CONTACT"
 *
 *  RETURNS
 *      None
 *
 *---------------------------------------------------------------------------*/
static void appBleStopHrSimulation(uint16 cid)
{
    gattGattServerInfo* pInfo = appGattGetInstanceFromConnId(cid);
    uint8 *p_hr_meas = malloc(HEART_RATE_NOTIFICATION_SIZE);

    /* Cancel the next scheduled Heart rate measurement */
//            MessageCancelFirst( sinkGetBleTask(), BLE_INTERNAL_MESSAGE_HR_READ_TIMER );

    if(p_hr_meas != NULL)
    {
        uint8  hr_meas_len  = 0;

         /* Read simulated HR measurement and send dummy data */
        HRMeasReading(gPtekHandle, p_hr_meas, &hr_meas_len, SENSOR_IN_CONTACT);

        /* Send the final Heart Rate Measurement notification with sensor in contact bit set to 0 */
        GattHrServerSendNotification(&(pInfo->hr_server),
                          cid, hr_meas_len, p_hr_meas);
        DEBUG_LOG("1===GATT Heart Rate Measurement %d bpm\n", p_hr_meas[1]);
        /* Free the memory */
        free(p_hr_meas);
    }
    else
    {
        /* Could not allocate memory */
        DEBUG_LOG("GATT Malloc Failed\n");
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      getHRMeasReading
 *
 *  DESCRIPTION
 *      This function sends handles EventSysBleHrSensorNotInContact and sends final
 *      HR Measurement data with Sensor Contact Bit set to "NOT IN CONTACT"
 *
 *  RETURNS
 *      None
 *
 *---------------------------------------------------------------------------*/
static void appBleNotifyHrMeasurements(GATT_HR_SERVER_TIMER_IND_T * ind)
{
    gattGattServerInfo* pInfo = appGattGetInstanceFromConnId(ind->cid);
#ifdef USE_MALLOC
    uint8 *p_hr_meas = malloc(HEART_RATE_NOTIFICATION_SIZE);
#else
    uint8 p_hr_meas[HEART_RATE_NOTIFICATION_SIZE]= {0,};
#endif

    MessageCancelAll(appGetGattHrsTask(), GATT_HR_SERVER_TIMER_IND);
    MAKE_MESSAGE(GATT_HR_SERVER_TIMER_IND);

    //DEBUG_LOGF("GATT_HR_SERVER_READ_CLIENT_CONFIG_IND hrs=[0x%p] cid=[0x%x]\n", (void *)ind->hr_sensor, ind->cid);
    message->hr_sensor = ind->hr_sensor;     /* Pass the instance which can be returned in the response */
    message->cid = ind->cid;                 /* Pass the CID so the client can be identified */
    //DEBUG_LOGF("GATT_HR_SERVER_READ_CLIENT_CONFIG_IND message=[0x%p]", message);
    MessageSendLater(appGetGattHrsTask(), GATT_HR_SERVER_TIMER_IND,message, GATT_SERVER_HR_UPDATE_TIME);

#ifdef USE_MALLOC
    if(p_hr_meas != NULL)
#endif
    {
        uint8  hr_meas_len  = 0;

         /* Read simulated HR measurement and send dummy data */
         HRMeasReading(gPtekHandle,p_hr_meas, &hr_meas_len, SENSOR_IN_CONTACT);

        /* Send the final Heart Rate Measurement notification with sensor in contact bit set to 0 */
        if(NotSendHR_Flag==0)
        {
            GattHrServerSendNotification(&(pInfo->hr_server),
                                  ind->cid, hr_meas_len, p_hr_meas);
            DEBUG_LOG("2===GATT Heart Rate Measurement %d bpm\n", p_hr_meas[1]);
        }
        //free(p_hr_meas);

#ifdef USE_MALLOC
        /* Free the memory */
        free(p_hr_meas);
#endif

        //ReadOffSkin(gPtekHandle);
    }
#ifdef USE_MALLOC
    else
    {
        /* Could not allocate memory */
        DEBUG_LOG("GATT Malloc Failed\n");
    }
#endif

}

/******************************************************************************/
void appBleHRSensorInContact(GATT_HR_SERVER_TIMER_IND_T * ind)
{
    if(FALSE == ptekGetIntOutPinStatus((const ptekConfig *)appConfigPtek()))
    {
        //ptekReset((const ptekConfig *)appConfigPtek());
        delay_ms(1);
    }
    appBleNotifyHrMeasurements(ind);
    /* Start a 10 second simulation timer after which the heart rate measurement notifications shall be stopped */
//    MessageSendLater( &theSink.task, EventSysBleHrSensorNotInContact , 0, STOP_HR_NOTIFICATION_TIME);
}

/******************************************************************************/
void appBleHRSensorNotInContact(uint16 cid)
{
    /* Sensor not in contact stop sending notifications */
    appBleStopHrSimulation(cid);
}

static bool ptekStartCmd(bitserial_handle handle)
{
    bitserial_result result = BITSERIAL_RESULT_TIMEOUT;
    #ifdef Jim
    uint8 start_cmd[START_CMD_LEN] = { 0x00, 0x03, PTEK_PKT_START_CHAR, 0x01, PTEK_CMD_START};
#else
    uint8 start_cmd[START_CMD_LEN] = {PTEK_I2C_WRITE_ADDR, 0x00, 0x03, PTEK_PKT_START_CHAR, 0x01, PTEK_CMD_START};
#endif
    uint8* value;
    int read_size = 0;

    result = BitserialWrite(handle,
                            BITSERIAL_NO_MSG,
                            start_cmd, sizeof(start_cmd),
                            BITSERIAL_FLAG_BLOCK);

    /*  Read 0x7C */
    read_size = getReadSize(handle);

    DEBUG_LOG("ptekStartCmd result= %d size %d", result, read_size);

    if(read_size==0) /* Read data must be at least 3 bytes. */
        return FALSE;

    value = malloc(read_size);

    if(!value)
        return FALSE;

    /*  Read Offset 0x7F */
    if (BITSERIAL_RESULT_SUCCESS == getReadOffset(handle,value,(uint16)read_size)){
        if(read_size>=2)
            DEBUG_LOG("start RxData[0] = 0x%x 0x%x", value[0], value[1]);
        if(read_size>=4)
            DEBUG_LOG("start RxData[2] = 0x%x 0x%x", value[2], value[3]);
    }
    free(value);

    return (result == BITSERIAL_RESULT_SUCCESS);
}

static bool ptekStopCmd(bitserial_handle handle)
{
    bitserial_result result = BITSERIAL_RESULT_TIMEOUT;
#ifdef Jim
    uint8 start_cmd[START_CMD_LEN] = { 0x00, 0x03, PTEK_PKT_START_CHAR, 0x01, PTEK_CMD_STOP};
#else
    uint8 start_cmd[START_CMD_LEN] = {PTEK_I2C_WRITE_ADDR, 0x00, 0x03, PTEK_PKT_START_CHAR, 0x01, PTEK_CMD_STOP};
#endif
    uint8* value;
    int read_size = 0;

    result = BitserialWrite(handle,
                            BITSERIAL_NO_MSG,
                            start_cmd, sizeof(start_cmd),
                            BITSERIAL_FLAG_BLOCK);

    /*  Read 0x7C */
    read_size = getReadSize(handle);
    DEBUG_LOG("ptekStopCmd result= %d size %d", result, read_size);

    if(read_size==0) /* Read data must be at least 3 bytes. */
        return FALSE;

    value = malloc(read_size);

    if(!value)
        return FALSE;

    /*  Read Offset 0x7F */
    if (BITSERIAL_RESULT_SUCCESS == getReadOffset(handle,value,(uint16)read_size)){
        //if(read_size>=2)DEBUG_LOG("Stop RxData[0] = 0x%x 0x%x", value[0], value[1]);
        //if(read_size>=4)DEBUG_LOG("Stop RxData[2] = 0x%x 0x%x", value[2], value[3]);
    }
    free(value);

    return (result == BITSERIAL_RESULT_SUCCESS);
}

#if 1
static bool ptekSetCmd(bitserial_handle handle)
{
    bitserial_result result = BITSERIAL_RESULT_TIMEOUT;
#ifdef Jim
    uint8 set_cmd[SET_CMD_LEN] = { 0x00, 0x12, PTEK_PKT_START_CHAR, 0x10, PTEK_CMD_SET, 0x16, 0x01, 0x9E,
                        0x17, 0x00, 0x00, 0x18, 0x01, 0xF4, 0x19, 0x00, 0xA8, 0x1B,
                        0x00, 0x01};
#else
    uint8 set_cmd[SET_CMD_LEN] = {PTEK_I2C_WRITE_ADDR, 0x00, 0x12, PTEK_PKT_START_CHAR, 0x10, PTEK_CMD_SET, 0x16, 0x01, 0x9E,
                        0x17, 0x00, 0x00, 0x18, 0x01, 0xF4, 0x19, 0x00, 0xA8, 0x1B,
                        0x00, 0x01};
#endif
    uint8* value;
    int read_size = 0;

    result = BitserialWrite(handle,
                            BITSERIAL_NO_MSG,
                            set_cmd, sizeof(set_cmd),
                            BITSERIAL_FLAG_BLOCK);

    /*  Read 0x7C */
    read_size = getReadSize(handle);

    DEBUG_LOG("ptekSetCmd result= %d size %d", result, read_size);

    if(read_size==0) /* Read data must be at least 3 bytes. */
        return FALSE;

    value = malloc(read_size);

    if(!value)
        return FALSE;

    /*  Read Offset 0x7F */
    if (BITSERIAL_RESULT_SUCCESS == getReadOffset(handle,value,(uint16)read_size)){
        //if(read_size>=2)DEBUG_LOG("SetCmd RxData[0] = 0x%x 0x%x", value[0], value[1]);
        //if(read_size>=4)DEBUG_LOG("SetCmd RxData[2] = 0x%x 0x%x", value[2], value[3]);
    }
    free(value);

    return (result == BITSERIAL_RESULT_SUCCESS);
}
#endif

#if 1
static bool ptekGetCmd(bitserial_handle handle, uint8 command, uint8* value, uint8* read_size)
{
    bitserial_result result = BITSERIAL_RESULT_TIMEOUT;
#ifdef Jim
    uint8 get_cmd[GET_CMD_LEN] = { 0x00, 0x04, PTEK_PKT_START_CHAR, 0x02, PTEK_CMD_GET, command};
#else
    uint8 get_cmd[GET_CMD_LEN] = {PTEK_I2C_WRITE_ADDR, 0x00, 0x05, PTEK_PKT_START_CHAR, 0x02, PTEK_CMD_GET, command};
#endif
    result = BitserialWrite(handle,
                            BITSERIAL_NO_MSG,
                            get_cmd, sizeof(get_cmd),
                            BITSERIAL_FLAG_BLOCK);


    /*  Read 0x7C */
    *read_size = getReadSize(handle);

    DEBUG_LOG("ptekGetCmd CMD 0x%X size %d result= %d ", command, *read_size, result);

    if(*read_size==0) /* Read data must be at least 3 bytes. */
        return FALSE;

    //if(!value)value = PanicUnlessMalloc(*read_size);

    /*  Read Offset 0x7F */
    if (BITSERIAL_RESULT_SUCCESS == getReadOffset(handle,value,(uint16)*read_size)){
        //if(*read_size>=2)DEBUG_LOG("ptekGetCmd CMD 0x%X RxData[0] = 0x%x 0x%x",command, value[0], value[1]);
        //if(*read_size>=4)DEBUG_LOG("ptekGetCmd CMD 0x%X RxData[2] = 0x%x 0x%x",command, value[2], value[3]);
        //if(*read_size >=6)DEBUG_LOG("ptekGetCmd CMD 0x%X RxData[4] = 0x%x 0x%x",command, value[4], value[5]);

        return TRUE;
    }

    return FALSE;
}
#endif


static bool checkPtekIdleMode(void)
{
    uint8 value[6] = {0,};
    uint8 read_size = 0;
    bool ret = FALSE;

    if(ptekGetCmd(gPtekHandle,(uint8)PTEK_POST_RESULT,(uint8*)value,&read_size))
    {
        //DEBUG_LOG("checkPtekIdleMode read_size= %d",read_size);

        if(value)
        {
           // if(read_size>=2)
            //    DEBUG_LOG("checkPtekIdleMode RxData[0] = 0x%x 0x%x", value[0], value[1]);
            //if(read_size>=4)
            //    DEBUG_LOG("checkPtekIdleMode RxData[2] = 0x%x 0x%x", value[2], value[3]);
            if(read_size >=6)
            {
                //DEBUG_LOG("checkPtekIdleMode RxData[4] = 0x%x 0x%x", value[4], value[5]);
                if(value[5]== 0)
                {
                    ret = TRUE;
                    //DEBUG_LOG("checkPtekIdleMode ret TRUE = 0x%x",value[5]);
                }
                else
                {
                    //delay_ms(100);
                    //DEBUG_LOG("checkPtekIdleMode ret FALSE = 0x%x",value[5]);
                }
            }
            //free(value); /* must free malloc after ptekGetCmd success */
        }
    }
    return ret;
}



bool ptekGetIntOutPinStatus(const ptekConfig *config)
{
    bool ret;
    if (PioCommonGetPio(config->pios.int_out) == TRUE) {
        //DEBUG_LOG("ptek INT OUT PIN = ON =======");
        ret = TRUE;
    }
    else {
        //DEBUG_LOG("ptek INT OUT PIN = OFF =======");
        ret = FALSE;
    }

    return ret;
}


bitserial_handle ptekEnable(const ptekConfig *config)
{
    bitserial_config bsconfig;
    uint32 i;
    uint16 bank;
    uint32 mask;

    struct
    {
        uint16 pio;
        pin_function_id func;
    } i2c_pios[] = {{config->pios.i2c_scl, BITSERIAL_0_CLOCK_OUT},
                    {config->pios.i2c_scl, BITSERIAL_0_CLOCK_IN},
                    {config->pios.i2c_sda, BITSERIAL_0_DATA_OUT},
                    {config->pios.i2c_sda, BITSERIAL_0_DATA_IN}};

    /* Setup power PIO then power-on the sensor */
    bank = PIO2BANK(config->pios.on);
    mask = PIO2MASK(config->pios.on);
    PanicNotZero(PioSetMapPins32Bank(bank, mask, mask));
    PanicNotZero(PioSetDir32Bank(bank, mask, mask));
    PanicNotZero(PioSet32Bank(bank, mask, mask));
    PanicNotZero(PioSetStrongBias32Bank(bank, mask, mask));


    //ptekReset((const ptekConfig *)appConfigPtek());// jim

    bank = PIO2BANK(config->pios.rst);
    mask = PIO2MASK(config->pios.rst);
    PanicNotZero(PioSetMapPins32Bank(bank, mask, mask));
    PanicNotZero(PioSetDir32Bank(bank, mask, 0));
    PanicNotZero(PioSet32Bank(bank, mask, mask));


    /* Setup Interrupt  */
    bank = PIO2BANK(config->pios.interrupt);
    mask = PIO2MASK(config->pios.interrupt);
    PanicNotZero(PioSetMapPins32Bank(bank, mask, mask));
    PanicNotZero(PioSetDir32Bank(bank, mask, 0));
    PanicNotZero(PioSet32Bank(bank, mask, mask));

    delay_ms(500);

    /* Setup Int out : input  */
    bank = PIO2BANK(config->pios.int_out);
    mask = PIO2MASK(config->pios.int_out);
    PanicNotZero(PioSetMapPins32Bank(bank, mask, 0));
    PanicNotZero(PioSetDir32Bank(bank, mask, 0));

    for (i = 0; i < ARRAY_DIM(i2c_pios); i++)
    {
        uint16 pio = i2c_pios[i].pio;
        bank = PIO2BANK(pio);
        mask = PIO2MASK(pio);

        /* Setup I2C PIOs with strong pull-up */
        PanicNotZero(PioSetMapPins32Bank(bank, mask, 0));
        PanicFalse(PioSetFunction(pio, i2c_pios[i].func));
        //Add by jim
        //PanicNotZero(PioSetDir32Bank(bank, mask, 0));
        //PanicNotZero(PioSet32Bank(bank, mask, mask));
        //PanicNotZero(PioSetStrongBias32Bank(bank, mask, mask));
    }

    /* Configure Bitserial to work with ptek */
    memset(&bsconfig, 0, sizeof(bsconfig));
    bsconfig.mode = BITSERIAL_MODE_I2C_MASTER;
    bsconfig.clock_frequency_khz = config->i2c_clock_khz;
    bsconfig.u.i2c_cfg.i2c_address = 0x45;
    //bsconfig.u.i2c_cfg.flags = BITSERIAL_I2C_ACT_ON_NAK_STOP;
    gPtekHandle = BitserialOpen((bitserial_block_index)BITSERIAL_BLOCK_0, &bsconfig);
    //gPtekHandle = BitserialOpen((bitserial_block_index)BITSERIAL_BLOCK_1, &bsconfig);
    DEBUG_LOG("ptekEnable 0x%x",gPtekHandle);

    delay_ms(100);

    ptekSetCmd(gPtekHandle);
    ptekStartCmd(gPtekHandle);

    for(i=0;i<READ_MAX_CNT;i++)
    {
        if(checkPtekIdleMode())
            break;
    }

    return gPtekHandle;
}


void ptekDisable(const ptekConfig *config)
{
    uint16 bank;
    uint32 mask;
    DEBUG_LOG("ptekDisable");

    //MessageCancelAll(appGetGattHrsTask(), GATT_HR_SERVER_TIMER_IND);
    ptekStopCmd(gPtekHandle);

    /* Disable interrupt and set weak pull down */
    bank = PIO2BANK(config->pios.interrupt);
    mask = PIO2MASK(config->pios.interrupt);
    PanicNotZero(PioSet32Bank(bank, mask, 0));

    /* Release bitserial instance */
    BitserialClose(gPtekHandle);
    gPtekHandle = BITSERIAL_HANDLE_ERROR;

    /* Power off ptek */
    PanicNotZero(PioSet32Bank(PIO2BANK(config->pios.on),
                              PIO2MASK(config->pios.on),
                              0));
}

void ptekReset(const ptekConfig *config)
{
    uint16 bank;
    uint32 mask;
    DEBUG_LOG("ptekReset");

    MessageCancelAll(appGetGattHrsTask(), GATT_HR_SERVER_TIMER_IND);
    bank = PIO2BANK(config->pios.interrupt);
    mask = PIO2MASK(config->pios.interrupt);
    PanicNotZero(PioSet32Bank(bank, mask, 0));

    bank = PIO2BANK(config->pios.rst);
    mask = PIO2MASK(config->pios.rst);
    PanicNotZero(PioSetMapPins32Bank(bank, mask, mask));
    PanicNotZero(PioSetDir32Bank(bank, mask, 0));
    PanicNotZero(PioSet32Bank(bank, mask, 0));
    delay_ms(1);
    PanicNotZero(PioSet32Bank(bank, mask, mask));
    delay_ms(100);

    bank = PIO2BANK(config->pios.interrupt);
    mask = PIO2MASK(config->pios.interrupt);
    PanicNotZero(PioSetDir32Bank(bank, mask, 0));
    PanicNotZero(PioSet32Bank(bank, mask, mask));

}



void handleReadHrMeasurementClientConfig(GATT_HR_SERVER_READ_CLIENT_CONFIG_IND_T * ind)
{
    gattGattServerInfo* pInfo = appGattGetInstanceFromConnId(ind->cid);
    uint16 client_config = 0;

    /* Return the current value of the client configuration descriptor for the device */
    DEBUG_LOG("GATT_HR_SERVER_READ_CLIENT_CONFIG_IND hrs=[0x%p] cid=[0x%x]\n", (void *)ind->hr_sensor, ind->cid);

        client_config = pInfo->config.heart_rate;

    GattHrServerReadClientConfigResponse(ind->hr_sensor, ind->cid, client_config);
    DEBUG_LOG("  client_config=[0x%x]\n", client_config);
}

void handleWriteHrMeasurementClientConfig(GATT_HR_SERVER_WRITE_CLIENT_CONFIG_IND_T * ind)
{
    gattGattServerInfo* pInfo = appGattGetInstanceFromConnId(ind->cid);

    /*
     * Check whether the remote device has enabled or disabled
     * notifications for the Heart rate Measurement characteristic. This value will need
     * to be stored as device attributes so they are persistent.
    */
    DEBUG_LOG("GATT_HR_SERVER_WRITE_CLIENT_CONFIG_IND hrs=[0x%p] hr_server=[0x%p] value[0x%x]\n",
        (void *)ind->hr_sensor, &(pInfo->hr_server), ind->config_value);

    if(ind->hr_sensor == &(pInfo->hr_server))
    {

        pInfo->config.heart_rate = ind->config_value;
        DEBUG_LOG(" Heart rate Sensor client_config[0x%x]\n", pInfo->config.heart_rate);
        if(!(pInfo->config.heart_rate & 1))
        {
            /* Disable the HR measurement notifications */
           // MessageCancelFirst( sinkGetBleTask(), BLE_INTERNAL_MESSAGE_HR_READ_TIMER );
        }
        else
        {
            MessageCancelAll(appGetGattHrsTask(), GATT_HR_SERVER_TIMER_IND);
            MAKE_MESSAGE(GATT_HR_SERVER_TIMER_IND);
            message->hr_sensor = ind->hr_sensor;     /* Pass the instance which can be returned in the response */
            message->cid = ind->cid;                 /* Pass the CID so the client can be identified */
            MessageSend(appGetGattHrsTask(), GATT_HR_SERVER_TIMER_IND,message);
        }
    }
}

#define PtekIRx

static bool ptekGetCmd_Read(bitserial_handle handle, uint8 command, uint8* value, uint8* read_size)
{
    bitserial_result result = BITSERIAL_RESULT_TIMEOUT;

    uint8 get_cmd[GET_CMD_LEN] = { 0x00, 0x04, PTEK_PKT_START_CHAR, 0x02, PTEK_CMD_GET, command};
    //DEBUG_LOG("result = BitserialWrite==========1");
    result = BitserialWrite(handle,
                            BITSERIAL_NO_MSG,
                            get_cmd, sizeof(get_cmd),
                            BITSERIAL_FLAG_BLOCK);

    //DEBUG_LOG("result = BitserialWrite==========2");
    //Read 0x7C
    *read_size = getReadSize(handle);

    DEBUG_LOG("ptekGetCmd CMD 0x%X size %d result= %d ", command, *read_size, result);

    if(*read_size==0)return FALSE; // Read data must be at least 3 bytes.

    //if(!value)value = PanicUnlessMalloc(*read_size);

    //Read Offset 0x7F
    if (BITSERIAL_RESULT_SUCCESS == getReadOffset(handle,value,(uint16)*read_size))
    {
        //if(*read_size>=2)DEBUG_LOG("ptekGetCmd CMD 0x%X RxData[0] = 0x%x 0x%x",command, value[0], value[1]);
        //if(*read_size>=4)DEBUG_LOG("ptekGetCmd CMD 0x%X RxData[2] = 0x%x 0x%x",command, value[2], value[3]);
        //if(*read_size >=6)DEBUG_LOG("ptekGetCmd CMD 0x%X RxData[4] = 0x%x 0x%x",command, value[4], value[5]);
        return TRUE;
    }

    return FALSE;
}


static bool ReadOptical(void)//static void ReadOptical(void)
{
    uint8 value[6] = {0,};
    uint8 read_size = 0;
    bool ret = FALSE;

    if(ptekGetCmd_Read(gPtekHandle,(uint8)0x12,(uint8*)value,&read_size))
    {
        //DEBUG_LOG("checkPtekIdleMode read_size= %d",read_size);

        if(value)
        {
            //if(read_size>=2)DEBUG_LOG("checkPtekIdleMode RxData[0] = 0x%x 0x%x", value[0], value[1]);
            //if(read_size>=4)DEBUG_LOG("checkPtekIdleMode RxData[2] = 0x%x 0x%x", value[2], value[3]);
            if(read_size >=6)
            {
                //DEBUG_LOG("checkPtekIdleMode RxData[4] = 0x%x 0x%x", value[4], value[5]);
                if(value[4] > 0x10)
                {
                    ret = TRUE;
                    //DEBUG_LOG("checkPtekIdleMode ret TRUE = 0x%x",value[4]);
                }
                else
                {
                    //DEBUG_LOG("checkPtekIdleMode ret FALSE = 0x%x",value[4]);
                }
            }
            //free(value); /* must free malloc after ptekGetCmd success */
        }
    }
    return ret;
}
/*
static bool ReadOffSkin_IR(void)//static void ReadOptical(void)
{
    uint8 value[6] = {0,};
    uint8 read_size = 0;
    bool ret = FALSE;

    if(ptekGetCmd_Read(gPtekHandle,(uint8)0x11,(uint8*)value,&read_size))
    {
        //DEBUG_LOG("checkPtekIdleMode read_size= %d",read_size);

        if(value)
        {
            //if(read_size>=2)DEBUG_LOG("checkPtekIdleMode RxData[0] = 0x%x 0x%x", value[0], value[1]);
            //if(read_size>=4)DEBUG_LOG("checkPtekIdleMode RxData[2] = 0x%x 0x%x", value[2], value[3]);
            if(read_size >=6)
            {
                //DEBUG_LOG("checkPtekIdleMode RxData[4] = 0x%x 0x%x", value[4], value[5]);
                if((value[5]&0x01) == 0)
                {
                    ret = TRUE;
                    //DEBUG_LOG("checkPtekIdleMode ret TRUE = 0x%x",value[5]);
                }
                else
                {
                    //delay_ms(100);
                    //DEBUG_LOG("checkPtekIdleMode ret FALSE = 0x%x",value[5]);
                }
            }
            //free(value); //must free malloc after ptekGetCmd success
        }
    }
    return ret;
}
*/

bool Pre_Play=0;
bool Pre_Play1=0;

bool IR_play=0;
void PtekIR_Det(void)
{

    Pre_Play = ReadOptical();
    //Pre_Play1 = ReadOffSkin_IR();
    //DEBUG_LOG("Pre_Play ========= 0x%x",Pre_Play);
    //DEBUG_LOG("IR_play ========= 0x%x",IR_play);
    if(IR_play != Pre_Play)
    {
        //DEBUG_LOG("BME\t2] ir_out: %d ir_gng : %d \n", PioCommonGetPio(accel->ir_out),PioCommonGetPio(accel->ir_gng));
        IR_play = Pre_Play;
        if (appSmIsOutOfCase())
        {
            if ( appDeviceIsHandsetAvrcpConnected()||(appDeviceIsPeerAvrcpConnectedForAv() && appPeerSyncIsComplete() && appPeerSyncIsPeerHandsetAvrcpConnected()))
            {
                if (Pre_Play)
                {
                    if(appHfpIsCall()==FALSE)
                    {
                        appAvPlay(FALSE);
                    }
                    if((NotSendHR_Flag==1) && (BleHR_Flag ==1))
                    {
                        NotSendHR_Flag = 0;
                    }
                    MessageCancelAll(appGetUiTask(), Out_of_ear);
                }
                else
                {
                    MessageSendLater(appGetUiTask(), Out_of_ear, NULL, D_SEC(60));
                    if(appHfpIsCall()==FALSE)
                    {
                        appAvPause(FALSE);
                    }
                }

            }
        }
    }
}

void Out_of_ear_HR(void)
{
if(BleHR_Flag ==1)
    NotSendHR_Flag = 1;
else
    NotSendHR_Flag = 0;

    //MessageCancelAll(appGetGattHrsTask(), GATT_HR_SERVER_TIMER_IND);
}

#endif

