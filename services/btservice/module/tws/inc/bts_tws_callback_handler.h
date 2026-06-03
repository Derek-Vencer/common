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
#ifndef __BTS_TWS_CALLBACK_HANDLER_H__
#define __BTS_TWS_CALLBACK_HANDLER_H__

#include "app_ibrt_conn_evt.h"

void tws_besaud_init(void);

void bts_tws_global_callback(const btif_event_t *event);

void bts_tws_besaud_callback(btif_besaud_event event);

void tws_besaud_extra_channel_send_data(const void* data, uint32_t len, void* context);

void bts_ibrt_conn_cmd_send_fail_error_handler(uint16_t cmd_code,uint8_t *p_buff);

void bts_tws_cmd_status_callback(const void *para);

void bts_tws_disconnect_tws_link_callback(const btif_event_t *event);

void bts_tws_conn_paring_state_changed(ibrt_conn_pairing_state state,uint8_t reason_code);

void bts_tws_conn_acl_state_changed(ibrt_conn_acl_state state,uint8_t reason_code);

void bts_tws_conn_role_changed(const bt_bdaddr_t *addr,ibrt_conn_role_change_state state,ibrt_role_e role);

#endif
