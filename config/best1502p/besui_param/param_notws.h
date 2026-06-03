/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
#if defined(BES_NOTWS_EN)
#include "tgt_hardware.h"
#include "aud_section.h"
#include "iir_process.h"
#include "fir_process.h"
#include "drc.h"
#include "limiter.h"
#include "spectrum_fix.h"
#include "dynamic_boost.h"
#include "tgt_hardware_capsensor.h"

#define IIR_COUNTER_FF_L (8)

#if 1
static const struct_anc_cfg POSSIBLY_UNUSED AncFirCoef_50p7k_mode0 = {
    .anc_cfg_ff_l = {
        .total_gain = 512,
        .iir_bypass_flag = 0,
        .iir_counter = 8,
        .iir_coef[0].coef_b = {0x041a9d50,0xf7cbb85e,0x0419aa8a},
        .iir_coef[0].coef_a = {0x08000000,0xf000c7b2,0x07ff3861},
        .iir_coef[1].coef_b = {0x07f9fdbb,0xf01820df,0x07edea8b},
        .iir_coef[1].coef_a = {0x08000000,0xf01820df,0x07e7e846},
        .iir_coef[2].coef_b = {0x07ebbba3,0xf02ccade,0x07e95158},
        .iir_coef[2].coef_a = {0x08000000,0xf02ccade,0x07d50cfb},
        .iir_coef[3].coef_b = {0x00e4a464,0xfe826759,0x00aa5c36},
        .iir_coef[3].coef_a = {0x08000000,0xf0c9b5b4,0x0747b240},
        .iir_coef[4].coef_b = {0x0801f8e3,0xf00fce81,0x07eecb3e},
        .iir_coef[4].coef_a = {0x08000000,0xf00fce81,0x07f0c422},
        .iir_coef[5].coef_b = {0x0796603b,0xf0f8ff86,0x078425e1},
        .iir_coef[5].coef_a = {0x08000000,0xf0f8ff86,0x071a861c},
        .iir_coef[6].coef_b = {0x080a2e10,0xf017daf8,0x07e56245},
        .iir_coef[6].coef_a = {0x08000000,0xf017daf8,0x07ef9055},
        .iir_coef[7].coef_b = {0x075b0662,0xf1e3c6e0,0x07365d23},
        .iir_coef[7].coef_a = {0x08000000,0xf1e3c6e0,0x06916385},
        .iir_coef[8].coef_b = {0,0,0},
        .iir_coef[8].coef_a = {0,0,0},
        .iir_coef[9].coef_b = {0,0,0},
        .iir_coef[9].coef_a = {0,0,0},
        .iir_coef[10].coef_b = {0,0,0},
        .iir_coef[10].coef_a = {0,0,0},
        .iir_coef[11].coef_b = {0,0,0},
        .iir_coef[11].coef_a = {0,0,0},
        .iir_coef[12].coef_b = {0,0,0},
        .iir_coef[12].coef_a = {0,0,0},
        .limiter_flag = 0,
        .limiter_reserved = 0,
        .limiter_th = 0.0,
        .limiter_att_time = 0.0,
        .limiter_rls_time = 0.0,
        .dac_gain_offset = 0,
        .adc_gain_offset = -24,
    },
    .anc_cfg_ff_r = {
        .total_gain = 0,
        .iir_bypass_flag = 0,
        .iir_counter = 1,
        .iir_coef[0].coef_b = {0x08000000,0xf022991b,0x07ddf8db},
        .iir_coef[0].coef_a = {0x08000000,0xf022991b,0x07ddf8db},
        .iir_coef[1].coef_b = {0,0,0},
        .iir_coef[1].coef_a = {0,0,0},
        .iir_coef[2].coef_b = {0,0,0},
        .iir_coef[2].coef_a = {0,0,0},
        .iir_coef[3].coef_b = {0,0,0},
        .iir_coef[3].coef_a = {0,0,0},
        .iir_coef[4].coef_b = {0,0,0},
        .iir_coef[4].coef_a = {0,0,0},
        .iir_coef[5].coef_b = {0,0,0},
        .iir_coef[5].coef_a = {0,0,0},
        .iir_coef[6].coef_b = {0,0,0},
        .iir_coef[6].coef_a = {0,0,0},
        .iir_coef[7].coef_b = {0,0,0},
        .iir_coef[7].coef_a = {0,0,0},
        .iir_coef[8].coef_b = {0,0,0},
        .iir_coef[8].coef_a = {0,0,0},
        .iir_coef[9].coef_b = {0,0,0},
        .iir_coef[9].coef_a = {0,0,0},
        .iir_coef[10].coef_b = {0,0,0},
        .iir_coef[10].coef_a = {0,0,0},
        .iir_coef[11].coef_b = {0,0,0},
        .iir_coef[11].coef_a = {0,0,0},
        .iir_coef[12].coef_b = {0,0,0},
        .iir_coef[12].coef_a = {0,0,0},
        .limiter_flag = 0,
        .limiter_reserved = 0,
        .limiter_th = 0.0,
        .limiter_att_time = 0.0,
        .limiter_rls_time = 0.0,
        .dac_gain_offset = 0,
        .adc_gain_offset = -24,
    },
    .anc_cfg_fb_l = {
        .total_gain = 0,
        .iir_bypass_flag = 0,
        .iir_counter = 1,
        .iir_coef[0].coef_b = {0x08000000,0xf022991b,0x07ddf8db},
        .iir_coef[0].coef_a = {0x08000000,0xf022991b,0x07ddf8db},
        .iir_coef[1].coef_b = {0,0,0},
        .iir_coef[1].coef_a = {0,0,0},
        .iir_coef[2].coef_b = {0,0,0},
        .iir_coef[2].coef_a = {0,0,0},
        .iir_coef[3].coef_b = {0,0,0},
        .iir_coef[3].coef_a = {0,0,0},
        .iir_coef[4].coef_b = {0,0,0},
        .iir_coef[4].coef_a = {0,0,0},
        .iir_coef[5].coef_b = {0,0,0},
        .iir_coef[5].coef_a = {0,0,0},
        .iir_coef[6].coef_b = {0,0,0},
        .iir_coef[6].coef_a = {0,0,0},
        .iir_coef[7].coef_b = {0,0,0},
        .iir_coef[7].coef_a = {0,0,0},
        .iir_coef[8].coef_b = {0,0,0},
        .iir_coef[8].coef_a = {0,0,0},
        .iir_coef[9].coef_b = {0,0,0},
        .iir_coef[9].coef_a = {0,0,0},
        .iir_coef[10].coef_b = {0,0,0},
        .iir_coef[10].coef_a = {0,0,0},
        .iir_coef[11].coef_b = {0,0,0},
        .iir_coef[11].coef_a = {0,0,0},
        .iir_coef[12].coef_b = {0,0,0},
        .iir_coef[12].coef_a = {0,0,0},
        .limiter_flag = 0,
        .limiter_reserved = 0,
        .limiter_th = 0.0,
        .limiter_att_time = 0.0,
        .limiter_rls_time = 0.0,
        .dac_gain_offset = 0,
        .adc_gain_offset = -24,
    },
    .anc_cfg_fb_r = {
        .total_gain = 0,
        .iir_bypass_flag = 0,
        .iir_counter = 1,
        .iir_coef[0].coef_b = {0x08000000,0xf022991b,0x07ddf8db},
        .iir_coef[0].coef_a = {0x08000000,0xf022991b,0x07ddf8db},
        .iir_coef[1].coef_b = {0,0,0},
        .iir_coef[1].coef_a = {0,0,0},
        .iir_coef[2].coef_b = {0,0,0},
        .iir_coef[2].coef_a = {0,0,0},
        .iir_coef[3].coef_b = {0,0,0},
        .iir_coef[3].coef_a = {0,0,0},
        .iir_coef[4].coef_b = {0,0,0},
        .iir_coef[4].coef_a = {0,0,0},
        .iir_coef[5].coef_b = {0,0,0},
        .iir_coef[5].coef_a = {0,0,0},
        .iir_coef[6].coef_b = {0,0,0},
        .iir_coef[6].coef_a = {0,0,0},
        .iir_coef[7].coef_b = {0,0,0},
        .iir_coef[7].coef_a = {0,0,0},
        .iir_coef[8].coef_b = {0,0,0},
        .iir_coef[8].coef_a = {0,0,0},
        .iir_coef[9].coef_b = {0,0,0},
        .iir_coef[9].coef_a = {0,0,0},
        .iir_coef[10].coef_b = {0,0,0},
        .iir_coef[10].coef_a = {0,0,0},
        .iir_coef[11].coef_b = {0,0,0},
        .iir_coef[11].coef_a = {0,0,0},
        .iir_coef[12].coef_b = {0,0,0},
        .iir_coef[12].coef_a = {0,0,0},
        .limiter_flag = 0,
        .limiter_reserved = 0,
        .limiter_th = 0.0,
        .limiter_att_time = 0.0,
        .limiter_rls_time = 0.0,
        .dac_gain_offset = 0,
        .adc_gain_offset = -24,
    },
    .anc_cfg_mc_l = {
        .total_gain = 0,
        .iir_bypass_flag = 0,
        .iir_counter = 1,
        .iir_coef[0].coef_b = {0x08000000,0xf022991b,0x07ddf8db},
        .iir_coef[0].coef_a = {0x08000000,0xf022991b,0x07ddf8db},
        .iir_coef[1].coef_b = {0,0,0},
        .iir_coef[1].coef_a = {0,0,0},
        .iir_coef[2].coef_b = {0,0,0},
        .iir_coef[2].coef_a = {0,0,0},
        .iir_coef[3].coef_b = {0,0,0},
        .iir_coef[3].coef_a = {0,0,0},
        .iir_coef[4].coef_b = {0,0,0},
        .iir_coef[4].coef_a = {0,0,0},
        .iir_coef[5].coef_b = {0,0,0},
        .iir_coef[5].coef_a = {0,0,0},
        .iir_coef[6].coef_b = {0,0,0},
        .iir_coef[6].coef_a = {0,0,0},
        .iir_coef[7].coef_b = {0,0,0},
        .iir_coef[7].coef_a = {0,0,0},
        .iir_coef[8].coef_b = {0,0,0},
        .iir_coef[8].coef_a = {0,0,0},
        .iir_coef[9].coef_b = {0,0,0},
        .iir_coef[9].coef_a = {0,0,0},
        .iir_coef[10].coef_b = {0,0,0},
        .iir_coef[10].coef_a = {0,0,0},
        .iir_coef[11].coef_b = {0,0,0},
        .iir_coef[11].coef_a = {0,0,0},
        .iir_coef[12].coef_b = {0,0,0},
        .iir_coef[12].coef_a = {0,0,0},
        .limiter_flag = 0,
        .limiter_reserved = 0,
        .limiter_th = 0.0,
        .limiter_att_time = 0.0,
        .limiter_rls_time = 0.0,
        .dac_gain_offset = 0,
        .adc_gain_offset = 0,
    },
    .anc_cfg_mc_r = {
        .total_gain = 0,
        .iir_bypass_flag = 0,
        .iir_counter = 1,
        .iir_coef[0].coef_b = {0x08000000,0xf022991b,0x07ddf8db},
        .iir_coef[0].coef_a = {0x08000000,0xf022991b,0x07ddf8db},
        .iir_coef[1].coef_b = {0,0,0},
        .iir_coef[1].coef_a = {0,0,0},
        .iir_coef[2].coef_b = {0,0,0},
        .iir_coef[2].coef_a = {0,0,0},
        .iir_coef[3].coef_b = {0,0,0},
        .iir_coef[3].coef_a = {0,0,0},
        .iir_coef[4].coef_b = {0,0,0},
        .iir_coef[4].coef_a = {0,0,0},
        .iir_coef[5].coef_b = {0,0,0},
        .iir_coef[5].coef_a = {0,0,0},
        .iir_coef[6].coef_b = {0,0,0},
        .iir_coef[6].coef_a = {0,0,0},
        .iir_coef[7].coef_b = {0,0,0},
        .iir_coef[7].coef_a = {0,0,0},
        .iir_coef[8].coef_b = {0,0,0},
        .iir_coef[8].coef_a = {0,0,0},
        .iir_coef[9].coef_b = {0,0,0},
        .iir_coef[9].coef_a = {0,0,0},
        .iir_coef[10].coef_b = {0,0,0},
        .iir_coef[10].coef_a = {0,0,0},
        .iir_coef[11].coef_b = {0,0,0},
        .iir_coef[11].coef_a = {0,0,0},
        .iir_coef[12].coef_b = {0,0,0},
        .iir_coef[12].coef_a = {0,0,0},
        .limiter_flag = 0,
        .limiter_reserved = 0,
        .limiter_th = 0.0,
        .limiter_att_time = 0.0,
        .limiter_rls_time = 0.0,
        .dac_gain_offset = 0,
        .adc_gain_offset = 0,
    },
};

