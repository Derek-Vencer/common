
/* Copyright 2024-2026 VE Starburst. All rights reserved.
 *
 * NOTICE：All information contained herein is, and remains the property of VE
 * Starburst. The intellectual and technical concepts contained herein are
 * proprietary to VE Starburst. and may be covered by patents, patents in process,
 * and are protected by trade secret or copyright law. Dissemination of this
 * information or reproduction of this material is strictly forbidden unless prior
 * written permission is obtained from VE Starburst.
 */

// #include "rt_thread.h"
#include "stb_bt_callback.h"
#include "stb_bt_hal.h"
#include "hal_trace.h"
#include "app_smartvoice_handle.h"

int stb_bt_recv_data(uint8_t connHdl, uint8_t *data, uint16_t len)
{
    // need to implement
    return 0;
}

int stb_bt_connected(uint8_t connHdl)
{
    // need to implement
    return 0;
}

int stb_bt_disconnected(uint8_t connHdl)
{
    // need to implement
    return 0;
}

int stb_bt_send_data(uint8_t connHdl, uint8_t *data, uint16_t len)
{
    return app_sv_huoshan_send_handler(connHdl, data, len);
}

void stb_bt_set_mtu(uint8_t connHdl, uint16_t mtu)
{
    app_sv_huoshan_update_mtu(connHdl, mtu);
}

uint16_t stb_bt_get_mtu(uint8_t connHdl)
{
    return app_sv_huoshan_get_mtu_handler(connHdl);
}

void stb_bt_set_connect_params(uint8_t connHdl, uint16_t interval, uint16_t latency, uint16_t timeout)
{
    app_sv_huoshan_upate_conn_param(connHdl, interval, interval, timeout, latency);
}

uint16_t stb_bt_get_conn_inv_min(uint8_t connHdl)
{
    return app_sv_huoshan_get_conn_interval(connHdl);
}

uint8_t stb_bt_is_connect(void)
{
    return app_sv_huoshan_is_connected();
}

void stb_bt_adapter_rx_handler(uint8_t connHdl, uint8_t *data, uint16_t dataLen)
{
    HUOSHAN_TRACE(0, "%s rec data len:%d", __func__, dataLen);
    HUOSHAN_DUMP8("%2x ", data, dataLen > 8 ? 8 : dataLen);
    stb_bt_recv_data(connHdl, data, dataLen);
}

void stb_bt_adapter_connect_handler(uint8_t connHdl)
{
    stb_bt_connected(connHdl);
}

void stb_bt_adapter_disconnect_handler(uint8_t connHdl)
{
    stb_bt_disconnected(connHdl);
}

void stb_bt_adapter_init(void)
{
    huoshan_callbacks_t cbs = {
        .connect_handler = stb_bt_adapter_connect_handler,
        .disconnect_handler = stb_bt_adapter_disconnect_handler,
        .rx_handler = stb_bt_adapter_rx_handler,
    };
    app_sv_huoshan_register_callback(&cbs);
}