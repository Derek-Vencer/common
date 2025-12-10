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

#include "beco_test.h"
#include "beco_rand64.h"
#include "beco_dsp/beco_matrix_functions.h"
#include "beco_dsp/naive_dspfunctions.h"
#include "beco_print_matrix.hpp"

// #define PRINT_RESULTS
#define FILL_RANDOM

void test_beco_mat_mult_i8xi8_i16_fastest(void)
{
    const int mat_m = 16;
    const int mat_n = 16;
    const int mat_k = 32;

    int8_t A[mat_m*mat_k];
    int8_t B[mat_k*mat_n];
    int16_t Cb[mat_m*mat_n];
    int16_t Cn[mat_m*mat_n];

    // Fill input data
#ifdef FILL_RANDOM
    beco_set_random_seed(1);
    beco_vec_fill_random_i8(A, mat_m*mat_k);
    beco_vec_fill_random_i8(B, mat_k*mat_n);
#else
    for (int i = 0; i < mat_m*mat_k; i++) {
        A[i] = i % 100 - 50;
    }
    for (int i = 0; i < mat_k*mat_n; i++) {
        B[i] = i % 100 - 50;
    }
#endif

    BECO_GET_TIME_ENTER(1);
    beco_mat_mult_i8xi8_i16_fastest(A, B, Cb, mat_m, mat_n, mat_k);
    BECO_GET_TIME_EXIT(1, "beco_mat_mult_i8xi8_i16_fastest");

    BECO_GET_TIME_ENTER(2);
    naive_mat_mult_i8xi8_i16(A, B, Cn, mat_m, mat_n, mat_k);
    BECO_GET_TIME_EXIT(2, "naive_mat_mult_i8xi8_i16");

#ifdef PRINT_RESULTS
    TRACE(3, "beco results:");
    print_matrix(Cb, mat_n, mat_m, mat_n);
    TRACE(3, "naive results:");
    print_matrix(Cn, mat_n, mat_m, mat_n);
#endif

    // Compare results
    if (memcmp(Cb, Cn, sizeof(Cn)) != 0) {
        TRACE(3, "Beco result doesn't match Reference");
    } else {
        TRACE(3, "Beco result MATCH Reference");
    }
}

void test_beco_mat_mult_u8xu8_u16_fastest(void)
{
    const int mat_m = 16;
    const int mat_n = 16;
    const int mat_k = 32;

    uint8_t A[mat_m*mat_k];
    uint8_t B[mat_k*mat_n];
    uint16_t Cb[mat_m*mat_n];
    uint16_t Cn[mat_m*mat_n];

    // Fill input data
#ifdef FILL_RANDOM
    beco_set_random_seed(1);
    beco_vec_fill_random_u8(A, mat_m*mat_k);
    beco_vec_fill_random_u8(B, mat_k*mat_n);
#else
    for (int i = 0; i < mat_m*mat_k; i++) {
        A[i] = i % 100 - 50;
    }
    for (int i = 0; i < mat_k*mat_n; i++) {
        B[i] = i % 100 - 50;
    }
#endif

    BECO_GET_TIME_ENTER(1);
    beco_mat_mult_u8xu8_u16_fastest(A, B, Cb, mat_m, mat_n, mat_k);
    BECO_GET_TIME_EXIT(1, "beco_mat_mult_u8xu8_u16_fastest");

    BECO_GET_TIME_ENTER(2);
    naive_mat_mult_u8xu8_u16(A, B, Cn, mat_m, mat_n, mat_k);
    BECO_GET_TIME_EXIT(2, "naive_mat_mult_u8xu8_u16");

#ifdef PRINT_RESULTS
    TRACE(3, "beco results:");
    print_matrix(Cb, mat_n, mat_m, mat_n);
    TRACE(3, "naive results:");
    print_matrix(Cn, mat_n, mat_m, mat_n);
#endif

    // Compare results
    if (memcmp(Cb, Cn, sizeof(Cn)) != 0) {
        TRACE(3, "Beco result doesn't match Reference");
    } else {
        TRACE(3, "Beco result MATCH Reference");
    }
}


void test_beco_mat_mult(void)
{
    TRACE(3, "===== test_beco_mat_mult START =====");
    test_beco_mat_mult_i8xi8_i16_fastest();
    test_beco_mat_mult_u8xu8_u16_fastest();
    TRACE(3, "===== test_beco_mat_mult END =====");
}
