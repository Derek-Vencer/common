/***************************************************************************
 *
 * @brief Bluetooth Service  Intelligent Bluetooth retransmission technology Low Energy Audio
 *
 * Copyright 2015-2024 BES.
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

/*****************************header include********************************/
#include <stdint.h>

/*********************external function declaration*************************/

/************************private macro defination***************************/

/************************private type defination****************************/
typedef enum {
    /// TWS primary role
    TWS_ROLE_PRIMARY = 0,
    /// TWS secondary role
    TWS_ROLE_SECONDARY,
    /// TWS unkonw role
    TWS_ROLE_UNKONW = 0xFF,
} BT_SVC_TWS_ROLE_E;

typedef struct
{
    /// get tws connection handle callback
    uint16_t (*tws_conhdl_get_cb)(void);
    /// get tws role callback
    uint8_t (*tws_role_get_cb)(void);
    /// get tws role callback
    void     (*ble_switch_done_cb)(void);
} bt_svc_tws_ble_switch_start_t;

/************************private variable defination************************/

/**********************private function declaration*************************/

/**
 ****************************************************************************************
 * @brief Open the ble connection switching function
 * @param[in]: param, ble switch open parameter.see@bt_svc_tws_ble_switch_start_t
 * @param[return]: error code, see@BT_SVC_ERROR_CODE_E
 ****************************************************************************************
 */
uint8_t bt_svc_tws_ble_switch_open(bt_svc_tws_ble_switch_start_t *param);

/**
 ****************************************************************************************
 * @brief Close the ble connection switching function
 * @param[out]: error code, see@BT_SVC_ERROR_CODE_E
 ****************************************************************************************
 */
uint8_t bt_svc_tws_ble_switch_close();

/**
 ****************************************************************************************
 * @brief Check if there are switchable ble links
 * @param[out]: 0:no have, 1:have
 ****************************************************************************************
 */
uint8_t bt_svc_tws_ble_switch_link_check(void);

/**
 ****************************************************************************************
 * @brief TWS role has changed
 * @param[in]: new_role, new role,see @BT_SVC_TWS_ROLE_E
 * @param[return]: error code, see@BT_SVC_ERROR_CODE_E
 ****************************************************************************************
 */
uint8_t bt_svc_tws_ble_switch_role_change(uint8_t new_role);

/**
 ****************************************************************************************
 * @brief Ble switch share infomation function TWS send processing function
 * @param[in]: p_buff, share infomation buffer
 * @param[in]: length, share infomation data length
 * @param[return]: none
 ****************************************************************************************
 */
void bt_svc_tws_ble_switch_share_info(uint8_t *p_buff, uint16_t length);

/**
 ****************************************************************************************
 * @brief Ble switch share infomation function TWS receive processing function
 * @param[in]: rsp_seq, response sequential number
 * @param[in]: p_buff, share infomation buffer
 * @param[in]: length, share infomation data length 
 * @param[return]: none
 ****************************************************************************************
 */
void bt_svc_tws_ble_switch_share_info_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

/**
 ****************************************************************************************
 * @brief Ble switch share infomation function TWS response timeout processing function
 * @param[in]: rsp_seq, response sequential number
 * @param[in]: p_buff, share infomation buffer
 * @param[in]: length, share infomation data length 
 * @param[return]: none
 ****************************************************************************************
 */
void bt_svc_tws_ble_switch_share_info_rsp_timeout_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

/**
 ****************************************************************************************
 * @brief Ble switch share infomation function TWS response processing function
 * @param[in]: rsp_seq, response sequential number
 * @param[in]: p_buff, share infomation buffer
 * @param[in]: length, share infomation data length 
 * @param[return]: none
 ****************************************************************************************
 */
void bt_svc_tws_ble_switch_share_info_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

/**
 ****************************************************************************************
 * @brief Ble switch share infomation function TWS send done processing function
 * @param[in]: cmdcode, send command code
 * @param[in]: rsp_seq, response sequential number
 * @param[in]: p_buff, share infomation buffer
 * @param[in]: length, share infomation data length 
 * @param[return]: none
 ****************************************************************************************
 */
void bt_svc_tws_ble_switch_share_info_send_done(uint16_t cmdcode, uint16_t rsp_seq, uint8_t *ptrParam, uint16_t paramLen);
