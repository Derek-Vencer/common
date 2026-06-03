/***************************************************************************
*
* Copyright 2015-2020 BES.
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

#include <string.h>
#include "app_trace_rx.h"
#include "app_bt_cmd.h"
#include "cmsis_os.h"
#include "hal_trace.h"
#include "bluetooth.h"
#include "crc_c.h"
#include "heap_api.h"
#include "hci_api.h"
#include "me_api.h"
#include "spp_api.h"
#include "l2cap_api.h"
#include "conmgr_api.h"
#include "bt_if.h"
#include "bt_drv_interface.h"
#include "bt_drv_reg_op.h"
#include "app_bt.h"
#include "btapp.h"
#include "app_factory_bt.h"
#include "apps.h"
#include "app_a2dp.h"
#include "app_hfp.h"
#include "nvrecord_extension.h"
#include "nvrecord_bt.h"
#include "app_bt_func.h"
#include "app_testmode.h"
#include "besaud_api.h"
#include "ddbif.h"
#include "audio_codec_api.h"
#include "app_ble_test.h"
#include "hid_api.h"

#if defined(UTILS_ESHELL_EN)
#include "eshell.h"
#endif

#ifdef APP_CHIP_BRIDGE_MODULE
#include "app_chip_bridge.h"
#endif

#if defined(BT_SOURCE)
#include "bt_source.h"
#include "app_a2dp_source.h"
#include "bts_a2dp_source.h"
#endif

#ifdef AUDIO_LINEIN
#include "app_bt_media_manager.h"
#endif

#ifdef LOCAL_AUDIO_SUPPORT
extern "C" void pmu_reboot(void);
#include "hal_bootmode.h"
#ifdef LOCAL_AUDIO_TEST_ENABLE
#include "app_local_audio_stream_handler.h"
#endif // LOCAL_AUDIO_TEST_ENABLE
#endif // LOCAL_AUDIO_SUPPORT

#include "audio_player_adapter.h"

#if !defined(CHIP_BEST1503) && !defined(CHIP_BEST1306P)
#define DIAL_NUMBER_LEN_MAX            32

typedef struct {
    bt_bdaddr_t addr;
    uint16_t len;
    char number[DIAL_NUMBER_LEN_MAX];
} bt_hf_dial_number_param_t;

static bt_bdaddr_t g_app_bt_pts_addr = {{
#if 1
    0xed, 0xb6, 0xf4, 0xdc, 0x1b, 0x00
#elif 0
    0x81, 0x33, 0x33, 0x22, 0x11, 0x11
#elif 0
    0x13, 0x71, 0xda, 0x7d, 0x1a, 0x00
#else
    0x14, 0x71, 0xda, 0x7d, 0x1a, 0x00
#endif
}};

bt_bdaddr_t *app_bt_get_pts_address(void)
{
    return &g_app_bt_pts_addr;
}

WEAK void app_otaMode_enter(APP_KEY_STATUS *status, void *param)
{

}

POSSIBLY_UNUSED static bool app_bt_scan_bdaddr_from_string(const char* param, uint32_t len, bt_bdaddr_t *out_addr)
{
    int bytes[sizeof(bt_bdaddr_t)] = {0};

    if (len < 17)
    {
        DEBUG_INFO(0, "%s wrong len %d '%s'", __func__, len, param);
        return false;
    }

    DEBUG_INFO(0,  "%s '%s'", __func__,param);


    if (6 != sscanf(param, "%x:%x:%x:%x:%x:%x", bytes+0, bytes+1, bytes+2, bytes+3, bytes+4, bytes+5))
    {
        DEBUG_INFO(0, "%s parse address failed %s sscanf=%d", __func__, param,sscanf(param, "%x:%x:%x:%x:%x:%x",
            bytes+0, bytes+1, bytes+2, bytes+3, bytes+4, bytes+5));
        return false;
    }

    bt_bdaddr_t addr = {{
        (uint8_t)(bytes[0]&0xff),
        (uint8_t)(bytes[1]&0xff),
        (uint8_t)(bytes[2]&0xff),
        (uint8_t)(bytes[3]&0xff),
        (uint8_t)(bytes[4]&0xff),
        (uint8_t)(bytes[5]&0xff)}};

    *out_addr = addr;

    return true;
}

static bool char2hex(char ch, uint8_t *hex)
{
    if ((ch >= '0') && (ch <= '9')) {
        *hex = ch - '0';
    } else if ((ch >= 'a') && (ch <= 'f')) {
        *hex = ch - 'a' + 10;
    } else if ((ch >= 'A') && (ch <= 'F')) {
        *hex = ch - 'A' + 10;
    } else {
        return false;
    }
    return true;
}

POSSIBLY_UNUSED static bool strdec2uint16hex(const char*str, uint32 len, uint16_t *data)
{
    uint8_t id = 0;
    uint8_t arry_id = 0;
    uint16_t data_temp = 0;
    uint8_t carry_bit = 0;
    uint8_t arry[4] = {0};
    uint8_t remain_len = len;
    if (remain_len >4) {
        return false;
    }
    carry_bit = remain_len;
    for (;remain_len > 0; remain_len--) {
        if (!char2hex(str[id], &arry[arry_id])) {
            return false;
        }
        id++;
        arry_id++;
    }

    if (carry_bit == 4) {
        data_temp = arry[0]*1000 + arry[1]*100 + arry[2]*10 + arry[3];
    }
    else if (carry_bit == 3) {
        data_temp = arry[0]*100 + arry[1]*10 + arry[2];
    } else if (carry_bit == 2) {
        data_temp = arry[0]*10 + arry[1];
    } else if (carry_bit == 1) {
        data_temp = arry[0];
    } else {
        return false;
    }
    *data = data_temp;
    return true;
}

#define APP_BT_CMD_TABLE_MAX    20
typedef struct
{
    struct
    {
        uint8_t cmd_number;
        const app_bt_host_cmd_table_t* table_list;
    } cmd_table[APP_BT_CMD_TABLE_MAX];
} app_bt_cmd_env_t;

extern void bt_audio_local_volume_up();
extern void bt_audio_volume_down();

bool app_bt_host_add_cmd_table(uint8_t cmd_number, const app_bt_host_cmd_table_t *cmd_table)
{
    DEBUG_INFO(0, "app_bt_host_add_cmd table not init becuase open UTILS_ESHELL_EN");
    return false;
}

unsigned int app_bt_host_cmd_callback(unsigned char *buf, unsigned int length)
{
    DEBUG_INFO(0, "app_bt_host_cmd table not init becuase open UTILS_ESHELL_EN");
    return 0;
}

void app_bt_host_cmd_init(void)
{
    DEBUG_INFO(0, "bt host cmd table not init becuase open UTILS_ESHELL_EN");
    return;
}

void app_bt_host_cmd_deinit(void)
{
    DEBUG_INFO(0, "bt host cmd table not init becuase open UTILS_ESHELL_EN");
}

static void app_bt_trace_rx_sleep(int argc, char* argv[])
{
#if defined(CHIP_SUBSYS_BTH) && defined(BTH_TRC_TO_SYS)
    TRACE(0, "%s: invalid cmd", __func__);
    return;
#else
    hal_trace_rx_sleep();
#endif
}

static void app_bt_flush_nv_test(int argc, char* argv[])
{
    nv_record_flash_flush();
}

static void app_bt_show_device_linkkey_test(int argc, char* argv[])
{
    DEBUG_INFO(1, "%s", __func__);
    nv_record_all_ddbrec_print();
}

static void app_bt_hci_print_statistic(int argc, char* argv[])
{
    btif_hci_print_statistic();
}

static void app_enable_hci_cmd_evt_debug(int argc, char* argv[])
{
    btif_hci_enable_cmd_evt_debug(true);
}

static void app_disable_hci_cmd_evt_debug(int argc, char* argv[])
{
    btif_hci_enable_cmd_evt_debug(false);
}

static void app_enable_hci_tx_flow_debug(int argc, char* argv[])
{
    btif_hci_enable_tx_flow_debug(true);
}

static void app_disable_hci_tx_flow_debug(int argc, char* argv[])
{
    btif_hci_enable_tx_flow_debug(false);
}

static void app_ota_mode_enter_test(int argc, char* argv[])
{
    app_otaMode_enter(NULL, NULL);
}

static void app_bt_sink_stop_sniff_test(int argc, char* argv[])
{
#ifndef BLE_ONLY_ENABLED
    struct BT_DEVICE_T *curr_device = app_bt_get_device(BT_DEVICE_ID_1);
    if (curr_device)
    {
        DUMP8("%02x ", &curr_device->remote, 6);
        btif_me_stop_sniff(curr_device->acl_conn_hdl);
    }
#endif
}

static void app_bt_source_stop_sniff_test(int argc, char* argv[])
{
#ifndef BLE_ONLY_ENABLED
    struct BT_DEVICE_T *curr_device = app_bt_get_device(BT_DEVICE_ID_1);
    if (curr_device)
    {
        DUMP8("%02x ", &curr_device->remote, 6);
        btif_me_stop_sniff(curr_device->acl_conn_hdl);
    }
#endif
}

#ifdef APP_SPP_DEMO
extern void app_spp_main(const bt_bdaddr_t *addr);
extern bool app_spp_serial_port_send_data(bt_bdaddr_t *remote, const uint8_t* ptrData, uint16_t length);
extern void app_spp_example_set_rx_loopback_mode(bool flag);
static void app_bt_client_spp_test(int argc, char* argv[])
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(BT_DEVICE_ID_1);
    if (!curr_device)
    {
        return;
    }
    app_spp_main(&curr_device->remote);
}

static void app_service_spp_test(int argc, char* argv[])
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(BT_DEVICE_ID_1);
    if (!curr_device)
    {
        return;
    }
    app_spp_main(NULL);
}

static void app_spp_data_test(int argc, char* argv[])
{
    uint8_t test_data[]={'s', 'p', 'p', ' ', 's', 'e', 'n', 'd', ' ', 'd', 'a', 't', 'a', ' ', 't', 'e', 's', 't', '\0'};
    struct BT_DEVICE_T *curr_device = app_bt_get_device(BT_DEVICE_ID_1);
    if (!curr_device)
    {
        return;
    }
    app_spp_serial_port_send_data(&curr_device->remote, &test_data[0], 19);
}

static void app_spp_set_loopback_test(int argc, char* argv[])
{
    app_spp_example_set_rx_loopback_mode(true);
}

static void app_spp_clear_loopback_test(int argc, char* argv[])
{
    app_spp_example_set_rx_loopback_mode(false);
}
#endif

#ifndef BLE_ONLY_ENABLED
void app_bt_key_handle_func_click_test(int argc, char* argv[])
{
    bt_key_handle_func_click();
}

void app_bt_key_handle_func_doubleclick_test(int argc, char* argv[])
{
    bt_key_handle_func_doubleclick();
}

void app_bt_key_handle_func_longpress_test(int argc, char* argv[])
{
    bt_key_handle_func_longpress();
}

void app_bt_start_both_scan_test(int argc, char* argv[])
{
    app_bt_set_access_mode(BTIF_BAM_GENERAL_ACCESSIBLE);
}

void app_bt_stop_both_scan_test(int argc, char* argv[])
{
    app_bt_set_access_mode(BTIF_BAM_NOT_ACCESSIBLE);
}

void app_bt_disconnect_acl_link_test(int argc, char* argv[])
{
    app_bt_disconnect_acl_link();
}

void app_bt_pts_rfc_register_channel_test(int argc, char* argv[])
{
    btif_pts_rfc_register_channel();
}

void app_bt_pts_pts_rfc_close_test(int argc, char* argv[])
{
    btif_pts_rfc_close();
}
#endif

#if defined(BT_HFP_TEST_SUPPORT)
static void app_bt_pts_create_hf_channel_test(int argc, char* argv[])
{
    app_bt_reconnect_hfp_profile(app_bt_get_pts_address());
}

static void app_bt_pts_hf_disc_service_link_test(int argc, char* argv[])
{
    app_pts_hf_disc_service_link();
}

static void app_bt_pts_hf_create_audio_link_test(int argc, char* argv[])
{
    app_pts_hf_create_audio_link();
}

static void app_bt_pts_hf_disc_audio_link_test(int argc, char* argv[])
{
    app_pts_hf_disc_audio_link();
}

static void app_bt_pts_hf_send_key_pressed_test(int argc, char* argv[])
{
    app_pts_hf_send_key_pressed();
}

static void app_bt_pts_hf_answer_call_test(int argc, char* argv[])
{
    app_pts_hf_answer_call();
}

static void app_bt_pts_hf_hangup_call_test(int argc, char* argv[])
{
    app_pts_hf_hangup_call();
}

static void app_bt_pts_hf_dial_number_test(int argc, char* argv[])
{
    app_pts_hf_dial_number();
}

static void app_bt_pts_hf_dial_number_memory_index_test(int argc, char* argv[])
{
    app_pts_hf_dial_number_memory_index();
}

static void app_bt_pts_hf_dial_number_invalid_memory_index_test(int argc, char* argv[])
{
    app_pts_hf_dial_number_invalid_memory_index();
}

static void app_bt_pts_hf_redial_call_test(int argc, char* argv[])
{
    app_pts_hf_redial_call();
}

static void app_bt_pts_hf_release_active_call_test(int argc, char* argv[])
{
    app_pts_hf_release_active_call();
}

static void app_bt_pts_hf_release_active_call_2_test(int argc, char* argv[])
{
    app_pts_hf_release_active_call_2();
}

static void app_bt_pts_hf_hold_active_call_test(int argc, char* argv[])
{
    app_pts_hf_hold_active_call();
}

static void app_bt_pts_hf_hold_active_call_2_test(int argc, char* argv[])
{
    app_pts_hf_hold_active_call_2();
}

static void app_bt_pts_hf_hold_call_transfer_test(int argc, char* argv[])
{
    app_pts_hf_hold_call_transfer();
}

static void app_bt_pts_hf_vr_enable_test(int argc, char* argv[])
{
    app_pts_hf_vr_enable();
}

static void app_bt_pts_hf_vr_disable_test(int argc, char* argv[])
{
    app_pts_hf_vr_disable();
}

static void app_bt_pts_hf_list_current_calls_test(int argc, char* argv[])
{
    app_pts_hf_list_current_calls();
}

static void app_bt_pts_hf_report_mic_volume_test(int argc, char* argv[])
{
    app_pts_hf_report_mic_volume();
}

static void app_bt_pts_hf_attach_voice_tag_test(int argc, char* argv[])
{
    app_pts_hf_attach_voice_tag();
}

static void app_bt_pts_hf_update_ind_value_test(int argc, char* argv[])
{
    app_pts_hf_update_ind_value();
}

static void app_bt_pts_hf_ind_activation_test(int argc, char* argv[])
{
    app_pts_hf_ind_activation();
}

static void app_bt_pts_hf_acs_bv_09_i_set_enable_test(int argc, char* argv[])
{
    bthost_cfg_t* bt_host_cfg = bt_host_get_cfg();
    bt_host_cfg->hfp_hf_pts_acs_bv_09_i = true;
}

static void app_bt_pts_hf_acs_bv_09_i_set_disable_test(int argc, char* argv[])
{
    bthost_cfg_t* bt_host_cfg = bt_host_get_cfg();
    bt_host_cfg->hfp_hf_pts_acs_bv_09_i = false;
}

static void app_bt_pts_hf_acs_bi_13_i_set_enable_test(int argc, char* argv[])
{
    app_pts_hf_acs_bi_13_i_set_enable();
}

static void app_bt_pts_hf_acs_bi_13_i_set_disable_test(int argc, char* argv[])
{
    app_pts_hf_acs_bi_13_i_set_disable();
}

static void app_bt_pts_hf_siri_voice_enable_test(int argc, char* argv[])
{
    app_pts_hfp_siri_voice_enable();
}

static void app_bt_pts_hf_siri_voice_disable_test(int argc, char* argv[])
{
    app_pts_hfp_siri_voice_disable();
}

#if defined(BT_HFP_AG_ROLE)
static void app_bt_pts_hf_ag_set_connectable_state(int argc, char* argv[])
{
    app_bt_source_set_connectable_state(true);
}

static void app_bt_pts_create_hf_ag_channel(int argc, char* argv[])
{
    //app_bt_source_set_connectable_state(true);
    bt_source_reconnect_hfp_profile(app_bt_get_pts_address());
}

static void app_bt_pts_hf_ag_create_audio_link(int argc, char* argv[])
{
    bt_source_create_audio_link(app_bt_get_pts_address());
}

static void app_bt_pts_hf_ag_disc_audio_link(int argc, char* argv[])
{
    bt_source_disc_audio_link(app_bt_get_pts_address());
}

static void app_bt_pts_hf_ag_send_mobile_signal_level(int argc, char* argv[])
{
    btif_ag_send_mobile_signal_level(app_bt_source_find_device(app_bt_get_pts_address())->base_device->hf_channel,3);
}

static void app_bt_pts_hf_ag_send_mobile_signal_level_0(int argc, char* argv[])
{
    btif_ag_send_mobile_signal_level(app_bt_source_find_device(app_bt_get_pts_address())->base_device->hf_channel,0);
}

static void app_bt_pts_hf_ag_send_service_status(int argc, char* argv[])
{
    btif_ag_send_service_status(app_bt_source_find_device(app_bt_get_pts_address())->base_device->hf_channel,true);
}

static void app_bt_pts_hf_ag_send_service_status_0(int argc, char* argv[])
{
    btif_ag_send_service_status(app_bt_source_find_device(app_bt_get_pts_address())->base_device->hf_channel,false);
}

static void app_bt_pts_hf_ag_send_call_active_status(int argc, char* argv[])
{
    btif_ag_send_call_active_status(app_bt_source_find_device(app_bt_get_pts_address())->base_device->hf_channel,true);
}

static void app_bt_pts_hf_ag_hangup_call(int argc, char* argv[])
{
    btif_ag_send_call_active_status(app_bt_source_find_device(app_bt_get_pts_address())->base_device->hf_channel,false
);
}

static void app_bt_pts_hf_ag_send_callsetup_status(int argc, char* argv[])
{
    btif_ag_send_callsetup_status(app_bt_source_find_device(app_bt_get_pts_address())->base_device->hf_channel,1);
}

static void app_bt_pts_hf_ag_send_callsetup_status_0(int argc, char* argv[])
{
    btif_ag_send_callsetup_status(app_bt_source_find_device(app_bt_get_pts_address())->base_device->hf_channel,0);
}

static void app_bt_pts_hf_ag_send_callsetup_status_2(int argc, char* argv[])
{
    btif_ag_send_callsetup_status(app_bt_source_find_device(app_bt_get_pts_address())->base_device->hf_channel,2);
}

static void app_bt_pts_hf_ag_send_callsetup_status_3(int argc, char* argv[])
{
    btif_ag_send_callsetup_status(app_bt_source_find_device(app_bt_get_pts_address())->base_device->hf_channel,3);
}

static void app_bt_pts_hf_ag_enable_roam(int argc, char* argv[])
{
    btif_ag_send_mobile_roam_status(app_bt_source_find_device(app_bt_get_pts_address())->base_device->hf_channel,true);
}

static void app_bt_pts_hf_ag_disable_roam(int argc, char* argv[])
{
    btif_ag_send_mobile_roam_status(app_bt_source_find_device(app_bt_get_pts_address())->base_device->hf_channel,false
);
}

static void app_bt_pts_hf_ag_send_mobile_battery_level(int argc, char* argv[])
{
    btif_ag_send_mobile_battery_level(app_bt_source_find_device(app_bt_get_pts_address())->base_device->hf_channel,3);
}

static void app_bt_pts_hf_ag_send_full_battery_level(int argc, char* argv[])
{
    btif_ag_send_mobile_battery_level(app_bt_source_find_device(app_bt_get_pts_address())->base_device->hf_channel,5);
}

static void app_bt_pts_hf_ag_send_battery_level_0(int argc, char* argv[])
{
    btif_ag_send_mobile_battery_level(app_bt_source_find_device(app_bt_get_pts_address())->base_device->hf_channel,0);
}

static void app_bt_pts_hf_ag_send_calling_ring(int argc, char* argv[])
{
    //const char* number = NULL;
    char number[] = "1234567";
    btif_ag_send_calling_ring(app_bt_source_find_device(app_bt_get_pts_address())->base_device->hf_channel,number);
}

static void app_bt_pts_hf_ag_enable_inband_ring_tone(int argc, char* argv[])
{
    btif_ag_set_inband_ring_tone(app_bt_source_find_device(app_bt_get_pts_address())->base_device->hf_channel,true);
}

static void app_bt_pts_hf_ag_place_a_call(int argc, char* argv[])
{
    app_bt_pts_hf_ag_send_callsetup_status();
    app_bt_pts_hf_ag_send_calling_ring();
    app_bt_pts_hf_ag_send_call_active_status();
    app_bt_pts_hf_ag_send_callsetup_status_0();
    app_bt_pts_hf_ag_create_audio_link();
}

static void app_bt_pts_hf_ag_ongoing_call(int argc, char* argv[])
{
    app_bt_pts_hf_ag_send_callsetup_status_2();
    app_bt_pts_hf_ag_send_callsetup_status_3();
}

static void app_bt_pts_hf_ag_ongoing_call_setup(int argc, char* argv[])
{
    app_bt_pts_hf_ag_send_callsetup_status_2();
    app_bt_pts_hf_ag_create_audio_link();
    osDelay(100);
    app_bt_pts_hf_ag_send_callsetup_status_3();
    app_bt_pts_hf_ag_send_call_active_status();
    app_bt_pts_hf_ag_send_callsetup_status_0();
}

static void app_bt_pts_hf_ag_answer_incoming_call(int argc, char* argv[])
{
    app_bt_pts_hf_ag_send_callsetup_status();
    app_bt_pts_hf_ag_send_call_active_status();
    app_bt_pts_hf_ag_send_callsetup_status_0();
    app_bt_pts_hf_ag_create_audio_link();
}

static void app_bt_pts_hf_ag_clear_last_dial_number(int argc, char* argv[])
{
    btif_ag_set_last_dial_number(app_bt_source_find_device(app_bt_get_pts_address())->base_device->hf_channel,false);
}

static void app_bt_pts_hf_ag_send_call_waiting_notification(int argc, char* argv[])
{
    char number[] = "7654321";
    btif_ag_send_call_waiting_notification(app_bt_source_find_device(app_bt_get_pts_address())->base_device->
hf_channel,number);
}

static void app_bt_pts_hf_ag_send_callheld_status(int argc, char* argv[])
{
    btif_ag_send_callheld_status(app_bt_source_find_device(app_bt_get_pts_address())->base_device->hf_channel,1);
}

static void app_bt_pts_hf_ag_send_callheld_status_0(int argc, char* argv[])
{
    btif_ag_send_callheld_status(app_bt_source_find_device(app_bt_get_pts_address())->base_device->hf_channel,0);
}

static void app_bt_pts_hf_ag_send_status_3_0_4_1(int argc, char* argv[])
{
    app_bt_pts_hf_ag_send_callsetup_status_0();
    app_bt_pts_hf_ag_send_callheld_status();
}

static void app_bt_pts_hf_ag_set_pts_enable(int argc, char* argv[])
{
    bthost_cfg_t* bt_host_cfg = bt_host_get_cfg();

    bt_host_cfg->hfp_ag_pts_enable = true;
}

static void app_bt_pts_hf_ag_set_pts_ecs_01(int argc, char* argv[])
{
    bthost_cfg_t* bt_host_cfg = bt_host_get_cfg();

    bt_host_cfg->hfp_ag_pts_ecs_01 = true;
}

static void app_bt_pts_hf_ag_set_pts_ecs_02(int argc, char* argv[])
{
    bthost_cfg_t* bt_host_cfg = bt_host_get_cfg();

    bt_host_cfg->hfp_ag_pts_ecs_02 = true;
}

static void app_bt_pts_hf_ag_set_pts_ecc(int argc, char* argv[])
{
    bthost_cfg_t* bt_host_cfg = bt_host_get_cfg();

    bt_host_cfg->hfp_ag_pts_ecc = true;
}
#endif
#endif

#if defined(BT_A2DP_TEST_SUPPORT)
static void app_bt_pts_create_av_channel_test(int argc, char* argv[])
{
    btif_pts_av_create_channel(app_bt_get_pts_address());
}

static void app_bt_pts_av_disc_channel_test(int argc, char* argv[])
{
    app_pts_av_disc_channel();
}

static void app_bt_enable_tone_intrrupt_a2dp_test(int argc, char* argv[])
{
    app_bt_manager.config.a2dp_prompt_play_only_when_avrcp_play_received = false;
}

static void app_bt_disable_tone_intrrupt_a2dp_test(int argc, char* argv[])
{
    app_bt_manager.config.a2dp_prompt_play_only_when_avrcp_play_received = true;
}

static void app_bt_enable_a2dp_delay_prompt_test(int argc, char* argv[])
{
    app_bt_manager.config.a2dp_delay_prompt_play = true;
}

static void app_bt_disable_a2dp_delay_prompt_test(int argc, char* argv[])
{
    app_bt_manager.config.a2dp_delay_prompt_play = false;
}

static void app_bt_disable_a2dp_aac_codec_test(int argc, char* argv[])
{
    app_bt_a2dp_disable_aac_codec(true);
}

static void app_bt_disable_a2dp_vendor_codec_test(int argc, char* argv[])
{
    app_bt_a2dp_disable_vendor_codec(true);
}

static void app_bt_pts_av_create_media_channel_test(int argc, char* argv[])
{
    btif_pts_av_create_media_channel();
}

static void app_bt_pts_av_close_channel_test(int argc, char* argv[])
{
    app_pts_av_close_channel();
}

static void app_bt_pts_av_send_getconf_test(int argc, char* argv[])
{
    btif_pts_av_send_getconf();
}

static void app_bt_pts_av_send_reconf_test(int argc, char* argv[])
{
    btif_pts_av_send_reconf();
}

static void app_bt_pts_av_send_open_test(int argc, char* argv[])
{
    btif_pts_av_send_open();
}

static void app_bt_pts_av_send_start_test(int argc, char* argv[])
{
    btif_pts_av_send_start();
}
#endif

#if defined(BT_AVRCP_TEST_SUPPORT) || defined(BT_HFP_TEST_SUPPORT)
#ifndef BTH_SUBSYS_ONLY
static void app_bt_audio_local_volume_up_test(int argc, char* argv[])
{
    bt_audio_local_volume_up();
}

static void app_bt_audio_volume_down_test(int argc, char* argv[])
{
    bt_audio_volume_down();
}
#endif
#endif

#ifdef BT_USE_COHEAP_ALLOC
static void app_bt_coheap_statistics_dump(int argc, char* argv[])
{
    cobuf_print_statistic();
}

static void app_bt_coheap_enable_debug(int argc, char* argv[])
{
    cobuf_enable_debug(true);
}

static void app_bt_coheap_disable_debug(int argc, char* argv[])
{
    cobuf_enable_debug(false);
}
#endif

#if defined(NORMAL_TEST_MODE_SWITCH)
static void app_bt_enter_signal_testmode_test(int argc, char* argv[])
{
    app_enter_signal_testmode();
}

static void app_bt_exit_signal_testmode_test(int argc, char* argv[])
{
    app_exit_signal_testmode();
}

static void app_bt_reboot_and_enter_nosignal_test(int argc, char* argv[])
{
    app_reboot_and_enter_nosignal_testmode();
}

static void app_bt_reboot_and_enter_signal_test(int argc, char* argv[])
{
    app_reboot_and_enter_signal_testmode();
}

static void app_bt_enter_nosignal_tx_test(int argc, char* argv[])
{
    app_enter_nosignal_tx_testmode();
}

static void app_bt_enter_nosignal_rx_test(int argc, char* argv[])
{
    app_enter_nosignal_rx_testmode();
}

static void app_bt_exit_nosignal_trx_test(int argc, char* argv[])
{
    app_exit_nosignal_trx_testmode();
}

static void app_bt_enter_ble_tx_v1_test(int argc, char* argv[])
{
    app_enter_ble_tx_v1_testmode();
}

static void app_bt_enter_ble_rx_v1_test(int argc, char* argv[])
{
    app_enter_ble_rx_v1_testmode();
}

static void app_bt_enter_ble_tx_v2_test(int argc, char* argv[])
{
    app_enter_ble_tx_v2_testmode();
}

static void app_bt_enter_ble_rx_v2_test(int argc, char* argv[])
{
    app_enter_ble_rx_v2_testmode();
}

static void app_bt_enter_ble_tx_v3_test(int argc, char* argv[])
{
    app_enter_ble_tx_v2_testmode();
}

static void app_bt_enter_ble_rx_v3_test(int argc, char* argv[])
{
    app_enter_ble_rx_v2_testmode();
}

static void app_bt_enter_ble_tx_v4_test(int argc, char* argv[])
{
    app_enter_ble_tx_v2_testmode();
}

static void app_bt_exit_ble_trx_test(int argc, char* argv[])
{
    app_exit_ble_trx_testmode();
}

static void app_bt_enter_normal_mode_test(int argc, char* argv[])
{
    app_enter_normal_mode();
}
#endif

#if defined(BT_SOURCE)
static void app_bt_source_start_search_test(int argc, char* argv[])
{
    app_bt_stop_inquiry();
    app_bt_source_search_device();
}

static void app_bt_source_stop_search_test(int argc, char* argv[])
{
    app_bt_stop_inquiry();
}

static void app_bts_start_search_test(int argc, char* argv[])
{
    app_bt_stop_inquiry();
#if defined(BT_WATCH_SERVICE_ON) && defined(BT_WATCH_SERVICE_DISTRIBUTE)
    bts_a2dp_source_search();
#endif
}

static void app_bts_stop_search_test(int argc, char* argv[])
{
    //sbts_a2dp_source_stop_search();
}

static void app_bts_slow_search_test(int argc, char* argv[])
{
    bthost_cfg_t* bt_host_cfg = bt_host_get_cfg();

    bt_host_cfg->force_normal_search = false;
    bt_host_cfg->watch_is_sending_spp = true;
    app_bts_start_search_test(0, NULL);
    bt_host_cfg->watch_is_sending_spp = false;
}

static void app_bts_force_normal_search_test(int argc, char* argv[])
{
    bthost_cfg_t* bt_host_cfg = bt_host_get_cfg();

    bt_host_cfg->watch_is_sending_spp = false;
    bt_host_cfg->force_normal_search = true;
    app_bts_start_search_test(0, NULL);
    bt_host_cfg->force_normal_search = false;
}

static void app_bts_boost_up_test(int argc, char* argv[])
{

}

static void app_bt_set_curr_nv_source(int argc, char* argv[])
{
    uint8_t device_id = app_bt_find_connected_device();
    struct BT_DEVICE_T *curr_device = NULL;
    btif_device_record_t record;

    if (device_id != BT_DEVICE_INVALID_ID)
    {
        curr_device = app_bt_get_device(device_id);
        if (ddbif_find_record(&curr_device->remote, &record) == BT_STS_SUCCESS)
        {
            ddbif_delete_record(&record.bdAddr);
            record.for_bt_source = true;
            ddbif_add_record(&record);
        }
        else
        {
            DEBUG_INFO(1, "%s no record", __func__);
        }
    }
    else
    {
        DEBUG_INFO(1, "%s device not found", __func__);
    }
}

static void app_bt_clear_curr_nv_source(int argc, char* argv[])
{
    uint8_t device_id = app_bt_find_connected_device();
    struct BT_DEVICE_T *curr_device = NULL;
    btif_device_record_t record;

    if (device_id != BT_DEVICE_INVALID_ID)
    {
        curr_device = app_bt_get_device(device_id);
        if (ddbif_find_record(&curr_device->remote, &record) == BT_STS_SUCCESS)
        {
            ddbif_delete_record(&record.bdAddr);
            record.for_bt_source = false;
            ddbif_add_record(&record);
        }
        else
        {
            DEBUG_INFO(1, "%s no record", __func__);
        }
    }
    else
    {
        DEBUG_INFO(1, "%s device not found", __func__);
    }
}

static bool a2dp_source_inited = false;
typedef int wal_status_t;
extern "C" void bts_a2dp_source_test_start_cmd(char* param);
extern "C" wal_status_t mock_music_init(void);

static void app_bt_start_a2dp_source_test(int argc, char* argv[])
{
    if (!a2dp_source_inited)
    {
        //sbts_a2dp_source_test_start_cmd(NULL);
        //mock_music_init();
        //a2dp_source_inited = true;
    }
}

static void app_bt_source_music_test(int argc, char* argv[])
{
    app_bt_start_a2dp_source_test(0, NULL);
}

static void app_bt_local_music_test(int argc, char* argv[])
{
    app_bt_start_a2dp_source_test(0, NULL);
}

static void app_bt_connect_earbud_link_test(int argc, char* argv[])
{
    bt_bdaddr_t addr;

    if (!app_bt_scan_bdaddr_from_string(argv[1], strlen(argv[1]), &addr))
    {
        return;
    }

    DEBUG_INFO(2, "%s %02x:%02x:%02x:%02x:%02x:%02x", __func__,
            addr.address[0], addr.address[1], addr.address[2],
            addr.address[3], addr.address[4], addr.address[5]);

    app_bt_start_a2dp_source_test(0, NULL);

    bt_source_perform_profile_reconnect(&addr);
}

static void app_bt_connect_earbud_a2dp_test(int argc, char* argv[])
{
    bt_bdaddr_t addr;

    if (!app_bt_scan_bdaddr_from_string(argv[1], strlen(argv[1]), &addr))
    {
        return;
    }

    DEBUG_INFO(2, "%s %02x:%02x:%02x:%02x:%02x:%02x", __func__,
            addr.address[0], addr.address[1], addr.address[2],
            addr.address[3], addr.address[4], addr.address[5]);

    app_bt_start_a2dp_source_test(0, NULL);

    bt_source_reconnect_a2dp_profile(&addr);
}

static void app_bt_connect_earbud_avrcp_test(int argc, char* argv[])
{
    bt_bdaddr_t addr;

    if (!app_bt_scan_bdaddr_from_string(argv[1], strlen(argv[1]), &addr))
    {
        return;
    }

    DEBUG_INFO(2, "%s %02x:%02x:%02x:%02x:%02x:%02x", __func__,
            addr.address[0], addr.address[1], addr.address[2],
            addr.address[3], addr.address[4], addr.address[5]);

    bt_source_reconnect_avrcp_profile(&addr);
}

static void app_bt_connect_earbud_hfp_test(int argc, char* argv[])
{
    bt_bdaddr_t addr;

    if (!app_bt_scan_bdaddr_from_string(argv[1], strlen(argv[1]), &addr))
    {
        return;
    }

    DEBUG_INFO(2, "%s %02x:%02x:%02x:%02x:%02x:%02x", __func__,
            addr.address[0], addr.address[1], addr.address[2],
            addr.address[3], addr.address[4], addr.address[5]);

    bt_source_reconnect_hfp_profile(&addr);
}

static void app_bt_connect_mobile_link_test(int argc, char* argv[])
{
    bt_bdaddr_t addr;

    if (!app_bt_scan_bdaddr_from_string(argv[1], strlen(argv[1]), &addr))
    {
        return;
    }

    DEBUG_INFO(2, "%s %02x:%02x:%02x:%02x:%02x:%02x", __func__,
            addr.address[0], addr.address[1], addr.address[2],
            addr.address[3], addr.address[4], addr.address[5]);

    app_bt_reconnect_a2dp_profile(&addr, A2DP_ROLE_SNK);

    app_bt_reconnect_hfp_profile(&addr);
}

static void app_bt_connect_mobile_a2dp_test(int argc, char* argv[])
{
    bt_bdaddr_t addr;

    if (!app_bt_scan_bdaddr_from_string(argv[1], strlen(argv[1]), &addr))
    {
        return;
    }

    DEBUG_INFO(2, "%s %02x:%02x:%02x:%02x:%02x:%02x", __func__,
            addr.address[0], addr.address[1], addr.address[2],
            addr.address[3], addr.address[4], addr.address[5]);

    app_bt_reconnect_a2dp_profile(&addr, A2DP_ROLE_SNK);
}

static void app_bt_connect_mobile_avrcp_test(int argc, char* argv[])
{
    bt_bdaddr_t addr;

    if (!app_bt_scan_bdaddr_from_string(argv[1], strlen(argv[1]), &addr))
    {
        return;
    }

    DEBUG_INFO(2, "%s %02x:%02x:%02x:%02x:%02x:%02x", __func__,
            addr.address[0], addr.address[1], addr.address[2],
            addr.address[3], addr.address[4], addr.address[5]);

    app_bt_reconnect_avrcp_profile(&addr);
}

static void app_bt_connect_mobile_hfp_test(int argc, char* argv[])
{
    bt_bdaddr_t addr;

   if (!app_bt_scan_bdaddr_from_string(argv[1], strlen(argv[1]), &addr))
    {
        return;
    }

    DEBUG_INFO(2, "%s %02x:%02x:%02x:%02x:%02x:%02x", __func__,
            addr.address[0], addr.address[1], addr.address[2],
            addr.address[3], addr.address[4], addr.address[5]);

    app_bt_reconnect_hfp_profile(&addr);
}

#if defined(BT_HFP_AG_ROLE)
static void app_bt_ag_create_audio_link_test(int argc, char* argv[])
{
    bt_bdaddr_t addr;

    if (!app_bt_scan_bdaddr_from_string(argv[1], strlen(argv[1]), &addr))
    {
        return;
    }

    DEBUG_INFO(2, "%s %02x:%02x:%02x:%02x:%02x:%02x", __func__,
            addr.address[0], addr.address[1], addr.address[2],
            addr.address[3], addr.address[4], addr.address[5]);

    bt_source_create_audio_link(&addr);
}

static void app_bt_ag_disc_audio_link_test(int argc, char* argv[])
{
    bt_bdaddr_t addr;

    if (!app_bt_scan_bdaddr_from_string(argv[1], strlen(argv[1]), &addr))
    {
        return;
    }

    DEBUG_INFO(2, "%s %02x:%02x:%02x:%02x:%02x:%02x", __func__,
            addr.address[0], addr.address[1], addr.address[2],
            addr.address[3], addr.address[4], addr.address[5]);

    bt_source_disc_audio_link(&addr);
}
#endif

#if defined(mHDT_SUPPORT)
static bool app_bt_scan_tx_rx_rate_from_string(const char* param, uint32_t len, uint8_t *tx_rate, uint8_t *rx_rate)
{
    //"tx:4rx:4"; at least 8 tyte
    if (len < 8)
    {
        DEBUG_INFO(0, "%s wrong len %d '%s'", __func__, len, param);
        return false;
    }

    int tx_rate_temp;
    int rx_rate_temp;

    DEBUG_INFO(0,  "%s '%s'", __func__,param);

    if (2 != sscanf(param, "tx:%d rx:%x",&tx_rate_temp, &rx_rate_temp))
    {
        DEBUG_INFO(0, "%s parse rate failed %s", __func__, param);
        return false;
    }
    *tx_rate = (uint8_t)tx_rate_temp;
    *rx_rate = (uint8_t)rx_rate_temp;

    return true;
}

static void app_bt_source_enter_mhdt_mode_test(int argc, char* argv[])
{
    uint8 tx_rates;
    uint8 rx_rates;
    if (!app_bt_scan_tx_rx_rate_from_string(param, param_len, &tx_rates, &rx_rates))
    {
        return;
    }
    app_a2dp_source_enter_mhdt_mode(tx_rates, rx_rates);
}

static void app_bt_source_exit_mhdt_mode_test(int argc, char* argv[])
{
    DEBUG_INFO(0,  "%s '%s'", __func__,param);
    app_a2dp_source_exit_mhdt_mode();
}

#endif

static void app_bt_source_disconnect_link(int argc, char* argv[])
{
    app_bt_source_disconnect_all_connections(true);
}

extern void a2dp_source_pts_send_sbc_packet(void);
static void app_bt_pts_source_send_sbc_packet(int argc, char* argv[])
{
    a2dp_source_pts_send_sbc_packet();
}

static void app_bt_pts_source_cretae_media_channel_test(int argc, char* argv[])
{
    btif_pts_source_cretae_media_channel();
}

static void app_bt_pts_source_send_close_cmd_test(int argc, char* argv[])
{
    btif_pts_source_send_close_cmd();
}

static void app_bt_pts_source_send_get_configuration_cmd_test(int argc, char* argv[])
{
    btif_pts_source_send_get_configuration_cmd();
}

static void app_bt_pts_source_send_reconfigure_cmd_test(int argc, char* argv[])
{
    btif_pts_source_send_reconfigure_cmd();
}

static void app_bt_pts_source_send_abort_cmd_test(int argc, char* argv[])
{
    btif_pts_source_send_abort_cmd();
}

static void app_bt_pts_source_send_suspend_cmd_test(int argc, char* argv[])
{
    btif_pts_source_send_suspend_cmd();
}

static void app_bt_source_reconfig_codec_to_sbc(int argc, char* argv[])
{
    struct BT_SOURCE_DEVICE_T *source_device = app_bt_source_get_device(BT_DEVICE_ID_1);
    app_bt_a2dp_reconfig_to_sbc(source_device->base_device->a2dp_connected_stream);
}

static void app_bt_pts_source_set_get_all_cap_flag(int argc, char* argv[])
{
    bthost_cfg_t* bt_host_cfg = bt_host_get_cfg();

    bt_host_cfg->source_get_all_cap_flag = true;
}

static void app_bt_pts_source_set_unknown_cmd_flag(int argc, char* argv[])
{
    bthost_cfg_t* bt_host_cfg = bt_host_get_cfg();

    bt_host_cfg->source_unknown_cmd_flag = true;
}

static void app_bt_pts_source_send_start_cmd_test(int argc, char* argv[])
{
    btif_pts_source_send_start_cmd();
}

#if defined(BT_HID_DEVICE)
static void app_bt_hid_host_connect_dev_req(const char* param, uint32_t param_len)
{
    bt_bdaddr_t addr;

    if (!app_bt_scan_bdaddr_from_string(param, param_len, &addr))
    {
        return;
    }

    DEBUG_INFO(2, "%s %02x:%02x:%02x:%02x:%02x:%02x", __func__,
            addr.address[0], addr.address[1], addr.address[2],
            addr.address[3], addr.address[4], addr.address[5]);

    bt_source_reconnect_hid_profile(&addr);
}
#endif // BT_HID_DEVICE

#endif // BT_SOURCE

static void app_bt_delete_device_linkkey_test(int argc, char* argv[])
{
    bt_bdaddr_t addr;

    if (!app_bt_scan_bdaddr_from_string(argv[1], strlen(argv[1]), &addr))
    {
        return;
    }

    DEBUG_INFO(2, "%s %02x:%02x:%02x:%02x:%02x:%02x", __func__,
            addr.address[0], addr.address[1], addr.address[2],
            addr.address[3], addr.address[4], addr.address[5]);

    nv_record_ddbrec_delete(&addr);

    DEBUG_INFO(0, "nv devices after delete:");
    nv_record_all_ddbrec_print();
}

void app_bt_set_linkey_test(int argc, char* argv[])
{
    btif_device_record_t record = {{{0}},0};
    uint32_t param_key[16] = {0};
    uint32_t param_addr[6] = {0};
    uint8_t linkkey[16] = {0};
    uint8_t address[6] = {0};
    uint8_t cod[3] = {0};
    const char* pos = argv[1] + 32;
    memcpy(param_key, argv[1], 32);
    memcpy(param_addr, pos, 12);
    sscanf(argv[1], "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", &param_key[0],
        &param_key[1],&param_key[2], &param_key[3],&param_key[4], &param_key[5],&param_key[6],
        &param_key[7],&param_key[8], &param_key[9],&param_key[10], &param_key[11],&param_key[12],
        &param_key[13],&param_key[14], &param_key[15]);
    sscanf(pos, "%02x%02x%02x%02x%02x%02x",&param_addr[0], &param_addr[1], &param_addr[2], &param_addr[3],
        &param_addr[4], &param_addr[5]);
    for(int i=0;i<16; i++){
        linkkey[i] = param_key[i];
    }
    for(int i=0;i<6; i++){
        address[i] = param_addr[i];
    }
    //DUMP8("%02x ", linkkey,16);
    //DUMP8("%02x ", address,6);

    memcpy(record.bdAddr.address, address, 6);
    memcpy(record.linkKey, linkkey, 16);
    memcpy(record.cod, cod, 3);

    record.keyType = 0x7;
    record.trusted = true;

    nv_record_add(section_usrdata_ddbrecord,(void *)&record);
}


#ifndef BLE_ONLY_ENABLED
static bool app_bt_is_pts_address(void *remote)
{
    return memcmp(remote, g_app_bt_pts_addr.address, sizeof(g_app_bt_pts_addr)) == 0;
}
#endif

static void app_bt_pts_set_address(int argc, char* argv[])
{
    bt_bdaddr_t addr;

    if (!app_bt_scan_bdaddr_from_string(argv[1], strlen(argv[1]), &addr))
    {
        return;
    }

    DEBUG_INFO(2, "%s %02x:%02x:%02x:%02x:%02x:%02x", __func__,
            addr.address[0], addr.address[1], addr.address[2],
            addr.address[3], addr.address[4], addr.address[5]);

    g_app_bt_pts_addr = addr;

#ifndef BLE_ONLY_ENABLED
    btif_register_is_pts_address_check_callback(app_bt_is_pts_address);
#endif
}

#ifdef AUDIO_LINEIN
static void app_bt_test_linein_start(int argc, char* argv[])
{
    Audio_device_t audio_device;
    audio_device.audio_device.device_id = BT_DEVICE_ID_1;
    audio_device.audio_device.device_type = AUDIO_TYPE_BT;
    audio_device.aud_id = MAX_RECORD_NUM;
    audio_player_play(BT_STREAM_LINEIN, &audio_device);
}

static void app_bt_test_linein_stop(int argc, char* argv[])
{
    Audio_device_t audio_device;
    audio_device.audio_device.device_id = BT_DEVICE_ID_1;
    audio_device.audio_device.device_type = AUDIO_TYPE_BT;
    audio_device.aud_id = MAX_RECORD_NUM;
    audio_player_playback_stop(BT_STREAM_LINEIN, &audio_device);
}
#endif

#ifndef BLE_ONLY_ENABLED
static void app_bt_set_access_mode_test(int argc, char* argv[])
{
    uint32_t access_mode = atoi(argv[1]);

    app_bt_set_access_mode(access_mode);
}

static void app_bt_access_mode_set_test(int argc, char* argv[])
{
    uint32_t access_mode = atoi(argv[1]);

    app_bt_set_access_mode(access_mode);
}

static void app_bt_connect_acl_test(int argc, char* argv[])
{
    bt_bdaddr_t bd_addr;

    if (argc != 2)
    {
        DEBUG_INFO(0, "%s: param num error=%d, should be 2", __func__, argc);
        return;
    }
    if (!app_bt_scan_bdaddr_from_string(argv[1], strlen(argv[1]), &bd_addr))
    {
        return;
    }

    POSSIBLY_UNUSED bt_status_t ret_val = bt_adapter_connect_acl(&bd_addr);
    DEBUG_INFO(0,"%s result:%d",__func__,ret_val);
}

static void app_bt_disconnect_acl_test(int argc, char* argv[])
{
    bt_bdaddr_t bd_addr;

    if (argc != 2)
    {
        DEBUG_INFO(0, "%s: param num error=%d, should be 2", __func__, argc);
        return;
    }
    if (!app_bt_scan_bdaddr_from_string(argv[1], strlen(argv[1]), &bd_addr))
    {
        return;
    }

    POSSIBLY_UNUSED bt_status_t ret_val = bt_adapter_disconnect_acl(&bd_addr);

    DEBUG_INFO(0,"%s result:%d",__func__,ret_val);
}

static void app_bt_connect_sco_test(int argc, char* argv[])
{
    bt_bdaddr_t bd_addr;

    if (argc != 2)
    {
        DEBUG_INFO(0, "%s: param num error=%d, should be 2", __func__, argc);
        return;
    }
    if (!app_bt_scan_bdaddr_from_string(argv[1], strlen(argv[1]), &bd_addr))
    {
        return;
    }

    bes_bt_sco_connect_req(&bd_addr);
    DEBUG_INFO(0,"%s end",__func__);
}

static void app_bt_start_inquiry_test(int argc, char* argv[])
{
    POSSIBLY_UNUSED bt_status_t ret_val = bt_adapter_start_inquiry();
    DEBUG_INFO(0,"%s result:%d",__func__,ret_val);
}

static void app_bt_cancel_inquiry_test(int argc, char* argv[])
{
    POSSIBLY_UNUSED bt_status_t ret_val = bt_adapter_cancel_inquiry();
    DEBUG_INFO(0,"%s result:%d",__func__,ret_val);
}

static void app_bt_set_eir_data_test(int argc, char* argv[])
{
    if (argc != 2)
    {
        DEBUG_INFO(0, "%s: param num error=%d, should be 2", __func__, argc);
        return;
    }

    bes_bt_me_set_extended_inquiry_response((uint8_t *)argv[1], strlen(argv[1]));
}

static void app_bt_accept_ssp_confirmation_test(int argc, char* argv[])
{
    bt_bdaddr_t bd_addr;

    if (argc != 2)
    {
        DEBUG_INFO(0, "%s: param num error=%d, should be 2", __func__, argc);
        return;
    }
    if (!app_bt_scan_bdaddr_from_string(argv[1], strlen(argv[1]), &bd_addr))
    {
        return;
    }

    bes_bt_me_confirmation_resp(&bd_addr,true);
}

static void app_bt_reject_ssp_confirmation_test(int argc, char* argv[])
{
    bt_bdaddr_t bd_addr;

    if (argc != 2)
    {
        DEBUG_INFO(0, "%s: param num error=%d, should be 2", __func__, argc);
        return;
    }
    if (!app_bt_scan_bdaddr_from_string(argv[1], strlen(argv[1]), &bd_addr))
    {
        return;
    }

    bes_bt_me_confirmation_resp(&bd_addr,false);
}

static void app_bt_a2dp_sink_connect_test(int argc, char* argv[])
{
    bt_bdaddr_t bd_addr;

    if (argc != 2)
    {
        DEBUG_INFO(0, "%s: param num error=%d, should be 2", __func__, argc);
        return;
    }
    if (!app_bt_scan_bdaddr_from_string(argv[1], strlen(argv[1]), &bd_addr))
    {
        return;
    }

    POSSIBLY_UNUSED bt_status_t ret_val = bt_a2dp_connect(&bd_addr,A2DP_ROLE_SNK);

    DEBUG_INFO(0,"%s result:%d",__func__,ret_val);
}

static void app_bt_a2dp_sink_disconnect_test(int argc, char* argv[])
{
    bt_bdaddr_t bd_addr;

    if (argc != 2)
    {
        DEBUG_INFO(0, "%s: param num error=%d, should be 2", __func__, argc);
        return;
    }
    if (!app_bt_scan_bdaddr_from_string(argv[1], strlen(argv[1]), &bd_addr))
    {
        return;
    }

    POSSIBLY_UNUSED bt_status_t ret_val = bt_a2dp_disconnect(&bd_addr);

    DEBUG_INFO(0,"%s result:%d",__func__,ret_val);
}

static void app_bt_a2dp_source_connect_test(int argc, char* argv[])
{
    bt_bdaddr_t bd_addr;

    if (argc != 2)
    {
        DEBUG_INFO(0, "%s: param num error=%d, should be 2", __func__, argc);
        return;
    }
    if (!app_bt_scan_bdaddr_from_string(argv[1], strlen(argv[1]), &bd_addr))
    {
        return;
    }

    POSSIBLY_UNUSED bt_status_t ret_val = BT_STS_SUCCESS;
#if defined(BT_SOURCE)
    ret_val = bt_a2dp_src_connect(&bd_addr);
#endif

    DEBUG_INFO(0,"%s result:%d",__func__,ret_val);
}

static void app_bt_a2dp_source_disconnect_test(int argc, char* argv[])
{
    bt_bdaddr_t bd_addr;

    if (argc != 2)
    {
        DEBUG_INFO(0, "%s: param num error=%d, should be 2", __func__, argc);
        return;
    }
    if (!app_bt_scan_bdaddr_from_string(argv[1], strlen(argv[1]), &bd_addr))
    {
        return;
    }

    POSSIBLY_UNUSED bt_status_t ret_val = BT_STS_SUCCESS;
#if defined(BT_SOURCE)
    ret_val = bt_a2dp_src_disconnect(&bd_addr);
#endif

    DEBUG_INFO(0,"%s result:%d",__func__,ret_val);
}

static void app_bt_a2dp_source_stream_start_test(int argc, char* argv[])
{
    bt_bdaddr_t bd_addr;

    if (argc != 2)
    {
        DEBUG_INFO(0, "%s: param num error=%d, should be 2", __func__, argc);
        return;
    }
    if (!app_bt_scan_bdaddr_from_string(argv[1], strlen(argv[1]), &bd_addr))
    {
        return;
    }

    POSSIBLY_UNUSED bt_status_t ret_val = BT_STS_SUCCESS;
#if defined(BT_SOURCE)
    ret_val = bt_a2dp_src_start_stream(&bd_addr);
#endif

    DEBUG_INFO(0,"%s result:%d",__func__,ret_val);
}

static void app_bt_a2dp_source_stream_suspend_test(int argc, char* argv[])
{
    bt_bdaddr_t bd_addr;

    if (argc != 2)
    {
        DEBUG_INFO(0, "%s: param num error=%d, should be 2", __func__, argc);
        return;
    }
    if (!app_bt_scan_bdaddr_from_string(argv[1], strlen(argv[1]), &bd_addr))
    {
        return;
    }

    POSSIBLY_UNUSED bt_status_t ret_val = BT_STS_SUCCESS;
#if defined(BT_SOURCE)
    ret_val = bt_a2dp_src_suspend_stream(&bd_addr);
#endif

    DEBUG_INFO(0,"%s result:%d",__func__,ret_val);
}

static void app_bt_hf_connect_test(int argc, char* argv[])
{
    bt_bdaddr_t bd_addr;

    if (argc != 2)
    {
        DEBUG_INFO(0, "%s: param num error=%d, should be 2", __func__, argc);
        return;
    }
    if (!app_bt_scan_bdaddr_from_string(argv[1], strlen(argv[1]), &bd_addr))
    {
        return;
    }

    POSSIBLY_UNUSED bt_status_t ret_val = bt_hf_connect(&bd_addr);

    DEBUG_INFO(0,"%s result:%d",__func__,ret_val);
}

static void app_bt_hf_disconnect_test(int argc, char* argv[])
{
    bt_bdaddr_t bd_addr;

    if (argc != 2)
    {
        DEBUG_INFO(0, "%s: param num error=%d, should be 2", __func__, argc);
        return;
    }
    if (!app_bt_scan_bdaddr_from_string(argv[1], strlen(argv[1]), &bd_addr))
    {
        return;
    }

    POSSIBLY_UNUSED bt_status_t ret_val = bt_hf_disconnect(&bd_addr);

    DEBUG_INFO(0,"%s result:%d",__func__,ret_val);
}

static void app_bt_hf_answer_call_test(int argc, char* argv[])
{
    bt_bdaddr_t bd_addr;

    if (argc != 2)
    {
        DEBUG_INFO(0, "%s: param num error=%d, should be 2", __func__, argc);
        return;
    }
    if (!app_bt_scan_bdaddr_from_string(argv[1], strlen(argv[1]), &bd_addr))
    {
        return;
    }

    POSSIBLY_UNUSED bt_status_t ret_val = BT_STS_FAILED;
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(&bd_addr);
    if (curr_device)
    {
        ret_val = btif_hf_answer_call(curr_device->hf_channel);
    }
    else
    {
        DEBUG_INFO(0,"%s curr_device NULL",__func__);
    }

    DEBUG_INFO(0,"%s result:%d",__func__,ret_val);
}

int app_bt_hf_dial_number(bt_bdaddr_t *bd_addr, char *number, uint16_t len)
{
    bt_hf_dial_number_param_t param;
    int ret = -1;

    if (len > DIAL_NUMBER_LEN_MAX)
    {
        DEBUG_INFO(0, "%s: ERROR: number len=%d", __func__, len);
        return ret;
    }

    for (uint16_t i = 0; i < len; i++)
    {
        if (*(number + i) < '0' || *(number + i) > '9')
        {
            DEBUG_INFO(0, "%s: ERROR: number invalid", __func__);
            return ret;
        }
    }

    memcpy(&param.addr, bd_addr, sizeof(bt_bdaddr_t));
    memcpy(param.number, number, len);
    param.len = len;

    POSSIBLY_UNUSED bt_status_t ret_val = bt_hf_dial((bt_bdaddr_t *)&param.addr, param.number);

    DEBUG_INFO(0,"%s result:%d",__func__,ret_val);

    return ret_val;
}

static void app_bt_hf_dial_number_test(int argc, char* argv[])
{
    bt_bdaddr_t bd_addr;

    if (argc != 3)
    {
        DEBUG_INFO(0, "%s: param num error=%d, should be 3", __func__, argc);
        return;
    }
    if (!app_bt_scan_bdaddr_from_string(argv[1], strlen(argv[1]), &bd_addr))
    {
        return;
    }
    app_bt_hf_dial_number(&bd_addr, argv[2], strlen(argv[2]));
}

static void app_bt_hf_hang_up_call_test(int argc, char* argv[])
{
    bt_bdaddr_t bd_addr;

    if (argc != 2)
    {
        DEBUG_INFO(0, "%s: param num error=%d, should be 2", __func__, argc);
        return;
    }
    if (!app_bt_scan_bdaddr_from_string(argv[1], strlen(argv[1]), &bd_addr))
    {
        return;
    }

    POSSIBLY_UNUSED bt_status_t ret_val = BT_STS_FAILED;
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(&bd_addr);
    if (curr_device)
    {
        ret_val = btif_hf_hang_up_call(curr_device->hf_channel);
    }
    else
    {
        DEBUG_INFO(0,"%s curr_device NULL",__func__);
    }

    DEBUG_INFO(0,"%s result:%d",__func__,ret_val);
}

static void app_bt_hf_start_voice_recognition_test(int argc, char* argv[])
{
    bt_bdaddr_t bd_addr;

    if (argc != 2)
    {
        DEBUG_INFO(0, "%s: param num error=%d, should be 2", __func__, argc);
        return;
    }
    if (!app_bt_scan_bdaddr_from_string(argv[1], strlen(argv[1]), &bd_addr))
    {
        return;
    }
    POSSIBLY_UNUSED bt_status_t ret_val = bt_hf_start_voice_recognition(&bd_addr);

    DEBUG_INFO(0,"%s result:%d",__func__,ret_val);
}
#endif /* BLE_ONLY_ENABLED */

