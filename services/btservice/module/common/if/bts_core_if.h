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
#ifndef __BTS_CORE_IF_H__
#define __BTS_CORE_IF_H__

#include "app_tws_ibrt_core_type.h"
#include "bts_core_type.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 ****************************************************************************************
 * @brief Convert uiRole to string
 *
 * @param[in] uiRole
 *
 * @return pointer of the string
 ****************************************************************************************
 */
const char* bts_core_uirole2str(TWS_UI_ROLE_E uiRole);

/**
 ****************************************************************************************
 * @brief Set ui-role
 *
 * @param[in] ui_role       ui_role
 * <table>
 * <th>Description   replace api "void app_ibrt_conn_set_ui_role(TWS_UI_ROLE_E ui_role)"
 * </table>
 ****************************************************************************************
 */
void bts_core_set_ui_role(TWS_UI_ROLE_E ui_role);

/**
 ****************************************************************************************
 * @brief Get ui-role
 *
 * @return ui-role
 * <table>
 * <th>Description   replace api "TWS_UI_ROLE_E app_ibrt_conn_get_ui_role(void)"
 * <th>Description   replace api "TWS_UI_ROLE_E app_ibrt_middleware_get_ui_role(void)"
 * <th>Description   replace api "TWS_UI_ROLE_E app_ui_get_current_role(void)"
 * <th>Description   replace api "TWS_UI_ROLE_E app_ibrt_if_get_ui_role(void)"
 * </table>
 ****************************************************************************************
 */
TWS_UI_ROLE_E bts_core_get_ui_role(void);

/**
 ****************************************************************************************
 * @brief Register tws ui role update callback handle
 *
 * <table>
 * <th>Description   replace api "void app_ibrt_register_tws_ui_role_update_handle(app_tws_ui_role_update_callback_handler_t func)"
 * </table>
 ****************************************************************************************
 */
void bts_core_register_tws_ui_role_update_handle(void (*cb)(uint8_t newRole));

/**
 ****************************************************************************************
 * @brief Initiate ibrt_conn
 ****************************************************************************************
 */
void app_ibrt_conn_init();

/**
 ****************************************************************************************
 * @brief Get the max support mobile device number
 *
 * @return The number of max support mobile device
 ****************************************************************************************
 */
uint8_t app_ibrt_conn_support_max_mobile_dev(void);

/**
 ****************************************************************************************
 * @brief Check if in freeman mode
 *
 * @return
 * <table>
 * <tr><th>Value        <th>Description
 * <tr><td>True  <td>In freeman mode
 * <tr><td>False  <td>Not in freeman mode
 * </table>
 ****************************************************************************************
 */
bool bts_core_is_freeman_mode(void);


bool bts_core_is_ui_master();

bool bts_core_is_ui_slave();


void app_tws_ibrt_reconfig_role(uint32_t role, const uint8_t *pMaster,
                        const uint8_t *pSlaveAddr, bool isRightMasterSidePolicy);

void app_tws_ibrt_reconfig_audio_chnl(AUDIO_CHANNEL_SELECT_E ch);

/**
 ****************************************************************************************
 * @brief Config pagescan status for test
 *
 * @param[in] disc_enable       discover enable
 * @param[in] conn_enable       connection enable
 *
 * @return An error status
 ****************************************************************************************
 */
ibrt_status_t app_ibrt_conn_set_discoverable_connectable(bool disc_enable, bool conn_enable);

/**
 ****************************************************************************************
 * @brief print conn info for debug (autotest)
 ****************************************************************************************
 */
void app_ibrt_conn_dump_ibrt_info();

bool app_ibrt_conn_is_phone_connect_happened(void);
 /**
  ****************************************************************************************
 * @brief app_ibrt_conn_pscan_setting
 *
 * @param[in] scan_window_slots  the window of page scan(Unit slot)
 * @param[in] scan_interval_slots  the interval of page scan(Unit slot)
 *
 * @return ibrt_status_t
 ****************************************************************************************
 */
ibrt_status_t app_ibrt_conn_pscan_setting(uint16_t scan_window_slots, uint16_t scan_interval_slots);

bt_status_t app_tws_ibrt_write_link_policy(const bt_bdaddr_t *p_addr, btif_link_policy_t policy);

bool app_tws_ibrt_compare_btaddr(void);

bt_status_t app_tws_update_local_bt_addr(uint8_t* pBtAddr);

bt_status_t app_tws_update_local_ble_addr(uint8_t * pBleAddr);

ibrt_link_type_e app_tws_ibrt_get_link_type_by_addr(const bt_bdaddr_t *p_addr);

void app_tws_ibrt_core_reconfig(ibrt_core_param_t *params,ibrt_core_config_t* configration);

void app_tws_ibrt_init(void);

ibrt_ctrl_t* app_tws_ibrt_get_bt_ctrl_ctx(void);

void app_tws_ibrt_start(ibrt_config_t *config, bool is_ibrt_search_ui);

int tws_ctrl_send_cmd(uint32_t cmd_code, uint8_t *p_buff, uint16_t length);

int tws_ctrl_send_rsp(uint16_t rsp_code, uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

void tws_ctrl_free_cmd(uint32_t cmd_code);

void bts_core_start_boost_freq();

void bts_core_stop_boost_freq(uint8_t extraDelay);

#ifdef __cplusplus
}
#endif
#endif /*__BTS_CORE_IF_H__*/
