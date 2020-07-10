/*!
\copyright  Copyright (c) 2008 - 2018 Qualcomm Technologies International, Ltd.\n
            All Rights Reserved.\n
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       av_headset_ui.c
\brief      Application User Interface
*/

#include <panic.h>
#include <ps.h>
#include <boot.h>
#include <stdio.h>
#include <input_event_manager.h>

#include "av_headset.h"
#include "av_headset_ui.h"
#include "av_headset_sm.h"
#include "av_headset_hfp.h"
#include "av_headset_power.h"
#include "av_headset_log.h"
#ifdef SOUL_BLADE
#include <avrcp.h>
#include <connection_no_ble.h>        
#endif

/*! Include the correct button header based on the number of buttons available to the UI */
#if defined(HAVE_9_BUTTONS)
#include "9_buttons.h"
#elif defined(HAVE_6_BUTTONS)
#include "6_buttons.h"
#elif defined(HAVE_1_BUTTON)
#include "1_button.h"
#else
#error "No buttons define found"
#endif


/*! User interface internal messasges */
enum ui_internal_messages
{
    /*! Message sent later when a prompt is played. Until this message is delivered
        repeat prompts will not be played */
    UI_INTERNAL_CLEAR_LAST_PROMPT,
};

/*! At the end of every tone, add a short rest to make sure tone mxing in the DSP doens't truncate the tone */
#define RINGTONE_STOP  RINGTONE_NOTE(REST, HEMIDEMISEMIQUAVER), RINGTONE_END

/*!@{ \name Definition of LEDs, and basic colour combinations

    The basic handling for LEDs is similar, whether there are
    3 separate LEDs, a tri-color LED, or just a single LED.
 */

#if (appConfigNumberOfLeds() == 3)
#define LED_0_STATE  (1 << 0)
#define LED_1_STATE  (1 << 1)
#define LED_2_STATE  (1 << 2)
#elif (appConfigNumberOfLeds() == 2)
/* We only have 2 LED so map all control to the same LED */
#define LED_0_STATE  (1 << 0)
#define LED_1_STATE  (1 << 1)
#define LED_2_STATE  (1 << 1)
#else
/* We only have 1 LED so map all control to the same LED */
#define LED_0_STATE  (1 << 0)
#define LED_1_STATE  (1 << 0)
#define LED_2_STATE  (1 << 0)
#endif

#ifdef SOUL_BLADE
#define LED_BLUE    (LED_0_STATE)
#define LED_WHITE   (LED_1_STATE)
#define LED_BLUE_WHITE   (LED_1_STATE|LED_0_STATE)

#define APP_POWER_LOCK_UI                           0x0001

#else
#define LED_RED     (LED_0_STATE)
#define LED_GREEN   (LED_1_STATE)
#define LED_BLUE    (LED_2_STATE)
#define LED_WHITE   (LED_0_STATE | LED_1_STATE | LED_2_STATE)
#define LED_YELLOW  (LED_RED | LED_GREEN)
#endif

/*!@} */

/*! \brief An LED filter used for battery low

    \param led_state    State of LEDs prior to filter

    \returns The new, filtered, state
*/
uint16 app_led_filter_battery_low(uint16 led_state)
{
    return (led_state) ? LED_WHITE : 0;
}

/*! \brief An LED filter used for low charging level

    \param led_state    State of LEDs prior to filter

    \returns The new, filtered, state
*/
uint16 app_led_filter_charging_low(uint16 led_state)
{
    UNUSED(led_state);
    return LED_WHITE;
}

/*! \brief An LED filter used for charging level OK

    \param led_state    State of LEDs prior to filter

    \returns The new, filtered, state
*/
uint16 app_led_filter_charging_ok(uint16 led_state)
{
    UNUSED(led_state);
    return LED_WHITE;
}

uint16 app_led_filter_DUT(uint16 led_state)
{
    UNUSED(led_state);
    return LED_BLUE_WHITE; //jim 20190305*/
}

/*! \brief An LED filter used for charging complete 

    \param led_state    State of LEDs prior to filter

    \returns The new, filtered, state
*/
uint16 app_led_filter_charging_complete(uint16 led_state)
{
    UNUSED(led_state);
    return LED_BLUE;
}

/*! \cond led_patterns_well_named
    No need to document these. The public interface is
    from public functions such as appUiPowerOn()
 */

const ledPattern app_led_pattern_Delete_Handsets[] =
{
   // 3/1S  jim 20190601
    LED_LOCK,
    LED_ON(LED_BLUE_WHITE),    LED_WAIT(200),
    LED_OFF(LED_BLUE_WHITE),   LED_WAIT(200),
    LED_UNLOCK,
    LED_END

};

const ledPattern app_led_pattern_Delete_TWS[] =
{
   // 3/1S  jim 20190601
    LED_LOCK,
    LED_ON(LED_BLUE_WHITE),    LED_WAIT(200),
    LED_OFF(LED_BLUE_WHITE),   LED_WAIT(200),
    LED_ON(LED_BLUE_WHITE),    LED_WAIT(200),
    LED_OFF(LED_BLUE_WHITE),   LED_WAIT(200),
    LED_UNLOCK,
    LED_END

};



const ledPattern app_led_pattern_power_on[] = 
{
    LED_LOCK,
    LED_ON(LED_BLUE),   LED_WAIT(1000),
    LED_OFF(LED_BLUE),  LED_WAIT(100),
    LED_UNLOCK,
    LED_END
};

const ledPattern app_led_pattern_power_off[] = 
{
    LED_LOCK,
    LED_ON(LED_WHITE), LED_WAIT(1000), LED_OFF(LED_WHITE),// LED_WAIT(200),
    LED_UNLOCK,
    LED_END
};

const ledPattern app_led_pattern_error[] = 
{
    LED_LOCK,
    LED_ON(LED_WHITE), LED_WAIT(100), LED_OFF(LED_WHITE), LED_WAIT(100),
    LED_REPEAT(1, 2),
    LED_UNLOCK,
    LED_END
};



const ledPattern app_led_pattern_idle[] = 
{
/*
        LED_SYNC(5000),
    LED_LOCK,
    LED_ON(LED_WHITE), LED_WAIT(100), LED_OFF(LED_BLUE),
    LED_UNLOCK,
    LED_REPEAT(0, 0),
*/
        LED_LOCK,
    LED_ON(LED_WHITE),   LED_WAIT(200),
    LED_OFF(LED_WHITE),  LED_WAIT(100),
    LED_UNLOCK,
    LED_END
};

const ledPattern app_led_pattern_idle_connected[] = 
{
    LED_SYNC(1000),
    LED_LOCK,
    LED_ON(LED_BLUE), LED_WAIT(100), LED_OFF(LED_BLUE),
    LED_UNLOCK,
    LED_REPEAT(0, 0),
};

