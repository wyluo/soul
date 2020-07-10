/* Automatically generated nanopb constant definitions */
/* Generated by nanopb-0.3.9.3 at Wed Jul 10 21:49:08 2019. */

#include "bme_protocol.pb.h"

/* @@protoc_insertion_point(includes) */
#if PB_PROTO_HEADER_VERSION != 30
#error Regenerate this file with the current version of nanopb generator.
#endif



const pb_field_t Beflex_BiomechEngine_Message_fields[14] = {
    PB_ONEOF_FIELD(typeName,   1, MESSAGE , ONEOF, STATIC  , FIRST, Beflex_BiomechEngine_Message, parameterRequest, parameterRequest, &Beflex_BiomechEngine_Parameter_Request_fields),
    PB_ONEOF_FIELD(typeName,   2, MESSAGE , ONEOF, STATIC  , UNION, Beflex_BiomechEngine_Message, parameterResponse, parameterResponse, &Beflex_BiomechEngine_Parameter_Response_fields),
    PB_ONEOF_FIELD(typeName,   3, MESSAGE , ONEOF, STATIC  , UNION, Beflex_BiomechEngine_Message, parameterGait, parameterGait, &Beflex_BiomechEngine_Parameter_Gait_fields),
    PB_ONEOF_FIELD(typeName,   4, MESSAGE , ONEOF, STATIC  , UNION, Beflex_BiomechEngine_Message, calibrationRequest, calibrationRequest, &Beflex_BiomechEngine_Calibration_Request_fields),
    PB_ONEOF_FIELD(typeName,   5, MESSAGE , ONEOF, STATIC  , UNION, Beflex_BiomechEngine_Message, calibrationResponse, calibrationResponse, &Beflex_BiomechEngine_Calibration_Response_fields),
    PB_ONEOF_FIELD(typeName,   6, MESSAGE , ONEOF, STATIC  , UNION, Beflex_BiomechEngine_Message, customRequest, customRequest, &Beflex_BiomechEngine_Custom_Request_fields),
    PB_ONEOF_FIELD(typeName,   7, MESSAGE , ONEOF, STATIC  , UNION, Beflex_BiomechEngine_Message, customResponse, customResponse, &Beflex_BiomechEngine_Custom_Response_fields),
    PB_ONEOF_FIELD(typeName,   8, MESSAGE , ONEOF, STATIC  , UNION, Beflex_BiomechEngine_Message, customNotification, customNotification, &Beflex_BiomechEngine_Custom_Notification_fields),
    PB_ONEOF_FIELD(typeName,   9, MESSAGE , ONEOF, STATIC  , UNION, Beflex_BiomechEngine_Message, dfuRequest, dfuRequest, &Beflex_BiomechEngine_FOTA_DfuRequest_fields),
    PB_ONEOF_FIELD(typeName,  10, MESSAGE , ONEOF, STATIC  , UNION, Beflex_BiomechEngine_Message, dfuResponse, dfuResponse, &Beflex_BiomechEngine_FOTA_DfuResponse_fields),
    PB_ONEOF_FIELD(typeName,  11, MESSAGE , ONEOF, STATIC  , UNION, Beflex_BiomechEngine_Message, packetRequest, packetRequest, &Beflex_BiomechEngine_FOTA_PacketRequest_fields),
    PB_ONEOF_FIELD(typeName,  12, MESSAGE , ONEOF, STATIC  , UNION, Beflex_BiomechEngine_Message, packetResponse, packetResponse, &Beflex_BiomechEngine_FOTA_PacketResponse_fields),
    PB_ONEOF_FIELD(typeName,  13, MESSAGE , ONEOF, STATIC  , UNION, Beflex_BiomechEngine_Message, dfuResult, dfuResult, &Beflex_BiomechEngine_FOTA_DfuResult_fields),
    PB_LAST_FIELD
};

const pb_field_t Beflex_BiomechEngine_Parameter_fields[1] = {
    PB_LAST_FIELD
};

