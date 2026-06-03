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

/*--------------------------------------------------------------------
                           GENERAL INCLUDES
--------------------------------------------------------------------*/
#include "plat_types.h"
#include <stdarg.h>
#include "co_math.h"
#include "hal_trace.h"
#include "hal_cmu.h"
#include "hal_timer.h"
#include "bt_drv_interface.h"
#include "app_bt_sync.h"
#ifdef IBRT
#include "app_tws_ibrt_cmd_handler.h"
#include "app_ibrt_internal.h"
#include "earbud_ux_api.h"
#include "bts_module_if.h"
#include "bts_tws_conn.h"
#endif
#ifdef USE_BASIC_THREADS
#include "app_thread.h"
#endif

#if defined(__BT_SYNC__) && defined(IBRT)
/*--------------------------------------------------------------------
                               MACROS
--------------------------------------------------------------------*/
// #define BT_SYNC_TEST_MODULE
// #define BT_SYNC_DEBUG_LEVEL

#define BT_SYNC_DEFAULT_POLL_US         (IBRT_UI_DEFAULT_POLL_INTERVAL * 4 * 625)
#define BT_SYNC_LONG_POLL_US            (IBRT_UI_LONG_POLL_INTERVAL * 8 * 625)
/*--------------------------------------------------------------------
                         INTERNAL VARIABLES
--------------------------------------------------------------------*/
static bool bt_sync_initilized = false;
static BT_SYNC_ENV_T bt_sync_env;

#ifndef USE_BASIC_THREADS
static osThreadId bt_sync_thread_tid = NULL;
static void app_bt_sync_thread(void const *argument);
osThreadDef(app_bt_sync_thread, osPriorityRealtime, 1, APP_BT_SYNC_THREAD_STACK_SIZE, "BT_SYNC");
osMailQDef(bt_sync_mailbox, 5, BT_SYNC_MSG_BLOCK_T);
static osMailQId bt_sync_mailbox = NULL;
#endif // endif USE_BASIC_THREADS

static APP_BT_SYNC_INFO_REPORT_T bt_sync_share_info_report_func = NULL;

static void app_bt_sync_supervision_timeout_handler(void const *param);
osTimerDef(BT_SYNC_SUPERVISION_TIMER, app_bt_sync_supervision_timeout_handler);
static osTimerId bt_sync_supervision_timer_id = NULL;
osMutexDef(bt_sync_mutex);
static osMutexId bt_sync_mutex_id = NULL;

/*--------------------------------------------------------------------
                             INTERNAL PROCEDURES
--------------------------------------------------------------------*/
static BT_SYNC_JOB_LIST_T *app_bt_sync_seek_job_handler(uint32_t opCode)
{
    uint32_t total_job_cnt = ((uint32_t)__app_bt_sync_command_handler_table_end-
        (uint32_t)__app_bt_sync_command_handler_table_start) /
        sizeof(BT_SYNC_JOB_LIST_T);

    for (uint32_t i = 0; i < total_job_cnt; i++)
    {
        BT_SYNC_JOB_LIST_T *p_job = APP_BT_SYNC_COMMAND_PTR_FROM_ENTRY_INDEX(i);
        if (p_job->opCode == opCode)
        {
            return p_job;
        }
    }

    return NULL;
}

static BT_SYNC_INSTANCE_T *app_bt_sync_seek_work_instance(uint32_t opCode)
{
    osMutexWait(bt_sync_mutex_id, osWaitForever);

    BT_SYNC_INSTANCE_T *p_work = bt_sync_env.workInstance;

    for (uint32_t i = 0; i < APP_BT_SYNC_CHANNEL_MAX; i++) {
        if ((p_work->syncType != BT_SYNC_TYPE_NONE) && (opCode == p_work->triInfo.opCode)) {
            osMutexRelease(bt_sync_mutex_id);
            return p_work;
        }
        p_work++;
    }
    osMutexRelease(bt_sync_mutex_id);

    return NULL;
}

// supervision list
#ifdef BT_SYNC_DEBUG_LEVEL
static void app_bt_sync_dump_supervison(void)
{
    BT_SYNC_INSTANCE_T *p_work= bt_sync_env.workInstance;

    for (uint32_t i = 0; i < APP_BT_SYNC_CHANNEL_MAX; i++) {
        BT_SYNC_TRACE(1, "(d%d)bt_sync_dump, opCode:0x%x, chl:%d, msRemain:%u type:%d",
            i, p_work->triInfo.opCode, p_work->triInfo.triChl, p_work->msTillTimeout, p_work->syncType);
        p_work++;
    }
}
#endif

