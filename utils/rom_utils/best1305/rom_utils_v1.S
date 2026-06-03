/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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
#include "asm_macros.h"

    def_FuncExt v1, rom1_entry,                         0x00028000
    def_FuncExt v1, set__impure_ptr,                    0x00028004
    def_FuncExt v1, sbc_bits_for_mono_alloc,            0x00028004
    def_FuncExt v1, sbc_bits_for_stereo_alloc,          0x00028220
    def_FuncExt v1, sbc_header_process,                 0x00028520
    def_FuncExt v1, sbc_factors_process,                0x00028640
    def_FuncExt v1, sbc_sbsamples_make,                 0x0002873c
    def_FuncExt v1, sbc_factors_calc,                   0x00029970
    def_FuncExt v1, sbc_header_make,                    0x00029b94
    def_FuncExt v1, sbc_frame_mute,                     0x00029dc4
    def_FuncExt v1, sbc_samples_quant,                  0x00029df4
    def_FuncExt v1, sbc_frame_length_get,               0x0002acf4
    def_FuncExt v1, sbc_decoder_init,                   0x0002ad84
    def_FuncExt v1, sbc_4band_synth_filter,             0x0002ad90
    def_FuncExt v1, sbc_8band_synth_filter,             0x0002b63c
    def_FuncExt v1, sbc_frames_parser,                  0x0002c8f4
    def_FuncExt v1, sbc_frames_decode_do,               0x0002caf4
    def_FuncExt v1, sbc_frames_decode,                  0x0002cec0
    def_FuncExt v1, sbc_frames_decode_select_channel,   0x0002cee0
    def_FuncExt v1, sbc_frames_decode_out_sbsamples,    0x0002cef4
    def_FuncExt v1, sbc_encoder_init,                   0x0002d360
    def_FuncExt v1, sbc_frames_encode,                  0x0002d36c
    def_FuncExt v1, sbc_frames_encode_with_sbsamples,   0x0002f338

    def_Symbol dec_coeff_4band_tbl,                     0x2802f564
    def_Symbol dec_coeff_8band_tbl,                     0x2802f604
    def_Symbol loudness_offs_4band,                     0x2802f744
    def_Symbol loudness_offs_8band,                     0x2802f754
    def_Symbol sbc_coef_4band,                          0x2802f774
    def_Symbol sbc_coef_8band,                          0x2802f814
    def_Symbol sbc_crc_tbl,                             0x2802f954
    def_Symbol sbc_enc_analyze_4band_tbl,               0x2802fa54
    def_Symbol sbc_enc_analyze_8band_tbl,               0x2802fad4
    def_Symbol sbc_samples_quant.str1.4,                0x2802fcd4
    def_Symbol synth_4band_tbl,                         0x2802fce0
    def_Symbol synth_8band_tbl,                         0x2802fd60
