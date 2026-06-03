/***************************************************************************
 *
 * Copyright 2015-2019 BES.
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a database or retrieval system, without prior written
 * permission of BES.
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary information of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/
#ifndef __APP_IBRT_CONN_EVENT_H__
#define __APP_IBRT_CONN_EVENT_H__

#include "nvrecord_bt.h"
#include "bluetooth_bt_api.h"
#include "app_tws_ibrt.h"

/*This file is not only contain ibrt connection message and also contain
* IBRT link mgr message, IBRT connection messge is from IBRT connection,
* such as mobile connection,TWS connection status changed and so on.
* if function need to switch thread ......, reuse recive thread.
*/

#define IBRT_CONN_STATE_MASK        0xF0
#define IBRT_CONN_STATE_PAIRING     0x00
#define IBRT_CONN_STATE_CONNECTION  0x10
#define IBRT_SM_EVENT_BASE          0x4000

typedef enum
{
    //Messages for handling connect/disconnect requests.
    SIGNAL_START_IBRT = IBRT_SM_EVENT_BASE,
    SIGNAL_START_MSS,
    SIGNAL_START_SET_ENV,
    SIGNAL_MOBILE_ENTER_ACTIVE_MODE,
    SIGNAL_STOP_IBRT,
    SIG_RETRY_START_IBRT,

    // Messages for handling error conditions.

    // Messages for feedback from custom API.
    EVT_MOBILE_MSS_COMPLETEED,
    EVT_SET_ENV_COMPLETEED,
    EVT_START_IBRT_COMPLETED,
    EVT_STOP_IBRT_COMPLETED,
    EVT_ENTER_ACTIVE_MODE_COMPLETED,
    EVT_DATA_EXCHANGE_COMPLETED,

    EVT_IBRT_TWS_DISCONNECTED,
    EVT_IBRT_MOBILE_DISCONNECTED,

    EVT_IBRT_MAX_NUM,
} ibrt_sm_message_e;

typedef enum {
    IBRT_CONN_STATUS_SUCCESS                  = 0,
    IBRT_CONN_STATUS_PENDING                  = 1,
    IBRT_CONN_STATUS_ERROR_INVALID_PARAMETERS = 2,
    IBRT_CONN_STATUS_ERROR_NO_CONNECTION      = 3,
    IBRT_CONN_STATUS_ERROR_CONNECTION_EXISTS  = 4,
    IBRT_CONN_STATUS_IN_PROGRESS              = 5,
    IBRT_CONN_STATUS_ERROR_DUPLICATE_REQUEST  = 6,
    IBRT_CONN_STATUS_ERROR_INVALID_STATE      = 7,
    IBRT_CONN_STATUS_ERROR_TIMEOUT            = 8,
    IBRT_CONN_STATUS_ERROR_ROLE_SWITCH_FAILED = 9,
    IBRT_CONN_STATUS_ERROR_UNEXPECTED_VALUE  = 10,
    IBRT_CONN_STATUS_ERROR_OP_NOT_ALLOWED    = 11,

    IBRT_CONN_MGR_STATUS_VENDOR_START = 0x80,

} ibrt_conn_status;

typedef enum {
    IBRT_CONN_PAIRING_IDLE        = 0,
    IBRT_CONN_PAIRING_IN_PROGRESS = 1,
    IBRT_CONN_PAIRING_COMPLETE    = 2,
    IBRT_CONN_PAIRING_TIMEOUT     = 3,
} ibrt_conn_pairing_state;

typedef enum {
    IBRT_CONN_ACL_DISCONNECTED              = 0,
    IBRT_CONN_ACL_CONNECTING                = 1,
    IBRT_CONN_ACL_CONNECTING_CANCELED       = 2,
    IBRT_CONN_ACL_CONNECTING_FAILURE        = 3,
    IBRT_CONN_ACL_RAW_CONNECTED             = 4,
    IBRT_CONN_ACL_CONNECTED                 = 5,
    IBRT_CONN_ACL_PROFILES_CONNECTED        = 6,
    IBRT_CONN_ACL_AUTH_COMPLETE             = 7,
    IBRT_CONN_ACL_DISCONNECTING             = 8,
    IBRT_CONN_ACL_SIMPLE_PIARING_COMPLETE   = 9,
    IBRT_CONN_ACL_UNKNOWN                   = 10,
} ibrt_conn_acl_state;

typedef enum {
    IBRT_CONN_SCO_DISCONNECTED        = 0,
    IBRT_CONN_SCO_CONNECTED           = 1,
    IBRT_CONN_SCO_UNKNOWN             = 2,
} ibrt_conn_sco_state;


typedef enum {
    IBRT_CONN_IBRT_DISCONNECTED       = 0,
    IBRT_CONN_IBRT_CONNECTED          = 1,
    IBRT_CONN_IBRT_START_FAIL         = 2,
    IBRT_CONN_IBRT_ACL_CONNECTED      = 3,
} ibrt_conn_ibrt_state;

