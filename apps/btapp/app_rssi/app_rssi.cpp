/***************************************************************************
 *
 * Copyright 2015-2024 BES.
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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app_tws_ibrt.h"
#include "app_ibrt_middleware.h"
#include "app_rssi.h"
#include "bt_drv_reg_op.h"
#include "hal_trace.h"
#include "bts_module_if.h"
#include "bts_tws_conn.h"

/* Open for debug */
// #define APP_RSSI_DEBUG



#define APP_RSSI_GET_TWS_HANDLE()       (bts_tws_if_get_tws_acl_handle())
#define APP_RSSI_IS_TWS_CONNECT()       (bts_tws_if_is_tws_link_connected())
#define APP_RSSI_IS_TWS_LEFT_SIDE()     (bts_tws_if_is_local_left_side())


typedef struct {
    int8_t buf[APP_RSSI_WINDOW_SIZE];
    int8_t value;
    int8_t value_min;
    int8_t value_max;
    int16_t value_sum;
    uint8_t size;
    uint8_t head;
    uint8_t tail;
} rssi_window_t;


static void app_rssi_window_init(rssi_window_t *w);
static void app_rssi_window_push(rssi_window_t *w, int8_t value);
static void app_rssi_window_dump(rssi_window_t *w);
static int8_t app_rssi_value_get(rssi_window_t *w);
static int8_t app_rssi_min_value_get(rssi_window_t *w);
static int8_t app_rssi_max_value_get(rssi_window_t *w);


static void app_rssi_read_tws_rssi();
static void app_rssi_dump_tws_rssi();
static void app_rssi_read_mobile_rssi();
static void app_rssi_dump_mobile_rssi();


typedef struct
{
    remote_rssi_t remote_rssi[APP_RSSI_MAX_RECORD_NUM];
} rssi_tws_pkt_t;

static void app_rssi_tws_cmd_func_reg();
static void app_rssi_tws_sync_process();
static void app_rssi_send_get_peer_rssi_req(rssi_tws_pkt_t *preq_pkt);
static void app_rssi_send_get_peer_rssi_rsp(uint16_t rsp_seq, rssi_tws_pkt_t *prsp_pkt);


static void app_rssi_vender_ll_monitor_callback(uint16_t handle, uint8_t ser);


typedef struct
{
    remote_rssi_t rssi;
    rssi_window_t window;
} remote_rssi_obj_t;

typedef struct
{
    osTimerId sample_timer;
    uint32_t runtime;

    rssi_t tws_rssi;
    rssi_window_t tws_window;

    remote_rssi_obj_t remote_rssi_obj[APP_RSSI_MAX_RECORD_NUM];
} rssi_contrl_t;

static rssi_contrl_t rssi_contrl;

static void app_rssi_window_init(rssi_window_t *w)
{
    memset(w, 0x00, sizeof(rssi_window_t));
}

static void app_rssi_window_push(rssi_window_t *w, int8_t value)
{
    w->value_sum += value;

    if (w->size == APP_RSSI_WINDOW_SIZE) {
        w->value_sum -= w->buf[w->head];
        w->head++;
    } else {
        w->size++;
    }

    w->buf[w->tail] = value;
    w->tail++;

    if (w->tail == APP_RSSI_WINDOW_SIZE) {
        w->tail = 0;
    }

    if (w->head == APP_RSSI_WINDOW_SIZE) {
        w->head = 0;
    }

    if (w->size == 1) {
        w->value = value;
        w->value_min = value;
        w->value_max = value;
    } else {
        w->value = (int8_t)(w->value_sum * 1.0 / w->size);
        if (value > w->value_max) {
            w->value_max = value;
        } else if (value < w->value_min) {
            w->value_min = value;
        }
    }
}

POSSIBLY_UNUSED static void app_rssi_window_dump(rssi_window_t *w)
{
    char str[120];
    int shift = 0;
    int pos = 0;
    char *pstr = str;

    shift = sprintf(pstr, "window:");
    pstr += shift;

    if (w->size == 0) {
        shift = sprintf(pstr, "empty window...");
        pstr += shift;
    }

    for (int8_t i = 0; i < w->size; i++) {
        pos = w->head + i;
        if (w->head + i >= APP_RSSI_WINDOW_SIZE) {
            pos -= APP_RSSI_WINDOW_SIZE;
        }
        shift = sprintf(pstr, "%d ", w->buf[pos]);
        pstr += shift;
    }

    BTAPP_TRACE(0,"dump:%s", str);
}

