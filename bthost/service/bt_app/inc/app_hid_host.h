
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
#ifndef __APP_HID_HOST_H__
#define __APP_HID_HOST_H__

#include "hid_service.h"

#ifdef BT_HID_HOST

#ifdef __cplusplus
extern "C" {
#endif

typedef struct hid_control_t* hid_channel_t;

bt_status_t app_hid_host_init(void);
bt_status_t app_hid_host_connect(const bt_bdaddr_t *remote);
bt_status_t app_hid_host_disconnect(const bt_bdaddr_t *remote);
bt_status_t app_hid_host_send_virtual_cable_unplug(const bt_bdaddr_t *remote);
#ifdef __cplusplus
}
#endif

#endif /* BT_HID_HOST */

#endif /* __APP_HID_HOST_H__ */
