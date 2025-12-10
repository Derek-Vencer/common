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
#ifndef __BTS_BT_SINK_CALLBACK_HANDLER_H__
#define __BTS_BT_SINK_CALLBACK_HANDLER_H__

void bts_bt_sink_global_callback(const btif_event_t *event);

void app_tws_ibrt_profile_callback(uint8_t device_id, uint64_t profile,void *param1,void *param2,void* param3);

void bts_bt_sink_register_remote_name_callback(void (*cb)(const bt_bdaddr_t *current_addr, const uint8_t *devName));

void bts_bt_sink_register_connection_complate_callback(void (*cb)(const bt_bdaddr_t *remote, uint8_t error_code));

void app_tws_ibrt_sdp_disconnect_callback(const bt_bdaddr_t *bd_addr);

void app_ibrt_conn_stack_report_create_acl_failed(const bt_bdaddr_t *addr);

int app_tws_ibrt_spp_callback(const bt_bdaddr_t *remote, bt_spp_event_t event, bt_spp_callback_param_t *param);

void bts_bt_sink_cmd_status_callback(const void *para);

bool app_tws_ibrt_avrcp_register_notify_send_callback(uint8_t event);

void app_tws_ibrt_avrcp_register_notify_resp_callback(uint8_t event);

uint16_t app_tws_ibrt_get_ibrt_handle_callback(const bt_bdaddr_t *bd_addr);

void bts_bt_sink_conn_disconnect_callback(const btif_event_t *event);

void bts_bt_conn_acl_state_changed(const bt_bdaddr_t *addr,ibrt_conn_acl_state state,uint8_t reason_code);

#endif
