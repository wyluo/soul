/*!
\copyright  Copyright (c) 2008 - 2018 Qualcomm Technologies International, Ltd.\n
            All Rights Reserved.\n
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       av_headset_charger.c
\brief      Charger monitoring
*/

#include "panic.h"

#include "av_headset.h"
#include "av_headset_ui.h"
#include "av_headset_charger.h"
#include "av_headset_log.h"
#include "av_headset_temperature.h"
#include "av_headset_sm_private.h"//jim

#ifdef SOUL_BLADE
#include "av_headset_conn_rules.h"
#endif

/**************************************************************************/
/*! /brief Check if we KNOW that the system can power off.

    Power off can only happen when the charger is not active, so this 
    fucntion checks the charger state to determine if charger has power.

    If the power status is not known, the function assumes that we cannot 
    power off. An example would be when charger detection is still pending
    and the USB status is unknown.
    
    \note If charger support has not been included, then this function
    will return FALSE as the system behaviour is not known/detectable.

    \returns TRUE if it is certain there is no external power
 */
bool appChargerCanPowerOff(void)
{
#ifdef INCLUDE_CHARGER
    switch (ChargerStatus())
    {
        case DISABLED_ERROR:
        case NO_POWER:
            /* Charger Status is reliable only after charger detection is complete */
            return (!appChargerDetectionIsPending());
            
        #ifdef SOUL_BLADE
        case STANDBY:
            DEBUG_LOGF("go to POWER OFF, Charger Status STANDBY!");
            return TRUE;
        #endif

        default:
            return FALSE;
    }
#else
    /* Assume we can't turn off */
    return FALSE;
#endif
}
#ifdef Jim
bool attach_cn=0;//add by jim / kimny
#endif

/**************************************************************************/
/*! \brief Check if a charger is connected

    \note   The value returned if the charger is disabled is indeterminate. 
            This function should only be relied upon if the charger is enabled.

    \note   If there is no charger support in the project, then this 
            function will return TRUE

    \return Charger connection state
 */
chargerConnectionState appChargerIsConnected(void)
{
#ifdef INCLUDE_CHARGER
    switch (UsbAttachedStatus())
    {
        case DETACHED:
		#ifndef Jim
        case UNKNOWN_STATUS:
		#endif
            return CHARGER_DISCONNECTED;

        default:
        {
            switch (ChargerStatus())
            {
                case TRICKLE_CHARGE:
                case FAST_CHARGE:
                case STANDBY:
                case PRE_CHARGE:
                    return CHARGER_CONNECTED_NO_ERROR;

                default:
                    return CHARGER_CONNECTED;
            }
        }
    }
#else
    return TRUE;
#endif
}



/* Only compile if CHARGER defined */
#ifdef INCLUDE_CHARGER

/*! Interval at which the charger status is checked */
#define CHARGER_READ_PERIOD (D_SEC(1))

/*! Add reason for the charger to be disabled */
static void appChargerDisableReasonAdd(chargerDisableReason reason);
/*! Clear reason for the charger to be disabled */
static void appChargerDisableReasonClear(chargerDisableReason reason);

/*! Internal message IDs used by the Charger module */
enum av_headset_charger_internal_messages
{
    /*! Periodically poll the charger status */
    CHARGER_INTERNAL_TIMER  = INTERNAL_MESSAGE_BASE + 0x80,
    /*! Used to limit the time spent in some charge phases */
    CHARGER_INTERNAL_CHARGE_TIMEOUT,
};

/**************************************************************************/
static  bool appChargerIsCharging(void)
{
    switch (UsbAttachedStatus())
    {
        case DETACHED:
            return FALSE;

        default:
        {
            switch (ChargerStatus())
            {
                case TRICKLE_CHARGE:
                case FAST_CHARGE:
                case PRE_CHARGE:
                    return TRUE;
                default:
                    return FALSE;
            }
        }
    }
}

