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


#ifndef __BECO_TEST_H__
#define __BECO_TEST_H__

#include "beco_nn/beco_nnfunctions.h"
#include "beco_nn/beco_nnsupportfunctions.h"
#include "beco_dsp/beco_dspfunctions.h"

#include "hal_timer.h"
#include "hal_trace.h"
#include "dwt.h"

#ifndef TRACE
#define TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#endif
#ifndef TRACE_FLUSH
#define TRACE_FLUSH()           NORM_LOG_FLUSH()
#endif

#define BECO_GET_TIME_ENTER(id) \
    static uint32_t s_time##id, e_time##id, n_cycle##id, lock##id; \
    dwt_enable();   \
    TRACE_FLUSH();  \
    hal_sys_timer_delay(16);    \
    s_time##id = hal_fast_sys_timer_get();  \
    lock##id = int_lock();  \
    dwt_reset_cycle_cnt();

#define BECO_GET_TIME_EXIT(id, func)    \
    n_cycle##id = dwt_read_cycle_cnt(); \
    int_unlock(lock##id);   \
    e_time##id = hal_fast_sys_timer_get();  \
    TRACE_FLUSH();  \
    hal_sys_timer_delay(16);    \
    TRACE(3, "[%s] cost %dus cycle: %d at sysfreq %d", \
        func, \
        FAST_TICKS_TO_US(e_time##id - s_time##id), \
        n_cycle##id, \
        hal_sys_timer_calc_cpu_freq(5, 0));

BECO_C_DECLARATIONS_START

// beco dsp test functions
void test_beco_mat_mult(void);
void test_beco_fir(void);

// beco nn test functions
void test_beco_fully_connected_s8(void);
void test_beco_fully_connected_s16(void);
void test_beco_convolve_1x1_s8(void);
void test_beco_convolve_1x1_s16(void);
void test_beco_convolve_s8(void);
void test_beco_convolve_s16(void);
void test_beco_transpose_convolve_s8(void);
void test_beco_transpose_convolve_s16(void);
void test_beco_depthwise_conv_nx3_s8(void);
void test_beco_depthwise_conv_nx3_s16(void);
void test_beco_transpose_depthwise_conv_s8(void);
void test_beco_transpose_depthwise_conv_s16(void);
void test_beco_gru_s16_pt(void);
void test_beco_gru_s16_pc(void);
void test_beco_bi_gru_s16(void);
void test_beco_lstm_s16_pt(void);
void test_beco_lstm_s16_pc(void);
void test_beco_bi_lstm_s16(void);
void test_beco_avgpool_s8(void);
void test_beco_avgpool_s16(void);
void test_beco_max_pool_s8(void);
void test_beco_max_pool_s16(void);


BECO_C_DECLARATIONS_END
#endif /*__BECO_TEST_H__*/