static void app_bt_sync_refresh_supervison_list(bool is_add, uint32_t opCode, uint8_t index)
{
    ASSERT(index < APP_BT_SYNC_CHANNEL_MAX, "(d%d)bt_sync_supv, param wrong opCode:%d", index, opCode);

    osMutexWait(bt_sync_mutex_id, osWaitForever);

    // Update the supervisor mask and timeout for the given index
    if (is_add) {
        bt_sync_env.workInstance[index].msTillTimeout = APP_BT_SYNC_SUPERVISON_TIMEOUT;
        bt_sync_env.supervisorMask |= (1 << index);
    } else {
        bt_sync_env.supervisorMask &= ~(1 << index);
    }

    BT_SYNC_TRACE(1, "(d%d)bt_sync_supv, is_add:%d bf:0x%x opCode:%d",
        index, is_add, bt_sync_env.supervisorMask, opCode);

#ifdef BT_SYNC_DEBUG_LEVEL
    app_bt_sync_dump_supervison();
#endif

    uint32_t nearest_idx = APP_BT_SYNC_CHANNEL_MAX;
    uint32_t nearest_time = APP_BT_SYNC_MAX_U32_VALUE;
    uint32_t totalCnt = CO_BIT_CNT(bt_sync_env.supervisorMask);

    if (totalCnt == 0) {
        // do nothing if no supervisor in the list
        bt_sync_env.nearest_idx = APP_BT_SYNC_CHANNEL_MAX;
        osTimerStop(bt_sync_supervision_timer_id);
        osMutexRelease(bt_sync_mutex_id);
        return;
    }

    uint32_t currentTicks = GET_CURRENT_TICKS();
    uint32_t passedTicks = 0;
    uint32_t passedMs = 0;

    // If it is the first adder, there is no need to calculate the time passed;
    // other cases require calculating the time used.
    if (!is_add || (totalCnt > 1))
    {
        if (currentTicks >= bt_sync_env.lastSysTicks) {
            passedTicks = (currentTicks - bt_sync_env.lastSysTicks);
        } else {
            // wrap
            passedTicks = (hal_sys_timer_get_max() - bt_sync_env.lastSysTicks + 1) + currentTicks;
        }
        passedMs = TICKS_TO_MS(passedTicks);
    }

    BT_SYNC_INSTANCE_T *p_work = bt_sync_env.workInstance;
    for (uint32_t idx = 0; idx < APP_BT_SYNC_CHANNEL_MAX; idx++, p_work++)
    {
        BT_SYNC_TRACE(2, "(d%d)bt_sync_supv:%u-%u, idx:%d type:%d", idx,
             passedMs, p_work->msTillTimeout, nearest_idx, p_work->syncType);

        if (p_work->syncType != BT_SYNC_TYPE_COMMON) {
            continue;
        }

        if (passedMs + 10 >= p_work->msTillTimeout)
        {
            uint32_t pre_idx = bt_sync_env.nearest_idx;
            bt_sync_env.nearest_idx = idx;
            app_bt_sync_supervision_timeout_handler(NULL);
            bt_sync_env.nearest_idx = pre_idx;
            continue;
        }

        // New monitored should not deduct the time passed
        if (!is_add || (idx != index)) {
            p_work->msTillTimeout -= passedMs;
        }

        if (nearest_time > p_work->msTillTimeout) {
            nearest_idx = idx;
            nearest_time = p_work->msTillTimeout;
        }
    }

    BT_SYNC_TRACE(2, "(d%d)bt_sync_supv:%u", nearest_idx, nearest_time);
    if (nearest_idx < APP_BT_SYNC_CHANNEL_MAX) {
        osTimerStart(bt_sync_supervision_timer_id, nearest_time);
        bt_sync_env.nearest_idx = nearest_idx;
        bt_sync_env.lastSysTicks = GET_CURRENT_TICKS();
    }

    osMutexRelease(bt_sync_mutex_id);
}

#ifdef USE_BASIC_THREADS
static int app_bt_sync_handle(APP_MESSAGE_BODY *msg_body)
{
    // Validate input parameter
    if (!msg_body) {
        BT_SYNC_TRACE(1, "bt_sync_handle MSG NULL");
        return -1;
    }
    uint32_t index = msg_body->message_Param1;
    bool triResult = !!msg_body->message_Param0;
    BT_SYNC_INSTANCE_T *p_work = &bt_sync_env.workInstance[index];
    if ((index >= APP_BT_SYNC_CHANNEL_MAX) || !p_work) {
        BT_SYNC_TRACE(1, "(d%d)bt_sync_handle invalid work", index);
        return -2;
    }
    // Skip if already processed
    if (p_work->syncType == BT_SYNC_TYPE_NONE) {
        return -3;
    }

    uint32_t opCode = p_work->triInfo.opCode;
    BT_SYNC_JOB_LIST_T *p_job_handler = app_bt_sync_seek_job_handler(opCode);
    bool isAbandoned = p_work->isAbandoned;

    BT_SYNC_TRACE(4, "(d%d)bt_sync_handle, job:%p-%p Code:0x%x, result:%d",
                index, p_work, p_job_handler, opCode, triResult);

    // Release resources for ALL cases (including abandoned)
    app_bt_sync_release_trigger_channel(p_work->triInfo.triChl);
    app_bt_sync_refresh_supervison_list(false, opCode, index);

    // Skip notification if abandoned or opcode invalid or job handler is NULL
    if (isAbandoned || (opCode >= APP_BT_SYNC_OP_MAX) || !p_job_handler) {
        return -4;
    }

    // Status notification
    if (p_job_handler && p_job_handler->statusNotify) {
        const bool bShareStatus = !!p_work->twsShareDone;
        p_job_handler->statusNotify(opCode, triResult, bShareStatus);
    }

    // Callback execution
    if (triResult && p_job_handler && p_job_handler->syncCallback) {
        p_job_handler->syncCallback();
    }

    return 0;
}