#ifdef APP_USB_A2DP_SOURCE
static void app_a2dp_set_a2dp_source_test(int argc, char* argv[])
{
    app_a2dp_set_a2dp_source();
}
#endif

static void app_bt_sink_start_sniff_test(int argc, char* argv[])
{
#ifndef BLE_ONLY_ENABLED
    uint16_t val = 0;
    btif_sniff_info_t sniff_info;
    struct BT_DEVICE_T *curr_device = app_bt_get_device(BT_DEVICE_ID_1);
    if (!curr_device)
    {
        return;
    }
    strdec2uint16hex(argv[1], sizeof(argv[1]), &val);
    sniff_info.minInterval = val;
    sniff_info.maxInterval = val;
    if (val == 0)
    {
        sniff_info.minInterval = 800;
        sniff_info.maxInterval = 800;
    }
    sniff_info.attempt     = 3;
    sniff_info.timeout     = 1;
    DUMP8("%02x ", &curr_device->remote, 6);
    btif_me_start_sniff(curr_device->acl_conn_hdl, &sniff_info);
#endif
}

static void app_bt_source_start_sniff_test(int argc, char* argv[])
{
#ifndef BLE_ONLY_ENABLED
    uint16_t val = 0;
    btif_sniff_info_t sniff_info;
    struct BT_DEVICE_T *curr_device = app_bt_get_device(BT_DEVICE_ID_1);
    if (!curr_device)
    {
        return;
    }
    //strdec2uint16hex(param, len, &val);
    strdec2uint16hex(argv[1], sizeof(argv[1]), &val);
    sniff_info.minInterval = val;
    sniff_info.maxInterval = val;
    if (val == 0)
    {
        sniff_info.minInterval = 800;
        sniff_info.maxInterval = 800;
    }
    sniff_info.attempt     = 3;
    sniff_info.timeout     = 1;
    DUMP8("%02x ", &curr_device->remote, 6);
    btif_me_start_sniff(curr_device->acl_conn_hdl, &sniff_info);
#endif
}

