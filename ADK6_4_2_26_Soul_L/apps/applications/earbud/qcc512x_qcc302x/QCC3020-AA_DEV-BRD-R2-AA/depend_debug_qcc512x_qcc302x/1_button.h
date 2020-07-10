#ifndef BUTTON_CONFIG_H
#define BUTTON_CONFIG_H

#include "input_event_manager.h"
extern const InputEventConfig_t InputEventConfig;
extern const InputActionMessage_t InputEventActions[15];

#define MFB_BUTTON           (1UL <<  0)

#define APP_MFB_BUTTON_10_SECOND_REL             1012
#define APP_MFB_BUTTON_5_SECOND                  1007
#define APP_MFB_BUTTON_10_SECOND                 1011
#define APP_MFB_BUTTON_1_SECOND_REL              1004
#define APP_MFB_BUTTON_3_SECOND                  1005
#define APP_MFB_BUTTON_8_SECOND_REL              1010
#define APP_MFB_BUTTON_VDIAL_ON                  1002
#define APP_MFB_BUTTON_5_SECOND_REL              1008
#define APP_MFB_BUTTON_15_SECOND                 1013
#define APP_MFB_BUTTON_ANC_TOGGLE_ON_OFF         1001
#define APP_MFB_BUTTON_8_SECOND                  1009
#define APP_MFB_BUTTON_1_SECOND                  1003
#define APP_MFB_BUTTON_15_SECOND_REL             1014
#define APP_MFB_BUTTON_3_SECOND_REL              1006
#define APP_MFB_BUTTON_PRESS                     1000

#endif