const ledPattern app_led_pattern_pairing[] = 
{
    LED_LOCK,
    LED_ON(LED_BLUE), LED_WAIT(300), LED_OFF(LED_BLUE), LED_WAIT(300),// jim
    LED_ON(LED_WHITE), LED_WAIT(300), LED_OFF(LED_WHITE), LED_WAIT(300),// jim
    LED_UNLOCK,
    LED_REPEAT(0, 0)//LED_REPEAT(1, 5)
};

const ledPattern app_led_pattern_pairing_deleted[] = 
{
    LED_LOCK,
    LED_ON(LED_BLUE), LED_WAIT(500), LED_OFF(LED_BLUE), LED_WAIT(500),
    LED_ON(LED_WHITE), LED_WAIT(500), LED_OFF(LED_WHITE), LED_WAIT(500),
    LED_UNLOCK,
    LED_END
};

const ledPattern app_led_pattern_peer_pairing[] =
{
    LED_LOCK,
    LED_ON(LED_BLUE), LED_WAIT(100), LED_OFF(LED_BLUE), LED_WAIT(100),// jim
    LED_ON(LED_WHITE), LED_WAIT(100), LED_OFF(LED_WHITE), LED_WAIT(100),// jim
    LED_UNLOCK,
    LED_REPEAT(0, 0)
};

#ifdef INCLUDE_DFU
const ledPattern app_led_pattern_dfu[] = 
{
    LED_LOCK,
    LED_ON(LED_WHITE), LED_WAIT(100), LED_OFF(LED_WHITE), LED_WAIT(100),
    LED_REPEAT(1, 2),
    LED_WAIT(400),
    LED_UNLOCK,
    LED_REPEAT(0, 0)
};
#endif

#ifdef INCLUDE_AV
const ledPattern app_led_pattern_streaming[] =
{
    LED_SYNC(2000),
    LED_LOCK,
    LED_ON(LED_BLUE), LED_WAIT(50), LED_OFF(LED_BLUE), LED_WAIT(50),
    LED_REPEAT(2, 2),
    LED_WAIT(500),
    LED_UNLOCK,
    LED_REPEAT(0, 0),
};
#endif

#ifdef INCLUDE_AV
const ledPattern app_led_pattern_streaming_aptx[] =
{
    LED_SYNC(2000),
    LED_LOCK,
    LED_ON(LED_BLUE), LED_WAIT(50), LED_OFF(LED_BLUE), LED_WAIT(50),
    LED_REPEAT(2, 2),
    LED_WAIT(500),
    LED_UNLOCK,
    LED_REPEAT(0, 0),
};
#endif

const ledPattern app_led_pattern_sco[] = 
{
/*
    LED_SYNC(2000),
    LED_LOCK,
    LED_ON(LED_BLUE), LED_WAIT(50), LED_OFF(LED_BLUE), LED_WAIT(50),
    LED_REPEAT(2, 1),
    LED_WAIT(500),
    LED_UNLOCK,
    LED_REPEAT(0, 0),
*/
	LED_SYNC(5000),
    LED_LOCK,
    LED_ON(LED_BLUE), LED_WAIT(100), LED_OFF(LED_BLUE),
    LED_UNLOCK,
    LED_REPEAT(0, 0),
};

#ifdef SOUL_BLADE
const ledPattern app_led_pattern_calling[] = 
{
    LED_LOCK,
    LED_SYNC(1000),
    LED_ON(LED_BLUE), LED_WAIT(100), LED_OFF(LED_BLUE), LED_WAIT(4900),
    LED_REPEAT(2, 5),
    LED_UNLOCK,
    LED_REPEAT(0, 0),
};

const ledPattern app_led_off[] = 
{
    LED_LOCK,
    LED_OFF(LED_BLUE), LED_WAIT(1000),
    LED_UNLOCK,
    LED_REPEAT(0, 0),

};    

const ledPattern app_led_pattern_charging[] = 
{
    LED_LOCK,
    LED_ON(LED_WHITE), LED_WAIT(1000),
    LED_UNLOCK,
    LED_REPEAT(0, 0),
};
#endif

const ledPattern app_led_pattern_call_incoming[] = 
{
/*
    LED_LOCK,
    LED_SYNC(1000),
    LED_ON(LED_BLUE), LED_WAIT(100), LED_OFF(LED_BLUE), LED_WAIT(900),
    LED_REPEAT(2, 5),
    LED_UNLOCK,
    LED_REPEAT(0, 0),
    
    LED_LOCK,
    LED_SYNC(1000),
    LED_ON(LED_BLUE), LED_WAIT(100), LED_OFF(LED_BLUE), LED_WAIT(4900),
    LED_REPEAT(2, 5),
    LED_UNLOCK,
    LED_REPEAT(0, 0),*/

    LED_SYNC(5000),
    LED_LOCK,
    LED_ON(LED_BLUE), LED_WAIT(100), LED_OFF(LED_BLUE),
    LED_UNLOCK,
    LED_REPEAT(0, 0),
};

const ledPattern app_led_pattern_battery_empty[] = 
{
    /*LED_LOCK,
    LED_ON(LED_WHITE),
    LED_REPEAT(1, 2),
    LED_UNLOCK,
    LED_END
    */

        LED_LOCK,
    LED_ON(LED_WHITE), LED_WAIT(500), LED_OFF(LED_WHITE),
    LED_UNLOCK,
    LED_END
};
/*! \endcond led_patterns_well_named
 */


/*! \cond constant_well_named_tones 
    No Need to document these tones. Their access through functions such as
    appUiIdleActive() is the public interface.
 */
 
const ringtone_note app_tone_button[] =
{
    RINGTONE_TIMBRE(sine), RINGTONE_DECAY(16),
    RINGTONE_NOTE(D7, SEMIQUAVER),
    RINGTONE_STOP
};

const ringtone_note app_tone_button_2[] =
{
    RINGTONE_TIMBRE(sine), RINGTONE_DECAY(16),
    RINGTONE_NOTE(D7, SEMIQUAVER),
    RINGTONE_NOTE(D7, SEMIQUAVER),
    RINGTONE_STOP
};

const ringtone_note app_tone_button_3[] =
{
    RINGTONE_TIMBRE(sine), RINGTONE_DECAY(16),
    RINGTONE_NOTE(D7, SEMIQUAVER),
    RINGTONE_NOTE(D7, SEMIQUAVER),
    RINGTONE_NOTE(D7, SEMIQUAVER),
    RINGTONE_STOP
};

const ringtone_note app_tone_button_4[] =
{
    RINGTONE_TIMBRE(sine), RINGTONE_DECAY(16),
    RINGTONE_NOTE(D7, SEMIQUAVER),
    RINGTONE_NOTE(D7, SEMIQUAVER),
    RINGTONE_NOTE(D7, SEMIQUAVER),
    RINGTONE_NOTE(D7, SEMIQUAVER),
    RINGTONE_STOP
};