#if defined(BT_SPP_SUPPORT)
#if defined(BES_OTA)
static void app_bt_spp_dev_conn_channel(uint8_t device_id, void* remote, bool succ, uint8_t errcode)
{
    DEBUG_INFO(0, "app_bt_spp_dev_conn_channel d(%d) succ %d", device_id, succ);
    uint8_t remote_uuid[] = {BT_SDP_SPLIT_16BITS_BE(SC_SERIAL_PORT)};
    osDelay(1000);
    btif_spp_connect(app_bt_get_pts_address(), RFCOMM_CHANNEL_BES_OTA, remote_uuid, sizeof(remote_uuid));
}
#endif // BES_OTA

static void app_bt_pts_spp_conn_channel_test(int argc, char* argv[])
{
#if defined(BES_OTA)
    btif_me_wait_acl_complete(app_bt_get_pts_address(), app_bt_spp_dev_conn_channel);
#else
    DEBUG_INFO(0, "not defined BES_OTA");
#endif
}
#endif // BT_SPP_SUPPORT

#ifdef APP_CHIP_BRIDGE_MODULE
static void app_test_uart_relay(int argc, char* argv[])
{
    char test_str[] = "pong";
    //app_uart_send_data((uint8_t *)test_str, strlen(test_str));
    app_chip_bridge_send_cmd(SET_TEST_MODE_CMD, (uint8_t *)test_str, strlen(test_str));
}
#endif

