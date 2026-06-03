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

// Fast high quality random.

#include "beco_rand64.h"


static uint64_t beco_random_state = 1;

static uint64_t xorshift64s(uint64_t *state)
{
    uint64_t x = *state;
    x ^= x >> 12; // a
    x ^= x << 25; // b
    x ^= x >> 27; // c
    *state = x;
    return x * UINT64_C(0x2545F4914F6CDD1D);
}

void beco_set_random_seed(uint64_t seed)
{
    beco_random_state = seed ^ UINT64_C(0xfbaf7d4a727d36f1);
}

uint64_t beco_rand64(void)
{
    return xorshift64s(&beco_random_state);
}

uint32_t beco_rand32(void)
{
    return (uint32_t)(beco_rand64() >> 32);
}


uint8_t beco_rand_uint8(void)
{
    return (uint8_t)(beco_rand32() & 0xFF);
}

int8_t beco_rand_int8(void)
{
    return (int8_t)beco_rand_uint8();
}

uint16_t beco_rand_uint16(void)
{
    return (uint16_t)(beco_rand32() & 0xFFFF);
}

int16_t beco_rand_int16(void)
{
    return (int16_t)beco_rand_uint16();
}

uint32_t beco_rand_uint32(void)
{
    return beco_rand32();
}

int32_t beco_rand_int32(void)
{
    uint32_t value = beco_rand32() & 0x3FFFFFFF;
    return (int32_t)(value - (1 << 26));
}


void beco_vec_fill_random_u8(uint8_t *p, int n)
{
    for (int i = 0; i < n; i++) {
        *p++ = beco_rand_uint8();
    }
}

void beco_vec_fill_random_i8(int8_t *p, int n)
{
    for (int i = 0; i < n; i++) {
        *p++ = beco_rand_int8();
    }
}

void beco_vec_fill_random_u16(uint16_t *p, int n)
{
    for (int i = 0; i < n; i++) {
        *p++ = beco_rand_uint16();
    }
}

void beco_vec_fill_random_i16(int16_t *p, int n)
{
    for (int i = 0; i < n; i++) {
        *p++ = beco_rand_int16();
    }
}

void beco_vec_fill_random_u32(uint32_t *p, int n)
{
    for (int i = 0; i < n; i++) {
        *p++ = beco_rand_uint32();
    }
}

void beco_vec_fill_random_i32(int32_t *p, int n)
{
    for (int i = 0; i < n; i++) {
        *p++ = beco_rand_int32();
    }
}