#ifdef INCLUDE_DFU
const ringtone_note app_tone_button_dfu[] =
{
    RINGTONE_TIMBRE(sine), RINGTONE_DECAY(16),
    RINGTONE_NOTE(D7, SEMIQUAVER),
    RINGTONE_NOTE(A7, SEMIQUAVER),
    RINGTONE_NOTE(D7, SEMIQUAVER),
    RINGTONE_STOP
};
#endif

const ringtone_note app_tone_button_factory_reset[] =
{
    RINGTONE_TIMBRE(sine), RINGTONE_DECAY(16),
    RINGTONE_NOTE(D7, SEMIQUAVER),
    RINGTONE_NOTE(A7, SEMIQUAVER),
    RINGTONE_NOTE(D7, SEMIQUAVER),
    RINGTONE_NOTE(C7, SEMIQUAVER),
    RINGTONE_NOTE(B7, SEMIQUAVER),
    RINGTONE_STOP
};

#ifdef INCLUDE_AV
const ringtone_note app_tone_av_connect[] =
{
    RINGTONE_TIMBRE(sine), RINGTONE_DECAY(16),
    RINGTONE_NOTE(D7, SEMIQUAVER),
    RINGTONE_STOP
};

const ringtone_note app_tone_av_disconnect[] =
{
    RINGTONE_TIMBRE(sine), RINGTONE_DECAY(16),
    RINGTONE_NOTE(D7, SEMIQUAVER),
    RINGTONE_STOP
};

const ringtone_note app_tone_av_remote_control[] =
{
    RINGTONE_TIMBRE(sine), RINGTONE_DECAY(16),
    RINGTONE_NOTE(D7, SEMIQUAVER),
    RINGTONE_STOP
};

const ringtone_note app_tone_av_connected[] =
{
    RINGTONE_TIMBRE(sine), RINGTONE_DECAY(16),
    RINGTONE_NOTE(D6,  SEMIQUAVER),
    RINGTONE_NOTE(A6,  SEMIQUAVER),
    RINGTONE_STOP
};

const ringtone_note app_tone_av_disconnected[] =
{
    RINGTONE_TIMBRE(sine), RINGTONE_DECAY(16),
    RINGTONE_NOTE(A6,  SEMIQUAVER),
    RINGTONE_NOTE(D6,  SEMIQUAVER),
    RINGTONE_STOP
};

const ringtone_note app_tone_av_link_loss[] =
{
    RINGTONE_TIMBRE(sine), RINGTONE_DECAY(16),
    RINGTONE_NOTE(A5,  SEMIQUAVER),
    RINGTONE_NOTE(D5,  SEMIQUAVER),
    RINGTONE_NOTE(D5,  SEMIQUAVER),
    RINGTONE_STOP
};
#endif

const ringtone_note app_tone_hfp_connect[] =
{
    RINGTONE_TIMBRE(sine), RINGTONE_DECAY(16),
    RINGTONE_NOTE(D7, SEMIQUAVER),
    RINGTONE_STOP
};

const ringtone_note app_tone_hfp_connected[] =
{
    RINGTONE_TIMBRE(sine), RINGTONE_DECAY(16),
    RINGTONE_NOTE(D6,  SEMIQUAVER),
    RINGTONE_NOTE(A6,  SEMIQUAVER),
    RINGTONE_STOP
};

const ringtone_note app_tone_hfp_disconnected[] =
{
    RINGTONE_TIMBRE(sine), RINGTONE_DECAY(16),
    RINGTONE_NOTE(A6,  SEMIQUAVER),
    RINGTONE_NOTE(D6,  SEMIQUAVER),
    RINGTONE_STOP
};

const ringtone_note app_tone_hfp_link_loss[] =
{
    RINGTONE_TIMBRE(sine), RINGTONE_DECAY(16),
    RINGTONE_NOTE(A5,  SEMIQUAVER),
    RINGTONE_NOTE(D5,  SEMIQUAVER),
    RINGTONE_NOTE(D5,  SEMIQUAVER),
    RINGTONE_STOP
};
        
const ringtone_note app_tone_hfp_sco_connected[] =
{
    RINGTONE_TIMBRE(sine), RINGTONE_DECAY(16),
    RINGTONE_NOTE(AS5, DEMISEMIQUAVER),
    RINGTONE_NOTE(DS6, DEMISEMIQUAVER),
    RINGTONE_STOP
};

const ringtone_note app_tone_hfp_sco_disconnected[] =
{
    RINGTONE_TIMBRE(sine), RINGTONE_DECAY(16),
    RINGTONE_NOTE(DS6, DEMISEMIQUAVER),
    RINGTONE_NOTE(AS5, DEMISEMIQUAVER),
    RINGTONE_STOP
};

const ringtone_note app_tone_hfp_mute_reminder[] =
{
    RINGTONE_TIMBRE(sine), RINGTONE_DECAY(16),
    RINGTONE_NOTE(D5,  SEMIQUAVER),
    RINGTONE_NOTE(A5,  SEMIQUAVER),
    RINGTONE_STOP
};

const ringtone_note app_tone_hfp_sco_unencrypted_reminder[] =
{
    RINGTONE_TIMBRE(sine), RINGTONE_DECAY(16),
    RINGTONE_NOTE(B5, SEMIQUAVER),
    RINGTONE_NOTE(B5, SEMIQUAVER),
    RINGTONE_NOTE(B5, SEMIQUAVER),
    RINGTONE_STOP
};

const ringtone_note app_tone_hfp_ring[] =
{
    RINGTONE_TIMBRE(sine), RINGTONE_DECAY(16),
    RINGTONE_NOTE(B6,   SEMIQUAVER),
    RINGTONE_NOTE(G6,   SEMIQUAVER),
    RINGTONE_NOTE(D7,   SEMIQUAVER),
    RINGTONE_NOTE(REST, SEMIQUAVER),
    RINGTONE_NOTE(B6,   SEMIQUAVER),
    RINGTONE_NOTE(G6,   SEMIQUAVER),
    RINGTONE_NOTE(D7,   SEMIQUAVER),
    RINGTONE_STOP
};

const ringtone_note app_tone_hfp_ring_caller_id[] =
{
    RINGTONE_TIMBRE(sine), RINGTONE_DECAY(16),
    RINGTONE_NOTE(B6,   SEMIQUAVER),
    RINGTONE_NOTE(G6,   SEMIQUAVER),
    RINGTONE_STOP
};

const ringtone_note app_tone_hfp_voice_dial[] =
{
    RINGTONE_TIMBRE(sine), RINGTONE_DECAY(16),
    RINGTONE_NOTE(D7, SEMIQUAVER),
    RINGTONE_STOP
};