#if defined(LOCAL_AUDIO_SUPPORT)
static void app_usb_mtp_enter_sys_usb_mtp_download_mode(int argc, char* argv[])
{
/*no macro find
    hal_sw_bootmode_clear(HAL_SW_BOOTMODE_USB_MTP_OPERATION);
    hal_sw_bootmode_set(HAL_SW_BOOTMODE_USB_MTP_OPERATION);
*/
    app_reset();
}

#if defined LOCAL_AUDIO_TEST_ENABLE
static void app_local_audio_enter_local_mode_test(int argc, char* argv[])
{
    app_local_audio_enter_local_mode();
}

static void app_local_audio_exit_local_mode_test(int argc, char* argv[])
{
    app_local_audio_exit_local_mode();
}

static void app_local_audio_request_to_stop_and_exit_local_mode_test(int argc, char* argv[])
{
    app_local_audio_request_to_stop_and_exit_local_mode();
}

static void app_local_audio_request_to_pause_and_exit_local_mode_test(int argc, char* argv[])
{
    app_local_audio_request_to_pause_and_exit_local_mode();
}

static void app_local_audio_play_song_test(int argc, char* argv[])
{
    app_local_audio_play_song();
}

static void app_local_audio_play_next_song_test(int argc, char* argv[])
{
    app_local_audio_play_next_song();
}

