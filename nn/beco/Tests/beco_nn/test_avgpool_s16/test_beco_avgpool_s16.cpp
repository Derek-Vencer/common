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

// #define PRINT_RESULTS
#define FILL_RANDOM

void compare_avgpool_s16(void)
{
    const int CH = 64;
    const int IH = 8;
    const int IW = 8;
    const int OH = 4;
    const int OW = 4;
    const int PX = 0;
    const int PY = 0;
    const int SX = 2;
    const int SY = 2;
    const int KX = 2;
    const int KY = 2;
    int16_t src[CH*IH*IW];
    int16_t dst_beco[CH*OH*OW];
    int16_t dst_cmsis[CH*OH*OW];

    // Fill input data
#ifdef FILL_RANDOM
    beco_set_random_seed(1);
    beco_vec_fill_random_i16(src, CH*IH*IW);
#else
    for (int i = 0; i < CH*IH*IW; i++) { src[i] = i % 100 - 50; }
#endif

    // Init Pool params
    cmsis_nn_context ctx;
    cmsis_nn_pool_params pool_params;
    cmsis_nn_dims input_dims;
    cmsis_nn_dims filter_dims;
    cmsis_nn_dims output_dims;

    input_dims.n = 1;
    input_dims.w = IW;
    input_dims.h = IH;
    input_dims.c = CH;
    filter_dims.w = KX;
    filter_dims.h = KY;
    output_dims.w = OW;
    output_dims.h = OH;
    output_dims.c = CH;

    pool_params.padding.w = PX;
    pool_params.padding.h = PY;
    pool_params.stride.w = SX;
    pool_params.stride.h = SY;

    pool_params.activation.min = NN_Q15_MIN;
    pool_params.activation.max = NN_Q15_MAX;

    /*-------------------------- beco_nn --------------------------*/
    ctx.size = beco_avgpool_s16_get_buffer_size(OW, CH);
    ctx.buf = malloc(ctx.size);

    BECO_GET_TIME_ENTER(1);
    beco_avgpool_s16(&ctx, &pool_params, &input_dims, src, &filter_dims, &output_dims, dst_beco);
    BECO_GET_TIME_EXIT(1, "beco_avgpool_s16");
    free(ctx.buf);

    /*-------------------------- cmsis --------------------------*/
    ctx.size = arm_avgpool_s16_get_buffer_size(OW, CH);
    ctx.buf = malloc(ctx.size);

    BECO_GET_TIME_ENTER(2);
    arm_avgpool_s16(&ctx, &pool_params, &input_dims, src, &filter_dims, &output_dims, dst_cmsis);
    BECO_GET_TIME_EXIT(2, "arm_avgpool_s16");
    free(ctx.buf);

#ifdef PRINT_RESULTS
    TRACE(3, "beco results:");
    print_matrix(dst_beco, OW*CH, OH, OW*CH);

    TRACE(3, "cmsis results:");
    print_matrix(dst_cmsis, OW*CH, OH, OW*CH);
#endif

    // Compare reference to beco
    if (memcmp(dst_beco, dst_cmsis, sizeof(dst_beco)) != 0) {
        TRACE(3, "Beco result doesn't match Reference");
    } else {
        TRACE(3, "Beco result MATCH Reference");
    }
}

void test_beco_avgpool_s16(void)
{
    TRACE(3, "===== test_beco_avgpool_s16 START =====");
    compare_avgpool_s16();
    TRACE(3, "===== test_beco_avgpool_s16 END =====");
}
