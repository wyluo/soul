#ifndef AV_HEADSET_GATT_DIS_H
#define AV_HEADSET_GATT_DIS_H
#ifdef SOUL_DEVICE_INFO

#include <gatt_device_info_server.h>
#include <csrtypes.h>
#include <message.h>

#define GATT_DIS_MAX_MANUF_NAME_LEN     31

#ifdef GATT_DIS_SERVER
#define sinkGattDeviceInfoServerGetSize() sizeof(gdiss_t)
#else
#define sinkGattDeviceInfoServerGetSize() 0
#endif

/*******************************************************************************
NAME
    appGattDeviceInfoServerInitialise
    
DESCRIPTION
    Initialise DIS server task.
    
PARAMETERS
    ptr - pointer to allocated memory to store server tasks rundata.
    
RETURNS
    TRUE if the DIS server task was initialised, FALSE otherwise.
*/
bool appGattDeviceInfoServerInitialise(int gatt_instance);
#endif
#endif // AV_HEADSET_GATT_DIS_H
