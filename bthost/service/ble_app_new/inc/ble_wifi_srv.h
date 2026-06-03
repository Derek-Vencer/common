/***************************************************************************
 *
 * Copyright 2020-2025 BES.
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
#ifndef __BLE_WIFI_SRV_H__
#define __BLE_WIFI_SRV_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "bes_dp_api.h"

#ifdef BLE_WIFI_SRV_ENABLED
#define BLE_INVALID_CONNECTION_INDEX    0xFF

struct ble_wifi_server_env_tag
{
    uint8_t connectionIndex;
    uint16_t NtfIndCfg;
};

void ble_wifi_srv_init(void);
void ble_wifi_srv_register_event_cb(ble_wifi_event_cb callback);
uint8_t ble_wifi_srv_send_data_via_notification(uint8_t* data, uint32_t len);

#endif

#ifdef __cplusplus
}
#endif

#endif /* __BLE_WIFI_SRV_H__ */
