/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
#ifndef __APP_BT_PAN_H__
#define __APP_BT_PAN_H__

#ifdef BT_PAN_SUPPORT
#include "pan_service.h"
#ifdef __cplusplus
extern "C" {
#endif


void app_bt_pan_register_callbacks(bt_pan_register_t *pan_register_callbacks);

int app_bt_pan_send_ARP_protocol_data(uint8_t device_id, const uint8_t *payload_data, uint16_t payload_length);

int app_bt_pan_send_IPv4_protocol_data(uint8_t device_id, const uint8_t *payload_data, uint16_t payload_length);

int app_bt_pan_send_IPv6_protocol_data(uint8_t device_id, const uint8_t *payload_data, uint16_t payload_length);

int app_bt_pan_send_address_specified_protocol_data(uint8_t device_id, const bt_pan_ethernet_data_info *data);

void app_bt_pan_init(void);

bool app_bt_pan_is_connected(uint8_t device_id);

int app_bt_connect_pan_profile(bt_bdaddr_t *remote);

void app_bt_disconnect_pan_profile(uint8_t device_id);

void app_bt_pan_test_send_ARP_probe(uint8_t device_id);

void app_bt_pan_test_send_ARP_protocol_request(uint8_t device_id);

void app_bt_pan_test_send_DHCP_protocol_discover(uint8_t device_id);

void app_bt_pan_test_send_DHCP_protocol_request(uint8_t device_id);

void app_bt_pan_test_send_DNS_protocol_request(uint8_t device_id, const char* domain_name, uint32_t len);

void app_bt_pan_test_send_ICMP_echo_request(uint8_t device_id);

void app_bt_pan_test_send_NDP_request(uint8_t device_id);

void app_bt_pan_test_send_HTTP_request(uint8_t device_id);

void app_bt_pan_test_send_HTTP_ipv6_request(uint8_t device_id);

#ifdef __cplusplus
}
#endif

#endif /* BT_PAN_SUPPORT */

#endif /* __APP_BT_PAN_H__ */

