/*!
    \copyright Copyright (c) 2020 Qualcomm Technologies International, Ltd.
        All Rights Reserved.
        Qualcomm Technologies International, Ltd. Confidential and Proprietary.
    \version 
    \file chain_forwarding_input_aac_stereo_right.c
    \brief The chain_forwarding_input_aac_stereo_right chain. This file is generated by C:/qtil/ADK_QCC512X_QCC302X_WIN_6.4.2.26/tools/chaingen/chaingen.py.
*/

#include <chain_forwarding_input_aac_stereo_right.h>
#include <cap_id_prim.h>
#include <opmsg_prim.h>
#include <hydra_macros.h>
#include <../av_headset_chain_roles.h>
static const operator_config_t operators[] =
{
    MAKE_OPERATOR_CONFIG(CAP_ID_RTP_DECODE, OPR_RTP_DECODER),
    MAKE_OPERATOR_CONFIG(CAP_ID_SPLITTER, OPR_SPLITTER),
    MAKE_OPERATOR_CONFIG(EB_CAP_ID_AAC_DECODER, OPR_AAC_DECODER),
    MAKE_OPERATOR_CONFIG(EB_CAP_ID_SWITCHED_PASSTHROUGH, OPR_CONSUMER)
};

static const operator_endpoint_t inputs[] =
{
    {OPR_RTP_DECODER, EPR_SINK_MEDIA, 0}
};

static const operator_endpoint_t outputs[] =
{
    {OPR_SPLITTER, EPR_SOURCE_FORWARDING_MEDIA, 1},
    {OPR_AAC_DECODER, EPR_SOURCE_DECODED_PCM, 1}
};

static const operator_connection_t connections[] =
{
    {OPR_RTP_DECODER, 0, OPR_SPLITTER, 0, 1},
    {OPR_SPLITTER, 0, OPR_AAC_DECODER, 0, 1},
    {OPR_AAC_DECODER, 0, OPR_CONSUMER, 0, 1}
};

const chain_config_t chain_forwarding_input_aac_stereo_right_config = {0, 0, operators, 4, inputs, 1, outputs, 2, connections, 3};

