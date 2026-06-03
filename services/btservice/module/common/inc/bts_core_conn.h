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
#ifndef __BTS_CORE_CONN_H__
#define __BTS_CORE_CONN_H__

#include "bts_core_type.h"
#include "app_tws_ibrt_core_type.h"

typedef struct
{
    ibrt_conn_config_t config;
    const app_ibrt_cmd_ui_handler_cb *ibrt_cmd_handler_cbs;
    const app_ibrt_if_cbs_t  *app_ibrt_if_cbs;
    bool (*reject_conn_req)(bt_bdaddr_t *addr, uint8_t *cod);
    void (*mobile_ssp_failed_cb)(bt_bdaddr_t *addr);
} app_ibrt_conn_t;

#ifdef __cplusplus
extern "C" {
#endif

app_ibrt_conn_t* app_ibrt_get_conn_ctx();

ibrt_conn_config_t* app_ibrt_conn_get_config();

void bts_core_common_init(void);

uint8_t app_ibrt_conn_get_custom_support_max_dev();

/**
 ****************************************************************************************
 * @brief register ibrt_if callback
 *
 * @param[in] cbs       app_ibrt_if_cbs_t callbacks struct
 ****************************************************************************************
 */
void app_ibrt_conn_reg_ibrt_if_cb(const app_ibrt_if_cbs_t *cbs);

/**
 ****************************************************************************************
 * @brief share some info asap, send tws share info complete event
 *
 ****************************************************************************************
 */
void app_ibrt_conn_send_share_info_complete_evt(void);

/**
 ****************************************************************************************
 * @brief Set freeman_enable
 ****************************************************************************************
 */
void app_ibrt_conn_set_freeman_enable(void);

/**
 ****************************************************************************************
 * @brief Clear freeman_enable
 ****************************************************************************************
 */
void app_ibrt_conn_clear_freeman_enable(void);

/**
 ****************************************************************************************
 * @brief register cmd_handler callback
 *
 * @param[in] cbs       app_ibrt_cmd_ui_handler_cb callbacks struct
 ****************************************************************************************
 */
void app_ibrt_conn_reg_cmd_handler_cbs(const app_ibrt_cmd_ui_handler_cb *cbs);

const app_ibrt_if_cbs_t* app_ibrt_conn_get_reg_ibrt_if_cb();

const app_ibrt_cmd_ui_handler_cb* app_ibrt_conn_get_cmd_handler_cbs(void);

void app_ibrt_conn_ble_switch_done_cb(void);

void app_ibrt_conn_controller_error_handler(ibrt_controller_error_type error_type, bool reset_controler,const bt_bdaddr_t* mobile_addr);

bool app_ibrt_conn_get_tws_use_same_addr_enable();

void app_tws_ibrt_sniff_manager_callback(const btif_event_t *event);

bool app_tws_ibrt_remote_is_mobile(const bt_bdaddr_t *remote);

/**
 ****************************************************************************************
 * @brief Dispatch event
 *
 * @param[in] param       event param, see@app_ibrt_conn_msg_t
 * @param[in] param_len   param length
 ****************************************************************************************
 */
void app_ibrt_conn_dispatch_event(uint8_t *param, int param_len);

void app_tws_ibrt_reset_bdaddr_to_nv_original(void);

void app_tws_ibrt_set_bdaddr_to_nv_master(void);

void app_tws_ibrt_start_boost_freq(void);

void app_tws_ibrt_stop_boost_freq(uint8_t extraDelay);

void app_tws_ibrt_use_the_same_bd_addr(void);

ibrt_core_param_t* app_tws_ibrt_get_custom_config();

uint8_t app_tws_ibrt_get_custom_support_max_dev();

ibrt_ctrl_t* app_tws_ibrt_get_bt_ctrl_ctx(void);

uint8_t app_tws_ibrt_support_max_remote_link();


#ifdef __cplusplus
}
#endif

#endif /*__BTS_CORE_CONN_H__*/
