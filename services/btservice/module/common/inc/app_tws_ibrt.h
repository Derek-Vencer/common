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
#ifndef __APP_TWS_IBRT__
#define __APP_TWS_IBRT__
#include "bluetooth_bt_api.h"
#include "a2dp_api.h"
#include "cmsis_os.h"
#include "spp_api.h"
#include "spp_api.h"
#include "hfp_api.h"
#include "bt_drv_reg_op.h"
#include "me_api.h"
#include "btapp.h"
//#include "bt_drv.h"
#include "bts_bt_common_define.h"

#define  MSS_RETRY_COUNTER_MAX          (10)

#define APP_IBRT_RECONNECT_TIMEOUT_MS   5

#define UINT16LOW(num)   ((uint8_t)(num & 0xFF))
#define UINT16HIGH(num)  ((uint8_t)((num >> 8) & 0xFF))

#define APP_IBRT_SUPV_TO (0x640)

#define NV_RAM_RECORD_NUM   (10)

#define INVALID_ERROR        0xFF
#define BT_INVALID_CONN_HANDLE       0xFFFF
#define BD_ADDR_LEN          6
#define DEVICE_INVALID_ID    0xFF


#define IBRT_DISABLE         0
#define IBRT_ENABLE          1
#define IBRT_SWITCH          2

#define IBRT_NONE_ROLE       0
#define IBRT_SNIFFER_ROLE    1
#define IBRT_FORWARD_ROLE    2

#define IBRT_TWS_SNIFF_MAX_INTERVAL (254)
#define IBRT_TWS_SNIFF_MIN_INTERVAL (254)
#define IBRT_TWS_SNIFF_TIMEOUT      (0)

#define IBRT_TWS_LINK_LARGE_DURATION           (12)

#define IBRT_TWS_LINK_DEFAULT_DURATION       (0x8)

#define IBRT_ENCRYPT_DISABLE 0x00
#define IBRT_ENCRYPT_ENABLE  0x01
#define IBRT_ENCRYPT_REFRESH 0x02

#define  IBRT_TWS_BT_TPOLL_STOP_IBRT           (40)

typedef enum
{
    NO_LINK_TYPE,
    SNOOP_LINK,
    MOBILE_LINK,
    TWS_LINK,
} ibrt_link_type_e;

typedef uint8_t sniff_req_direction_e;
#define   SNIFF_LOCAL_REQ        0
#define   SNIFF_REMOTE_REQ       1

/*
 * tx data protect in tws switch
 */
#ifndef __TWS_SWITCH_TX_DATA_PROTECT__
#define __TWS_SWITCH_TX_DATA_PROTECT__
#endif

/*
 * wait some high priority criticals to be finished
 * such as sending accept response of avdtp start/suspend cmd
 * then launch tws switch
 */
#define  TSS_DELAY_CRITICAL_TIMEOUT_MS  50

/*
 * wait profile layer tx data to hci txbuf list
 * wait controller tx data to air
 */
#define  TSS_DELAY_HCI_CMD_TIMEOUT_MS  20



// hci tx buf type mapping
#define IBRT_HBT_TX_ACL     0x05  //#define HBT_TX_ACL   0x05
#define IBRT_HBT_TX_BLE     0x08  //#define HBT_TX_BLE   0x08

typedef  uint8_t   ibrt_controller_error_type;

#define  IBRT_CONTROLLER_TWS_NO_03      (1)
#define  IBRT_CONTROLLER_MOBILE_NO_03   (2)
#define  IBRT_CONTROLLER_TWS_NO_05      (3)
#define  IBRT_CONTROLLER_MOBILE_NO_05   (4)
#define  IBRT_CONTROLLER_DEAD           (5)
#define  IBRT_CONTROLLER_RX_SEQ_ERROR   (6)
#define  IBRT_MOBILE_ENCRYP_ERROR       (7)
#define  IBRT_TWS_BESAUD_CONNECT_ERROR  (8)