static void app_bt_sync_supervision_timeout_handler(void const *param)
{
    osMutexWait(bt_sync_mutex_id, osWaitForever);
    POSSIBLY_UNUSED uint32_t opCode = bt_sync_env.workInstance[bt_sync_env.nearest_idx].triInfo.opCode;
    uint32_t trigger_chl = bt_sync_env.nearest_idx + APP_BT_SYNC_CHANNEL_OFFSET;

    BT_SYNC_TRACE(2, "(d%d)bt_sync_timeout, opCode:%d", bt_sync_env.nearest_idx, opCode);
    if (bt_sync_env.nearest_idx >= APP_BT_SYNC_CHANNEL_MAX)
    {
        osMutexRelease(bt_sync_mutex_id);
        return;
    }
    osMutexRelease(bt_sync_mutex_id);

    hal_cmu_bt_trigger_set_handler((enum HAL_CMU_BT_TRIGGER_SRC_T)trigger_chl, NULL);
    APP_MESSAGE_BLOCK msg;
    msg.mod_id = APP_MODULE_BTSYNC;
    msg.mod_level = APP_MOD_LEVEL_0;
    msg.msg_body.message_Param0 = APP_BT_SYNC_RESULT_FAILED;
    msg.msg_body.message_Param1 = bt_sync_env.nearest_idx;
    app_mailbox_put(&msg);
}

static void app_bt_sync_irq_handler(enum HAL_CMU_BT_TRIGGER_SRC_T src)
{
    uint32_t index = src - APP_BT_SYNC_CHANNEL_OFFSET;
    // sanity check
    if (index > APP_BT_SYNC_CHANNEL_MAX) {
        BT_SYNC_TRACE(1, "(d%d)bt_sync_irq_handler, error", index);
        return;
    }
    POSSIBLY_UNUSED BT_SYNC_INSTANCE_T *p_work = &(bt_sync_env.workInstance[index]);

    BT_SYNC_TRACE(3, "(d%d)bt_sync_irq_handler, src:%d opCode:0x%x",
        index, src, p_work->triInfo.opCode);

    btdrv_syn_clr_trigger((uint8_t)src);
    hal_cmu_bt_trigger_disable(src);

    APP_MESSAGE_BLOCK msg;
    msg.mod_id = APP_MODULE_BTSYNC;
    msg.mod_level = APP_MOD_LEVEL_0;
    msg.msg_body.message_Param0 = APP_BT_SYNC_RESULT_PASS;
    msg.msg_body.message_Param1 = index;
    app_mailbox_put(&msg);
}

static int app_bt_sync_module_init(void)
{
    int32_t lock = int_lock();
    if (bt_sync_initilized) {
        int_unlock(lock);
        return 0;
    }
    bt_sync_initilized = true;
    int_unlock(lock);

    app_set_threadhandle(APP_MODULE_BTSYNC, app_bt_sync_handle);

    if (!bt_sync_supervision_timer_id)
    {
        bt_sync_supervision_timer_id = osTimerCreate(osTimer(BT_SYNC_SUPERVISION_TIMER), osTimerOnce, NULL);
    }

    if (!bt_sync_mutex_id)
    {
        bt_sync_mutex_id = osMutexCreate(osMutex(bt_sync_mutex));
    }

    if ((!bt_sync_supervision_timer_id) || (!bt_sync_mutex_id))
    {
        return -1;
    }

    memset((uint8_t *)&bt_sync_env, 0x00, sizeof(bt_sync_env));

    return 0;
}

#else // USE_BASIC_THREADS
static int app_bt_sync_mailbox_free(BT_SYNC_MSG_BLOCK_T *msg_p)
{
    osStatus status;

    status = osMailFree(bt_sync_mailbox, msg_p);
    if (osOK != status) {
        BT_SYNC_TRACE(1, "%s failed to free", __func__);
    }
    return (int)status;
}

static int app_bt_sync_mailbox_get(BT_SYNC_MSG_BLOCK_T **msg_p)
{
    osEvent evt;
    evt = osMailGet(bt_sync_mailbox, osWaitForever);
    if (evt.status == osEventMail) {
        *msg_p = (BT_SYNC_MSG_BLOCK_T *)evt.value.p;
        return 0;
    }
    return -1;
}