static int8_t app_rssi_value_get(rssi_window_t *w)
{
    return w->value;
}

static int8_t app_rssi_min_value_get(rssi_window_t *w)
{
    return w->value_min;
}

static int8_t app_rssi_max_value_get(rssi_window_t *w)
{
    return w->value_max;
}

uint16_t app_rssi_get_mobile_acl_handle_by_addr(bt_bdaddr_t *addr)
{
    uint16_t dev_conhandle = bts_bt_if_get_dev_acl_handle(addr);
    uint16_t ibrt_conhandle = bts_ibrt_if_get_dev_ibrt_handle(addr);

    if (dev_conhandle != BT_INVALID_CONN_HANDLE && dev_conhandle != 0)
    {
        return dev_conhandle;
    } 
    else if (ibrt_conhandle != BT_INVALID_CONN_HANDLE && ibrt_conhandle != 0)
    {
        return ibrt_conhandle;
    }
    return BT_INVALID_CONN_HANDLE;
}

uint8_t app_rssi_get_mobile_device_id_by_addr(bt_bdaddr_t *addr)
{
    return btif_me_get_device_id_from_addr(addr);
}

remote_rssi_obj_t *app_rssi_find_and_new_remote_obj(bt_bdaddr_t *addr)
{
    int pos = -1;

    for (int i = 0; i < APP_RSSI_MAX_RECORD_NUM; i++) {
        remote_rssi_obj_t *cur_obj = &rssi_contrl.remote_rssi_obj[i];

        if (memcmp(&cur_obj->rssi.addr, addr, sizeof(bt_bdaddr_t)) == 0) {
            if (app_rssi_get_mobile_acl_handle_by_addr(&cur_obj->rssi.addr) == BT_INVALID_CONN_HANDLE) {
                memset(cur_obj, 0x00, sizeof(remote_rssi_obj_t));
                return NULL;
            }
            return cur_obj;
        }

        if (pos == -1) {
            if (app_rssi_get_mobile_acl_handle_by_addr(&cur_obj->rssi.addr) == BT_INVALID_CONN_HANDLE) {
                pos = i;
            }
        }
    }

    if (pos != -1) {
        remote_rssi_obj_t *obj = &rssi_contrl.remote_rssi_obj[pos];
        memset(obj, 0x00, sizeof(remote_rssi_obj_t));
        memcpy(&obj->rssi.addr, addr, sizeof(bt_bdaddr_t));
        app_rssi_window_init(&obj->window);
        return obj;
    }

    return NULL;
}

static void app_rssi_read_tws_rssi()
{
    rx_agc_t agc = {0};

    if (APP_RSSI_IS_TWS_CONNECT()) {
        bt_drv_reg_op_read_rssi_in_dbm(APP_RSSI_GET_TWS_HANDLE(), &agc);

        rssi_contrl.tws_rssi.rxgain = agc.rxgain;
        app_rssi_window_push(&rssi_contrl.tws_window, agc.rssi);
        rssi_contrl.tws_rssi.rssi = app_rssi_value_get(&rssi_contrl.tws_window);
        rssi_contrl.tws_rssi.rssi_min = app_rssi_min_value_get(&rssi_contrl.tws_window);
        rssi_contrl.tws_rssi.rssi_max = app_rssi_max_value_get(&rssi_contrl.tws_window);
    }
}

static void app_rssi_dump_tws_rssi()
{
    if (APP_RSSI_IS_TWS_CONNECT()) {
#ifdef APP_RSSI_DEBUG
        app_rssi_window_dump(&rssi_contrl.tws_window);
#endif
        BTAPP_TRACE(0,"dump:(TWS)gain=%d, rssi=%d, min=%d, max=%d, ser=%d", rssi_contrl.tws_rssi.rxgain, rssi_contrl.tws_rssi.rssi,
              rssi_contrl.tws_rssi.rssi_min, rssi_contrl.tws_rssi.rssi_max, rssi_contrl.tws_rssi.ser);
    }
}

