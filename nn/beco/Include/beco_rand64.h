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

#pragma once

#ifndef _BECO_RAND64_H
#define _BECO_RAND64_H

#include "beco_common.h"

BECO_C_DECLARATIONS_START

void beco_set_random_seed(uint64_t seed);
uint64_t beco_rand64(void);
uint32_t beco_rand32(void);

uint8_t beco_rand_uint8(void);
int8_t beco_rand_int8(void);
uint16_t beco_rand_uint16(void);
int16_t beco_rand_int16(void);
uint32_t beco_rand_uint32(void);
int32_t beco_rand_int32(void);

void beco_vec_fill_random_u8(uint8_t *p, int n);
void beco_vec_fill_random_i8(int8_t *p, int n);
void beco_vec_fill_random_u16(uint16_t *p, int n);
void beco_vec_fill_random_i16(int16_t *p, int n);
void beco_vec_fill_random_u32(uint32_t *p, int n);
void beco_vec_fill_random_i32(int32_t *p, int n);

BECO_C_DECLARATIONS_END

#endif
