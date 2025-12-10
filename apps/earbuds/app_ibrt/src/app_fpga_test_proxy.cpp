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
#if defined(FPGA)

#include <string.h>
#include "hal_trace.h"
#include "bluetooth_bt_api.h"
#include "app_bt.h"
#include "app_bt_func.h"
#include "app_fpga_test_proxy.h"
#include "bts_tws_if.h"
#include "bts_bt_if.h"
#include "bts_ibrt_tws_switch.h"
#if defined(IBRT_UI)
#include "app_ibrt_internal.h"
#include "app_ui_queues.h"
#endif

/* Mobile address list */
static const bt_bdaddr_t fpga_mobile_addr_list[] = {
    {{0x6C, 0x04, 0x5E, 0x6C, 0x66, 0x5C}}, // Index 0 (Link 1)
    {{0x3E, 0x44, 0x82, 0x73, 0x4D, 0x6C}}, // Index 1 (Link 2)
    {{0x26, 0x88, 0x2D, 0x43, 0xE1, 0xBC}}, // Index 2 (Link 3)
};

/* Command handler type */
typedef void (*fpga_cmd_handler)(uint32_t param);

/* Command handler entry */
typedef struct {
    fpga_test_cmd_type_t cmd_type;
    fpga_cmd_handler      handler;
    bool                 has_param;
} fpga_cmd_entry_t;

/******************* Command Handlers *******************/
static void fpga_tws_master_pairing(uint32_t param)
{
    bts_tws_fpga_start_tws_pairing(0);
}

static void fpga_tws_slave_pairing(uint32_t param)
{
    bts_tws_fpga_start_tws_pairing(1);
}

static void fpga_set_access_mode(uint32_t param) {
    app_ibrt_conn_set_discoverable_connectable(true, true);
}

static void fpga_tws_connect(uint32_t param)
{
    bts_tws_if_connect_acl_link(0);
}

static void fpga_tws_disconnect(uint32_t param)
{
    bts_tws_if_disconnect_acl_link();
}

static void fpga_w4_mobile_connect(uint32_t param)
{
    if (bts_tws_if_is_nv_master())
    {
        app_bt_set_access_mode(BTIF_BAM_GENERAL_ACCESSIBLE);
    }
    else
    {
        app_bt_set_access_mode(BTIF_BAM_NOT_ACCESSIBLE);
    }
}

static void fpga_mobile_connect(uint32_t param)
{
    const bt_bdaddr_t* addr = fpga_get_mobile_addr(param);
    if (addr)
    {
        app_ibrt_if_connect_mobile_device(addr, 0);
    }
}

static void fpga_mobile_disconnect(uint32_t param)
{
    const bt_bdaddr_t* addr = fpga_get_mobile_addr(param);
    if (addr)
    {
        bts_bt_if_dev_disconnect_acl_link(addr);
    }
}

static void fpga_start_ibrt_link(uint32_t param)
{
    const bt_bdaddr_t* addr = fpga_get_mobile_addr(param);
    if (addr)
    {
        bts_ibrt_if_dev_connect_ibrt_link(addr);
    }
}

static void fpga_stop_ibrt_link(uint32_t param)
{
    const bt_bdaddr_t* addr = fpga_get_mobile_addr(param);
    if (addr)
    {
        bts_ibrt_if_dev_disconnect_ibrt_link(addr);
    }
}

static void fpga_role_switch(uint32_t param)
{
    const bt_bdaddr_t* addr = fpga_get_mobile_addr(param);
    if (!addr)
        return;

    ibrt_status_t status = IBRT_STATUS_SUCCESS;
    if(!bts_bt_conn_is_profile_connecting(addr))
    {
        status = IBRT_STATUS_ERROR_OP_NOT_ALLOWED;
    }
    else
    {
        status = bts_ibrt_conn_tws_role_switch(addr);
    }
    EARBUDS_TRACE(0,"%s status=%d", __func__, status);
}

static void fpga_dump_conn_info(uint32_t param)
{
    app_ibrt_conn_dump_ibrt_info();
}

#if defined(IBRT_UI)
static void fpga_dump_mgr_info(uint32_t param)
{
    app_ibrt_if_dump_ui_status();
}

static void fpga_mgr_unit_test(uint32_t param)
{
    // app_ui_queue_test();
}
#endif

/******************* Command Table *******************/
static const fpga_cmd_entry_t cmd_table[] = {
    {TWS_MASTER_PAIRING,     fpga_tws_master_pairing,  false},
    {TWS_SLAVE_PAIRING,      fpga_tws_slave_pairing,   false},
    {SET_ACCESS_MODE,        fpga_set_access_mode,     false},
    {TWS_CONNECT,            fpga_tws_connect,         false},
    {TWS_DISCONNECT,         fpga_tws_disconnect,      false},
    {W4_MOBILE_CONNECT,      fpga_w4_mobile_connect,   false},
    {MOBILE_LINK_CONNECT,    fpga_mobile_connect,      true},
    {MOBILE_LINK_DISCONNECT, fpga_mobile_disconnect,   true},
    {START_LINK_IBRT,        fpga_start_ibrt_link,     true},
    {STOP_LINK_IBRT,         fpga_stop_ibrt_link,      true},
    {MOBILE_LINK_RW,         fpga_role_switch,         true},
    {DUMP_IBRT_CONN_INFO,    fpga_dump_conn_info,      false},
#if defined(IBRT_UI)
    {DUMP_IBRT_MGR_INFO,     fpga_dump_mgr_info,       false},
    {IBRT_MGR_UNIT_TEST,     fpga_mgr_unit_test,       false},
#endif
};

/******************* Utility Functions *******************/
static const bt_bdaddr_t* fpga_get_mobile_addr(uint32_t link_idx)
{
    /* Convert 1-based index to 0-based */
    uint32_t index = link_idx - 1;
    if (index >= ARRAY_SIZE(fpga_mobile_addr_list))
    {
        EARBUDS_TRACE(0,"Invalid link index: %lu", link_idx);
        return NULL;
    }

    return &fpga_mobile_addr_list[index];
}

void app_fpga_proxy_cmd_request(fpga_test_cmd_type_t cmd_type, uint32_t param)
{
    EARBUDS_TRACE(0,"FPGA Proxy Cmd: 0x%02X, Param: %lu", cmd_type, param);

    for (size_t i = 0; i < ARRAY_SIZE(cmd_table); i++)
    {
        if (cmd_table[i].cmd_type == cmd_type)
        {
            if (cmd_table[i].has_param && !fpga_get_mobile_addr(param))
            {
                EARBUDS_TRACE(0,"Invalid param for cmd 0x%02X: %lu", cmd_type, param);
                return;
            }
            cmd_table[i].handler(param);
            return;
        }
    }

    EARBUDS_TRACE(0,"Unknown command type: 0x%02X", cmd_type);
}

void app_fpga_test_proxy_entry(fpga_test_cmd_type_t cmd_type, uint32_t param)
{
    app_bt_start_custom_function_in_bt_thread((uint32_t)cmd_type, param, (uint32_t)app_fpga_proxy_cmd_request);
}

#endif
