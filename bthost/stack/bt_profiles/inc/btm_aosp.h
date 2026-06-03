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
#ifndef __BTM_AOSP_H__
#define __BTM_AOSP_H__
#include "bluetooth.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define APCF_MAX_UUID_DATA_LEN          (16)
#define APCF_MAX_SERV_DATA_LEN          (29)

typedef enum btm_aosp_apcf_opcode
{
    /// 0x00 - APCF Enable
    BTM_AOSP_APCF_ENABLE = 0x00,
    /// 0x01 - APCF Set Filtering parameters
    BTM_AOSP_APCF_SET_FILTERING_PARAMETERS = 0X01,
    /// 0x02 - APCF Broadcaster Address
    BTM_AOSP_APCF_BROADCASTER_ADDRESS = 0X02,
    /// 0x03 - APCF Service UUID
    BTM_AOSP_APCF_SERVICE_UUID = 0X03,
    /// 0x04 - APCF Service Solicitation UUID
    BTM_AOSP_APCF_SERVICE_SOLICITATION_UUID = 0X04,
    /// 0x05 - APCF Local Name
    BTM_AOSP_APCF_LOCAL_NAME = 0X05,
    /// 0x06 - APCF Manufacturer Data
    BTM_AOSP_APCF_MANUFACTURER_DATA = 0X06,
    /// 0x07 - APCF Service Data
    BTM_AOSP_APCF_SERVICE_DATA = 0X07,
    /// 0x08 - APCF Transport Discovery Service
    BTM_AOSP_APCF_TRANSPORT_DISCOVERY_SERVICE = 0X08,
    /// 0x09 - APCF AD Type Filter
    BTM_AOSP_APCF_AD_TYPE_FILTER = 0X09,
    /// 0x10 ~ 0xAF - Reserved for future use
    /// 0xB0 ~ 0xDF - Reserved for vendor
    /// 0xE0 ~ 0xFE - Reserved for future use
    /// 0xFF - APCF Read extended Features
    BTM_AOSP_APCF_READ_EXTENDED_FEATURES = 0XFF,
} btm_aosp_apcf_op_e;

/// Delete will clear the specific filter along with associated feature entries in other tables.
/// Clear will clear all the filters and associated entries in other tables.
typedef enum btm_aosp_apcf_action
{
    /// 0x00 - Add
    BTM_AOSP_APCF_ACTION_ADD = 0x00,
    /// 0x01 - Delete
    BTM_AOSP_APCF_ACTION_DEL = 0x01,
    /// 0x02 - Clear
    BTM_AOSP_APCF_ACTION_CLR = 0x02,
} btm_aosp_apcf_action_e;

typedef enum btm_aosp_cmd_type
{
    BTM_AOSP_CMD_APCF = 0x00,
} btm_aosp_cmd_type_e;

// 0x00: Enable
typedef struct
{
    uint8_t enable;
} aosp_apcf_enable_param_t;

// 0x01: Set Filtering Parameters
typedef struct
{
    uint8_t feature_selection;
    uint8_t logic_type;
    uint8_t filter_logic_type;
    uint8_t rssi_high_thres;
    uint8_t rssi_low_thres;
    uint8_t delivery_mode;
    uint8_t onfound_timeout;
    uint8_t onfound_timeout_cnt;
    uint8_t onlost_timeout;
    uint8_t num_of_tracking_entries;
} aosp_apcf_filter_param_param_t;

// 0x02: Broadcaster Address
typedef struct
{
    uint8_t broadcaster_address[6];
    uint8_t broadcaster_address_type;
} aosp_apcf_broadcaster_addr_param_t;

// 0x03: Service UUID
typedef struct
{
    uint8_t uuid_len; // 2, 4, or 16
    uint8_t uuid[APCF_MAX_UUID_DATA_LEN];
    uint8_t uuid_mask[APCF_MAX_UUID_DATA_LEN];
} aosp_apcf_service_uuid_param_t;