static int app_bt_sync_mailbox_put(BT_SYNC_MSG_BLOCK_T *msg_src)
{
    osStatus status;
    BT_SYNC_MSG_BLOCK_T *msg_p = NULL;

    msg_p = (BT_SYNC_MSG_BLOCK_T *)osMailAlloc(bt_sync_mailbox, 0);

    if (!msg_p) {
        BT_SYNC_TRACE(1, "%s failed to alloc", __func__);
        return -1;
    }

    *msg_p = *msg_src;
    status = osMailPut(bt_sync_mailbox, msg_p);
    if (osOK != status) {
        BT_SYNC_TRACE(1, "%s failed(%d) to put", __func__, status);
    }

    return (int)status;
}

static void app_bt_sync_thread(void const *argument)
{
    while (1)
    {
        BT_SYNC_MSG_BLOCK_T *msg_p = NULL;

        if (app_bt_sync_mailbox_get(&msg_p)) {
            continue;
        }

        uint32_t index = msg_p->workIdx;
        bool triResult = !!msg_p->triResult;
        app_bt_sync_mailbox_free(msg_p);

        BT_SYNC_INSTANCE_T *p_work = &bt_sync_env.workInstance[index];
        if ((index >= APP_BT_SYNC_CHANNEL_MAX) || !p_work) {
            BT_SYNC_TRACE(1, "(d%d)bt_sync_thread invalid work", index);
            continue;
        }

        // Skip if already processed
        if (p_work->syncType == BT_SYNC_TYPE_NONE) {
            continue;
        }

        uint32_t opCode = p_work->triInfo.opCode;
        BT_SYNC_JOB_LIST_T *p_job_handler = app_bt_sync_seek_job_handler(opCode);
        bool isAbandoned = p_work->isAbandoned;

        BT_SYNC_TRACE(4, "(d%d)bt_sync_thread, job:%p-%p code:0x%x type:%d status:%d",
            index, p_work, p_job_handler, opCode, p_work->syncType, triResult);

        // Release resources for ALL cases (including abandoned)
        app_bt_sync_release_trigger_channel(p_work->triInfo.triChl);
        app_bt_sync_refresh_supervison_list(false, opCode, index);

        // Skip notification if abandoned or opcode invalid or job handler is NULL
        if (isAbandoned || (opCode >= APP_BT_SYNC_OP_MAX) || !p_job_handler) {
            continue;
        }

        // Notify status if handler exists
        if (p_job_handler->statusNotify) {
            const bool bShareStatus = !!p_work->twsShareDone;
            p_job_handler->statusNotify(opCode, triResult, bShareStatus);
        }

        // Execute sync callback if conditions met
        if (triResult && p_job_handler->syncCallback) {
            p_job_handler->syncCallback();
        }
    }
}

static void app_bt_sync_supervision_timeout_handler(void const *param)
{
    osMutexWait(bt_sync_mutex_id, osWaitForever);
    POSSIBLY_UNUSED uint32_t opCode = bt_sync_env.workInstance[bt_sync_env.nearest_idx].triInfo.opCode;
    uint32_t trigger_chl = bt_sync_env.nearest_idx + APP_BT_SYNC_CHANNEL_OFFSET;

    BT_SYNC_TRACE(2, "(d%d)bt_sync_timeout, opCode:%d", bt_sync_env.nearest_idx, opCode);
    if (bt_sync_env.nearest_idx >= APP_BT_SYNC_CHANNEL_MAX)
    {
        osMutexRelease(bt_sync_mutex_id);
        return;
    }
    osMutexRelease(bt_sync_mutex_id);

    hal_cmu_bt_trigger_set_handler((enum HAL_CMU_BT_TRIGGER_SRC_T)trigger_chl, NULL);
    BT_SYNC_MSG_BLOCK_T msg;
    msg.triResult = APP_BT_SYNC_RESULT_FAILED;
    msg.workIdx = bt_sync_env.nearest_idx;
    app_bt_sync_mailbox_put(&msg);
}

static void app_bt_sync_irq_handler(enum HAL_CMU_BT_TRIGGER_SRC_T src)
{
    uint32_t index = src - APP_BT_SYNC_CHANNEL_OFFSET;
    // sanity check
    if (index >= APP_BT_SYNC_CHANNEL_MAX) {
        BT_SYNC_TRACE(1, "(d%d)bt_sync_irq_handler, error", index);
        return;
    }

    POSSIBLY_UNUSED BT_SYNC_INSTANCE_T *p_work = &(bt_sync_env.workInstance[index]);

    BT_SYNC_TRACE(3, "(d%d)bt_sync_irq_handler, opCode:0x%x",
        index, p_work->triInfo.opCode);

    btdrv_syn_clr_trigger((uint8_t)src);
    hal_cmu_bt_trigger_disable(src);

    BT_SYNC_MSG_BLOCK_T msg;
    msg.triResult = APP_BT_SYNC_RESULT_PASS;
    msg.workIdx = index;
    app_bt_sync_mailbox_put(&msg);
}

