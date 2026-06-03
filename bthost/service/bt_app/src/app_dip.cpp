/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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
#ifdef BT_DIP_SUPPORT
#undef MOUDLE
#define MOUDLE APP_BT
#include <stdio.h>
#include "cmsis_os.h"
#include "hal_trace.h"
#include "dip_api.h"
#include "plat_types.h"
#include "app_bt.h"
#include "app_dip.h"
#include "app_bt_func.h"
#include "btapp.h"
#include "sdp_api.h"
#include "me_api.h"
#include "spp_api.h"
#include "besaud_api.h"
#include "bluetooth.h"

#include "bluetooth_nv_mgr.h"

#if defined(IBRT)
#include "app_tws_ibrt_cmd_handler.h"
#include "app_ibrt_internal.h"
#include "earbud_ux_api.h"
#include "bts_bt_conn.h"
#include "bts_bt_if.h"
#include "bts_ibrt_conn.h"
#include "bts_ibrt_if.h"
#include "bts_core_if.h"
#include "bts_tws_if.h"
#endif

#if defined(__AI_VOICE__) || defined(BISTO_ENABLED)
#include "app_ai_if.h"
#endif

#if defined(__AI_VOICE__)
#include "ai_thread.h"
#endif

#ifdef WATCH_AI_ENABLED
#include "app_ai_control.h"
#endif

void app_dip_sync_dip_info(void);

static app_dip_info_queried_callback_func_t dip_info_queried_callback = NULL;

static void app_dip_callback(bt_bdaddr_t *_addr, struct dip_callback_param cb_para)
{
    enum dip_event event = cb_para.event;

    DEBUG_INFO(2,"%s event %d addr :", __func__, event);
    DUMP8("%x ", _addr->address, BT_ADDR_OUTPUT_PRINT_NUM);

    if (event == DIP_EVENT_SDP_REQ_FAIL)
    {
        DEBUG_INFO(2,"%s DIP req fail", __func__);
        bt_dip_queried_state_t fail = {0};
        fail.error_code = cb_para.p.fail.reason;
        btif_report_bt_event(_addr, BT_EVENT_DIP_QUIRED_FAIL, &fail);
        return;
    }

    bt_dip_queried_state_t res = {0};
    btif_report_bt_event(_addr, BT_EVENT_DIP_QUIRED_SUCCESS, &res);

#if defined(IBRT)

    if (TWS_LINK == app_tws_ibrt_get_link_type_by_addr(_addr))
    {
        DEBUG_INFO(1,"%s connect type is TWS", __func__);
        return;
    }

    app_dip_sync_dip_info();
#endif

#if defined(__AI_VOICE__) || defined(BISTO_ENABLED)
    app_ai_if_mobile_connect_handle((void*)_addr);
#endif

#if defined(WATCH_AI_ENABLED)
    app_watch_ai_mobile_connect_handle((void*)_addr);
#endif

    if (dip_info_queried_callback)
    {
        bt_dip_pnp_info_t* pnp_info = btif_dip_get_device_info(_addr);
        dip_info_queried_callback(_addr->address, (bt_dip_pnp_info_t *)pnp_info);
    }
}

#if defined(IBRT)
void app_dip_sync_info_prepare_handler(uint8_t *buf,
    uint16_t *totalLen, uint16_t *len, uint16_t expectLen)
{
    uint32_t offset = 0;
    bt_dip_pnp_info_t pnp_info;
    bt_bdaddr_t *mobile_addr = NULL;

    ibrt_mobile_info_t *mInfo = bts_bt_sink_conn_get_mobile_info_ext();
    mobile_addr = &(mInfo->mobile_addr);

    bt_bdaddr_t mobile_addr_list[BT_DEVICE_NUM];
    uint8_t connected_mobile_num = bts_bt_conn_get_dev_connected_list(mobile_addr_list);

    for(uint8_t i=0; i<connected_mobile_num; i++)
    {
        mobile_addr = &mobile_addr_list[i];
        nv_record_get_pnp_info(mobile_addr, &pnp_info);
        memcpy(buf+offset, mobile_addr->address, 6);
        offset += 6;
        memcpy(buf+offset, (uint8_t *)&pnp_info, sizeof(pnp_info));
        offset += sizeof(pnp_info);
    }
    *totalLen = *len = offset;
}