static void app_rssi_read_mobile_rssi()
{
    static bt_bdaddr_t device_addr[BT_DEVICE_NUM];
    uint8_t count = bts_bt_if_get_dev_connected_list(&device_addr[0]);

    for (uint8_t i = 0; i < count; i++) {
        remote_rssi_obj_t *rssi_obj = app_rssi_find_and_new_remote_obj(&device_addr[i]);

        if (rssi_obj != NULL) {
            rx_agc_t agc = {0};
            bt_drv_reg_op_read_rssi_in_dbm(app_rssi_get_mobile_acl_handle_by_addr(&rssi_obj->rssi.addr), &agc);
            rssi_obj->rssi.local_rssi.rxgain = agc.rxgain;
            app_rssi_window_push(&rssi_obj->window, agc.rssi);
            rssi_obj->rssi.local_rssi.rssi = app_rssi_value_get(&rssi_obj->window);
            rssi_obj->rssi.local_rssi.rssi_min = app_rssi_min_value_get(&rssi_obj->window);
            rssi_obj->rssi.local_rssi.rssi_max = app_rssi_max_value_get(&rssi_obj->window);
        }
    }
}

static void app_rssi_dump_mobile_rssi()
{
    for (int i = 0; i < APP_RSSI_MAX_RECORD_NUM; i++) {
        remote_rssi_obj_t *cur_obj = &rssi_contrl.remote_rssi_obj[i];

        if (app_rssi_get_mobile_acl_handle_by_addr(&cur_obj->rssi.addr) != BT_INVALID_CONN_HANDLE) {
#ifdef APP_RSSI_DEBUG
            app_rssi_window_dump(&cur_obj->window);
#endif
            BTAPP_TRACE(0,"dump:(d%d)gain=%d, rssi=%d, min=%d, max=%d, ser=%d", app_rssi_get_mobile_device_id_by_addr(&cur_obj->rssi.addr),
                  cur_obj->rssi.local_rssi.rxgain, cur_obj->rssi.local_rssi.rssi, cur_obj->rssi.local_rssi.rssi_min,
                  cur_obj->rssi.local_rssi.rssi_max, cur_obj->rssi.local_rssi.ser);
            BTAPP_TRACE(0,"--->peer:gain=%d, rssi=%d, min=%d, max=%d, ser=%d", cur_obj->rssi.peer_rssi.rxgain, cur_obj->rssi.peer_rssi.rssi,
                  cur_obj->rssi.peer_rssi.rssi_min, cur_obj->rssi.peer_rssi.rssi_max, cur_obj->rssi.peer_rssi.ser);
        }
    }
}

void app_rssi_dump_data_pkt(rssi_pkt_t *pkt)
{
    BTAPP_TRACE(0,"app_rssi_dump_data_pkt:size=%d", sizeof(rssi_pkt_t));
    BTAPP_DUMP8("%02X ", (uint8_t *)pkt, sizeof(rssi_pkt_t));

    BTAPP_TRACE(0,"dump:side=%s", pkt->side ? "L" : "R");

    BTAPP_TRACE(0,"dump:(TWS)gain=%d, rssi=%d, min=%d, max=%d, ser=%d", pkt->tws_rssi.rxgain,
          pkt->tws_rssi.rssi, pkt->tws_rssi.rssi_min, pkt->tws_rssi.rssi_max, pkt->tws_rssi.ser);

    for (int i = 0; i < APP_RSSI_MAX_RECORD_NUM; i++) {
        remote_rssi_t *remote_rssi = &pkt->remote_rssi[i];

        if (app_rssi_get_mobile_acl_handle_by_addr(&remote_rssi->addr) != BT_INVALID_CONN_HANDLE) {
            BTAPP_TRACE(0,"dump:(d%d)gain=%d, rssi=%d, min=%d, max=%d, ser=%d", app_rssi_get_mobile_device_id_by_addr(&remote_rssi->addr),
                  remote_rssi->local_rssi.rxgain, remote_rssi->local_rssi.rssi, remote_rssi->local_rssi.rssi_min,
                  remote_rssi->local_rssi.rssi_max, remote_rssi->local_rssi.ser);
            BTAPP_TRACE(0,"--->peer:gain=%d, rssi=%d, min=%d, max=%d, ser=%d", remote_rssi->peer_rssi.rxgain, remote_rssi->peer_rssi.rssi,
                  remote_rssi->peer_rssi.rssi_min, remote_rssi->peer_rssi.rssi_max, remote_rssi->peer_rssi.ser);
        }
    }
}