const pb_field_t Beflex_BiomechEngine_Parameter_Request_fields[5] = {
    PB_FIELD(  1, UENUM   , SINGULAR, STATIC  , FIRST, Beflex_BiomechEngine_Parameter_Request, type, type, 0),
    PB_FIELD(  2, UINT32  , SINGULAR, STATIC  , OTHER, Beflex_BiomechEngine_Parameter_Request, period, type, 0),
    PB_FIELD(  3, FLOAT   , SINGULAR, STATIC  , OTHER, Beflex_BiomechEngine_Parameter_Request, height, period, 0),
    PB_FIELD(  4, BYTES   , SINGULAR, STATIC  , OTHER, Beflex_BiomechEngine_Parameter_Request, calValue, height, 0),
    PB_LAST_FIELD
};

const pb_field_t Beflex_BiomechEngine_Parameter_Response_fields[3] = {
    PB_FIELD(  1, UENUM   , SINGULAR, STATIC  , FIRST, Beflex_BiomechEngine_Parameter_Response, type, type, 0),
    PB_FIELD(  2, BOOL    , SINGULAR, STATIC  , OTHER, Beflex_BiomechEngine_Parameter_Response, success, type, 0),
    PB_LAST_FIELD
};

const pb_field_t Beflex_BiomechEngine_Parameter_Gait_fields[19] = {
    PB_FIELD(  1, UINT32  , SINGULAR, STATIC  , FIRST, Beflex_BiomechEngine_Parameter_Gait, state, state, 0),
    PB_FIELD(  2, UINT32  , SINGULAR, STATIC  , OTHER, Beflex_BiomechEngine_Parameter_Gait, elapsed, state, 0),
    PB_FIELD(  3, UINT32  , SINGULAR, STATIC  , OTHER, Beflex_BiomechEngine_Parameter_Gait, stepCount, elapsed, 0),
    PB_FIELD(  4, FLOAT   , SINGULAR, STATIC  , OTHER, Beflex_BiomechEngine_Parameter_Gait, cadence, stepCount, 0),
    PB_FIELD(  5, FLOAT   , SINGULAR, STATIC  , OTHER, Beflex_BiomechEngine_Parameter_Gait, stepWidth, cadence, 0),
    PB_FIELD(  6, FLOAT   , SINGULAR, STATIC  , OTHER, Beflex_BiomechEngine_Parameter_Gait, headAngle, stepWidth, 0),
    PB_FIELD(  7, FLOAT   , SINGULAR, STATIC  , OTHER, Beflex_BiomechEngine_Parameter_Gait, wssl_rgtl, headAngle, 0),
    PB_FIELD(  8, FLOAT   , SINGULAR, STATIC  , OTHER, Beflex_BiomechEngine_Parameter_Gait, wssr_rgtr, wssl_rgtl, 0),
    PB_FIELD(  9, FLOAT   , SINGULAR, STATIC  , OTHER, Beflex_BiomechEngine_Parameter_Gait, wdsl_rftl, wssr_rgtr, 0),
    PB_FIELD( 10, FLOAT   , SINGULAR, STATIC  , OTHER, Beflex_BiomechEngine_Parameter_Gait, wdsr_rgtr, wdsl_rftl, 0),
    PB_FIELD( 11, FLOAT   , SINGULAR, STATIC  , OTHER, Beflex_BiomechEngine_Parameter_Gait, wfpl_rmll, wdsr_rgtr, 0),
    PB_FIELD( 12, FLOAT   , SINGULAR, STATIC  , OTHER, Beflex_BiomechEngine_Parameter_Gait, wfpr_rmlr, wfpl_rmll, 0),
    PB_FIELD( 13, FLOAT   , SINGULAR, STATIC  , OTHER, Beflex_BiomechEngine_Parameter_Gait, shock, wfpr_rmlr, 0),
    PB_FIELD( 14, FLOAT   , SINGULAR, STATIC  , OTHER, Beflex_BiomechEngine_Parameter_Gait, symmetry, shock, 0),
    PB_FIELD( 15, FLOAT   , SINGULAR, STATIC  , OTHER, Beflex_BiomechEngine_Parameter_Gait, vo, symmetry, 0),
    PB_FIELD( 16, FLOAT   , SINGULAR, STATIC  , OTHER, Beflex_BiomechEngine_Parameter_Gait, legStiffness, vo, 0),
    PB_FIELD( 17, FLOAT   , SINGULAR, STATIC  , OTHER, Beflex_BiomechEngine_Parameter_Gait, consistency, legStiffness, 0),
    PB_FIELD( 18, FLOAT   , SINGULAR, STATIC  , OTHER, Beflex_BiomechEngine_Parameter_Gait, speed, consistency, 0),
    PB_LAST_FIELD
};

