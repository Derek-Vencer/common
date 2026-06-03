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
#if 1 //1503D
    def_FuncExt v1, rom1_entry,                         0x00030000
    def_FuncExt v1, set__impure_ptr,                    0x00030ad8
    def_FuncExt v1, sbc_bits_for_mono_alloc,            0x00030ad8
    def_FuncExt v1, sbc_bits_for_stereo_alloc,          0x00030da0
    def_FuncExt v1, sbc_header_process,                 0x00031aa4
    def_FuncExt v1, sbc_factors_process,                0x000319f8
    def_FuncExt v1, sbc_factors_calc,                   0x00031760
    def_FuncExt v1, sbc_header_make,                    0x00032708
    def_FuncExt v1, sbc_samples_quant,                  0x00032bb4
    def_FuncExt v1, sbc_decoder_init,                   0x000322e4
    def_FuncExt v1, sbc_4band_synth_filter,             0x000330bc
    def_FuncExt v1, sbc_8band_synth_filter,             0x0003155c
    def_FuncExt v1, sbc_frames_parser,                  0x00031dbc
    def_FuncExt v1, sbc_decode,                         0x000322f4
    def_FuncExt v1, sbc_encoder_init,                   0x00030a64
    def_FuncExt v1, sbc_encode,                         0x00030aa8
    def_FuncExt v1, btif_sbc_init_decoder,              0x00032360
    def_FuncExt v1, btif_sbc_decode_frames,             0x00032364
    def_FuncExt v1, btif_sbc_decode_frames_select_channel, 0x000324ac
    def_FuncExt v1, btif_sbc_decode_frames_parser,      0x00032604
    def_FuncExt v1, btif_sbc_init_encoder,              0x0003260c
    def_FuncExt v1, btif_sbc_encode_frames,             0x00032614
    def_FuncExt v1, btif_plc_update_sbc_decoder_state,  0x00032670
    def_Symbol sbc_crc_tbl,                             0x220270e0

    def_FuncExt v1, aacDecoder_Close_real,              0x00033584
    def_FuncExt v1, is_aacDecoder_Close_real            0x000335b8
    def_FuncExt v1, aacDecoder_GetStreamInfo_real       0x000335c8
    def_FuncExt v1, aacDecoder_DecodeFrame_real         0x000333e0
    def_FuncExt v1, aacDecoder_Fill_real                0x000333b0
    def_FuncExt v1, aacDecoder_DecodeFrame_Config_real  0x000335cc
    def_FuncExt v1, aacDecoder_SetParam_real            0x000332e8
    def_FuncExt v1, aacDecoder_Open_real                0x00033330
    def_FuncExt v1, aacCreateMuteFrame_real             0x000335d0

    def_FuncExt v1, CvsdEncInit                         0x00033648
    def_FuncExt v1, CvsdDecInit                         0x00033668
    def_FuncExt v1, CvsdEncode                          0x00033688
    def_FuncExt v1, CvsdDecode                          0x00033ba8

    def_FuncExt v1, lc3_api_decoder_init_sneak_rom      0x000352f0
    def_FuncExt v1, lc3_api_encoder_init_sneak_rom      0x0003549c
#endif

#if 0 //1503C
    def_FuncExt v1, rom1_entry,                         0x00030000
    def_FuncExt v1, set__impure_ptr,                    0x00030de8
    def_FuncExt v1, sbc_bits_for_mono_alloc,            0x00030de8
    def_FuncExt v1, sbc_bits_for_stereo_alloc,          0x000310b0
    def_FuncExt v1, sbc_header_process,                 0x00031db0
    def_FuncExt v1, sbc_factors_process,                0x00031d04
    def_FuncExt v1, sbc_factors_calc,                   0x00031a70
    def_FuncExt v1, sbc_header_make,                    0x000326d4
    def_FuncExt v1, sbc_samples_quant,                  0x00032b80
    def_FuncExt v1, sbc_decoder_init,                   0x000325f0
    def_FuncExt v1, sbc_4band_synth_filter,             0x00033088
    def_FuncExt v1, sbc_8band_synth_filter,             0x0003186c
    def_FuncExt v1, sbc_frames_parser,                  0x000320c8
    def_FuncExt v1, sbc_decode,                         0x00032600
    def_FuncExt v1, sbc_encoder_init,                   0x00030d74
    def_FuncExt v1, sbc_encode,                         0x00030db8
    def_FuncExt v1, btif_sbc_init_decoder,              0x00030744
    def_FuncExt v1, btif_sbc_decode_frames,             0x00030748
    def_FuncExt v1, btif_sbc_decode_frames_select_channel, 0x00030890
    def_FuncExt v1, btif_sbc_decode_frames_parser,      0x000309b8
    def_FuncExt v1, btif_sbc_init_encoder,              0x000309c0
    def_FuncExt v1, btif_sbc_encode_frames,             0x000309c8
    def_FuncExt v1, btif_plc_update_sbc_decoder_state,  0x00030a24
    def_Symbol sbc_crc_tbl,                             0x22026bf0

    def_FuncExt v1, aacDecoder_Close_real,              0x00033514
    def_FuncExt v1, is_aacDecoder_Close_real            0x00033538
    def_FuncExt v1, aacDecoder_GetStreamInfo_real       0x00033548
    def_FuncExt v1, aacDecoder_DecodeFrame_real         0x00033384
    def_FuncExt v1, aacDecoder_Fill_real                0x0003336c
    def_FuncExt v1, aacDecoder_DecodeFrame_Config_real  0x0003354c
    def_FuncExt v1, aacDecoder_SetParam_real            0x000332b8
    def_FuncExt v1, aacDecoder_Open_real                0x00033300
    def_FuncExt v1, aacCreateMuteFrame_real             0x00033550

    def_FuncExt v1, CvsdEncInit                         0x000335c8
    def_FuncExt v1, CvsdDecInit                         0x000335e8
    def_FuncExt v1, CvsdEncode                          0x00033608
    def_FuncExt v1, CvsdDecode                          0x00033b28

    def_FuncExt v1, lc3_api_decoder_init_sneak_rom      0x00035270
    def_FuncExt v1, lc3_api_encoder_init_sneak_rom      0x0003541c
#endif