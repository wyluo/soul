/*!
\copyright  Copyright (c) 2017 Qualcomm Technologies International, Ltd.
            All Rights Reserved.
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       av_headset_bme300.c
\brief      Support for the bme300 accelerometer
*/
#ifdef INCLUDE_BME300
#include <bitserial_api.h>
#include <panic.h>
#include <pio.h>
#include <pio_common.h>
#include <system_clock.h>
#include <hydra_macros.h>
#include <input_event_manager.h>

#include "../av_headset.h"
#include "../av_headset_log.h"
#include "../av_headset_accelerometer.h"
#include "bme300.h"

#define jim_IRx

/*! \brief Returns the PIOs bank number.
    \param pio The pio.
*/
#define PIO2BANK(pio) ((uint16)((pio) / 32))
/*! \brief Returns the PIO bit position mask within a bank.
    \param pio The pio.
*/
#define PIO2MASK(pio) (1UL << ((pio) % 32))


#ifdef IR_ENABLE
#include "../av_headset_ir.h"

static void delay_us(uint16 v_msec_16)
{
    /** add one to the delay to make sure we don't return early */
    uint32 v_delay = SystemClockGetTimerTime() + (v_msec_16) + 1;

    while (((int32)(SystemClockGetTimerTime() - v_delay)) < 0);
}

/*! Get the client message based on the PIO state and mask.
    The sensor signals detected motion by setting the masked PIO high. */
static MessageId getMessage(uint32 pio_state, uint32 pio_mask)
{
    bool in_motion = (0 == (pio_state & pio_mask));
    return in_motion ? ACCELEROMETER_MESSAGE_IN_MOTION : ACCELEROMETER_MESSAGE_NOT_IN_MOTION;
}
#endif


/*! \brief Handle the accelerometer interrupt */
static void bme300InterruptHandler(Task task, MessageId id, Message msg)
{
#if defined(INCLUDE_BME300) || defined(IR_ENABLE)
    accelTaskData *accel = (accelTaskData *)task;
#else
UNUSED(task);
UNUSED(msg);
#endif    
    //DEBUG_LOGF("BME\t] Handler MessageId :  0x%x(0x%x) \n", id, ACCELEROMETER_MESSAGE_BOOT_FROM_RAM);
    switch(id)
    {
#ifdef IR_ENABLE
        case MESSAGE_PIO_CHANGED:
        {
            const MessagePioChanged *mpc = (const MessagePioChanged *)msg;
            uint32 state = mpc->state16to31 << 16 | mpc->state;
            uint16 bank = PIO2BANK(accel->ir_out);
            uint32 mask = PIO2MASK(accel->ir_out);
            if (mpc->bank == bank)
            {
                appTaskListMessageSendId(accel->clients, getMessage(state, mask));
            }
        }
        break;
       
        case ACCELEROMETER_MESSAGE_GNG_TIMER:
        {
            bool bPlay;
            MessageCancelAll(&accel->task, ACCELEROMETER_MESSAGE_GNG_TIMER);
            PioCommonSetPio(accel->ir_gng,pio_drive,TRUE);
            bPlay = (PioCommonGetPio(accel->ir_out)== FALSE);
            if(accel->ir_play != bPlay)
            {        
                DEBUG_LOGF("BME\t2] ir_out: %d ir_gng : %d \n", PioCommonGetPio(accel->ir_out),PioCommonGetPio(accel->ir_gng));
                accel->ir_play = bPlay;
                if (appSmIsOutOfCase())
                {
                    if ( appDeviceIsHandsetAvrcpConnected()||
                        (appDeviceIsPeerAvrcpConnectedForAv() && appPeerSyncIsComplete() && appPeerSyncIsPeerHandsetAvrcpConnected()))
                    {     
                        if (bPlay)
                        {
                            if(appHfpIsCall()==FALSE)
                            {
								#ifdef jim_IR
								appAvPause(FALSE);
								#else
			                    appAvPlay(FALSE);
								#endif
                            }
							
                        }
                        else
                        {
                            if(appHfpIsCall()==FALSE)
                            {
	                            #ifdef jim_IR
								appAvPlay(FALSE);
								#else
								appAvPause(FALSE);
								#endif
                            }
                        }
                    }
                }
            }

            //DEBUG_LOGF("BME\ Timer] ir_out: %d ir_gng : %d \n", PioCommonGetPio(accel->ir_out),PioCommonGetPio(accel->ir_gng));
            delay_us(250);
            PioCommonSetPio(accel->ir_gng,pio_drive,FALSE);
            MessageSendLater(&accel->task, ACCELEROMETER_MESSAGE_GNG_TIMER, NULL,100);
        }
        break;
#endif     
        
       
        case ACCELEROMETER_MESSAGE_BME_PARCER_TIMER:
        {
            uint16 *bmeParceTimer = (uint16*)msg;
            MessageCancelAll(&accel->task, ACCELEROMETER_MESSAGE_BME_PARCER_TIMER);
            
            while(BME_getIntPinStatus())
            {
                BME_parseFifo();
                
                DEBUG_LOGF("<<<<< MINI >>>>>>> BME_parseFifo()");
            }
            
            uint16* message = (uint16 *)PanicNull(calloc(1,sizeof(uint16)));
            *message = *bmeParceTimer;
            
            //DEBUG_LOGF("<<<<< MINI >>>>>>> BME_parseTimer(%d)",*message);
            MessageSendLater(&accel->task, ACCELEROMETER_MESSAGE_BME_PARCER_TIMER, message,*message);
        }
        break;
        
        case ACCELEROMETER_MESSAGE_BOOT_FROM_RAM:
        {
            BME_bootFromRam();
            DEBUG_LOGF("BME\t] Boot from RAM done\n");
            while(BME_getIntPinStatus())
            {
                BME_parseFifo();
                DEBUG_LOG("int status: 0x%x", getIntStatus());
            }
            uint16* message = (uint16 *)PanicNull(calloc(1,sizeof(uint16)));
            *message = 300;
            MessageSendLater(&accel->task, ACCELEROMETER_MESSAGE_BME_PARCER_TIMER, message,*message);
        }
        break;  
        default:
        break;
    }
}


