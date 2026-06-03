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
#include "asm_macros.h"
    def_FuncExt v1, rom1_entry,                         0x00030000
    def_FuncExt v1, set__impure_ptr,                    0x00030b10
    def_FuncExt v1, sbc_bits_for_mono_alloc,            0x00030b10
    def_FuncExt v1, sbc_bits_for_stereo_alloc,          0x00030dd8
    def_FuncExt v1, sbc_header_process,                 0x00031adc
    def_FuncExt v1, sbc_factors_process,                0x00031a30
    def_FuncExt v1, sbc_factors_calc,                   0x00031798
    def_FuncExt v1, sbc_header_make,                    0x00032738
    def_FuncExt v1, sbc_samples_quant,                  0x00032be4
    def_FuncExt v1, sbc_decoder_init,                   0x0003231c
    def_FuncExt v1, sbc_4band_synth_filter,             0x000330ec
    def_FuncExt v1, sbc_8band_synth_filter,             0x00031594
    def_FuncExt v1, sbc_frames_parser,                  0x00031df4
    def_FuncExt v1, sbc_decode,                         0x0003232c
    def_FuncExt v1, sbc_encoder_init,                   0x00030a9c
    def_FuncExt v1, sbc_encode,                         0x00030ae0
    def_FuncExt v1, btif_sbc_init_decoder,              0x00032398
    def_FuncExt v1, btif_sbc_decode_frames,             0x0003239c
    def_FuncExt v1, btif_sbc_decode_frames_select_channel, 0x000324e4
    def_FuncExt v1, btif_sbc_decode_frames_parser,      0x0003263c
    def_FuncExt v1, btif_sbc_init_encoder,              0x00032644
    def_FuncExt v1, btif_sbc_encode_frames,             0x0003264c
    def_FuncExt v1, btif_plc_update_sbc_decoder_state,  0x000326a8
    def_Symbol sbc_crc_tbl,                             0x22027188

    def_FuncExt v1, aacDecoder_Close_real,              0x000335b4
    def_FuncExt v1, is_aacDecoder_Close_real            0x000335e8
    def_FuncExt v1, aacDecoder_GetStreamInfo_real       0x000335f8
    def_FuncExt v1, aacDecoder_DecodeFrame_real         0x00033410
    def_FuncExt v1, aacDecoder_Fill_real                0x000333e0
    def_FuncExt v1, aacDecoder_DecodeFrame_Config_real  0x000335fc
    def_FuncExt v1, aacDecoder_SetParam_real            0x00033318
    def_FuncExt v1, aacDecoder_Open_real                0x00033360
    def_FuncExt v1, aacCreateMuteFrame_real             0x00033600

    def_FuncExt v1, CvsdEncInit                         0x00033678
    def_FuncExt v1, CvsdDecInit                         0x00033698
    def_FuncExt v1, CvsdEncode                          0x000336b8
    def_FuncExt v1, CvsdDecode                          0x00033bd8

    def_FuncExt v1, lc3_api_decoder_init_sneak_rom      0x00035330
    def_FuncExt v1, lc3_api_encoder_init_sneak_rom      0x000354e4
