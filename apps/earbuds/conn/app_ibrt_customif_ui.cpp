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
#include "cmsis_os.h"
#include <string.h>
#include "apps.h"
#include "hal_trace.h"
#include "app_ibrt_internal.h"
#include "app_ibrt_customif_ui.h"
#include "bluetooth_bt_api.h"
#include "me_api.h"
#include "app_vendor_cmd_evt.h"
#include "besaud_api.h"
#include "app_battery.h"
#include "app_tws_ibrt_cmd_handler.h"
#include "bts_core_if.h"
#include "app_hfp.h"
#include "app_a2dp.h"
#include "app_bt_media_manager.h"
#include "app_dip.h"
#include "app_bt.h"
#include "audio_policy.h"
#include "app_ibrt_configure.h"
#include "app_ui_param_config.h"
#include "bts_module_if.h"
#if defined(IBRT_UI)
#include "app_ui_tws_fsm.h"
#endif
#include "dip_api.h"
#include "app_ibrt_debug.h"
#include "earbud_profiles_api.h"
#include "earbud_ux_api.h"
#ifdef MEDIA_PLAYER_SUPPORT
#include "app_media_player.h"
#endif
#include "ble_core_common.h"
#include "bes_gap_api.h"

#if defined(SNDP_VAD_ENABLE)
#include "mcu_sensor_hub_app_soundplus.h"
#endif

#ifdef __BIXBY
#include "app_bixby_thirdparty_if.h"
#endif

#ifdef IBRT_UI
#include "app_ui_api.h"
#include "app_bt.h"

#ifdef GFPS_ENABLED
#include "gfps.h"
#endif

#ifdef __AI_VOICE__
#include "ai_spp.h"
#endif

#ifdef BESUI_APP_EN
#ifdef TOTA_v2
#include "app_tota_general.h"
#endif
#endif

#ifdef FINDMY_ENABLED
#include "findmy.h"
#endif

#if (BLE_AUDIO_ENABLED)
#include "bt_svc_lea_api.h"
#endif

#if defined(BESUI_TWS_EN) || defined(RSSI_SWITCH_ROLE_EN)
#include "besui_common.h"
#include "twsui_comm.h"
#endif
#ifdef BESUI_BTMSG_EN
#include "twsui_btmsg.h"
#endif
#if defined(BESUI_APP_EN) && defined(USER_TOTA_SPP_SYNC_KEY_EN)
#include "app_tota_conn.h"
#endif

extern void app_ibrt_customif_cmd_sync_battery_level(uint8_t current_level);

extern ibrt_link_status_changed_cb_t* ibrt_link_status_changed_client_cb;
extern ibrt_mgr_status_changed_cb_t *ibrt_mgr_status_changed_client_cb;
extern ibrt_ext_conn_policy_cb_t *ibrt_ext_conn_policy_client_cb;

uint32_t app_ibrt_customif_set_profile_delaytime_on_spp_connect(const uint8_t *uuid_data_ptr, uint8_t uuid_len);
extern void ntt_master_sync_all_user_settings_to_peer(void);

static uint8_t g_device_id_need_resume_sco = BT_DEVICE_INVALID_ID;

void app_ibrt_customif_ui_vender_event_handler_ind(uint8_t evt_type, uint8_t *buffer, uint8_t length)
{
    uint8_t subcode = evt_type;

    switch (subcode)
    {
        case HCI_DBG_SNIFFER_INIT_CMP_EVT_SUBCODE:
            break;

        case HCI_DBG_IBRT_CONNECTED_EVT_SUBCODE:
            break;

        case HCI_DBG_IBRT_DISCONNECTED_EVT_SUBCODE:
            break;

        case HCI_DBG_IBRT_SWITCH_COMPLETE_EVT_SUBCODE:
            break;

        case HCI_NOTIFY_CURRENT_ADDR_EVT_CODE:
            break;

        case HCI_DBG_TRACE_WARNING_EVT_CODE:
            break;

        case HCI_SCO_SNIFFER_STATUS_EVT_CODE:
            break;

        case HCI_DBG_RX_SEQ_ERROR_EVT_SUBCODE:
            break;

        case HCI_LL_MONITOR_EVT_CODE:
#if defined(__CONNECTIVITY_LOG_REPORT__)
            app_ibrt_if_update_link_monitor_info(&buffer[3]);
#endif
            break;

        case HCI_GET_TWS_SLAVE_MOBILE_RSSI_CODE:
            break;

        default:
            break;
    }
}