/**************************************************************************/
static void appChargerCheck(void)
{
    chargerTaskData *theCharger = appGetCharger();
    chargerConnectionState is_connected = appChargerIsConnected();
    bool is_charging = appChargerIsCharging();

#ifdef SOUL_BLADE
    DEBUG_LOGF("appChargerCheck, status %u,%u, connected %u,%u, charging %u",
        theCharger->status, ChargerStatus(),
        theCharger->is_connected, is_connected,
        is_charging);

    if (is_charging)
    {
        //appLedEnable(TRUE);
        //appUiCharging();    
    }
    else if (!is_connected)  
    {
		if(DUT_Flag==0)
		appUiPowerOn();
    }
#endif    
    if ((theCharger->status != ChargerStatus()) ||
        (theCharger->is_connected != is_connected))
    {
        DEBUG_LOGF("appChargerCheck, status %u,%u, connected %u,%u, charging %u",
                   theCharger->status, ChargerStatus(),
                   theCharger->is_connected, is_connected,
                   is_charging);
    }

    if (theCharger->status != ChargerStatus())
    {
        uint32 charge_timeout_ms = 0;
        theCharger->status = ChargerStatus();
        switch (theCharger->status)
        {
            case TRICKLE_CHARGE:
                DEBUG_LOG("appChargerCheck, trickle charge");
                break;
            case FAST_CHARGE:
                DEBUG_LOG("appChargerCheck, fast charge");
                charge_timeout_ms = appConfigChargerFastChargeTimeoutMs();
                break;
            case DISABLED_ERROR:
                DEBUG_LOG("appChargerCheck, disabled");
                break;
            case STANDBY:
                DEBUG_LOG("appChargerCheck, standby");
                break;
            case PRE_CHARGE:
                DEBUG_LOG("appChargerCheck, pre charge");
                charge_timeout_ms = appConfigChargerPreChargeTimeoutMs();
                break;
            case NO_POWER:
                DEBUG_LOG("appChargerCheck, no power");
                break;
            case HEADROOM_ERROR:
                DEBUG_LOG("appChargerCheck, headroom error");
                break;
            default:
                DEBUG_LOGF("appChargerCheck, status %d", ChargerStatus());
                break;
        }

        MessageCancelAll(&theCharger->task, CHARGER_INTERNAL_CHARGE_TIMEOUT);
        if (charge_timeout_ms)
        {
            MessageSendLater(&theCharger->task, CHARGER_INTERNAL_CHARGE_TIMEOUT,
                             NULL, charge_timeout_ms);
        }
    }

    /* Check if connected status has changed */
    if (is_connected != theCharger->is_connected)
    {
        /* Check if disconnected */
        if (is_connected == CHARGER_DISCONNECTED)
        {
            /* Indicate charger disconnection */
		if(DUT_Flag==0)
            appUiChargerDisconnected();

            /* Cancel any timers running */
            MessageCancelAll(&theCharger->task, CHARGER_INTERNAL_TIMER);

            /* Clear charging flag */
            theCharger->is_charging = FALSE;

            /* Allow the battery to charge after timeout if the charger is disconnected */
            appChargerDisableReasonClear(CHARGER_DISABLE_REASON_TIMEOUT);

            /* Inform all clients that charger has detached */
            appTaskListMessageSendId(theCharger->client_tasks, CHARGER_MESSAGE_DETACHED);
        }
        else
        {
            /* Inform all clients that charger has attached when old state was disconnected */
            if (theCharger->is_connected == CHARGER_DISCONNECTED)
                appTaskListMessageSendId(theCharger->client_tasks, CHARGER_MESSAGE_ATTACHED);

            /* Indicate charger connection */
            appUiChargerConnected();

            /* Clear charging flag now to kick off charging indication later */
            theCharger->is_charging = FALSE;

            /* Check if charger is connected with no error before attempt supply switch */
            if (is_connected == CHARGER_CONNECTED_NO_ERROR)
            {
                /* Attempt to switch chip supply to charger */
                if (!PsuConfigure(PSU_ALL, PSU_SMPS_INPUT_SEL_VBAT, 0))
                {
                    /* Switch failed, so charger isn't working correctly */
                    DEBUG_LOG("appChargerCheck, failed to switch to charge supply");
                    is_connected = CHARGER_CONNECTED;

                    /* Disable and re-enable charger to 'reset' it */
                    PanicFalse(ChargerConfigure(CHARGER_ENABLE, 0));
                    PanicFalse(ChargerConfigure(CHARGER_ENABLE, 1));
                }
            }
        }

        /* Store updated connected state */
        theCharger->is_connected = is_connected;
    }
    
    /* Check if connected */
    if (is_connected == CHARGER_CONNECTED_NO_ERROR)
    {
            DEBUG_LOG("appBatteryGetVoltage(): %d\n\n", appBatteryGetVoltage());    
        /* Check if we have finished charging */
        if (!is_charging && theCharger->is_charging)
        {
            appUiChargerComplete();
			#ifdef Jim
			//appLedCancelFilter(1);
			//appLedEnable(FALSE);
			if(!PsuGetVregEn())
			MessageSendLater(appGetSmTask(), SM_INTERNAL_TIMEOUT_LIMBO, NULL,2000);//my_appPowerDoPowerOff();
			
			#endif
			
		}
        else if (is_charging)
        {
            if (appBatteryGetVoltage() > appConfigBatteryVoltageOk())
            {
            if(DUT_Flag==0)
                appUiChargerChargingOk();
            }
            else
            {
				if(DUT_Flag==0)
				appUiChargerChargingLow();
            }
        }

        /* Store updated charging status */
        theCharger->is_charging = is_charging;
    }

    /* Start charger poll timer if charger is connected and still charging */
    if (is_connected && is_charging)
    {
        MessageCancelAll(&theCharger->task, CHARGER_INTERNAL_TIMER);
        MessageSendLater(&theCharger->task, CHARGER_INTERNAL_TIMER, 0, CHARGER_READ_PERIOD);
    }
}