static const struct_anc_cfg POSSIBLY_UNUSED AncFirCoef_50p7k_mode1 = {
    .anc_cfg_ff_l = {
        .total_gain = 512,
        .iir_bypass_flag = 0,
        .iir_counter = 5,
        .iir_coef[0].coef_b = {0x0804fdb2,0xf00cf810,0x07ee2eec},
        .iir_coef[0].coef_a = {0x08000000,0xf00cf810,0x07f32c9e},
        .iir_coef[1].coef_b = {0x080d09bd,0xf0119d21,0x07e1b6f0},
        .iir_coef[1].coef_a = {0x08000000,0xf0119d21,0x07eec0ac},
        .iir_coef[2].coef_b = {0x08229490,0xf01c6d3a,0x07c2472b},
        .iir_coef[2].coef_a = {0x08000000,0xf01c6d3a,0x07e4dbbb},
        .iir_coef[3].coef_b = {0x0844af17,0xf03b03ba,0x07856826},
        .iir_coef[3].coef_a = {0x08000000,0xf03b03ba,0x07ca173e},
        .iir_coef[4].coef_b = {0x02969389,0xfb188590,0x02574526},
        .iir_coef[4].coef_a = {0x08000000,0xf076f1ba,0x078f6c85},
        .iir_coef[5].coef_b = {0,0,0},
        .iir_coef[5].coef_a = {0,0,0},
        .iir_coef[6].coef_b = {0,0,0},
        .iir_coef[6].coef_a = {0,0,0},
        .iir_coef[7].coef_b = {0,0,0},
        .iir_coef[7].coef_a = {0,0,0},
        .iir_coef[8].coef_b = {0,0,0},
        .iir_coef[8].coef_a = {0,0,0},
        .iir_coef[9].coef_b = {0,0,0},
        .iir_coef[9].coef_a = {0,0,0},
        .iir_coef[10].coef_b = {0,0,0},
        .iir_coef[10].coef_a = {0,0,0},
        .iir_coef[11].coef_b = {0,0,0},
        .iir_coef[11].coef_a = {0,0,0},
        .iir_coef[12].coef_b = {0,0,0},
        .iir_coef[12].coef_a = {0,0,0},
        .limiter_flag = 0,
        .limiter_reserved = 0,
        .limiter_th = 0.0,
        .limiter_att_time = 0.0,
        .limiter_rls_time = 0.0,
        .dac_gain_offset = 0,
        .adc_gain_offset = -24,
    },
    .anc_cfg_ff_r = {
        .total_gain = 0,
        .iir_bypass_flag = 0,
        .iir_counter = 1,
        .iir_coef[0].coef_b = {0x08000000,0xf022991b,0x07ddf8db},
        .iir_coef[0].coef_a = {0x08000000,0xf022991b,0x07ddf8db},
        .iir_coef[1].coef_b = {0,0,0},
        .iir_coef[1].coef_a = {0,0,0},
        .iir_coef[2].coef_b = {0,0,0},
        .iir_coef[2].coef_a = {0,0,0},
        .iir_coef[3].coef_b = {0,0,0},
        .iir_coef[3].coef_a = {0,0,0},
        .iir_coef[4].coef_b = {0,0,0},
        .iir_coef[4].coef_a = {0,0,0},
        .iir_coef[5].coef_b = {0,0,0},
        .iir_coef[5].coef_a = {0,0,0},
        .iir_coef[6].coef_b = {0,0,0},
        .iir_coef[6].coef_a = {0,0,0},
        .iir_coef[7].coef_b = {0,0,0},
        .iir_coef[7].coef_a = {0,0,0},
        .iir_coef[8].coef_b = {0,0,0},
        .iir_coef[8].coef_a = {0,0,0},
        .iir_coef[9].coef_b = {0,0,0},
        .iir_coef[9].coef_a = {0,0,0},
        .iir_coef[10].coef_b = {0,0,0},
        .iir_coef[10].coef_a = {0,0,0},
        .iir_coef[11].coef_b = {0,0,0},
        .iir_coef[11].coef_a = {0,0,0},
        .iir_coef[12].coef_b = {0,0,0},
        .iir_coef[12].coef_a = {0,0,0},
        .limiter_flag = 0,
        .limiter_reserved = 0,
        .limiter_th = 0.0,
        .limiter_att_time = 0.0,
        .limiter_rls_time = 0.0,
        .dac_gain_offset = 0,
        .adc_gain_offset = -24,
    },
    .anc_cfg_fb_l = {
        .total_gain = 0,
        .iir_bypass_flag = 0,
        .iir_counter = 1,
        .iir_coef[0].coef_b = {0x08000000,0xf022991b,0x07ddf8db},
        .iir_coef[0].coef_a = {0x08000000,0xf022991b,0x07ddf8db},
        .iir_coef[1].coef_b = {0,0,0},
        .iir_coef[1].coef_a = {0,0,0},
        .iir_coef[2].coef_b = {0,0,0},
        .iir_coef[2].coef_a = {0,0,0},
        .iir_coef[3].coef_b = {0,0,0},
        .iir_coef[3].coef_a = {0,0,0},
        .iir_coef[4].coef_b = {0,0,0},
        .iir_coef[4].coef_a = {0,0,0},
        .iir_coef[5].coef_b = {0,0,0},
        .iir_coef[5].coef_a = {0,0,0},
        .iir_coef[6].coef_b = {0,0,0},
        .iir_coef[6].coef_a = {0,0,0},
        .iir_coef[7].coef_b = {0,0,0},
        .iir_coef[7].coef_a = {0,0,0},
        .iir_coef[8].coef_b = {0,0,0},
        .iir_coef[8].coef_a = {0,0,0},
        .iir_coef[9].coef_b = {0,0,0},
        .iir_coef[9].coef_a = {0,0,0},
        .iir_coef[10].coef_b = {0,0,0},
        .iir_coef[10].coef_a = {0,0,0},
        .iir_coef[11].coef_b = {0,0,0},
        .iir_coef[11].coef_a = {0,0,0},
        .iir_coef[12].coef_b = {0,0,0},
        .iir_coef[12].coef_a = {0,0,0},
        .limiter_flag = 0,
        .limiter_reserved = 0,
        .limiter_th = 0.0,
        .limiter_att_time = 0.0,
        .limiter_rls_time = 0.0,
        .dac_gain_offset = 0,
        .adc_gain_offset = -24,
    },
    .anc_cfg_fb_r = {
        .total_gain = 0,
        .iir_bypass_flag = 0,
        .iir_counter = 1,
        .iir_coef[0].coef_b = {0x08000000,0xf022991b,0x07ddf8db},
        .iir_coef[0].coef_a = {0x08000000,0xf022991b,0x07ddf8db},
        .iir_coef[1].coef_b = {0,0,0},
        .iir_coef[1].coef_a = {0,0,0},
        .iir_coef[2].coef_b = {0,0,0},
        .iir_coef[2].coef_a = {0,0,0},
        .iir_coef[3].coef_b = {0,0,0},
        .iir_coef[3].coef_a = {0,0,0},
        .iir_coef[4].coef_b = {0,0,0},
        .iir_coef[4].coef_a = {0,0,0},
        .iir_coef[5].coef_b = {0,0,0},
        .iir_coef[5].coef_a = {0,0,0},
        .iir_coef[6].coef_b = {0,0,0},
        .iir_coef[6].coef_a = {0,0,0},
        .iir_coef[7].coef_b = {0,0,0},
        .iir_coef[7].coef_a = {0,0,0},
        .iir_coef[8].coef_b = {0,0,0},
        .iir_coef[8].coef_a = {0,0,0},
        .iir_coef[9].coef_b = {0,0,0},
        .iir_coef[9].coef_a = {0,0,0},
        .iir_coef[10].coef_b = {0,0,0},
        .iir_coef[10].coef_a = {0,0,0},
        .iir_coef[11].coef_b = {0,0,0},
        .iir_coef[11].coef_a = {0,0,0},
        .iir_coef[12].coef_b = {0,0,0},
        .iir_coef[12].coef_a = {0,0,0},
        .limiter_flag = 0,
        .limiter_reserved = 0,
        .limiter_th = 0.0,
        .limiter_att_time = 0.0,
        .limiter_rls_time = 0.0,
        .dac_gain_offset = 0,
        .adc_gain_offset = -24,
    },
    .anc_cfg_mc_l = {
        .total_gain = 0,
        .iir_bypass_flag = 0,
        .iir_counter = 1,
        .iir_coef[0].coef_b = {0x08000000,0xf022991b,0x07ddf8db},
        .iir_coef[0].coef_a = {0x08000000,0xf022991b,0x07ddf8db},
        .iir_coef[1].coef_b = {0,0,0},
        .iir_coef[1].coef_a = {0,0,0},
        .iir_coef[2].coef_b = {0,0,0},
        .iir_coef[2].coef_a = {0,0,0},
        .iir_coef[3].coef_b = {0,0,0},
        .iir_coef[3].coef_a = {0,0,0},
        .iir_coef[4].coef_b = {0,0,0},
        .iir_coef[4].coef_a = {0,0,0},
        .iir_coef[5].coef_b = {0,0,0},
        .iir_coef[5].coef_a = {0,0,0},
        .iir_coef[6].coef_b = {0,0,0},
        .iir_coef[6].coef_a = {0,0,0},
        .iir_coef[7].coef_b = {0,0,0},
        .iir_coef[7].coef_a = {0,0,0},
        .iir_coef[8].coef_b = {0,0,0},
        .iir_coef[8].coef_a = {0,0,0},
        .iir_coef[9].coef_b = {0,0,0},
        .iir_coef[9].coef_a = {0,0,0},
        .iir_coef[10].coef_b = {0,0,0},
        .iir_coef[10].coef_a = {0,0,0},
        .iir_coef[11].coef_b = {0,0,0},
        .iir_coef[11].coef_a = {0,0,0},
        .iir_coef[12].coef_b = {0,0,0},
        .iir_coef[12].coef_a = {0,0,0},
        .limiter_flag = 0,
        .limiter_reserved = 0,
        .limiter_th = 0.0,
        .limiter_att_time = 0.0,
        .limiter_rls_time = 0.0,
        .dac_gain_offset = 0,
        .adc_gain_offset = 0,
    },
    .anc_cfg_mc_r = {
        .total_gain = 0,
        .iir_bypass_flag = 0,
        .iir_counter = 1,
        .iir_coef[0].coef_b = {0x08000000,0xf022991b,0x07ddf8db},
        .iir_coef[0].coef_a = {0x08000000,0xf022991b,0x07ddf8db},
        .iir_coef[1].coef_b = {0,0,0},
        .iir_coef[1].coef_a = {0,0,0},
        .iir_coef[2].coef_b = {0,0,0},
        .iir_coef[2].coef_a = {0,0,0},
        .iir_coef[3].coef_b = {0,0,0},
        .iir_coef[3].coef_a = {0,0,0},
        .iir_coef[4].coef_b = {0,0,0},
        .iir_coef[4].coef_a = {0,0,0},
        .iir_coef[5].coef_b = {0,0,0},
        .iir_coef[5].coef_a = {0,0,0},
        .iir_coef[6].coef_b = {0,0,0},
        .iir_coef[6].coef_a = {0,0,0},
        .iir_coef[7].coef_b = {0,0,0},
        .iir_coef[7].coef_a = {0,0,0},
        .iir_coef[8].coef_b = {0,0,0},
        .iir_coef[8].coef_a = {0,0,0},
        .iir_coef[9].coef_b = {0,0,0},
        .iir_coef[9].coef_a = {0,0,0},
        .iir_coef[10].coef_b = {0,0,0},
        .iir_coef[10].coef_a = {0,0,0},
        .iir_coef[11].coef_b = {0,0,0},
        .iir_coef[11].coef_a = {0,0,0},
        .iir_coef[12].coef_b = {0,0,0},
        .iir_coef[12].coef_a = {0,0,0},
        .limiter_flag = 0,
        .limiter_reserved = 0,
        .limiter_th = 0.0,
        .limiter_att_time = 0.0,
        .limiter_rls_time = 0.0,
        .dac_gain_offset = 0,
        .adc_gain_offset = 0,
    },
};

