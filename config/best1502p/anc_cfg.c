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
#if !defined(BESUI_TWS_EN) && !defined(BESUI_STEREO_EN)
#define IIR_COUNTER_FF_L (6)
#define IIR_COUNTER_FF_R (6)
#define IIR_COUNTER_FB_L (5)
#define IIR_COUNTER_FB_R (5)

static const struct_anc_cfg POSSIBLY_UNUSED AncFirCoef_50p7k_mode0 = {
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
        // .iir_coef[2].coef_b={133132833, -265460650, 132663529},//3k
        // .iir_coef[2].coef_a={134217728, -265460650, 131578634},

        .iir_coef[3].coef_b={0x8000000,0,0},
        .iir_coef[3].coef_a={0x8000000,0,0},

        .iir_coef[4].coef_b={0x8000000,0,0},
        .iir_coef[4].coef_a={0x8000000,0,0},

        .iir_coef[5].coef_b={0x8000000,0,0},
        .iir_coef[5].coef_a={0x8000000,0,0},

        .dac_gain_offset=0,
        .adc_gain_offset=(0)*4,
    },
    .anc_cfg_ff_r = {
        .total_gain = 512*2,

        .iir_bypass_flag=0,
        .iir_counter=IIR_COUNTER_FF_R,

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

        .dac_gain_offset=0,
        .adc_gain_offset=(0)*4,
    },
    .anc_cfg_fb_l = {
        .total_gain = 512/2,

        .iir_bypass_flag=0,
        .iir_counter=IIR_COUNTER_FB_L,

        .iir_coef[0].coef_b={0x8000000,0,0},
        .iir_coef[0].coef_a={0x8000000,0,0},

        .iir_coef[1].coef_b={0x8000000,0,0},
        .iir_coef[1].coef_a={0x8000000,0,0},

        .iir_coef[2].coef_b={0x8000000,0,0},
        .iir_coef[2].coef_a={0x8000000,0,0},
        // .iir_coef[2].coef_b={133853574, -267512069, 133696049},//1k
        // .iir_coef[2].coef_a={134217728, -267512069, 133331895},

        .iir_coef[3].coef_b={0x8000000,0,0},
        .iir_coef[3].coef_a={0x8000000,0,0},

        .iir_coef[4].coef_b={0x8000000,0,0},
        .iir_coef[4].coef_a={0x8000000,0,0},

        .iir_coef[5].coef_b={0x8000000,0,0},
        .iir_coef[5].coef_a={0x8000000,0,0},

        .dac_gain_offset=0,
        .adc_gain_offset=(0)*4,
    },
    .anc_cfg_fb_r = {
        .total_gain = 512/2,

        .iir_bypass_flag=0,
        .iir_counter=IIR_COUNTER_FB_L,

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

        .dac_gain_offset=0,
        .adc_gain_offset=(0)*4,
    },
    .anc_cfg_tt_l = {
        .total_gain = 512/2,

        .iir_bypass_flag=0,
        .iir_counter=IIR_COUNTER_FF_L,

        .iir_coef[0].coef_b={0x8000000,0,0},
        .iir_coef[0].coef_a={0x8000000,0,0},

        .iir_coef[1].coef_b={0x8000000,0,0},
        .iir_coef[1].coef_a={0x8000000,0,0},

        .iir_coef[2].coef_b={0x8000000,0,0},
        .iir_coef[2].coef_a={0x8000000,0,0},
        // .iir_coef[2].coef_b={130003803,    -257605108,    128180940}, //4k test code
        // .iir_coef[2].coef_a={134217728,    -257605108,    123967016},

        .iir_coef[3].coef_b={0x8000000,0,0},
        .iir_coef[3].coef_a={0x8000000,0,0},

        .iir_coef[4].coef_b={0x8000000,0,0},
        .iir_coef[4].coef_a={0x8000000,0,0},

        .iir_coef[5].coef_b={0x8000000,0,0},
        .iir_coef[5].coef_a={0x8000000,0,0},

        .dac_gain_offset=0,
        .adc_gain_offset=(0)*4,
    },
    .anc_cfg_tt_r = {
        .total_gain = 512/2,

        .iir_bypass_flag=0,
        .iir_counter=IIR_COUNTER_FF_R,

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

        .dac_gain_offset=0,
        .adc_gain_offset=(0)*4,
    },
    .anc_cfg_mc_l = {
        .total_gain = 512/2,

        .iir_bypass_flag=0,
        .iir_counter=IIR_COUNTER_FB_L,

        .iir_coef[0].coef_b={0x8000000,0,0},
        .iir_coef[0].coef_a={0x8000000,0,0},

        .iir_coef[1].coef_b={0x8000000,0,0},
        .iir_coef[1].coef_a={0x8000000,0,0},

        .iir_coef[2].coef_b={0x8000000,0,0},
        .iir_coef[2].coef_a={0x8000000,0,0},
        // .iir_coef[2].coef_b={131723630,    -260565855,    130644731},  //7K
        // .iir_coef[2].coef_a={134217728,    -260565855,    128150634},

        .iir_coef[3].coef_b={0x8000000,0,0},
        .iir_coef[3].coef_a={0x8000000,0,0},

        .iir_coef[4].coef_b={0x8000000,0,0},
        .iir_coef[4].coef_a={0x8000000,0,0},

        .iir_coef[5].coef_b={0x8000000,0,0},
        .iir_coef[5].coef_a={0x8000000,0,0},

        .dac_gain_offset=0,
        .adc_gain_offset=(0)*4,
    },
    .anc_cfg_mc_r = {
        .total_gain = 512/2,

        .iir_bypass_flag=0,
        .iir_counter=IIR_COUNTER_FB_L,

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

        .dac_gain_offset=0,
        .adc_gain_offset=(0)*4,
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
    .anc_cfg_fb_l = {
        .total_gain = 512/2,

        .iir_bypass_flag=0,
        .iir_counter=IIR_COUNTER_FB_L,

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
    .anc_cfg_mc_l = {
        .total_gain = 512*2,

        .iir_bypass_flag=0,
        .iir_counter=5,

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
    .anc_cfg_fb_l = {
        .total_gain = 512/2,

        .iir_bypass_flag=0,
        .iir_counter=IIR_COUNTER_FB_L,

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
    .anc_cfg_mc_l = {
        .total_gain = 512,

        .iir_bypass_flag=0,
        .iir_counter=5,

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

        .dac_gain_offset=0,
        .adc_gain_offset=(0)*4,
    },
};
#endif //#if !defined(BESUI_TWS_EN) && !defined(BESUI_STEREO_EN)

const struct_anc_cfg * anc_coef_list_50p7k[ANC_COEF_LIST_NUM] = {
    &AncFirCoef_50p7k_mode0,
#if defined(BESUI_TWS_EN) || defined(BESUI_STEREO_EN)
    &AncFirCoef_50p7k_mode1,
#else
    &AncFirCoef_50p7k_mode0,
    &AncFirCoef_50p7k_mode0,
    &AncFirCoef_50p7k_mode0,
    &AncFirCoef_50p7k_mode0,
    &AncFirCoef_50p7k_mode0,
    &AncFirCoef_50p7k_mode0,
    &AncFirCoef_50p7k_mode0,
    &AncFirCoef_50p7k_mode0,
    &AncFirCoef_50p7k_mode0,
#endif
};

const struct_anc_cfg * anc_coef_list_48k[ANC_COEF_LIST_NUM] = {
    &AncFirCoef_48k_mode0,
#if defined(BESUI_TWS_EN) || defined(BESUI_STEREO_EN)
    &AncFirCoef_48k_mode1,
#endif
};

const struct_anc_cfg * anc_coef_list_44p1k[ANC_COEF_LIST_NUM] = {
    &AncFirCoef_44p1k_mode0,
#if defined(BESUI_TWS_EN) || defined(BESUI_STEREO_EN)
    &AncFirCoef_44p1k_mode1,
#endif
};

static const struct_psap_cfg POSSIBLY_UNUSED hwlimiter_para_44p1k_mode0 = {
    .psap_cfg_l = {
        .psap_total_gain = 811,
        .psap_band_num = 2,
        .psap_band_gain={0x3f8bd79e, 0x08000000},
        .psap_iir_coef[0].iir0.coef_b={0x04fa1dfc, 0x0c6d6b7e, 0x08000000, 0x00000000},
        .psap_iir_coef[0].iir0.coef_a={0x08000000, 0x0c6d6b7e, 0x04fa1dfc, 0x00000000},
        .psap_iir_coef[0].iir1.coef_b={0x04f9f115, 0x11279bfc, 0x1404d0c8, 0x08000000},
        .psap_iir_coef[0].iir1.coef_a={0x08000000, 0x1404d0c8, 0x11279bfc, 0x04f9f115},
        .psap_cpd_cfg[0]={0x5740, 0x0000, 0x4e02, 0x0000, 0x3b85, 0x0000, 0x7cf7, 0x0309, 0x7ff0, 0x0010, 0x7333, 0x0ccd, 0},
        .psap_limiter_cfg={524287, 0x7cf7, 0x0309, 0x7ff0, 0x0010, 127},
        .psap_dehowling_cfg.dehowling_delay=0,
        .psap_dehowling_cfg.dehowling_l.total_gain=0,
        .psap_dehowling_cfg.dehowling_l.iir_bypass_flag=0,
        .psap_dehowling_cfg.dehowling_l.iir_counter=1,
        .psap_dehowling_cfg.dehowling_l.iir_coef[0].coef_b={0x08000000, 0xf096cb51, 0x07733cd7},
        .psap_dehowling_cfg.dehowling_l.iir_coef[0].coef_a={0x08000000, 0xf096cb51, 0x07733cd7},
        .psap_dehowling_cfg.dehowling_l.dac_gain_offset=0,
        .psap_dehowling_cfg.dehowling_l.adc_gain_offset=0,
        .psap_type = 3,
        .psap_dac_gain_offset=0,
        .psap_adc_gain_offset=0,
    }
};

static const struct_psap_cfg POSSIBLY_UNUSED hwlimiter_para_48k_mode0 = {
    .psap_cfg_l = {
        .psap_total_gain = 811,
        .psap_band_num = 2,
        .psap_band_gain={0x3f8bd79e, 0x08000000},
        .psap_iir_coef[0].iir0.coef_b={0x0364529c, 0x09dd9c0b, 0x08000000, 0x00000000},
        .psap_iir_coef[0].iir0.coef_a={0x08000000, 0x09dd9c0b, 0x0364529c, 0x00000000},
        .psap_iir_coef[0].iir1.coef_b={0x0361ed64, 0x0cc9bec2, 0x109eeca2, 0x08000000},
        .psap_iir_coef[0].iir1.coef_a={0x08000000, 0x109eeca2, 0x0cc9bec2, 0x0361ed64},
        .psap_cpd_cfg[0]={0x5740, 0x0000, 0x4e02, 0x0000, 0x3b85, 0x0000, 0x7cf7, 0x0309, 0x7ff0, 0x0010, 0x7333, 0x0ccd, 0},
        .psap_limiter_cfg={524287, 0x7cf7, 0x0309, 0x7ff0, 0x0010, 127},
        .psap_dehowling_cfg.dehowling_delay=0,
        .psap_dehowling_cfg.dehowling_l.total_gain=0,
        .psap_dehowling_cfg.dehowling_l.iir_bypass_flag=0,
        .psap_dehowling_cfg.dehowling_l.iir_counter=1,
        .psap_dehowling_cfg.dehowling_l.iir_coef[0].coef_b={0x08000000, 0xf08a323c, 0x077e4bc1},
        .psap_dehowling_cfg.dehowling_l.iir_coef[0].coef_a={0x08000000, 0xf08a323c, 0x077e4bc1},
        .psap_dehowling_cfg.dehowling_l.dac_gain_offset=0,
        .psap_dehowling_cfg.dehowling_l.adc_gain_offset=0,
        .psap_type = 3,
        .psap_dac_gain_offset=0,
        .psap_adc_gain_offset=0,
    }
};

static const struct_psap_cfg POSSIBLY_UNUSED hwlimiter_para_50p7k_mode0 = {
    .psap_cfg_l = {
        .psap_total_gain = 811,
        .psap_band_num = 2,
        .psap_band_gain={0x3f8bd79e, 0x08000000},
        .psap_iir_coef[0].iir0.coef_b={0x03c46400, 0x0a8a3bd5, 0x08000000, 0x00000000},
        .psap_iir_coef[0].iir0.coef_a={0x08000000, 0x0a8a3bd5, 0x03c46400, 0x00000000},
        .psap_iir_coef[0].iir1.coef_b={0x03c305c1, 0x0de12c8a, 0x118f7298, 0x08000000},
        .psap_iir_coef[0].iir1.coef_a={0x08000000, 0x118f7298, 0x0de12c8a, 0x03c305c1},
        .psap_cpd_cfg[0]={0x5740, 0x0000, 0x4e02, 0x0000, 0x3b85, 0x0000, 0x7cf7, 0x0309, 0x7ff0, 0x0010, 0x7333, 0x0ccd, 0},
        .psap_limiter_cfg={524287, 0x7cf7, 0x0309, 0x7ff0, 0x0010, 127},
        .psap_dehowling_cfg.dehowling_delay=0,
        .psap_dehowling_cfg.dehowling_l.total_gain=0,
        .psap_dehowling_cfg.dehowling_l.iir_bypass_flag=0,
        .psap_dehowling_cfg.dehowling_l.iir_counter=1,
        .psap_dehowling_cfg.dehowling_l.iir_coef[0].coef_b={0x08000000, 0xf08d9c03, 0x077b49d0},
        .psap_dehowling_cfg.dehowling_l.iir_coef[0].coef_a={0x08000000, 0xf08d9c03, 0x077b49d0},
        .psap_dehowling_cfg.dehowling_l.dac_gain_offset=0,
        .psap_dehowling_cfg.dehowling_l.adc_gain_offset=0,
        .psap_type = 3,
        .psap_dac_gain_offset=0,
        .psap_adc_gain_offset=0,
    }
};

const struct_psap_cfg * hwlimiter_para_list_50p7k[HWLIMITER_PARA_LIST_NUM] = {
    &hwlimiter_para_50p7k_mode0,
};

const struct_psap_cfg * hwlimiter_para_list_48k[HWLIMITER_PARA_LIST_NUM] = {
    &hwlimiter_para_48k_mode0,
};

const struct_psap_cfg * hwlimiter_para_list_44p1k[HWLIMITER_PARA_LIST_NUM] = {
    &hwlimiter_para_44p1k_mode0,
};

static const struct_psap_cfg POSSIBLY_UNUSED PsapFirCoef_50p7k_mode0 = {
    .psap_cfg_l = {
        .psap_total_gain = 723,
        .psap_band_num = 9,
        .psap_band_gain={0, 476222470, 672682118, 534330399, 950188747, 1066129310, 1066129310, 424433723, 0},
        .psap_iir_coef[0].iir0.coef_b={131557370, -265754754, 134217728, 0},
        .psap_iir_coef[0].iir0.coef_a={134217728, -265754754, 131557370, 0},
        .psap_iir_coef[0].iir1.coef_b={-131557370, 397299677, -399959783, 134217728},
        .psap_iir_coef[0].iir1.coef_a={134217728, -399959783, 397299677, -131557370},
        .psap_iir_coef[1].iir0.coef_b={129986488, -264152445, 134217728, 0},
        .psap_iir_coef[1].iir0.coef_a={134217728, -264152445, 129986488, 0},
        .psap_iir_coef[1].iir1.coef_b={-129986488, 394107454, -398337669, 134217728},
        .psap_iir_coef[1].iir1.coef_a={134217728, -398337669, 394107454, -129986488},
        .psap_iir_coef[2].iir0.coef_b={125134130, -259108702, 134217728, 0},
        .psap_iir_coef[2].iir0.coef_a={134217728, -259108702, 125134130, 0},
        .psap_iir_coef[2].iir1.coef_b={-125134129, 384097929, -393171000, 134217728},
        .psap_iir_coef[2].iir1.coef_a={134217728, -393171000, 384097929, -125134129},
        .psap_iir_coef[3].iir0.coef_b={120220377, -253849171, 134217728, 0},
        .psap_iir_coef[3].iir0.coef_a={134217728, -253849171, 120220377, 0},
        .psap_iir_coef[3].iir1.coef_b={-120220366, 373726258, -387683554, 134217728},
        .psap_iir_coef[3].iir1.coef_a={134217728, -387683554, 373726258, -120220366},
        .psap_iir_coef[4].iir0.coef_b={109848832, -242200690, 134217728, 0},
        .psap_iir_coef[4].iir0.coef_a={134217728, -242200690, 109848832, 0},
        .psap_iir_coef[4].iir1.coef_b={-109848632, 351018377, -375156857, 134217728},
        .psap_iir_coef[4].iir1.coef_a={134217728, -375156857, 351018377, -109848632},
        .psap_iir_coef[5].iir0.coef_b={81171724, -205166742, 134217728, 0},
        .psap_iir_coef[5].iir0.coef_a={134217728, -205166742, 81171724, 0},
        .psap_iir_coef[5].iir1.coef_b={-81156770, 281801460, -331721088, 134217728},
        .psap_iir_coef[5].iir1.coef_a={134217728, -331721088, 281801460, -81156770},
        .psap_iir_coef[6].iir0.coef_b={66181540, -181899702, 134217728, 0},
        .psap_iir_coef[6].iir0.coef_a={134217728, -181899702, 66181540, 0},
        .psap_iir_coef[6].iir1.coef_b={-66113394, 241306967, -301500735, 134217728},
        .psap_iir_coef[6].iir1.coef_a={134217728, -301500735, 241306967, -66113394},
        .psap_iir_coef[7].iir0.coef_b={62863906, -176244865, 134217728, 0},
        .psap_iir_coef[7].iir0.coef_a={134217728, -176244865, 62863906, 0},
        .psap_iir_coef[7].iir1.coef_b={-62771443, 231892680, -293809571, 134217728},
        .psap_iir_coef[7].iir1.coef_a={134217728, -293809571, 231892680, -62771443},
        .psap_cpd_cfg[0]={27069, 0, 27069, 0, 10505, -29491, 32689, 79, 32766, 2, 29491, 3277, 0},
        .psap_cpd_cfg[1]={27069, 0, 27069, 0, 10505, -29491, 32689, 79, 32766, 2, 29491, 3277, 0},
        .psap_cpd_cfg[2]={27069, 0, 27069, 0, 10505, -29491, 32689, 79, 32766, 2, 29491, 3277, 0},
        .psap_cpd_cfg[3]={27069, 0, 27069, 0, 10505, -29491, 32689, 79, 32766, 2, 29491, 3277, 0},
        .psap_cpd_cfg[4]={27069, 0, 27069, 0, 10505, -29491, 32689, 79, 32766, 2, 29491, 3277, 0},
        .psap_cpd_cfg[5]={27069, 0, 27069, 0, 10505, -29491, 32689, 79, 32766, 2, 29491, 3277, 0},
        .psap_cpd_cfg[6]={27069, 0, 27069, 0, 10505, -29491, 32689, 79, 32766, 2, 29491, 3277, 0},
        .psap_cpd_cfg[7]={27069, 0, 27069, 0, 10505, -29491, 32689, 79, 32766, 2, 29491, 3277, 0},
        .psap_limiter_cfg={524287, 31991, 777, 32752, 16, 0},
        .psap_dac_gain_offset=0,
        .psap_adc_gain_offset=-12,
    }
};

const struct_psap_cfg * psap_coef_list_50p7k[PSAP_COEF_LIST_NUM] = {
    &PsapFirCoef_50p7k_mode0,
};

const struct_psap_cfg * psap_coef_list_48k[PSAP_COEF_LIST_NUM] = {
    &PsapFirCoef_50p7k_mode0,
};

const struct_psap_cfg * psap_coef_list_44p1k[PSAP_COEF_LIST_NUM] = {
    &PsapFirCoef_50p7k_mode0,
};
