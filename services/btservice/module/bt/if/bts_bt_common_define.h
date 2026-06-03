/***************************************************************************
 *
 * Copyright 2015-2025 BES.
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
#ifndef __BTS_SINK_COMMON_DEFINE_H__
#define __BTS_SINK_COMMON_DEFINE_H__
#include "bluetooth_bt_api.h"
#include "bt_drv_reg_op.h"
#include "btapp.h"

typedef uint8_t ibrt_role_e;
#define   IBRT_MASTER       0
#define   IBRT_SLAVE        1
#define   IBRT_UNKNOW       0xff


typedef uint8_t tss_state_e;
#define  TSS_IDLE_STATE          (0x00)
#define  TSS_CRITICAL_WAIT_STATE (0x01)
#define  TSS_TX_WAIT_STATE       (0x02)
#define  TSS_TIMER_WAIT_STATE    (0x04)
#define  TSS_CMD_STATE           (0x08)
#define  TSS_SLAVE_WAIT_STATE    (0x10)

typedef uint8_t ibrt_link_mode_e;
#define IBRT_ACTIVE_MODE     0x00
#define IBRT_HOLD_MODE       0x01
#define IBRT_SNIFF_MODE      0x02
#define IBRT_PARK_MODE       0x03
#define IBRT_SCATTER_MODE    0x04

typedef enum {
    IBRT_CONN_EVENT_FIRST                   = 0x1000,
    IBRT_CONN_EVENT_API_STATUS              = IBRT_CONN_EVENT_FIRST,
    IBRT_CONN_EVENT_TW_PAIRING_STATE        = 0x1001,
    IBRT_CONN_EVENT_TW_CONNECTION_STATE     = 0x1002,
    IBRT_CONN_EVENT_TW_ROLE_CHANGE_STATE    = 0x1003,
    IBRT_CONN_EVENT_HOST_PAIRING_STATE      = 0x1004,
    IBRT_CONN_EVENT_MOBILE_CONNECTION_STATE = 0x1005,
    IBRT_CONN_EVENT_IBRT_CONNECTION_STATE   = 0x1006,
    IBRT_CONN_EVENT_LE_STATE                = 0x1007,
    IBRT_CONN_EVENT_BT_STATE                = 0x1008,
    IBRT_CONN_EVENT_RSSI_REPORT             = 0x1009,
    IBRT_CONN_EVENT_A2DP_STATE              = 0x100a,
    IBRT_CONN_EVENT_AVRCP_STATE             = 0x100b,
    IBRT_CONN_EVENT_HFP_STATE               = 0x100c,
    IBRT_CONN_EVENT_HSP_STATE               = 0x100d,
    IBRT_CONN_EVENT_GLOBAL_STATE            = 0x100e,
    IBRT_CONN_EVENT_SCO_CONNECTION_STATE    = 0x100f,
    IBRT_CONN_EVENT_ACCESS_MODE             = 0x1010,
    IBRT_CONN_EVENT_SMP_IA_EXCH_CMP         = 0x1011,

    IBRT_CONN_EVENT_LAST                    = 0x2000
} ibrt_conn_event_type;

typedef enum {
    IBRT_CONN_L2CAP_DISCONNECTED = 0,
    IBRT_CONN_L2CAP_CONNECTING   = 1,
    IBRT_CONN_L2CAP_CONNECTED    = 2,
    IBRT_CONN_L2CAP_ERROR        = 3,
} ibrt_conn_l2cap_state;

typedef enum {
    IBRT_CONN_A2DP_IDLE             = 0,
    IBRT_CONN_A2DP_CLOSED           = 1,
    IBRT_CONN_A2DP_CODEC_CONFIGURED = 2,
    IBRT_CONN_A2DP_OPEN             = 3,
    IBRT_CONN_A2DP_STREAMING        = 4,
    IBRT_CONN_A2DP_SUSPENED         = 5,
} ibrt_conn_a2dp_state;

typedef enum {
    IBRT_CONN_CODEC_AAC  = 0,
    IBRT_CONN_CODEC_SBC  = 1,
    IBRT_CONN_CODEC_MSBC = 2,
    IBRT_CONN_CODEC_CVSD = 3,
} ibrt_conn_codec_type;

typedef enum {
    IBRT_CONN_AUDIO_SAMPLING_RATE_8000  = 0,
    IBRT_CONN_AUDIO_SAMPLING_RATE_16000 = 1,
    IBRT_CONN_AUDIO_SAMPLING_RATE_44100 = 2,
    IBRT_CONN_AUDIO_SAMPLING_RATE_48000 = 3,
} ibrt_conn_audio_sampling_rate;

typedef enum {
    IBRT_CONN_AVRCP_DISCONNECTED   = 0,
    IBRT_CONN_AVRCP_CONNECTED      = 1,
    IBRT_CONN_AVRCP_VOLUME_UPDATED = 2,
    IBRT_CONN_AVRCP_REMOTE_CT_0104 = 3,
    IBRT_CONN_AVRCP_REMOTE_SUPPORT_PLAYBACK_STATUS_CHANGED_EVENT = 4,
    IBRT_CONN_AVRCP_PLAYBACK_STATUS_CHANGED = 5,
    IBRT_CONN_AVRCP_PLAY_STATUS_CHANGED = 6,
    IBRT_CONN_AVRCP_PLAY_POS_CHANGED = 7,
} ibrt_conn_avrcp_state;

typedef enum {
    IBRT_CONN_HFP_SLC_DISCONNECTED = 0,
    IBRT_CONN_HFP_SLC_OPEN         = 1,
    IBRT_CONN_HFP_SCO_OPEN         = 2,
    IBRT_CONN_HFP_SCO_CLOSED       = 3,
    IBRT_CONN_HFP_RING_IND         = 4,
    IBRT_CONN_HFP_CALL_IND         = 5,
    IBRT_CONN_HFP_CALLSETUP_IND    = 6,
    IBRT_CONN_HFP_CALLHELD_IND     = 7,
    IBRT_CONN_HFP_CIEV_SERVICE_IND = 8,
    IBRT_CONN_HFP_CIEV_SIGNAL_IND  = 9,
    IBRT_CONN_HFP_CIEV_ROAM_IND    = 10,
    IBRT_CONN_HFP_CIEV_BATTCHG_IND = 11,
    IBRT_CONN_HFP_SPK_VOLUME_IND   = 12,
    IBRT_CONN_HFP_MIC_VOLUME_IND   = 13,
    IBRT_CONN_HFP_IN_BAND_RING_IND = 14,
    IBRT_CONN_HFP_VR_STATE_IND     = 15,
    IBRT_CONN_HFP_AT_CMD_COMPLETE  = 16,
    IBRT_CONN_HFP_CURRENT_CALL_STATE = 17,
} ibrt_conn_hfp_state;



typedef struct
{
    uint8_t  codec_type;
    uint8_t  sample_bit;
    uint8_t  sample_rate;
    uint8_t  vendor_para;
} __attribute__((packed)) ibrt_codec_t;

typedef struct
{
    uint8_t *hfp_data;
    uint8_t *a2dp_data;
    uint8_t *avrcp_data;
    uint16_t hfp_data_len;
    uint16_t a2dp_data_len;
    uint16_t avrcp_data_len;

    uint8_t *map_data;
    uint16_t map_data_len;

#if defined(__GATT_OVER_BR_EDR__)
    uint8_t *btgatt_data;
    uint16_t btgatt_data_len;
#endif

    uint8_t *hid_data;
    uint16_t hid_data_len;

    //spp related
    uint8_t *spp_data[SPP_DEVICE_NUM];
    uint16_t spp_data_len[SPP_DEVICE_NUM];
    uint64_t app_id[SPP_DEVICE_NUM];
    uint8_t  spp_amount;
    uint8_t  spp_num;
} data_store_mem_t;

typedef struct
{
    bt_bdaddr_t mobile_addr;
    ibrt_role_e current_role;
    uint16_t mobile_conhandle;
    uint16_t ibrt_conhandle;

    uint64_t mobile_constate;
    uint64_t ibrt_constate;

    uint64_t latest_disconnected_profile;

    osTimerDefEx_t delay_tws_switch_timer_def;
    osTimerId delay_tws_switch_cmd_timer_id;

    osTimerDefEx_t delay_profile_send_timer_def;
    osTimerId delay_profile_send_timer_id;

    osTimerDefEx_t wait_profile_ready_timer_def;
    osTimerId wait_profile_ready_timer_id;

    osTimerDefEx_t wait_profile_exchange_complete_timer_def;
    osTimerId wait_profile_exchange_complete_timer_id;

    bool wait_profile;
    uint16_t profile_exchange_state;
    uint32_t profile_exchange_delay;
    uint64_t rx_profile_update;
    uint64_t tx_profile_update;
    bool snoop_connected;
    bool profile_exchanged;
    bool a2dp_profile_exchanged;
    bool avrcp_profile_exchanged;
    tss_state_e tss_state;

    bool mobile_pair_canceled;
    bool mobile_encryp_done;

    uint8_t ibrt_disc_reason;

    //SCO sniffing connected before HFP profle restored
    uint8_t ibrt_sco_activate;
    hfp_sco_codec_t ibrt_sco_codec;
    uint8_t is_waiting_for_hfp_status_from_master;

    uint8_t tws_switch_local_done: 1;
    uint8_t tws_switch_peer_done: 1;
    uint8_t master_tws_switch_pending: 1;
    uint8_t avrcp_register_notify_event;
    uint8_t avrcp_wait_register_notify_rsp;

    btif_remote_device_t *p_mobile_remote_dev;
    ibrt_codec_t a2dp_codec;
    void *p_mobile_a2dp_profile;
    void *p_mobile_hfp_profile;
    void *p_mobile_avrcp_profile;
    void *p_mobile_hid_profile;
    void *p_mobile_reseved0_profile;
    void *p_mobile_reseved1_profile;
    ibrt_link_mode_e mobile_mode;
    uint16_t sniff_interval;
    uint8_t mobile_linkKey[16];
    bool phone_connect_happened;
    bool role_switch_cmd_set;
// using for role-switch test -begin
    bool rs_monitor_start_request;
    bool rs_monitor_send_tws_cmd;
    bool rs_monitor_get_tws_rsp;
    bool rs_monitor_send_cmd_to_btc;
    bool rs_monitor_get_btc_rsp;

    bool rs_monitor_get_tws_cmd;
    bool rs_monitor_send_tws_rsp;
// using for role-switch test -end

    bool sync_a2dp_status;
    uint64_t basic_profiles;
    data_store_mem_t profile_data;
} ibrt_mobile_info_t;

typedef struct {
    ibrt_conn_event_type   type;
    uint16_t               length;
} ibrt_conn_evt_header;

typedef struct {
    ibrt_conn_codec_type          codec;
    ibrt_conn_audio_sampling_rate sampling_rate;
    uint32_t                      bitrate;
    bool                          variable_bitrate;
} ibrt_conn_a2dp_audio_settings;

#define MAX_CLCC_NUM_SIZE     0x10
typedef struct {
    uint8_t num_len;
    char number[MAX_CLCC_NUM_SIZE+1];
} clcc_state_t;

typedef struct {
    ibrt_conn_evt_header          header;
    ibrt_conn_l2cap_state         l2cap_state;
    ibrt_conn_a2dp_state          a2dp_state;
    ibrt_conn_a2dp_audio_settings audio_settings; /* Valid for CODEC_CONFIGURED */
    bool                          delay_report_support;
    uint8_t                       device_id;
    bt_bdaddr_t                   addr;
} ibrt_conn_a2dp_state_change;

