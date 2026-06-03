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
#ifndef __BTS_BT_SINK_IF_H__
#define __BTS_BT_SINK_IF_H__
#include "bluetooth_bt_api.h"
#include "app_tws_ibrt.h"
#include "app_tws_ibrt_core_type.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 ****************************************************************************************
 * @brief Check if a mobile link is connecting
 *
 * @return  true there is a mobile link connecting
 *          false no mobile link connecting
 * <table>
 * <th>Description   replace api "bool app_ibrt_conn_any_mobile_connecting(void)"
 * </table>
 ****************************************************************************************
 */
bool bts_bt_if_has_any_dev_link_connecting(void);

/**
 ****************************************************************************************
 * @brief Check if addr mobile link is connecting
 *
 * @return  true there is addr mobile link connecting
 *          false no mobile link connecting
 * <table>
 * <th>Description   replace api "bool app_ibrt_conn_is_mobile_connecting(const bt_bdaddr_t* addr)"
 * </table>
 ****************************************************************************************
 */

bool bts_bt_if_is_dev_link_connecting(const bt_bdaddr_t* addr);

/**
 ****************************************************************************************
 * @brief Check if addr mobile profile is connecting
 *
 * @return  true there is addr mobile profile connecting
 *          false no mobile profile connecting
 * <table>
 * <th>Description   replace api "bool app_ibrt_conn_is_any_profile_connecting(void)"
 * </table>
 ****************************************************************************************
 */

bool bts_bt_if_is_any_profile_connecting(void);

/**
 ****************************************************************************************
 * @brief Check if addr mobile profile is connecting
 *
 * @return  true there is addr mobile profile connecting
 *          false no mobile profile connecting
 * <table>
 * <th>Description   replace api "bool app_ibrt_conn_is_profile_connecting(const bt_bdaddr_t *addr)"
 * </table>
 ****************************************************************************************
 */

bool bts_bt_if_is_profile_connecting(const bt_bdaddr_t *addr);

/**
 ****************************************************************************************
 * @brief Check mobile link connection by mobile address
 *
 * @param[in] addr       Mobile address
 *
 * @return
 * <table>
 * <tr><th>Value        <th>Description
 * <tr><td>True  <td>mobile link is connected
 * <tr><td>False  <td>mobile link not connected
 * </table>
 * <table>
 * <th>Description   replace api "bool app_ibrt_conn_mobile_link_connected(const bt_bdaddr_t* addr)"
 * <th>Description   replace api "bool app_tws_ibrt_mobile_link_connected(const bt_bdaddr_t *mobile_addr)"
 * </table>
 ****************************************************************************************
 */
bool bts_bt_if_is_dev_link_connected(const bt_bdaddr_t *p_mobile_addr);

 /**
 ****************************************************************************************
 * @brief Check if there exist mobile connection
 *
 * @return
 * <table>
 * <tr><th>Value        <th>Description
 * <tr><td>True  <td>Exist mobile connection and ibrt connection
 * <tr><td>False <td>No mobile connection
 * </table>
 * <table>
 * <th>Description   replace api "bool app_ibrt_conn_any_mobile_connected(void)"
 * </table>
 ****************************************************************************************
 */
bool bts_bt_if_has_any_dev_link_connected(void);

/**
 ****************************************************************************************
 * @brief Start connect profiles for mobile device
 *
 * @param[in] addr       Mobile address
 *
 * @return An error status
 * <table>
 * <th>Description   replace api "ibrt_status_t app_ibrt_conn_connect_profiles(const bt_bdaddr_t *addr)"
 * </table>
 ****************************************************************************************
 */
ibrt_status_t bts_bt_if_dev_connect_all_profiles(const bt_bdaddr_t *addr);

/**
 ****************************************************************************************
 * @brief check if any basic profile exits
 *
 * <table>
 * <th>Description   replace api "bool app_ibrt_conn_any_basic_profiles_exits(const bt_bdaddr_t *addr)"
 * </table>
 ****************************************************************************************
 */
bool bts_bt_if_dev_any_basic_profiles_exits(const bt_bdaddr_t *addr);

/**
 ****************************************************************************************
 * @brief Get mobile link mode
 *
 * @param[in] addr       Mobile address
 *
 * @return mobile link mode
 *
 * <table>
 * <th>Description   replace api "ibrt_link_mode_e app_ibrt_conn_get_mobile_mode(void* addr)"
 * </table>
 ****************************************************************************************
 */
ibrt_link_mode_e bts_bt_if_get_dev_link_mode(void* addr);

/**
 ****************************************************************************************
 * @brief Get current connected mobile device count and address list.
 *
 * @param[out] addr_list          Used to return current connected mobile device address list
 *
 * @return The number of current connected mobile
 * <table>
 * <th>Description   replace api "uint8_t app_ibrt_conn_get_valid_device_list(bt_bdaddr_t *addr_list)"
 * </table>
 ****************************************************************************************
 */
