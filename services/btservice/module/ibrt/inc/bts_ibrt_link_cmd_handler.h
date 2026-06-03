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
#ifndef __BTS_IBRT_LINK_CMD_HANDLER_H__
#define __BTS_IBRT_LINK_CMD_HANDLER_H__

typedef struct
{
    struct a2dp_command_t cmd;
    bool is_inuse;
    bool may_be_lost;
    bt_bdaddr_t remote;
    unsigned int timestamp;
    uint32_t sequence_n;
} ibrt_a2dp_stream_packet_t;

void app_ibrt_inform_stop_ibrt_mode(uint8_t *p_buff, uint16_t length);
void app_ibrt_stop_ibrt_mode_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_stop_ibrt_rsp_timeout_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_stop_ibrt_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

void app_ibrt_set_env_cmd(uint8_t *p_buff, uint16_t length);
void app_ibrt_set_env_cmd_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_set_env_cmd_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_set_env_cmd_rsp_timeout_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

void app_ibrt_stop_ibrt_failed(uint8_t *p_buff, uint16_t length);
void app_ibrt_stop_ibrt_failed_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

void app_ibrt_start_ibrt_failed(uint8_t *p_buff, uint16_t length);
void app_ibrt_start_ibrt_failed_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

void app_ibrt_exit_mobile_sniff(uint8_t *p_buff, uint16_t length);
void app_ibrt_exit_mobile_sniff_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

// void app_ibrt_perform_action(uint8_t *p_buff, uint16_t length);
// void app_ibrt_perform_action_handler_v2(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
// void app_ibrt_new_master_is_ready(const bt_bdaddr_t *remote);


void app_ibrt_a2dp_stream_packet_start_switch_check(void* remote);

ibrt_a2dp_stream_packet_t *app_ibrt_find_free_a2dp_stream_packet(bt_bdaddr_t *remote);
uint32_t app_ibrt_new_a2dp_stream_packet_seqn(void);

void app_ibrt_enhanced_rs_send(uint8_t *p_buff, uint16_t length);
void app_ibrt_enhanced_rs_cmd_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_enhanced_rs_rsp_timeout_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ibrt_enhanced_rs_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

#endif

