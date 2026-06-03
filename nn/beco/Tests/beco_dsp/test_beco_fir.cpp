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
#include "beco_print_matrix.hpp"
#include "beco_dsp/naive_dspfunctions.h"

// #define PRINT_RESULTS
#define FILL_RANDOM


void test_beco_fir_q15(void)
{
    const int numTaps = 256;
    const int blockSize = 256;

    int16_t pSrc[blockSize];
    int16_t pCoeffs[numTaps];
    int16_t pState[numTaps + blockSize];
    int16_t pDst_beco[blockSize];
    int16_t pDst_naive[blockSize];

    // Fill input data
#ifdef FILL_RANDOM
    beco_set_random_seed(1);
    beco_vec_fill_random_i16(pSrc, blockSize);
    beco_vec_fill_random_i16(pCoeffs, numTaps);
#else
    for (int i = 0; i < blockSize; i++) {
        pSrc[i] = i % 100 + 5000;
    }
    for (int i = 0; i < numTaps; i++) {
        pCoeffs[i] = i % 100 + 5000;
    }
#endif

    beco_fir_instance_q15 beco_instance;
    beco_fir_init_q15(&beco_instance, numTaps, pCoeffs, pState, blockSize);
    BECO_GET_TIME_ENTER(1);
    beco_fir_q15(&beco_instance, pSrc, pDst_beco, blockSize);
    BECO_GET_TIME_EXIT(1, "beco_fir_q15");

    memset(pState, 0, (numTaps + blockSize) * sizeof(int16_t));
    BECO_GET_TIME_ENTER(2);
    naive_fir_q15(pSrc, pCoeffs, pState, numTaps, blockSize, pDst_naive);
    BECO_GET_TIME_EXIT(2, "naive_fir_q15");

#ifdef PRINT_RESULTS
    TRACE(3, "beco results:");
    print_matrix(pDst_beco, blockSize, 1, blockSize);
    TRACE(3, "naive results:");
    print_matrix(pDst_naive, blockSize, 1, blockSize);
#endif

    // Compare results
    if (memcmp(pDst_beco, pDst_naive, sizeof(pDst_beco)) != 0) {
        TRACE(3, "Beco result doesn't match Reference");
    } else {
        TRACE(3, "Beco result MATCH Reference");
    }
}


void test_beco_fir(void)
{
    TRACE(3, "===== test_beco_fir START =====");
    test_beco_fir_q15();
    TRACE(3, "===== test_beco_fir END =====");
}
