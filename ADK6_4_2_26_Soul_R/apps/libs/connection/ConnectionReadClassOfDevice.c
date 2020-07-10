/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    ConnectionReadClassOfDevice.c        

DESCRIPTION
    This file contains the implementation of the entity responsible for 
    configuring the local baseband. This includes making the device
    discoverable, connectable etc.

NOTES

*/


/****************************************************************************
    Header files
*/
#include    "connection.h"
#include    "connection_private.h"
#include    "common.h"

#include <message.h>
#include <panic.h>
#include <string.h>
#include <vm.h>


/*****************************************************************************/
void ConnectionReadClassOfDevice(Task theAppTask)
{
    /* Create internal message and sent to the CL */
    MAKE_CL_MESSAGE(CL_INTERNAL_DM_READ_CLASS_OF_DEVICE_REQ);
    message->theAppTask = theAppTask;
    MessageSend(connectionGetCmTask(), CL_INTERNAL_DM_READ_CLASS_OF_DEVICE_REQ, message);
}

#ifdef SOUL_BLADE
/*****************************************************************************/
void ConnectionReadStoredLinkKey(const bdaddr *addr, uint8 read_all)
{
    /* Send an internal message requesting this action */
    MAKE_CL_MESSAGE(CL_INTERNAL_DM_READ_STORED_LINK_KEY_REQ);
    message->bd_addr = *addr;
    message->read_all = read_all;
    MessageSend(connectionGetCmTask(), CL_INTERNAL_DM_READ_STORED_LINK_KEY_REQ, message);
}
#endif