/*****************************************************************************
 Prototype    : app_ibrt_customif_pairing_mode_entry
 Description  : indicate custom ui TWS pairing state entry
 Input        : void
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History        :
 Date         : 2019/3/15
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/
void app_ibrt_customif_pairing_mode_entry()
{
    app_ui_config_t* p_app_ui_config = app_ui_get_config();
    uint8_t select_sco_device = app_bt_audio_get_curr_playing_sco();
    uint8_t select_a2dp_device = app_bt_audio_get_curr_playing_a2dp();
    struct BT_DEVICE_T *curr_device = NULL;

    EARBUDS_TRACE(0,"custom_ui pairing mode entry: disc_sco_during_paring %d", p_app_ui_config->pairing_with_disc_hf_cfg);

#ifdef BESUI_BTMSG_EN
    besui_bt_msg_put(ENTER_PAIRMODE_EVENT, 0xff, BT_DEVICE_NUM);
#endif
    if (p_app_ui_config->pairing_with_disc_hf_cfg == IBRT_PAIRING_DISC_SCO && select_sco_device != BT_DEVICE_INVALID_ID)
    {
        g_device_id_need_resume_sco = select_sco_device;

        for (int i = 0; i < BT_DEVICE_NUM; ++i)
        {
            curr_device = app_bt_get_device(i);
            if (curr_device->hf_audio_state == BT_HFP_AUDIO_CON)
            {
                app_ibrt_if_hf_disc_audio_link(i);
            }
        }
    } else if (p_app_ui_config->pairing_with_disc_hf_cfg == IBRT_PAIRING_HF_HUNGUP) {
        for (int i = 0; i < BT_DEVICE_NUM; ++i)
        {
            curr_device = app_bt_get_device(i);
            if (curr_device->hf_audio_state == BT_HFP_AUDIO_CON)
            {
                app_ibrt_if_hf_call_hangup(i);
            }
        }
    }

    if (p_app_ui_config->pairing_with_pause_music && select_a2dp_device != BT_DEVICE_INVALID_ID ) {
        for (int i = 0; i < BT_DEVICE_NUM; ++i)
        {
            curr_device = app_bt_get_device(i);
            if (curr_device->a2dp_streamming)
            {
                app_ibrt_if_a2dp_send_pause(i);
            }
        }
    }

#if !defined(BESUI_TWS_EN) && !defined(BESUI_STEREO_EN)
#ifdef GFPS_ENABLED
    gfps_enter_fastpairing_mode();
#endif
#endif

    if (ibrt_mgr_status_changed_client_cb && ibrt_mgr_status_changed_client_cb->ibrt_mgr_pairing_mode_entry_hook) {
        ibrt_mgr_status_changed_client_cb->ibrt_mgr_pairing_mode_entry_hook();
    }
}

/*****************************************************************************
 Prototype    : app_ibrt_customif_pairing_mode_exit
 Description  : indicate custom ui TWS pairing state exit
 Input        : void
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History        :
 Date         : 2019/3/15
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/
void app_ibrt_customif_pairing_mode_exit()
{
    app_ui_config_t* p_app_ui_config = app_ui_get_config();
    uint8_t resume_sco_device = g_device_id_need_resume_sco;

    EARBUDS_TRACE(0,"custom_ui pairing mode exit: resume_sco_device %x", resume_sco_device);

    if (p_app_ui_config->pairing_with_disc_hf_cfg == IBRT_PAIRING_DISC_SCO && resume_sco_device != BT_DEVICE_INVALID_ID)
    {
        app_ibrt_if_hf_create_audio_link(resume_sco_device);
    }

    g_device_id_need_resume_sco = BT_DEVICE_INVALID_ID;
    if (ibrt_mgr_status_changed_client_cb && ibrt_mgr_status_changed_client_cb->ibrt_mgr_pairing_mode_exit_hook) {
        ibrt_mgr_status_changed_client_cb->ibrt_mgr_pairing_mode_exit_hook();
    }
}

void app_ibrt_customif_pairing_mode_timeout_pre_callback()
{
    EARBUDS_TRACE(0,"custom_ui pairing timeout pre");
    if (ibrt_mgr_status_changed_client_cb && ibrt_mgr_status_changed_client_cb->ibrt_mgr_pairing_mode_timeout_pre_hook) {
        ibrt_mgr_status_changed_client_cb->ibrt_mgr_pairing_mode_timeout_pre_hook();
    }
}

void app_ibrt_customif_pairing_mode_timeout_post_callback()
{
    EARBUDS_TRACE(0,"custom_ui pairing timeout post");
    if (ibrt_mgr_status_changed_client_cb && ibrt_mgr_status_changed_client_cb->ibrt_mgr_pairing_mode_timeout_post_hook) {
        ibrt_mgr_status_changed_client_cb->ibrt_mgr_pairing_mode_timeout_post_hook();
    }
}

void app_ibrt_customif_global_state_callback(ibrt_global_state_change_event *state)
{
    EARBUDS_TRACE(0,"custom_ui global_status changed = %d", state->state);

    switch (state->state)
    {
        case IBRT_BLUETOOTH_ENABLED:
            break;
        case IBRT_BLUETOOTH_DISABLED:
            break;
        default:
            break;
    }
    if (ibrt_link_status_changed_client_cb && ibrt_link_status_changed_client_cb->ibrt_global_state_changed) {
        ibrt_link_status_changed_client_cb->ibrt_global_state_changed(state);
    }
}


static void _close_some_chnls_when_both_a2dp_hfp_closed(const bt_bdaddr_t *addr, uint8_t device_id)
{
#ifdef __IAG_BLE_INCLUDE__
    if (app_bt_is_a2dp_disconnected(device_id) &&
#ifdef BT_HFP_SUPPORT
        app_bt_is_hfp_disconnected(device_id) &&
#endif
        app_bt_is_profile_connected_before(device_id)) {

#if defined(__GATT_OVER_BR_EDR__)
        if (app_btgatt_over_br_edr_enabled() &&
            app_btgatt_is_connected(device_id)) {
            app_btgatt_disconnect(device_id);
        }
#endif

#ifdef __AI_VOICE__
        ai_spp_enumerate_disconnect_service((uint8_t *)addr, device_id);
#endif
#ifdef GFPS_ENABLED
        gfps_disconnect(SET_BT_ID(device_id));
#endif
    }
#endif
}

#ifdef BESUI_BTMSG_EN
void besui_a2dp_state_event(const bt_bdaddr_t* addr, ibrt_conn_a2dp_state_change *state)
{
    EARBUDS_TRACE(0,"%s, a2dp_state = %d", __func__, state->a2dp_state);
    if(state->a2dp_state == IBRT_CONN_A2DP_OPEN)
    {
        nv_record_all_ddbrec_print();
        if(!app_bt_get_mobile_connected_status(state->device_id))
        {
            app_bt_set_mobile_connected_status(state->device_id, true);
            app_bt_set_mobile_connected_info(state->device_id, (uint8_t*)addr);
            besui_bt_msg_put(PHONE_CONNECTED_EVENT, 0xff, state->device_id);

            uint8_t device_id = app_bt_compare_openreconnect_addr((uint8_t*)addr);
            if(device_id < BT_DEVICE_NUM)
            {
                app_bt_mobile_set_openreconnect_info(device_id, CONNECT_DEFAULT, false);
            }

            device_id = app_bt_compare_reconnect_addr((uint8_t*)addr);
            if(device_id < BT_DEVICE_NUM)
            {
                app_bt_mobile_clear_reconnect_info(device_id);
            }

            if(besui_get_profile_conn_num() >= BT_DEVICE_NUM)
            {
                EARBUDS_TRACE(0,"connected two device, need clear reconnect flag");
                app_bt_mobile_set_openreconnect_info(device_id, CONNECT_DEFAULT, true);
                app_bt_mobile_clear_reconnect_info(0);
                app_bt_mobile_clear_reconnect_info(1);
            }
        }
    }
    else if(state->a2dp_state == IBRT_CONN_A2DP_SUSPENED)
    {
#ifdef BESUI_APP_EN
#if defined(VOICE_ASSIST_CUSTOM_LEAK_DETECT)
#ifdef TOTA_v2
        if(app_tota_get_detect_flag())
        {
            app_tota_start_leak_detect();
            app_tota_set_leak_detect_value();
        }
#endif
#endif
#endif
    }
}

void besui_hfp_state_event(const bt_bdaddr_t* addr, ibrt_conn_hfp_state_change *state)
{
    EARBUDS_TRACE(0,"%s, hfp_state = %d", __func__, state->hfp_state);
    if(!app_bt_get_mobile_connected_status(state->device_id))
    {
        app_bt_set_mobile_connected_status(state->device_id, true);
        app_bt_set_mobile_connected_info(state->device_id, (uint8_t*)addr);
        besui_bt_msg_put(PHONE_CONNECTED_EVENT, 0xff, state->device_id);

        uint8_t device_id = app_bt_compare_openreconnect_addr((uint8_t*)addr);
        if(device_id < BT_DEVICE_NUM)
        {
            app_bt_mobile_set_openreconnect_info(device_id, CONNECT_DEFAULT, false);
        }

        device_id = app_bt_compare_reconnect_addr((uint8_t*)addr);
        if(device_id < BT_DEVICE_NUM)
        {
            app_bt_mobile_clear_reconnect_info(device_id);
        }

        if(besui_get_profile_conn_num() >= BT_DEVICE_NUM)
        {
            EARBUDS_TRACE(0,"connected two device, need clear reconnect flag");
            app_bt_mobile_set_openreconnect_info(device_id, CONNECT_DEFAULT, true);
            app_bt_mobile_clear_reconnect_info(0);
            app_bt_mobile_clear_reconnect_info(1);
        }
    }
}
#endif

void app_ibrt_customif_a2dp_callback(const bt_bdaddr_t* addr, ibrt_conn_a2dp_state_change *state)
{
    if (addr != NULL)
    {
        EARBUDS_TRACE(0,"(d%x) custom_ui a2dp_status changed = %d addr:%02x:%02x:*:*:*:%02x",
            state->device_id, state->a2dp_state, addr->address[0], addr->address[1], addr->address[5]);
    }
    else
    {
        EARBUDS_TRACE(0,"(d%x) custom_ui a2dp_status changed = %d addr is NULL",
            state->device_id, state->a2dp_state);
    }

    if (state->a2dp_state == IBRT_CONN_A2DP_IDLE || state->a2dp_state == IBRT_CONN_A2DP_CLOSED)
    {
        _close_some_chnls_when_both_a2dp_hfp_closed(addr, state->device_id);
    }

#ifdef BESUI_BTMSG_EN
    besui_a2dp_state_event(addr, state);
#endif
    switch(state->a2dp_state)
    {
        case IBRT_CONN_A2DP_IDLE:
            break;
        case IBRT_CONN_A2DP_OPEN:
            if(bts_bt_if_is_dev_link_connected(addr))
            {
                app_bt_get_remote_device_name(addr);
            }
            break;
        case IBRT_CONN_A2DP_CODEC_CONFIGURED:
            EARBUDS_TRACE(0,"custom_ui delay report support %d", state->delay_report_support);
            break;
        case IBRT_CONN_A2DP_STREAMING:
            if (bts_ibrt_if_is_ibrt_link_connected(addr)) {
                bt_drv_reg_op_afh_assess_en(false);
            } else {
                bt_drv_reg_op_afh_assess_en(true);
            }
            //just add Qos setting interface for music state, if need, uncomment it.
            //app_ibrt_if_update_mobile_link_qos(state->device_id, 40);
            break;
        case IBRT_CONN_A2DP_SUSPENED:
            break;
        case IBRT_CONN_A2DP_CLOSED:
            if (app_bt_audio_count_streaming_a2dp() == 0) {
                bt_drv_reg_op_afh_assess_en(false);
            }
            break;
        default:
            break;
    }
    if (ibrt_link_status_changed_client_cb && ibrt_link_status_changed_client_cb->ibrt_a2dp_state_changed) {
        ibrt_link_status_changed_client_cb->ibrt_a2dp_state_changed(addr, state);
    }
}

void app_ibrt_customif_hfp_callback(const bt_bdaddr_t* addr, ibrt_conn_hfp_state_change *state)
{
#ifdef BT_HFP_SUPPORT
    if (addr != NULL)
    {
        EARBUDS_TRACE(0,"(d%x) custom_ui hfp_status changed = %d addr:%02x:%02x:*:*:*:%02x",
            state->device_id, state->hfp_state, addr->address[0], addr->address[1], addr->address[5]);
    }
    else
    {
        EARBUDS_TRACE(0,"(d%x) custom_ui hfp_status changed = %d addr is NULL",
            state->device_id, state->hfp_state);
    }
#ifdef BESUI_BTMSG_EN
    besui_hfp_state_event(addr, state);
#endif
    switch(state->hfp_state)
    {
        case IBRT_CONN_HFP_SLC_DISCONNECTED:
            _close_some_chnls_when_both_a2dp_hfp_closed(addr, state->device_id);
            break;
        case IBRT_CONN_HFP_SLC_OPEN:
            break;
        case IBRT_CONN_HFP_SCO_OPEN:
            break;
        case IBRT_CONN_HFP_SCO_CLOSED:
            break;

        case IBRT_CONN_HFP_RING_IND:
            break;

        case IBRT_CONN_HFP_CALL_IND:
            break;

        case IBRT_CONN_HFP_CALLSETUP_IND:
            break;

        case IBRT_CONN_HFP_CALLHELD_IND:
            break;

        case IBRT_CONN_HFP_CIEV_SERVICE_IND:
            break;

        case IBRT_CONN_HFP_CIEV_SIGNAL_IND:
            break;

        case IBRT_CONN_HFP_CIEV_ROAM_IND:
            break;

        case IBRT_CONN_HFP_CIEV_BATTCHG_IND:
            break;

        case IBRT_CONN_HFP_SPK_VOLUME_IND:
            break;

        case IBRT_CONN_HFP_MIC_VOLUME_IND:
            break;

        case IBRT_CONN_HFP_IN_BAND_RING_IND:
            EARBUDS_TRACE(0,"%s is in-band ring %d",__func__, state->in_band_ring_enable);
            app_ibrt_if_BSIR_command_event(state->in_band_ring_enable);
            break;

        case IBRT_CONN_HFP_VR_STATE_IND:
            break;

        case IBRT_CONN_HFP_AT_CMD_COMPLETE:
            break;

        default:
            break;
    }
    if (ibrt_link_status_changed_client_cb && ibrt_link_status_changed_client_cb->ibrt_hfp_state_changed) {
        ibrt_link_status_changed_client_cb->ibrt_hfp_state_changed(addr, state);
    }
#endif
}

void app_ibrt_customif_avrcp_callback(const bt_bdaddr_t* addr, ibrt_conn_avrcp_state_change *state)
{
    if (addr != NULL)
    {
        EARBUDS_TRACE(0,"(d%x) custom_ui avrcp_status changed = %d addr:%02x:%02x:*:*:*:%02x",
            state->device_id, state->avrcp_state, addr->address[0], addr->address[1], addr->address[5]);
    }
    else
    {
        EARBUDS_TRACE(0,"(d%x) custom_ui avrcp_status changed = %d addr is NULL",
            state->device_id, state->avrcp_state);
    }

    switch(state->avrcp_state)
    {
        case IBRT_CONN_AVRCP_DISCONNECTED:
            break;
        case IBRT_CONN_AVRCP_CONNECTED:
            break;
        case IBRT_CONN_AVRCP_VOLUME_UPDATED:
            EARBUDS_TRACE(0,"custom_ui volume %d %d%%", state->volume, state->volume*100/128);
            break;
        case IBRT_CONN_AVRCP_REMOTE_CT_0104:
            EARBUDS_TRACE(0,"custom_ui remote ct 1.4 support %d volume %d", state->support, state->volume);
            break;
        case IBRT_CONN_AVRCP_REMOTE_SUPPORT_PLAYBACK_STATUS_CHANGED_EVENT:
            EARBUDS_TRACE(0,"custom_ui playback status support %d", state->support);
            break;
        case IBRT_CONN_AVRCP_PLAYBACK_STATUS_CHANGED:
            EARBUDS_TRACE(0,"custom_ui playback status %d", state->playback_status);
            break;
        case IBRT_CONN_AVRCP_PLAY_STATUS_CHANGED:
            EARBUDS_TRACE(0,"custom_ui playStatus=%d, pos/totle=[%d/%d]", state->playback_status, state->play_position, state->play_length);
            break;
        case IBRT_CONN_AVRCP_PLAY_POS_CHANGED:
            EARBUDS_TRACE(0,"custom_ui playPos=%d", state->play_position);
        default:
            break;
    }
    if (ibrt_link_status_changed_client_cb && ibrt_link_status_changed_client_cb->ibrt_avrcp_state_changed) {
        ibrt_link_status_changed_client_cb->ibrt_avrcp_state_changed(addr, state);
    }
}

void app_ibrt_customif_avrcp_media_info_callback(const bt_bdaddr_t *addr, const avrcp_adv_rsp_parms_t *mediainforsp)
{
    EARBUDS_TRACE(0,"custom_ui [%02x:%02x:*:*:*:%02x] Media info update.",
          addr->address[0], addr->address[1], addr->address[5]);

    uint8_t num_of_elements = mediainforsp->element.numIds;
    for (int i = 0; i < num_of_elements && i < BTIF_AVRCP_NUM_MEDIA_ATTRIBUTES; i++) {
        if (mediainforsp->element.txt[i].length > 0) {
            EARBUDS_TRACE(0,"custom_ui MediaInfo:[%d] %s %d: %s", i,
                  avrcp_get_track_element_name(mediainforsp->element.txt[i].attrId),
                  mediainforsp->element.txt[i].length, mediainforsp->element.txt[i].string);
        }
    }
}

void app_ibrt_customif_tws_on_paring_state_changed(ibrt_conn_pairing_state state,uint8_t reason_code)
{
    POSSIBLY_UNUSED TWS_UI_ROLE_E ui_role = bts_core_get_ui_role();

    EARBUDS_TRACE(0,"custom_ui tws pairing_state changed = %d with reason 0x%x,role=%d",state,reason_code, ui_role);

    switch(state)
    {
        case IBRT_CONN_PAIRING_IDLE:
            break;
        case IBRT_CONN_PAIRING_IN_PROGRESS:
            break;
        case IBRT_CONN_PAIRING_COMPLETE:
#ifdef MEDIA_PLAYER_SUPPORT
#ifndef BESUI_TWS_EN
            if (app_ibrt_if_is_ui_slave() && (bes_bt_tws_besaud_is_connected()))
            {
                //media_PlayAudio(AUD_ID_BT_PAIRING_SUC, 0);
            }
#endif
#endif
            break;
        default:
            break;
    }
    if (ibrt_link_status_changed_client_cb && ibrt_link_status_changed_client_cb->ibrt_tws_pairing_changed) {
        ibrt_link_status_changed_client_cb->ibrt_tws_pairing_changed(state, reason_code);
    }
}

#ifdef BESUI_BTMSG_EN
void besui_tws_state_event(ibrt_conn_tws_conn_state_event *state, uint8_t reason_code)
{
	EARBUDS_TRACE(0, "[UICONN]%s, state = %d, reason_code = %d", __func__, state->state.acl_state, reason_code);
    if(state->state.acl_state == IBRT_CONN_ACL_DISCONNECTED
    || state->state.acl_state == IBRT_CONN_ACL_PROFILES_CONNECTED)
    {
        besui_bt_msg_put(TWS_STATUS_EVENT, 0xff, 0);        
    }

    switch (state->state.acl_state)
    {
        case IBRT_CONN_ACL_DISCONNECTED:
            besui_bt_msg_put(TWS_DISCONNECTED_EVENT, reason_code, BT_DEVICE_NUM);
            break;
        case IBRT_CONN_ACL_PROFILES_CONNECTED:
#ifdef USER_TOTA_SPP_SYNC_KEY_EN
            if(app_ibrt_if_is_ui_master())
                tota_spp_aeskey_to_slave();
#endif
            besui_bt_msg_put(TWS_CONNECTED_EVENT, 0xff, BT_DEVICE_NUM);
            break;
        default:
            break;
    }
}
#endif

#ifdef IBRT
#define NTT_USER_SETTING_SYNC_DELAY_MS    2000

static osTimerId ntt_user_setting_sync_timer = NULL;
static bool ntt_user_setting_synced_once = false;

static void ntt_user_setting_sync_timer_handler(void const *param)
{
    if (ntt_user_setting_synced_once)
    {
        EARBUDS_TRACE(0, "[NTT_USER_SYNC] skip, already synced once");
        return;
    }

    if (!app_ibrt_middleware_is_ui_slave())
    {
        EARBUDS_TRACE(0, "[NTT_USER_SYNC] delayed sync user settings");

        ntt_user_setting_synced_once = true;
        ntt_master_sync_all_user_settings_to_peer();
    }
    else
    {
        EARBUDS_TRACE(0, "[NTT_USER_SYNC] skip, role is slave");
    }
}

osTimerDef(NTT_USER_SETTING_SYNC_TIMER,
           ntt_user_setting_sync_timer_handler);

static void ntt_user_setting_sync_delay_start(void)
{
    if (ntt_user_setting_synced_once)
    {
        EARBUDS_TRACE(0, "[NTT_USER_SYNC] skip start, already synced once");
        return;
    }

    if (ntt_user_setting_sync_timer == NULL)
    {
        ntt_user_setting_sync_timer =
            osTimerCreate(osTimer(NTT_USER_SETTING_SYNC_TIMER),
                          osTimerOnce,
                          NULL);
    }

    if (ntt_user_setting_sync_timer == NULL)
    {
        EARBUDS_TRACE(0, "[NTT_USER_SYNC] delay timer create failed");
        return;
    }

    osTimerStop(ntt_user_setting_sync_timer);
    osTimerStart(ntt_user_setting_sync_timer,
                 NTT_USER_SETTING_SYNC_DELAY_MS);

    EARBUDS_TRACE(0,
        "[NTT_USER_SYNC] delay start %d ms",
        NTT_USER_SETTING_SYNC_DELAY_MS);
}
#endif

void app_ibrt_customif_tws_on_acl_state_changed(ibrt_conn_tws_conn_state_event *state, uint8_t reason_code)
{
    EARBUDS_TRACE(0,"custom_ui tws acl state changed = %d with reason 0x%x role %d", state->state.acl_state, reason_code, state->current_role);

#ifdef BESUI_BTMSG_EN
    besui_tws_state_event(state, reason_code);
#endif

#if defined(A2DP_AUDIO_STEREO_MIX_CTRL)
    if (state->state.acl_state == IBRT_CONN_ACL_CONNECTED) {
        a2dp_audio_stereo_set_mix(false);
    } else if (state->state.acl_state == IBRT_CONN_ACL_DISCONNECTED) {
        a2dp_audio_stereo_set_mix(true);
    }
#endif

    switch (state->state.acl_state)
    {
        case IBRT_CONN_ACL_CONNECTED:
            break;
        case IBRT_CONN_ACL_PROFILES_CONNECTED:
#ifdef IBRT
        if (!app_ibrt_middleware_is_ui_slave())
        {
            EARBUDS_TRACE(0,
                "[NTT_PROFILE_SYNC] TWS profiles connected, skip early profile sync test");
            ntt_user_setting_sync_delay_start();
        }
#endif
        break;
        case IBRT_CONN_ACL_DISCONNECTED:
            break;
        case IBRT_CONN_ACL_CONNECTING_CANCELED:
            break;
        case IBRT_CONN_ACL_CONNECTING_FAILURE:
            break;
        default:
            break;
    }
    if (ibrt_link_status_changed_client_cb && ibrt_link_status_changed_client_cb->ibrt_tws_acl_state_changed) {
        ibrt_link_status_changed_client_cb->ibrt_tws_acl_state_changed(state, reason_code);
    }
}

#ifdef BESUI_BTMSG_EN
void besui_mobile_state_event(const bt_bdaddr_t *addr, ibrt_mobile_conn_state_event *state, uint8_t reason_code)
{
	EARBUDS_TRACE(0, "[UICONN]%s, state = %d, reason_code = %d", __func__, state->state.acl_state, reason_code);
    uint8_t device_id = BT_DEVICE_NUM;
    uint8_t reconnect_device_id = BT_DEVICE_NUM;

    switch (state->state.acl_state)
    {
        case IBRT_CONN_ACL_DISCONNECTED:
            uictl.auth_start_flag = 0;
            if(reason_code == 0x08)
            {
                device_id = app_bt_compare_connect_addr((uint8_t*)addr);
                if(device_id < BT_DEVICE_NUM)
                {
                    app_bt_clear_mobile_connect_info(device_id);
                    device_id = app_bt_compare_openreconnect_addr((uint8_t*)addr);
                    if(device_id < BT_DEVICE_NUM) //may be disconnected addr is reconnect addr.so clear status
                        app_bt_mobile_clear_openreconnect_info(device_id, false);
                    besui_bt_msg_put(PHONE_DISCONECTED_EVENT, reason_code, device_id);
                }

                device_id = app_bt_compare_reconnect_addr((uint8_t*)addr);
                if(device_id >= BT_DEVICE_NUM)
                {
                    if(app_bt_get_reconnected_type() == 3) //two device reconnecting
                    {}
                    else if(app_bt_get_reconnected_type() == 2) //device id 1 is reconnecting
                        app_bt_mobile_set_reconnect_info(0, (uint8_t*)addr);
                    else if(app_bt_get_reconnected_type() == 1) //device id 0 is reconnecting
                        app_bt_mobile_set_reconnect_info(1, (uint8_t*)addr);
                    else if(app_bt_get_reconnected_type() == 0)  //no reconnecting
                        app_bt_mobile_set_reconnect_info(0, (uint8_t*)addr);
                }
            }
            else
            {
                device_id = app_bt_compare_connect_addr((uint8_t*)addr);
                EARBUDS_TRACE(0, "[UIBT]%s, connect device_id=%d", __func__, device_id);
                if(device_id < BT_DEVICE_NUM)
                {
                    app_bt_clear_mobile_connect_info(device_id);
                    device_id = app_bt_compare_openreconnect_addr((uint8_t*)addr);
                    EARBUDS_TRACE(0, "[UIBT]%s, openreconnect device_id=%d", __func__, device_id);
                    if(device_id < BT_DEVICE_NUM) //may be disconnected addr is reconnect addr.so clear status
                        app_bt_mobile_clear_openreconnect_info(device_id, false);
                    besui_bt_msg_put(PHONE_DISCONECTED_EVENT, reason_code, device_id);
                }
                else
                {
                    device_id = app_bt_compare_openreconnect_addr((uint8_t*)addr);
                    if(device_id < BT_DEVICE_NUM)
                        besui_bt_msg_put(PHONE_DISCONECTED_EVENT, reason_code, device_id);
                    //else
                        //besui_bt_msg_put(PHONE_DISCONECTED_EVENT, reason_code, BT_DEVICE_NUM);
                }
            }
#ifdef BESUI_PROMPT_ISSUE_EN
            prompt_set_flag(false);
#endif
            break;
        case IBRT_CONN_ACL_CONNECTING_CANCELED:
            uictl.auth_start_flag = 0;
            break;
        case IBRT_CONN_ACL_CONNECTING_FAILURE:
            uictl.auth_start_flag = 0;            
            reconnect_device_id = app_bt_compare_openreconnect_addr((uint8_t*)addr);
            if(reconnect_device_id < BT_DEVICE_NUM)
                besui_bt_msg_put(OPENRECONNECTED_TIMEOUT_ENTER_PAIRMODE_EVENT, 0xff, reconnect_device_id);
            else
            {
                reconnect_device_id = app_bt_compare_reconnect_addr((uint8_t*)addr);
                if(reconnect_device_id < BT_DEVICE_NUM)
                    besui_bt_msg_put(RECONNECTED_TIMEOUT_ENTER_PAIRMODE_EVENT, 0xff, reconnect_device_id);
            }
            break;
        case IBRT_CONN_ACL_CONNECTED:
            uictl.auth_start_flag = 0;
#ifdef BESUI_PROMPT_ISSUE_EN
            prompt_issue_timer_onoff(true, 1000); //master
#endif
            break;
        case IBRT_CONN_ACL_AUTH_COMPLETE:
            uictl.auth_start_flag = 0;
            if(reason_code == 0x00)
            {
                besui_bt_msg_put(AUTH_SUCCESS_EVENT, 0xff, reconnect_device_id);
            }
            break;
        default:
            break;
    }
}
#endif

void app_ibrt_customif_on_mobile_acl_state_changed(const bt_bdaddr_t *addr, ibrt_mobile_conn_state_event *state, uint8_t reason_code)
{
    if (addr != NULL)
    {
        EARBUDS_TRACE(0,"(d%x) custom_ui mobile acl state changed = %d reason 0x%x, addr:%02x:%02x:*:*:*:%02x",
            state->device_id, state->state.acl_state, reason_code, addr->address[0], addr->address[1], addr->address[5]);
    }
    else
    {
        EARBUDS_TRACE(0,"(d%x) custom_ui mobile acl state changed = %d reason 0x%x, addr is NULL",
            state->device_id, state->state.acl_state, reason_code);
    }

#ifdef BESUI_BTMSG_EN
    besui_mobile_state_event(addr, state, reason_code);
#endif

#ifdef BT_DIP_SUPPORT
    bt_remver_t rem_ver;
#endif

    switch (state->state.acl_state)
    {
        case IBRT_CONN_ACL_DISCONNECTED:
#if defined(SNDP_VAD_ENABLE)
            if (!app_ibrt_middleware_is_ui_slave())
                app_sensor_hub_sndp_mcu_request_vad_stop();
#endif

#ifdef GFPS_ENABLED
            gfps_link_disconnect_handler(SET_BT_ID(state->device_id), addr, reason_code);
#endif
            break;
        case IBRT_CONN_ACL_CONNECTING:
            break;
        case IBRT_CONN_ACL_CONNECTING_CANCELED:
            break;
        case IBRT_CONN_ACL_CONNECTING_FAILURE:
            break;
        case IBRT_CONN_ACL_CONNECTED:
        {
        #ifdef BT_DIP_SUPPORT
            uint16_t mobile_conhandle = bts_bt_if_get_dev_acl_handle(addr);
            if(mobile_conhandle != 0 && mobile_conhandle != BT_INVALID_CONN_HANDLE)
            {
                rem_ver = btif_me_get_remote_version_by_handle(mobile_conhandle);
                if ((rem_ver.compid == 0xa) && (rem_ver.subvers == 0x2918)) {
                    EARBUDS_TRACE(0,"<Sony S313>: don't send dip request to avoid profile connection failure");
                } else {
                    app_dip_query_remote_info(state->device_id);
                }
            }
        #endif
#if defined(SNDP_VAD_ENABLE)
            if (!app_ibrt_middleware_is_ui_slave())
                app_sensor_hub_sndp_mcu_request_vad_start();
#endif

#ifdef GFPS_ENABLED
            gfps_link_connect_handler(SET_BT_ID(state->device_id), addr);
#endif

#ifdef FINDMY_ENABLED
            findmy_bt_connected_handler((bt_bdaddr_t *)addr);
#endif
        }
            break;
        case IBRT_CONN_ACL_PROFILES_CONNECTED:
            break;
        case IBRT_CONN_ACL_AUTH_COMPLETE:
            break;
        case IBRT_CONN_ACL_DISCONNECTING:
            break;
        case IBRT_CONN_ACL_UNKNOWN:
            break;
        default:
            break;
    }
    if (ibrt_link_status_changed_client_cb && ibrt_link_status_changed_client_cb->ibrt_mobile_acl_state_changed) {
        ibrt_link_status_changed_client_cb->ibrt_mobile_acl_state_changed(addr, state, reason_code);
    }
}

void app_ibrt_customif_sco_state_changed(const bt_bdaddr_t *addr, ibrt_sco_conn_state_event *state, uint8_t reason_code)
{
    EARBUDS_TRACE(0,"custom_ui sco state changed = %d with reason 0x%x", state->state.sco_state, reason_code);

    switch (state->state.sco_state)
    {
        case IBRT_CONN_SCO_CONNECTED:
            break;
        case IBRT_CONN_SCO_DISCONNECTED:
            app_ibrt_if_sco_disconnect((uint8_t *)addr, reason_code);
            break;
        default:
            break;
    }
    if (ibrt_link_status_changed_client_cb && ibrt_link_status_changed_client_cb->ibrt_sco_state_changed) {
        ibrt_link_status_changed_client_cb->ibrt_sco_state_changed(addr, state, reason_code);
    }
}

/*****************************************************************************
 Prototype    : app_ibrt_customif_on_tws_role_changed
 Description  : role state changed callabck
 Input        : bt_bdaddr_t *addr
                ibrt_conn_role_change_state state
                ibrt_role_e role
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History      :
 Date         : 2019/12/02
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/
void app_ibrt_customif_on_tws_role_switch_status_ind(const bt_bdaddr_t *addr,ibrt_conn_role_change_state state,ibrt_role_e role)
{
    if(addr != NULL)
    {
        EARBUDS_TRACE(0,"custom_ui tws role switch changed = %d with role 0x%x addr: %02x:%02x:*:*:*:%02x",
            state, role, addr->address[0], addr->address[1], addr->address[5]);
    }
    else
    {
        EARBUDS_TRACE(0,"custom_ui tws role switch changed = %d with role 0x%x addr is NULL", state, role);
    }

    switch (state)
    {
        case IBRT_CONN_ROLE_SWAP_INITIATED: //this msg will be notified if initiate role switch
            break;
        case IBRT_CONN_ROLE_SWAP_COMPLETE: //role switch complete,both sides will receice this msg
            if (app_bt_audio_count_streaming_a2dp()) {
                if (role == IBRT_MASTER) {
                    bt_drv_reg_op_afh_assess_en(true);
                } else {
                    bt_drv_reg_op_afh_assess_en(false);
                }
            }
            break;
        case IBRT_CONN_ROLE_CHANGE_COMPLETE://nv role changed
            break;
        default:
            break;
    }
    if (ibrt_link_status_changed_client_cb && ibrt_link_status_changed_client_cb->ibrt_tws_role_switch_status_ind) {
        ibrt_link_status_changed_client_cb->ibrt_tws_role_switch_status_ind(addr, state, role);
    }
}

void app_ibrt_customif_on_ibrt_state_changed(const bt_bdaddr_t *addr, ibrt_connection_state_event* state,ibrt_role_e role,uint8_t reason_code)
{
    if (addr != NULL)
    {
        EARBUDS_TRACE(0,"(d%x) custom_ui ibrt state changed = %d with reason 0x%x role=%d addr: %02x:%02x:*:*:*:%02x",
            state->device_id, state->state.ibrt_state, reason_code,role,
            addr->address[0], addr->address[1], addr->address[5]);
    }
    else
    {
        EARBUDS_TRACE(0,"(d%x) custom_ui ibrt state changed = %d with reason 0x%x role=%d addr is NULL",
            state->device_id, state->state.ibrt_state, reason_code,role);
    }

    switch(state->state.ibrt_state)
    {
        case IBRT_CONN_IBRT_DISCONNECTED:
            bts_ibrt_clear_profile_connect_protect(state->device_id, APP_IBRT_HFP_PROFILE_ID);
            bts_ibrt_clear_profile_connect_protect(state->device_id, APP_IBRT_A2DP_PROFILE_ID);
            bts_ibrt_clear_profile_connect_protect(state->device_id, APP_IBRT_AVRCP_PROFILE_ID);
            bts_ibrt_clear_profile_disconnect_protect(state->device_id, APP_IBRT_HFP_PROFILE_ID);
            bts_ibrt_clear_profile_disconnect_protect(state->device_id, APP_IBRT_A2DP_PROFILE_ID);
            bts_ibrt_clear_profile_disconnect_protect(state->device_id, APP_IBRT_AVRCP_PROFILE_ID);
#ifdef BESUI_PROMPT_ISSUE_EN
            prompt_set_flag(false);
#endif
            break;
        case IBRT_CONN_IBRT_CONNECTED:
        {
            uint8_t local_level = app_battery_current_level();

            EARBUDS_TRACE(1,
                "[BAT_SYNC][IBRT_CONNECTED] local=%d role=%d",
                local_level,
                role);

            app_ibrt_customif_cmd_sync_battery_level(local_level);

            if (IBRT_SLAVE == role)
            {
            #ifdef BT_DIP_SUPPORT
                btif_dip_set_state(addr, DIP_CTRL_ST_IDLE);
            #endif
            }
            break;
        }
        case IBRT_CONN_IBRT_START_FAIL:
            break;
        case IBRT_CONN_IBRT_ACL_CONNECTED:
            bts_ibrt_clear_profile_connect_protect(state->device_id, APP_IBRT_HFP_PROFILE_ID);
            bts_ibrt_clear_profile_connect_protect(state->device_id, APP_IBRT_A2DP_PROFILE_ID);
            bts_ibrt_clear_profile_connect_protect(state->device_id, APP_IBRT_AVRCP_PROFILE_ID);
            bts_ibrt_clear_profile_disconnect_protect(state->device_id, APP_IBRT_HFP_PROFILE_ID);
            bts_ibrt_clear_profile_disconnect_protect(state->device_id, APP_IBRT_A2DP_PROFILE_ID);
            bts_ibrt_clear_profile_disconnect_protect(state->device_id, APP_IBRT_AVRCP_PROFILE_ID);
#ifdef BESUI_PROMPT_ISSUE_EN
            prompt_issue_timer_onoff(true, 1000); //slave
#endif
            break;
        default:
            break;
    }
    if (ibrt_link_status_changed_client_cb && ibrt_link_status_changed_client_cb->ibrt_ibrt_state_changed) {
        ibrt_link_status_changed_client_cb->ibrt_ibrt_state_changed(addr, state, role, reason_code);
    }
}

void app_ibrt_customif_on_access_mode_changed(btif_accessible_mode_t newAccessibleMode)
{
    EARBUDS_TRACE(0,"Access mode changed to %d", newAccessibleMode);
    if (ibrt_link_status_changed_client_cb && ibrt_link_status_changed_client_cb->ibrt_access_mode_changed) {
        ibrt_link_status_changed_client_cb->ibrt_access_mode_changed(newAccessibleMode);
    }
#ifdef BESUI_TWS_EN
    if(newAccessibleMode == BTIF_BAM_GENERAL_ACCESSIBLE)
    {
        if(TWS_UI_UNKNOWN == app_ibrt_if_get_ui_role())
        {
            besui_bt_msg_put(BT_SINGLE_PAIRMODE, 0xff, 0);
        }
    }
#endif
}

bool app_ibrt_customif_incoming_conn_req_callback(const bt_bdaddr_t* addr)
{
    if (addr == NULL)
    {
        EARBUDS_TRACE(0,"custom_ui incoming_conn_req addr is NULL, reject");
        return false;
    }
    EARBUDS_TRACE(0,"custom_ui incoming_conn_req addr: %02x:%02x:*:*:*:%02x", addr->address[0], addr->address[1], addr->address[5]);
    if (ibrt_ext_conn_policy_client_cb && ibrt_ext_conn_policy_client_cb->accept_incoming_conn_req_hook) {
        return ibrt_ext_conn_policy_client_cb->accept_incoming_conn_req_hook(addr);
    }
    return true;
}

bool app_ibrt_customif_extra_incoming_conn_req_callback(const bt_bdaddr_t* incoming_addr, bt_bdaddr_t *steal_addr)
{
    if (incoming_addr == NULL)
    {
        EARBUDS_TRACE(0,"custom_ui incoming_conn_req addr is NULL, reject");
        return false;
    }
    EARBUDS_TRACE(0,"custom_ui extra_incoming_conn_req addr: %02x:%02x:*:*:*:%02x",
        incoming_addr->address[0], incoming_addr->address[1], incoming_addr->address[5]);
    if (ibrt_ext_conn_policy_client_cb && ibrt_ext_conn_policy_client_cb->accept_extra_incoming_conn_req_hook) {
        return ibrt_ext_conn_policy_client_cb->accept_extra_incoming_conn_req_hook(incoming_addr, steal_addr);
    }
    return true;
}

bool app_ibrt_customif_custom_disallow_reconnect_mob_callback(const bt_bdaddr_t* addr, const uint16_t active_event)
{
    bool ret = false;
    if (ibrt_ext_conn_policy_client_cb && ibrt_ext_conn_policy_client_cb->disallow_start_reconnect_mob_hook) {
        ret = ibrt_ext_conn_policy_client_cb->disallow_start_reconnect_mob_hook(addr, active_event);
    }
    EARBUDS_TRACE(0,"custom_ui reconnect dev addr: %02x:%02x:*:*:*:%02x, event:0x%02x, ret: %d",
        addr->address[0], addr->address[1], addr->address[5], active_event, ret);
#if defined(BESUI_RECONN_IOS_ISSUE)
    besui_ios_open_reconn_issue();
#endif
    return ret;
}

bool app_ibrt_customif_custom_disallow_reconnect_tws_callback(void)
{
    bool ret = false;
    if (ibrt_ext_conn_policy_client_cb && ibrt_ext_conn_policy_client_cb->disallow_start_reconnect_tws_hook) {
        ret = ibrt_ext_conn_policy_client_cb->disallow_start_reconnect_tws_hook();
    }
    EARBUDS_TRACE(0,"custom_ui reconnect tws: %d", ret);
    return ret;
}

bool app_ibrt_customif_disallow_tws_role_switch_callback()
{
    EARBUDS_TRACE(0,"custom_ui:tws role switch ext-policy");

    /*
     * So procedure may not working well when tws role switch happen, return
     * true to disallow tws role switch here.
     */
    if (ibrt_ext_conn_policy_client_cb && ibrt_ext_conn_policy_client_cb->disallow_tws_role_switch_hook) {
        return ibrt_ext_conn_policy_client_cb->disallow_tws_role_switch_hook();
    }
    return false;
}

