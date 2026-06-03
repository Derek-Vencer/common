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
#include "cp_accel.h"
#include "cmsis_os.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "anc_ff_fir_lms.h"
#include "stream_mcps.h"
#include "app_voice_assist_fir_lms_cp.h"
#include "app_voice_assist_fir_lms_thread_common.h"
#include "app_mcpp_comm.h"
#include "mcpp_server.h"

// #define CALC_MIPS_CNT_THD_MS (3000)

extern int32_t voice_assist_event_detection_process(process_frame_data_t *buf, EventController* event_control);
extern int32_t voice_assist_anc_ff_fir_lms_process(process_frame_data_t *buf, EventController* event_control);
#ifdef MC_FIR_LMS_ENABLED
extern int32_t voice_assist_anc_mc_fir_lms_process(float **mic_data, EventController* event_control);
#endif

//fifo cache data
static CP_BSS_LOC fir_lms_fifo_t event_detection_fifo;
static CP_BSS_LOC float event_detection_mic_cache_buf[MIC_INDEX_QTY][FIR_LMS_QUEUE_BUF_LEN];
static CP_BSS_LOC uint8_t event_detection_evc_cache_buf[FIR_LMS_EVC_QUEUE_BUF_LEN];

static CP_BSS_LOC fir_lms_fifo_t fir_lms_fifo;
static CP_BSS_LOC float fir_lms_mic_cache_buf[MIC_INDEX_QTY][FIR_LMS_QUEUE_BUF_LEN];
static CP_BSS_LOC uint8_t fir_lms_evc_cache_buf[FIR_LMS_EVC_QUEUE_BUF_LEN];

//algo process data
static CP_BSS_LOC float fir_lms_mic_using_buf[MIC_INDEX_QTY][FIR_BLOCK_SIZE];
static CP_BSS_LOC process_frame_data_t fir_lms_process_data;
static CP_BSS_LOC EventController input_evc_buf;

typedef enum {
    FIR_LMS_CP_STATUS_NONE,
    FIR_LMS_CP_STATUS_IDLE,
    FIR_LMS_CP_STATUS_BUSY,
    FIR_LMS_CP_STATUS_DETECTION,
} FIR_LMS_CP_STATUS_T;

typedef enum {
    EVENT_DETECTION_AP_STATUS_NONE,
    EVENT_DETECTION_AP_STATUS_IDLE,
    EVENT_DETECTION_AP_STATUS_BUSY,
} EVENT_DETECTION_AP_STATUS_T;

static FIR_LMS_CP_STATUS_T g_fir_lms_status = FIR_LMS_CP_STATUS_NONE;
static EVENT_DETECTION_AP_STATUS_T g_event_detection_status = EVENT_DETECTION_AP_STATUS_NONE;
static bool g_event_detection_cp_status = false;
// static bool g_fir_lms_cp_status = false;
static bool g_fir_send_ctl_flag = false;
static void *g_fir_send_ctl_ptr = NULL;
static uint16_t g_fir_send_ctl_ptr_len = 0;

