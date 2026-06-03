/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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
#ifndef __APP_UI_SHARE_INFO_H__
#define __APP_UI_SHARE_INFO_H__

typedef void (*app_ui_merge_nv_callback_t)(
    ibrt_ui_nv_info_rec *src_list,
    ibrt_ui_nv_info_rec *local_list,
    ibrt_ui_nv_info_rec *merged_list
);

void app_ui_share_info_callback(void);

void app_ui_sync_info_cb_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ui_sync_info_cb_rsp_handler(uint8_t *p_buff, uint16_t length);
void app_ui_sync_info_cb_rsp_timeout_handler(uint8_t *p_buff, uint16_t length);
bool app_ui_sync_info_get_local_pending_req();
bool app_ui_sync_info_get_peer_pending_req();
bt_bdaddr_t *app_ui_sync_info_get_local_pending_addr(void);
bt_bdaddr_t * app_ui_sync_info_get_peer_pending_addr(void);

void app_ui_register_merge_nv_callback(app_ui_merge_nv_callback_t callback);

#endif /* __APP_UI_SHARE_INFO_H__ */
