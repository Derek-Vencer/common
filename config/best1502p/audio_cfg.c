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

const IIR_CFG_T audio_eq_sw_iir_cfg = {
    .gain0 = 0,
    .gain1 = 0,
    .num = 5,
    .param = {
        {IIR_TYPE_PEAK, .0,   200,   2},
        {IIR_TYPE_PEAK, .0,   600,  2},
        {IIR_TYPE_PEAK, .0,   2000.0, 2},
        {IIR_TYPE_PEAK, .0,  6000.0, 2},
        {IIR_TYPE_PEAK, .0,  12000.0, 2}
    }
};

const IIR_CFG_T * const audio_eq_sw_iir_cfg_list[EQ_SW_IIR_LIST_NUM]={
    &audio_eq_sw_iir_cfg,
};

const FIR_CFG_T audio_eq_hw_fir_cfg_44p1k = {
    .gain = 0.0f,
    .len = 384,
    .coef =
    {
        (1<<23)-1,
    }
};

const FIR_CFG_T audio_eq_hw_fir_cfg_48k = {
    .gain = 0.0f,
    .len = 384,
    .coef =
    {
        (1<<23)-1,
    }
};


const FIR_CFG_T audio_eq_hw_fir_cfg_96k = {
    .gain = 0.0f,
    .len = 384,
    .coef =
    {
        (1<<23)-1,
    }
};

FIR_CFG_T audio_eq_hw_fir_adaptive_eq_cfg = {
    .gain = 0.0f,
    .len = 384,
    .coef =
    {
        (1<<23)-1,
    }
};

const FIR_CFG_T * const audio_eq_hw_fir_cfg_list[EQ_HW_FIR_LIST_NUM]={
    &audio_eq_hw_fir_cfg_44p1k,
    &audio_eq_hw_fir_cfg_48k,
    &audio_eq_hw_fir_cfg_96k,
};

#if !defined(BESUI_TWS_EN) && !defined(BESUI_STEREO_EN)
//hardware dac iir eq
const IIR_CFG_T audio_eq_hw_dac_iir_cfg = {
#if defined(AUDIO_HEARING_COMPSATN)
    .gain0 = -22,
    .gain1 = -22,
#else
    .gain0 = 0,
    .gain1 = 0,
#endif
    .num = 8,
    .param = {
        {IIR_TYPE_PEAK, 0,   1000.0,   0.7},
        {IIR_TYPE_PEAK, 0,   1000.0,   0.7},
        {IIR_TYPE_PEAK, 0,   1000.0,   0.7},
        {IIR_TYPE_PEAK, 0,   1000.0,   0.7},
        {IIR_TYPE_PEAK, 0,   1000.0,   0.7},
        {IIR_TYPE_PEAK, 0,   1000.0,   0.7},
        {IIR_TYPE_PEAK, 0,   1000.0,   0.7},
        {IIR_TYPE_PEAK, 0,   1000.0,   0.7},
    }
};

const IIR_CFG_T audio_eq_anc_hw_dac_iir_cfg = {
#if defined(AUDIO_HEARING_COMPSATN)
    .gain0 = -22,
    .gain1 = -22,
#else
    .gain0 = 0,
    .gain1 = 0,
#endif
    .num = 8,
    .param = {
        {IIR_TYPE_PEAK, 0,   1000.0,   0.7},
        {IIR_TYPE_PEAK, 0,   1000.0,   0.7},
        {IIR_TYPE_PEAK, 0,   1000.0,   0.7},
        {IIR_TYPE_PEAK, 0,   1000.0,   0.7},
        {IIR_TYPE_PEAK, 0,   1000.0,   0.7},
        {IIR_TYPE_PEAK, 0,   1000.0,   0.7},
        {IIR_TYPE_PEAK, 0,   1000.0,   0.7},
        {IIR_TYPE_PEAK, 0,   1000.0,   0.7},
    }
};
#endif //#if !defined(BESUI_TWS_EN) && !defined(BESUI_STEREO_EN)

IIR_CFG_T audio_eq_hw_dac_iir_adaptive_eq_cfg = {
    .gain0 = 0,
    .gain1 = 0,
    .num = 1,
    .param = {
        {IIR_TYPE_PEAK, 0,    1000,    0.7},
    }
};