static const struct_anc_cfg POSSIBLY_UNUSED AncFirCoef_48k_mode0 = {
    .anc_cfg_ff_l = {
        .total_gain = 512*2,

        .iir_bypass_flag=0,
        .iir_counter=IIR_COUNTER_FF_L,

        .iir_coef[0].coef_b={0x8000000,0,0},
        .iir_coef[0].coef_a={0x8000000,0,0},

        .iir_coef[1].coef_b={0x8000000,0,0},
        .iir_coef[1].coef_a={0x8000000,0,0},

        .iir_coef[2].coef_b={0x8000000,0,0},
        .iir_coef[2].coef_a={0x8000000,0,0},

        .iir_coef[3].coef_b={0x8000000,0,0},
        .iir_coef[3].coef_a={0x8000000,0,0},

        .iir_coef[4].coef_b={0x8000000,0,0},
        .iir_coef[4].coef_a={0x8000000,0,0},

        .iir_coef[5].coef_b={0x8000000,0,0},
        .iir_coef[5].coef_a={0x8000000,0,0},

/*        .fir_bypass_flag=1,
        .fir_len = AUD_COEF_LEN,
        .fir_coef =
        {
            32767,
        },
*/
        .dac_gain_offset=0,
        .adc_gain_offset=(0)*4,
    },
};

