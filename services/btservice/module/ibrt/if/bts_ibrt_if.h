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
#ifndef __BTS_IBRT_IF_H__
#define __BTS_IBRT_IF_H__

#include "bts_core_type.h"

/**************************************API FOR APP RSSI CALL**************************************/
/**
 ****************************************************************************************
 * @brief send get peer rssi req
 *
 * @param[in] buf        data buf
 * @param[in] len        data len
 * <table>
 * <th>Description   replace api "void app_ibrt_conn_send_get_peer_rssi_req(uint8_t *buf, uint16_t len)"
 * </table>
 ****************************************************************************************
 */
void bts_ibrt_if_send_peer_rssi_req(uint8_t *buf, uint16_t len);

/**
 ****************************************************************************************
 * @brief send get peer rssi req
 *
 * @param[in] rsp_seq    rsp seq
 * @param[in] buf        data buf
 * @param[in] len        data len
 * <table>
 * <th>Description   replace api "void app_ibrt_conn_send_get_peer_rssi_rsp(uint16_t rsp_seq, uint8_t *buf, uint16_t len)"
 * </table>
 ****************************************************************************************
 */
void bts_ibrt_if_send_peer_rssi_rsp(uint16_t rsp_seq, uint8_t *buf, uint16_t len);

/**
 ****************************************************************************************
 * @brief Get ibrt constate
 *
 * @param[in] addr       Mobile address
 *
 * @return ibrt constate
 * <table>
 * <th>Description   replace api "uint32_t app_ibrt_conn_get_ibrt_constate(void* addr)"
 * </table>
 ****************************************************************************************
 */
uint32_t bts_ibrt_if_get_ibrt_constate(void* addr);

/**
 ****************************************************************************************
 * @brief Initiate ibrt connection for mobile device
 *
 * @param[in] addr       Mobile address
 *
 * @return An error status
 * <table>
 * <th>Description   replace api "uint16_t app_tws_ibrt_get_ibrt_handle(const bt_bdaddr_t *p_bd_addr)"
 * </table>
 ****************************************************************************************
 */
uint16_t bts_ibrt_if_get_dev_ibrt_handle(const bt_bdaddr_t *p_bd_addr);

/**
 ****************************************************************************************
 * @brief Initiate ibrt connection for mobile device
 *
 * @param[in] addr       Mobile address
 *
 * @return An error status
 * <table>
 * <th>Description   replace api "ibrt_status_t app_ibrt_conn_connect_ibrt(const bt_bdaddr_t *addr)"
 * </table>
 ****************************************************************************************
 */
ibrt_status_t bts_ibrt_if_dev_connect_ibrt_link(const bt_bdaddr_t *addr);

/**
 ****************************************************************************************
 * @brief Disconnect ibrt connection for mobile device
 *
 * @param[in] addr       Mobile address
 *
 * @return An error status
 * <table>
 * <th>Description   replace api "ibrt_status_t app_ibrt_conn_disconnect_ibrt(const bt_bdaddr_t *addr)"
 * </table>
 ****************************************************************************************
 */
ibrt_status_t bts_ibrt_if_dev_disconnect_ibrt_link(const bt_bdaddr_t *addr);

/**
 ****************************************************************************************
 * @brief Check a2dp profile is exchanged
 *
 * @param[in] addr       remote address
 *
 * @return bool
 * <table>
 * <th>Description   replace api "bool app_ibrt_a2dp_profile_is_exchanged(const bt_bdaddr_t *addr)"
 * </table>
 ****************************************************************************************
 */
bool bts_ibrt_if_a2dp_profile_is_exchanged(const bt_bdaddr_t *addr);

/**
 ****************************************************************************************
 * @brief Check if profile_exchanged for the mobile
 *
 * @param[in] addr       remote address
 *
 * @return
 * <table>
 * <tr><th>Value        <th>Description
 * <tr><td>True  <td>profile_exchanged
 * <tr><td>False  <td>profile_exchanged
 * </table>
 * <table>
 * <th>Description   replace api "bool app_ibrt_conn_is_profile_exchanged(const bt_bdaddr_t *addr)"
 * </table>
 ****************************************************************************************
 */
bool bts_ibrt_if_is_profile_exchanged(const bt_bdaddr_t *addr);

/**
 ****************************************************************************************
 * @brief Get the number of local snoop mobile devices
 *
 * @return The number of local snoop mobile devices
 ****************************************************************************************
 */
uint8_t bts_ibrt_if_get_dev_ibrt_connected_count();

#ifdef __cplusplus
extern "C" {
#endif
/**
 ****************************************************************************************
 * @brief Check mobile ibrt snoop link is connected
 *
 * @param[in] addr       Mobile address
 *
 * @return bool
 * <table>
 * <th>Description   replace api "bool app_tws_ibrt_slave_ibrt_link_connected(const bt_bdaddr_t *p_mobile_addr)"
 * </table>
 ****************************************************************************************
 */
bool bts_ibrt_if_is_ibrt_link_connected(const bt_bdaddr_t *p_dev_addr);

#ifdef __cplusplus
}
#endif

/**
 ****************************************************************************************
 * @brief Check ibrt is idle
 *
 * @param[in] addr       remote address
 *
 * @return bool
 * <table>
 * <th>Description   replace api "bool app_ibrt_conn_is_ibrt_idle(const bt_bdaddr_t *addr)"
 * </table>
 ****************************************************************************************
 */

bool bts_ibrt_if_is_ibrt_idle(const bt_bdaddr_t *addr);

#endif