/*****************************************************************************
 Prototype    : app_ibrt_customif_case_open_run_complete_callback
 Description  : called after case open event run complete
 Input        : None
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History      :
 Date         : 2019/12/02
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/
void app_ibrt_customif_case_open_run_complete_callback()
{
    EARBUDS_TRACE(0,"custom_ui:case open run complete");
}

/*****************************************************************************
 Prototype    : app_ibrt_customif_case_close_run_complete_callback
 Description  : called after case close event run complete
 Input        : None
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History      :
 Date         : 2019/12/02
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/
void app_ibrt_customif_case_close_run_complete_callback()
{
    EARBUDS_TRACE(0,"custom_ui:case close run complete");
}

/*****************************************************************************
 Prototype    : app_ibrt_customif_case_close_run_complete_callback
 Description  : notify mode_switch_manager exit earbud run complete
 Input        : None
 Output       : None
 Return Value :
 Calls        : None
 Called By    :

 History      :
 Date         : 2023/2/14
 Author       : bestechnic
 Modification : Created function
*****************************************************************************/
void app_ibrt_customif_exit_earbud_mode_run_complete_callback()
{
    EARBUDS_TRACE(0,"custom_ui:exit earbude mode run complete");
}

static void app_ibrt_customif_evt_run_complete_callback(app_ui_evt_t evt)
{
    switch (evt)
    {
        case APP_UI_EV_CASE_OPEN:
            app_ibrt_customif_case_open_run_complete_callback();
            break;
        case APP_UI_EV_CASE_CLOSE:
            app_ibrt_customif_case_close_run_complete_callback();
            break;
        case APP_UI_EV_EXIT_EARBUD_MODE:
            app_ibrt_customif_exit_earbud_mode_run_complete_callback();
            break;
        case APP_UI_EV_DESTROY_DEVICE:
#ifdef GFPS_ENABLED
            gfps_link_destory_handler();
#endif
            break;
        default:
            break;
    }
    if (ibrt_mgr_status_changed_client_cb && ibrt_mgr_status_changed_client_cb->ibrt_mgr_event_run_comp_hook) {
        ibrt_mgr_status_changed_client_cb->ibrt_mgr_event_run_comp_hook(evt);
    }
}