void app_rssi_update_date_pkt(rssi_pkt_t *p_pkt)
{
    if (APP_RSSI_TRIGGER_MODE == APP_RSSI_EVENT_TRIGGER) {
        app_rssi_sample_task_start();
    }

    p_pkt->side = APP_RSSI_IS_TWS_LEFT_SIDE();

    p_pkt->tws_rssi = rssi_contrl.tws_rssi;

    int pos = 0;
    for (int i = 0; i < APP_RSSI_MAX_RECORD_NUM; i++) {
        remote_rssi_obj_t *cur_obj = &rssi_contrl.remote_rssi_obj[i];

        if (app_rssi_get_mobile_acl_handle_by_addr(&cur_obj->rssi.addr) != BT_INVALID_CONN_HANDLE) {
            p_pkt->remote_rssi[pos++] = cur_obj->rssi;
        }
    }
    p_pkt->remote_num = pos;

#ifdef APP_RSSI_DEBUG
    app_rssi_dump_data_pkt(p_pkt);
#endif
}

void app_rssi_sample_task_start()
{
    rssi_contrl.runtime = 0;
    if (APP_RSSI_TRIGGER_MODE == APP_RSSI_EVENT_TRIGGER) {
        if (!osTimerIsRunning(rssi_contrl.sample_timer)) {
            osTimerStart(rssi_contrl.sample_timer, APP_RSSI_BASE_SAMPLE_INTERVAL);
        }
    }
}

void app_rssi_sample_task_stop()
{
    if (APP_RSSI_TRIGGER_MODE == APP_RSSI_EVENT_TRIGGER) {
        osTimerStop(rssi_contrl.sample_timer);
    }
}

static void app_rssi_tws_sync_process()
{
    rssi_tws_pkt_t req_pkt;
    int pos = 0;

    if (!APP_RSSI_IS_TWS_CONNECT()) {
        return;
    }

    if (bts_core_get_ui_role() != TWS_UI_MASTER) {
        return;
    }

    for (int i = 0; i < APP_RSSI_MAX_RECORD_NUM; i++) {
        remote_rssi_obj_t *cur_obj = &rssi_contrl.remote_rssi_obj[i];
        if (app_rssi_get_mobile_acl_handle_by_addr(&cur_obj->rssi.addr) != BT_INVALID_CONN_HANDLE) {
            req_pkt.remote_rssi[pos++] = cur_obj->rssi;
        }
    }

    if (pos) {
        app_rssi_send_get_peer_rssi_req(&req_pkt);
    }
}

static void app_rssi_send_get_peer_rssi_req(rssi_tws_pkt_t *preq_pkt)
{
    bts_ibrt_if_send_peer_rssi_req((uint8_t *)preq_pkt, sizeof(rssi_tws_pkt_t));
}

void app_rssi_get_peer_rssi_handle(uint16_t rsp_seq, rssi_tws_pkt_t *prcv_pkt)
{
    rssi_tws_pkt_t rsp_pkt;
    int rsp_pos = 0;

    if (APP_RSSI_TRIGGER_MODE == APP_RSSI_EVENT_TRIGGER) {
        app_rssi_sample_task_start();
    }

    for (int i = 0; i < APP_RSSI_MAX_RECORD_NUM; i++) {
        remote_rssi_t *pprssi = &prcv_pkt->remote_rssi[i];
        if (app_rssi_get_mobile_acl_handle_by_addr(&pprssi->addr) != BT_INVALID_CONN_HANDLE) {
            for (int j = 0; j < APP_RSSI_MAX_RECORD_NUM; j++) {
                remote_rssi_obj_t *cur_obj = &rssi_contrl.remote_rssi_obj[j];

                if (memcmp(&pprssi->addr, &cur_obj->rssi.addr, sizeof(bt_bdaddr_t)) == 0) {
                    cur_obj->rssi.peer_rssi = pprssi->local_rssi;
                    rsp_pkt.remote_rssi[rsp_pos++] = cur_obj->rssi;
                }
            }
        }
    }

    if (rsp_pos != 0) {
        BTAPP_TRACE(0,"rcv_peer:update rssi");
#ifdef APP_RSSI_DEBUG
        app_rssi_dump_mobile_rssi();
#endif
        app_rssi_send_get_peer_rssi_rsp(rsp_seq, &rsp_pkt);
    }
}

static void app_rssi_send_get_peer_rssi_rsp(uint16_t rsp_seq, rssi_tws_pkt_t *prsp_pkt)
{
    bts_ibrt_if_send_peer_rssi_rsp(rsp_seq, (uint8_t *)prsp_pkt, sizeof(rssi_tws_pkt_t));
}