static int app_bt_sync_module_init(void)
{
    int32_t lock = int_lock();
    if (bt_sync_initilized) {
        int_unlock(lock);
        return 0;
    }
    bt_sync_initilized = true;
    int_unlock(lock);

    if (!bt_sync_mailbox) {
        bt_sync_mailbox = osMailCreate(osMailQ(bt_sync_mailbox), NULL);
    }

    if (!bt_sync_thread_tid) {
        bt_sync_thread_tid = osThreadCreate(osThread(app_bt_sync_thread), NULL);
    }

    if (!bt_sync_supervision_timer_id) {
        bt_sync_supervision_timer_id = osTimerCreate(osTimer(BT_SYNC_SUPERVISION_TIMER), osTimerOnce, NULL);
    }

    if (!bt_sync_mutex_id) {
        bt_sync_mutex_id = osMutexCreate(osMutex(bt_sync_mutex));
    }

    if (!bt_sync_mutex_id || !bt_sync_mailbox ||
        !bt_sync_thread_tid || !bt_sync_supervision_timer_id) {
        BT_SYNC_TRACE(4, "bt_sync_init, %p-%p-%p-%p", bt_sync_mutex_id,
            bt_sync_mailbox, bt_sync_thread_tid, bt_sync_supervision_timer_id);
        return -1;
    }

    memset((uint8_t *)&bt_sync_env, 0x00, sizeof(bt_sync_env));
    return 0;
}
#endif // USE_BASIC_THREADS