const pb_field_t Beflex_BiomechEngine_Calibration_fields[1] = {
    PB_LAST_FIELD
};

const pb_field_t Beflex_BiomechEngine_Calibration_Request_fields[3] = {
    PB_FIELD(  1, UENUM   , SINGULAR, STATIC  , FIRST, Beflex_BiomechEngine_Calibration_Request, type, type, 0),
    PB_FIELD(  2, UINT32  , SINGULAR, STATIC  , OTHER, Beflex_BiomechEngine_Calibration_Request, period, type, 0),
    PB_LAST_FIELD
};

const pb_field_t Beflex_BiomechEngine_Calibration_Response_fields[4] = {
    PB_FIELD(  1, UENUM   , SINGULAR, STATIC  , FIRST, Beflex_BiomechEngine_Calibration_Response, type, type, 0),
    PB_FIELD(  2, BOOL    , SINGULAR, STATIC  , OTHER, Beflex_BiomechEngine_Calibration_Response, success, type, 0),
    PB_FIELD(  3, BYTES   , SINGULAR, STATIC  , OTHER, Beflex_BiomechEngine_Calibration_Response, calValue, success, 0),
    PB_LAST_FIELD
};

const pb_field_t Beflex_BiomechEngine_Custom_fields[1] = {
    PB_LAST_FIELD
};

const pb_field_t Beflex_BiomechEngine_Custom_Request_fields[4] = {
    PB_FIELD(  1, UENUM   , SINGULAR, STATIC  , FIRST, Beflex_BiomechEngine_Custom_Request, type, type, 0),
    PB_FIELD(  2, UINT32  , SINGULAR, STATIC  , OTHER, Beflex_BiomechEngine_Custom_Request, param, type, 0),
    PB_FIELD(  3, BYTES   , SINGULAR, STATIC  , OTHER, Beflex_BiomechEngine_Custom_Request, data, param, 0),
    PB_LAST_FIELD
};

const pb_field_t Beflex_BiomechEngine_Custom_Response_fields[5] = {
    PB_FIELD(  1, UENUM   , SINGULAR, STATIC  , FIRST, Beflex_BiomechEngine_Custom_Response, type, type, 0),
    PB_FIELD(  2, UINT32  , SINGULAR, STATIC  , OTHER, Beflex_BiomechEngine_Custom_Response, param, type, 0),
    PB_FIELD(  3, BOOL    , SINGULAR, STATIC  , OTHER, Beflex_BiomechEngine_Custom_Response, success, param, 0),
    PB_FIELD(  4, BYTES   , SINGULAR, STATIC  , OTHER, Beflex_BiomechEngine_Custom_Response, data, success, 0),
    PB_LAST_FIELD
};

const pb_field_t Beflex_BiomechEngine_Custom_Notification_fields[3] = {
    PB_FIELD(  1, UINT32  , SINGULAR, STATIC  , FIRST, Beflex_BiomechEngine_Custom_Notification, param, param, 0),
    PB_FIELD(  2, BYTES   , SINGULAR, STATIC  , OTHER, Beflex_BiomechEngine_Custom_Notification, data, param, 0),
    PB_LAST_FIELD
};

