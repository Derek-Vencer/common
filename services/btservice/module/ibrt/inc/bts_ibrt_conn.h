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
#ifndef __BTS_IBRT_CONN_H__
#define __BTS_IBRT_CONN_H__

#include "bts_core_type.h"
#include "app_tws_ibrt_core_type.h"
#include "bts_ibrt_sm.h"

typedef struct
{
    int (*ibrt_event_notify)(void *event);
    void (*vendor_event_notify)(uint8_t evt_type, uint8_t * buffer, uint32_t length);
} bts_ibrt_to_ui_cbs_t;

typedef struct
{
    osMutexId      ibrt_core_mutex;
    osMutexDefEx_t ibrt_core_mutex_def;
    ibrt_state_machine_t ibrt_sm[BT_DEVICE_NUM];
    const bts_ibrt_to_ui_cbs_t *ui_cbs;
} bts_ibrt_ctrl_t;

#ifdef __cplusplus
extern "C" {
#endif

bts_ibrt_ctrl_t* bts_ibrt_conn_get_ctrl();

void bts_ibrt_conn_init(void);

void bts_ibrt_conn_dump_info(void);

#if BLE_AUDIO_ENABLED
ibrt_status_t app_ibrt_conn_notify_bt_nv_changed(bt_bdaddr_t* mobile_addr);

ibrt_status_t app_ibrt_conn_sync_smp_remote_device(uint8_t* buf,uint16_t len );
#endif

/**
 ****************************************************************************************
 * @brief Set stop ibrt is ongoing
 *
 * @param[in] stop_ibrt       if stop ibrt is ongoing
 ****************************************************************************************
 */
void app_ibrt_conn_set_stop_ibrt_ongoing(bool stop_ibrt);

/**
 ****************************************************************************************
 * @brief Check if stop ibrt is ongoing
 *
 * @return
 * <table>
 * <tr><th>Value        <th>Description
 * <tr><td>True  <td>stop ibrt is ongoing
 * <tr><td>False  <td>stop ibrt is not ongoing
 * </table>
 ****************************************************************************************
 */
bool app_ibrt_conn_stop_ibrt_ongoing(void);

void app_ibrt_conn_profile_data_exchange(ibrt_mobile_info_t* mobile_info);

/**
 ****************************************************************************************
 * @brief perform choice mobile connect
 *
 * @param[in] addr       Mobile address
 ****************************************************************************************
 */
void bts_ibrt_conn_connect_choice_mobile(const bt_bdaddr_t *addr, uint8_t try_count);

/**
 ****************************************************************************************
 * @brief send sync ui mode infomation
 *
 * @param[in] buf       ui mode
 * @param[in] len       buf len
 ****************************************************************************************
 */
ibrt_status_t app_ibrt_conn_notify_ui_info(uint8_t *buf, uint16_t len);

/**
 ****************************************************************************************
 * @brief common chnl send data
 *
 * @param[in] buf       data buf
 * @param[in] len       data length
 ****************************************************************************************
 */
bool app_ibrt_common_chnl_send_data(uint8_t *buf, uint16_t len);

/**
 ****************************************************************************************
 * @brief Send prepare complete messege
 ****************************************************************************************
 */
void app_ibrt_conn_notify_prepare_complete(void);

/*---------------------------------------------------------------------------
 *            app_ibrt_middleware_master_prepare_rs
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    master prepare for role switch for BISTO and AI
 *
 * Parameters:
 *    void
 *
 * Return:
 *    void
 */
void app_ibrt_middleware_master_prepare_rs(uint8_t *p_buff, uint16_t length);

/*---------------------------------------------------------------------------
 *            app_ibrt_middleware_slave_continue_rs
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    slave continue role switch progress
 *
 * Parameters:
 *    void
 *
 * Return:
 *    void
 */
void app_ibrt_middleware_slave_continue_rs(uint8_t *p_buff, uint16_t length);

/*---------------------------------------------------------------------------
 *            app_ibrt_middleware_role_switch_complete_handler
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    callback function of role switch complete event for tws system
 *    NOTE: tws system include relay_tws and IBRT
 *
 * Parameters:
 *    newRole - current role of device after role switch complete
 *
 * Return:
 *    void
 */
void app_ibrt_middleware_role_switch_complete_handler(uint8_t newRole);

bool app_ibrt_internal_tws_switch_prepare_needed(uint32_t *wait_ms);

void app_tws_ibrt_sync_sco_codec_info_v2(const bt_bdaddr_t* remote, uint8_t codec);

void app_tws_ibrt_sync_sco_codec_info_when_needed(void);

void app_tws_ibrt_report_spp_close_to_slave(const bt_bdaddr_t *remote, uint8_t dlci);

/**
 ****************************************************************************************
 * @brief send get peer rssi req
 *
 * @param[in] buf        data buf
 * @param[in] len        data len
 ****************************************************************************************
 */
void app_ibrt_conn_send_get_peer_rssi_req(uint8_t *buf, uint16_t len);

/**
 ****************************************************************************************
 * @brief send get peer rssi req
 *
 * @param[in] rsp_seq    rsp seq
 * @param[in] buf        data buf
 * @param[in] len        data len
 ****************************************************************************************
 */
void app_ibrt_conn_send_get_peer_rssi_rsp(uint16_t rsp_seq, uint8_t *buf, uint16_t len);

/**
 ****************************************************************************************
 * @brief Send exchange info
 *
 * @param[in] ibrt_mgr_info      the pointer of ibrt_mgr_info
 * @param[in] len               the length of ibrt_mgr_info
 *
 * @return An error status
 ****************************************************************************************
 */
ibrt_status_t app_ibrt_conn_exchange_earbuds_info(uint8_t *ibrt_mgr_info, uint16_t len);

/**
 ****************************************************************************************
 * @brief Send exchange info rsp
 *
 * @param[in] rsp_seq            the pointer of rsp sequence
 * @param[in] ibrt_mgr_info      the pointer of ibrt_mgr_info
 * @param[in] len                the length of ibrt_mgr_info
 *
 * @return An error status
 ****************************************************************************************
 */
ibrt_status_t app_ibrt_conn_exchange_ibrt_mgr_info_rsp(uint16_t rsp_seq,uint8_t *ibrt_mgr_info, uint16_t len);

/**
 ****************************************************************************************
 * @brief Send notify running info
 *
 * @param[in] running_info       the pointer of running_info
 * @param[in] len               the length of running_info
 *
 * @return An error status
 ****************************************************************************************
 */
ibrt_status_t app_ibrt_conn_notify_running_info(uint8_t *running_info, uint16_t len);

/**
 ****************************************************************************************
 * @brief Send notify running info rsp
 *
 * @param[in] rsp_seq            the pointer of rsp sequence
 * @param[in] running_info       the pointer of running_info
 * @param[in] len                the length of running_info
 *
 * @return An error status
 ****************************************************************************************
 */
ibrt_status_t app_ibrt_conn_notify_running_info_rsp(uint16_t rsp_seq,uint8_t *running_info, uint16_t len);

/**
 ****************************************************************************************
 * @brief Send notify run complete info
 *
 * @param[in] running_info       the pointer of running_info
 * @param[in] len               the length of running_info
 *
 * @return An error status
 ****************************************************************************************
 */
ibrt_status_t app_ibrt_conn_notify_run_complete_info(uint8_t *running_info, uint16_t len);

/**
 ****************************************************************************************
 * @brief Send notify run complete info rsp
 *
 * @param[in] rsp_seq            the pointer of rsp sequence
 * @param[in] running_info       the pointer of running_info
 * @param[in] len                the length of running_info
 *
 * @return An error status
 ****************************************************************************************
 */
ibrt_status_t app_ibrt_conn_notify_run_complete_info_rsp(uint16_t rsp_seq,uint8_t *running_info, uint16_t len);

/**
 ****************************************************************************************
 * @brief Send devices mgr manager info
 *
 * @param[in] devices_info       the pointer of devices_info
 * @param[in] len                the length of devices_info
 *
 * @return An error status
 ****************************************************************************************
 */
ibrt_status_t app_ibrt_conn_send_dev_mgr(uint8_t *devices_info, uint16_t len);

/**
 ****************************************************************************************
 * @brief Send devices mgr manager info rsp
 *
 * @param[in] rsp_seq            the pointer of rsp sequence
 * @param[in] devices_info       the pointer of devices_info
 * @param[in] len                the length of devices_info
 *
 * @return An error status
 ****************************************************************************************
 */
ibrt_status_t app_ibrt_conn_send_dev_mgr_rsp(uint16_t rsp_seq,uint8_t *devices_info, uint16_t len);

/**
 ****************************************************************************************
 * @brief sync_info
 *
 * @param[in] sync_info       buff
 ****************************************************************************************
 */
ibrt_status_t app_ibrt_conn_sync_info(uint8_t *buff, uint16_t len);

/**
 ****************************************************************************************
 * @brief sync_info_rsp
 *
 * @param[in] sync_info       buff
 ****************************************************************************************
 */
ibrt_status_t app_ibrt_conn_sync_info_rsp(uint16_t rsp_seq,uint8_t *buff, uint16_t len);

/**
 ****************************************************************************************
 * @brief send destroy device req
 *
 * @param[in] buf       destroy req
 * @param[in] len       buf len
 ****************************************************************************************
 */
ibrt_status_t app_ibrt_conn_destroy_device(uint8_t *buf, uint16_t len);

/**
 ****************************************************************************************
 * @brief response destroy device rsp
 *
 * @param[in] addr       Mobile address
 ****************************************************************************************
 */
ibrt_status_t app_ibrt_conn_destroy_device_rsp(uint16_t rsp_seq, uint8_t * address, uint16_t len);

int app_tws_ibrt_start_role_switch(const bt_bdaddr_t *mobile_addr);

bool bts_ibrt_conn_is_ibrt_idle(const bt_bdaddr_t *addr);

bool bts_ibrt_conn_is_w4_ibrt(const bt_bdaddr_t *addr);

bool bts_ibrt_conn_is_w4_data_exchange(const bt_bdaddr_t *addr);

bool bts_ibrt_conn_is_ibrt_connected(const bt_bdaddr_t *addr);

void bts_ibrt_conn_set_exchange_profile_complete(const bt_bdaddr_t *addr);

bool bts_ibrt_conn_is_profile_exchanged(const bt_bdaddr_t *addr);

ibrt_status_t bts_ibrt_conn_send_ibrt_msg(const bt_bdaddr_t *addr, ibrt_sm_message_e evt,uint32_t param0,uint32_t param1);

ibrt_status_t bts_ibrt_conn_send_msg_to_links(ibrt_sm_message_e evt,uint32_t param0,uint32_t param1);

void bts_ibrt_conn_ibrt_sm_quit(const bt_bdaddr_t *addr);

uint32_t bts_ibrt_conn_get_ibrt_constate(void* addr);

/**
 ****************************************************************************************
 * @brief Get current connected ibrt connected device count and address list.
 *
 * @param[out] addr_list          Used to return current ibrt connected mobile device address list
 *
 * @return The number of current ibrt connected count
 ****************************************************************************************
 */
uint8_t app_ibrt_conn_get_ibrt_connected_list(bt_bdaddr_t *addr_list);

/**
 ****************************************************************************************
 * @brief get snoop link count
 ****************************************************************************************
 */
uint8_t app_ibrt_conn_get_snoop_connected_link_num(void);

uint8_t bts_ibrt_conn_get_link_count(void);

bool bts_ibrt_conn_any_ibrt_connected(void);

bool bts_ibrt_conn_any_snoop_link(void);

uint8_t bts_ibrt_conn_get_snoop_mobile_count();

uint16_t bts_ibrt_conn_get_ibrt_handle(const bt_bdaddr_t *p_bd_addr);

ibrt_status_t bts_ibrt_conn_slave_startup_ibrt(const bt_bdaddr_t *addr, ibrt_sm_message_e param);

bool app_tws_ibrt_start_needed(const bt_bdaddr_t* mobile_addr);

ibrt_status_t bts_ibrt_conn_connect_ibrt(const bt_bdaddr_t *addr);

ibrt_status_t bts_ibrt_conn_disconnect_ibrt(const bt_bdaddr_t *addr);

void bts_ibrt_conn_disconnect_all_ibrt(void);

bool app_tws_ibrt_set_env_needed(const bt_bdaddr_t *mobile_addr);

void  app_tws_ibrt_set_env(ibrt_mobile_info_t* mobile_info);

bool bts_ibrt_conn_is_ibrt_link_connected(const bt_bdaddr_t *p_dev_addr);

void bts_ibrt_conn_register_ui_cbs(const bts_ibrt_to_ui_cbs_t *cbs);

void bts_ibrt_conn_event_notify(ibrt_conn_evt_header* evt);

void bts_ibrt_conn_vendor_event_notify(uint8_t evt_type, uint8_t * buffer, uint32_t length);

#ifdef __cplusplus
}
#endif

#endif