const ringtone_note app_tone_hfp_voice_dial_disable[] =
{
    RINGTONE_TIMBRE(sine), RINGTONE_DECAY(16),
    RINGTONE_NOTE(D7, SEMIQUAVER),
    RINGTONE_STOP
};

const ringtone_note app_tone_hfp_answer[] =
{
    RINGTONE_TIMBRE(sine), RINGTONE_DECAY(16),
    RINGTONE_NOTE(D7, SEMIQUAVER),
    RINGTONE_STOP
};

const ringtone_note app_tone_hfp_hangup[] =
{
    RINGTONE_TIMBRE(sine), RINGTONE_DECAY(16),
    RINGTONE_NOTE(D7, SEMIQUAVER),
    RINGTONE_STOP
};

const ringtone_note app_tone_hfp_mute_active[] =
{
    RINGTONE_TIMBRE(sine), RINGTONE_DECAY(16),
    RINGTONE_NOTE(CS7, SEMIQUAVER),
    RINGTONE_NOTE(DS7, SEMIQUAVER),
    RINGTONE_STOP
};

const ringtone_note app_tone_hfp_mute_inactive[] =
{
    RINGTONE_TIMBRE(sine), RINGTONE_DECAY(16),
    RINGTONE_NOTE(DS7, SEMIQUAVER),
    RINGTONE_NOTE(CS7, SEMIQUAVER),
    RINGTONE_STOP
};

const ringtone_note app_tone_hfp_talk_long_press[] =
{
    RINGTONE_TIMBRE(sine), RINGTONE_DECAY(16),
    RINGTONE_NOTE(D7, SEMIQUAVER),
    RINGTONE_STOP
};

const ringtone_note app_tone_pairing[] =
{
    RINGTONE_TIMBRE(sine), RINGTONE_DECAY(16),
    RINGTONE_NOTE(D7, SEMIQUAVER),
    RINGTONE_STOP
};

const ringtone_note app_tone_paired[] =
{
    RINGTONE_TIMBRE(sine), RINGTONE_DECAY(16),
    RINGTONE_NOTE(A6, SEMIQUAVER),
    RINGTONE_NOTE(D7, SEMIQUAVER),
    RINGTONE_STOP
};

const ringtone_note app_tone_pairing_deleted[] =
{
    RINGTONE_TIMBRE(sine), RINGTONE_DECAY(16),
    RINGTONE_NOTE(D7, SEMIQUAVER),
    RINGTONE_NOTE(A6, SEMIQUAVER),
    RINGTONE_STOP
};

const ringtone_note app_tone_volume[] =
{
    RINGTONE_TIMBRE(sine), RINGTONE_DECAY(16),
    RINGTONE_NOTE(D7, SEMIQUAVER),
    RINGTONE_STOP
};

const ringtone_note app_tone_volume_limit[] =
{
    RINGTONE_TIMBRE(sine), RINGTONE_DECAY(16),
    RINGTONE_NOTE(D7, SEMIQUAVER),
    RINGTONE_NOTE(D7, SEMIQUAVER),
    RINGTONE_STOP
};

const ringtone_note app_tone_error[] =
{
    RINGTONE_TIMBRE(sine), RINGTONE_DECAY(16),
    RINGTONE_NOTE(B5, SEMIQUAVER),
    RINGTONE_NOTE(B5, SEMIQUAVER),
    RINGTONE_NOTE(B5, SEMIQUAVER),
    RINGTONE_STOP
};

const ringtone_note app_tone_battery_empty[] =
{
    RINGTONE_TIMBRE(sine), RINGTONE_DECAY(16),
    RINGTONE_NOTE(B6, SEMIQUAVER),
    RINGTONE_NOTE(B6, SEMIQUAVER),
    RINGTONE_STOP
};

const ringtone_note app_tone_power_on[] =
{
    RINGTONE_TIMBRE(sine), RINGTONE_DECAY(16),
    RINGTONE_NOTE(CS5, SEMIQUAVER),
    RINGTONE_NOTE(D5,  SEMIQUAVER),
    RINGTONE_NOTE(A5,  SEMIQUAVER),
    RINGTONE_STOP
};

const ringtone_note app_tone_power_off[] =
{
    RINGTONE_TIMBRE(sine), RINGTONE_DECAY(16),
    RINGTONE_NOTE(A5,  SEMIQUAVER),
    RINGTONE_NOTE(D5,  SEMIQUAVER),
    RINGTONE_NOTE(CS5, SEMIQUAVER),
    RINGTONE_STOP
};

const ringtone_note app_tone_paging_reminder[] =
{
    RINGTONE_TIMBRE(sine), RINGTONE_DECAY(16),
    RINGTONE_NOTE(A5,  SEMIQUAVER),
    RINGTONE_NOTE(A5,  SEMIQUAVER),
    RINGTONE_STOP
};

const ringtone_note app_tone_peer_pairing[] =
{
    RINGTONE_TIMBRE(sine), RINGTONE_DECAY(16),
    RINGTONE_NOTE(D7, SEMIQUAVER),
    RINGTONE_STOP
};

const ringtone_note app_tone_peer_pairing_error[] =
{
    RINGTONE_TIMBRE(sine), RINGTONE_DECAY(16),
    RINGTONE_NOTE(B5, SEMIQUAVER),
    RINGTONE_NOTE(B5, SEMIQUAVER),
    RINGTONE_NOTE(B5, SEMIQUAVER),
    RINGTONE_STOP
};

const ringtone_note app_tone_peer_pairing_reminder[] =
{
    RINGTONE_TIMBRE(sine), RINGTONE_DECAY(16),
    RINGTONE_NOTE(A5,  SEMIQUAVER),
    RINGTONE_NOTE(A5,  SEMIQUAVER),
    RINGTONE_STOP
};

#ifdef INCLUDE_DFU
const ringtone_note app_tone_dfu[] =
{
    RINGTONE_TIMBRE(sine), RINGTONE_DECAY(16),
    RINGTONE_NOTE(CS5, SEMIQUAVER),
    RINGTONE_NOTE(D5,  SEMIQUAVER),
    RINGTONE_NOTE(D5,  SEMIQUAVER),
    RINGTONE_STOP
};
#endif

/*! \endcond constant_well_named_tones */

/*! \brief Play tone.
    \param tone The tone to play.
    \param interruptible If TRUE, always play to completion, if FALSE, the tone may be
    interrupted before completion.
    \param queueable If TRUE, tone can be queued behind already playing tone, if FALSE, the tone will
    only play if no other tone playing or queued.
    \param client_lock If not NULL, bits set in client_lock_mask will be cleared
    in client_lock when the tone finishes - either on completion, when interrupted,
    or if the tone is not played at all, because the UI is not currently playing tones.
    \param client_lock_mask A mask of bits to clear in the client_lock.
*/
void appUiPlayToneCore(const ringtone_note *tone, bool interruptible, bool queueable,
                       uint16 *client_lock, uint16 client_lock_mask)
{
#ifndef INCLUDE_TONES
    UNUSED(tone);
    UNUSED(interruptible);
    UNUSED(queueable);
#else
    /* Only play tone if it can be heard */
    if (PHY_STATE_IN_EAR == appPhyStateGetState())
    {
        if (queueable || !appKymeraIsTonePlaying())
            appKymeraTonePlay(tone, interruptible, client_lock, client_lock_mask);
    }
    else
#endif
    {
        if (client_lock)
        {
            *client_lock &= ~client_lock_mask;
        }
    }
}