static void app_local_audio_play_prev_song_test(int argc, char* argv[])
{
    app_local_audio_play_prev_song();
}

static void app_local_audio_pause_song_without_exit_local_mode_test(int argc, char* argv[])
{
    app_local_audio_pause_song_without_exit_local_mode();
}

static void app_local_audio_stop_song_without_exit_local_mode_test(int argc, char* argv[])
{
    app_local_audio_stop_song_without_exit_local_mode();
}
#endif
#endif // LOCAL_AUDIO_SUPPORT

#ifdef APP_CHIP_BRIDGE_MODULE
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "app_test_uart_relay", "bth", app_test_uart_relay);
#endif
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_trace_rx_sleep", "bth", app_bt_trace_rx_sleep);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_flush_nv", "bth", app_bt_flush_nv_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_show_device_linkkey", "bth", app_bt_show_device_linkkey_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "hci_check_state", "bth", app_bt_hci_print_statistic);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "hci_en_cmd_evt_debug", "bth", app_enable_hci_cmd_evt_debug);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "hci_de_cmd_evt_debug", "bth", app_disable_hci_cmd_evt_debug);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "hci_en_tx_flow_debug", "bth", app_enable_hci_tx_flow_debug);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "hci_de_tx_flow_debug", "bth", app_disable_hci_tx_flow_debug);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "ota_mode_enter", "bth", app_ota_mode_enter_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_sink_stop_sniff", "bth", app_bt_sink_stop_sniff_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_source_stop_sniff", "bth", app_bt_source_stop_sniff_test);