const IIR_CFG_T * const POSSIBLY_UNUSED audio_eq_hw_dac_iir_cfg_list[EQ_HW_DAC_IIR_LIST_NUM]={
    &audio_eq_hw_dac_iir_cfg,
    &audio_eq_anc_hw_dac_iir_cfg,
#if defined(EQ_SET_CUSTOMER_EN)
    &audio_eq_hw_dac_iir_cfg_1,
    &audio_eq_hw_dac_iir_cfg_2,
    &audio_eq_hw_dac_iir_cfg_3,
    &audio_eq_hw_dac_iir_cfg_4,
    &audio_eq_hw_dac_iir_cfg_5,
#endif
};

#if defined(__IIR_EQ_PROCESS_LR_2CH__)
const IIR_CFG_T audio_eq_hw_dac_iir_cfg_2 = {
#if defined(AUDIO_HEARING_COMPSATN)
    .gain0 = -22,
    .gain1 = -22,
#else
    .gain0 = 0,
    .gain1 = 0,
#endif
    .num = 8,
    .param = {
        {IIR_TYPE_PEAK, 0,   1000.0,   0.7},
        {IIR_TYPE_PEAK, 0,   1000.0,   0.7},
        {IIR_TYPE_PEAK, 0,   1000.0,   0.7},
        {IIR_TYPE_PEAK, 0,   1000.0,   0.7},
        {IIR_TYPE_PEAK, 0,   1000.0,   0.7},
        {IIR_TYPE_PEAK, 0,   1000.0,   0.7},
        {IIR_TYPE_PEAK, 0,   1000.0,   0.7},
        {IIR_TYPE_PEAK, 0,   1000.0,   0.7},
    }
};

// R channel iir
const IIR_CFG_T * const POSSIBLY_UNUSED audio_eq_hw_dac_iir_cfg_list2[EQ_HW_DAC_IIR_LIST_NUM]={
    &audio_eq_hw_dac_iir_cfg_2,
};
#endif

//hardware dac iir eq
const IIR_CFG_T audio_eq_hw_adc_iir_adc_cfg = {
    .gain0 = 0,
    .gain1 = 0,
    .num = 1,
    .param = {
        {IIR_TYPE_PEAK, 0.0,   1000.0,   0.7},
    }
};

const IIR_CFG_T * const POSSIBLY_UNUSED audio_eq_hw_adc_iir_cfg_list[EQ_HW_ADC_IIR_LIST_NUM]={
    &audio_eq_hw_adc_iir_adc_cfg,
};

//hardware iir eq
const IIR_CFG_T audio_eq_hw_iir_cfg = {
    .gain0 = 0,
    .gain1 = 0,
    .num = 8,
    .param = {
        {IIR_TYPE_PEAK, -10.1,   100.0,   7},
        {IIR_TYPE_PEAK, -10.1,   400.0,   7},
        {IIR_TYPE_PEAK, -10.1,   700.0,   7},
        {IIR_TYPE_PEAK, -10.1,   1000.0,   7},
        {IIR_TYPE_PEAK, -10.1,   3000.0,   7},
        {IIR_TYPE_PEAK, -10.1,   5000.0,   7},
        {IIR_TYPE_PEAK, -10.1,   7000.0,   7},
        {IIR_TYPE_PEAK, -10.1,   9000.0,   7},

    }
};

const IIR_CFG_T * const POSSIBLY_UNUSED audio_eq_hw_iir_cfg_list[EQ_HW_IIR_LIST_NUM]={
    &audio_eq_hw_iir_cfg,
};

const DrcConfig audio_drc_cfg = {
     .knee = 3,
     .filter_type = {1000, -1},
     .band_num = 2,
     .look_ahead_time = 10,
     .band_settings = {
         {-20, 0, 2, 3, 3000, 1},
         {-20, 0, 2, 3, 3000, 1},
     }
 };

const LimiterConfig audio_limiter_cfg = {
    .knee = 2,
    .look_ahead_time = 10,
    .threshold = -1,
    .makeup_gain = 0,
    .ratio = 1000,
    .attack_time = 3,
    .release_time = 3000,
};

const SpectrumFixConfig audio_spectrum_cfg = {
    .freq_num = 9,
    .freq_list = {200, 400, 600, 800, 1000, 1200, 1400, 1600, 1800},
};

const ReverbConfig audio_reverb_cfg = {
    .bypass = 0,
    .high_pass_f0 = 100,
    .gain = 0,
};

const BassEnhancerConfig audio_bass_cfg =
{
    .low_cut_freq = 100,
    .high_cut_freq = 200,
    .gain0 = 0,
    .gain1 = -40,
};