/*! Handle a temperature state change */
static void appChargerHandleTemperatureStateChange(const TEMPERATURE_STATE_CHANGED_IND_T *ind)
{
    switch (ind->state)
    {
        case TEMPERATURE_STATE_WITHIN_LIMITS:
            DEBUG_LOG("appChargerHandleTemperatureStateChange, within limits");
            appChargerDisableReasonClear(CHARGER_DISABLE_REASON_EXTREME_TEMPERATURE);
        break;

        case TEMPERATURE_STATE_ABOVE_UPPER_LIMIT:
        case TEMPERATURE_STATE_BELOW_LOWER_LIMIT:
            DEBUG_LOG("appChargerHandleTemperatureStateChange, outside limits");
            appChargerDisableReasonAdd(CHARGER_DISABLE_REASON_EXTREME_TEMPERATURE);
        break;

        default:
            Panic();
        break;
    }
}

/*! Handle a charge timeout */
static void appChargerHandleChargeTimeout(void)
{
    chargerTaskData *theCharger = appGetCharger();
    DEBUG_LOGF("appChargerHandleChargeTimeout, charger_status %d", theCharger->status);
    appChargerDisableReasonAdd(CHARGER_DISABLE_REASON_TIMEOUT);
}


#ifdef Jim
extern uint8 percent_t;	//add by kimny / jim