#ifdef APP_SPP_DEMO
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_client_spp_test", "bth", app_bt_client_spp_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_service_spp_test", "bth", app_service_spp_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_spp_send_data_test", "bth", app_spp_data_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_spp_set_loopback", "bth", app_spp_set_loopback_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_spp_clear_loopback", "bth", app_spp_clear_loopback_test);
#endif
#ifndef BLE_ONLY_ENABLED
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_key_click", "bth", app_bt_key_handle_func_click_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_key_double_click", "bth", app_bt_key_handle_func_doubleclick_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_key_long_click", "bth", app_bt_key_handle_func_longpress_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_start_both_scan", "bth", app_bt_start_both_scan_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_stop_both_scan", "bth", app_bt_stop_both_scan_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_disc_acl_link", "bth", app_bt_disconnect_acl_link_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "rfc_register", "bth", app_bt_pts_rfc_register_channel_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "rfc_close", "bth", app_bt_pts_pts_rfc_close_test);
#endif
#if defined(BT_HFP_TEST_SUPPORT)
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "hf_create_service_link", "bth", app_bt_pts_create_hf_channel_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "hf_disc_service_link", "bth", app_bt_pts_hf_disc_service_link_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "hf_create_audio_link", "bth", app_bt_pts_hf_create_audio_link_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "hf_disc_audio_link", "bth", app_bt_pts_hf_disc_audio_link_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "hf_send_key_pressed", " bth", app_bt_pts_hf_send_key_pressed_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "hf_answer_call", "bth", app_bt_pts_hf_answer_call_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "hf_hangup_call", "bth", app_bt_pts_hf_hangup_call_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "hf_dial_number", "bth", app_bt_pts_hf_dial_number_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "hf_dial_number_memory_index", "bth", app_bt_pts_hf_dial_number_memory_index_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "hf_dial_number_invalid_index", "bth", app_bt_pts_hf_dial_number_invalid_memory_index_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "hf_redial_call", "bth", app_bt_pts_hf_redial_call_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "hf_release_active_call", "bth",app_bt_pts_hf_release_active_call_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "hf_release_active_call_2", "bth", app_bt_pts_hf_release_active_call_2_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "hf_hold_active_call", "bth", app_bt_pts_hf_hold_active_call_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "hf_hold_active_call_2", "bth", app_bt_pts_hf_hold_active_call_2_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "hf_hold_call_transfer", "bth", app_bt_pts_hf_hold_call_transfer_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "hf_vr_enable", "bth", app_bt_pts_hf_vr_enable_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "hf_vr_disable", "bth", app_bt_pts_hf_vr_disable_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "hf_list_current_calls", "bth", app_bt_pts_hf_list_current_calls_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "hf_report_mic_volume", "bth", app_bt_pts_hf_report_mic_volume_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "hf_attach_voice_tag", "bth", app_bt_pts_hf_attach_voice_tag_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "hf_update_ind_value", "bth", app_bt_pts_hf_update_ind_value_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "hf_ind_activation", "bth", app_bt_pts_hf_ind_activation_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "hf_acs_bv_09_i_set_enable", "bth", app_bt_pts_hf_acs_bv_09_i_set_enable_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "hf_acs_bv_09_i_set_disable", "bth", app_bt_pts_hf_acs_bv_09_i_set_disable_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "hf_acs_bi_13_i_set_enable", "bth", app_bt_pts_hf_acs_bi_13_i_set_enable_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "hf_acs_bi_13_i_set_disable", "bth", app_bt_pts_hf_acs_bi_13_i_set_disable_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "hf_siri_voice_enable", "bth", app_bt_pts_hf_siri_voice_enable_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "hf_siri_voice_disable", "bth", app_bt_pts_hf_siri_voice_disable_test);

