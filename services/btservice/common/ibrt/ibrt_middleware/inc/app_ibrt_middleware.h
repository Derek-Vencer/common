/***************************************************************************
*
*Copyright 2015-2019 BES.
*All rights reserved. All unpublished rights reserved.
*
*No part of this work may be used or reproduced in any form or by any
*means, or stored in a database or retrieval system, without prior written
*permission of BES.
*
*Use of this work is governed by a license granted by BES.
*This work contains confidential and proprietary information of
*BES. which is protected by copyright, trade secret,
*trademark and other intellectual property rights.
*
****************************************************************************/

#ifndef __APP_IBRT_MIDDLEWARE_H__
#define __APP_IBRT_MIDDLEWARE_H__

#ifdef __cplusplus
extern "C" {
#endif

/*****************************header include********************************/

enum {
    DISCONNECTED = 0,
    CONNECTED    = 1,

    CONNECTION_STATE_NUM,
};



typedef struct
{
    int32_t a2dp_local_volume;
    int32_t hfp_local_volume;
} TWS_VOLUME_SYNC_INFO_T;

/// Definition of the bits preventing the BTC from enter sniff
enum app_ibrt_if_prevent_sniff
{
    /// Flag indicating that the OTA process is ongoing
    OTA_ONGOING                       = 0x01,
    /// Flag indicating that vocie record is ongoing
    AI_VOICE_RECORD                   = 0x02,
    /// Flag indicating that AVRCP status change is ongoing
    AVRCP_STATUS_CHANING         = 0x04,
    /// Flag indicating that Call is on ongoing
    HFP_CALL_ONGOING = 0x08,
};

/****************************function declearation**************************/
/*---------------------------------------------------------------------------
 *            app_ibrt_middleware_init
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    initialize the tws interface related parameter
 *
 * Parameters:
 *    void
 *
 * Return:
 *    void
 */
void app_ibrt_middleware_init(void);

/*---------------------------------------------------------------------------
 *            app_ibrt_middleware_ibrt_connected_handler
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    handler of ibrt connected
 * Parameters:
 *    void
 *
 * Return:
 *    void
 */
void app_ibrt_middleware_ibrt_connected_handler(void);

/*---------------------------------------------------------------------------
 *            app_ibrt_middleware_ibrt_disconnected_handler
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    handler of ibrt disconnected
 * Parameters:
 *    void
 *
 * Return:
 *    void
 */
void app_ibrt_middleware_ibrt_disconnected_handler(void);

/*---------------------------------------------------------------------------
 *            app_ibrt_middleware_ble_connected_handler
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    handler of ble connected event for tws system
 *
 * Parameters:
 *    void
 *
 * Return:
 *    void
 */
void app_ibrt_middleware_ble_connected_handler(void);

/*---------------------------------------------------------------------------
 *            app_ibrt_middleware_ble_disconnected_handler
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    ble disconnected handler for tws system
 *
 * Parameters:
 *    void
 *
 * Return:
 *    void
 */
void app_ibrt_middleware_ble_disconnected_handler(void);

/*---------------------------------------------------------------------------
 *            app_ibrt_middleware_tws_connected_sync_info
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    sync BLE\BISTO\AI info after tws connected
 *
 * Parameters:
 *    void
 *
 * Return:
 *    void
 */
void app_ibrt_middleware_tws_connected_sync_info(void);

/*---------------------------------------------------------------------------
 *            app_ibrt_middleware_tws_connected_handler
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    handler of tws connected event
 *
 * Parameters:
 *    void
 *
 * Return:
 *    void
 */
void app_ibrt_middleware_tws_connected_handler(void);

/*---------------------------------------------------------------------------
 *            app_ibrt_middleware_tws_disconnected_handler
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    handler of tws disconnected event
 *
 * Parameters:
 *    void
 *
 * Return:
 *    void
 */
void app_ibrt_middleware_tws_disconnected_handler(void);


/*---------------------------------------------------------------------------
 *            app_ibrt_middleware_handle_click
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    handle the power key click event
 *
 * Parameters:
 *    void
 *
 * Return:
 *    void
 */
void app_ibrt_middleware_handle_click(void);


void app_tws_ai_send_cmd_to_peer(uint8_t *p_buff, uint16_t length);
void app_tws_ai_rev_peer_cmd_hanlder(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_tws_ai_send_cmd_with_rsp_to_peer(uint8_t *p_buff, uint16_t length);
void app_tws_ai_send_cmd_rsp_to_peer(uint8_t *p_buff, uint16_t rsp_seq, uint16_t length);
void app_tws_ai_rev_peer_cmd_with_rsp_hanlder(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_tws_ai_rev_cmd_rsp_from_peer_hanlder(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_tws_ai_rev_cmd_rsp_timeout_hanlder(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_if_prevent_sniff_set(uint8_t *p_mobile_addr, uint16_t prv_sniff_bit);
void app_ibrt_if_prevent_sniff_clear(uint8_t *p_mobile_addr, uint16_t prv_sniff_bit);
bool app_ibrt_if_customer_prevent_sniff(bt_bdaddr_t *p_mobile_addr);
void app_ibrt_middleware_ui_role_updated_handler(uint8_t newRole);

#ifdef GFPS_ENABLED
void app_ibrt_share_fastpair_info(uint8_t *p_buff, uint16_t length);
void app_ibrt_share_fastpair_info_received_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
#endif

int app_ibrt_middleware_fill_debug_info(char* buf, unsigned int buf_len);

void app_ibrt_middleware_exit_sniff_with_mobile(uint8_t* mobileAddr);
void app_ibrt_reconect_mobile_after_factorty_test(void);

void app_ibrt_middleware_write_bt_local_address(uint8_t* btAddr);
void app_ibrt_middleware_write_ble_local_address(uint8_t* bleAddr);
uint8_t *app_ibrt_middleware_get_bt_local_address(void);
uint8_t *app_ibrt_middleware_get_ble_local_address(void);
uint8_t *app_ibrt_middleware_get_bt_peer_address(void);
void app_ibrt_middleware_big_little_switch(uint8_t *in, uint8_t *out, uint8_t len);
bool app_ibrt_middleware_is_ui_slave(void);
bool app_ibrt_middleware_is_ui_master(void);


void app_ibrt_middleware_prevent_sniff_set(uint8_t *p_mobile_addr, uint16_t prv_sniff_bit);
void app_ibrt_middleware_reset_prevent_sniff_clear(const uint8_t *p_mobile_addr);
void app_ibrt_middleware_prevent_sniff_clear(uint8_t *p_mobile_addr, uint16_t prv_sniff_bit);
#ifdef __cplusplus
}
#endif

#endif /* #ifndef __APP_IBRT_MIDDLEWARE_H__ */