/*****************************************************************************
 Prototype    : app_ibrt_customif_switch_ui_role_run_complete_callback
 Description  : called after APP_UI_EV_USER_SWITCH2MASTER/APP_UI_EV_USER_SWITCH2SLAVE event run complete
 Input        : None
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History      :
 Date         : 2022/12/08
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/
void app_ibrt_customif_switch_ui_role_run_complete_callback(TWS_UI_ROLE_E current_role, uint8_t errCode)
{
    if (!errCode)
    {
        EARBUDS_TRACE(0,"custom_ui:switch ibrt role to %d run complete", current_role);
    }
    else
    {
        EARBUDS_TRACE(0,"custom_ui:exist device doing ibrt role switch to %d failed", current_role);
    }
    if (ibrt_mgr_status_changed_client_cb && ibrt_mgr_status_changed_client_cb->ibrt_mgr_tws_role_switch_comp_hook) {
        ibrt_mgr_status_changed_client_cb->ibrt_mgr_tws_role_switch_comp_hook(current_role, errCode);
    }
}

void app_ibrt_customif_peer_box_state_update_callback(bud_box_state box_state)
{
#ifdef BESUI_TWS_EN
    user_set_peer_box_sta(box_state);
#endif
    EARBUDS_TRACE(0,"custom_ui:peer box state update=%s", app_ui_box_state_to_string(box_state));

    if (box_state == IBRT_IN_BOX_OPEN)
    {
        uint8_t local_percent = app_battery_current_level();

        EARBUDS_TRACE(1,
            "[BAT_SYNC][BOX_OPEN] local=%d",
            local_percent);

        app_ibrt_customif_cmd_sync_battery_level(local_percent);
    }

    if (ibrt_mgr_status_changed_client_cb && ibrt_mgr_status_changed_client_cb->peer_box_state_update_hook) {
        ibrt_mgr_status_changed_client_cb->peer_box_state_update_hook(box_state);
    }
}

void app_ibrt_customif_pre_handle_box_event_callback(app_ui_evt_t box_evt)
{
    EARBUDS_TRACE(0,"custom_ui:pre handle event=%s", app_ui_event_to_string(box_evt));
    if (ibrt_mgr_status_changed_client_cb && ibrt_mgr_status_changed_client_cb->pre_handle_box_event_hook) {
        ibrt_mgr_status_changed_client_cb->pre_handle_box_event_hook(box_evt);
    }
}
/*
* custom tws switch interface
* tws switch cmd send sucess, return true, else return false
*/
void app_ibrt_customif_ui_tws_switch(void)
{
    app_ibrt_if_tws_role_switch_request();
}

