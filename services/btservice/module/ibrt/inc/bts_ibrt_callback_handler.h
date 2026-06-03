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
#ifndef __BTS_IBRT_CALLBACK_HANDLER_H__
#define __BTS_IBRT_CALLBACK_HANDLER_H__

void bts_ibrt_global_callback(const btif_event_t *event);

void app_ibrt_conn_cmd_complete_callback(uint16_t opcode, uint8_t *param, uint8_t param_len);

int app_ibrt_pack_a2dp_command_accept_event(void* p_remote, uint8_t transaction, uint8_t signal_id);

void bts_ibrt_conn_tws_switch_callback(ibrt_mobile_info_t *mobile_info, uint8_t new_role);

void bts_ibrt_conn_ibrt_state_changed(const bt_bdaddr_t *addr, ibrt_conn_ibrt_state state,ibrt_role_e role,uint8_t reason_code);

void bts_ibrt_conn_report_enhanced_rs_evt(bool allowed, ibrt_role_e role);

#endif
