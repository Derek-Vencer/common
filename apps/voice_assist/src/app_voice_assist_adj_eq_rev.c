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
#include "hal_trace.h"
#include "plat_types.h"
#include "audioflinger.h"
#include "speech_ssat.h"
#include "app_anc_assist.h"
#include "adj_eq_rev.h"
#include "audio_dump.h"
#include "app_utils.h"

#define HEAP_BUFF_SIZE (3 * 1024)
#include "ext_heap.h"
static uint8_t adj_eq_ext_heap[HEAP_BUFF_SIZE];
// #define CAL_MIPS
#ifdef CAL_MIPS
#include "hal_timer.h"
#endif
static int32_t g_working_status = 0;
static AdjEQState *adj_eq_rev_st;
extern const AdjEQConfig audio_adj_eq_rev_cfg;
extern void app_anc_assist_get_cfg(int *frame_size, int *sample_rate, int *ch_num);
extern void audio_adj_eq_rev_switch(int val);

static int32_t _app_voice_assist_adj_eq_rev_callback(void *buf, uint32_t len, void *other);

int32_t app_voice_assist_adj_eq_rev_init(void)
{
    app_anc_assist_register(ANC_ASSIST_USER_ADJ_EQ_REV, _app_voice_assist_adj_eq_rev_callback);
    return 0;
}

int32_t app_voice_assist_adj_eq_rev_open(void)
{
    FS_TRACE(0, "[%s] ADJ EQ open stream", __func__);
    int anc_assist_frame, anc_assist_sample_rate, ch_num;
    app_anc_assist_get_cfg(&anc_assist_frame, &anc_assist_sample_rate, &ch_num);
    ext_heap_init(adj_eq_ext_heap);
    adj_eq_rev_st = adj_eq_rev_create(ch_num, anc_assist_frame, anc_assist_sample_rate, &audio_adj_eq_rev_cfg, &ext_allocator);
    // speed up
    app_sysfreq_req(APP_SYSFREQ_USER_ADJ_EQ_REV, APP_SYSFREQ_104M);
    app_anc_assist_open(ANC_ASSIST_USER_ADJ_EQ_REV);
    return 0;
}

int32_t app_voice_assist_adj_eq_rev_close(void)
{
    FS_TRACE(0, "[%s] ADJ EQ close stream", __func__);
    // recover sysfreq
    app_sysfreq_req(APP_SYSFREQ_USER_ADJ_EQ_REV, APP_SYSFREQ_32K);
    app_anc_assist_close(ANC_ASSIST_USER_ADJ_EQ_REV);
    adj_eq_rev_destroy(adj_eq_rev_st);
    ext_heap_deinit();
    return 0;
}

static int32_t _app_voice_assist_adj_eq_rev_callback(void *buf, uint32_t len, void *other)
{
#ifdef CAL_MIPS
    static int start = 0;
    static int end = 0;
    static int pre = 0;
    int cpu_freq = hal_sys_timer_calc_cpu_freq(5, 0) / 1000000;
    FS_TRACE(2, "[%s]:CPU freq: %u M",__func__, cpu_freq);
    FS_TRACE(3,"=============== start ================");
    start = hal_fast_sys_timer_get();
#endif

    if(g_working_status == 0){
        // FS_TRACE(0, "[%s] ADJ EQ has already closed", __func__);
        return 0;
    }

    uint32_t frame_len = len;//60
    float **fb_buf  = (float **)buf;
    float **ref_buf = (float **)other;
    adj_eq_rev_filter_estimate(adj_eq_rev_st, fb_buf, ref_buf, frame_len);

#ifdef CAL_MIPS
    end = hal_fast_sys_timer_get();
    int mips = (end - start) * cpu_freq / (start - pre);
    TRACE(3,"[%s] Usage: %d / %d (ticks).", __func__, end - start, start - pre);
    TRACE(2,"[%s] Usage: %d (MIPS).", __func__, mips);
    pre = start;
    TRACE(3,"=============== end ================");
#endif
    return 0;
}
void app_voice_assist_adj_eq_rev_set_status(int val)
{
    g_working_status = val;
    audio_adj_eq_rev_switch(g_working_status);
}