#endif
static void appChargerHandleChargerAttachStatus(usb_attached_status attached_status)
{
    DEBUG_LOG("appChargerHandleChargerAttachStatus, attached status %d", attached_status);
    switch (attached_status)
    {
        case DETACHED:
            DEBUG_LOG("appChargerHandleChargerAttachStatus, charger not detected");
            appChargerDisableReasonAdd(CHARGER_DISABLE_REASON_NOT_DETECTED);
#ifdef Jim

            if(attach_cn==1)//add by jim / kimny
            {
                my_PsKey_WriteTure();
                attach_cn=0;
                appSmCancelLimboTimeout();
				if(DUT_Flag==0)
				{
				appLedEnable(TRUE);
				appLedSetPattern(app_led_pattern_power_on, LED_PRI_EVENT);
				}
				//sinkStartAutoEnterPairingTimer();
				sinkStartAutoReconectHeadsetTimer();
				MessageCancelAll(appGetUiTask(), APP_Battery_Critical); 
				percent_t=0;
            }
#endif
        break;

        case HOST_OR_HUB:
        case DEDICATED_CHARGER:
        case HOST_OR_HUB_CHARGER:
        case CHARGING_PORT:
        case NON_COMPLIANT_CHARGER:
        case FLOATING_CHARGER:
        case NON_USB_CHARGER:
            DEBUG_LOG("appChargerHandleChargerAttachStatus, charger detected");
            appChargerDisableReasonClear(CHARGER_DISABLE_REASON_NOT_DETECTED);
        break;

        case UNKNOWN_STATUS:
            // Temporary state returned by the FW, the charger detection is not complete yet
            DEBUG_LOG("appChargerHandleChargerAttachStatus, charger detection not complete. No action taken");
            // Charger will be enable if MESSAGE_CHARGER_DETECTED is received
        break;
        /* No default, all USB status covered */
    }
}

static void appChargerHandleChargerDetected(const MessageChargerDetected *msg)
{
    appChargerHandleChargerAttachStatus(msg->attached_status);
    appChargerCheck();
}

/**************************************************************************/
static void appChargerHandleMessage(Task task, MessageId id, Message message)
{
    UNUSED(task);
    UNUSED(message);

    switch (id)
    {
        case TEMPERATURE_STATE_CHANGED_IND:
            appChargerHandleTemperatureStateChange(message);
        break;

        case CHARGER_INTERNAL_CHARGE_TIMEOUT:
            appChargerHandleChargeTimeout();
        break;

        case MESSAGE_CHARGER_DETECTED:
            appChargerHandleChargerDetected(message);
        break;

        default:
            /* Check for charger events */
            appChargerCheck();
        break;
    }
}

/* Set the configuration of the charger.
 */
static void appChargerConfigureCharger(void)
{
	#ifdef Jim
	MessageCancelAll(appGetUiTask(), Ptek_IR_Det);
	my_PsKey_WriteTure();
	#endif
    PanicFalse(ChargerConfigure(CHARGER_ENABLE, 0));

    /* Currents */
    PanicFalse(ChargerConfigure(CHARGER_TRICKLE_CURRENT, appConfigChargerTrickleCurrent()));
    PanicFalse(ChargerConfigure(CHARGER_PRE_CURRENT, appConfigChargerPreCurrent()));
    PanicFalse(ChargerConfigure(CHARGER_FAST_CURRENT, appConfigChargerFastCurrent()));
    PanicFalse(ChargerConfigure(CHARGER_ITERM_CTRL, appConfigChargerTerminationCurrent()));

    /* Voltages */
    PanicFalse(ChargerConfigure(CHARGER_PRE_FAST_THRESHOLD, appConfigChargerPreFastThresholdVoltage()));
    PanicFalse(ChargerConfigure(CHARGER_TERM_VOLTAGE, appConfigChargerTerminationVoltage()));

    /* Thresholds/timing */
    PanicFalse(ChargerConfigure(CHARGER_STANDBY_FAST_HYSTERESIS, appConfigChargerStandbyFastVoltageHysteresis()));
    PanicFalse(ChargerConfigure(CHARGER_STATE_CHANGE_DEBOUNCE, appConfigChargerStateChangeDebounce()));

    DEBUG_LOG("appChargerConfigureCharger, charger enabled");
	//add by jim / kimny
	#ifdef Jim
    attach_cn=1;
    percent_t = 0;
    //check_poweroff_count = 0;
    //is_poweringoff_flag=0; // add at 20190829
	MessageCancelAll(appGetUiTask(), APP_Battery_Critical); 
	#endif
    PanicFalse(ChargerConfigure(CHARGER_ENABLE, 1));
}