const pb_field_t Beflex_BiomechEngine_FOTA_fields[1] = {
    PB_LAST_FIELD
};

const pb_field_t Beflex_BiomechEngine_FOTA_DfuRequest_fields[4] = {
    PB_FIELD(  1, UINT32  , SINGULAR, STATIC  , FIRST, Beflex_BiomechEngine_FOTA_DfuRequest, target, target, 0),
    PB_FIELD(  2, UINT32  , SINGULAR, STATIC  , OTHER, Beflex_BiomechEngine_FOTA_DfuRequest, chunkSize, target, 0),
    PB_FIELD(  3, UINT32  , SINGULAR, STATIC  , OTHER, Beflex_BiomechEngine_FOTA_DfuRequest, totalLength, chunkSize, 0),
    PB_LAST_FIELD
};

const pb_field_t Beflex_BiomechEngine_FOTA_DfuResponse_fields[2] = {
    PB_FIELD(  1, BOOL    , SINGULAR, STATIC  , FIRST, Beflex_BiomechEngine_FOTA_DfuResponse, success, success, 0),
    PB_LAST_FIELD
};

const pb_field_t Beflex_BiomechEngine_FOTA_PacketRequest_fields[2] = {
    PB_FIELD(  1, BYTES   , SINGULAR, STATIC  , FIRST, Beflex_BiomechEngine_FOTA_PacketRequest, value, value, 0),
    PB_LAST_FIELD
};

const pb_field_t Beflex_BiomechEngine_FOTA_PacketResponse_fields[2] = {
    PB_FIELD(  1, BOOL    , SINGULAR, STATIC  , FIRST, Beflex_BiomechEngine_FOTA_PacketResponse, success, success, 0),
    PB_LAST_FIELD
};

const pb_field_t Beflex_BiomechEngine_FOTA_DfuResult_fields[2] = {
    PB_FIELD(  1, BOOL    , SINGULAR, STATIC  , FIRST, Beflex_BiomechEngine_FOTA_DfuResult, success, success, 0),
    PB_LAST_FIELD
};





/* Check that field information fits in pb_field_t */
#if !defined(PB_FIELD_32BIT)
/* If you get an error here, it means that you need to define PB_FIELD_32BIT
 * compile-time option. You can do that in pb.h or on compiler command line.
 * 
 * The reason you need to do this is that some of your messages contain tag
 * numbers or field sizes that are larger than what can fit in 8 or 16 bit
 * field descriptors.
 */
PB_STATIC_ASSERT((pb_membersize(Beflex_BiomechEngine_Message, typeName.parameterRequest) < 65536 && pb_membersize(Beflex_BiomechEngine_Message, typeName.parameterResponse) < 65536 && pb_membersize(Beflex_BiomechEngine_Message, typeName.parameterGait) < 65536 && pb_membersize(Beflex_BiomechEngine_Message, typeName.calibrationRequest) < 65536 && pb_membersize(Beflex_BiomechEngine_Message, typeName.calibrationResponse) < 65536 && pb_membersize(Beflex_BiomechEngine_Message, typeName.customRequest) < 65536 && pb_membersize(Beflex_BiomechEngine_Message, typeName.customResponse) < 65536 && pb_membersize(Beflex_BiomechEngine_Message, typeName.customNotification) < 65536 && pb_membersize(Beflex_BiomechEngine_Message, typeName.dfuRequest) < 65536 && pb_membersize(Beflex_BiomechEngine_Message, typeName.dfuResponse) < 65536 && pb_membersize(Beflex_BiomechEngine_Message, typeName.packetRequest) < 65536 && pb_membersize(Beflex_BiomechEngine_Message, typeName.packetResponse) < 65536 && pb_membersize(Beflex_BiomechEngine_Message, typeName.dfuResult) < 65536), YOU_MUST_DEFINE_PB_FIELD_32BIT_FOR_MESSAGES_Beflex_BiomechEngine_Message_Beflex_BiomechEngine_Parameter_Beflex_BiomechEngine_Parameter_Request_Beflex_BiomechEngine_Parameter_Response_Beflex_BiomechEngine_Parameter_Gait_Beflex_BiomechEngine_Calibration_Beflex_BiomechEngine_Calibration_Request_Beflex_BiomechEngine_Calibration_Response_Beflex_BiomechEngine_Custom_Beflex_BiomechEngine_Custom_Request_Beflex_BiomechEngine_Custom_Response_Beflex_BiomechEngine_Custom_Notification_Beflex_BiomechEngine_FOTA_Beflex_BiomechEngine_FOTA_DfuRequest_Beflex_BiomechEngine_FOTA_DfuResponse_Beflex_BiomechEngine_FOTA_PacketRequest_Beflex_BiomechEngine_FOTA_PacketResponse_Beflex_BiomechEngine_FOTA_DfuResult)
#endif