typedef struct {
    ibrt_conn_evt_header          header;
    ibrt_conn_l2cap_state         l2cap_state;
    ibrt_conn_avrcp_state         avrcp_state;
    uint8_t                       volume; /* Valid for VOLUME_UPDATE */
    uint8_t                       playback_status;
    uint32_t                      play_position;    /* playback progress */
    uint32_t                      play_length;      /* current media length in ms */
    bool                          support;
    uint8_t                       device_id;
    bt_bdaddr_t                   addr;
} ibrt_conn_avrcp_state_change;

typedef struct {
    ibrt_conn_evt_header  header;
    ibrt_conn_l2cap_state l2cap_state;
    ibrt_conn_hfp_state   hfp_state;
    ibrt_conn_codec_type  codec;
    uint8_t               device_id;
    bt_bdaddr_t           addr;
    uint8_t               ciev_status;
    uint8_t               volume_ind;
    uint8_t               error_code;
    uint8_t               voice_rec_state;
    uint8_t               in_band_ring_enable;
    clcc_state_t          curr_call;
} ibrt_conn_hfp_state_change;

typedef struct {
    void (*ibrt_a2dp_state_changed_hook)(const bt_bdaddr_t *addr, ibrt_conn_a2dp_state_change *state);
    void (*ibrt_hfp_state_changed_hook)(const bt_bdaddr_t *addr, ibrt_conn_hfp_state_change *state);
    void (*ibrt_avrcp_state_changed_hook)(const bt_bdaddr_t *addr, ibrt_conn_avrcp_state_change *state);
}app_ibrt_profiles_hook_cb;

typedef struct {
    void (*profile_evt_callback)(const bt_bdaddr_t *addr, uint8_t device_id, uint64_t profile, uint8_t event, uint8_t param);
    void (*dev_acl_disconnect_callback)(void);
}bts_bt_to_gfps_cbs_t;

typedef struct
{
    bt_bdaddr_t remote;
    uint8_t a2dp_local_volume;
    uint8_t hfp_local_volume;
} TWS_VOLUME_SYNC_INFO_T_V2;

typedef struct
{
    unsigned int event;
    unsigned int param0;
    unsigned int param1;
    unsigned int param2;
} app_ibrt_conn_msg_t;

#endif