static bool app_bt_sync_process(uint32_t opCode, uint8_t extra_len, uint8_t *p_extra_info,
    uint8_t policy = APP_BT_SYNC_POLICY_DEFAULT, uint8_t triChl = APP_BT_SYNC_INVALID_CHANNEL)
{
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    uint16_t tws_conhandle = bts_tws_if_get_tws_acl_handle();
    BT_SYNC_SHARE_INFO_T share_info = {0};
    uint32_t tg_tick = 0;
    uint32_t curr_ticks = 0;
    uint8_t index = 0xff;
    uint8_t btRole = BTIF_BCR_MASTER;
    uint32_t delayUs = 0;
    struct BT_DEVICE_T *p_device = NULL;
    bool all_in_sniff = false;
    int ret = 0;
    rx_agc_t link_agc_info = {0};
    btif_cmgr_handler_t *cmgrHandler = NULL;
    uint32_t chnl = APP_BT_SYNC_INVALID_CHANNEL;

    /**
    * Step1: Initialize the module
    *
    */
    ret = app_bt_sync_module_init();
    if (ret) {
        goto exit;
    }

    do {
        /**
        * Step2: Do sanity check here
        * 1. TWS not connected or manager handler is null
        * 2. (Support share info to peer bud) The length of share info exceeds the max allowed length
        *    Max size: ARRAY_SIZE(shareBuf) - sizeof(BT_SYNC_TRIGGER_INFO_T)
        * 3. (Support share info to peer bud) Length is not zero, but the pointer is empty.
        * 4. Check to see if there is the same. If yes. then check policy
        *
        */
        if (BT_INVALID_CONN_HANDLE == tws_conhandle) {
            ret = -2;
            break;
        }

        cmgrHandler = btif_lock_free_cmgr_get_acl_handler(p_ibrt_ctrl->p_tws_remote_dev);
        if (!cmgrHandler) {
            ret = -3;
            break;
        }

        if ((extra_len > APP_BT_SYNC_EXTRA_INFO_LEN) ||
            ((extra_len > 0) && !p_extra_info)) {
            ret = -4;
            break;
        }

        /**
        * Step3: Calculate trigger time and get available channel
        *
        */
        btRole = btif_me_get_remote_device_role(p_ibrt_ctrl->p_tws_remote_dev);

        chnl = app_bt_sync_get_available_trigger_channel(opCode, policy);
        if (chnl < APP_BT_SYNC_CHANNEL_TOTAL) {
            /// get the index of trigger management module(
            /// minus 2 for reserved channel0 for A2DP/HFP, channel 1 for prompt)
            index = chnl - APP_BT_SYNC_CHANNEL_OFFSET;
        } else {
            // For central role, should share the last trigger info to peripheral device
            if (APP_BT_SYNC_POLICY_MULTIPLEX == policy) {
                if ((BTIF_BCR_MASTER == btRole) && (triChl != APP_BT_SYNC_INVALID_CHANNEL)) {
                    share_info.trigger_info.triChl = triChl;
                    BT_SYNC_INSTANCE_T *p_work = app_bt_sync_seek_work_instance(opCode);
                    if (p_work) {
                        share_info.trigger_info.triTick = p_work->triInfo.triTick;
                    }
                    goto cal_done;
                } else {
                    break;
                }
            }
            ret = -5;
            break;
        }

        if (BTIF_BCR_SLAVE == btRole) {
            share_info.trigger_info.triChl = chnl;
            goto cal_done;
        }
        // sync trigger channel to peer side
        // No need to sync the trigger channel if the initiator side is central(triChl == APP_BT_SYNC_INVALID_CHANNEL)
        share_info.trigger_info.triChl = triChl;

        if (app_ibrt_if_is_any_mobile_connected())
        {
            all_in_sniff = true;
            for (uint32_t i = 0; i < BT_DEVICE_NUM; ++i) {
                p_device = app_bt_get_device(i);
                if (bts_bt_if_get_dev_link_mode(&p_device->remote) != IBRT_SNIFF_MODE) {
                    all_in_sniff = false;
                    break;
                }
            }
        }

        if (all_in_sniff || p_ibrt_ctrl->tws_mode == IBRT_SNIFF_MODE) {
            bts_tws_if_tws_link_exit_sniff_mode();
            BT_SYNC_TRACE(2, "bt_sync_enable, in sniff mode,itv:%d-%d", p_ibrt_ctrl->tws_mode, all_in_sniff);
            delayUs = BT_SYNC_LONG_POLL_US;
        } else {
            delayUs = BT_SYNC_DEFAULT_POLL_US;
        }

        if (bt_drv_reg_op_read_rssi_in_dbm(tws_conhandle, &link_agc_info)) {
            BT_SYNC_TRACE(1, "bt_sync_enable, TWS-RSSI:%d", link_agc_info.rssi);
            if (link_agc_info.rssi < -90) {
                delayUs *= 4;
            } else if (link_agc_info.rssi < -85) {
                delayUs *= 2;
            }
        }

        curr_ticks = bt_syn_get_curr_ticks(tws_conhandle);
        tg_tick = curr_ticks + US_TO_BTCLKS(delayUs);
        share_info.trigger_info.triTick = tg_tick;

cal_done:
        /**
        * Step4: Share with peer bud if it is BT central
        *
        */
        share_info.trigger_info.opCode = opCode;
        share_info.trigger_info.policy = policy;
        if (p_extra_info)
        {
            memcpy(share_info.extra_info , p_extra_info, extra_len);
        }

        int result = tws_ctrl_send_cmd(APP_TWS_CMD_SET_SYNC_TIME,
                    (uint8_t *)&share_info, sizeof(BT_SYNC_TRIGGER_INFO_T) + extra_len);

        if (!result && (index != APP_BT_SYNC_INVALID_CHANNEL)) {
            app_bt_sync_refresh_supervison_list(true, opCode, index);
            if (BTIF_BCR_MASTER == btRole)
            {
                BT_SYNC_INSTANCE_T *p_work = &bt_sync_env.workInstance[index];
                p_work->triInfo.triTick = tg_tick;
                BT_SYNC_TRACE(4, "bt_sync_enable, triSrc:%d, curr tick:%u+delayUs:%d > tg_tick:%u, policy:%d",
                    p_work->triInfo.triChl, curr_ticks, delayUs, tg_tick, policy);

                /**
                * Step5: enable BT STAMP
                *
                */
                bt_syn_set_tg_ticks(tg_tick, tws_conhandle,
                    BT_TRIG_MASTER_ROLE, chnl, false);
                hal_cmu_bt_trigger_set_handler((enum HAL_CMU_BT_TRIGGER_SRC_T)chnl,
                    app_bt_sync_irq_handler);
                hal_cmu_bt_trigger_enable((enum HAL_CMU_BT_TRIGGER_SRC_T)chnl);
            }
        } else {
            ret = -6;
        }

    } while(0);

exit:
    BT_SYNC_TRACE(1, "bt_sync_enable, ret:%d chl:%d", ret, chnl);
    if (ret) {
        app_bt_sync_release_trigger_channel(chnl);
        return false;
    }
    return true;
}
/*--------------------------------------------------------------------
                         EXTERNAL PROCEDURES
--------------------------------------------------------------------*/
void app_bt_sync_get_master_time_from_slave_time(uint32_t SlaveBtTicksUs,
    uint32_t* p_master_clk_cnt, uint16_t* p_master_bit_cnt)
{
    int32_t offset;
    uint8_t link_id = btdrv_conhdl_to_linkid(bts_tws_if_get_tws_acl_handle());
    uint32_t slaveClockCnt;
    uint16_t finecnt;
    btdrv_reg_op_bts_to_bt_time(SlaveBtTicksUs, &slaveClockCnt, &finecnt);

    if (btdrv_is_link_index_valid(link_id))
    {
        offset = bt_drv_reg_op_get_clkoffset(link_id);
    }
    else
    {
        offset = 0;
    }

    *p_master_clk_cnt = (slaveClockCnt + offset) & 0x0fffffff;
    *p_master_bit_cnt = finecnt;
}

uint32_t app_bt_sync_get_slave_time_from_master_time(uint32_t master_clk_cnt, uint16_t master_bit_cnt)
{
    int32_t offset;
    uint8_t link_id = btdrv_conhdl_to_linkid(bts_tws_if_get_tws_acl_handle());

    if (btdrv_is_link_index_valid(link_id))
    {
        offset = bt_drv_reg_op_get_clkoffset(link_id);
    }
    else
    {
        offset = 0;
    }
    uint32_t slaveClockCnt = (master_clk_cnt - offset) & 0x0fffffff;

    uint32_t SlaveBtTicksUs = btdrv_reg_op_bt_time_to_bts(slaveClockCnt, master_bit_cnt);
    return SlaveBtTicksUs;
}