typedef enum {
    IBRT_BLUETOOTH_ENABLED      = 1,
    IBRT_BLUETOOTH_DISABLED     = 2,
} ibrt_global_state_enum;

typedef struct {
    ibrt_conn_acl_state acl_state;
    /* Valid for ACL_DISCONNECTED, ACL_CONNECTING_FAILURE, ACL_CONNECTING_CANCELED */
    uint8_t          bluetooth_reason_code;
} ibrt_conn_connection_state;

typedef enum {
    IBRT_CORE_V2_HOST_CONNECT_CANCEL = 0,
} ibrt_conn_api;

typedef struct {
    ibrt_conn_ibrt_state ibrt_state;
    uint8_t              ibrt_reason_code;
} ibrt_conn_ibrt_connection_state;

typedef struct {
    ibrt_conn_sco_state  sco_state;
    uint8_t              sco_reason_code;
} ibrt_conn_sco_connection_state;

typedef enum {
    IBRT_CONN_ROLE_IDLE             = 0,
    IBRT_CONN_ROLE_SWAP_INITIATED   = 1,
    IBRT_CONN_ROLE_SWAP_COMPLETE    = 2,
    IBRT_CONN_ROLE_CHANGE_REQUESTED = 3,
    IBRT_CONN_ROLE_CHANGE_COMPLETE  = 4,
    IBRT_CONN_ROLE_CHANGED          = 5,
    IBRT_CONN_ROLE_SWAP_DISALLOW    = 6,
    IBRT_CONN_ROLE_SWAP_PASSIVE     = 7,
    IBRT_CONN_ROLE_UPDATE           = 8,
    IBRT_CONN_ROLE_SWAP_FAILED      = 9,
    IBRT_CONN_ROLE_SWAP_LE_DONE     = 10,
} ibrt_conn_role_change_state;

typedef struct {
    ibrt_conn_evt_header header;
    ibrt_conn_api        command;
    ibrt_conn_status     status;
} ibrt_conn_api_status_event;

typedef struct {
    ibrt_conn_evt_header       header;
    ibrt_conn_connection_state connection_state;
    ibrt_conn_pairing_state    pairing_state;
} ibrt_conn_tws_pairing_state_evt;

typedef struct {
    ibrt_conn_evt_header     header;
    ibrt_conn_connection_state state;
    uint8_t              current_role;
} ibrt_conn_tws_conn_state_event;

typedef struct {
    ibrt_conn_evt_header         header;
    ibrt_conn_role_change_state  role_state;
    ibrt_conn_status             role_change_status;
    ibrt_role_e                  role;
    bt_bdaddr_t                  addr;
} ibrt_conn_tw_role_change_state_event;

typedef struct {
    ibrt_conn_evt_header       header;
    ibrt_conn_connection_state connection_state;
    ibrt_conn_pairing_state    pairing_state;
    bt_bdaddr_t                addr;
} ibrt_mobile_pairing_state_event;

typedef struct {
    ibrt_conn_evt_header       header;
    ibrt_conn_connection_state state;
    uint8_t                    current_role;
    uint8_t                    device_id;
    bt_bdaddr_t                addr;
} ibrt_mobile_conn_state_event;

typedef struct {
    ibrt_conn_evt_header             header;
    ibrt_conn_ibrt_connection_state  state;
    uint8_t                          current_role;
    uint8_t                          device_id;
    bt_bdaddr_t                      addr;
} ibrt_connection_state_event;

typedef struct {
    ibrt_conn_evt_header             header;
    ibrt_conn_sco_connection_state   state;
    uint8_t                          current_role;
    uint8_t                          device_id;
    bt_bdaddr_t                      addr;
} ibrt_sco_conn_state_event;

typedef struct {
    ibrt_conn_evt_header          header;
    ibrt_global_state_enum        state;
} ibrt_global_state_change_event;

typedef struct {
    ibrt_conn_evt_header          header;
    btif_accessible_mode_t        access_mode;
} ibrt_access_mode_change_event;

typedef struct {
    ibrt_conn_evt_header                header;
    ble_bdaddr_t                        addr;
} ibrt_conn_smp_ia_exch_cmp_t;

//Add more packet
typedef union {
  ibrt_conn_tws_pairing_state_evt       tws_pairing_state;
  ibrt_conn_tws_conn_state_event        tws_conn_state;
  ibrt_conn_tw_role_change_state_event  role_change_state;
  ibrt_mobile_pairing_state_event       mobile_pairing_state;
  ibrt_mobile_conn_state_event          mobile_conn_state;
  ibrt_connection_state_event           ibrt_conn_state;
  ibrt_conn_a2dp_state_change           a2dp_state;
  ibrt_conn_avrcp_state_change          avrcp_state;
  ibrt_conn_hfp_state_change            hfp_state;
  ibrt_global_state_change_event        global_state;
  ibrt_sco_conn_state_event             sco_conn_state;
  ibrt_access_mode_change_event         access_mode_state;
  ibrt_conn_smp_ia_exch_cmp_t           smp_ia_exch_cmp;
} ibrt_conn_event_packet;

#endif