uint8_t bts_bt_if_get_dev_list(bt_bdaddr_t *addr_list);

/**
 ****************************************************************************************
 * @brief Get current connected mobile device count and address list.
 *
 * @param[out] addr_list          Used to return current connected mobile device address list
 *
 * @return The number of current connected mobile
 * <table>
 * <th>Description   replace api "uint8_t app_ibrt_conn_get_connected_mobile_list(bt_bdaddr_t *addr_list)"
 * </table>
 ****************************************************************************************
 */
uint8_t bts_bt_if_get_dev_connected_list(bt_bdaddr_t *addr_list);

/**
 ****************************************************************************************
 * @brief Get the number of local connected mobile devices
 *
 * @return The number of local connected mobile devices
 * <table>
 * <th>Description   replace api "uint8_t app_ibrt_conn_get_local_connected_mobile_count()"
 * </table>
 ****************************************************************************************
 */
uint8_t bts_bt_if_get_dev_acl_connected_count();

/**
 ****************************************************************************************
 * @brief Get mobile constate
 *
 * @param[in] addr       Mobile address
 *
 * @return mobile constate
 * <table>
 * <th>Description   replace api "uint32_t app_ibrt_conn_get_mobile_constate(void* addr)"
 * </table>
 ****************************************************************************************
 */
uint32_t bts_bt_if_get_dev_constate(const bt_bdaddr_t* addr);

/**
 ****************************************************************************************
 * @brief Get mobile connection handle by mobile address
 *
 * @param[in] addr       Mobile address
 *
 * @return handle
 *
 * <table>
 * <th>Description   replace api "bool app_ibrt_conn_get_mobile_handle(const bt_bdaddr_t* addr)"
 * </table>
 ****************************************************************************************
 */
uint16_t bts_bt_if_get_dev_acl_handle(const bt_bdaddr_t* addr);

/**
 ****************************************************************************************
 * @brief Not implemented
 *
 * @param[in] cbs       Mobile addr
 * <table>
 * <th>Description   replace api "void app_ibrt_conn_disc_all_mobile_link()"
 * </table>
 ****************************************************************************************
 */
void bts_bt_if_dev_disconnect_all_links();

/**
 ****************************************************************************************
 * @brief Cancel connection for mobile device
 *
 * @param[in] addr       Mobile address
 *
 * @return An error status
 * <table>
 * <th>Description   replace api "ibrt_status_t app_ibrt_conn_remote_dev_connect_cancel_request(const bt_bdaddr_t *addr)"
 * </table>
 ****************************************************************************************
 */
ibrt_status_t bts_bt_if_dev_cancel_create_connection(const bt_bdaddr_t *addr);

/**
 ****************************************************************************************
 * @brief Disconnect mobile device
 *
 * @param[in] addr           Mobile address
 *
 * @return An error status
 * <table>
 * <th>Description   replace api "rt_status_t app_ibrt_conn_remote_dev_disconnect_request(const bt_bdaddr_t *addr,ibrt_post_func post_func)"
 * </table>
 ****************************************************************************************
 */
ibrt_status_t bts_bt_if_dev_disconnect_acl_link(const bt_bdaddr_t *addr);

/**
 ****************************************************************************************
 * @brief Mobile exit sniff mode
 *
 * @param[in] addr       remote address
 *
 * @return bool
 * <table>
 * <th>Description   replace api "bt_status_t app_tws_ibrt_exit_sniff_with_mobile(const bt_bdaddr_t *p_mobile_addr)"
 * </table>
 ****************************************************************************************
 */

bt_status_t bts_bt_if_dev_link_exit_sniff_mode(const bt_bdaddr_t *p_dev_addr);

void bts_bt_if_spp_bisto_save_ctx(bt_bdaddr_t *p_remote_addr, bt_spp_channel_t* spp_devi);

/**
 ****************************************************************************************
 * @brief Get mobile connection state
 *
 * @param[in] addr       Mobile address
 *
 * @return mobile connection state
 * <table>
 * <th>Description   replace api "ibrt_conn_acl_state app_ibrt_conn_get_mobile_conn_state(const bt_bdaddr_t* addr)"
 * </table>
 ****************************************************************************************
 */
ibrt_conn_acl_state bts_bt_if_get_dev_acl_state(const bt_bdaddr_t* addr);

/**
 ****************************************************************************************
 * @brief report ll monitor event to upper layer
 *
 * <table>
 * <th>Description   replace api "void app_hci_vender_register_ll_monitor_handle(void (*fn)(uint16_t handle, uint8_t ser))"
 * </table>
 ****************************************************************************************
 */
void bts_bt_if_register_vender_ll_monitor_handle(void (*fn)(uint16_t handle, uint8_t ser));

/**
 ****************************************************************************************
 * @brief report profiles event to gfps
 *
 ****************************************************************************************
 */
void bts_bt_if_register_gfps_cbs(const bts_bt_to_gfps_cbs_t *cb);

#ifdef __cplusplus
}
#endif
#endif
