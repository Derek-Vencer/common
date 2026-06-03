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
#include "plat_types.h"
#include "hal_trace.h"
#include "stream_mcps.h"
#include "app_anc_assist.h"
#include "algo_process.h"

/***************************fir lms***************************/
#ifdef VOICE_ASSIST_FF_FIR_LMS
#include "anc_ff_fir_lms.h"
#include "app_voice_assist_fir_lms.h"
#include "anc_assist_mic.h"

extern int voice_assist_fir_lms_process(process_frame_data_t *process_frame_data);

static algo_process_cfg_t g_playback_cfg = {0};
static int32_t algo_process_voice_assist_fir_lms_open(algo_process_cfg_t *alg_cfg)
{
    g_playback_cfg = *alg_cfg;
    MCPP_TRACE(4, "[%s] sample_rate=%d, frame_len=%d, mic_num=%d, mode=%d", __func__, \
                                        g_playback_cfg.sample_rate, \
                                        g_playback_cfg.frame_len, \
                                        g_playback_cfg.mic_num, \
                                        g_playback_cfg.mode);

    // FIR_LMS_CALIB_GAIN *gain_p = (FIR_LMS_CALIB_GAIN *)g_playback_cfg.params;
    // MCPP_TRACE(0,"[%s] %p calib_gain = ff %d fb %d mc %d", __func__, gain_p, (int)(gain_p->ff_gain), (int)(gain_p->fb_gain), (int)(gain_p->mc_gain));

    void *share_list = (void*)g_playback_cfg.params;
    voice_assist_fir_lms_open(share_list);

#if defined(CALC_MIPS_CNT_THD_MS)
    float frame_ms = (float)(g_playback_cfg.frame_len * 1000) / (float)g_playback_cfg.sample_rate;
    stream_mcps_start("FIR_LMS", SYSFREQ_VALUE_FOR_CALC_MCPS, frame_ms, CALC_MIPS_CNT_THD_MS);
    MCPP_TRACE(4, "[%s] sample rate=%d, frams ms=%.2f, cnt_ms=%d", __func__,
                        g_playback_cfg.sample_rate, (double)frame_ms, CALC_MIPS_CNT_THD_MS);
#endif
    return 0;
}

static int32_t algo_process_voice_assist_fir_lms_close(void)
{
    MCPP_TRACE(1, "[%s] ...", __func__);

    voice_assist_fir_lms_close();

#if defined(CALC_MIPS_CNT_THD_MS)
    stream_mcps_stop("FIR_LMS");
#endif
    return 0;
}

static int32_t algo_process_voice_assist_fir_lms_set_cfg(uint8_t *cfg, uint32_t len)
{
    ASSERT(cfg != NULL, "[%s] cfg is NULL", __func__);

    // Set your algo config or tuning algo config

    return 0;
}

static int32_t algo_process_voice_assist_fir_lms_ctl(uint32_t ctrl, uint8_t *ptr, uint32_t ptr_len)
{
    MCPP_TRACE(0, "[%s] ctrl %d", __func__, ctrl);

    voice_assist_fir_lms_set_cfg(ctrl, ptr, ptr_len);

    return 0;
}

static int32_t algo_process_voice_assist_fir_lms_process(uint8_t *in[], uint8_t *out, uint32_t frame_len)
{
#if defined(CALC_MIPS_CNT_THD_MS)
    stream_mcps_run_pre("FIR_LMS");
#endif

    int32_t ret = 0;

    int loop_cnt = 1;
    int offset = 0;

    ASSERT(((frame_len % FIR_BLOCK_SIZE) == 0), "[%s] the 120 data_len is error ", __func__);
    loop_cnt = frame_len / FIR_BLOCK_SIZE;
    for (int i = 0; i < loop_cnt; i++) {
        float **input_data = (float **)in;
        float *ff_data[MAX_FF_CHANNEL_NUM] = {};
        float *fb_data[MAX_FB_CHANNEL_NUM] = {};
        float *talk_data[MAX_TALK_CHANNEL_NUM] = {};
        float *ref_data[MAX_REF_CHANNEL_NUM] = {};
        float *vpu_data = NULL;
        uint32_t j = 0;
        for (uint32_t i = 0; i < MAX_FF_CHANNEL_NUM; i++){
            ff_data[i] = input_data[j] + offset;
            j++;
        }
        for (uint32_t i = 0; i < MAX_FB_CHANNEL_NUM; i++){
            fb_data[i] = input_data[j] + offset;
            j++;
        }
        for (uint32_t i = 0; i < MAX_TALK_CHANNEL_NUM; i++){
            talk_data[i] = input_data[j] + offset;
            j++;;
        }
        for (uint32_t i = 0; i < MAX_REF_CHANNEL_NUM; i++){
            ref_data[i] = input_data[j] + offset;
            j++;
        }
    #if defined(ANC_ASSIST_VPU)
        vpu_data = input_data[j];
    #endif


        // MCPP_TRACE(0, "[%s] fir lms process...", __func__);
        process_frame_data_t process_frame_data;

        process_frame_data.ff_mic = ff_data;
        process_frame_data.ff_ch_num = MAX_FF_CHANNEL_NUM;
        process_frame_data.fb_mic = fb_data;
        process_frame_data.fb_ch_num = MAX_FB_CHANNEL_NUM;
        process_frame_data.talk_mic = talk_data;
        process_frame_data.talk_ch_num = MAX_TALK_CHANNEL_NUM;
        process_frame_data.ref = ref_data;
        process_frame_data.ref_ch_num = MAX_REF_CHANNEL_NUM;
        process_frame_data.vpu_mic = vpu_data;
        process_frame_data.frame_len = FIR_BLOCK_SIZE;
        ret = voice_assist_fir_lms_process(&process_frame_data);

        offset += FIR_BLOCK_SIZE;
    }

#if defined(CALC_MIPS_CNT_THD_MS)
    stream_mcps_run_post("FIR_LMS");
#endif
    return ret;
}
const algo_process_stream_t voice_assist_fir_lms = {
    .open    = algo_process_voice_assist_fir_lms_open,
    .close   = algo_process_voice_assist_fir_lms_close,
    .set_cfg = algo_process_voice_assist_fir_lms_set_cfg,
    .ctl     = algo_process_voice_assist_fir_lms_ctl,
    .process = algo_process_voice_assist_fir_lms_process,
};
#else
const algo_process_stream_t voice_assist_fir_lms = {
};
#endif