uint8_t app_bt_sync_get_available_trigger_channel(uint32_t opcode = APP_BT_SYNC_OP_MAX,
    uint8_t policy = APP_BT_SYNC_POLICY_DEFAULT)
{
    // Need to initilize first.
    // Otherwise the variable locTriggerInfo will be cleared by sync module
    int ret = app_bt_sync_module_init();
    if (ret) {
        return APP_BT_SYNC_INVALID_CHANNEL;
    }

    osMutexWait(bt_sync_mutex_id, osWaitForever);
    uint32_t index = 0;
    uint32_t chl = APP_BT_SYNC_INVALID_CHANNEL;
    BT_SYNC_INSTANCE_T *p_work = &bt_sync_env.workInstance[0];

    for (; index < APP_BT_SYNC_CHANNEL_MAX; index++)
    {
        // The same opcode is already in the queue.
        if ((p_work->syncType != BT_SYNC_TYPE_NONE) && (opcode == p_work->triInfo.opCode))
        {
            BT_SYNC_TRACE(1,"bt_sync_get_ava_chl,find same job, chl:%d, policy:%d",
                p_work->triInfo.triChl, policy);
            if (APP_BT_SYNC_POLICY_UPDATE == policy) {
                btdrv_syn_clr_trigger(p_work->triInfo.triChl);
                hal_cmu_bt_trigger_disable((HAL_CMU_BT_TRIGGER_SRC_T)p_work->triInfo.triChl);
                break;
            } else if (APP_BT_SYNC_POLICY_MULTIPLEX == policy) {
                goto exit;
            }
        }

        if (p_work->syncType == BT_SYNC_TYPE_NONE)
        {
            break;
        }
        p_work++;
    }

    if (index < APP_BT_SYNC_CHANNEL_MAX)
    {
        chl = index + APP_BT_SYNC_CHANNEL_OFFSET;
        p_work = &bt_sync_env.workInstance[index];
        p_work->syncType = opcode == APP_BT_SYNC_OP_MAX ? BT_SYNC_TYPE_AUDIO : BT_SYNC_TYPE_COMMON;
        p_work->triInfo.opCode = opcode;
        // channel0 is reserved for A2DP/HFP, channel1 is reserved for prompt
        p_work->triInfo.triChl = chl;
    }

exit:
    osMutexRelease(bt_sync_mutex_id);
    BT_SYNC_TRACE(2, "bt_sync_get_ava_chl, chl:%d, opcode:%d", chl, opcode);
    return chl;
}

void app_bt_sync_register_report_info_callback(APP_BT_SYNC_INFO_REPORT_T cb)
{
    bt_sync_share_info_report_func = cb;
}

void app_bt_sync_send_tws_cmd_done(uint8_t *ptrParam, uint16_t paramLen)
{
    if (!ptrParam || (paramLen < sizeof(BT_SYNC_TRIGGER_INFO_T))) {
        BT_SYNC_TRACE(1, "%s, len error:%d", __func__, paramLen);
        return;
    }

    BT_SYNC_TRIGGER_INFO_T triInfo;
    memcpy((uint8_t*)&triInfo, ptrParam, sizeof(BT_SYNC_TRIGGER_INFO_T));

    BT_SYNC_INSTANCE_T *p_work = NULL;

    p_work = app_bt_sync_seek_work_instance(triInfo.opCode);
    if (p_work) {
        p_work->twsShareDone = 1;
    }
}

