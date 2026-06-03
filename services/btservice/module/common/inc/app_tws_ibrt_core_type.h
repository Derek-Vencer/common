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
#ifndef __APP_TWS_IBRT_CORE_TYPE_H__
#define __APP_TWS_IBRT_CORE_TYPE_H__

#if defined(IBRT_UI)

#include "stdint.h"
#include "cmsis_os.h"

#include "app_ibrt_conn_evt.h"

#define  IBRT_UI_INVALID_RSSI                      (100)

#define  IBRT_UI_MIN_RSSI                          (-100)

#define  STEAL_MOBILE_TIMEOUT                      (1000)

#define IBRT_HOST_CANCEL_PAGE                      (2)

typedef enum
{
    IBRT_UI_NO_ERROR,
    IBRT_UI_RSP_TIMEOUT,
    IBRT_UI_NOT_ACCEPT,
    IBRT_UI_CONNECT_FAILED,
    IBRT_UI_PAGE_TIMEOUT             = 0x04,
    IBRT_UI_STATUS_ERROR,
    IBRT_CONN_CONNECTION_TIMEOUT     = 0x08,
    IBRT_UI_ACL_ALREADY_EXIST        = 0x0B,
    IBRT_UI_CMD_DISALLOWED           = 0X0C,
    IBRT_UI_LIMITED_RESOURCE         = 0x0D,
    IBRT_UI_HOST_ACCEPT_TIMEOUT      = 0x10,
    IBRT_CONN_TERM_USER_REQ          = 0x13,
    IBRT_UI_CONN_TERM_LOW_RESOURCES  = 0x14,
    IBRT_CONN_TERM_BY_LOCAL_HOST     = 0x16,
    IBRT_CONN_ERROR_LMP_RSP_TIMEOUT  = 0x22,
    IBRT_UI_MOBILE_CONN_DISCONNECTED = 0X2A,
    IBRT_UI_TWS_CONN_DISCONNECTED    = 0X2B,
    IBRT_UI_SNOOP_DISCONNECTED       = 0X2C,
    IBRT_UI_EST_OR_SYNC_TIMEOUT      = 0x3E,
    IBRT_CONN_NO_PROFILE_CONNECTED   = 0x50,
    IBRT_UI_CONNECTION_INCOMING      = 0x99,
    IBRT_UI_EVT_STATUS_ERROR         = 0x9A,
    IBRT_UI_TWS_CMD_SEND_FAILED      = 0X9B,
    IBRT_UI_MOBILE_PAIR_CANCLED      = 0X9C,
    BT_LINK_REAL_DISCONNECTED        = 0xB8,
    IBRT_BT_CANCEL_PAGE              = 0xB9,
    IBRT_UI_SWITCH_IN_POCESS         = 0xBA,
    IBRT_LMP_TX_BUFFER_OVERFLOW      = 0xBD,
    IBRT_CONN_LINK_KEY_MISSING       = 0xF1,
} ibrt_conn_error_e;

typedef struct
{
    uint16_t battery_volt;
    uint16_t  mobile_conhandle;
    uint16_t  tws_conhandle;
} app_ui_rssi_battery_info_t;

#define  IBRT_STOP_IBRT                         1

//HCI opcode
#define    IBRT_HCI_CREATE_CON_CMD_OPCODE       0x0405
#define    IBRT_HCI_EXIT_SNIFF_MODE_CMD_OPCODE  0x0804
#define    IBRT_HCI_SWITCH_ROLE_CMD_OPCODE      0x080B
#define    IBRT_HCI_STOP_IBRT_OPCODE            0xFCA8
#define    IBRT_HCI_START_IBRT_OPCODE           0xFCA3
#define    IBRT_HCI_RESET_OPCODE                0x0C03
#define    IBRT_HCI_DSIC_CON_CMD_OPCODE         0x0406
#define    IBRT_HCI_SET_ENV_CMD_OPCODE          0xFC8E


typedef enum
{
    OUTGOING_CONNECTION_REQ = 0,
    INCOMMING_CONNECTION_REQ,

}connection_direction_t;

typedef struct
{
    uint8_t   num_hci_cmd_packets;
    uint16_t  cmd_opcode;
    uint8_t   param[1];
} __attribute__ ((packed)) ibrt_cmd_comp_t;

typedef struct
{
    uint8_t  status;
    uint8_t  num_hci_cmd_packets;
    uint16_t cmd_opcode;
    bt_bdaddr_t bdaddr;
} __attribute__ ((packed)) ibrt_cmd_status_t;

/*
 * IBRT RAW_UI EVENT
 */
typedef enum {
    IBRT_STATUS_SUCCESS                  = 0,
    IBRT_STATUS_PENDING                  = 1,
    IBRT_STATUS_ERROR_INVALID_PARAMETERS = 2,
    IBRT_STATUS_ERROR_NO_CONNECTION      = 3,
    IBRT_STATUS_ERROR_CONNECTION_EXISTS  = 4,
    IBRT_STATUS_IN_PROGRESS              = 5,
    IBRT_STATUS_ERROR_DUPLICATE_REQUEST  = 6,
    IBRT_STATUS_ERROR_INVALID_STATE      = 7,
    IBRT_STATUS_ERROR_TIMEOUT            = 8,
    IBRT_STATUS_ERROR_ROLE_SWITCH_FAILED = 9,
    IBRT_STATUS_ERROR_UNEXPECTED_VALUE  = 10,
    IBRT_STATUS_ERROR_OP_NOT_ALLOWED    = 11,

    // Start vendor section
    IBRT_STATUS_VENDOR_START = 0x80,
    // BTCLIENT_STATUS_ERROR_SAMPLE = BTCLIENT_STATUS_VENDOR_START + 0,
    // End vendor
} ibrt_status_t;