/*! \brief Play prompt.
    \param prompt The prompt to play.
    \param interruptible If TRUE, always play to completion, if FALSE, the prompt may be
    interrupted before completion.
    \param client_lock If not NULL, bits set in client_lock_mask will be cleared
    in client_lock when the prompt finishes - either on completion, when interrupted,
    or if the prompt is not played at all, because the UI is not currently playing prompts.
    \param client_lock_mask A mask of bits to clear in the client_lock.
*/
void appUiPlayPromptCore(voicePromptName prompt, bool interruptible, bool queueable,
                         uint16 *client_lock, uint16 client_lock_mask)
{
#ifndef INCLUDE_PROMPTS
    UNUSED(prompt);
    UNUSED(interruptible);
    UNUSED(queueable);
#else
    uiTaskData *theUi = appGetUi();
    PanicFalse(prompt < NUMBER_OF_PROMPTS);
    /* Only play prompt if it can be heard */
    if ((PHY_STATE_IN_EAR == appPhyStateGetState()) && (prompt != theUi->prompt_last) &&
        (queueable || !appKymeraIsTonePlaying()))
    {
        const promptConfig *config = appConfigGetPromptConfig(prompt);
        FILE_INDEX *index = theUi->prompt_file_indexes + prompt;
        if (*index == FILE_NONE)
        {
            const char* name = config->filename;
            *index = FileFind(FILE_ROOT, name, strlen(name));
            /* Prompt not found */
            PanicFalse(*index != FILE_NONE);
        }
        appKymeraPromptPlay(*index, config->format, config->rate,
                            interruptible, client_lock, client_lock_mask);

        if (appConfigPromptNoRepeatDelay())
        {
            MessageCancelFirst(&theUi->task, UI_INTERNAL_CLEAR_LAST_PROMPT);
            MessageSendLater(&theUi->task, UI_INTERNAL_CLEAR_LAST_PROMPT, NULL,
                             appConfigPromptNoRepeatDelay());
            theUi->prompt_last = prompt;
        }
    }
    else
#endif
    {
        if (client_lock)
        {
            *client_lock &= ~client_lock_mask;
        }
    }
}

/*! \brief Report a generic error on LEDs and play tone */
void appUiError(void)
{
    //appUiPlayTone(app_tone_error);
    //appLedSetPattern(app_led_pattern_error, LED_PRI_EVENT);
}

/*! \brief Play HFP error tone and set LED error pattern.
    \param silent If TRUE the error is not presented on the UI.
*/
void appUiHfpError(bool silent)
{
    if (!silent)
    {
        //appUiPlayTone(app_tone_error);
        //appLedSetPattern(app_led_pattern_error, LED_PRI_EVENT);
    }
}

/*! \brief Play AV error tone and set LED error pattern.
    \param silent If TRUE the error is not presented on the UI.
*/
void appUiAvError(bool silent)
{
    if (!silent)
    {
        //appUiPlayTone(app_tone_error);
        //appLedSetPattern(app_led_pattern_error, LED_PRI_EVENT);
    }
}

/*! \brief Play power on prompt and LED pattern */
void appUiPowerOn(void)
{
    /* Enable LEDs */
    appLedEnable(TRUE);
    
    appLedSetPattern(app_led_pattern_power_on, LED_PRI_EVENT);
    appUiPlayPrompt(PROMPT_POWER_ON);
}

/*! \brief Play power off prompt and LED pattern.
    \param lock The caller's lock, may be NULL.
    \param lock_mask Set bits in lock_mask will be cleared in lock when the UI completes.
 */
void appUiPowerOff(uint16 *lock, uint16 lock_mask)
{
    appLedSetPattern(app_led_pattern_power_off, LED_PRI_EVENT);
    appUiPlayPromptClearLock(PROMPT_POWER_OFF, lock, lock_mask);

    /* Disable LEDs */
    appLedEnable(FALSE);
}

/*! \brief Prepare UI for sleep.
    \note If in future, this function is modified to play a tone, it should
    be modified to resemble #appUiPowerOff, so the caller's lock is cleared when
    the tone is completed. */
void appUiSleep(void)
{
    appLedSetPattern(app_led_pattern_power_off, LED_PRI_EVENT);
    appLedEnable(FALSE);
}

/*! \brief Message Handler

    This function is the main message handler for the UI module, all user button
    presses are handled by this function.

    NOTE - only a single button config is currently defined for both earbuds.
    The following defines could be used to split config amongst the buttons on
    two earbuds.

        APP_RIGHT_CONFIG
        APP_SINGLE_CONFIG
        APP_LEFT_CONFIG
*/
extern uint8 percent_t;	//add by kimny / jim

uint8 click_count = 0;
uint8 battery_critical_count = 0;

