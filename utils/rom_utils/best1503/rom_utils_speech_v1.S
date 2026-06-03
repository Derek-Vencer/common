/***************************************************************************
 *
 * Copyright 2015-2023 BES.
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

def_FuncExt  v1,  speech_dc_filter_create,                        0x001901c4
def_FuncExt  v1,  speech_dc_filter_create_with_custom_allocator,  0x00190004
def_FuncExt  v1,  speech_dc_filter_destroy,                       0x00190388
def_FuncExt  v1,  speech_dc_filter_set_config,                    0x001903a0
def_FuncExt  v1,  speech_dc_filter_ctl,                           0x00190540
def_FuncExt  v1,  speech_dc_filter_process,                       0x001906dc
def_FuncExt  v1,  speech_dc_filter_process_int24,                 0x001907d0
def_FuncExt  v1,  speech_dc_filter_dump,                          0x0019059c
def_FuncExt  v1,  speech_dc_filter_get_required_mips,             0x001905bc
def_FuncExt  v1,  ec2float_create,                                0x001909f0
def_FuncExt  v1,  ec2float_create_with_custom_allocator,          0x001907d8
def_FuncExt  v1,  ec2float_destroy,                               0x00190c0c
def_FuncExt  v1,  ec2float_set_config,                            0x00190c70
def_FuncExt  v1,  ec2float_process,                               0x00190e40
def_FuncExt  v1,  ec2float_set_external_vad,                      0x00190fe4
def_FuncExt  v1,  ec2float_get_fir_filter_coeffs,                 0x00190ff8
def_FuncExt  v1,  ec2float_get_fir_filter_freq_coeffs,            0x00191000
def_FuncExt  v1,  ec2float_get_adaptive_filter_length,            0x00191008
def_FuncExt  v1,  ec2float_get_adaptive_filter_data,              0x00191014
def_FuncExt  v1,  ec2float_get_required_mips,                     0x00191028
def_FuncExt  v1,  ec2float_get_snr,                               0x001910a0
def_FuncExt  v1,  ec2float_get_noise_power,                       0x001910a8
def_FuncExt  v1,  speech_ns2float_create,                         0x00191588
def_FuncExt  v1,  speech_ns2float_create_with_custom_allocator,   0x001913e0
def_FuncExt  v1,  speech_ns2float_destroy,                        0x001910b0
def_FuncExt  v1,  speech_ns2float_set_config,                     0x00191144
def_FuncExt  v1,  speech_ns2float_process,                        0x001920fc
def_FuncExt  v1,  speech_ns2float_process_int24,                  0x00192b74
def_FuncExt  v1,  speech_ns2float_set_echo_state,                 0x001915ac
def_FuncExt  v1,  speech_ns2float_set_echo_suppress,              0x001915c4
def_FuncExt  v1,  speech_ns2float_set_external_vad,               0x00191660
def_FuncExt  v1,  speech_ns2float_get_required_mips,              0x00191674
def_FuncExt  v1,  eq_arm_init,                                    0x00192f04
def_FuncExt  v1,  eq_arm_init_with_custom_allocator,              0x00192e98
def_FuncExt  v1,  multi_eq_arm_init,                              0x00192f24
def_FuncExt  v1,  multi_eq_arm_init_with_custom_allocator,        0x00192e34
def_FuncExt  v1,  eq_arm_destroy,                                 0x00192b7c
def_FuncExt  v1,  eq_arm_set_config,                              0x00192bbc
def_FuncExt  v1,  eq_arm_process,                                 0x00192f88
def_FuncExt  v1,  eq_arm_process_int24,                           0x00193064
def_FuncExt  v1,  eq_arm_process_float,                           0x00193134
def_FuncExt  v1,  eq_arm_process2,                                0x001931c4
def_FuncExt  v1,  eq_arm_process2_int24,                          0x001932cc
def_FuncExt  v1,  eq_arm_process2_float,                          0x001933cc
def_FuncExt  v1,  eq_arm_get_required_mips,                       0x001934c8
def_FuncExt  v1,  speech_ns8_create,                              0x00059bd4
def_FuncExt  v1,  speech_ns8_destory,                             0x00058b2c
def_FuncExt  v1,  speech_ns8_process,                             0x00058c2c
def_FuncExt  v1,  speech_ns8_set_config,                          0x00059110
def_FuncExt  v1,  speech_2mic_ns8_create,                         0x00056004
def_FuncExt  v1,  speech_2mic_ns8_destroy,                        0x00056474
def_FuncExt  v1,  speech_2mic_ns8_process,                        0x00056544
def_FuncExt  v1,  speech_2mic_ns8_get_required_mips,              0x00056d1c
def_FuncExt  v1,  speech_2mic_ns8_get_delay,                      0x00056d28