static void appChargerDisableReasonAdd(chargerDisableReason reason)
{
    chargerTaskData *theCharger = appGetCharger();
    if (reason)
    {
        if (!theCharger->disable_reason)
        {
            DEBUG_LOG("appChargerConfigureCharger, charger disabled");
            PanicFalse(ChargerConfigure(CHARGER_ENABLE, 0));
        }
        theCharger->disable_reason |= reason;
    }
}

static void appChargerDisableReasonClear(chargerDisableReason reason)
{
    chargerTaskData *theCharger = appGetCharger();
    if (reason && theCharger->disable_reason)
    {
        theCharger->disable_reason &= ~reason;
        if (!theCharger->disable_reason)
        {
            appChargerConfigureCharger();
        }
    }
}

#endif /* INCLUDE_CHARGER */

/*! \brief Make sure the charger is disabled

    This function should be called from power off code to make sure that
    the charger is off.
*/
void appChargerForceDisable(void)
{
#ifdef INCLUDE_CHARGER
    appChargerDisableReasonAdd(CHARGER_DISABLE_REASON_REQUEST);
#endif
}

/*! \brief Restore the charger, if there are no other reasons for the charger
    to be disabled.

    This function should only be called after a call to
    \ref appChargerForceDisable().
*/
void appChargerRestoreState(void)
{
#ifdef INCLUDE_CHARGER
    appChargerDisableReasonClear(CHARGER_DISABLE_REASON_REQUEST);
#endif
}


/*! \brief Initialise the application handling of charger

    This function should be called during application initialisation.
    If charger support is not required, the function ensures that
    the charger is disabled.
*/
#ifdef INCLUDE_CHARGER
void appChargerInit(void)
{
    bool registered;
    chargerTaskData *theCharger = appGetCharger();

    /* Set up task handler & record current charger status */
    theCharger->task.handler = appChargerHandleMessage;
    theCharger->client_tasks = appTaskListInit();
    theCharger->is_connected = CHARGER_DISCONNECTED;
    theCharger->is_charging = FALSE;
    theCharger->status = ENABLE_FAIL_UNKNOWN;
    theCharger->disable_reason = CHARGER_DISABLE_REASON_NONE;

    /* Register for charger messages */
    MessageChargerTask(&theCharger->task);

    /* Need to switch charger when temperature is outside battery's operating range.
       TEMPERATURE_STATE_CHANGED_IND will trigger enable/disable of the charger. */
    appChargerDisableReasonAdd(CHARGER_DISABLE_REASON_EXTREME_TEMPERATURE);

    /* Need to enable the charger when it has been detected */
    appChargerHandleChargerAttachStatus(UsbAttachedStatus());

    registered = appTemperatureClientRegister(&theCharger->task,
                                              appConfigBatteryChargingTemperatureMin(),
                                              appConfigBatteryChargingTemperatureMax());
    if (!registered)
    {
        /* Could not register for temperature, just enable the charger.
           Should only happen when temperature support is not compiled. */
        appChargerDisableReasonClear(CHARGER_DISABLE_REASON_EXTREME_TEMPERATURE);
    }
}
#endif /* INCLUDE_CHARGER */

bool appChargerClientRegister(Task client_task)
{
#ifdef INCLUDE_CHARGER
    chargerTaskData *theCharger = appGetCharger();
    if (appTaskListAddTask(theCharger->client_tasks, client_task))
    {
        /* Send initial state if charger is attached */
        if (appChargerIsConnected() == CHARGER_CONNECTED_NO_ERROR)
        {
            MessageSend(client_task, CHARGER_MESSAGE_ATTACHED, NULL);
        }
        else
        {
            MessageSend(client_task, CHARGER_MESSAGE_DETACHED, NULL);
        }
        return TRUE;
    }
#else
    UNUSED(client_task);
#endif
    return FALSE;
}


void appChargerClientUnregister(Task client_task)
{
#ifdef INCLUDE_CHARGER
    chargerTaskData *theCharger = appGetCharger();
    appTaskListRemoveTask(theCharger->client_tasks, client_task);
#else
    UNUSED(client_task);
#endif
}

