/****************************************************************************
Copyright (c) 2005 - 2016 Qualcomm Technologies International, Ltd.
*/
/**
\file
\ingroup sink_app
\brief
    Application level control of AT commands sent and received via the HFP
    connections.

    These can be configured in 2 ways,
    received AT commands that ate unrecognised by the HFP library will be passed
    to the applciation (here)

    If these match any of the commands in the cofiguration, then a response also
    in the configuration will be sent to the AG device.

    Alternatively, User events can be configured to send configured AT commands
    (from the same list) to the AG on receipt of any user event,

    E.g. This allows the user to configure an event that can be sent to the AG on
    connection or when the battery level changes.

NOTES


*/
/****************************************************************************
    Header files
*/
#include "av_headset_at_commands.h"
#include <string.h>
#include <byte_utils.h>
#include <stdlib.h>
#include "av_headset.h"
#include "av_headset_latency.h"
#include "av_headset_power.h"
#include "av_headset_log.h"
#include "av_headset_ui.h"

/* vender id - product id - version- report bit */
static const char batt_enable_string[]   = "AT+XAPL=05AC-1702-0100,7\r";
static const char batt_level_string[]   = "AT+IPHONEACCEV=1,1,0\r";

#define BATT_POS   19

/****************************************************************************
DESCRIPTION
    Sends a given AT command to handset

*/
void avAtCmdEnableBattReport(void)
{
     DEBUG_PRINTF("AT Send:[%s]\n", batt_enable_string);
     HfpAtCmdRequest(hfp_primary_link, batt_enable_string);
}

void avAtCmdSendBatt(uint8 percent)
{
    char *at_command_data;

    at_command_data = malloc(sizeof(batt_level_string));
    
    if (at_command_data)
    {
        memcpy(at_command_data, batt_level_string, sizeof(batt_level_string));
		
        DEBUG_LOGF("AT: Send Cmd battery percent %d\n",percent) ;
	 at_command_data[BATT_POS]=	(percent/10)+0x30;
                
        DEBUG_PRINTF("AT Send:[%s]\n", at_command_data);
        HfpAtCmdRequest(hfp_primary_link, at_command_data);
			
    }
    free(at_command_data);
}

#ifdef Jim
uint8 percent_t=0;	//add by kimny
#endif
static void appAtCmdHandleBatteryLevelUpdatePercent(MESSAGE_BATTERY_LEVEL_UPDATE_PERCENT_T *msg)
{
    DEBUG_LOG("AT: appAtCmdHandleBatteryLevelUpdatePercent\n") ;
	
    #ifdef Jim
    if(appChargerIsConnected()==CHARGER_DISCONNECTED)
    {
        if(msg->percent < 21 && (percent_t==0)) // add by jim 20190701
        {
            percent_t = 1;
			appUiBATTERY_LOW_16k();
            MessageSendLater(appGetUiTask(), APP_Battery_Critical, NULL, D_SEC(5));
        }
        if(msg->percent >=21 && (percent_t == 1))
        {
            percent_t = 0;
            MessageCancelAll(appGetUiTask(), APP_Battery_Critical);
        }
        if(msg->percent < 2 && (percent_t==1))
        {
            MessageSendLater(appGetUiTask(), OTA_Sync_POWER_OFF, NULL, D_SEC(5));
        }
    }
    #endif

    if(appDeviceIsHandsetHfpConnected())
       avAtCmdSendBatt(msg->percent);
}

typedef struct
{
    TaskData task;
    batteryRegistrationForm Client; 
} atTaskData;

static void appAtCmdHandleMessage(Task task, MessageId id, Message message)
{
    UNUSED(task);
    UNUSED(message);
    /* Handle internal messages */
    switch (id)
    {
        case MESSAGE_BATTERY_LEVEL_UPDATE_PERCENT:
            appAtCmdHandleBatteryLevelUpdatePercent((MESSAGE_BATTERY_LEVEL_UPDATE_PERCENT_T *)message);
            break;
			
	 default: break;
    }
}

atTaskData atTask; 
void avAtCmdInit(void)
{
    /* Set up task handler */
    atTask.task.handler = appAtCmdHandleMessage;
	
    atTask.Client.task = &atTask.task;
    atTask.Client.hysteresis = 1;
    atTask.Client.representation = battery_level_repres_percent;
	
    appBatteryRegister(&atTask.Client);
}