bool poweron_hold_enterpairing_flag;
bool poweron_flag = 0;
bool PowerOff_Flag;
static void appUiHandleMessage(Task task, MessageId id, Message message)
{
    uiTaskData *theUi = (uiTaskData *)task;
    UNUSED(message);
	if (appSmIsOutOfCase())
    {
		if(!PsuGetVregEn())
		{
			//if(poweron_hold_enterpairing_flag)poweron_hold_enterpairing_flag=0;
		}
	}

    switch (id)
    {
        case UI_INTERNAL_CLEAR_LAST_PROMPT:
            theUi->prompt_last = PROMPT_NONE;
        break;
        /* ANC ON and OFF based on current ANC status */
        case APP_MFB_BUTTON_DOUBLE://APP_MFB_BUTTON_ANC_TOGGLE_ON_OFF:
       	DEBUG_LOGF("APP_MFB_BUTTON_ANC_TOGGLE_ON_OFF:");
		click_count = 0;
        if (appSmIsOutOfCase())
        {
#ifdef SOUL_BLADE            
            DEBUG_LOGF("PHY_STATE_OUT_OF_EAR:%d, appSmIsInEar:%d", appSmIsOutOfCase(),appSmIsInEar() );    
			if (appHfpIsCallIncoming())
			{
				DEBUG_LOGF("appHfpCallReject \n\n ");
				appHfpCallReject();
			}
			else if (appScoFwdIsCallIncoming())
			{
				DEBUG_LOGF("appHfpCallReject \n\n ");
				appScoFwdCallReject();
			}
			/* If voice call active, hangup */
			else if (appHfpIsCallActive())
			{
				//appUiHfpHangup();
				//appHfpCallHangup();
                //appHfpTransferToHeadset();//
                if (appHfpIsCall())
                {
                    if (appHfpIsScoActive())
                            appHfpTransferToAg();
                        else
                            appHfpTransferToHeadset();
                    }
			}
			/* Sco Forward can be streaming a ring tone */
			else if (appScoFwdIsReceiving() && !appScoFwdIsCallIncoming())
			{
				//appUiHfpHangup();
				//appScoFwdCallHangup();
                //appHfpTransferToAg();//
                if (appHfpIsCall())
                {
                    if (appHfpIsScoActive())
                           appHfpTransferToAg();
                        else
                            appHfpTransferToHeadset();
                    }
			}
            else if ((appAvPlayStatus()==avrcp_play_status_playing)
                 ||(appAvPlayStatus()==avrcp_play_status_stopped)
                    || (appAvPlayStatus()== avrcp_play_status_paused))
            {
                DEBUG_LOGF("appAvForward \n\n");                        
                appAvForward();
            }
#endif
        }
        break;

        case APP_MFB_BUTTON_TRIPLE:
        click_count = 0;
		DEBUG_LOGF("APP_MFB_BUTTON_TRIPLE:");
		if (appSmIsOutOfCase())
		{
			appHfpVoiceDial();                        
		}
        break;

		case APP_MFB_PANIC:
			appPowerReboot();
		break;

        case APP_MFB_BUTTON_FOURTH:
        click_count = 0;
		DEBUG_LOGF("APP_MFB_BUTTON_FOURTH:");
		if(appChargerIsConnected())//(appSmIsInCase())
		{
			DEBUG_LOG("ConnectionEnterDutMode");
			DUT_PsKey_WriteTure();
			MessageSendLater(appGetUiTask(), APP_MFB_PANIC, NULL, 1000);
		}
        break;

		case APP_MFB_BUTTON_PRESS:
		click_count++;
        DEBUG_LOGF("=================================click_count = %d", click_count);
		if(click_count >7)click_count=1;
		if(click_count==6)
		{
			click_count = 0;
			MessageSendLater(appGetUiTask(), APP_MFB_BUTTON_15_SECOND, NULL, 300);
			
		}
		else if(click_count==5)
		{
			MessageCancelAll(appGetUiTask(), APP_MFB_BUTTON_FOURTH);
			MessageSendLater(appGetUiTask(), APP_MFB_BUTTON_FOURTH, NULL, 300);
		}
		else if(click_count==4)
		{
			MessageCancelAll(appGetUiTask(), APP_MFB_BUTTON_TRIPLE);
            MessageSendLater(appGetUiTask(), APP_MFB_BUTTON_FOURTH, NULL, 300);
		}
		else if(click_count==3)
		{
			MessageCancelAll(appGetUiTask(), APP_MFB_BUTTON_DOUBLE);
            MessageSendLater(appGetUiTask(), APP_MFB_BUTTON_TRIPLE, NULL, 300);
		}
		else if(click_count==2)
		{
			MessageCancelAll(appGetUiTask(), APP_MFB_BUTTON_Slick);
            MessageSendLater(appGetUiTask(), APP_MFB_BUTTON_DOUBLE, NULL, 300);
		}
		else if(click_count==1)
		{
            MessageSendLater(appGetUiTask(), APP_MFB_BUTTON_Slick, NULL, 300);
			if(appChargerIsConnected())//if(appSmIsInCase())
            {
                DEBUG_LOG("==============================My_FactoryReset_Flag=1");
                //My_FactoryReset_Flag=1;
                //MessageSendLater(appGetUiTask(), APP_EVENT_FactoryReset_Flag, NULL, 2000);
            }
		}
        //MFB_BUTTON_H_FLAG = 1;
		break;

        /* HFP call/reject & A2DP play/pause */
		case APP_MFB_BUTTON_Slick:
        {
            click_count = 0;
            DEBUG_LOGF("APP_MFB_BUTTON_PRESS");
            if (appSmIsOutOfCase())
            {
                /* If voice call active, hangup */
                if (appHfpIsCallActive())
                    appHfpCallHangup();
                    /* Sco Forward can be streaming a ring tone */
                else if (appScoFwdIsReceiving() && !appScoFwdIsCallIncoming())
                    appScoFwdCallHangup();
                /* If outgoing voice call, hangup */
                else if (appHfpIsCallOutgoing())
                    appHfpCallHangup();
                /* If incoming voice call, accept */
                else if (appHfpIsCallIncoming())
                    appHfpCallAccept();
                else if (appScoFwdIsCallIncoming())
                    appScoFwdCallAccept();
                /* If AVRCP to handset connected, send play or pause */
                else if (appDeviceIsHandsetAvrcpConnected())
                    appAvPlayToggle(TRUE);
                /* If AVRCP is peer is connected and peer is connected to handset, send play or pause */
                else if (appDeviceIsPeerAvrcpConnectedForAv() && appPeerSyncIsComplete() && appPeerSyncIsPeerHandsetAvrcpConnected())
                    appAvPlayToggle(TRUE);
                else
                {
					//if(Link_lost_Flag==1)
					{
						appSmConnectHandset();
                    	appUiAvConnect();
					}
                }
            }
        }
        break;

        case APP_MFB_BUTTON_1_SECOND:
        {
            DEBUG_LOGF("APP_MFB_BUTTON_1_SECOND");
            if (appSmIsOutOfCase())
            {
                if (appHfpIsCallActive())  /* Mic Mute not handled for SCO Fwding */
                {
                    if (appHfpIsScoActive())
                        appHfpTransferToAg();
                    else
                        appHfpTransferToHeadset();
                }/*
                else if (appHfpIsCallIncoming())
                    appHfpCallReject();
                else if (appScoFwdIsCallIncoming())
                    appScoFwdCallReject();
                //else if (appAvHasAConnection())appAvStop(TRUE); */
                else
                {
					#ifdef LONG_PRESS_POWERON
					{
						appState state=appGetState();
						appSmCancelLimboTimeout();
						if (state == APP_STATE_DFU_CHECK)
						{
							appPowerOn();
							appSetState(APP_STATE_STARTUP);
							//is_poweringoff_flag=0; // add at 20190829
							//Power_flag = 1;
							 poweron_hold_enterpairing_flag = 1;
                             poweron_flag=1;
							//check_poweroff_count = 0;
							my_PsKey_WriteTure();
							//MessageCancelAll(appGetUiTask(),APP_CHECK_POWEROFF);
							//MessageSendLater(appGetUiTask(), APP_CHECK_POWEROFF, 0,10000);
						}
						else if (appGetState() >= APP_STATE_STARTUP)
						{
                            //Power_flag = 0;
							#ifdef INCLUDE_AEC_LEAKTHROUGH
                            if(appDeviceIsHandsetA2dpConnected()||appDeviceIsHandsetHfpConnected()||appPeerSyncIsPeerHandsetA2dpConnected() || appPeerSyncIsPeerHandsetHfpConnected())
							{
								DEBUG_LOG("APP_MFB_BUTTON_ANC_AEC_TOGGLE_ON_OFF:");
								
				                if(appKymeraIsLeakthroughEnabled())
				                {
				                    DEBUG_LOG("AEC Leakthrough OFF EVENT");
									appUiButton();
				                    appKymeraDisableLeakThrough();
				                }
				                else
				                {
				                   DEBUG_LOG("AEC Leakthrough ON EVENT");
								   appUiButton2();
				                   appKymeraEnableLeakThrough();
				                }
							}
                            #endif
						}
					}
					#endif

                }
            }
        }
        break;

		case APP_MFB_BUTTON_3_SECOND:
		DEBUG_LOGF("APP_MFB_BUTTON_3_SECOND");
		break;
		
		case APP_MFB_BUTTON_1_SECOND_REL:
			DEBUG_LOGF("APP_MFB_BUTTON_1_SECOND_REL");
		case APP_MFB_BUTTON_3_SECOND_REL:
			DEBUG_LOGF("APP_MFB_BUTTON_3_SECOND_REL");
            if(poweron_flag == 1)
			{
				sinkStartAutoReconectHeadsetTimer();// ADD BY JIM 20190723
				//sinkStartAutoEnterPairingTimer();
			}
            poweron_flag =0;
            poweron_hold_enterpairing_flag =0;
		break;

        /* Handset Pairing */
		case APP_MFB_Pairing:
        DEBUG_LOGF("APP_MFB_Pairing");
        if (appSmIsOutOfCase())//bt20191030
        {
            if(appPeerSyncIsPeerHandsetA2dpConnected()||appPeerSyncIsPeerHandsetHfpConnected()||appPeerSyncIsPeerPairing())
            {
				MessageCancelAll(appGetUiTask(), APP_MFB_Pairing);
				 break;
			}
			if (!appSmIsPairing())    
            {
                if(appSmStateIsIdle(appGetState()))
                {
                    MessageCancelAll(appGetUiTask(), APP_MFB_Pairing);
					bdaddr bd_addr;
			        if(appDeviceGetHandsetBdAddr(&bd_addr))
			        {
			            appConManagerSendCloseAclRequest(&bd_addr, TRUE);
			        }
                    appSmPairHandset();
                }
                else
                {
                    MessageSendLater(appGetUiTask(),APP_MFB_Pairing, NULL, 200);
                }
            }
        }
        break;
			
		case APP_MFB_BUTTON_5_SECOND:
		DEBUG_LOGF("APP_MFB_BUTTON_5_SECOND");
		if (appSmIsOutOfCase())//bt20191030
		{
            if(poweron_flag == 1)
			{
				if (!appSmIsPairing())
				{
					/*
					if(appPeerSyncIsPeerHandsetA2dpConnected() || appPeerSyncIsPeerHandsetHfpConnected())
					{
					
					}
					else*/
					{
						if(appSmStateIsIdle(appGetState()))
						{
							bdaddr bd_addr;
							if(appDeviceGetHandsetBdAddr(&bd_addr))
							{
								appConManagerSendCloseAclRequest(&bd_addr, TRUE);
							}
							appSmPairHandset();
						}
						else
						{
							MessageSendLater(appGetUiTask(),APP_MFB_Pairing, NULL, 200);
						}
					}
				}
			}
		}
		break;

		case APP_MFB_BUTTON_5_SECOND_REL:
        poweron_hold_enterpairing_flag =0;
        poweron_flag=0;
		break;

		
		case UI_DoPowerOff:
			PowerOff_Flag=0;
			my_appPowerDoPowerOff();
		break;

        case OTA_Sync_POWER_OFF:
        if(appChargerIsConnected()==CHARGER_DISCONNECTED)
        {
            Pre_ptekDisable();
            if (appAvPlayStatus() == avrcp_play_status_playing)appAvPause(TRUE);
            if(appSmIsPairing())appPairingHandsetPairCancel();
            my_PsKey_WriteFalse();
            PowerOff_Flag=1;
            appGetPower()->lock |= 0x01;
            appUiPowerOff(&appGetPower()->lock, 0x01);
            appLedEnable(TRUE);
            appLedSetPattern(app_led_pattern_power_off, LED_PRI_EVENT);
            appUiPlayPrompt(PROMPT_POWER_OFF);

            my_appPowerOffRequest();
            MessageSendLater(appGetUiTask(), UI_DoPowerOff, NULL, 3000);
        }
        break;

        case APP_MFB_BUTTON_8_SECOND:
			DEBUG_LOGF("APP_MFB_BUTTON_8_SECOND");

             DEBUG_LOGF("=================================poweron_flag = %d", poweron_flag);
            if((appChargerIsConnected()==CHARGER_DISCONNECTED)&&(poweron_flag == 0))
			{
            	//appPowerOffRequest();
            	DEBUG_LOGF("POWER OFF");
				if(appDeviceIsPeerConnected())
				My_appScoDucoData(SFWD_OTA_POWEROFF_TONE,1);
				else
				{
					MessageSendLater(appGetUiTask(), OTA_Sync_POWER_OFF, NULL, 100);	
				}
			}
            break;

        case APP_MFB_BUTTON_8_SECOND_REL:
            DEBUG_LOGF("APP_MFB_BUTTON_8_SECOND_REL");
            poweron_hold_enterpairing_flag =0;
            poweron_flag=0;
			#ifdef INCLUDE_BME300_CES 
            appUipowerOff();
			#endif
        break;
			
		case APP_MFB_BUTTON_10_SECOND:
			DEBUG_LOGF("APP_MFB_BUTTON_10_SECOND");
			/* Delete Handset Pairings */
			if(poweron_flag==1)//if(appChargerIsConnected())
			{
				appLedEnable(TRUE);
            	appUiDelete_Handsets();//jim 20190701
            	appSmDeleteHandsets();
			}
		break;
			
		case APP_MFB_BUTTON_10_SECOND_REL:
		DEBUG_LOGF("APP_MFB_BUTTON_10_SECOND_REL");
        poweron_hold_enterpairing_flag =0;
        poweron_flag=0;
		break;

        case APP_MFB_BUTTON_15_SECOND:
		DEBUG_LOGF("APP_MFB_BUTTON_15_SECOND");
		if(appChargerIsConnected())
        {
            //if(My_FactoryReset_Flag==1)
            {
                appChargerForceDisable();
                appUiChargerDisconnected();
                appLedEnable(TRUE);
                appUiDelete_TWS();//jim 20190701
                DEBUG_LOG("========appSmFactoryReset======================My_FactoryReset_Flag=1");
                //My_FactoryReset_Flag=0;
				#if 1
				bdaddr addr;
				if (appDeviceGetPeerBdAddr(&addr))
				{
					//appDeviceDelete(&addr);
				    DEBUG_LOGF("appDeviceDelete---------------------------------------------------------%2x :%2x :%x------PEER",addr.uap,addr.nap,addr.lap);
					ConnectionAuthSetPriorityDevice(&addr, FALSE);
					ConnectionSmDeleteAuthDevice(&addr);
				}
				while(appDeviceGetHandsetAddr(&addr))
				{
		       		//appDeviceDelete(&addr);
		     		DEBUG_LOGF("appDeviceDelete---------------------------------------------------------%2x :%2x :%x------headset",addr.uap,addr.nap,addr.lap);
					ConnectionAuthSetPriorityDevice(&addr, FALSE);
        			ConnectionSmDeleteAuthDevice(&addr);
				}			
				#endif
                 appSmFactoryReset();//appSmFactoryReset_later(3000);
            }
        }
        break;

        case APP_MFB_BUTTON_15_SECOND_REL:
			DEBUG_LOG("APP_MFB_BUTTON_15_SECOND_REL");
            poweron_hold_enterpairing_flag =0;
            poweron_flag=0;
        break;
		
		case APP_EVENT_RECONNECT_HEADSET:
            DEBUG_LOG("APP_EVENT_RECONNECT_HEADSET");
			if (appSmIsOutOfCase())
			{
				if(!appSmIsPairing())
				{
					if (appPeerSyncIsPeerHandsetA2dpConnected() || appPeerSyncIsPeerHandsetHfpConnected())
					{

					}
					else
					{
					/* Generate event that will run rules to connect to handset */
					appConnRulesSetEvent(appGetSmTask(), RULE_EVENT_USER_CONNECT); 
					}
				}
			}
        break;

        case APP_Battery_Critical:
            DEBUG_LOG("========APP_Battery_Critical");
            appUiBatteryCritical();
            MessageSendLater(appGetUiTask(), APP_Battery_Critical, NULL, D_SEC(5));
            battery_critical_count++;
            if(battery_critical_count>=120/5)
            {
                battery_critical_count =0;
                appUiBATTERY_LOW_16k();
            }
            break;

        case appExitInEarIdle_LED:
            //appLedEnable(TRUE);
            if(appDeviceIsHandsetA2dpConnected()||appDeviceIsHandsetHfpConnected()/*||appDeviceIsHandsetAvrcpConnected()*/)
            {
            	if(percent_t == 0)
            	appUiBatteryCritical();
            }
            MessageSendLater(appGetUiTask(), appExitInEarIdle_LED, NULL, D_SEC(5));
            break;

		case Ptek_IR_Det:
			PtekIR_Det();
			MessageSendLater(appGetUiTask(), Ptek_IR_Det, NULL, 500);
			 break;

        case Cancel_HR:
            Cancel_Advertising_HR();
         break;

        #ifdef SOUL_BLADE
        case APP_MFB_BUTTON_VDIAL_ON:
        {/*
            if (appHfpIsConnected())
            {

                appHfpVoiceDial();                        
            }
            else
            {
                ConnectionEnterDutMode();        
            }     */           
        }
        break;
        #endif
    }
}