#ifdef AUDIO_DYNAMIC_BOOST
const DynamicBoostConfig audio_dynamic_boost_cfg = {
    .debug = 1,
    .xover_freq = {200},
    .order = 4,
    .CT = -40,
    .CS = 0.18,
    .WT = -40,
    .WS = 0.3,
    .ET = -60,
    .ES = 0,
    .attack_time        = 0.0001f,
    .release_time       = 0.0001f,
    .makeup_gain        = -6,
    .delay              = 128,
    .tav                = 1.0f,
    .eq_num = 2,
    .boost_eq = {
        {
            .type = IIR_TYPE_PEAK,
            .gain = 10,
            .freq = 33,
            .Q = 0.5,
        },
        {
            .type = IIR_TYPE_PEAK,
            .gain = -1,
            .freq = 240,
            .Q = 1.1,
        },
        {
            .type = IIR_TYPE_PEAK,
            .gain = -1,
            .freq = 1000, // -1 for unused eq
            .Q = 0.7,
        },
        {
            .type = IIR_TYPE_PEAK,
            .gain = -1,
            .freq = 2000, // -1 for unused eq
            .Q = 0.7,
        }
    }
};
#endif

const VirtualSurroundConfig audio_virtual_surround_cfg =
{
    .switch_on = false,
    .pre_delay_1 = 115,
    .pre_delay_2 = 215,
    .pre_delay_3 = 215,
    .allpass_len1 = 400,
    .allpass_len2 = 353,
    .allpass_len3 = 215,
    .allpass_len4 = 400,
    .allpass_len5 = 130,
    .allpass_len6 = 400,

    // percent
    .reverb_input_ratio_1 = 100,
    .reverb_input_ratio_2 = 100,

    .k1 = 50,
    .k2 = 40,
    .k3 = 50,
    .k4 = 40,
    .k5 = 20,
    .k6 = 100,

    .center_channel_ratio = 30,
    .surround_channel_ratio = 100,
    .output_ratio = 100,

    .hrtf_input_ratio = 100,
    .hrtf_output_ratio = 100,
    .hrtf_select_index = 0,
};

const BassEnhancerConfig audio_bass_enhancer_cfg =
{
    .switch_on = true,
    .low_cut_freq = 100,  // low_cut_freq
    .high_cut_freq = 700, // high_cut_freq
    .gain0 = 0, // invalid
    .gain1 = 7.95,  // gain(db)
};

const DynamicEqConfig audio_dynamic_eq_cfg = {
    .switch_on = 1,
    .debug = 0,
    .offset = 3.9,
    .gain   = 0.f,
    .eq_num = 6,
    .dyeq_param_cfg = {
        /*
            {
                .dyeq_eq_cfg = {type,   gain,   fc,   q}
                .dyeq_drc_cfg = {threshold,  attack,   release}
            }
        */
        {
            .dyeq_eq_cfg = {IIR_BIQUARD_PEAKINGEQ, 6.0,   100.0,   1.0},
            .dyeq_drc_cfg = {-50.0,   20.0,   200.0},
        },
        {
            .dyeq_eq_cfg = {IIR_BIQUARD_PEAKINGEQ, 0.0,   500.0,   1.0},
            .dyeq_drc_cfg = {-50.0,   20.0,   200.0},
        },
        {
            .dyeq_eq_cfg = {IIR_BIQUARD_PEAKINGEQ, 0.0,   2000.0,   1.0},
            .dyeq_drc_cfg = {-50.0,   20.0,   200.0},
        },
        {
            .dyeq_eq_cfg = {IIR_BIQUARD_PEAKINGEQ, 0.0,   4000.0,   1.0},
            .dyeq_drc_cfg = {-50.0,   20.0,   200.0},
        },
        {
            .dyeq_eq_cfg = {IIR_BIQUARD_PEAKINGEQ, 0.0,   6000.0,   1.0},
            .dyeq_drc_cfg = {-50.0,   20.0,   200.0},
        },
        {
            .dyeq_eq_cfg = {IIR_BIQUARD_PEAKINGEQ, 0.0,   8000.0,   1.0},
            .dyeq_drc_cfg = {-50.0,   20.0,   200.0},
        },
    }
};