bool appAccelerometerClientRegister(Task task)
{
    accelTaskData *accel = appGetAccelerometer();
    bool ret = FALSE;

    if (NULL == accel->clients)
    {
        accel->clients = appTaskListInit();
        accel->config = appConfigAccelerometer();

#ifdef IR_ENABLE
        accel->ir_out = IR_OUT;
        accel->ir_gng= IR_GNG;
        IREnable();
#endif
        accel->handle = BME_begin();
        PanicFalse(accel->handle != BITSERIAL_HANDLE_ERROR);     
        
        BME_softReset();
        BME_init();
        
        /* Register for interrupt events */
        accel->task.handler = bme300InterruptHandler;
        MessageSend(task, ACCELEROMETER_MESSAGE_IN_MOTION, NULL);
        ret = appTaskListAddTask(accel->clients, task);

        //rtime_t start = SystemClockGetTimerTime();
        FILE_INDEX *index=NULL;

        *index = FileFind(FILE_ROOT, accel->config->fwFilename, strlen(accel->config->fwFilename));
        /* Firmware not found */
        if(*index == FILE_NONE)
        {
            DEBUG_LOGF("BME\t] FW Not Found :  %s \n", accel->config->fwFilename);
        }
        else
        {
            if(BME_downloadFirmware((uint16)*index, accel->config->fwFilelen))
            {
                MessageSendLater(&accel->task,ACCELEROMETER_MESSAGE_BOOT_FROM_RAM,NULL,10);
            }
        } 
        //rtime_t finish = SystemClockGetTimerTime();
        //DEBUG_LOGF("BME\t] FW download done :  time %u us", rtime_sub(finish, start));
        DEBUG_LOG("[BME\t] boot from RAM");
    }
    
#ifdef IR_ENABLE
    accel->ir_play = (PioCommonGetPio(accel->ir_out)== FALSE);
    if (appSmIsOutOfCase())
    {
        if ( appDeviceIsHandsetAvrcpConnected()||
             (appDeviceIsPeerAvrcpConnectedForAv() && appPeerSyncIsComplete() && appPeerSyncIsPeerHandsetAvrcpConnected()))
        {
            if (accel->ir_play)
            {
                if(appHfpIsCall()==FALSE)
                {
                	#ifdef jim_IR
					appAvPause(FALSE);
					#else
                    appAvPlay(FALSE);
					#endif
                }
            }
            else
            {
                if(appHfpIsCall()==FALSE)
                {
					#ifdef jim_IR
					appAvPlay(FALSE);
					#else
					appAvPause(FALSE);
					#endif
                }
            }
        }
    }
    //DEBUG_LOGF("BME\t1] ir_out: %d ir_gng : %d \n", PioCommonGetPio(accel->ir_out),PioCommonGetPio(accel->ir_gng));
    delay_us(250);
    MessageCancelAll(&accel->task, ACCELEROMETER_MESSAGE_GNG_TIMER);
    PioCommonSetPio(accel->ir_gng,pio_drive,FALSE);
    MessageSendLater(&accel->task, ACCELEROMETER_MESSAGE_GNG_TIMER, NULL,100);
#endif    
    //DEBUG_LOGF("BME\t2] ir_out: %d ir_gng : %d \n", PioCommonGetPio(accel->ir_out),PioCommonGetPio(accel->ir_gng));

    return ret;
}

void appAccelerometerClientUnregister(Task task)
{
    accelTaskData *accel = appGetAccelerometer();

    
    MessageCancelAll(&accel->task, ACCELEROMETER_MESSAGE_BOOT_FROM_RAM);

    appTaskListRemoveTask(accel->clients, task);
    if (0 == appTaskListSize(accel->clients))
    {
        appTaskListDestroy(accel->clients);
        accel->clients = NULL;

        PanicFalse(accel->handle != BITSERIAL_HANDLE_ERROR);
#ifdef IR_ENABLE
        /* Unregister for interrupt events */
        InputEventManagerUnregisterTask(&accel->task, accel->ir_out);
        MessageCancelAll(&accel->task, ACCELEROMETER_MESSAGE_GNG_TIMER); 
        IRDisable();
#endif    

        BME_end();
        accel->handle = BITSERIAL_HANDLE_ERROR;
    }
}

#ifdef IR_ENABLE
bool appAccelerometerGetDormantConfigureKeyValue(dormant_config_key *key, uint32* value)
{
    accelTaskData *accel = appGetAccelerometer();
    uint8 interrupt = accel->ir_out;

    if (appConfigPioIsLed(interrupt))
    {
        *key = LED_WAKE_MASK;
        *value = 1 << appConfigPioLedNumber(interrupt);
    }
    else if (appConfigPioCanWakeFromDormant(interrupt))
    {
        *key = PIO_WAKE_MASK;
        *value = 1 << interrupt;
    }
    else
    {
        DEBUG_LOGF("The accelerometer interrupt PIO (%d) cannot wake the chip from dormant", interrupt);
        return FALSE;
    }
    return TRUE;
}
#endif
#endif /* INCLUDE_BME300 */
