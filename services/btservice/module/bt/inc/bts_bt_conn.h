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
#ifndef __BTS_BT_SINK_CONN_H__
#define __BTS_BT_SINK_CONN_H__

#include "app_tws_ibrt_core_type.h"
#include "app_tws_ibrt_mobile_conn_sm.h"
#include "me_api.h"
typedef struct
{
    int (*bt_event_notify)(void *event);
    void (*dev_connect_ind)(const bt_bdaddr_t *addr);
} bts_bt_to_ui_cbs_t;

typedef struct
{
    mobile_statemachine_t mobile_sm_array[BT_DEVICE_NUM];
    const app_ibrt_profiles_hook_cb *profiles_hook_cb;
    const bts_bt_to_gfps_cbs_t *gfps_cbs;
    const bts_bt_to_ui_cbs_t *ui_cbs;
} bts_bt_conn_ctrl_t;

#ifdef __cplusplus
extern "C" {
#endif

bts_bt_conn_ctrl_t* bts_bt_conn_get_ctrl();

void app_ibrt_conn_ctx_init(void);

void app_tws_ibrt_mobile_info_init(const bt_bdaddr_t *addr,ibrt_mobile_info_t *p_mobile_info);

ibrt_conn_acl_state bts_bt_conn_get_dev_acl_state(const bt_bdaddr_t* addr);

bool bts_bt_conn_is_dev_connecting(const bt_bdaddr_t* addr);

bool bts_bt_conn_any_dev_connecting(void);

bool bts_bt_sink_conn_profile_connecting(ibrt_mobile_info_t *p_mobile_info);

bool bts_bt_conn_is_any_profile_connecting(void);

bool bts_bt_conn_is_profile_connecting(const bt_bdaddr_t *addr);

bool bts_bt_conn_dev_acl_link_connected(const bt_bdaddr_t *p_mobile_addr);

bool bts_bt_conn_has_any_dev_acl_link_connected(void);

void app_ibrt_conn_free_mobile_sm(const bt_bdaddr_t *addr);

ibrt_mobile_info_t* app_ibrt_conn_get_mobile_info_by_hci_handle(uint16_t hci_handle);

ibrt_mobile_info_t *bts_bt_sink_conn_get_mobile_info_by_addr(const bt_bdaddr_t* addr);

/**
 ****************************************************************************************
 * @brief Get mobile info in the begin of the map
 *
 * @return pointer of mobile info
 ****************************************************************************************
 */
ibrt_mobile_info_t *bts_bt_sink_conn_get_mobile_info_ext(void);

uint8_t bts_bt_conn_get_dev_list(bt_bdaddr_t *addr_list);

uint8_t bts_bt_conn_get_dev_connected_list(bt_bdaddr_t *addr_list);

uint8_t bts_bt_sink_conn_get_all_valid_mobile_info(ibrt_mobile_info_t *p_mobile_info_array[],uint8_t max_size);

uint8_t bts_bt_conn_get_dev_acl_connected_count();

uint8_t app_tws_ibrt_get_mobile_num_connected(void);

mobile_statemachine_t* bts_bt_sink_conn_get_mobile_sm_by_addr(const bt_bdaddr_t* addr);

mobile_statemachine_t* app_ibrt_conn_slave_mock_mobile_link(const bt_bdaddr_t *addr,uint32_t param0);

/**
 ****************************************************************************************
 * @brief Start connect profiles for mobile device
 *
 * @param[in] addr              Mobile address
 * @param[in] direction         The connection is initialed by local or remote
 * @param[in] request_connect   request connect
 * @param[in] timeout           page_timeout (unit is 625us)
 *
 * @return An error status
 ****************************************************************************************
 */
ibrt_status_t app_ibrt_conn_remote_dev_connect_request(const bt_bdaddr_t *addr, connection_direction_t direction, bool request_connect, uint32_t timeout);

/**
 ****************************************************************************************
 * @brief get connected mobile device handle
 *
 * @param[int] mobile device address,if input paramter is null,return connected device handle
 *
 * @return handle
 ****************************************************************************************
 */
uint16_t app_ibrt_conn_get_dev_acl_handle(const bt_bdaddr_t* addr);

bool bts_bt_sink_conn_is_mobile_connhandle(uint16_t connhandle);

bt_status_t app_tws_ibrt_create_mobile_connection(uint16_t mobile_page_timeout,bt_bdaddr_t *mobile_addr);

ibrt_status_t app_ibrt_conn_send_mobile_msg(const bt_bdaddr_t *addr, ibrt_mobile_message_e evt,uint32_t param0, uint32_t param1);

void app_ibrt_conn_mobile_ssp_failed_handler(bt_bdaddr_t *addr);

/**
 ****************************************************************************************
 * @brief check if both side earbuds address is null
 ****************************************************************************************
 */
void app_ibrt_conn_reg_super_handle_connect_req_cb(bool (*cb)(bt_bdaddr_t *addr, uint8_t *cod));

/**
 ****************************************************************************************
 * @brief handle cbk when mobile ssp fail
 ****************************************************************************************
 */
void app_ibrt_conn_set_mobile_ssp_fail_cb(void (*cb)(bt_bdaddr_t *addr));

bool app_ibrt_conn_super_handle_connect_request(bt_bdaddr_t *addr, uint8_t *cod);

void app_ibrt_delay_profile_send_timer(ibrt_mobile_info_t *p_mobile_info);

uint8_t app_tws_ibrt_get_mobile_num_in_sniff_mode(void);

bt_status_t bts_bt_sink_conn_set_role(const bt_bdaddr_t *p_bd_addr, ibrt_role_e new_role);

ibrt_role_e bts_bt_sink_conn_get_ibrt_role(const bt_bdaddr_t *p_bd_addr);

btif_connection_role_t bts_bt_sink_conn_get_local_mobile_role(bt_bdaddr_t *p_mobile_addr);

ibrt_status_t bts_bt_conn_dev_connect_all_profiles(const bt_bdaddr_t *addr);

bool app_ibrt_basic_profiles_established(ibrt_mobile_info_t *p_mobile_info);

bool app_ibrt_any_basic_profiles_established(ibrt_mobile_info_t *p_mobile_info);

bool bts_bt_conn_dev_any_basic_profiles_exits(const bt_bdaddr_t *addr);

ibrt_link_mode_e bts_bt_conn_get_dev_link_mode(void* addr);

void bts_bt_sink_conn_set_local_volume_info(uint8_t * p_buffer);

ibrt_status_t app_ibrt_conn_send_ibrt_msg(const bt_bdaddr_t *addr, ibrt_sm_message_e evt,uint32_t param0,uint32_t param1);

bool bts_bt_sink_conn_sniff_allowed(bt_bdaddr_t *remote_addr, ibrt_link_type_e link_type, uint8_t direction);

void bts_bt_sink_conn_exit_sniff_for_all_mobile_device(void);

void bts_bt_sink_conn_sniff_timeout_handler(evm_timer_t *timer, unsigned int *skipInternalHandler);

void app_tws_ibrt_restart_ibrt_master_sniff_timer(uint16_t conn_handle);

void app_tws_set_ibrt_slave_sniff_timer_info(ibrt_mobile_info_t *p_mobile_info);

uint32_t bts_bt_conn_get_dev_constate(const bt_bdaddr_t *addr);

void bts_bt_conn_dev_disconnect_all_links();

ibrt_status_t bts_bt_conn_dev_cancel_create_connection(const bt_bdaddr_t *addr);

ibrt_status_t bts_bt_conn_dev_disconnect_acl_link(const bt_bdaddr_t *addr,void (*post_func)(void));

bt_status_t bts_bt_sink_conn_disconnect_connection(btif_remote_device_t *remdev);

void app_tws_ibrt_disconnect_mobile(bt_bdaddr_t *mobile_addr);

void bts_bt_sink_conn_set_a2dp_codec_v2(const bt_bdaddr_t *p_addr, const a2dp_callback_parms_t *info);

bool bts_bt_sink_conn_is_profile_initiator(ibrt_mobile_info_t *p_mobile_info,uint64_t profile);

bt_status_t app_tws_ibrt_do_mss_with_mobile(bt_bdaddr_t *p_mobile_addr);

BTIF_BT_ACTIVITY_STOP_E app_tws_ibrt_stop_pending_page_activity(const bt_bdaddr_t* addr);

void app_tws_ibrt_clear_lastest_disconnect_profile(ibrt_mobile_info_t *p_mobile_info);

void app_tws_ibrt_write_link_policy_in_tws_switch(bt_bdaddr_t *mobile_addr, bool sniff_allowed);

bool app_tws_ibrt_mobile_pair_canceled(const bt_bdaddr_t *p_bd_addr);

void app_tws_ibrt_set_mobile_addr(ibrt_mobile_info_t *p_mobile_info, bt_bdaddr_t *p_bd_addr);

bool app_tws_ibrt_switch_role(bt_bdaddr_t *p_bd_addr);

bool app_tws_ibrt_get_phone_connect_happened(ibrt_mobile_info_t *p_mobile_info);

void app_tws_ibrt_clear_phone_connect_happened(ibrt_mobile_info_t *p_mobile_info);

void app_tws_ibrt_set_phone_connect_happened(ibrt_mobile_info_t *p_mobile_info);

bool app_tws_ibrt_local_initiate_pairing_is_not_allowed(const bt_bdaddr_t *p_btaddr);

uint8_t app_tws_ibrt_sync_airmode_check_ind_handler(uint8_t voice_setting, bt_bdaddr_t *bdaddr);

bool app_tws_ibrt_if_any_a2dp_streaming(void);

uint8_t app_tws_ibrt_get_audio_voice_active_state(uint8_t device_id);

bool app_tws_ibrt_rx_data_filter_callback(const bt_bdaddr_t  *bd_addr,uint8_t rx_filter_type,void*para);

void app_tws_ibrt_hci_tx_buf_tss_process(void);

void app_tws_ibrt_set_tss_state(ibrt_mobile_info_t *p_mobile_info, tss_state_e state);

tss_state_e app_tws_ibrt_get_tss_state(ibrt_mobile_info_t *p_mobile_info);

void app_tws_ibrt_reset_tss_state(ibrt_mobile_info_t *p_mobile_info);

void app_tws_ibrt_set_controller_rx_flow_stop(ibrt_mobile_info_t *p_mobile_info );

void app_tws_ibrt_clear_controller_rx_flow_stop(ibrt_mobile_info_t *p_mobile_info);

void app_tws_ibrt_store_mobile_link_key(const bt_bdaddr_t *addr,uint8_t *p_link_key);

bt_status_t bts_bt_conn_dev_link_exit_sniff_mode(const bt_bdaddr_t *p_dev_addr);

void bts_bt_sink_register_ui_profile_hook_callbcak(const app_ibrt_profiles_hook_cb *cb);

void bts_bt_sink_profile_hook_callbcak_handle(uint64_t profile, const bt_bdaddr_t *addr, void *param);

void bts_bt_sink_init(void);

void app_hci_vender_ibrt_ll_monitor_handle(uint16_t handle, uint8_t *ptr);

/**
 ****************************************************************************************
 * @brief report ll monitor event to upper layer
 *
 ****************************************************************************************
 */
void app_hci_vender_register_ll_monitor_handle(void (*fn)(uint16_t handle, uint8_t ser));

void bts_bt_conn_register_ui_cbs(const bts_bt_to_ui_cbs_t *cbs);

void bts_bt_conn_event_notify(ibrt_conn_evt_header* evt);

void bts_bt_conn_notify_dev_connect_ind(const bt_bdaddr_t *addr);

void bts_bt_conn_register_gfps_cbs(const bts_bt_to_gfps_cbs_t *cb);

void bts_bt_conn_profile_evt_notify_gfps(const bt_bdaddr_t *addr, uint8_t device_id, uint64_t profile, uint8_t event, uint8_t param);

void bts_bt_conn_dev_acl_disconnect_notify_gfps(void);

void bts_bt_conn_notify_bluetooth_enabled(void);

void bts_bt_conn_notify_bluetooth_disabled(void);

bool stop_ibrt_ongoing(void);

void app_custom_ui_safe_disconnect_process(uint8_t device_id);

void app_custom_ui_all_safe_disconnect(void);

void app_custom_ui_cancel_all_connection(void);

#ifdef __cplusplus
}
#endif

#endif
