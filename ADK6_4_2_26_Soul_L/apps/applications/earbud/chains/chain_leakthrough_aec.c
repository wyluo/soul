/*!
    \copyright Copyright (c) 2020 Qualcomm Technologies International, Ltd.
        All Rights Reserved.
        Qualcomm Technologies International, Ltd. Confidential and Proprietary.
    \version 
    \file chain_leakthrough_aec.c
    \brief The chain_leakthrough_aec chain. This file is generated by C:/qtil/ADK_QCC512X_QCC302X_WIN_6.4.2.26/tools/chaingen/chaingen.py.
*/

#include <chain_leakthrough_aec.h>
#include <cap_id_prim.h>
#include <opmsg_prim.h>
#include <hydra_macros.h>
#include <../av_headset_chain_roles.h>
static const operator_config_t operators[] =
{
    MAKE_OPERATOR_CONFIG(CAP_ID_DOWNLOAD_AEC_REFERENCE, OPR_LEAKTHROUGH_AEC),
    MAKE_OPERATOR_CONFIG(EB_CAP_ID_SWITCHED_PASSTHROUGH, OPR_LEAKTHROUGH_INPUT_SPC)
};

static const operator_endpoint_t inputs[] =
{
    {OPR_LEAKTHROUGH_AEC, EPR_LEAKTHROUGH_IN, 0}
};

static const operator_endpoint_t outputs[] =
{
    {OPR_LEAKTHROUGH_AEC, EPR_A2DP_LEAKTHROUGH_AEC_OUT, 1}
};

static const operator_connection_t connections[] =
{
    {OPR_LEAKTHROUGH_AEC, 3, OPR_LEAKTHROUGH_INPUT_SPC, 0, 1}
};

const chain_config_t chain_leakthrough_aec_config = {1, 0, operators, 2, inputs, 1, outputs, 1, connections, 1};

