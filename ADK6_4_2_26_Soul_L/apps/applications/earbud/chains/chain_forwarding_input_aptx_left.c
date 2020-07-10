/*!
    \copyright Copyright (c) 2020 Qualcomm Technologies International, Ltd.
        All Rights Reserved.
        Qualcomm Technologies International, Ltd. Confidential and Proprietary.
    \version 
    \file chain_forwarding_input_aptx_left.c
    \brief The chain_forwarding_input_aptx_left chain. This file is generated by C:/qtil/ADK_QCC512X_QCC302X_WIN_6.4.2.26/tools/chaingen/chaingen.py.
*/

#include <chain_forwarding_input_aptx_left.h>
#include <cap_id_prim.h>
#include <opmsg_prim.h>
#include <hydra_macros.h>
#include <../av_headset_chain_roles.h>
static const operator_config_t operators[] =
{
    MAKE_OPERATOR_CONFIG(CAP_ID_RTP_DECODE, OPR_RTP_DECODER),
    MAKE_OPERATOR_CONFIG(EB_CAP_ID_APTX_CLASSIC_DEMUX, OPR_APTX_DEMUX),
    MAKE_OPERATOR_CONFIG(EB_CAP_ID_SWITCHED_PASSTHROUGH, OPR_SWITCHED_PASSTHROUGH_CONSUMER),
    MAKE_OPERATOR_CONFIG(CAP_ID_APTX_CLASSIC_MONO_DECODER_NO_AUTOSYNC, OPR_APTX_CLASSIC_MONO_DECODER_NO_AUTOSYNC)
};

static const operator_endpoint_t inputs[] =
{
    {OPR_RTP_DECODER, EPR_SINK_MEDIA, 0}
};

static const operator_endpoint_t outputs[] =
{
    {OPR_SWITCHED_PASSTHROUGH_CONSUMER, EPR_SOURCE_FORWARDING_MEDIA, 0},
    {OPR_APTX_CLASSIC_MONO_DECODER_NO_AUTOSYNC, EPR_SOURCE_DECODED_PCM, 0}
};

static const operator_connection_t connections[] =
{
    {OPR_RTP_DECODER, 0, OPR_APTX_DEMUX, 0, 1},
    {OPR_APTX_DEMUX, 0, OPR_APTX_CLASSIC_MONO_DECODER_NO_AUTOSYNC, 0, 1},
    {OPR_APTX_DEMUX, 1, OPR_SWITCHED_PASSTHROUGH_CONSUMER, 0, 1}
};

const chain_config_t chain_forwarding_input_aptx_left_config = {0, 0, operators, 4, inputs, 1, outputs, 2, connections, 3};