#if defined(BT_HFP_AG_ROLE)
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "ag_set_connect_state", "bth", app_bt_pts_hf_ag_set_connectable_state);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "ag_create_service_link", "bth", app_bt_pts_create_hf_ag_channel);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "ag_create_audio_link", "bth", app_bt_pts_hf_ag_create_audio_link);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "ag_disc_audio_link", "bth", app_bt_pts_hf_ag_disc_audio_link);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "ag_send_mobile_signal_level", "bth", app_bt_pts_hf_ag_send_mobile_signal_level);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "ag_send_mobile_signal_level_0", "bth", app_bt_pts_hf_ag_send_mobile_signal_level_0);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "ag_send_service_status", "bth", app_bt_pts_hf_ag_send_service_status);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "ag_send_service_status_0", "bth", app_bt_pts_hf_ag_send_service_status_0);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "ag_send_callsetup_status", "bth", app_bt_pts_hf_ag_send_callsetup_status);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "ag_send_callsetup_status_0", "bth", app_bt_pts_hf_ag_send_callsetup_status_0);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "ag_send_callactive_status", "bth", app_bt_pts_hf_ag_send_call_active_status);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "ag_send_hangup_call", "bth", app_bt_pts_hf_ag_hangup_call);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "ag_send_enable_roam", "bth", app_bt_pts_hf_ag_enable_roam);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "ag_send_disable_roam", "bth", app_bt_pts_hf_ag_disable_roam);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "ag_send_batt_level", "bth", app_bt_pts_hf_ag_send_mobile_battery_level);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "ag_send_full_batt_level", "bth", app_bt_pts_hf_ag_send_full_battery_level);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "ag_send_batt_level_0", "bth", app_bt_pts_hf_ag_send_battery_level_0);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "ag_send_calling_ring", "bth", app_bt_pts_hf_ag_send_calling_ring);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "ag_enable_inband_ring_tone", "bth", app_bt_pts_hf_ag_enable_inband_ring_tone);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "ag_place_a_call", "bth", app_bt_pts_hf_ag_place_a_call);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "ag_ongoing_call", "bth", app_bt_pts_hf_ag_ongoing_call);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "ag_ongoing_call_setup", "bth", app_bt_pts_hf_ag_ongoing_call_setup);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "ag_clear_last_dial_number", "bth", app_bt_pts_hf_ag_clear_last_dial_number);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "ag_send_call_waiting_notification", "bth", app_bt_pts_hf_ag_send_call_waiting_notification);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "ag_send_callheld_status", "bth", app_bt_pts_hf_ag_send_callheld_status);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "ag_send_callheld_status_0", "bth", app_bt_pts_hf_ag_send_callheld_status_0);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "ag_send_status_3_0_4_1", "bth", app_bt_pts_hf_ag_send_status_3_0_4_1);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "ag_set_pts_enable", "bth", app_bt_pts_hf_ag_set_pts_enable);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "ag_answer_incoming_call", "bth", app_bt_pts_hf_ag_answer_incoming_call);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "ag_set_pts_ecs_01", "bth", app_bt_pts_hf_ag_set_pts_ecs_01);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "ag_set_pts_ecs_02", "bth", app_bt_pts_hf_ag_set_pts_ecs_02);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "ag_set_pts_ecc", "bth", app_bt_pts_hf_ag_set_pts_ecc);
#endif /* BT_HFP_AG_ROLE */
#endif /* BT_HFP_TEST_SUPPORT */
#if defined(BT_A2DP_TEST_SUPPORT)
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "av_create_channel", "bth", app_bt_pts_create_av_channel_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "av_disc_channel", "bth", app_bt_pts_av_disc_channel_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "av_en_tone_intrrupt", "bth", app_bt_enable_tone_intrrupt_a2dp_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "av_de_tone_intrrupt", "bth", app_bt_disable_tone_intrrupt_a2dp_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "av_en_delay_prompt", "bth", app_bt_enable_a2dp_delay_prompt_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "av_de_delay_prompt", "bth", app_bt_disable_a2dp_delay_prompt_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "av_de_aac_codec", "bth", app_bt_disable_a2dp_aac_codec_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "av_de_vnd_codec", "bth", app_bt_disable_a2dp_vendor_codec_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "av_create_media_channel", "bth", app_bt_pts_av_create_media_channel_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "av_close_channel", "bth", app_bt_pts_av_close_channel_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "av_send_getconf", "bth", app_bt_pts_av_send_getconf_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "av_send_reconf", "bth", app_bt_pts_av_send_reconf_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "av_send_open", "bth", app_bt_pts_av_send_open_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "av_send_start", "bth", app_bt_pts_av_send_start_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "av_send_discover", "bth", btif_pts_av_send_discover);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "av_send_getcap", "bth", btif_pts_av_send_getcap);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "av_send_setconf", "bth", btif_pts_av_send_setconf);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "av_send_getallcap", "bth", btif_pts_av_send_getallcap);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "av_send_suspend", "bth", btif_pts_av_send_suspend);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "av_send_abort", "bth", btif_pts_av_send_abort);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "av_send_security_control", "bth", btif_pts_av_send_security_control);
#endif /* BT_A2DP_TEST_SUPPORT */
#if defined(BT_AVRCP_TEST_SUPPORT) || defined(BT_HFP_TEST_SUPPORT)
#ifndef BTH_SUBSYS_ONLY
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_sink_volume_up", "bth", app_bt_audio_local_volume_up_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_sink_volume_down", "bth", app_bt_audio_volume_down_test);
#endif
#endif

#if defined(BT_USE_COHEAP_ALLOC)
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "coheap_dump", "bth", app_bt_coheap_statistics_dump);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "coheap_enable_debug", "bth", app_bt_coheap_enable_debug);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "coheap_disable_debug", "bth", app_bt_coheap_disable_debug);
#endif