typedef enum
{
    TWS_INIT_TRIGGER,
    MOBILE_INIT_TRIGGER,
    START_IBRT_TRIGGER,
    EXCHANGE_PROFILE_TRIGGER,
    SCO_CONNECTED_TRIGGER,
    SCO_DISCONNECT_TRIGGER,
    MOBILE_MSS_TRIGGER,
} ibrt_trigger_link_policy_e;

typedef enum
{
    AUDIO_CHANNEL_SELECT_STEREO,
    AUDIO_CHANNEL_SELECT_LRMERGE,
    AUDIO_CHANNEL_SELECT_LCHNL,
    AUDIO_CHANNEL_SELECT_RCHNL,
} AUDIO_CHANNEL_SELECT_E;

typedef enum {
    IBRT_ROLE_SWITCH_USER_AI    = (1 << 0),
    IBRT_ROLE_SWITCH_USER_OTA   = (1 << 1),
}IBRT_ROLE_SWITCH_USER_E;

typedef enum {
    IBRT_LINK_SWITCH_TO_SINGLE_POINT    = 0,
    IBRT_LINK_SWITCH_TO_MULTI_POINT,
}IBRT_SWITCH_LINK_E;

typedef enum{
    TWS_TIMING_CONTROL_PARAM_PRIORITY_NORMAL = 0,
    TWS_TIMING_CONTROL_PARAM_PRIORITY_ABOVE_NORMAL0,
    TWS_TIMING_CONTROL_PARAM_PRIORITY_ABOVE_NORMAL1,
    TWS_TIMING_CONTROL_PARAM_PRIORITY_ABOVE_NORMAL2,
    TWS_TIMING_CONTROL_PARAM_PARAM_PRIORITY_HIGH,
}TWS_TIMING_CONTROL_PARAM_PRIORITY_E;
/**
 * @brief The user of the timing control user
 *
 */
typedef enum
{
    TWS_TIMING_CONTROL_USER_SNIFF = 0,

    TWS_TIMING_CONTROL_USER_DEFAULT,

    TWS_TIMING_CONTROL_USER_A2DP,

    TWS_TIMING_CONTROL_USER_OTA,

    TWS_TIMING_CONTROL_USER_AI_VOICE,

    TWS_TIMING_CONTROL_USER_START_IBRT,

    TWS_TIMING_CONTROL_USER_IBRT_SWITCH,

    TWS_TIMING_CONTROL_USER_FAST_COMMUNICATION,

    TWS_TIMING_CONTROL_USER_PAGE_ONGOING,

    TWS_TIMING_CONTROL_USER_NUM,
} TWS_TIMING_CONTROL_USER_E;
//sorted by priority

typedef struct
{
    bool freeman_enable;
    bool giveup_reconn_when_peer_unpaired;
    uint8_t support_max_remote_link;
    bool lowlayer_monitor_enable;
    bool allow_sniff_in_sco;
    uint16_t reconnect_ibrt_max_times;
    bool is_changed_to_ui_master_on_tws_disconnected;
    uint8_t llmonitor_report_format;
    uint32_t llmonitor_report_count;
    bool profile_concurrency_supported;
}ibrt_core_param_t;

typedef struct
{
    uint8_t default_tws_duration;
    uint8_t large_tws_duration;
    uint8_t audio_sync_mismatch_resume_version;
    bool    tws_switch_tx_data_protect;

    uint32_t tws_cmd_send_timeout;
    uint32_t tws_cmd_send_counter_threshold;

    uint16_t mobile_page_timeout;
    uint16_t tws_connection_timeout;
} ibrt_core_config_t;

typedef struct
{
    bt_bdaddr_t  addr;
    uint8_t      link_type;
    uint16_t     link_policy;
}link_policy_info_t;

typedef struct
{
    bt_bdaddr_t mobile_addr;
}exit_sniff_info_t;