/*
* custom reconfig bd_addr
*/
void app_ibrt_customif_ui_reconfig_bd_addr(bt_bdaddr_t local_addr, bt_bdaddr_t peer_addr, ibrt_role_e nv_role)
{
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();

    p_ibrt_ctrl->local_addr = local_addr;
    p_ibrt_ctrl->peer_addr  = peer_addr;
    p_ibrt_ctrl->nv_role    = nv_role;

    if (!p_ibrt_ctrl->is_ibrt_search_ui)
    {
        if (IBRT_MASTER == p_ibrt_ctrl->nv_role)
        {
            p_ibrt_ctrl->peer_addr = local_addr;
            btif_me_set_bt_address(p_ibrt_ctrl->local_addr.address);
        }
        else if (IBRT_SLAVE == p_ibrt_ctrl->nv_role)
        {
            p_ibrt_ctrl->local_addr = peer_addr;
            btif_me_set_bt_address(p_ibrt_ctrl->local_addr.address);
        }
        else
        {
            ASSERT(0, "%s nv_role error", __func__);
        }
    }
}

/*custom can block connect mobile if needed*/
bool app_ibrt_customif_connect_mobile_needed_ind(void)
{
    return true;
}

uint32_t app_ibrt_customif_set_profile_delaytime_on_spp_connect(const uint8_t *uuid_data_ptr, uint8_t uuid_len)
{
    // While spp connect and no basic profile established, this func will be called.
    // Add custom implementation here, if you need to make peer buds connects to the spp as soon as possible,
    // just return a none-zero value. For example, "return 1" will be the fastest.
    // return t : wait t ms to start profile exchange
    // return 0 : will not change the process of "profile exchange"
    EARBUDS_TRACE(0,"spp_uuid len=%d bytes, uuid:", uuid_len);
    EARBUDS_DUMP8("0x%x ", uuid_data_ptr, uuid_len);

    return 0;
}