const AdjEQConfig audio_adj_eq_rev_cfg = {
    .debug_en = 1,
    .forget_factor = 0.1, // 0 ~ 1, the smaller, the faster.
    .adj_eq_rev_num = 5, //max 5
    .normalsz_lab = {
        /* default in-ear ref/fb */
        3.62193,2.8863,2.6083,2.4545,2.168,1.8439,1.5996,1.5367,1.4452,1.3541,1.3029,1.236,1.1745,1.1502,1.1111,1.0944,1.0768,1.1609,1.1024,1.0235,1.1367,0.90604,0.88235,0.90596,1.1543,0.81243,0.78145,1.3122,4.1499,1.0651,1.3106,2.2671,1.1991
        // /* default on-ear ref/fb */
        // 1.8517,1.1662,1.1377,1.0753,0.99899,0.90121,0.81115,0.75371,0.71601,0.68958,0.67433,0.67873,0.68826,0.69131,0.69101,0.68153,0.66703,0.64742,0.62056,0.58568,0.56067,0.54479,0.55702,0.51505,0.47969,0.50826,0.40059,0.44995,0.34503,0.4123,0.44098,0.45335,0.23673
        },
    .adj_eq_rev_cfg = {  // filter type: IIR_BIQUARD_LOWSHELF/IIR_BIQUARD_PEAKINGEQ
        {
            .type = IIR_BIQUARD_LOWSHELF,
            .freq = 62.5,
            .Q = 1.,
            .gain_cali = 0,//-24,
            .min_gain = -15,
            .max_gain = 20,
        },{
            .type = IIR_BIQUARD_LOWSHELF,
            .freq = 125,
            .Q = 0.5,
            .gain_cali = 0,// -19,
            .min_gain = -15,
            .max_gain = 20,
        },{
            .type = IIR_BIQUARD_PEAKINGEQ,
            .freq = 125,
            .Q = 1.5,
            .gain_cali = 0,//-19,
            .min_gain = -15,
            .max_gain = 20,
        },{
            .type = IIR_BIQUARD_PEAKINGEQ,
            .freq = 250,
            .Q = 1.2,
            .gain_cali = 0,//-13,
            .min_gain = -15,
            .max_gain = 20,
        },{
            .type = IIR_BIQUARD_PEAKINGEQ,
            .freq = 500,
            .Q = 1.4,
            .gain_cali = 0,//-6,
            .min_gain = -15,
            .max_gain = 20,
        }
    }
};

const AdaptiveVolumeConfig audio_adaptive_volume_cfg =
{
    .debug = 0,
    .CT = -50,
    .CS = -0.3,
    .WT = -50,
    .WS = -0.0,
    .ET = -65,
    .ES = 0.0,
    .makeup_gain = 0,
    .attack_time = 0.0001,
    .release_time = 0.0001,
    .smooth_factor = 0.6,
};

/* AUDIO_VOL_CTRL_EQ */
const IIR_CFG_T audio_eq_cfg_vol_0 = {
    .gain0 = 0,
    .gain1 = 0,
    .num = 2,
    .param = {
        {IIR_TYPE_PEAK, 0,  1000,   0.707},
        {IIR_TYPE_PEAK, 0,  1000,   0.707},
    }
};

const IIR_CFG_T audio_eq_cfg_vol_1 = {
    .gain0 = 0,
    .gain1 = 0,
    .num = 2,
    .param = {
        {IIR_TYPE_PEAK, 0,  1000,   0.707},
        {IIR_TYPE_PEAK, 0,  1000,   0.707},
    }
};

const IIR_CFG_T audio_eq_cfg_vol_2 = {
    .gain0 = 0,
    .gain1 = 0,
    .num = 2,
    .param = {
        {IIR_TYPE_PEAK, 0,  1000,   0.707},
        {IIR_TYPE_PEAK, 0,  1000,   0.707},
    }
};

const IIR_CFG_T audio_eq_cfg_vol_3 = {
    .gain0 = 0,
    .gain1 = 0,
    .num = 2,
    .param = {
        {IIR_TYPE_PEAK, 0,  1000,   0.707},
        {IIR_TYPE_PEAK, 0,  1000,   0.707},
    }
};

const IIR_CFG_T audio_eq_cfg_vol_4 = {
    .gain0 = 0,
    .gain1 = 0,
    .num = 2,
    .param = {
        {IIR_TYPE_PEAK, 0,  1000,   0.707},
        {IIR_TYPE_PEAK, 0,  1000,   0.707},
    }
};

const IIR_CFG_T audio_eq_cfg_vol_5 = {
    .gain0 = 0,
    .gain1 = 0,
    .num = 2,
    .param = {
        {IIR_TYPE_PEAK, 0,  1000,   0.707},
        {IIR_TYPE_PEAK, 0,  1000,   0.707},
    }
};