static const struct_anc_cfg POSSIBLY_UNUSED AncFirCoef_48k_mode1 = {
    .anc_cfg_ff_l = {
        .total_gain = 512*2,

        .iir_bypass_flag=0,
        .iir_counter=IIR_COUNTER_FF_L,

        .iir_coef[0].coef_b={0x8000000,0,0},
        .iir_coef[0].coef_a={0x8000000,0,0},

        .iir_coef[1].coef_b={0x8000000,0,0},
        .iir_coef[1].coef_a={0x8000000,0,0},

        .iir_coef[2].coef_b={0x8000000,0,0},
        .iir_coef[2].coef_a={0x8000000,0,0},

        .iir_coef[3].coef_b={0x8000000,0,0},
        .iir_coef[3].coef_a={0x8000000,0,0},

        .iir_coef[4].coef_b={0x8000000,0,0},
        .iir_coef[4].coef_a={0x8000000,0,0},

        .iir_coef[5].coef_b={0x8000000,0,0},
        .iir_coef[5].coef_a={0x8000000,0,0},

/*        .fir_bypass_flag=1,
        .fir_len = AUD_COEF_LEN,
        .fir_coef =
        {
            32767,
        },
*/
        .dac_gain_offset=0,
        .adc_gain_offset=(0)*4,
    },
};