#if defined(NORMAL_TEST_MODE_SWITCH)
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "enter_signal_testmode", "bth", app_bt_enter_signal_testmode_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "exit_signal_testmode", "bth", app_bt_exit_signal_testmode_test);

ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "reboot_and_enter_nosignal_testmode", "bth", app_bt_reboot_and_enter_nosignal_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "reboot_and_enter_signal_testmode", "bth", app_bt_reboot_and_enter_signal_test);

ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "enter_nosignal_tx_testmode", "bth", app_bt_enter_nosignal_tx_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "enter_nosignal_rx_testmode", "bth", app_bt_enter_nosignal_rx_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "exit_nosignal_trx_testmode", "bth", app_bt_exit_nosignal_trx_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "enter_ble_tx_v1", "bth", app_bt_enter_ble_tx_v1_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "enter_ble_rx_v1", "bth", app_bt_enter_ble_rx_v1_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "enter_ble_tx_v2", "bth", app_bt_enter_ble_tx_v2_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "enter_ble_rx_v2", "bth", app_bt_enter_ble_rx_v2_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "enter_ble_tx_v3", "bth", app_bt_enter_ble_tx_v3_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "enter_ble_rx_v3", "bth", app_bt_enter_ble_rx_v3_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "enter_ble_tx_v4", "bth", app_bt_enter_ble_tx_v4_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "exit_ble_trx", "bth", app_bt_exit_ble_trx_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "enter_normal_mode", "bth", app_bt_enter_normal_mode_test);
#endif

#if defined(BT_SOURCE)
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_start_search", "bth", app_bt_source_start_search_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_stop_search", " bth", app_bt_source_stop_search_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bts_start_search", "bth", app_bts_start_search_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bts_stop_search", "bth", app_bts_stop_search_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bts_slow_search", "bth", app_bts_slow_search_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bts_normal_search", "bth", app_bts_force_normal_search_test);

ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bts_boost_up_test", "bth", app_bts_boost_up_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_set_curr_nv_source", "bth", app_bt_set_curr_nv_source);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_clear_curr_nv_source", "bth", app_bt_clear_curr_nv_source);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_source_music", "bth", app_bt_source_music_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_local_music", "bth", app_bt_local_music_test);

ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "source_disc_link", "bth", app_bt_source_disconnect_link);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "reconfig_to_sbc", "bth", app_bt_source_reconfig_codec_to_sbc);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "source_send_sbc_pkt", "bth", app_bt_pts_source_send_sbc_packet);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "source_cretae_media_chnl", "bth", app_bt_pts_source_cretae_media_channel_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "source_send_close_cmd", "bth", app_bt_pts_source_send_close_cmd_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "source_send_get_config_cmd", "bth", app_bt_pts_source_send_get_configuration_cmd_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "source_send_reconfigure_cmd", "bth", app_bt_pts_source_send_reconfigure_cmd_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "source_send_abort_cmd", "bth", app_bt_pts_source_send_abort_cmd_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "source_send_suspend_cmd", "bth", app_bt_pts_source_send_suspend_cmd_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "source_set_get_all_cap_flag", "bth", app_bt_pts_source_set_get_all_cap_flag);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "source_set_unknown_cmd_flag", "bth", app_bt_pts_source_set_unknown_cmd_flag);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "source_send_start_cmd", "bth", app_bt_pts_source_send_start_cmd_test);

   // bt_conn_earbud_link|af:19:b0:bb:22:74
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_connect_earbud_link", "bth", app_bt_connect_earbud_link_test);
   // bt_conn_earbud_a2dp|af:19:b0:bb:22:74
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_connect_earbud_a2dp", "bth", app_bt_connect_earbud_a2dp_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_connect_earbud_avrcp", "bth", app_bt_connect_earbud_avrcp_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_connect_earbud_hfp", "bth", app_bt_connect_earbud_hfp_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_connect_mobile_link", "bth", app_bt_connect_mobile_link_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_connect_mobile_a2dp", "bth", app_bt_connect_mobile_a2dp_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_connect_mobile_avrcp", "bth", app_bt_connect_mobile_avrcp_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_connect_mobile_hfp", "bth", app_bt_connect_mobile_hfp_test);
#if defined(BT_HFP_AG_ROLE)
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "ag_create_audio_link", "bth", app_bt_ag_create_audio_link_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "ag_disc_audio_link", "bth", app_bt_ag_disc_audio_link_test);
#endif
#if defined(mHDT_SUPPORT)
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "enter_mhdt_mode", "bth", app_bt_source_enter_mhdt_mode_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "exit_mhdt_mode", "bth", app_bt_source_exit_mhdt_mode_test);
#endif
#endif

#ifdef AUDIO_LINEIN
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "linein_start", "bth", app_bt_test_linein_start);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "linein_stop", "bth", app_bt_test_linein_stop);
#endif

#ifdef APP_USB_A2DP_SOURCE
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "reg_usb_btaudio", "bth", app_a2dp_set_a2dp_source_test);
#endif
#if defined(LOCAL_AUDIO_SUPPORT)
    //enter usb m55 usb mtp download mode
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "enter_sys_usb_mtp_download_mode", "bth", app_usb_mtp_enter_sys_usb_mtp_download_mode);
#if defined LOCAL_AUDIO_TEST_ENABLE
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "local_audio_enter_local_mode", "bth", app_local_audio_enter_local_mode_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "local_audio_exit_local_mode", "bth", app_local_audio_exit_local_mode_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "local_audio_pause_and_exit", "bth", app_local_audio_request_to_pause_and_exit_local_mode_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "local_audio_stop_and_exit", "bth", app_local_audio_request_to_stop_and_exit_local_mode_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "local_audio_pause_and_no_exit", "bth", app_local_audio_pause_song_without_exit_local_mode_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "local_audio_stop_and_no_exit", "bth", app_local_audio_stop_song_without_exit_local_mode_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "local_audio_play_song_test", "bth", app_local_audio_play_song_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "local_audio_play_next_song_test", "bth", app_local_audio_play_next_song_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "local_audio_play_prev_song_test", "bth", app_local_audio_play_prev_song_test);
#endif // LOCAL_AUDIO_TEST_ENABLE
#endif // LOCAL_AUDIO_SUPPORT

    ///param cmd
#ifndef BLE_ONLY_ENABLED
// bt_set_access_mode 2
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_set_access_mode", "bth", app_bt_set_access_mode_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_access_mode_set", "bth", app_bt_access_mode_set_test);
//  command example: app_bt_connect_acl 56:34:12:22:11:11
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "app_bt_connect_acl", "bth", app_bt_connect_acl_test);
// command example: app_bt_disconnect_acl 56:34:12:22:11:11
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "app_bt_disconnect_acl", "bth", app_bt_disconnect_acl_test);
//  command example: app_bt_connect_sco 56:34:12:22:11:11
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "app_bt_connect_sco", "bth", app_bt_connect_sco_test);
// command example: app_bt_start_inquiry
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "app_bt_start_inquiry", "bth", app_bt_start_inquiry_test);
// command example: app_bt_cancel_inquiry
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "app_bt_cancel_inquiry", "bth", app_bt_cancel_inquiry_test);
// command example: app_bt_set_eir_data_test <eir_data>
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "app_bt_set_eir_data", "bth", app_bt_set_eir_data_test);
// command example: bt_adapter_accept_ssp_confirmation 56:34:12:22:11:11
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "app_bt_accept_ssp_confirmation", "bth", app_bt_accept_ssp_confirmation_test);
// command example: app_bt_reject_ssp_confirmation_test 56:34:12:22:11:11
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "app_bt_reject_ssp_confirmation", "bth", app_bt_reject_ssp_confirmation_test);
// command example: app_bt_a2dp_sink_connect 56:34:12:22:11:11
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "app_bt_a2dp_sink_connect", "bth", app_bt_a2dp_sink_connect_test);
// command example: app_bt_a2dp_sink_disconnect 56:34:12:22:11:11
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "app_bt_a2dp_sink_disconnect", "bth", app_bt_a2dp_sink_disconnect_test);
// command example: app_bt_a2dp_source_connect 56:34:12:22:11:11
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "app_bt_a2dp_source_connect", "bth", app_bt_a2dp_source_connect_test);
// command example: app_bt_a2dp_source_disconnect 56:34:12:22:11:11
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "app_bt_a2dp_source_disconnect", "bth", app_bt_a2dp_source_disconnect_test);
// command example: app_bt_a2dp_source_stream_start_test 56:34:12:22:11:11
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "app_bt_a2dp_source_stream_start_test", "bth", app_bt_a2dp_source_stream_start_test);
// command example: app_bt_a2dp_source_stream_suspend_test 56:34:12:22:11:11
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "app_bt_a2dp_source_stream_suspend_test", "bth", app_bt_a2dp_source_stream_suspend_test);
// command example: app_bt_hf_connect 56:34:12:22:11:11
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "app_bt_hf_connect", "bth", app_bt_hf_connect_test);
// command example: app_bt_hf_disconnect 56:34:12:22:11:11
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "app_bt_hf_disconnect", "bth", app_bt_hf_disconnect_test);
// command example: app_bt_hf_answer_call 56:34:12:22:11:11
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "app_bt_hf_answer_call", "bth", app_bt_hf_answer_call_test);
// command example: app_bt_hf_dial_number 56:34:12:22:11:11 <phone_number_to_dial>
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "app_bt_hf_dial_number", "bth", app_bt_hf_dial_number_test);
// command example: app_bt_hf_hang_up_call 56:34:12:22:11:11
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "app_bt_hf_hang_up_call", "bth", app_bt_hf_hang_up_call_test);
// command example: app_bt_hf_start_voice_recognition 56:34:12:22:11:11
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "app_bt_hf_start_voice_recognition", "bth", app_bt_hf_start_voice_recognition_test);
#endif
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_sink_start_sniff", "bth", app_bt_sink_start_sniff_test);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_source_start_sniff", "bth", app_bt_source_start_sniff_test);
// bt_set_pts_address|ed:b6:f4:dc:1b:00
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_pts_set_address", "bth", app_bt_pts_set_address);
// bt_delete_link_key|ed:b6:f4:dc:1b:00
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_delete_device_linkkey", "bth", app_bt_delete_device_linkkey_test);
//bt_set_linkey|e7ab95d59015ae53fb2d800d340e23412c1d7297d658  linkeydevicebtaddr
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_set_linkey", "bth", app_bt_set_linkey_test);
#if defined(BT_SPP_SUPPORT)
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "spp_conn_test", "bth", app_bt_pts_spp_conn_channel_test);
#endif // BT_SPP_SUPPORT

#else
static bt_bdaddr_t g_app_bt_pts_addr = {{
#if 1
    0xed, 0xb6, 0xf4, 0xdc, 0x1b, 0x00
#elif 0
    0x81, 0x33, 0x33, 0x22, 0x11, 0x11
#elif 0
    0x13, 0x71, 0xda, 0x7d, 0x1a, 0x00
#else
    0x14, 0x71, 0xda, 0x7d, 0x1a, 0x00
#endif
}};

bt_bdaddr_t *app_bt_get_pts_address(void)
{
    return &g_app_bt_pts_addr;
}
#endif