typedef struct
{
    uint8_t     status;
    bt_bdaddr_t mobile_addr;
}start_ibrt_rsp_info_t;

typedef struct
{
    bt_bdaddr_t mobile_addr;
}start_ibrt_info_t;

typedef struct
{
    bt_bdaddr_t mobile_addr;
    uint8_t restart;
}set_env_info_t;

typedef struct
{
    uint8_t status;
    uint8_t opcode;
    uint8_t error_code;
    bt_bdaddr_t mobile_addr;
}stop_ibrt_rsp_t;

typedef struct
{
    uint8_t opcode;
    uint8_t error_code;
    bt_bdaddr_t mobile_addr;
    uint8_t ibrt_role;
}stop_ibrt_info_t;

typedef struct
{
    bt_bdaddr_t addr;
    uint16_t  mobile_conn_handle;
    uint16_t  ibrt_conn_handle;
    uint32_t constate;
} mobile_info_exchange_t;

typedef struct
{
    bt_bdaddr_t mobile_addr;
    uint8_t master_volume_change_trans_id;
    uint8_t master_avctp_ctl_trans_id;
    bt_bdaddr_t a2dp_last_paused_device;
    bool rs_disallow;
}ibrt_tws_switch_data_sync_t;



typedef struct
{
    ibrt_role_e init_done;
    ibrt_role_e nv_role;
    uint8_t role_switch_debonce_time;
    uint16_t local_battery_volt;
    uint16_t peer_battery_volt;
    bt_bdaddr_t local_addr;
    bt_bdaddr_t peer_addr;
    uint8_t mobile_linkKey[16];
    uint8_t  ibrt_disc_reason;
    uint16_t peer_mobile_conhandle;
    uint32_t tws_constate;
    uint32_t ibrt_ai_role_switch_handle;    //one bit represent a AI
    uint32_t ibrt_role_switch_handle_user;  //one bit represent a user
    osMailQId tws_mailbox;
    osMailQId tws_mailbox2;
    ibrt_codec_t a2dp_codec;
    uint32_t     audio_chnl_sel;
    btif_remote_device_t *p_tws_remote_dev;
    bool is_ibrt_search_ui;
    uint8_t tws_switch_local_done: 1;
    uint8_t tws_switch_peer_done: 1;
    uint8_t master_tws_switch_pending: 1;
    uint8_t avrcp_register_notify_event;
    uint8_t avrcp_wait_register_notify_rsp;
    ibrt_link_mode_e tws_mode;

    bool ibrt_in_poweroff;
    uint32_t tws_cmd_send_time;
    bool w4_stop_ibrt_for_rx_seq_error;
    bool dbg_state_timer_ongoing;
    bool stop_ibrt_timer_ongoing;
    bool ibrt_stopped_due_to_checker;

    bool tws_link_id_is_set;
    osTimerId delay_set_tws_link_id_timer;

    ibrt_core_config_t config;
    ibrt_core_param_t custom_config;

    mobile_info_exchange_t peer_mobile_dev_info[BT_DEVICE_NUM];

    // tws timing control info
    bool    isPeerMobileLinkExistButLocalNotOnTwsConnected;
} ibrt_ctrl_t;

typedef struct
{
    ibrt_role_e nv_role;
    bt_bdaddr_t local_addr;
    bt_bdaddr_t peer_addr;
    bt_bdaddr_t mobile_addr;
    uint32_t    audio_chnl_sel;
} ibrt_config_t;

typedef uint32_t (*spp_is_connected_t)(const uint8_t *uuid_data_ptr, uint8_t uuid_len);

#ifdef __cplusplus
extern "C" {
#endif

void app_spp_is_connected_register(spp_is_connected_t func);
bool app_ui_incoming_sco_req_callback(uint8_t device_id, void *addr);
bool app_ui_incoming_extra_conn_req_callback(uint8_t *addr, uint8_t *cod);

#ifdef __cplusplus
}
#endif

#endif/*__APP_TWS_IBRT__ */