WEAK void app_ibrt_customif_ui_update_mgr_config(app_ui_config_t* pConfig)
{

}

#ifdef IS_REGISTER_TWP_TEST_FUNCTION
static void app_ibrt_customif_ui_link_loss_universal_info_handle_test(ANALYSISING_INFO_TYPE info_type, uint16_t conn_hdl, uint8_t* info, uint8_t info_len)
{
    EARBUDS_TRACE(0,"Link loss universal info received: type=%d", info_type);
    tws_ibrt_link_loss_universal_info_t *univ_info = (tws_ibrt_link_loss_universal_info_t*)info;
    EARBUDS_TRACE(0,"conn_hdl=0x%x       num_conns=0x%x      link_role=%d", univ_info->conn_hdl, univ_info->num_conn_role.num_conns, univ_info->num_conn_role.link_role);
    EARBUDS_TRACE(0,"LST_RSSI=%d         LSR_RSSI=%d       LS_clk=0x%x", univ_info->LST_RSSI, univ_info->LSR_RSSI, univ_info->LS_clk);
    EARBUDS_TRACE(0,"AFH_state=0x%x AFH_MAP:", univ_info->AFH_state);
    EARBUDS_DUMP8("%02x ", univ_info->AFH_MAP, BT_LINK_LOSS_AFH_MAP_LEN);
    EARBUDS_TRACE(0,"super_tmot=0x%x     sniff_mode=0x%x     sniff_offset=0x%x", univ_info->super_tmot, univ_info->sniff_mode, univ_info->sniff_offset);
    EARBUDS_TRACE(0,"super_interval=0x%x sniff_atmt=%0xx     sniff_tmot=0x%x", univ_info->sniff_interval, univ_info->sniff_atmt, univ_info->sniff_tmot);
    EARBUDS_TRACE(0,"sub_latency=0x%x    sub_min_Rmt_to=0x%x sub_min_Lcl_to=0x%x", univ_info->sub_latency, univ_info->sub_min_Rmt_to, univ_info->sub_min_Lcl_to);
    EARBUDS_TRACE(0,"lsto_tx_info:       type=%d BR_EDR=%d BB_flow_stop=%d l2cap_stop_flag=%d",
            univ_info->lsto_tx_info.lsto_tx_type, univ_info->lsto_tx_info.lsto_tx_br_edr, univ_info->lsto_tx_info.lsto_tx_BB_flow_stop, univ_info->lsto_tx_info.lsto_tx_l2cap_flow_stop);
    EARBUDS_TRACE(0,"lsto_tx_pwr=%d      lsto_tx_lmp=0x%x    lsto_tx_retr_tx=0x%x", univ_info->lsto_tx_pwr, univ_info->lsto_tx_lmp, univ_info->lsto_tx_retr_tx);
    EARBUDS_TRACE(0,"lsto_rx_info:       type=%d BR_EDR=%d BB_flow_stop=%d l2cap_stop_flag=%d",
            univ_info->lsto_rx_info.lsto_rx_type, univ_info->lsto_rx_info.lsto_rx_br_edr, univ_info->lsto_rx_info.lsto_rx_BB_flow_stop, univ_info->lsto_rx_info.lsto_rx_l2cap_flow_stop);
    EARBUDS_TRACE(0,"lsto_rx_lmp=%x      lsto_op_tmot=%x", univ_info->lsto_rx_lmp, univ_info->lmp_op_tmot);
    for (int i = 0;i < BT_NETWORK_TOPOLOGY_MAX_ACTIVE_CONN_NUM;i++)
    {
        EARBUDS_TRACE(0,"conn%d: net_topo_info: handle=0x%x role=%d", i, univ_info->net_topo_info.nt_conn_hdl[i], univ_info->net_topo_info.nt_role[i]);
    }
    EARBUDS_DUMP8("%02x ", &(univ_info->net_topo_info), 21);
}

static void app_ibrt_customif_ui_link_loss_clock_info_handle_test(ANALYSISING_INFO_TYPE info_type, uint16_t conn_hdl, uint8_t* info, uint8_t info_len)
{
    EARBUDS_TRACE(0,"Link loss clock info received: type=%d", info_type);
    tws_ibrt_link_loss_clock_info_t *clock_info = (tws_ibrt_link_loss_clock_info_t*)info;
    EARBUDS_TRACE(0,"conn_hdl:0x%x nmu_no_sync_anchor=0x%x", conn_hdl, clock_info->nmu_no_sync_anchor);
    EARBUDS_TRACE(0,"Number of Clock:%d", BT_LINK_LOSS_RX_CLOCK_INFO_SIZE);
    for (int i = 0;i < BT_LINK_LOSS_RX_CLOCK_INFO_SIZE;i++)
    {
        EARBUDS_TRACE(0,"Piconet RX/TX Clock#%d: 0x%x(ch:%d)", i, clock_info->Rx_clk[i], clock_info->rf_channel_num[i]);
    }
}

static void app_ibrt_customif_ui_a2dp_sink_info_handle_test(ANALYSISING_INFO_TYPE info_type, uint16_t conn_hdl, uint8_t* info, uint8_t info_len)
{
    EARBUDS_TRACE(0,"A2DP sink info received: type=%d", info_type);
    tws_ibrt_a2dp_sink_info_t *a2dp_sink_info = (tws_ibrt_a2dp_sink_info_t*)info;
    EARBUDS_TRACE(0,"conn_hdl=0x%x      retran_cnt=0x%x  ant_gt=0x%x", a2dp_sink_info->conn_hdl, a2dp_sink_info->retran_cnt, a2dp_sink_info->ant_gt);
    EARBUDS_TRACE(0,"hec_cnt=0x%x       crc_cnt=0x%x     st_cnt=0x%x", a2dp_sink_info->hec_cnt, a2dp_sink_info->crc_cnt, a2dp_sink_info->sync_tmot_cnt);
    EARBUDS_TRACE(0,"lmp_tx_cnt=%x      pull_tx_cnt=0x%x null_tx_cnt=0x%x", a2dp_sink_info->lmp_tx_cnt, a2dp_sink_info->pull_tx_cnt, a2dp_sink_info->null_tx_cnt);
    EARBUDS_TRACE(0,"lmp_rx_cnt=%x      pull_rx_cnt=0x%x null_rx_cnt=0x%x", a2dp_sink_info->lmp_rx_cnt, a2dp_sink_info->pull_rx_cnt, a2dp_sink_info->null_rx_cnt);
    EARBUDS_TRACE(0,"last_txpwr=%d      last_rssi=%d   last_channel=0x%x", a2dp_sink_info->last_txpwr, a2dp_sink_info->last_rssi, a2dp_sink_info->last_channel);
    EARBUDS_TRACE(0,"last_pic_clk=%x    last_jitter_buf_cnt=0x%x", a2dp_sink_info->last_pic_clk, a2dp_sink_info->last_jitter_buf_cnt);
    EARBUDS_TRACE(0,"pre_txpwr=%d       pre_rssi=%d    pre_channel=0x%x", a2dp_sink_info->pre_txpwr, a2dp_sink_info->pre_rssi, a2dp_sink_info->pre_channel);
    EARBUDS_TRACE(0,"pre_pic_clk=%x     pre_jitter_buf_cnt=0x%x", a2dp_sink_info->pre_pic_clk, a2dp_sink_info->pre_jitter_buf_cnt);
}
#endif

static APP_IBRT_IF_LINK_STATUS_CHANGED_CALLBACK link_status_cbs = {
    .ibrt_global_state_changed          = app_ibrt_customif_global_state_callback,
    .ibrt_a2dp_state_changed            = app_ibrt_customif_a2dp_callback,
    .ibrt_hfp_state_changed             = app_ibrt_customif_hfp_callback,
    .ibrt_avrcp_state_changed           = app_ibrt_customif_avrcp_callback,
    .ibrt_tws_pairing_changed           = app_ibrt_customif_tws_on_paring_state_changed,
    .ibrt_tws_acl_state_changed         = app_ibrt_customif_tws_on_acl_state_changed,
    .ibrt_mobile_acl_state_changed      = app_ibrt_customif_on_mobile_acl_state_changed,
    .ibrt_sco_state_changed             = app_ibrt_customif_sco_state_changed,
    .ibrt_tws_role_switch_status_ind    = app_ibrt_customif_on_tws_role_switch_status_ind,
    .ibrt_ibrt_state_changed            = app_ibrt_customif_on_ibrt_state_changed,
    .ibrt_access_mode_changed           = app_ibrt_customif_on_access_mode_changed,
};

static ibrt_mgr_status_changed_cb_t mgr_status_cbs = {
    .ibrt_mgr_event_run_comp_hook           = app_ibrt_customif_evt_run_complete_callback,
    .ibrt_mgr_pairing_mode_entry_hook       = app_ibrt_customif_pairing_mode_entry,
    .ibrt_mgr_pairing_mode_exit_hook        = app_ibrt_customif_pairing_mode_exit,
    .ibrt_mgr_pairing_mode_timeout_pre_hook = app_ibrt_customif_pairing_mode_timeout_pre_callback,
    .ibrt_mgr_pairing_mode_timeout_post_hook= app_ibrt_customif_pairing_mode_timeout_post_callback,
    .ibrt_mgr_tws_role_switch_comp_hook     = app_ibrt_customif_switch_ui_role_run_complete_callback,
    .peer_box_state_update_hook             = app_ibrt_customif_peer_box_state_update_callback,
    .pre_handle_box_event_hook              = app_ibrt_customif_pre_handle_box_event_callback,
};