void app_rssi_get_peer_rssi_rsp_handle(rssi_tws_pkt_t *prcv_pkt)
{
    for (int i = 0; i < APP_RSSI_MAX_RECORD_NUM; i++) {
        remote_rssi_t *pprssi = &prcv_pkt->remote_rssi[i];
        if (app_rssi_get_mobile_acl_handle_by_addr(&pprssi->addr) != BT_INVALID_CONN_HANDLE) {
            for (int j = 0; j < APP_RSSI_MAX_RECORD_NUM; j++) {
                remote_rssi_obj_t *cur_obj = &rssi_contrl.remote_rssi_obj[j];

                if (memcmp(&pprssi->addr, &cur_obj->rssi.addr, sizeof(bt_bdaddr_t)) == 0) {
                    cur_obj->rssi.peer_rssi = pprssi->local_rssi;
                }
            }
        }
    }
    BTAPP_TRACE(0,"rcv_rsp:update rssi");
    app_rssi_dump_tws_rssi();
    app_rssi_dump_mobile_rssi();
}

void app_rssi_get_peer_rssi_handle(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    BTAPP_TRACE(0,"tws_rcv_hdl:len=%d", length);

    app_rssi_get_peer_rssi_handle(rsp_seq, (rssi_tws_pkt_t *)p_buff);
}

void app_rssi_get_peer_rssi_rsp_handle(uint8_t *p_buff, uint16_t length)
{
    BTAPP_TRACE(0,"tws_rsp_hdl:len=%d", length);

    app_rssi_get_peer_rssi_rsp_handle((rssi_tws_pkt_t *)p_buff);
}

static const app_ibrt_get_peer_rssi_cb tws_rssi_cmd_cbs = {
    .send_get_peer_rssi_hanlder_cb = app_rssi_get_peer_rssi_handle,
    .send_get_peer_rssi_rsp_handler_cb = app_rssi_get_peer_rssi_rsp_handle,
};

static void app_rssi_tws_cmd_func_reg()
{
    bts_tws_get_peer_rssi_cb(&tws_rssi_cmd_cbs);
}

static void app_rssi_vender_ll_monitor_callback(uint16_t handle, uint8_t ser)
{
    if (APP_RSSI_IS_TWS_CONNECT()) {
        if (APP_RSSI_GET_TWS_HANDLE() == handle) {
            rssi_contrl.tws_rssi.ser = ser;
            return;
        }
    }

    for (int i = 0; i < APP_RSSI_MAX_RECORD_NUM; i++) {
        remote_rssi_obj_t *cur_obj = &rssi_contrl.remote_rssi_obj[i];
        if (app_rssi_get_mobile_acl_handle_by_addr(&cur_obj->rssi.addr) == handle) {
            cur_obj->rssi.local_rssi.ser = ser;
        }
    }
}

static void rssi_mnt_sample_timer_callback(void const *param);
osTimerDef(RSSI_SAMPLE_TIMER, rssi_mnt_sample_timer_callback);

static void rssi_mnt_sample_timer_callback(void const *param)
{
    static int cnt = 0;
    static int tws_cnt = 0;

    if ((tws_cnt++) * APP_RSSI_BASE_SAMPLE_INTERVAL >= APP_RSSI_TWS_SAMPLE_INTERVAL) {
        tws_cnt = 0;
        app_rssi_read_tws_rssi();
    }

    app_rssi_read_mobile_rssi();

    if ((cnt++) * APP_RSSI_BASE_SAMPLE_INTERVAL >= APP_RSSI_GET_PEER_RSSI_INTERVAL) {
        cnt = 0;
#ifdef APP_RSSI_DEBUG
        app_rssi_dump_tws_rssi();
        app_rssi_dump_mobile_rssi();
#endif
        app_rssi_tws_sync_process();
    }

    rssi_contrl.runtime += APP_RSSI_BASE_SAMPLE_INTERVAL;

    if (APP_RSSI_TRIGGER_MODE == APP_RSSI_EVENT_TRIGGER) {
        if (rssi_contrl.runtime >= APP_RSSI_SAMPLE_TIMEOUT) {
            app_rssi_sample_task_stop();
        }
    }
}

void app_rssi_init()
{
    BTAPP_TRACE(0,"init...");

    rssi_contrl.sample_timer = osTimerCreate(osTimer(RSSI_SAMPLE_TIMER), osTimerPeriodic, NULL);

    app_rssi_tws_cmd_func_reg();

    bts_bt_if_register_vender_ll_monitor_handle(app_rssi_vender_ll_monitor_callback);

    if (APP_RSSI_TRIGGER_MODE == APP_RSSI_ALWAYS_TRIGGER) {
        osTimerStart(rssi_contrl.sample_timer, APP_RSSI_BASE_SAMPLE_INTERVAL);
    }
}