/*! brief Initialise UI module */
void appUiInit(void)
{
    uiTaskData *theUi = appGetUi();
    
    /* Set up task handler */
    theUi->task.handler = appUiHandleMessage;
    
    /* Initialise input event manager with auto-generated tables for
     * the target platform */
    theUi->input_event_task = InputEventManagerInit(appGetUiTask(), InputEventActions,
                                                    sizeof(InputEventActions),
                                                    &InputEventConfig);

    memset(theUi->prompt_file_indexes, FILE_NONE, sizeof(theUi->prompt_file_indexes));

    theUi->prompt_last = PROMPT_NONE;
	my_PsKey_Ready();
}

void sinkStartAutoEnterPairingTimer(void) //add by kimny jim
{
    DEBUG_LOG("APO: Starting auto enter pairing timer \n");
    if(!appSmIsPairing() && !appDeviceIsHandsetConnected())
    {
        DEBUG_LOG("APO: not pairing state,  auto enter pairing timer go... \n");
        MessageCancelAll(appGetUiTask(), APP_MFB_Pairing);
        MessageSendLater(appGetUiTask(), APP_MFB_Pairing, NULL, D_SEC(30));	//
    }
}

void sinkStopAutoEnterPairingTimer(void) //add by kimny jim
{
    DEBUG_LOG("APO: Stopping auto enter pairing timer \n");
    MessageCancelAll(appGetUiTask(), APP_MFB_Pairing);
	//Link_lost_Flag = 0;
	PowerOnReconnectTimeoutFlag=0;
	My_appScoDucoData_OTA(SFWD_OTA_Peer_PairingHeadset,0);
	MessageSendLater(appGetUiTask(), Ptek_IR_Det, NULL, 100);
}


