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


#ifndef __ANC_FIR_LMS_COMMON_H__
#define __ANC_FIR_LMS_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stdbool.h"
#include "stdint.h"
#include "iirfilt.h"
#include "custom_allocator.h"
#include "anc_assist.h"


#ifndef VQE_SIMULATE
#include "anc_process.h"
#else
#include "hal_aud.h"
// copy from anc_process.h
#define AUD_COEF_LEN        (500)
#define AUD_IIR_NUM        (13)

typedef struct _aud_fir_item
{
    int32_t    fir_bypass_flag;
    int32_t    fir_len;
    int32_t    fir_coef[AUD_COEF_LEN];
} aud_fir_item;

typedef struct _struct_anc_fir_cfg
{
    aud_fir_item anc_fir_cfg_ff_l;
    aud_fir_item anc_fir_cfg_ff_r;
    aud_fir_item anc_fir_cfg_fb_l;
    aud_fir_item anc_fir_cfg_fb_r;
    aud_fir_item anc_fir_cfg_tt_l;
    aud_fir_item anc_fir_cfg_tt_r;
    aud_fir_item anc_fir_cfg_mc_l;
    aud_fir_item anc_fir_cfg_mc_r;
} struct_anc_fir_cfg;
#endif

#if defined(FREEMAN_ENABLED_STERO)
#define FIR_CHANNEL_NUM (2)
#else
#define FIR_CHANNEL_NUM (1)
#endif

// #define WEAR_LEAK_STATUS_CHANGED_DETECT
// #define MC_FIR_LMS_ENABLED

#if (FIR_CHANNEL_NUM == 2)
#define LOCAL_FIR_LEN (960)
#else
#define LOCAL_FIR_LEN (960)
#endif

#define FIR_SAMPLE_RATE (32000)

#if FIR_SAMPLE_RATE == 48000
#define FIR_BLOCK_SIZE (360)
#elif FIR_SAMPLE_RATE == 32000
#define FIR_BLOCK_SIZE (240)
#else
#define FIR_BLOCK_SIZE (120)
#endif

typedef enum
{
    ANC_FF_FIR_LMS_STAGE_IDLE = 0,
    ANC_FF_FIR_LMS_STAGE_SKIP,
    ANC_FF_FIR_LMS_STAGE_PNC,
    ANC_FF_FIR_LMS_STAGE_WAITING_ANC_ON,
    ANC_FF_FIR_LMS_STAGE_ANC,
    ANC_FF_FIR_LMS_STAGE_WAITING_ADAPTIVE_ANC_ON,
    ANC_FF_FIR_LMS_STAGE_ADAPTIVE_IIR,
    ANC_FF_FIR_LMS_STAGE_ADAPTIVE_ANC,
    ANC_FF_FIR_LMS_STAGE_NORMAL_ANC,
    ANC_FF_FIR_LMS_STAGE_WAITING_ANC_OFF,
    ANC_FF_FIR_LMS_STAGE_NUM,
} ANC_FF_FIR_LMS_STAGE;

typedef enum
{
    ANC_FF_FIR_LMS_RES_NORMAL = 0,
    ANC_FF_FIR_LMS_RES_PZ_ERR,
    ANC_FF_FIR_LMS_RES_SZ_ERR,
    ANC_FF_FIR_LMS_RES_BACKTRACK,
    ANC_FF_FIR_LMS_RES_BYPASS_FF_FIR,
    ANC_FF_FIR_LMS_RES_CONVERGENT,
    ANC_FF_FIR_LMS_RES_IN_CONVERGING,
} ANC_FF_FIR_LMS_RES;

static POSSIBLY_UNUSED const char *stage_desc[ANC_FF_FIR_LMS_STAGE_NUM] = {
    "IDLE",
    "SKIP",
    "PNC",
    "WAITING ANC ON",
    "ANC",
    "WAITING ADAPTIVE ANC ON",
    "ADAPTIVE IIR",
    "ADAPTIVE ANC",
    "NORMAL ANC",
    "WAITING ANC OFF",
};

typedef struct {
    float ff_gain;
    float fb_gain;
    float mc_gain;
} FIR_LMS_CALIB_GAIN;

typedef struct {
    enum IIR_BIQUARD_TYPE type;
    float       gain;
    int         freq;
    float       q;
}Biquard_Param;

typedef struct {
    bool reverse;
    float ref_gain;
    uint32_t filt_cnt;
    Biquard_Param filt[AUD_IIR_NUM];
}FIR_LMS_FILTER_CFG_T;

typedef struct {
    float **ff_mic;
    uint8_t ff_ch_num;
    float **fb_mic;
    uint8_t fb_ch_num;
    float **talk_mic;
    uint8_t talk_ch_num;
    float **ref;
    uint8_t ref_ch_num;
    float *vpu_mic;
    uint32_t frame_len;
} process_frame_data_t;

typedef struct
{
    // Status changed flag
    uint32_t    ff_gain_changed[MAX_FF_CHANNEL_NUM];
    uint32_t    fb_gain_changed[MAX_FB_CHANNEL_NUM];
    uint32_t    curve_changed[MAX_FB_CHANNEL_NUM];
    uint32_t    fir_flag_changed;

    // Who change gain or curve
    anc_assist_algo_id_t    ff_gain_id[MAX_FF_CHANNEL_NUM];
    anc_assist_algo_id_t    fb_gain_id[MAX_FB_CHANNEL_NUM];
    anc_assist_algo_id_t    curve_id[MAX_FB_CHANNEL_NUM];
    anc_assist_algo_id_t    fir_flag_id;

    // Gain or curve
    float   ff_gain[MAX_FF_CHANNEL_NUM];
    float   fb_gain[MAX_FB_CHANNEL_NUM];
    uint32_t    curve_index[MAX_FB_CHANNEL_NUM];
    uint32_t    fir_flag;

    noise_status_t noise_status;
    wind_status_t wind_status;

} ANCFFFirLmsRes;

int32_t dsp_set_anc_fir_cfg(struct_anc_fir_cfg *cfg, enum ANC_TYPE_T cmd, enum AUD_CHANNEL_MAP_T map);

typedef int32_t (*anc_ff_fir_lms_set_iir_handler_t)(enum ANC_TYPE_T anc_type, uint32_t index, int32_t *coeffs, int32_t len);

typedef void (*anc_ff_fir_lms_set_fir_freq_handler_t)(int freq, int line);


#ifdef __cplusplus
}
#endif

#endif