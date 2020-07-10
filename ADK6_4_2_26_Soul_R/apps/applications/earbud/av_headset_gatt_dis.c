/*!
\copyright  Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
            All Rights Reserved.
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version
\file       av_headset_gatt_ptek.c
\brief      Application support for PTEK, HR Service
*/

#include "av_headset.h"

#ifdef SOUL_DEVICE_INFO
#include <gatt.h>
#include <gatt_manager.h>
#include <gatt_server.h>

#include "av_headset_db.h"
#include "av_headset_log.h"
#include "av_headset_gatt.h"
#include "av_headset_gatt_dis.h"



static gatt_dis_init_params_t dis_init_params;

/*******************************************************************************/
static bool appGattGetDeviceInfoParams(void)
{
    dis_init_params.dis_strings = PanicUnlessMalloc(sizeof(gatt_dis_strings_t));

    if(dis_init_params.dis_strings != NULL)
    {
        memset(dis_init_params.dis_strings, 0, sizeof(gatt_dis_strings_t));

        dis_init_params.dis_strings->manufacturer_name_string = (const char*)"Beflex Inc.";
        dis_init_params.dis_strings->model_num_string = NULL;
        dis_init_params.dis_strings->serial_num_string = (const char*)0x06B9;
        dis_init_params.dis_strings->hw_revision_string = NULL;
        dis_init_params.dis_strings->fw_revision_string = NULL;
        dis_init_params.dis_strings->sw_revision_string = NULL;
        
        return TRUE;
    }

    /* Failed to allocate memory */
    DEBUG_LOGF("GATT Device Info Server failed to allocate memory\n");
    return FALSE;
}

/*******************************************************************************/
static void appGattFreeDisPtrs(void)
{
    if(dis_init_params.dis_strings != NULL)
        free(dis_init_params.dis_strings);
}

/*******************************************************************************/
bool appGattDeviceInfoServerInitialise(int gatt_instance)
{

    /* Read the device information service to be initialized */
    if(appGattGetDeviceInfoParams())
    {
        if (GattDeviceInfoServerInit(appGetGattDisTask(), 
                                appGetGattDeviceInfoServerInst(gatt_instance),
                                &dis_init_params,
                                HANDLE_DEVICE_INFORMATION_SERVICE,
                                HANDLE_DEVICE_INFORMATION_SERVICE_END))
        {
            DEBUG_LOGF("GATT Device Info Server initialised\n");
            return TRUE;
        }

        /* Failed to initialize Device Information server */
        DEBUG_LOGF("GATT Device Info Server init failed\n");
        /* Free the allocated memory */
        appGattFreeDisPtrs();
    }
    return FALSE;
}

#endif /* SOUL_BLADE */

