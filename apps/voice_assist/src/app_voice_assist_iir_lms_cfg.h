/***************************************************************************
 *
 * Copyright 2015-2023 BES.
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
#include "anc_ff_iir_lms.h"

//bth_cfg copy to cfg,algo init with cfg
// static ANC_FF_IIR_LMS_CFG_T cfg;
static ANC_FF_IIR_LMS_CFG_T bth_cfg ={
    .iir_cfg = {
        .debug_en = 1,
        .max_cnt = 1000,
        .g_gain_delta = 3.0,
        .g_gain_threshold = 8.0,
        .gain_delta = 0.5,
        .freq_delta = 20,
        .q_delta = 0.1,
        .mu = 1.0,

        .delta_thresh = 5e-6,  // judge convergence
        .start_freq = 50,
        .end_freq = 3500,
        .start_filter_idx = 7, // the idx of the fisrt filter for adaptive iir
    },

    .mc_filt_cfg = { // anc mic coeff
        .reverse = false,
        .ref_gain = -3.0,
        .filt_cnt = 5,
        .filt = {
            {IIR_BIQUARD_PEAKINGEQ,   -10.0,   2200,   1.4},
            {IIR_BIQUARD_HIGHSHELF,   -19.0,   7000,   0.9},
            {IIR_BIQUARD_LOWSHELF,    -11.0,   13,     0.7},
            {IIR_BIQUARD_PEAKINGEQ,    6,      80,     0.25},
            {IIR_BIQUARD_PEAKINGEQ,   -2.2,    860,    0.9},
        },
    },
    
    .ff_filt_cfg = {
        .reverse = false,
        .ref_gain = -5.5,
#if (AUD_IIR_NUM > 8)
        .filt_cnt = 11,
#else
        .filt_cnt = 8,
#endif
        .filt =  {
            {IIR_BIQUARD_PEAKINGEQ,     -10,  13000,   1},
            {IIR_BIQUARD_PEAKINGEQ,     -6,  17000,   1.5},
            {IIR_BIQUARD_PEAKINGEQ,     -10,  9500,   1.5},
            {IIR_BIQUARD_LOWSHELF,     14.5,  20,   0.7},
            {IIR_BIQUARD_PEAKINGEQ,     -14.2,  3100,   2.0},
            {IIR_BIQUARD_PEAKINGEQ,     3,  900,   0.5},
            {IIR_BIQUARD_PEAKINGEQ,     -1,  320,   0.9},
            {IIR_BIQUARD_LOWSHELF,     0,  400,   0.5},
#if (AUD_IIR_NUM > 8)
            {IIR_BIQUARD_PEAKINGEQ,     0,  3100,   2.0},
            {IIR_BIQUARD_PEAKINGEQ,     0,  900,   0.5},
            {IIR_BIQUARD_PEAKINGEQ,     0,  320,   0.9},
#endif
        },
    },

    .fb_filt_cfg = { // anc mic coeff
        .reverse = false,
        .ref_gain = -5.0,
        .filt_cnt = 8,
        .filt = {
            {IIR_BIQUARD_PEAKINGEQ,  -14.5,   6356,   0.66},
            {IIR_BIQUARD_PEAKINGEQ,   18.1,   244,   0.45},
            {IIR_BIQUARD_LOWSHELF,  5.7,  100,   0.6},
            {IIR_BIQUARD_HIGHSHELF, 13.8,  863,   0.3},
            {IIR_BIQUARD_PEAKINGEQ, -12.3,  2576,  1.78},
            {IIR_BIQUARD_PEAKINGEQ,   0,   1522,   1.48},
            {IIR_BIQUARD_PEAKINGEQ,  -10,  28000,   1.5},
            {IIR_BIQUARD_PEAKINGEQ,  -6.4,  3755,   1.4},
        },
    },

    .custom_weighting_cfg = {
        .bypass = 0,
        .gain = 0.f,
        .num = 6,
        .params = {
            {IIR_BIQUARD_HPF, {{100, 0, 0.707}}},
            {IIR_BIQUARD_HPF, {{100, 0, 0.707}}},
            {IIR_BIQUARD_HPF, {{100, 0, 0.707}}},
            {IIR_BIQUARD_LPF, {{4000, 0, 0.707}}},
            {IIR_BIQUARD_LPF, {{4000, 0, 0.707}}},
            {IIR_BIQUARD_LPF, {{4000, 0, 0.707}}},
        },
    },
    .mc_custom_weighting_cfg = {
        .bypass = 0,
        .gain = 0.f,
        .num = 4,
        .params = {
            {IIR_BIQUARD_HPF, {{200, 0, 0.707}}},
            {IIR_BIQUARD_HPF, {{200, 0, 0.707}}},
            {IIR_BIQUARD_LPF, {{2000, 0, 0.707}}},
            {IIR_BIQUARD_LPF, {{2000, 0, 0.707}}},
        },
    },

    .Cp = {
#include "fir_lms_virtual/average_Cp.txt"
    },
    .Cs = {
#include "fir_lms_virtual/average_Cs.txt"
    },
    .Csop = {
#include "fir_lms_virtual/average_Csop.txt"
    },
    .use_Csop = false,

    .event_cfg = {
        .bypass = true,

    .wind_cfg = {
        .debug_en = 0,
        .scale_size = 8,           // freq range,8/scale=1k
        .to_none_targettime = 500, // time=500*7.5ms=3.75s
        .power_thd = 0.0001,
        .no_thd = 0.8,
        .small_thd = 0.7,
        .normal_thd = 0.55,
        .strong_thd = 0.4,
        .gain_none = 1.0,
        .gain_small_to_none = 0.7500,
        .gain_small = 0.5,
        .gain_normal_to_small = 0.3750,
        .gain_normal = 0.25,
        .gain_strong_to_normal = 0.1563,
        .gain_strong = 0.0625,
    },
    .noise_cfg = {
        .debug_en = 0,
        .strong_low_thd = 320,
        .strong_limit_thd = 60,
        .lower_low_thd = 3,
        .lower_mid_thd = 17,
        .quiet_out_thd = 1.5,
        .quiet_thd = 0.2,
        .extremely_quiet_out_thd = 0.25,
        .extremely_quiet_in_thd = 0.1,
        .snr_thd = 100,
        .period = 16,
        .window_size = 5,
        // .strong_count = 18,
        // .normal_count = 12,
        // .quiet_count = 4,
        .trans_table = {
            {0, 4, 12, 12, 18},
            {4, 0, 12, 12, 18},
            {4, 4, 0 , 12, 18},
            {4, 4, 12, 0 , 18},
            {4, 4, 12, 12, 0 },
        },
        .band_freq = {100, 400, 1000, 2000},
        .band_weight = {0.5, 0.5, 0},
    },
    .fb_howling_cfg = {
        .start_freq = 3000,
        .end_freq = 7000,
        .gain_fb = 0.7,
        .trans_table = {
            {0, 0, 0},
            {0, 0, 0},
            {0, 7500, 0},
        },
    },
    .ff_tone_cfg = {
        .debug_en = 0,
        .ind0 = 16,
        .ind1 = 96,
        .time_thd = 1000,
        .power_thd = 1e8f, //4.6e10 0db
    },
    .fb_tone_cfg = {
        .debug_en = 0,
        .ind0 = 16,
        .ind1 = 96,
        .time_thd = 1000,
        .power_thd = 1e9f,
    },
    .env_noise_cfg = {
        .debug_en = 0,
        .up_thresh = -3,
        .quiet_thresh = -60,
        .low_thresh = -85,
        .thresh_delta = 3,
        .trans_table = {
            {0, 67, 67, 67},
            {5,  0, 67,  5},
            {5, 67,  0,  5},
            {67, 5,  5,  0},
        },
    },
    .energy_detection_cfg = {
        .debug_en = 0,
        .timer = 5,
        .up_thresh = 1.2,
        .low_thresh = 0.00005,
        .normal_period = 12,
    },
    .weak_fir_cfg = {
        .debug_en = 0,
        .wear_noise_frame_num = 50,     //100 * 7.5ms
        .wear_change_num = 5,           //10 * (100 * 7.5ms)
        .noise_change_num = 5,          //10 * (100 * 7.5ms)
    },
    .mic_speech_detection_cfg = {
        .debug_en = 0,
        .pnc_energy_ratio_thd = 1.0,
        .anc_energy_ratio_thd_quiet = -10.0, //ff energy < 65 dB
        .msc_thd_quiet = 0.6,
        .anc_energy_ratio_thd_middle = -20.0, //if speak,energy_ratio > energy_ratio_thd,msc < msc_thd;
        .msc_thd_middle = 0.78,
        .detection_cnt_thd = 3,//speech_keep_cnt >= detection_cnt_thd,output status change
        .energy_clac_start = (uint32_t)(200/62.5),
        .energy_clac_end = (uint32_t)(1000/62.5),
        .denoise_thd = -10,
    },
    .vpu_speech_detection_cfg  = {
            .low_freq_threshold = 0.2,
            .high_freq_threshold = 0.06,
            .ratio_threshold = 50,
        },
    }
};
