/*
Copyright (c) 2005 - 2016 Qualcomm Technologies International, Ltd.
*/
/**
\file
\ingroup sink_app

*/

#ifndef _SINK_AT_H_
#define _SINK_AT_H_


/*these values have special meanings in the AT commands and will be replaced in place*/
void avAtCmdInit(void);
void avSendBattATCmd(void);
void avAtCmdSendBatt(uint8 percent);
void avAtCmdEnableBattReport(void);
#endif /* _SINK_AT_H_ */