static const struct_anc_cfg POSSIBLY_UNUSED AncFirCoef_44p1k_mode0 = {
    .anc_cfg_ff_l = {
        .total_gain =512*2,

        .iir_bypass_flag=0,
        .iir_counter=IIR_COUNTER_FF_L,

        .iir_coef[0].coef_b={0x8000000,0,0},
        .iir_coef[0].coef_a={0x8000000,0,0},

        .iir_coef[1].coef_b={0x8000000,0,0},
        .iir_coef[1].coef_a={0x8000000,0,0},

        .iir_coef[2].coef_b={0x8000000,0,0},
        .iir_coef[2].coef_a={0x8000000,0,0},

        .iir_coef[3].coef_b={0x8000000,0,0},
        .iir_coef[3].coef_a={0x8000000,0,0},

        .iir_coef[4].coef_b={0x8000000,0,0},
        .iir_coef[4].coef_a={0x8000000,0,0},

        .iir_coef[5].coef_b={0x8000000,0,0},
        .iir_coef[5].coef_a={0x8000000,0,0},

/*        .fir_bypass_flag=1,
        .fir_len = AUD_COEF_LEN,
        .fir_coef =
        {
            32767,
        },
*/
        .dac_gain_offset=0,
        .adc_gain_offset=(0)*4,
    },
};
static const struct_anc_cfg POSSIBLY_UNUSED AncFirCoef_44p1k_mode1 = {
    .anc_cfg_ff_l = {
        .total_gain =512*2,

        .iir_bypass_flag=0,
        .iir_counter=IIR_COUNTER_FF_L,

        .iir_coef[0].coef_b={0x8000000,0,0},
        .iir_coef[0].coef_a={0x8000000,0,0},

        .iir_coef[1].coef_b={0x8000000,0,0},
        .iir_coef[1].coef_a={0x8000000,0,0},

        .iir_coef[2].coef_b={0x8000000,0,0},
        .iir_coef[2].coef_a={0x8000000,0,0},

        .iir_coef[3].coef_b={0x8000000,0,0},
        .iir_coef[3].coef_a={0x8000000,0,0},

        .iir_coef[4].coef_b={0x8000000,0,0},
        .iir_coef[4].coef_a={0x8000000,0,0},

        .iir_coef[5].coef_b={0x8000000,0,0},
        .iir_coef[5].coef_a={0x8000000,0,0},

/*        .fir_bypass_flag=1,
        .fir_len = AUD_COEF_LEN,
        .fir_coef =
        {
            32767,
        },
*/
        .dac_gain_offset=0,
        .adc_gain_offset=(0)*4,
    },
};