const IIR_CFG_T audio_eq_cfg_vol_6 = {
    .gain0 = 0,
    .gain1 = 0,
    .num = 2,
    .param = {
        {IIR_TYPE_PEAK, 0,  1000,   0.707},
        {IIR_TYPE_PEAK, 0,  1000,   0.707},
    }
};

const IIR_CFG_T audio_eq_cfg_vol_7 = {
    .gain0 = 0,
    .gain1 = 0,
    .num = 2,
    .param = {
        {IIR_TYPE_PEAK, 0,  1000,   0.707},
        {IIR_TYPE_PEAK, 0,  1000,   0.707},
    }
};

const IIR_CFG_T audio_eq_cfg_vol_8 = {
    .gain0 = 0,
    .gain1 = 0,
    .num = 2,
    .param = {
        {IIR_TYPE_PEAK, 0,  1000,   0.707},
        {IIR_TYPE_PEAK, 0,  1000,   0.707},
    }
};

const IIR_CFG_T audio_eq_cfg_vol_9 = {
    .gain0 = 0,
    .gain1 = 0,
    .num = 2,
    .param = {
        {IIR_TYPE_PEAK, 0,  1000,   0.707},
        {IIR_TYPE_PEAK, 0,  1000,   0.707},
    }
};

const IIR_CFG_T audio_eq_cfg_vol_10 = {
    .gain0 = 0,
    .gain1 = 0,
    .num = 2,
    .param = {
        {IIR_TYPE_PEAK, 0,  1000,   0.707},
        {IIR_TYPE_PEAK, 0,  1000,   0.707},
    }
};

const IIR_CFG_T audio_eq_cfg_vol_11 = {
    .gain0 = 0,
    .gain1 = 0,
    .num = 2,
    .param = {
        {IIR_TYPE_PEAK, 0,  1000,   0.707},
        {IIR_TYPE_PEAK, 0,  1000,   0.707},
    }
};

const IIR_CFG_T audio_eq_cfg_vol_12 = {
    .gain0 = 0,
    .gain1 = 0,
    .num = 2,
    .param = {
        {IIR_TYPE_PEAK, 0,  1000,   0.707},
        {IIR_TYPE_PEAK, 0,  1000,   0.707},
    }
};

const IIR_CFG_T audio_eq_cfg_vol_13 = {
    .gain0 = 0,
    .gain1 = 0,
    .num = 2,
    .param = {
        {IIR_TYPE_PEAK, 0,  1000,   0.707},
        {IIR_TYPE_PEAK, 0,  1000,   0.707},
    }
};

const IIR_CFG_T audio_eq_cfg_vol_14 = {
    .gain0 = 0,
    .gain1 = 0,
    .num = 2,
    .param = {
        {IIR_TYPE_PEAK, 0,  1000,   0.707},
        {IIR_TYPE_PEAK, 0,  1000,   0.707},
    }
};

const IIR_CFG_T audio_eq_cfg_vol_15 = {
    .gain0 = 0,
    .gain1 = 0,
    .num = 2,
    .param = {
        {IIR_TYPE_PEAK, 0,  1000,   0.707},
        {IIR_TYPE_PEAK, 0,  1000,   0.707},
    }
};

const IIR_CFG_T audio_eq_cfg_vol_16 = {
    .gain0 = 0,
    .gain1 = 0,
    .num = 2,
    .param = {
        {IIR_TYPE_PEAK, 0,  1000,   0.707},
        {IIR_TYPE_PEAK, 0,  1000,   0.707},
    }
};

const IIR_CFG_T * const POSSIBLY_UNUSED audio_eq_cfg_vol_list[VOL_CTRL_EQ_LIST_NUM] = {
    &audio_eq_cfg_vol_0,
    &audio_eq_cfg_vol_1,
    &audio_eq_cfg_vol_2,
    &audio_eq_cfg_vol_3,
    &audio_eq_cfg_vol_4,
    &audio_eq_cfg_vol_5,
    &audio_eq_cfg_vol_6,
    &audio_eq_cfg_vol_7,
    &audio_eq_cfg_vol_8,
    &audio_eq_cfg_vol_9,
    &audio_eq_cfg_vol_10,
    &audio_eq_cfg_vol_11,
    &audio_eq_cfg_vol_12,
    &audio_eq_cfg_vol_13,
    &audio_eq_cfg_vol_14,
    &audio_eq_cfg_vol_15,
    &audio_eq_cfg_vol_16,
};
