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
#ifndef BESBT_CFG_H
#define BESBT_CFG_H
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    // BASIC
    uint8_t bt_tx_type;
    uint8_t sniff:1;
    uint8_t dont_auto_report_delay_report:1;
    uint8_t vendor_codec_en:1;
    uint8_t force_use_cvsd:1;
    uint8_t support_enre_mode:1;
    uint8_t source_get_all_cap_flag:1;
    uint8_t source_unknown_cmd_flag:1;
    uint8_t disc_acl_after_auth_key_missing:1;
    uint8_t use_page_scan_repetition_mode_r1:1;
    uint8_t normal_test_mode_switch:1;
    uint8_t mark_some_code_for_fuzz_test:1;
    uint8_t watch_is_sending_spp:1;
    uint8_t force_normal_search:1;
    uint8_t pts_test_dont_bt_role_switch:1;
    uint8_t bt_source_enable:1;
    uint8_t bt_sink_enable:1;
    // L2CAP
    uint8_t send_l2cap_echo_req:1;
    // DIP
    uint16_t dip_vendor_id;
    uint16_t dip_product_id;
    uint16_t dip_product_version;
    uint16_t dip_vendor_id_source;
    // A2DP
    uint8_t a2dp_source_enable:1;
    uint8_t a2dp_sink_enable:1;
    uint8_t a2dp_play_stop_media_first:1;
    uint8_t avdtp_cp_enable:1;
    uint8_t bt_source_48k:1;
    uint8_t lhdc_v3:1;
    uint8_t dont_play_mute_when_a2dp_stuck:1;
    // AVRCP
    uint8_t avrcp_tg_enable:1;
    uint8_t avrcp_ct_enable:1;
    // HFP
    uint8_t hfp_ag_enable:1;
    uint8_t hfp_cli_enable:1;
    uint8_t apple_hf_at_support:1;
    uint8_t hf_dont_support_cli_feature:1;
    uint8_t hf_dont_support_enhanced_call:1;
    uint8_t hf_dont_support_3way_call:1;
    uint8_t hfp_support_lc3_swb_en:1;
    uint8_t hf_support_hf_ind_feature:1;
    uint8_t hfp_ag_pts_enable:1;
    uint8_t hfp_ag_pts_ecs_01:1;
    uint8_t hfp_ag_pts_ecs_02:1;
    uint8_t hfp_ag_pts_ecc:1;
    uint8_t hfp_hf_pts_acs_bv_09_i:1;
    // HSP
    uint8_t hsp_enable:1;
    // HID
    uint8_t bt_hid_cod_enable:1;
    // BLE
    uint8_t le_audio_enabled:1;
    // HCI
    uint8_t hci_buff_trace_enable:1;
    uint8_t hci_buffer_trace_high_enable:1;
    uint8_t hci_trace_enable:1;
    uint8_t hci_a2dp_stream_trace_enable:1;
    uint8_t hci_acl_packet_trace_enable:1;
    uint8_t hci_iso_packet_trace_enable:1;
    // HFP or A2DP
    bool    need_handle_specially:1;
    uint8_t handle_code;
    bool dont_send_cnum_cops_cmee_at_cmd:1;
}bthost_cfg_t;

#ifdef __cplusplus
}
#endif
#endif /* BESBT_H */