static ibrt_ext_conn_policy_cb_t conn_policy_cbs = {
    .accept_incoming_conn_req_hook      = app_ibrt_customif_incoming_conn_req_callback,
    .accept_extra_incoming_conn_req_hook= app_ibrt_customif_extra_incoming_conn_req_callback,
    .disallow_start_reconnect_mob_hook  = app_ibrt_customif_custom_disallow_reconnect_mob_callback,
    .disallow_start_reconnect_tws_hook  = app_ibrt_customif_custom_disallow_reconnect_tws_callback,
    .disallow_tws_role_switch_hook      = app_ibrt_customif_disallow_tws_role_switch_callback,
};

#if BLE_AUDIO_ENABLED
static void app_ble_aud_adv_state_changed(AOB_ADV_STATE_T state, uint8_t err_code)
{
    EARBUDS_TRACE(0,"custom_ui:leaudio adv state changed = %d", state);
    switch (state)
    {
        case AOB_ADV_START:
            break;
        case AOB_ADV_STOP:
            break;
        case AOB_ADV_FAILED:
            break;
        default:
            break;
    }
}

static void app_ble_aud_mob_acl_state_changed(uint8_t conidx, const ble_bdaddr_t* addr, AOB_ACL_STATE_T state, uint8_t errCode)
{
    EARBUDS_TRACE(0,"custom_ui:d(%d) le mobile acl state changed = %d, errCode = %d", conidx, state, errCode);
    static bool first_bond = true;

    switch (state)
    {
        case AOB_ACL_DISCONNECTED:
            first_bond = true;
            break;
        case AOB_ACL_CONNECTING:
            break;
        case AOB_ACL_FAILED:
           break;
        case AOB_ACL_CONNECTED:
            break;
        /* report bond success after encrypt success, so will not report encrypt success later */
        case AOB_ACL_BOND_SUCCESS:
            break;
        case AOB_ACL_BOND_FAILURE:
            break;
        case AOB_ACL_ENCRYPT:
            break;
        case AOB_ACL_ATTR_BOND:
            if (first_bond) {
                EARBUDS_TRACE(0,"LE AUDIO ATTR BOND!!!");
                first_bond = false;
                app_ui_keep_only_the_leaudio_device(conidx);
            }
            break;
        case AOB_ACL_DISCONNECTING:
            break;
    }
}

static void app_ble_aud_vol_changed_cb(uint8_t con_lid, uint8_t volume, uint8_t mute)
{

}

static void app_ble_aud_vocs_offset_changed_cb(int16_t offset, uint8_t output_lid)
{

}

static void app_ble_aud_vocs_bond_data_changed_cb(uint8_t output_lid, uint8_t cli_cfg_bf)
{

}

static void app_ble_aud_media_track_change_cb(uint8_t con_lid)
{

}

static void app_ble_aud_media_stream_status_change_cb(uint8_t con_lid, uint8_t ase_lid, AOB_MGR_STREAM_STATE_E state)
{

}

static void app_ble_aud_media_playback_status_change_cb(uint8_t con_lid, AOB_MGR_PLAYBACK_STATE_E state)
{
    EARBUDS_TRACE(0,"custom_ui:d(%d) media_pb_status_changed= %d",con_lid,state);

    switch (state)
    {
       case AOB_MGR_PLAYBACK_STATE_INACTIVE:
          break;
       case AOB_MGR_PLAYBACK_STATE_PLAYING:
          break;
       case AOB_MGR_PLAYBACK_STATE_PAUSED:
          break;
       case AOB_MGR_PLAYBACK_STATE_SEEKING:
         break;
       default:
        break;
    }
}

static void app_ble_aud_media_mic_state_cb(uint8_t mute)
{


}

static void app_ble_aud_media_iso_link_quality_cb(AOB_ISO_LINK_QUALITY_INFO_T *param)
{

}

static void app_ble_aud_media_pacs_cccd_written_cb(uint8_t con_lid)
{

}

static void app_ble_aud_call_state_change_cb(uint8_t con_lid, void *param)
{
    AOB_SINGLE_CALL_INFO_T * call_state_changed = (AOB_SINGLE_CALL_INFO_T*)param;

    EARBUDS_TRACE(0,"custom_ui:d(%d) call_state_changed = %d",con_lid,call_state_changed->state);

    switch (call_state_changed->state)
    {
        case AOB_CALL_STATE_INCOMING:
            break;
        case AOB_CALL_STATE_DIALING:
            break;
        case AOB_CALL_STATE_ALERTING:
            break;
        case AOB_CALL_STATE_ACTIVE:
            break;
        case AOB_CALL_STATE_LOCAL_HELD:
            break;
        case AOB_CALL_STATE_REMOTE_HELD:
            break;
        case AOB_CALL_STATE_LOCAL_AND_REMOTE_HELD:
            break;
        case AOB_CALL_STATE_IDLE:
            break;
        default:
            break;

    }

}

static void app_ble_aud_call_srv_signal_strength_value_ind_cb(uint8_t con_lid, uint8_t value)
{

}

static void app_ble_aud_call_status_flags_ind_cb(uint8_t con_lid, bool inband_ring, bool silent_mode)
{

}

static void app_ble_aud_call_ccp_opt_supported_opcode_ind_cb(uint8_t con_lid, bool local_hold_op_supported, bool join_op_supported)
{

}

static void app_ble_aud_call_terminate_reason_ind_cb(uint8_t con_lid, uint8_t call_id, uint8_t reason)
{


}

static void app_ble_aud_call_incoming_number_inf_ind_cb(uint8_t con_lid, uint8_t url_len, uint8_t *url)
{

}

static void app_ble_aud_aob_call_svc_changed_ind_cb(uint8_t con_lid)
{

}

static void app_ble_aud_call_action_result_ind_cb(uint8_t con_lid, void *param)
{

}

const static bts_ble_audio_event_cb_t custom_ui_le_audio_event_cb = {
    .ble_audio_adv_state_changed  = app_ble_aud_adv_state_changed,
    .mob_acl_state_changed = app_ble_aud_mob_acl_state_changed,
    .vol_changed_cb = app_ble_aud_vol_changed_cb,
    .vocs_offset_changed_cb = app_ble_aud_vocs_offset_changed_cb,
    .vocs_bond_data_changed_cb = app_ble_aud_vocs_bond_data_changed_cb,
    .media_track_change_cb = app_ble_aud_media_track_change_cb,
    .media_stream_status_change_cb = app_ble_aud_media_stream_status_change_cb,
    .media_playback_status_change_cb = app_ble_aud_media_playback_status_change_cb,
    .media_mic_state_cb = app_ble_aud_media_mic_state_cb,
    .media_iso_link_quality_cb = app_ble_aud_media_iso_link_quality_cb,
    .media_pacs_cccd_written_cb = app_ble_aud_media_pacs_cccd_written_cb,
    .call_state_change_cb = app_ble_aud_call_state_change_cb,
    .call_srv_signal_strength_value_ind_cb = app_ble_aud_call_srv_signal_strength_value_ind_cb,
    .call_status_flags_ind_cb = app_ble_aud_call_status_flags_ind_cb,
    .call_ccp_opt_supported_opcode_ind_cb = app_ble_aud_call_ccp_opt_supported_opcode_ind_cb,
    .call_terminate_reason_ind_cb = app_ble_aud_call_terminate_reason_ind_cb,
    .call_incoming_number_inf_ind_cb = app_ble_aud_call_incoming_number_inf_ind_cb,
    .call_svc_changed_ind_cb = app_ble_aud_aob_call_svc_changed_ind_cb,
    .call_action_result_ind_cb = app_ble_aud_call_action_result_ind_cb,
};

#endif /* #if BLE_AUDIO_ENABLED  */

