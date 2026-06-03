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
#ifndef __BTS_IBRT_TWS_SWITCH_H__
#define __BTS_IBRT_TWS_SWITCH_H__

#define RS_MIN_THRESHOLD                     (10) // MS
#define APP_IBRT_PROFILE_CONN_PROTECT_TIME   (3000)
#define APP_IBRT_PROFILE_DISC_PROTECT_TIME   (2000)
#define APP_IBRT_AUDIO_TRIGGER_PROTECT_TIME  (2000)

typedef enum {
    RS_TASK_INVALID_ID = 0,
    // profile conn/disconn
    RS_TASK_HFP_CONN_D0,
    RS_TASK_HFP_DISC_D0,
    RS_TASK_HFP_CONN_D1,
    RS_TASK_HFP_DISC_D1,
    RS_TASK_HFP_CONN_D2,
    RS_TASK_HFP_DISC_D2,
    RS_TASK_A2DP_CONN_D0,
    RS_TASK_A2DP_DISC_D0,
    RS_TASK_A2DP_CONN_D1,
    RS_TASK_A2DP_DISC_D1,
    RS_TASK_A2DP_CONN_D2,
    RS_TASK_A2DP_DISC_D2,
    RS_TASK_AVRCP_CONN_D0,
    RS_TASK_AVRCP_DISC_D0,
    RS_TASK_AVRCP_CONN_D1,
    RS_TASK_AVRCP_DISC_D1,
    RS_TASK_AVRCP_CONN_D2,
    RS_TASK_AVRCP_DISC_D2,
    RS_TASK_SDP_CONN_D0,
    RS_TASK_SDP_CONN_D1,
    RS_TASK_SDP_CONN_D2,
    //
    RS_TASK_AVRCP_KEY,
    RS_TASK_A2DP_TRIGGER,
    RS_TASK_USER_PROC,
    RS_TASK_NUM,
} RS_TASK_ID;

typedef enum {
    RS_CTL_SET,
    RS_CTL_CLR,
} RS_CTL_TYPE;

/* TWS enhanced switch implementation */
typedef struct {
    uint8_t initiator_role;
    uint8_t req;
    uint8_t responsor_role;
    uint8_t rsp;
    uint8_t master_initiate_rs;
    uint16_t master_rs_delay;
} ERS_PKT_T;

#ifdef __cplusplus
extern "C" {
#endif

void bts_ibrt_conn_rs_mgr_init(void);

bool bts_ibrt_conn_rs_task_set(uint8_t task_id, uint16_t life_cycle);

bool bts_ibrt_conn_user_rs_task_set(uint16_t life_cycle);

void bts_ibrt_conn_user_rs_task_clr(void);

bool bts_ibrt_conn_is_enable_rs_delay(void);

void bts_ibrt_rs_profile_protect(uint8_t device_id, int profile_id, bool enable, bool is_connect_profile);

void bts_ibrt_set_profile_connect_protect(uint8_t device_id, int profile_id);
void bts_ibrt_clear_profile_connect_protect(uint8_t device_id, int profile_id);
void bts_ibrt_set_profile_disconnect_protect(uint8_t device_id, int profile_id);
void bts_ibrt_clear_profile_disconnect_protect(uint8_t device_id, int profile_id);
void bts_ibrt_set_audio_trigger_protect(void);

void bts_ibrt_clear_audio_trigger_protect(void);


/**
 ****************************************************************************************
 * @brief Check if all link run role switch complete
 *
 * @return
 * <table>
 * <tr><th>Value        <th>Description
 * <tr><td>True  <td>all link run role switch complete
 * <tr><td>False  <td>exist link run role switch event failed or current doing role switch
 * </table>
 ****************************************************************************************
 */
bool bts_ibrt_conn_all_ibrt_role_complete(ibrt_role_e role);

/**
 ****************************************************************************************
 * @brief Start ibrt role switch for mobile device
 *
 * @param[in] addr       Mobile address
 *
 * @return An error status
 ****************************************************************************************
 */
ibrt_status_t bts_ibrt_conn_tws_role_switch(const bt_bdaddr_t *addr);

/**
 ****************************************************************************************
 * @brief Handling RS done event
 *
 * @param[in] deviceId       Device id
 ****************************************************************************************
 */
void app_ibrt_conn_switch_to_ibrt_slave_process(uint8_t deviceId);

/**
 ****************************************************************************************
 * @brief for codec error handle, switch to ibrt slave
 *
 ****************************************************************************************
 */
void bts_ibrt_conn_codec_error_handler(uint32_t streamId, uint32_t streamType);


/**
 ****************************************************************************************
 * @brief Check if exist link on role switch
 *
 * @return
 * <table>
 * <tr><th>Value        <th>Description
 * <tr><td>True  <td>exist link on role switch
 * <tr><td>False  <td>no link is on role switch
 * </table>
 ****************************************************************************************
 */
bool bts_ibrt_conn_any_role_switch_running(void);

bool bts_ibrt_conn_is_tws_role_switching(const bt_bdaddr_t *mobile_addr);

bool app_tws_ibrt_slave_cmd_filter(uint8_t* pbuf, uint32_t length);

/**
 ****************************************************************************************
 * @brief Perform ibrt-role-switch for all links
 *
 * @return
 * <table>
 *      none
 ****************************************************************************************
 */
void bts_ibrt_conn_all_dev_start_tws_role_switch(void);

/**
 ****************************************************************************************
 * @brief initiate role switch by btc, when ibrt connected
 *
 * @param[in] addr       Mobile addr
 ****************************************************************************************
 */

void bts_ibrt_conn_btc_role_switch(const bt_bdaddr_t *addr);

void app_ibrt_conn_enhanced_rs_rsp_cb(uint16_t rsp_seq, uint8_t *buf, uint16_t len);

void app_ibrt_conn_enhanced_rs_cmd_cb(uint16_t rsp_seq, uint8_t *buf, uint16_t len);

void app_ibrt_conn_delay_rs_register_cb(void (*cb)(void));

void app_ibrt_register_need_reconnect_cb(void (*cb)(struct bdaddr_t *addr, uint8 reason));

/**
 ****************************************************************************************
 * @brief initiate role switch without mobile link
 *
 * @param[in] msDelay       Specify the delay of the role switch
 ****************************************************************************************
 */
void app_ibrt_conn_enhanced_rs(uint16_t msDelay);

#ifdef __cplusplus
}
#endif

#endif