// 0x04: Solicitation UUID
typedef struct
{
    uint8_t uuid_len; // 2, 4, or 16
    uint8_t uuid[APCF_MAX_UUID_DATA_LEN];
    uint8_t uuid_mask[APCF_MAX_UUID_DATA_LEN];
} aosp_apcf_solicitation_uuid_param_t;

// 0x05: Local Name
typedef struct
{
    uint8_t name_len;
    uint8_t name[APCF_MAX_SERV_DATA_LEN];
} aosp_apcf_local_name_param_t;

// 0x06: Manufacturer Data
typedef struct
{
    uint8_t data_len;
    uint8_t data[APCF_MAX_SERV_DATA_LEN];
    uint8_t data_mask[APCF_MAX_SERV_DATA_LEN];
} aosp_apcf_manufacturer_data_param_t;

// 0x07: Service Data
typedef struct
{
    uint8_t data_len;
    uint8_t data[APCF_MAX_SERV_DATA_LEN];
    uint8_t data_mask[APCF_MAX_SERV_DATA_LEN];
} aosp_apcf_service_data_param_t;

// 0x09: AD Type Filter
typedef struct
{
    uint8_t ad_type;
    uint8_t data_len;
    const uint8_t *data;
    const uint8_t *data_mask;
} aosp_apcf_ad_type_param_t;

// Union of all APCF parameters
typedef union
{
    aosp_apcf_enable_param_t              enable;
    aosp_apcf_filter_param_param_t        filter_param;
    aosp_apcf_broadcaster_addr_param_t    broadcaster_addr;
    aosp_apcf_service_uuid_param_t        service_uuid;
    aosp_apcf_solicitation_uuid_param_t   solicitation_uuid;
    aosp_apcf_local_name_param_t          local_name;
    aosp_apcf_manufacturer_data_param_t   manufacturer_data;
    aosp_apcf_service_data_param_t        service_data;
    aosp_apcf_ad_type_param_t             ad_type;
} btm_aosp_apcf_params_u;

typedef struct btm_aosp_apcf_cmd_cmpl
{
    uint8_t status;
    /// @see btm_aosp_apcf_op_e
    uint8_t apcf_opcode;
    /// Different opcode return params
    union
    {
        /// BTM_AOSP_APCF_ENABLE
        uint8_t enable;
        /// For 0x01 - 0x09
        struct
        {
            uint8_t action;
            uint8_t ava_space;
        };
        /// BTM_AOSP_APCF_READ_EXTENDED_FEATURES
        uint16_t ext_features;
    };
} btm_aosp_apcf_cmd_cmpl_t;

typedef union btm_aosp_cmd_cmpl
{
    /// BTM_AOSP_CMD_APCF
    btm_aosp_apcf_cmd_cmpl_t apcf_cmpl;

    /// Others
} btm_aosp_cmd_cmpl_t;

/**
 * @brief An AOSP cmd event callback for all aosp cmd(s)
 *
 */
typedef void (*btm_aosp_cmd_cmpl_cb)(btm_aosp_cmd_type_e cmd_type, const btm_aosp_cmd_cmpl_t *cmpl);

/**
 * @brief Register AOSP command complete callabck for receiving complete event
 *
 * @param[in] cmpl_cb  @see btm_aosp_cmd_cmpl_cb
 *
 * @return bt_status_t Register status
 */
bt_status_t btm_aosp_cmd_complete_cb_register(btm_aosp_cmd_cmpl_cb cmpl_cb);

/**
 * @brief Use this to enable/disable/setup the Advertising Packet Content Filter (APCF) in the controller.
 *
 * @param[in] op       @see btm_aosp_apcf_op_e, opcode
 * @param[in] action   @see btm_aosp_apcf_action_e, action
 * @param[in] filter_index
 *                     Filter index to action, up to num max filter
 * @param[in] params   Parameters
 *
 * @return bt_status_t Cmd send status
 */
bt_status_t btm_aosp_apcf_cmd_send(btm_aosp_apcf_op_e op, btm_aosp_apcf_action_e action,
                                   uint8_t filter_index, const btm_aosp_apcf_params_u *params);

#if defined(__cplusplus)
}
#endif

#endif /* __BTM_AOSP_H__ */