void sinkStartAutoReconectHeadsetTimer(void) //add by kimny jim
{
    DEBUG_LOG("APO: Starting auto ReconectHeadset timer \n");
	bdaddr bd_addr;
	if(appDeviceGetHandsetBdAddr(&bd_addr))
	{
	    if(!appSmIsPairing() && !appDeviceIsHandsetConnected())
	    {
	        DEBUG_LOG("APO: not pairing state,  auto enter ReconectHeadset timer go... \n");
	        //MessageCancelAll(appGetSmTask(), CONN_RULES_CONNECT_HANDSET);
	        MessageCancelAll(appGetUiTask(), APP_EVENT_RECONNECT_HEADSET);
	        DEBUG_LOG("appSmHandleConnRulesConnectHandset profiles HFP & A2DP, Wait Xsecs");
	        //MessageSendLater(appGetSmTask(), CONN_RULES_CONNECT_HANDSET, NULL, D_SEC(AUTORECONNECTHEADSET_TIME-1));	//
	        MessageSendLater(appGetUiTask(), APP_EVENT_RECONNECT_HEADSET, NULL, D_SEC(3));
	    }
	}
	else
	{
		MessageCancelAll(appGetUiTask(), APP_MFB_Pairing);
        MessageSendLater(appGetUiTask(), APP_MFB_Pairing, NULL, D_SEC(3));	//
	}
}


void sinkStopAutoReconectHeadsetTimer(void) //add by kimny jim
{

	DEBUG_LOG("APO: Stopping auto ReconectHeadset timer \n");
    //MessageCancelAll(appGetSmTask(), CONN_RULES_CONNECT_HANDSET);
    MessageCancelAll(appGetUiTask(), APP_EVENT_RECONNECT_HEADSET);
}





