
/* Copyright 2024-2026 VE Starburst. All rights reserved.
 *
 * NOTICE：All information contained herein is, and remains the property of VE
 * Starburst. The intellectual and technical concepts contained herein are
 * proprietary to VE Starburst. and may be covered by patents, patents in process,
 * and are protected by trade secret or copyright law. Dissemination of this
 * information or reproduction of this material is strictly forbidden unless prior
 * written permission is obtained from VE Starburst.
 */

#ifndef _STB_BT_CALLBACK_H
#define _STB_BT_CALLBACK_H

#include "plat_types.h"

#ifdef __cplusplus
extern "C" {
#endif

int stb_bt_connected(uint8_t conn_index);

int stb_bt_disconnected(uint8_t conn_index);

int stb_bt_recv_data(uint8_t conn_index, uint8_t *data, uint16_t data_len);

#ifdef __cplusplus
}
#endif

#endif