typedef enum {
    ACL_CONNECTED_USR_TERMINATE        = 0,
    ACL_CONNECTING_CANCELED ,
    ACL_CONNECTING_FAILURE,
} acl_disconnect_reason;

typedef struct
{
    bool tws_switch_according_to_rssi_value;
    uint8_t rssi_threshold;
    uint8_t role_switch_timer_threshold;

    uint32_t connect_no_03_timeout;
    uint32_t disconnect_no_05_timeout;
} ibrt_conn_config_t;

typedef enum
{
    FREEMAN_MODE,
    IBRT_MODE,
}ibrt_mode_e;

typedef uint8_t tws_role_e;
#define   TWS_MASTER       0
#define   TWS_SLAVE        1
#define   TWS_ROLE_UNKNOW  0xff



typedef struct
{
    bt_bdaddr_t   mobile_addr;
    uint8_t       mobile_connected;
    uint8_t       sco_status;
} __attribute__((packed)) app_tws_start_ibrt_info_t;

typedef struct
{
    bt_bdaddr_t     local_addr;
    tws_role_e      current_ibrt_role;

    bt_bdaddr_t     peer_addr;
    tws_role_e      peer_ibrt_role;

    tws_role_e      nv_role;
} app_tws_buds_info_t;

typedef struct
{
    int32_t a2dp_volume;
    int32_t hfp_volume;
} ibrt_volume_info_t;

typedef void (*ibrt_post_func)(void);

//Register for BES UI
typedef struct {
    void (*send_mgr_info_hanlder_cb)(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
    void (*send_mgr_info_rsp_handler_cb)(uint8_t *p_buff, uint16_t length);
    void (*send_mgr_info_rsp_timeout_hanlder_cb)(uint8_t *p_buff, uint16_t length);
    void (*send_mgr_info_fail_cb)(uint8_t *p_buff);

    void (*link_run_complete_info_handler)(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
    void (*link_run_complete_info_rsp_handler)(uint8_t *p_buff, uint16_t length);
    void (*link_run_complete_info_rsp_timeout_handler)(uint8_t *p_buff, uint16_t length);

    void (*peer_run_complete_info_handler)(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
    void (*peer_run_complete_info_rsp_handler)(uint8_t *p_buff, uint16_t length);
    void (*peer_run_complete_info_rsp_timeout_handler)(uint8_t *p_buff, uint16_t length);

    void (*sync_info_handler)(uint16_t rsp_seq,uint8_t *p_buff, uint16_t length);
    void (*sync_info_rsp_handler)(uint8_t *p_buff, uint16_t length);
    void (*sync_info_rsp_timeout_handler)(uint8_t *p_buff, uint16_t length);

    void (*destroy_device_handler)(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
    void (*destroy_device_rsp_handler)(uint8_t *p_buff, uint16_t length);
    void (*destroy_device_rsp_timeout_handler)(uint8_t *p_buff, uint16_t length);

    void (*perform_received_choice_mobile_connect)(const bt_bdaddr_t *addr, uint8_t try_count);
    void (*notify_ui_info_handler)(uint8_t *p_buff, uint16_t length);

    void (*common_chnl_recv_handler)(uint8_t *p_buff, uint16_t length);

#if BLE_AUDIO_ENABLED
    void (*notify_peer_bt_nv_recored_changed_handler)(uint8_t *p_buff, uint16_t length);
    void (*sync_deivce_irk_handler)(uint8_t *p_buff, uint16_t length);
#endif

    void (*send_dev_mgr_hanlder_cb)(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
    void (*send_dev_mgr_rsp_handler_cb)(uint8_t *p_buff, uint16_t length);
    void (*send_dev_mgr_rsp_timeout_hanlder_cb)(uint8_t *p_buff, uint16_t length);

    void (*lea_addr_mgr_handler)(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
    void (*lea_addr_mgr_rsp_handler)(uint8_t *p_buff, uint16_t length);
    void (*lea_addr_mgr_rsp_timeout_handler)(uint8_t *p_buff, uint16_t length);
} app_ibrt_cmd_ui_handler_cb;

typedef struct {
    void (*keyboard_request_handler)(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
    void (*ui_perform_user_action)(uint8_t *p_buff, uint16_t length);
    void (*conn_profile_handler)(bool is_ibrt_slave_receive_request, uint8_t *p_buff, uint16_t length);
    void (*disc_profile_handler)(uint8_t *p_buff, uint16_t length);
    void (*disc_rfcomm_handler)(uint8_t *p_buff);
    bool (*ibrt_if_sniff_prevent_need)(bt_bdaddr_t *p_mobile_addr);
    bool (*tws_switch_prepare_needed)(uint32_t *wait_ms);
    void (*tws_swtich_prepare)(uint32_t timeoutMs);
    void (*switch_background_handler)(uint8_t *p_buff, uint16_t length);
}app_ibrt_if_cbs_t;

#endif

#endif /*__APP_TWS_IBRT_RAW_UI_H__*/