void flr_lms_cp_switch_event_detection_core(bool is_cp)
{
    if (g_event_detection_cp_status == is_cp) {
        return;
    }

    if ((is_cp && !get_cpu_id()) || (!is_cp && get_cpu_id())) {
        VOICE_ASSIST_TRACE(1, "[%s] set g_event_detection_cp_status to %d", __func__, is_cp);
        g_event_detection_cp_status = is_cp;
        return;
    }

    uint32_t cnt = 0;
    if (is_cp && get_cpu_id()) {
        while (g_event_detection_status == EVENT_DETECTION_AP_STATUS_BUSY) {
            hal_sys_timer_delay_us(100);
            cnt++;
            if (cnt % 10 == 0) {
                if (cnt == 10 * 15) {     // 15ms
                    VOICE_ASSIST_TRACE(1, "[%s] ERROR: AP process not stop!!!", __func__);
                    break;
                } else {
                    VOICE_ASSIST_TRACE(1, "[%s] Wait AP process done...%d(us)", __func__, cnt * 100);
                }
            }
        }
    } else if (!is_cp && !get_cpu_id()) {
        while (g_fir_lms_status == FIR_LMS_CP_STATUS_BUSY) {
            hal_sys_timer_delay_us(100);
            cnt++;
            if (cnt % 10 == 0) {
                if (cnt == 10 * 15) {     // 15ms
                    VOICE_ASSIST_TRACE(1, "[%s] ERROR: CP process not stop!!!", __func__);
                    break;
                } else {
                    VOICE_ASSIST_TRACE(1, "[%s] Wait CP process done...%d(us)", __func__, cnt * 100);
                }
            }
        }
    }

    VOICE_ASSIST_TRACE(1, "[%s] set g_event_detection_cp_status to %d", __func__, is_cp);
    g_event_detection_cp_status = is_cp;
}

// void set_flr_lms_cp_status(bool status)
// {
//     VOICE_ASSIST_TRACE(1, "[%s] set g_fir_lms_cp_status to %d", __func__, status);
//     g_fir_lms_cp_status = status;
// }

CP_TEXT_SRAM_LOC
static void fir_lms_cp_process_handler(void)
{
    if (g_fir_lms_status == FIR_LMS_CP_STATUS_NONE) {
        return;
    }

    // Algo process
    g_fir_lms_status = FIR_LMS_CP_STATUS_BUSY;

#if defined(CALC_MIPS_CNT_THD_MS)
    stream_mcps_run_pre("FIR_LMS_CP");
#endif

    if (g_event_detection_cp_status && !fir_lms_fifo_data_is_ready(&fir_lms_fifo)) {
        fir_lms_fifo_get_data(&event_detection_fifo, &fir_lms_process_data, FIR_BLOCK_SIZE, &input_evc_buf, sizeof(EventController));
        voice_assist_event_detection_process(&fir_lms_process_data, &input_evc_buf);
    } else {
        fir_lms_fifo_get_data(&fir_lms_fifo, &fir_lms_process_data, FIR_BLOCK_SIZE, &input_evc_buf, sizeof(EventController));
    }

    g_fir_lms_status = FIR_LMS_CP_STATUS_DETECTION;

    // if (g_fir_lms_cp_status) {
        POSSIBLY_UNUSED int32_t ff_res = -1;
        ff_res = voice_assist_anc_ff_fir_lms_process(&fir_lms_process_data, &input_evc_buf);
#ifdef MC_FIR_LMS_ENABLED
        if(ff_res != ANC_FF_FIR_LMS_RES_IN_CONVERGING){
            voice_assist_anc_mc_fir_lms_process(&fir_lms_process_data, &input_evc_buf);
        }
#endif
    // }

#if defined(CALC_MIPS_CNT_THD_MS)
    stream_mcps_run_post("FIR_LMS_CP");
#endif

    g_fir_lms_status = FIR_LMS_CP_STATUS_IDLE;
}

CP_TEXT_SRAM_LOC
static unsigned int fir_lms_cp_main(uint8_t event)
{
    fir_lms_cp_process_handler();

    return 0;
}

static const struct cp_task_desc task_desc_fir_lms = {CP_ACCEL_STATE_CLOSED, fir_lms_cp_main, NULL, NULL, NULL};

