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
#include "beco_print_matrix.hpp"

#include "transpose_depthwise_conv_s16_test01.h"

// #define PRINT_RESULTS


static void beco_transpose_depthwise_conv_s16_test01(void)
{
    // Init DW Conv params
    cmsis_nn_context ctx = {nullptr, 0};
    cmsis_nn_dw_conv_params dw_conv_params;
    cmsis_nn_per_channel_quant_params quant_params;
    cmsis_nn_dims input_dims;
    cmsis_nn_dims filter_dims;
    cmsis_nn_dims bias_dims = {0, 0, 0, 0};
    cmsis_nn_dims output_dims;

    input_dims.n = 1;
    input_dims.h = TRANSPOSE_DEPTH_S16_TEST01_INPUT_H;
    input_dims.w = TRANSPOSE_DEPTH_S16_TEST01_INPUT_W;
    input_dims.c = TRANSPOSE_DEPTH_S16_TEST01_INPUT_CH;
    filter_dims.h = TRANSPOSE_DEPTH_S16_TEST01_KERNEL_Y;
    filter_dims.w = TRANSPOSE_DEPTH_S16_TEST01_KERNEL_X;
    output_dims.h = TRANSPOSE_DEPTH_S16_TEST01_OUTPUT_H;
    output_dims.w = TRANSPOSE_DEPTH_S16_TEST01_OUTPUT_W;
    output_dims.c = TRANSPOSE_DEPTH_S16_TEST01_OUTPUT_CH;

    dw_conv_params.padding.w = TRANSPOSE_DEPTH_S16_TEST01_PADDING_X;
    dw_conv_params.padding.h = TRANSPOSE_DEPTH_S16_TEST01_PADDING_Y;
    dw_conv_params.stride.w = TRANSPOSE_DEPTH_S16_TEST01_STRIDE_X;
    dw_conv_params.stride.h = TRANSPOSE_DEPTH_S16_TEST01_STRIDE_Y;
    dw_conv_params.dilation.w = TRANSPOSE_DEPTH_S16_TEST01_DILATION_X;
    dw_conv_params.dilation.h = TRANSPOSE_DEPTH_S16_TEST01_DILATION_Y;
    dw_conv_params.ch_mult = 1;

    dw_conv_params.input_offset = TRANSPOSE_DEPTH_S16_TEST01_INPUT_OFFSET;
    dw_conv_params.output_offset = TRANSPOSE_DEPTH_S16_TEST01_OUTPUT_OFFSET;
    dw_conv_params.activation.min = NN_Q15_MIN;
    dw_conv_params.activation.max = NN_Q15_MAX;
    quant_params.multiplier = transpose_depth_s16_test01_multiplier;
    quant_params.shift = transpose_depth_s16_test01_shift;

    /*-------------------------- beco_nn --------------------------*/
    int16_t out_beco[TRANSPOSE_DEPTH_S16_TEST01_OUTPUT_CH*TRANSPOSE_DEPTH_S16_TEST01_OUTPUT_H*TRANSPOSE_DEPTH_S16_TEST01_OUTPUT_W];
    int32_t buf_size = beco_transpose_depthwise_conv_s16_get_buffer_size(
        &input_dims, &output_dims, &dw_conv_params, &filter_dims);
    ctx.buf = malloc(buf_size);

    BECO_GET_TIME_ENTER(1);
    beco_transpose_depthwise_conv_s16(&ctx, &dw_conv_params, &quant_params, &input_dims,
                                     transpose_depth_s16_test01_beco_input, &filter_dims,
                                     transpose_depth_s16_test01_beco_weight, &bias_dims,
                                     transpose_depth_s16_test01_beco_bias, &output_dims, out_beco);
    BECO_GET_TIME_EXIT(1, "test01 beco_transpose_depthwise_conv_s16");
    free(ctx.buf);

#ifdef PRINT_RESULTS
    TRACE(3, "beco results:%s", "");
    print_matrix(out_beco, TRANSPOSE_DEPTH_S16_TEST01_OUTPUT_H*TRANSPOSE_DEPTH_S16_TEST01_OUTPUT_W,
                 TRANSPOSE_DEPTH_S16_TEST01_OUTPUT_CH,
                 TRANSPOSE_DEPTH_S16_TEST01_OUTPUT_H*TRANSPOSE_DEPTH_S16_TEST01_OUTPUT_W);
#endif
}


extern "C"
void test_beco_transpose_depthwise_conv_s16(void)
{
    TRACE(3, "========== test_beco_transpose_depthwise_conv_s16 START ==========%s", "");
    beco_transpose_depthwise_conv_s16_test01();
    TRACE(3, "========== test_beco_transpose_depthwise_conv_s16 END ==========\n%s", "");
}