#endif

//hardware dac iir eq
const IIR_CFG_T audio_eq_hw_dac_iir_cfg = { //default basic EQ
#if 1
    .gain0 = 4.5,
    .gain1 = 4.5,
    .num = 10,
    .param = {
        {IIR_TYPE_PEAK, -3.7, 32, 0.9},
        {IIR_TYPE_PEAK, 6.8, 51, 0.75},
        {IIR_TYPE_PEAK, -2, 99, 0.45},
        {IIR_TYPE_PEAK, -15.4, 265, 0.25},
        {IIR_TYPE_PEAK, -1.7, 830, 1},
        {IIR_TYPE_PEAK, 5, 1164, 0.7},
        {IIR_TYPE_PEAK, 2.8, 3573, 0.6},
        {IIR_TYPE_PEAK, 0, 15785, 0.45},
        {IIR_TYPE_PEAK, -3.8, 6811, 1.5},
        {IIR_TYPE_PEAK, -7, 1900, 0.7},
        /*{IIR_TYPE_LOW_SHELF, -4.5, 2800, 1.2},
        {IIR_TYPE_PEAK, 5, 75, 1.5},
        {IIR_TYPE_PEAK, -3.5, 21, 0.9},
        {IIR_TYPE_PEAK, -6.5, 160, 0.65},
        {IIR_TYPE_PEAK, -3.6, 500, 0.8},
        {IIR_TYPE_PEAK, -15, 5500, 2.3},
        {IIR_TYPE_PEAK, 6, 15000, 2.8},
        {IIR_TYPE_PEAK, 1.5, 8800, 2.5},
        {IIR_TYPE_PEAK, -2.0, 1200, 0.5},
        {IIR_TYPE_PEAK, 2.6, 3800, 2},*/
    }
#else
    .gain0 = 0,
    .gain1 = 0,
        .num = 0,
    .param = {

    }
    .num = 5,
    .param = {
        {IIR_TYPE_PEAK,      -3.2,   1200,      0.6},
   {IIR_TYPE_LOW_SHELF,      -1.8,   30.0,      0.6},
        {IIR_TYPE_PEAK,      -1.0,   100.0,     0.6},
        {IIR_TYPE_PEAK,      -3.7,   400.0,     0.5},
        {IIR_TYPE_PEAK,      -2.0,   200.0,     0.6},
    }
#endif
};