int fir_lms_cp_init(void)
{
    // framework init
#if defined(CALC_MIPS_CNT_THD_MS)
    float frame_ms = (float)(FIR_BLOCK_SIZE * 1000) / (float)FIR_SAMPLE_RATE;
    stream_mcps_start("FIR_LMS_CP", 96, frame_ms, CALC_MIPS_CNT_THD_MS);
    VOICE_ASSIST_TRACE(4, "[%s] sample rate=%d, frams us=%d, cnt_ms=%d", __func__,
                        FIR_SAMPLE_RATE, (int32_t)frame_ms*1000, CALC_MIPS_CNT_THD_MS);
#endif

    // fifo init
    float *mic_buf[MIC_INDEX_QTY];
    for (uint32_t i = 0; i < MIC_INDEX_QTY; i++) {
        mic_buf[i] = fir_lms_mic_cache_buf[i];
    }
    fir_lms_fifo_init(&fir_lms_fifo, mic_buf, fir_lms_evc_cache_buf);
    VOICE_ASSIST_TRACE(0,"[%s] fir_lms_fifo %p",__func__, &fir_lms_fifo);

    for (uint32_t i = 0; i < MIC_INDEX_QTY; i++) {
        mic_buf[i] = event_detection_mic_cache_buf[i];
    }
    fir_lms_fifo_init(&event_detection_fifo, mic_buf, event_detection_evc_cache_buf);
    VOICE_ASSIST_TRACE(0,"[%s] event_detection_fifo %p",__func__, &event_detection_fifo);

    // process_data init
    static float *ff_mic_buf[MAX_FF_CHANNEL_NUM] = {NULL,};
    static float *fb_mic_buf[MAX_FB_CHANNEL_NUM] = {NULL,};
    static float *talk_mic_buf[MAX_TALK_CHANNEL_NUM] = {NULL,};
    static float *ref_mic_buf[MAX_REF_CHANNEL_NUM] = {NULL,};
    float *vpu_buf = NULL;

    uint32_t j = 0;
    for (uint32_t i = 0; i < MAX_FF_CHANNEL_NUM; i++){
        ff_mic_buf[i] = fir_lms_mic_using_buf[j];
        j++;
    }
    for (uint32_t i = 0; i < MAX_FB_CHANNEL_NUM; i++){
        fb_mic_buf[i] = fir_lms_mic_using_buf[j];
        j++;
    }
    for (uint32_t i = 0; i < MAX_TALK_CHANNEL_NUM; i++){
        talk_mic_buf[i] = fir_lms_mic_using_buf[j];
        j++;
    }
    for (uint32_t i = 0; i < MAX_REF_CHANNEL_NUM; i++){
        ref_mic_buf[i] = fir_lms_mic_using_buf[j];
        j++;
    }
#if defined(ANC_ASSIST_VPU)
    vpu_buf = fir_lms_mic_using_buf[j];
#endif
    void *mic_buf2[5] = {ff_mic_buf, fb_mic_buf, talk_mic_buf, ref_mic_buf, vpu_buf};
    fir_lms_fifo_process_data_init(&fir_lms_process_data, mic_buf2, FIR_BLOCK_SIZE);

    // Algo init

    // cp init
    cp_accel_open(CP_TASK_FIR_LMS, &task_desc_fir_lms);

    uint32_t cnt = 0;
    while(cp_accel_init_done() == false) {
        hal_sys_timer_delay_us(100);
        cnt++;
        if (cnt % 10 == 0) {
            if (cnt == 10 * 200) {     // 200ms
                ASSERT(0, "[%s] ERROR: Can not init cp!!!", __func__);
            } else {
                VOICE_ASSIST_TRACE(1, "[%s] Wait CP init done...%d(ms)", __func__, cnt/10);
            }
        }
    }

    g_fir_lms_status = FIR_LMS_CP_STATUS_IDLE;
    g_event_detection_status = EVENT_DETECTION_AP_STATUS_IDLE;

    return 0;
}

