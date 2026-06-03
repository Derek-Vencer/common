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
#ifndef __BTS_TWS_CONN_H__
#define __BTS_TWS_CONN_H__

#include "bts_core_type.h"
#include "bts_tws_type.h"
#include "app_tws_ibrt_core_type.h"

typedef struct
{
    bool     timing_enable;
    uint8_t  acl_slot_num;
    uint16_t acl_interval;
    uint16_t acl_interval_in_sco;
    uint8_t priority;
} TWS_TIMING_CONTROL_INFO_T;

typedef struct
{
    bt_bdaddr_t   mobile_addr;
    bt_bdaddr_t   nv_mobile_addr;
    ibrt_role_e   ibrt_role;
    uint8_t       mobile_connected:1;
    uint8_t       ibrt_connected:1;
    uint8_t       connecting_mobile:1;
    uint64_t      constate;
    uint16_t      conn_hdl;
} __attribute__((packed)) app_tws_info_t;

typedef struct {
    void (*send_get_peer_rssi_hanlder_cb)(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
    void (*send_get_peer_rssi_rsp_handler_cb)(uint8_t *p_buff, uint16_t length);
}app_ibrt_get_peer_rssi_cb;

typedef struct
{
    int (*tws_event_notify)(void *event);
    bool (*tws_disconnect_update_ui_master_cfg)(void);
} bts_tws_to_ui_cbs_t;

typedef struct
{
    uint16_t tws_conhandle;

    uint16_t peer_tws_conhandle;

    const app_ibrt_get_peer_rssi_cb *get_peer_rssi_cb;

    app_tws_info_t  peer_tws_info;

    TWS_TIMING_CONTROL_INFO_T *tws_timing_table;
    uint8_t  acl_slot_num;
    uint16_t acl_interval;
    uint16_t acl_interval_in_sco;

    uint8_t  default_acl_slot_num;
    uint16_t default_acl_interval;
    uint16_t default_acl_interval_in_sco;

    bool (*role_switch_disallow)(void);
    const bts_tws_to_ui_cbs_t *ui_cbs;
} bts_tws_ctrl_t;

#ifdef __cplusplus
 extern "C" {
#endif

void bts_tws_init(void);

bts_tws_ctrl_t* bts_tws_get_ctrl(void);

void bts_tws_notify_btc_tws_link(uint8_t link_id);

void bts_tws_perform_set_tws_link_id_req(void *param);

void bts_tws_perform_set_tws_link_id_rsp(void *param);

void bts_tws_sync_set_tws_link_id(bool local_has_mobile);

bool bts_tws_is_connected_with_wrong_peer(void);

/**
 ****************************************************************************************
 * @brief register app_ibrt_get_peer_rssi_cb tws  callback
 *
 * @param[in] cbs       app_ibrt_get_peer_rssi_cb callbacks struct
 ****************************************************************************************
 */
void bts_tws_get_peer_rssi_cb(const app_ibrt_get_peer_rssi_cb *cb);

const app_ibrt_get_peer_rssi_cb* bts_tws_reg_get_rssi_cmd_cb();

void bts_tws_init_default_tws_bandwidth_config(uint8_t defaultAclSlotNum,
    uint16_t defaultAclInterval, uint16_t defaultAclIntervalInSco);

void bts_tws_bandwidth_table_clean();

void bts_tws_bandwidth_set_reconfig(void);

/**
 ****************************************************************************************
 * @brief share some info asap, register tws info callback, only tws BT_MASTER will
 *
 * @param[in] cb       void (*cb)(void)
  * <th>Description    replace api??"void app_ibrt_conn_reg_tws_share_info_cb(void (*cb)(void));"
 ****************************************************************************************
 */
void bts_tws_reg_tws_share_info_cb(void (*cb)(void));

bt_status_t bts_tws_connect_request_handler(uint32_t page_to);

bool bts_tws_peer_earbuds_addr_null();

#if defined(FPGA)
void bts_tws_fpga_start_tws_pairing(ibrt_role_e role);
#endif

void bts_tws_reset_peer_tws_info(void);

app_tws_info_t* bts_tws_get_peer_tws_info();

void bts_tws_bandwidth_sniff_manager(void);

void bts_tws_restart_tws_sniff_timer(void);

/******************************************************************************************
 * @brief cancel tws page
 *
 * @return An error status
 *****************************************************************************************/
ibrt_status_t bts_tws_cancel_page(void);

/**
 ****************************************************************************************
 * @brief Set earbud side.
 *
 * @param[in] side               Only can be set to left or right side
 ****************************************************************************************
 */
void bts_ibrt_conn_set_side(APP_TWS_SIDE_T side);

APP_TWS_SIDE_T bts_ibrt_conn_get_side(void);

void app_ibrt_internal_tws_swtich_prepare(uint32_t timeoutMs);

void app_ibrt_middleware_deregister_sync_user(TWS_SYNC_USER_E id);


/*---------------------------------------------------------------------------
 *            app_tws_middleware_common_info_rsp_recieved_handler
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    shared common info response received handler
 *
 * Parameters:
 *    rsp_seq - sequence number of response
 *    p_buff - pointer for received data
 *    lenght - length of received data
 *
 * Return:
 *    void
 */
void app_ibrt_middleware_sync_info_rsp_received_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

/*---------------------------------------------------------------------------
 *            app_ibrt_middleware_sync_info_rsp_timeout_handler
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    shared common info response timeout handler
 *
 * Parameters:
 *    rsp_seq - sequence number of response
 *    p_buff - pointer for received data
 *    lenght - length of received data
 *
 * Return:
 *    void
 */
void app_ibrt_middleware_sync_info_rsp_timeout_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);


void bts_ibrt_init_tws_env_info(void);

/*---------------------------------------------------------------------------
 *            app_ibrt_middleware_sync_info_received_handler
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    handler for tws common info received event
 *    NOTE: see @ to get more info for common info received
 *
 * Parameters:
 *    rsp_seq - sequence number of response
 *    p_buff - pointer for received data
 *    lenght - length of received data
 *
 * Return:
 *    void
 */
void app_ibrt_middleware_sync_info_received_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

void bts_tws_conn_register_ui_cbs(const bts_tws_to_ui_cbs_t *cbs);

void bts_tws_conn_event_notify(ibrt_conn_evt_header* evt);

bool bts_tws_conn_get_tws_disconnect_update_ui_master_cfg(void);

#ifdef __cplusplus
}
#endif
#endif
