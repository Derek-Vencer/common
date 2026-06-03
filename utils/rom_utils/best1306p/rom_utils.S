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

    .syntax    unified
    .align    2

    #include "rom_utils_v1.S"
    #include "rom_utils_speech_v1.S"
    #include "rom_utils_speech_v2.S"
    #include "rom_utils_speech_v3.S"

    .macro    def_RomFunc   name
    .section .text.\name , "ax", %progbits
    .thumb
    .thumb_func
    .align    2
    .globl    \name
    .type    \name, %function
\name\():
    ldr pc, =\name\()_v1
    .pool
    .size    \name, . - \name
    .endm

    def_RomFunc rom1_entry
    def_RomFunc set__impure_ptr
    def_RomFunc sbc_bits_for_mono_alloc
    def_RomFunc sbc_bits_for_stereo_alloc
    def_RomFunc sbc_header_process
    def_RomFunc sbc_factors_process
    def_RomFunc sbc_factors_calc
    def_RomFunc sbc_header_make
    def_RomFunc sbc_samples_quant
    def_RomFunc sbc_decoder_init
    def_RomFunc sbc_4band_synth_filter
    def_RomFunc sbc_8band_synth_filter
    def_RomFunc sbc_frames_parser
    def_RomFunc sbc_frames_decode
    def_RomFunc sbc_encoder_init
    def_RomFunc btif_sbc_init_decoder
    def_RomFunc btif_sbc_decode_frames
    def_RomFunc btif_sbc_decode_frames_select_channel
    def_RomFunc btif_sbc_decode_frames_parser
    def_RomFunc btif_sbc_init_encoder
    def_RomFunc btif_sbc_encode_frames
    def_RomFunc btif_plc_update_sbc_decoder_state
#ifdef AAC_IN_ROM
    def_RomFunc aacDecoder_Close_real
    def_RomFunc is_aacDecoder_Close_real
    def_RomFunc aacDecoder_GetStreamInfo_real
    def_RomFunc aacDecoder_DecodeFrame_real
    def_RomFunc aacDecoder_Fill_real
    def_RomFunc aacDecoder_DecodeFrame_Config_real
    def_RomFunc aacDecoder_SetParam_real
    def_RomFunc aacDecoder_Open_real
    def_RomFunc aacCreateMuteFrame_real
#endif
#ifdef CVSD_IN_ROM
    def_RomFunc CvsdEncInit
    def_RomFunc CvsdDecInit
    def_RomFunc CvsdEncode
    def_RomFunc CvsdDecode
#endif
#ifdef LC3_IN_ROM_V2
    def_RomFunc lc3_api_decoder_init_sneak_rom
    def_RomFunc lc3_api_encoder_init_sneak_rom
#endif

 #include "rom_utils_speech.inc"

    .end
