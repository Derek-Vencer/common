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

    .syntax    unified
    .align    2

    #include "rom_utils_v1.S"

    .macro    def_RomFunc   name
    .section .text.\name , "ax", %progbits
    .thumb
    .thumb_func
    .align    2
    .globl    \name
    .type    \name, %function
\name\():
#if 0
    push {r0-r3, r10, lr}
    bl hal_get_chip_metal_id
    cmp r0, 0
    bgt .L0_\name
    pop {r0-r3, r10, lr}
    ldr pc, =\name\()_v1
.L0_\name\():
    pop {r0-r3, r10, lr}
#endif
    ldr pc, =\name\()_v1
    .pool
    .size    \name, . - \name
    .endm

    def_RomFunc rom1_entry
    def_RomFunc set__impure_ptr
    def_RomFunc sbc_frames_encode
    def_RomFunc sbc_frames_decode
    def_RomFunc sbc_decoder_init
    def_RomFunc sbc_encoder_init
    def_RomFunc sbc_frames_decode_out_sbsamples
    def_RomFunc sbc_frames_encode_with_sbsamples
    def_RomFunc InitCQueue
    def_RomFunc IsEmptyCQueue
    def_RomFunc LengthOfCQueue
    def_RomFunc AvailableOfCQueue
    def_RomFunc EnCQueue
    def_RomFunc EnCQueueFront
    def_RomFunc DeCQueue
    def_RomFunc PeekCQueue
    def_RomFunc fdk_aac_set_BitMask
    def_RomFunc fdk_aac_set_SineTable480
    def_RomFunc fdk_aac_set_SineTable512
    def_RomFunc fdk_aac_set_RotVectorReal240
    def_RomFunc fdk_aac_set_RotVectorImag240
    def_RomFunc fdk_aac_set_RotVectorReal480
    def_RomFunc fdk_aac_set_RotVectorImag480
    def_RomFunc fdk_aac_set_windowSlopes
    def_RomFunc fdk_aac_set_ldIntCoeff
    def_RomFunc fdk_aac_set_invSqrtTab
    def_RomFunc _Z13CalcInvLdDatal
    def_RomFunc _Z5fLog2li
    def_RomFunc _Z5fLog2liPi
    def_RomFunc _Z8sqrtFixpl
    def_RomFunc _Z12invSqrtNorm2lPi
    def_RomFunc _Z8fDivNormllPi
    def_RomFunc _Z8fDivNormll
    def_RomFunc _Z16fDivNormHighPrecllPi
    def_RomFunc _Z12LdDataVectorPlS_i
    def_RomFunc _Z9schur_divlli
    def_RomFunc _Z9fMultNormllPi
    def_RomFunc _Z10CalcLdDatal
    def_RomFunc _Z9CalcLdInti
    def_RomFunc _Z14InitInvSqrtTabv
    def_RomFunc _Z7fPowIntliiPi
    def_RomFunc _Z4fPowliliPi
    def_RomFunc _Z9InitLdIntv
    def_RomFunc _Z5f2PowliPi
    def_RomFunc _Z5f2Powli
    def_RomFunc FDK_put
    def_RomFunc FDK_putBwd
    def_RomFunc FDK_getValidBits
    def_RomFunc FDK_pushBack
    def_RomFunc FDK_pushForward
    def_RomFunc FDK_ResetBitBuffer
    def_RomFunc FDK_get
    def_RomFunc FDK_InitBitBuffer
    def_RomFunc FDK_Fetch
    def_RomFunc FDK_getFreeBits
    def_RomFunc FDK_get32
    def_RomFunc FDK_Feed
    def_RomFunc _Z17FDKgetWindowSlopeii
    def_RomFunc _Z23getBitstreamElementList17AUDIO_OBJECT_TYPEahh
    def_RomFunc _Z6dct_IVPliPi
    def_RomFunc _Z3fftiPlPi
    def_RomFunc _Z7dit_fftPliPK8FIXP_SPKi
    def_RomFunc _ZL9getTablesPPK8FIXP_SPKS2_Pii

    .end

