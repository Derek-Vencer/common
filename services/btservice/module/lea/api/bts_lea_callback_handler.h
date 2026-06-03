/**
 * @brief Bluetooth Service Low Energy Audio Application Programming Interface
 *
 * @copyright Copyright (c) 2015-20223 BES Technic.
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
 */
#ifndef __BTS_LEA_CALLBACK_HANDLER_H__
#define __BTS_LEA_CALLBACK_HANDLER_H__

/*****************************header include********************************/
#include <stdint.h>
#include <stdbool.h>

/******************************macro defination*****************************/

/******************************type defination******************************/

typedef struct
{
    void (*ble_audio_adv_state_changed)(AOB_ADV_STATE_T state, uint8_t err_code);
    void (*mob_acl_state_changed)(uint8_t conidx, const ble_bdaddr_t* addr, AOB_ACL_STATE_T state, uint8_t errCode);
    void (*vol_changed_cb)(uint8_t con_lid, uint8_t volume, uint8_t mute);
    void (*vocs_offset_changed_cb)(int16_t offset, uint8_t output_lid);
    void (*vocs_bond_data_changed_cb)(uint8_t output_lid, uint8_t cli_cfg_bf);
    void (*media_track_change_cb)(uint8_t con_lid);
    void (*media_stream_status_change_cb)(uint8_t con_lid, uint8_t ase_lid, AOB_MGR_STREAM_STATE_E state);
    void (*media_playback_status_change_cb)(uint8_t con_lid, AOB_MGR_PLAYBACK_STATE_E state);
    void (*media_mic_state_cb)(uint8_t mute);
    void (*media_iso_link_quality_cb)(AOB_ISO_LINK_QUALITY_INFO_T *param);
    void (*media_pacs_cccd_written_cb)(uint8_t con_lid);
    void (*call_state_change_cb)(uint8_t con_lid, void *param);
    void (*call_srv_signal_strength_value_ind_cb)(uint8_t con_lid, uint8_t value);
    void (*call_status_flags_ind_cb)(uint8_t con_lid, bool inband_ring, bool silent_mode);
    void (*call_ccp_opt_supported_opcode_ind_cb)(uint8_t con_lid, bool local_hold_op_supported, bool join_op_supported);
    void (*call_terminate_reason_ind_cb)(uint8_t con_lid, uint8_t call_id, uint8_t reason);
    void (*call_incoming_number_inf_ind_cb)(uint8_t con_lid, uint8_t url_len, uint8_t *url);
    void (*call_svc_changed_ind_cb)(uint8_t con_lid);
    void (*call_action_result_ind_cb)(uint8_t con_lid, void *param);
    void (*ble_bis_sink_status_cb)(uint8_t grp_lid, uint8_t state, uint32_t stream_pos_bf);
    void (*ble_bis_sink_stream_status_cb)(uint8_t grp_lid, bool status);
    void (*ble_bis_deleg_source_add_ri_cb)(uint8_t src_lid, uint8_t con_lid, uint8_t pa_sync_req);
}bts_ble_audio_event_cb_t;

/****************************function declearation**************************/

/**
 ****************************************************************************************
 * @brief Register custom ble audio callback
 *
 * @param[in] cb                    Custom LEA callback
 ****************************************************************************************
 */
void bts_lea_register_audio_event_callback(const bts_ble_audio_event_cb_t* cb);

void bts_lea_register_ui_cb(int (*cb)(void *evt));

void bts_lea_register_gfps_audio_event_callback(void (*cb)(void *evt));

void app_custom_ui_lea_ctx_init();

#endif //__BTS_LEA_CALLBACK_HANDLER_H__