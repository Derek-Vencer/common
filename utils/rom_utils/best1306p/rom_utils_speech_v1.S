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

def_FuncExt  v1,  speech_dc_filter_create,                        0x0005897c
def_FuncExt  v1,  speech_dc_filter_create_with_custom_allocator,  0x00058798
def_FuncExt  v1,  speech_dc_filter_destroy,                       0x00058b64
def_FuncExt  v1,  speech_dc_filter_set_config,                    0x00058b7c
def_FuncExt  v1,  speech_dc_filter_ctl,                           0x00058d1c
def_FuncExt  v1,  speech_dc_filter_process,                       0x00058eb8
def_FuncExt  v1,  speech_dc_filter_process_int24,                 0x00058fac
def_FuncExt  v1,  speech_dc_filter_dump,                          0x00058d78
def_FuncExt  v1,  speech_dc_filter_get_required_mips,             0x00058d98
def_FuncExt  v1,  ec2float_create,                                0x000591d8
def_FuncExt  v1,  ec2float_create_with_custom_allocator,          0x00058fb4
def_FuncExt  v1,  ec2float_destroy,                               0x00059400
def_FuncExt  v1,  ec2float_set_config,                            0x00059468
def_FuncExt  v1,  ec2float_process,                               0x00059638
def_FuncExt  v1,  ec2float_set_external_vad,                      0x000597dc
def_FuncExt  v1,  ec2float_get_fir_filter_coeffs,                 0x000597f0
def_FuncExt  v1,  ec2float_get_fir_filter_freq_coeffs,            0x000597f8
def_FuncExt  v1,  ec2float_get_adaptive_filter_length,            0x00059800
def_FuncExt  v1,  ec2float_get_adaptive_filter_data,              0x0005980c
def_FuncExt  v1,  ec2float_get_required_mips,                     0x00059820
def_FuncExt  v1,  ec2float_get_snr,                               0x00059898
def_FuncExt  v1,  ec2float_get_noise_power,                       0x000598a0
def_FuncExt  v1,  eq_arm_init,                                    0x00059c28
def_FuncExt  v1,  eq_arm_init_with_custom_allocator,              0x00059bbc
def_FuncExt  v1,  multi_eq_arm_init,                              0x00059c48
def_FuncExt  v1,  multi_eq_arm_init_with_custom_allocator,        0x00059b58
def_FuncExt  v1,  eq_arm_destroy,                                 0x000598a8
def_FuncExt  v1,  eq_arm_set_config,                              0x000598e8
def_FuncExt  v1,  eq_arm_process,                                 0x00059cac
def_FuncExt  v1,  eq_arm_process_int24,                           0x00059d88
def_FuncExt  v1,  eq_arm_process_float,                           0x00059e58
def_FuncExt  v1,  eq_arm_process2,                                0x00059ee8
def_FuncExt  v1,  eq_arm_process2_int24,                          0x00059ff0
def_FuncExt  v1,  eq_arm_process2_float,                          0x0005a0f0
def_FuncExt  v1,  eq_arm_get_required_mips,                       0x0005a1ec
def_FuncExt  v1,  speech_ns9_create,                              0x000579cc
def_FuncExt  v1,  speech_ns9_destory,                             0x00056914
def_FuncExt  v1,  speech_ns9_process,                             0x00056b74
def_FuncExt  v1,  speech_ns9_set_config,                          0x00056fbc
def_FuncExt  v1,  compexp_create,                                 0x0005a524
def_FuncExt  v1,  compexp_create_with_custom_allocator,           0x0005a4d4
def_FuncExt  v1,  compexp_destroy,                                0x0005a240
def_FuncExt  v1,  compexp_set_config,                             0x0005a258
def_FuncExt  v1,  compexp_process,                                0x0005a84c
def_FuncExt  v1,  compexp_process_adj_eq,                         0x0005a570
def_FuncExt  v1,  compexp_process_int24,                          0x0005ab60
def_FuncExt  v1,  compexp_get_required_mips,                      0x0005ae68
