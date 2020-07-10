/*!
    \copyright Copyright (c) 2020 Qualcomm Technologies International, Ltd.
        All Rights Reserved.
        Qualcomm Technologies International, Ltd. Confidential and Proprietary.
    \version 
    \file chain_output_volume.c
    \brief The chain_output_volume chain. This file is generated by C:/qtil/ADK_QCC512X_QCC302X_WIN_6.4.2.26/tools/chaingen/chaingen.py.
*/

#include <chain_output_volume.h>
#include <cap_id_prim.h>
#include <opmsg_prim.h>
#include <hydra_macros.h>
#include <../av_headset_chain_roles.h>
static const operator_config_t operators[] =
{
    MAKE_OPERATOR_CONFIG(CAP_ID_BASIC_PASS, OPR_LATENCY_BUFFER),
    MAKE_OPERATOR_CONFIG(CAP_ID_SOURCE_SYNC, OPR_SOURCE_SYNC),
    MAKE_OPERATOR_CONFIG(CAP_ID_VOL_CTRL_VOL, OPR_VOLUME_CONTROL),
    MAKE_OPERATOR_CONFIG(CAP_ID_PEQ, OPR_PEQ)
};

static const operator_endpoint_t inputs[] =
{
    {OPR_LATENCY_BUFFER, EPR_SINK_MIXER_MAIN_IN, 0},
    {OPR_VOLUME_CONTROL, EPR_VOLUME_AUX, 1}
};

static const operator_endpoint_t outputs[] =
{
    {OPR_VOLUME_CONTROL, EPR_SOURCE_MIXER_OUT, 0}
};

static const operator_connection_t connections[] =
{
    {OPR_LATENCY_BUFFER, 0, OPR_SOURCE_SYNC, 0, 1},
    {OPR_SOURCE_SYNC, 0, OPR_PEQ, 0, 1},
    {OPR_PEQ, 0, OPR_VOLUME_CONTROL, 0, 1}
};

const chain_config_t chain_output_volume_config = {0, 0, operators, 4, inputs, 2, outputs, 1, connections, 3};