const IIR_CFG_T audio_eq_anc_hw_dac_iir_cfg = { //anc eq
#if 1
    .gain0 = -6,
    .gain1 = -6,
    .num = 8,
    .param = {
        {IIR_TYPE_LOW_SHELF, -4.5, 2800, 1.2},
        {IIR_TYPE_PEAK, 5.5, 75, 1.5},
        {IIR_TYPE_PEAK, -3.5, 21, 0.9},
        {IIR_TYPE_PEAK, -6.5, 160, 0.65},
        {IIR_TYPE_PEAK, -3.9, 500, 0.8},
        {IIR_TYPE_PEAK, -4, 4500, 1.6},
        {IIR_TYPE_PEAK, 6, 15000, 2.8},
        {IIR_TYPE_PEAK, 2, 8000, 2.5},
        /*{IIR_TYPE_LOW_SHELF, -4.5, 2800, 1.2},
        {IIR_TYPE_PEAK, 5, 75, 1.5},
        {IIR_TYPE_PEAK, -3.5, 21, 0.9},
        {IIR_TYPE_PEAK, -6.5, 160, 0.65},
        {IIR_TYPE_PEAK, -3.6, 500, 0.8},
        {IIR_TYPE_PEAK, -15, 5500, 2.3},
        {IIR_TYPE_PEAK, 6, 15000, 2.8},
        {IIR_TYPE_PEAK, 1.5, 8800, 2.5},
        {IIR_TYPE_PEAK, -2.0, 1200, 0.5},
        {IIR_TYPE_PEAK, 2.6, 3800, 2},*/
    }
#else
    .gain0 = 0,
    .gain1 = 0,
        .num = 0,
    .param = {

    }
    .num = 5,
    .param = {
        {IIR_TYPE_PEAK,      -3.2,   1200,      0.6},
   {IIR_TYPE_LOW_SHELF,      -1.8,   30.0,      0.6},
        {IIR_TYPE_PEAK,      -1.0,   100.0,     0.6},
        {IIR_TYPE_PEAK,      -3.7,   400.0,     0.5},
        {IIR_TYPE_PEAK,      -2.0,   200.0,     0.6},
    }
#endif
};


const IIR_CFG_T audio_eq_hw_dac_iir_cfg_1 = { //pop
    .gain0 = -7,
    .gain1 = -7,
    .num = 9,
    .param = {
        {IIR_TYPE_LOW_SHELF, -1.8, 2800, 1.2},
        {IIR_TYPE_PEAK, -1.8, 75, 0.8},
        {IIR_TYPE_PEAK, 1.5, 21, 0.9},
        {IIR_TYPE_PEAK, -9.5, 160, 0.65},
        {IIR_TYPE_PEAK, -8.9, 500, 0.8},
        {IIR_TYPE_PEAK, 1.2, 4500, 1.6},
        {IIR_TYPE_PEAK, 5, 15000, 2.8},
        {IIR_TYPE_PEAK, 5, 8000, 2.5},
        {IIR_TYPE_PEAK, 1.5, 950, 0.9},
    }
};

