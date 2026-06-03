
/* Copyright 2024-2026 VE Starburst. All rights reserved.
 *
 * NOTICE：All information contained herein is, and remains the property of VE
 * Starburst. The intellectual and technical concepts contained herein are
 * proprietary to VE Starburst. and may be covered by patents, patents in process,
 * and are protected by trade secret or copyright law. Dissemination of this
 * information or reproduction of this material is strictly forbidden unless prior
 * written permission is obtained from VE Starburst.
 */

#ifndef _STB_BT_HAL_H
#define _STB_BT_HAL_H

#ifdef __cplusplus
extern "C" {
#endif

int stb_bt_send_data(uint8_t conn_index, uint8_t *frame, uint16_t frame_length);

void stb_bt_set_mtu(uint8_t conn_index, uint16_t mtu);

uint16_t stb_bt_get_mtu(uint8_t conn_index);

void stb_bt_set_connect_params(uint8_t conn_index, uint16_t interval, uint16_t latency, uint16_t timeout);

uint16_t stb_bt_get_conn_inv_min(uint8_t conn_index);

uint8_t stb_bt_is_connect(void);

void stb_bt_adapter_init(void);

#ifdef __cplusplus
}
#endif

#endif
