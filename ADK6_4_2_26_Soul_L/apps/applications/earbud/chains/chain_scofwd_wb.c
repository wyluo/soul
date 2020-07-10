/*!
    \copyright Copyright (c) 2020 Qualcomm Technologies International, Ltd.
        All Rights Reserved.
        Qualcomm Technologies International, Ltd. Confidential and Proprietary.
    \version 
    \file chain_scofwd_wb.c
    \brief The chain_scofwd_wb chain. This file is generated by C:/qtil/ADK_QCC512X_QCC302X_WIN_6.4.2.26/tools/chaingen/chaingen.py.
*/

#include <chain_scofwd_wb.h>
#include <cap_id_prim.h>
#include <opmsg_prim.h>
#include <hydra_macros.h>
#include <../av_headset_chain_roles.h>
static const operator_config_t operators[] =
{
    MAKE_OPERATOR_CONFIG_PRIORITY_MEDIUM(CAP_ID_WBS_DEC, OPR_SCO_RECEIVE),
    MAKE_OPERATOR_CONFIG_PRIORITY_HIGH(CAP_ID_WBS_ENC, OPR_SCO_SEND),
    MAKE_OPERATOR_CONFIG(CAP_ID_CVC_RECEIVE_WB, OPR_CVC_RECEIVE),
    MAKE_OPERATOR_CONFIG(CAP_ID_SPLITTER, OPR_SCOFWD_SPLITTER),
    MAKE_OPERATOR_CONFIG(CAP_ID_BASIC_PASS, OPR_SCOFWD_BASIC_PASS),
    MAKE_OPERATOR_CONFIG(EB_CAP_ID_SWITCHED_PASSTHROUGH, OPR_SWITCHED_PASSTHROUGH_CONSUMER),
    MAKE_OPERATOR_CONFIG(EB_CAP_ID_ASYNC_WBS_ENC, OPR_SCOFWD_SEND),
    MAKE_OPERATOR_CONFIG(CAP_ID_CVCHS1MIC_SEND_WB, OPR_CVC_SEND),
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
    {OPR_SCO_SEND, EPR_SCO_TO_AIR, 0},
    {OPR_SWITCHED_PASSTHROUGH_CONSUMER, EPR_SCOFWD_TX_OTA, 0}
};

static const operator_connection_t connections[] =
{
    {OPR_SCO_RECEIVE, 0, OPR_CVC_RECEIVE, 0, 1},
    {OPR_CVC_RECEIVE, 0, OPR_SOURCE_SYNC, 0, 1},
    {OPR_SOURCE_SYNC, 0, OPR_SCOFWD_SPLITTER, 0, 1},
    {OPR_VOLUME_CONTROL, 0, OPR_SCO_AEC, 0, 1},
    {OPR_SCOFWD_SPLITTER, 0, OPR_VOLUME_CONTROL, 0, 1},
    {OPR_SCOFWD_SPLITTER, 1, OPR_SCOFWD_BASIC_PASS, 0, 1},
    {OPR_SCOFWD_BASIC_PASS, 0, OPR_SCOFWD_SEND, 0, 1},
    {OPR_SCOFWD_SEND, 0, OPR_SWITCHED_PASSTHROUGH_CONSUMER, 0, 1},
    {OPR_SCO_AEC, 3, OPR_CVC_SEND, 1, 1},
    {OPR_CVC_SEND, 0, OPR_SCO_SEND, 0, 1},
    {OPR_SCO_AEC, 0, OPR_CVC_SEND, 0, 1}
};

const chain_config_t chain_scofwd_wb_config = {1, 0, operators, 11, inputs, 3, outputs, 3, connections, 11};