const IIR_CFG_T audio_eq_hw_dac_iir_cfg_2 = { //rock
    .gain0 = -7,
    .gain1 = -7,
    .num = 9,
    .param = {
        {IIR_TYPE_LOW_SHELF, -5, 2800, 1.2},
        {IIR_TYPE_PEAK, 2.5, 55, 1.0},
        {IIR_TYPE_PEAK, 2.9, 21, 0.9},
        {IIR_TYPE_PEAK, 5, 160, 0.65},
        {IIR_TYPE_PEAK, -2.9, 500, 1.8},
        {IIR_TYPE_PEAK, -8, 4500, 1.6},
        {IIR_TYPE_PEAK, 5, 15000, 2.8},
        {IIR_TYPE_PEAK, 5, 8000, 2.5},
        {IIR_TYPE_PEAK, 8, 1050, 0.9},
    }
};

const IIR_CFG_T audio_eq_hw_dac_iir_cfg_3 = { //jazz
    .gain0 = -7,
    .gain1 = -7,
    .num = 10,
    .param = {
        {IIR_TYPE_LOW_SHELF, -5, 2800, 1.2},
        {IIR_TYPE_PEAK, 4.0, 55, 0.8},
        {IIR_TYPE_PEAK, 2.5, 21, 0.9},
        {IIR_TYPE_PEAK, -0.8, 200, 1.35},
        {IIR_TYPE_PEAK, 4.2, 500, 0.8},
        {IIR_TYPE_PEAK, -8, 5200, 1.6},
        {IIR_TYPE_PEAK, 5, 15000, 2.8},
        {IIR_TYPE_PEAK, 0.2, 8000, 2.5},
        {IIR_TYPE_PEAK, 6.5, 1050, 0.6},
        {IIR_TYPE_PEAK, -2, 3500, 1.8},
    }
};

const IIR_CFG_T audio_eq_hw_dac_iir_cfg_4 = { //classic
    .gain0 = -7,
    .gain1 = -7,
    .num = 9,
    .param = {
        {IIR_TYPE_LOW_SHELF, -5, 2800, 1.2},
        {IIR_TYPE_PEAK, 3.5, 55, 0.8},
        {IIR_TYPE_PEAK, 3.5, 21, 0.9},
        {IIR_TYPE_PEAK, 1.0, 120, 1.35},
        {IIR_TYPE_PEAK, 2.3, 500, 0.8},
        {IIR_TYPE_PEAK, -4, 5200, 1.6},
        {IIR_TYPE_PEAK, 5, 15000, 2.8},
        {IIR_TYPE_PEAK, 0.2, 8000, 2.5},
        {IIR_TYPE_PEAK, 8.5, 1250, 0.6},
    }
};

const IIR_CFG_T audio_eq_hw_dac_iir_cfg_5 = { //country
    .gain0 = -7,
    .gain1 = -7,
    .num = 9,
    .param = {
        {IIR_TYPE_LOW_SHELF, -5, 2800, 1.2},
        {IIR_TYPE_PEAK, 1.9, 65, 1.0},
        {IIR_TYPE_PEAK, 2.9, 21, 0.9},
        {IIR_TYPE_PEAK, 2.2, 160, 0.65},
        {IIR_TYPE_PEAK, -1.9, 500, 0.8},
        {IIR_TYPE_PEAK, -1.2, 4500, 1.6},
        {IIR_TYPE_PEAK, 5, 15000, 2.8},
        {IIR_TYPE_PEAK, 0.5, 8000, 2.5},
        {IIR_TYPE_PEAK, 3, 1900, 0.9},
    }
};

#if (CFG_HW_PWL_NUM > 0)
const struct HAL_IOMUX_PIN_FUNCTION_MAP cfg_hw_pinmux_pwl[CFG_HW_PWL_NUM] = {

    {HAL_IOMUX_PIN_P0_2, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_MEM, HAL_IOMUX_PIN_PULLUP_ENABLE}, //red
    {HAL_IOMUX_PIN_P0_3, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_MEM, HAL_IOMUX_PIN_PULLUP_ENABLE},//blue
};
#endif
const struct HAL_IOMUX_PIN_FUNCTION_MAP app_battery_ext_charger_detecter_cfg = {
    HAL_IOMUX_PIN_NUM, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_MEM, HAL_IOMUX_PIN_PULLUP_ENABLE
};
const struct HAL_IOMUX_PIN_FUNCTION_MAP app_fixed_left_right_side_cfg = {
    HAL_IOMUX_PIN_P0_1, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_MEM, HAL_IOMUX_PIN_PULLUP_ENABLE
};
const struct HAL_IOMUX_PIN_FUNCTION_MAP app_putin_putout_box_cfg = {
    HAL_IOMUX_PIN_NUM, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_MEM, HAL_IOMUX_PIN_PULLUP_ENABLE
};

const struct HAL_IOMUX_PIN_FUNCTION_MAP app_shipmode_gpio_cfg = {
    HAL_IOMUX_PIN_NUM, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_MEM, HAL_IOMUX_PIN_PULLUP_ENABLE
};
#endif