int app_ibrt_customif_ui_start(void)
{
    app_ui_config_t config;

    // zero init the config
    memset(&config, 0, sizeof(app_ui_config_t));

    //freeman mode config, default should be false
#ifdef FREEMAN_ENABLED_STERO
    config.freeman_enable                           = true;
#else
    config.freeman_enable                           = false;
#endif

    //enable sdk lea adv strategy, default should be true
    config.sdk_lea_adv_enable                       = true;

    //pairing mode timeout value config
    config.pairing_timeout_value                    = IBRT_UI_DISABLE_BT_SCAN_TIMEOUT;

    //SDK pairing enable, the default should be true
    config.sdk_pairing_enable                       = true;

    //passive enter pairing when no mobile record, the default should be false
#if defined(BESUI_TWS_EN) || defined(BESUI_STEREO_EN)
    config.enter_pairing_on_empty_record            = true;
#else
    config.enter_pairing_on_empty_record            = false;
#endif

    //passive enter pairing when reconnect failed, the default should be false
#ifdef FREEMAN_ENABLED_STERO
    config.enter_pairing_on_reconnect_mobile_failed = true;
#else
#ifdef BESUI_TWS_EN
    config.enter_pairing_on_reconnect_mobile_failed = true;
#else
    config.enter_pairing_on_reconnect_mobile_failed = false;
#endif
#endif

    //passive enter pairing when mobile disconnect, the default should be false
#if defined(BESUI_TWS_EN) || defined(BESUI_STEREO_EN)
    config.enter_pairing_on_mobile_disconnect       = true;
#else
    config.enter_pairing_on_mobile_disconnect       = false;
#endif

    //exit pairing when peer close, the default should be true
#if defined(BESUI_TWS_EN) || defined(BESUI_STEREO_EN)
    config.exit_pairing_on_peer_close               = false;
#else
    config.exit_pairing_on_peer_close               = true;
#endif

    //exit pairing when a2dp or hfp streaming, the default should be true
    config.exit_pairing_on_streaming                = true;

    //disconnect remote devices when entering pairing mode actively
    config.paring_with_disc_mob_num                 = IBRT_PAIRING_DISC_ONE_MOB;

    //disconnect remote le devices when entering pairing mode actively
    config.paring_with_disc_le_mob_num              = IBRT_PAIRING_DISC_NONE;

    //disconnect SCO or not when accepting phone connection in pairing mode
    config.pairing_with_disc_hf_cfg                 = IBRT_PAIRING_HF_HUNGUP;

    //pause current music when entering pairing mode
    config.pairing_with_pause_music                 = true;

    //not start ibrt in pairing mode, the default should be false
    config.pairing_without_start_ibrt               = false;

    //set nv master to tws master for lea connection
    config.pairing_with_set_nv_master_as_master     = true;

    //accept new dev only in pairing state
    config.accept_new_dev_only_in_pairing           = false;

    //disconnect tws immediately when single bud closed in box
    config.disc_tws_imm_on_single_bud_closed        = false;

    //do tws switch when RSII value change, default should be true
    config.tws_switch_according_to_rssi_value       = false;

    //do tws switch when RSII value change, timer threshold
    config.role_switch_timer_threshold                    = IBRT_UI_ROLE_SWITCH_TIME_THRESHOLD;

    //do tws switch when rssi value change over threshold
    config.rssi_threshold                                 = IBRT_UI_ROLE_SWITCH_THRESHOLD_WITH_RSSI;

    //close box delay disconnect tws timeout
    config.close_box_delay_tws_disc_timeout               = 0;

    //close box debounce time config
    config.close_box_event_wait_response_timeout          = IBRT_UI_CLOSE_BOX_EVENT_WAIT_RESPONSE_TIMEOUT;

    //wait time before launch reconnect event
    config.reconnect_mobile_wait_response_timeout         = IBRT_UI_RECONNECT_MOBILE_WAIT_RESPONSE_TIMEOUT;

    //reconnect event internal config wait timer when tws disconnect
    config.reconnect_wait_ready_timeout                   = IBRT_UI_MOBILE_RECONNECT_WAIT_READY_TIMEOUT;//inquiry scan enable timeout when enter paring

    config.tws_conn_failed_wait_time                      = TWS_CONN_FAILED_WAIT_TIME;

    config.reconnect_mobile_wait_ready_timeout            = IBRT_UI_MOBILE_RECONNECT_WAIT_READY_TIMEOUT;
    config.reconnect_tws_wait_ready_timeout               = IBRT_UI_TWS_RECONNECT_WAIT_READY_TIMEOUT;
    config.reconnect_ibrt_wait_response_timeout           = IBRT_UI_RECONNECT_IBRT_WAIT_RESPONSE_TIMEOUT;
    config.nv_master_reconnect_tws_wait_response_timeout  = IBRT_UI_NV_MASTER_RECONNECT_TWS_WAIT_RESPONSE_TIMEOUT;
    config.nv_slave_reconnect_tws_wait_response_timeout   = IBRT_UI_NV_SLAVE_RECONNECT_TWS_WAIT_RESPONSE_TIMEOUT;

    config.check_plugin_excute_closedbox_event            = true;
    config.ibrt_with_ai                                   = false;
    config.giveup_reconn_when_peer_unpaired               = false;

    //if tws&mobile disc, reconnect tws first until tws connected then reconn mobile
    config.delay_reconn_mob_until_tws_connected           = false;

    config.delay_reconn_mob_max_times                     = IBRT_UI_DELAY_RECONN_MOBILE_MAX_TIMES;

    //open box reconnect mobile times config
    config.open_reconnect_mobile_max_times                = IBRT_UI_OPEN_RECONNECT_MOBILE_MAX_TIMES;

    //open box reconnect tws times config
    config.open_reconnect_tws_max_times                   = IBRT_UI_OPEN_RECONNECT_TWS_MAX_TIMES;

    //connection timeout reconnect mobile times config
    config.reconnect_mobile_max_times                     = IBRT_UI_RECONNECT_MOBILE_MAX_TIMES;

    //connection timeout reconnect tws times config
    config.reconnect_tws_max_times                        = IBRT_UI_RECONNECT_TWS_MAX_TIMES;

    //connection timeout reconnect ibrt times config
    config.reconnect_ibrt_max_times                       = IBRT_UI_RECONNECT_IBRT_MAX_TIMES;

    //controller basband monitor
    config.lowlayer_monitor_enable                        = true;
    config.llmonitor_report_format                        = REP_FORMAT_PACKET;
    config.llmonitor_report_count                         = 1000;
  //  config.llmonitor_report_format                      = REP_FORMAT_TIME;
  //  config.llmonitor_report_count                       = 1600;////625us uint

    config.mobile_page_timeout                            = IBRT_MOBILE_PAGE_TIMEOUT;
    //tws connection supervision timeout
    config.tws_connection_timeout                         = IBRT_UI_TWS_CONNECTION_TIMEOUT;

    config.rx_seq_error_timeout                           = IBRT_UI_RX_SEQ_ERROR_TIMEOUT;
    config.rx_seq_error_threshold                         = IBRT_UI_RX_SEQ_ERROR_THRESHOLD;
    config.rx_seq_recover_wait_timeout                    = IBRT_UI_RX_SEQ_ERROR_RECOVER_TIMEOUT;

    config.rssi_monitor_timeout                           = IBRT_UI_RSSI_MONITOR_TIMEOUT;

    config.radical_scan_interval_nv_slave                 = IBRT_UI_RADICAL_SAN_INTERVAL_NV_SLAVE;
    config.radical_scan_interval_nv_master                = IBRT_UI_RADICAL_SAN_INTERVAL_NV_MASTER;

    config.scan_interval_in_sco_tws_disconnected          = IBRT_UI_SCAN_INTERVAL_IN_SCO_TWS_DISCONNECTED;
    config.scan_window_in_sco_tws_disconnected            = IBRT_UI_SCAN_WINDOW_IN_SCO_TWS_DISCONNECTED;

    config.scan_interval_in_sco_tws_connected             = IBRT_UI_SCAN_INTERVAL_IN_SCO_TWS_CONNECTED;
    config.scan_window_in_sco_tws_connected               = IBRT_UI_SCAN_WINDOW_IN_SCO_TWS_CONNECTED;

    config.scan_interval_in_a2dp_tws_disconnected         = IBRT_UI_SCAN_INTERVAL_IN_A2DP_TWS_DISCONNECTED;
    config.scan_window_in_a2dp_tws_disconnected           = IBRT_UI_SCAN_WINDOW_IN_A2DP_TWS_DISCONNECTED;

    config.scan_interval_in_a2dp_tws_connected            = IBRT_UI_SCAN_INTERVAL_IN_A2DP_TWS_CONNECTED;
    config.scan_window_in_a2dp_tws_connected              = IBRT_UI_SCAN_WINDOW_IN_A2DP_TWS_CONNECTED;

    config.connect_no_03_timeout                          = CONNECT_NO_03_TIMEOUT;
    config.disconnect_no_05_timeout                       = DISCONNECT_NO_05_TIMEOUT;

    config.tws_switch_tx_data_protect                     = true;

    config.tws_cmd_send_timeout                           = IBRT_UI_TWS_CMD_SEND_TIMEOUT;
    config.tws_cmd_send_counter_threshold                 = IBRT_UI_TWS_COUNTER_THRESHOLD;

    config.profile_concurrency_supported                  = true;

    config.audio_sync_mismatch_resume_version             = 2;

#ifdef APP_MULTIPOINT_ONOFF_EN 
    config.support_steal_connection                       = false;
#else
    config.support_steal_connection                       = true;
#endif
    config.support_steal_connection_in_sco                = false;
    config.support_steal_connection_in_a2dp_steaming      = true;
    config.steal_audio_inactive_device                    = false;

    config.allow_sniff_in_sco                             = false;

    config.always_interlaced_scan                         = true;

    config.disallow_reconnect_bt_device_in_streaming_state = true;

    config.disallow_reconnect_tws_in_streaming_state      = true;

    config.open_without_auto_reconnect                    = false;

    config.without_reconnect_if_remote_driving_disc       = false;
#if defined(BESUI_TWS_EN) || defined(BESUI_STEREO_EN)
    config.without_reconnect_when_fetch_out_wear_up       = true;
#else
    config.without_reconnect_when_fetch_out_wear_up       = false;
#endif

    config.disallow_rs_by_box_state                       = false;

    config.only_rs_when_sco_active                        = true;

    config.reject_same_cod_device_conn_req                = false;

#ifdef IBRT_UI_MASTER_ON_TWS_DISCONNECTED
    config.is_changed_to_ui_master_on_tws_disconnected    = true;
#else
    config.is_changed_to_ui_master_on_tws_disconnected    = false;
#endif

    config.connected_max_device_num_allow_new_connect     = true;

    config.lea_connected_allow_new_connect                = true;

    config.dev_idle_allow_nonsupport_stay_connected       = true;

    app_ibrt_customif_ui_update_mgr_config(&config);
    app_ui_register_link_status_callback(&link_status_cbs);
    app_ui_register_mgr_status_callback(&mgr_status_cbs);
    app_ui_register_ext_conn_policy_callback(&conn_policy_cbs);
#if BLE_AUDIO_ENABLED
    bts_lea_register_audio_event_callback(&custom_ui_le_audio_event_cb);
#endif
    app_ibrt_if_register_vender_handler_ind(app_ibrt_customif_ui_vender_event_handler_ind);

#ifdef IS_REGISTER_TWP_TEST_FUNCTION
    app_ibrt_if_register_link_loss_universal_info_received_cb(app_ibrt_customif_ui_link_loss_universal_info_handle_test);
    app_ibrt_if_register_link_loss_clock_info_received_cb(app_ibrt_customif_ui_link_loss_clock_info_handle_test);
    app_ibrt_if_register_a2dp_sink_info_received_cb(app_ibrt_customif_ui_a2dp_sink_info_handle_test);
#endif
    bts_core_register_tws_ui_role_update_handle(app_ibrt_customif_tws_ui_role_updated);
    app_ibrt_if_config(&config);

#ifdef BT_ALWAYS_IN_DISCOVERABLE_MODE
    btif_me_configure_keeping_both_scan(true);
#endif

    app_spp_is_connected_register(app_ibrt_customif_set_profile_delaytime_on_spp_connect);

    return 0;
}
#endif // IBRT_UI

void app_ibrt_customif_tws_ui_role_updated(uint8_t newRole)
{
    app_ibrt_middleware_ui_role_updated_handler(newRole);
    EARBUDS_TRACE(0,"%s newRole=%d", __func__, newRole);
    extern int bt_sco_chain_set_master_role(bool is_master);
    bt_sco_chain_set_master_role(newRole == TWS_UI_MASTER);
    // to add custom implementation here
}

bool app_ibrt_customif_disallow_pagescan()
{
    return false;
}

bool app_ibrt_customif_disallow_reconnect_mobile()
{
#ifdef AUDIO_LINEIN
    if (bt_media_cur_is_linein_stream())
    {
        EARBUDS_TRACE(0,"aux mode, disallow reconnect mobile");
        return true;
    }
#endif

    return false;
}