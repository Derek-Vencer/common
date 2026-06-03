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

    .macro    def_FuncV1    name addr
    \name\()_v1 = (\addr | 1)
    @.set \name\()_v1, (\addr | 1)
    .endm

    def_FuncV1 rom1_entry, 0x0000c000
    def_FuncV1 set__impure_ptr, 0x0000c06c
    def_FuncV1 sbc_frames_encode, 0x000112b0
    def_FuncV1 sbc_frames_decode, 0x00010b84
    def_FuncV1 sbc_decoder_init, 0x0000f0cc
    def_FuncV1 sbc_encoder_init, 0x000112a4
    def_FuncV1 sbc_frames_decode_out_sbsamples, 0x00010eb4
    def_FuncV1 sbc_frames_encode_with_sbsamples, 0x00013154
    def_FuncV1 InitCQueue, 0x0000c078
    def_FuncV1 IsEmptyCQueue, 0x0000c090
    def_FuncV1 LengthOfCQueue, 0x0000c09c
    def_FuncV1 AvailableOfCQueue, 0x0000c0a0
    def_FuncV1 EnCQueue, 0x0000c0a8
    def_FuncV1 EnCQueueFront, 0x0000c104
    def_FuncV1 DeCQueue, 0x0000c174
    def_FuncV1 PeekCQueue, 0x0000c1e0
    def_FuncV1 fdk_aac_set_BitMask, 0x000132b4
    def_FuncV1 fdk_aac_set_SineTable480, 0x000132c0
    def_FuncV1 fdk_aac_set_SineTable512, 0x000132cc
    def_FuncV1 fdk_aac_set_RotVectorReal240, 0x000132d8
    def_FuncV1 fdk_aac_set_RotVectorImag240, 0x000132e4
    def_FuncV1 fdk_aac_set_RotVectorReal480, 0x000132f0
    def_FuncV1 fdk_aac_set_RotVectorImag480, 0x000132fc
    def_FuncV1 fdk_aac_set_windowSlopes, 0x00013308
    def_FuncV1 fdk_aac_set_ldIntCoeff, 0x00013314
    def_FuncV1 fdk_aac_set_invSqrtTab, 0x00013320
    def_FuncV1 _Z13CalcInvLdDatal, 0x00013344
    def_FuncV1 _Z5fLog2li, 0x00013880
    def_FuncV1 _Z5fLog2liPi, 0x000137d0
    def_FuncV1 _Z8sqrtFixpl, 0x00013470
    def_FuncV1 _Z12invSqrtNorm2lPi, 0x00013408
    def_FuncV1 _Z8fDivNormllPi, 0x00013550
    def_FuncV1 _Z8fDivNormll, 0x000135a4
    def_FuncV1 _Z16fDivNormHighPrecllPi, 0x000135ec
    def_FuncV1 _Z12LdDataVectorPlS_i, 0x000138b0
    def_FuncV1 _Z9schur_divlli, 0x000134ac
    def_FuncV1 _Z9fMultNormllPi, 0x00013514
    def_FuncV1 _Z10CalcLdDatal, 0x000138a8
    def_FuncV1 _Z9CalcLdInti, 0x000133cc
    def_FuncV1 _Z14InitInvSqrtTabv, 0x00013404
    def_FuncV1 _Z7fPowIntliiPi, 0x00013740
    def_FuncV1 _Z4fPowliliPi, 0x00013848
    def_FuncV1 _Z9InitLdIntv, 0x000133c8
    def_FuncV1 _Z5f2PowliPi, 0x00013640
    def_FuncV1 _Z5f2Powli, 0x000136a8
    def_FuncV1 FDK_put, 0x00013b28
    def_FuncV1 FDK_putBwd, 0x00013c18
    def_FuncV1 FDK_getValidBits, 0x00013db0
    def_FuncV1 FDK_pushBack, 0x00013d24
    def_FuncV1 FDK_pushForward, 0x00013d4c
    def_FuncV1 FDK_ResetBitBuffer, 0x000139c8
    def_FuncV1 FDK_get, 0x000139d8
    def_FuncV1 FDK_InitBitBuffer, 0x000139a8
    def_FuncV1 FDK_Fetch, 0x00013f2c
    def_FuncV1 FDK_getFreeBits, 0x00013db4
    def_FuncV1 FDK_get32, 0x000139e4
    def_FuncV1 FDK_Feed, 0x00013dc4
    def_FuncV1 _Z17FDKgetWindowSlopeii, 0x00013f88
    def_FuncV1 _Z23getBitstreamElementList17AUDIO_OBJECT_TYPEahh, 0x00013ff0
    def_FuncV1 _Z6dct_IVPliPi, 0x000140dc
    def_FuncV1 _Z3fftiPlPi, 0x000142bc
    def_FuncV1 _Z7dit_fftPliPK8FIXP_SPKi, 0x00014d18
    def_FuncV1 _ZL9getTablesPPK8FIXP_SPKS2_Pii, 0x00015160