int fir_lms_cp_deinit(void)
{
    VOICE_ASSIST_TRACE(1,"[%s] ...", __func__);

    uint32_t cnt = 0;
    while (g_fir_lms_status != FIR_LMS_CP_STATUS_BUSY) {
        hal_sys_timer_delay_us(100);
        cnt++;
        if (cnt % 10 == 0) {
            if (cnt == 10 * 5) {     // 5ms
                VOICE_ASSIST_TRACE(1, "[%s] ERROR: CP process not stop!!!", __func__);
                break;
            } else {
                VOICE_ASSIST_TRACE(1, "[%s] Wait CP process done...%d(us)", __func__, cnt * 100);
            }
        }
    }

    cp_accel_close(CP_TASK_FIR_LMS);
    fir_lms_fifo_data_clear(&event_detection_fifo, &fir_lms_process_data, FIR_BLOCK_SIZE, &input_evc_buf, sizeof(EventController));
    fir_lms_fifo_data_clear(&fir_lms_fifo, &fir_lms_process_data, FIR_BLOCK_SIZE, &input_evc_buf, sizeof(EventController));
    g_event_detection_cp_status = false;
    // g_fir_lms_cp_status = false;
    g_fir_lms_status = FIR_LMS_CP_STATUS_NONE;
    g_event_detection_status = EVENT_DETECTION_AP_STATUS_NONE;

#if defined(CALC_MIPS_CNT_THD_MS)
    stream_mcps_stop("FIR_LMS_CP");
#endif

    // Algo deinit

    return 0;
}

void app_voice_assist_fir_lms_cp_process(process_frame_data_t *data_buf, EventController* event_control)
{
    if (g_fir_send_ctl_flag == true) {
        mcpp_capture_ctl_cmd_send_handler_done(APP_MCPP_USER_VOICE_ASSIST_FIR_LMS, g_fir_send_ctl_ptr, g_fir_send_ctl_ptr_len);
        g_fir_send_ctl_flag = false;
    }

    static uint32_t fifo_total_cnt = 0;
    static uint32_t fifo_overflow_cnt = 0;
    uint32_t fifo_res = 0;

    g_event_detection_status = EVENT_DETECTION_AP_STATUS_BUSY;
    if (g_event_detection_cp_status == false) {
        voice_assist_event_detection_process(data_buf, event_control);
        fifo_res = fir_lms_fifo_put_data(&fir_lms_fifo, data_buf, FIR_BLOCK_SIZE, event_control, sizeof(EventController));
    } else {
        fifo_res = fir_lms_fifo_put_data(&event_detection_fifo, data_buf, FIR_BLOCK_SIZE, event_control, sizeof(EventController));
    }
    g_event_detection_status = EVENT_DETECTION_AP_STATUS_IDLE;

    if (fifo_res == -1){
        fifo_overflow_cnt++;
    }
    fifo_total_cnt++;
    if(fifo_total_cnt % 1000 == 0){
        VOICE_ASSIST_TRACE(0,"[fir lms fifo] total_cnt = %d, overflow_cnt = %d", fifo_total_cnt, fifo_overflow_cnt);
    }

    cp_accel_send_event_mcu2cp(CP_BUILD_ID(CP_TASK_FIR_LMS, CP_EVENT_FIR_LMS_PROCESSING));
}

void app_voice_assist_fir_lms_cp_send_stl_to_bth(void *ptr, uint16_t ptr_len)
{
    if (get_cpu_id() == 0) {
        mcpp_capture_ctl_cmd_send_handler_done(APP_MCPP_USER_VOICE_ASSIST_FIR_LMS, ptr, ptr_len);
        return;
    }

    g_fir_send_ctl_ptr = ptr;
    g_fir_send_ctl_ptr_len = ptr_len;
    g_fir_send_ctl_flag = true;

    int32_t cnt = 0;

    while (g_fir_send_ctl_flag) {
        hal_sys_timer_delay_us(100);
        cnt++;
        if (cnt % 10 == 0) {
            if (cnt == 10 * 50) {     // 50ms
                VOICE_ASSIST_TRACE(1, "[%s] FIR ctl command send error!!!", __func__);
                break;
            } else {
                VOICE_ASSIST_TRACE(1, "[%s] FIR ctl command sending ...%d(us)", __func__, cnt * 100);
            }
        }
    }
}