void app_dip_sync_info_received_handler(uint8_t *buf, uint16_t length, bool isContinueInfo)
{
    bt_bdaddr_t mobile_addr;
    bt_dip_pnp_info_t pnp_info;
    pnp_info.vend_id = 0;
    uint32_t offset = 0;
    nvrec_btdevicerecord *record = NULL;

    while((length - offset) >= 10)
    {
        memcpy(mobile_addr.address, buf+offset, 6);
        offset += 6;
        memcpy((uint8_t *)&pnp_info, buf+offset, sizeof(pnp_info));
        offset += sizeof(pnp_info);
        DEBUG_INFO(2, "%s vend_id 0x%x vend_id_source 0x%x addr:", __func__,
            pnp_info.vend_id, pnp_info.vend_id_source);
        DUMP8("0x%x ", mobile_addr.address, BT_ADDR_OUTPUT_PRINT_NUM);

        if (pnp_info.vend_id)
        {
            if (0 != nv_record_btdevicerecord_find(&mobile_addr, &record))
            {
                //update prams to record
                nvrec_btdevicerecord btdevicerecord;
                memset(&btdevicerecord, 0, sizeof(nvrec_btdevicerecord));
                memcpy(btdevicerecord.record.bdAddr.address, mobile_addr.address, BT_ADDR_OUTPUT_PRINT_NUM);
                //add new record
                bluetooth_nv_mgr_bt_record_add(BT_NV_REC_ADD_DIP_SYNC_INFO_RECV, (void *)&btdevicerecord);

                if (!nv_record_btdevicerecord_find(&mobile_addr, &record))
                {
                    nv_record_btdevicerecord_set_pnp_info(record, &pnp_info);
                }
            }
            else
            {
                nv_record_btdevicerecord_set_pnp_info(record, &pnp_info);
            }
        }
    }
}

void app_dip_sync_init(void)
{
    TWS_SYNC_USER_T user_app_dip_t = {
        app_dip_sync_info_prepare_handler,
        app_dip_sync_info_received_handler,
        NULL,
        NULL,
        NULL,
    };

    bts_tws_if_register_tws_sync_user(TWS_SYNC_USER_DIP, &user_app_dip_t);
}

void app_dip_sync_dip_info(void)
{
    bts_tws_if_prepare_user_sync_tws_info();
    bts_tws_if_user_sync_tws_info(TWS_SYNC_USER_DIP);
    bts_tws_if_flush_user_sync_tws_info();
}
#endif

static void app_dip_info_queried_default_callback(uint8_t *remote_addr, bt_dip_pnp_info_t *pnp_info)
{
    if (pnp_info)
    {
        DEBUG_INFO(4, "get dip info vendor_id 0x%04x product_id 0x%04x product_version 0x%04x",
                pnp_info->vend_id, pnp_info->prod_id, pnp_info->prod_ver);
    }
    else
    {
        DEBUG_INFO(1, "%s N/A", __func__);
    }
}

void app_dip_register_dip_info_queried_callback(app_dip_info_queried_callback_func_t func)
{
    dip_info_queried_callback = func;
}

void app_dip_init(void)
{
    btif_dip_init(app_dip_callback);

    app_dip_register_dip_info_queried_callback(app_dip_info_queried_default_callback);
}

void app_dip_query_remote_info(uint8_t device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);

    if (curr_device)
    {
#ifdef IBRT
        if (bes_bt_tws_besaud_is_connected() &&
            bts_bt_sink_conn_get_ibrt_role(&curr_device->remote) == IBRT_SLAVE)
        {
            return;
        }
#endif
        struct bt_defer_param_t param_a = bt_alloc_param_size(&curr_device->remote, 6);
        bt_defer_call_func_1((uint32_t)btif_dip_query_remote_info, param_a);
    }
}

#endif

