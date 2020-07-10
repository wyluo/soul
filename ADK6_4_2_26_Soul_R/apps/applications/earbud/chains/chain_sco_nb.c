/*!
    \copyright Copyright (c) 2020 Qualcomm Technologies International, Ltd.
        All Rights Reserved.
        Qualcomm Technologies International, Ltd. Confidential and Proprietary.
    \version 
    \file chain_sco_nb.c
    \brief The chain_sco_nb chain. This file is generated by C:/qtil/ADK_QCC512X_QCC302X_WIN_6.4.2.26/tools/chaingen/chaingen.py.
*/

#include <chain_sco_nb.h>
#include <cap_id_prim.h>
#include <opmsg_prim.h>
#include <hydra_macros.h>
#include <../av_headset_chain_roles.h>
static const operator_config_t operators[] =
{
    MAKE_OPERATOR_CONFIG_PRIORITY_MEDIUM(CAP_ID_SCO_RCV, OPR_SCO_RECEIVE),
    MAKE_OPERATOR_CONFIG_PRIORITY_HIGH(CAP_ID_SCO_SEND, OPR_SCO_SEND),
    MAKE_OPERATOR_CONFIG(CAP_ID_CVC_RECEIVE_NB, OPR_CVC_RECEIVE),
    MAKE_OPERATOR_CONFIG(CAP_ID_CVCHS1MIC_SEND_NB, OPR_CVC_SEND),
    MAKE_OPERATOR_CONFIG(CAP_ID_SOURCE_SYNC, OPR_SOURCE_SYNC),
    MAKE_OPERATOR_CONFIG(CAP_ID_VOL_CTRL_VOL, OPR_VOLUME_CONTROL),
    MAKE_OPERATOR_CONFIG(EB_CAP_ID_AEC_REF, OPR_SCO_AEC)
};

static const operator_endpoint_t inputs[] =
{
    {OPR_SCO_RECEIVE, EPR_SCO_FROM_AIR, 0},
    {OPR_SCO_AEC, EPR_SCO_MIC1, 2},
    {OPR_VOLUME_CONTROL, EPR_VOLUME_AUX, 1}
};

static const operator_endpoint_t outputs[] =
{
    {OPR_SCO_AEC, EPR_SCO_SPEAKER, 1},
    {OPR_SCO_SEND, EPR_SCO_TO_AIR, 0}
};

static const operator_connection_t connections[] =
{
    {OPR_SCO_RECEIVE, 0, OPR_CVC_RECEIVE, 0, 1},
    {OPR_CVC_RECEIVE, 0, OPR_SOURCE_SYNC, 0, 1},
    {OPR_SOURCE_SYNC, 0, OPR_VOLUME_CONTROL, 0, 1},
    {OPR_VOLUME_CONTROL, 0, OPR_SCO_AEC, 0, 1},
    {OPR_SCO_AEC, 3, OPR_CVC_SEND, 1, 1},
    {OPR_CVC_SEND, 0, OPR_SCO_SEND, 0, 1},
    {OPR_SCO_AEC, 0, OPR_CVC_SEND, 0, 1}
};

const chain_config_t chain_sco_nb_config = {0, 0, operators, 7, inputs, 3, outputs, 2, connections, 7};

