/* Copyright (c) 2017 Qualcomm Technologies International, Ltd. */
/*  */

/*!
@file    
@brief   Header file for the GATT BME300 Service library.

        This file provides documentation for the GATT BME300 Service library
        API (library name: gatt_transport_discovery_server).
*/

#ifndef GATT_BME300_SERVER_H
#define GATT_BME300_SERVER_H


#include <csrtypes.h>
#include <message.h>

#include <library.h>
#include <source_.h>
#include <sink_.h>

#include "gatt_manager.h"


/* There currently are no transport-specific data. Update this value if needed.*/
#define TRANSPORT_SPECIFIC_DATA_SIZE 0

/*
    The TDS indication is made up of an octet for the requested Op Code, an octet
    for the result code, and optionally an octet for the relevant Organization ID
    followed by transport-specific data up to the size of the MTU.
*/
#define BME300_INDICATION_SIZE    (3 + TRANSPORT_SPECIFIC_DATA_SIZE)

#define activate_transport 0x01
#define SIG_org_ID 0x01


/* All current result codes. */
#define op_code_success             0x00
#define op_code_not_supported       0x01
#define op_code_invalid_parameter   0x02
#define op_code_unsupported_org_id  0x03
#define op_code_operation_failed    0x04


/*! @brief BME300 Server library  data structure type .
 */

/* This structure is made public to application as application is responsible for managing resources 
 * for the elements of this structure. The data elements are indented to use by BME300 Server lib only. 
 * Application SHOULD NOT access (read/write) any elements of this library structure at any point of time and doing so  
 * may cause undesired behavior of this library's functionalities.
 */
typedef struct __GBMES_T
{
    TaskData lib_task;
    Task app_task;
} GBMES_T;


typedef struct _bmePacketIns{
    Source bmeSrc;
    Sink bmeSnk;
    uint16 handle_data_endpoint;
    uint16 handle_response_endpoint;
} bmePacketIns_t;


/*! @brief Contents of the GATT_BME300_SERVER_READ_CLIENT_CONFIG_IND message that is sent by the library,
    due to a read of the TDS client configuration characteristic.
 */
typedef struct __GATT_BME300_SERVER_READ_CLIENT_CONFIG_IND
{
    const GBMES_T *bmes;      /*! Reference structure for the instance  */
    uint16 cid;  			/*! Connection ID */
    bmePacketIns_t bmeIns;
} GATT_BME300_SERVER_READ_CLIENT_CONFIG_IND_T;

/*! @brief Contents of the GATT_BME300_SERVER_WRITE_CLIENT_CONFIG_IND message that is sent by the library,
    due to a write of the TDS client configuration characteristic.
 */
typedef struct __GATT_BME300_SERVER_WRITE_CLIENT_CONFIG_IND
{
    const GBMES_T *bmes;      /*! Reference structure for the instance  */
    uint16 cid;             /*! Connection ID */
    bmePacketIns_t bmeIns;
    uint16 size_value;
} GATT_BME300_SERVER_WRITE_CLIENT_CONFIG_IND_T;

typedef struct __GATT_BME300_SERVER_WRITE_MORE_DATA_IND
{
    const GBMES_T *bmes;      /*! Reference structure for the instance  */
    uint16 cid;             /*! Connection ID */
    bmePacketIns_t bmeIns;
    uint16 dataOffset;
    uint16 remainSize;
} GATT_BME300_SERVER_WRITE_MORE_DATA_IND_T;

typedef struct __GATT_BME300_SERVER_WRITE_READY_IND
{
    const GBMES_T *bmes;      /*! Reference structure for the instance  */
    uint16 cid;             /*! Connection ID */
} GATT_BME300_SERVER_WRITE_READY_IND_T;

typedef struct
{
    const GBMES_T *bmes;		/*! Reference structure for the instance  */
    gatt_status_t  status;		/*! Disconnection status. */
    uint16         cid;			/*! Connection identifier of remote device. */
} GATT_BME300_SERVER_DISCONNECT_IND_T;

/*! @brief Enumeration of messages an application task can receive from the TDS library.
 */
typedef enum
{
    /* Server messages */
    GATT_BME300_SERVER_READ_CLIENT_CONFIG_IND = GATT_BME300_SERVER_MESSAGE_BASE,
    GATT_BME300_SERVER_WRITE_CLIENT_CONFIG_CFM, 
    GATT_BME300_SERVER_WRITE_CLIENT_CONFIG_IND, 
    GATT_BME300_SERVER_WRITE_MORE_DATA_IND,
    GATT_BME300_SERVER_WRITE_READY_IND,
    GATT_BME300_SERVER_DISCONNECT_IND,
    
    /* Library message limit */
    GATT_BME300_SERVER_MESSAGE_TOP
} gatt_bmes_server_message_id_t;


void setBmePacketInstance(bmePacketIns_t bmeIns);

void bleProtoProc(const GBMES_T *bmes,uint16 cid);
void updateGaitPram(const GBMES_T *bmes, uint16 cid);


/*!
    @brief Initializes the BME300 Service Library.

    @param appTask The Task that will receive the messages sent from this TDS library.
    @param bmes A valid area of memory that the TDS library can use.Must be of at least the size of GBMES_T
    @param start_handle This indicates the start handle of the service
    @param end_handle This indicates the end handle of the service
    
    @return TRUE if success, FALSE otherwise.

*/
uint8 GattBme300ServerInit(Task appTask, GBMES_T *const bmes, uint16 start_handle, uint16 end_handle);

#endif /* GATT_BME300_SERVER_H */