#if !defined(PB_FIELD_16BIT) && !defined(PB_FIELD_32BIT)
/* If you get an error here, it means that you need to define PB_FIELD_16BIT
 * compile-time option. You can do that in pb.h or on compiler command line.
 * 
 * The reason you need to do this is that some of your messages contain tag
 * numbers or field sizes that are larger than what can fit in the default
 * 8 bit descriptors.
 */
PB_STATIC_ASSERT((pb_membersize(Beflex_BiomechEngine_Message, typeName.parameterRequest) < 256 && pb_membersize(Beflex_BiomechEngine_Message, typeName.parameterResponse) < 256 && pb_membersize(Beflex_BiomechEngine_Message, typeName.parameterGait) < 256 && pb_membersize(Beflex_BiomechEngine_Message, typeName.calibrationRequest) < 256 && pb_membersize(Beflex_BiomechEngine_Message, typeName.calibrationResponse) < 256 && pb_membersize(Beflex_BiomechEngine_Message, typeName.customRequest) < 256 && pb_membersize(Beflex_BiomechEngine_Message, typeName.customResponse) < 256 && pb_membersize(Beflex_BiomechEngine_Message, typeName.customNotification) < 256 && pb_membersize(Beflex_BiomechEngine_Message, typeName.dfuRequest) < 256 && pb_membersize(Beflex_BiomechEngine_Message, typeName.dfuResponse) < 256 && pb_membersize(Beflex_BiomechEngine_Message, typeName.packetRequest) < 256 && pb_membersize(Beflex_BiomechEngine_Message, typeName.packetResponse) < 256 && pb_membersize(Beflex_BiomechEngine_Message, typeName.dfuResult) < 256), YOU_MUST_DEFINE_PB_FIELD_16BIT_FOR_MESSAGES_Beflex_BiomechEngine_Message_Beflex_BiomechEngine_Parameter_Beflex_BiomechEngine_Parameter_Request_Beflex_BiomechEngine_Parameter_Response_Beflex_BiomechEngine_Parameter_Gait_Beflex_BiomechEngine_Calibration_Beflex_BiomechEngine_Calibration_Request_Beflex_BiomechEngine_Calibration_Response_Beflex_BiomechEngine_Custom_Beflex_BiomechEngine_Custom_Request_Beflex_BiomechEngine_Custom_Response_Beflex_BiomechEngine_Custom_Notification_Beflex_BiomechEngine_FOTA_Beflex_BiomechEngine_FOTA_DfuRequest_Beflex_BiomechEngine_FOTA_DfuResponse_Beflex_BiomechEngine_FOTA_PacketRequest_Beflex_BiomechEngine_FOTA_PacketResponse_Beflex_BiomechEngine_FOTA_DfuResult)
#endif


/* @@protoc_insertion_point(eof) */
