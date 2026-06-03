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
#ifdef BT_HID_HOST

#undef MOUDLE
#define MOUDLE APP_BT

#include <stdio.h>
#include "cmsis_os.h"
#include "hal_uart.h"
#include "hal_timer.h"
#include "lockcqueue.h"
#include "hal_trace.h"
#include "app_bt_cmd.h"
#include "app_trace_rx.h"
#include "bluetooth.h"
#include "besbt.h"
#include "cqueue.h"
#include "btapp.h"
#include "app_bt.h"
#include "bt_if.h"
#include "app_bt_func.h"
#include "me_api.h"
#include "app_hid_host.h"
#include "hid_api.h"
#include "bt_sys_config.h"
#include "bt_source.h"

static int app_bt_hid_host_callback(const bt_bdaddr_t *remote, bt_hid_event_t event, bt_hid_callback_param_t param)
{
    struct BT_SOURCE_DEVICE_T* source_dev = NULL;

    uint8_t device_id = param.opened->device_id;
    uint8_t error_code = param.opened->error_code;
    hid_channel_t chan = (hid_channel_t)param.opened->channel;

    if (device_id == BT_DEVICE_INVALID_ID && event == BT_HID_EVENT_CLOSED)
    {
        // hid profile is closed due to acl created fail
        DEBUG_INFO(0, "::BT_HID_EVENT_CLOSED acl created error %x", param.closed->error_code);
        return 0;
    }

    source_dev = app_bt_source_get_device(device_id);

    ASSERT(source_dev->base_device->hid_channel == chan, "hid host channel must match");

    DEBUG_INFO(0, "(d%x) %s channel %p event 0x%04x errno %02x %02x:%02x:***:%02x",
                device_id, __func__, chan, event, error_code,
                remote->address[0], remote->address[1], remote->address[5]);

    switch (event)
    {
        case BT_HID_EVENT_OPENED:
        {
            source_dev->base_device->hid_conn_flag = true;
            btif_report_bt_event(&source_dev->base_device->remote, BT_EVENT_HID_OPENED, param.opened);
        }
        break;
        case BT_HID_EVENT_CLOSED:
        {
            source_dev->base_device->hid_conn_flag = false;
            btif_report_bt_event(&source_dev->base_device->remote, BT_EVENT_HID_CLOSED, param.closed);
        }
        break;
        case BT_HID_EVENT_TXDONE:
            btif_report_bt_event(&source_dev->base_device->remote, BT_EVENT_HID_TXDONE, param.txdone);
        break;
        case BT_HID_EVENT_RCV_CTL_DATA:
            DEBUG_INFO(0, "BT_HID_EVENT_RCV_CTL_DATA");
            btif_report_bt_event(&source_dev->base_device->remote, BT_EVENT_HID_RCV_CTL_DATA, param.ctl_data);
        break;
        case BT_HID_EVENT_RCV_INT_DATA:
            DEBUG_INFO(0, "BT_HID_EVENT_RCV_INT_DATA");
            btif_report_bt_event(&source_dev->base_device->remote, BT_EVENT_HID_RCV_INT_DATA, param.int_data);
        break;
        case BT_HID_EVENT_RCV_SDP_DATA:
            DEBUG_INFO(0, "BT_HID_EVENT_RCV_SDP_DATA");

            btif_report_bt_event(&source_dev->base_device->remote, BT_EVENT_HID_SDP_REC_DATA, param.sdp_data);
        break;
            break;
    default:
        break;
    }

    return 0;
}

bt_status_t app_hid_host_connect(const bt_bdaddr_t *remote)
{
    DEBUG_INFO(8, "%s %02x:%02x:%02x:%02x:%02x:%02x", __func__,
            remote->address[0], remote->address[1], remote->address[2],
            remote->address[3], remote->address[4], remote->address[5]);

    struct bt_defer_param_t param_a = bt_alloc_param_size(remote, 6);

    bt_defer_call_func_1(btif_hid_connect, param_a);

    return BT_STS_SUCCESS;
}

bt_status_t app_hid_host_disconnect(const bt_bdaddr_t *remote)
{
    uint8_t device_id = app_bt_get_connected_device_id_byaddr(remote);
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);

    DEBUG_INFO(1, "(d%x) %s", device_id, __func__);

    if (!curr_device)
    {
        DEBUG_INFO(0, "app_hid_host_disconnect: no connection");
        return BT_STS_FAILED;
    }

    if (!curr_device->hid_conn_flag)
    {
        DEBUG_INFO(0, "(d%x) %s: hid not connected", curr_device->device_id, __func__);
        return BT_STS_FAILED;
    }

    if (curr_device)
    {
        app_bt_start_custom_function_in_bt_thread((uint32_t)curr_device->hid_channel, (uint32_t)NULL, (uint32_t)btif_hid_disconnect);
    }

    return BT_STS_SUCCESS;
}

bt_status_t app_hid_host_send_virtual_cable_unplug(const bt_bdaddr_t *bd_addr)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);

    if (!curr_device || !curr_device->hid_conn_flag)
    {
        DEBUG_INFO(3, "(d%x) %s hid not connected", curr_device->device_id, __func__);
        return BT_STS_FAILED;
    }

    app_bt_start_custom_function_in_bt_thread((uint32_t)curr_device->hid_channel, (uint32_t)NULL, (uint32_t)btif_hid_send_virtual_cable_unplug);

    return BT_STS_SUCCESS;
}

static bool hid_host_initialized = false;
bt_status_t app_hid_host_init(void)
{
    bthost_cfg_t* bt_host_cfg = bt_host_get_cfg();
    DEBUG_INFO(2, "%s source_enable %d", __func__, bt_host_cfg->bt_source_enable);

    if (hid_host_initialized)
    {
        return BT_STS_SUCCESS;
    }

    hid_host_initialized = true;

    // init hid channle (include host and device)
    btif_hid_channel_init();

    // init some interface
    btif_hid_intf_init();

    // init l2cap channel
    btif_hid_init_port();

    if (bt_host_cfg->bt_source_enable)
    {
        struct BT_SOURCE_DEVICE_T* source_dev = NULL;
        for (int i = 0; i < BT_DEVICE_NUM; i += 1)
        {
            source_dev = app_bt_source_get_device(i);
            if (source_dev)
            {
                source_dev->base_device->hid_channel = btif_hid_channel_alloc_and_init(i, false, app_bt_hid_host_callback);
                source_dev->base_device->capture_wait_timer_id = NULL;
                source_dev->base_device->hid_wait_disc_timer_id = NULL;
                source_dev->base_device->hid_conn_flag = false;
                source_dev->base_device->wait_send_capture_key = false;
            }
        }
    }

    return BT_STS_SUCCESS;
}

#endif /* BT_HID_HOST*/