void app_bt_sync_tws_cmd_handler(uint8_t *p_buff, uint16_t length)
{
    uint32_t len_tri_info = sizeof(BT_SYNC_TRIGGER_INFO_T);
    if (!p_buff || (length < len_tri_info)) {
        BT_SYNC_TRACE(1, "%s, len error:%d", __func__, length);
        return;
    }

    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    uint16_t tws_conhandle = bts_tws_if_get_tws_acl_handle();
    uint32_t curr_ticks = 0;
    int ret = 0;
    uint32_t chnl = APP_BT_SYNC_INVALID_CHANNEL;
    uint32_t index = 0;
    uint8_t btRole = BTIF_BCR_MASTER;
    BT_SYNC_SHARE_INFO_T *p_shareInfo = (BT_SYNC_SHARE_INFO_T *)p_buff;

    // TODO: report function add into function table
    if (bt_sync_share_info_report_func && (length > len_tri_info)) {
        bt_sync_share_info_report_func(p_shareInfo->trigger_info.opCode, p_shareInfo->extra_info, length - len_tri_info);
    }

    btRole = btif_me_get_remote_device_role(p_ibrt_ctrl->p_tws_remote_dev);
    BT_SYNC_TRACE(4, "bt_sync_tws_cmd_handler, opCode:%d role:%d tick:%u, chl:%d", p_shareInfo->trigger_info.opCode,
        btRole, p_shareInfo->trigger_info.triTick, p_shareInfo->trigger_info.triChl);

    if (BTIF_BCR_SLAVE == btRole)
    {
        do {
            /**
            * Step1: Do sanity check here
            * 1. Module init failed
            * 2. TWS not connected
            * 3. The maximum limit has been reached.
            * 4. Check trigger ticks
            *
            */
            ret = app_bt_sync_module_init();
            if (ret) {
                BT_SYNC_TRACE(2, "bt_sync_tws_cmd_handler, init failed:%d", ret);
                ret = -1;
                break;
            }

            if (BT_INVALID_CONN_HANDLE == tws_conhandle) {
                ret = -2;
                break;
            }

            curr_ticks = bt_syn_get_curr_ticks(tws_conhandle);
            BT_SYNC_TRACE(2, "bt_sync_tws_cmd_handler, curr tick:%u-%u", curr_ticks, p_shareInfo->trigger_info.triTick);
            if (curr_ticks >= p_shareInfo->trigger_info.triTick){
                ret = -3;
                break;
            }

            chnl = app_bt_sync_get_available_trigger_channel(p_shareInfo->trigger_info.opCode, APP_BT_SYNC_POLICY_UPDATE);
            if (chnl < APP_BT_SYNC_CHANNEL_TOTAL) {
                /// get the index of trigger management module(
                /// minus 2 for reserved channel0 for A2DP/HFP, channel 1 for prompt)
                index = chnl - APP_BT_SYNC_CHANNEL_OFFSET;
            } else {
                // do nothing, find ava channel failed or policy is APP_BT_SYNC_POLICY_MULTIPLEX
                ret = -4;
                break;
            }

            app_bt_sync_refresh_supervison_list(true, p_shareInfo->trigger_info.opCode, index);
            bt_syn_set_tg_ticks(p_shareInfo->trigger_info.triTick, tws_conhandle,
                BT_TRIG_SLAVE_ROLE, chnl, false);
            hal_cmu_bt_trigger_set_handler((enum HAL_CMU_BT_TRIGGER_SRC_T)chnl,
                app_bt_sync_irq_handler);
            hal_cmu_bt_trigger_enable((enum HAL_CMU_BT_TRIGGER_SRC_T)chnl);
        } while(0);
    }
    else if (BTIF_BCR_MASTER == btRole)
    {
        ret = app_bt_sync_enable(p_shareInfo->trigger_info.opCode, 0, NULL, p_shareInfo->trigger_info.policy);
    }
    else
    {
        ret = -5;
    }

    BT_SYNC_TRACE(1, "bt_sync_tws_cmd_handler, ret:%d", ret);
    return;
}

void app_bt_sync_abandon_work_instance(uint32_t opCode)
{
    osMutexWait(bt_sync_mutex_id, osWaitForever);
    BT_SYNC_INSTANCE_T *p_work = app_bt_sync_seek_work_instance(opCode);
    if (p_work) {
        p_work->isAbandoned = true;
    }
    osMutexRelease(bt_sync_mutex_id);
}

void app_bt_sync_release_trigger_channel(uint8_t chnl)
{
    BT_SYNC_INSTANCE_T *p_work = NULL;
    uint32_t index = APP_BT_SYNC_INVALID_CHANNEL;

    osMutexWait(bt_sync_mutex_id, osWaitForever);
    index = chnl - APP_BT_SYNC_CHANNEL_OFFSET;
    // sanity check
    if (index >= APP_BT_SYNC_CHANNEL_MAX) {
        osMutexRelease(bt_sync_mutex_id);
        BT_SYNC_TRACE(1, "bt_sync_release_chl, chl(%d) error", chnl);
        return;
    }

    p_work = &bt_sync_env.workInstance[index];
    BT_SYNC_TRACE(1, "(d%d)bt_sync_release_chl, chl:%d type:%d, opcode:%d",
        index, chnl, p_work->syncType, p_work->triInfo.opCode);
    memset(p_work, 0x0, sizeof(BT_SYNC_INSTANCE_T));
    osMutexRelease(bt_sync_mutex_id);
}

bool app_bt_sync_enable(uint32_t opCode, uint8_t extra_len,
    uint8_t *p_extra_info, uint8_t policy = APP_BT_SYNC_POLICY_DEFAULT)
{
    return app_bt_sync_process(opCode, extra_len, p_extra_info, policy, APP_BT_SYNC_INVALID_CHANNEL);
}

#ifdef BT_SYNC_TEST_MODULE
static void app_bt_sync_test_handler(void)
{
    TRACE(1, "%s", __func__);
}
static void app_bt_sync_status_notify(uint32_t opCode, bool triStatus, bool triInfoSentStatus)
{
    TRACE(1, "%s op:%d sts:%d sendSts:%d", __func__, opCode, triStatus, triInfoSentStatus);
}
APP_BT_SYNC_COMMAND_TO_ADD(APP_BT_SYNC_OP_TEST, app_bt_sync_test_handler, app_bt_sync_status_notify);
#endif /* BT_SYNC_TEST_MODULE */
#endif /* defined(__BT_SYNC__) && defined(IBRT) */
