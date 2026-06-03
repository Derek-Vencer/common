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
#ifndef __BT_DIP_SERVICE_H__
#define __BT_DIP_SERVICE_H__

#include "adapter_service.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t error_code;
} bt_dip_queried_state_t;

typedef union {
    void *param_ptr;
    bt_dip_queried_state_t *res;
} bt_dip_callback_param_t;

typedef enum {
    BT_DIP_EVENT_START = BT_EVENT_DIP_START,
    BT_DIP_EVENT_QUERIED_SUCCESS,
    BT_DIP_EVENT_QUERIED_FAIL,
    BT_DIP_EVENT_END,
} bt_dip_event_t;

typedef int (*bt_dip_callback_t)(const bt_bdaddr_t *bd_addr, bt_dip_event_t event, bt_dip_callback_param_t param);

bt_status_t bt_dip_init(bt_hid_callback_t callback);

bt_status_t bt_dip_cleanup(void);

#ifdef __cplusplus
}
#endif

#endif /* __BT_HID_SERVICE_H__ */