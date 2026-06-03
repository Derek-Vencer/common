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
    def_FuncV1 set__impure_ptr, 0x0003edec
    def_FuncV1 sbc_frames_encode, 0x000138bc
    def_FuncV1 sbc_frames_decode, 0x000130b8
    def_FuncV1 sbc_decoder_init, 0x000115dc
    def_FuncV1 sbc_encoder_init, 0x000138b0
    def_FuncV1 sbc_frames_decode_out_sbsamples, 0x00013454
    def_FuncV1 sbc_frames_encode_with_sbsamples, 0x0001587c