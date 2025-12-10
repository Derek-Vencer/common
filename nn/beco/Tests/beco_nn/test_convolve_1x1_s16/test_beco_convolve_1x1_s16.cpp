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
#include "beco_nn/naive_nnfunctions.h"

#include "convolve_1x1_s16_test01.h"
#include "convolve_1x1_s16_test02.h"
#include "convolve_1x1_s16_test03.h"

// #define PRINT_RESULTS
#define FILL_RANDOM


void compare_convolve_1x1_s16(void)
{
    const int IH = 3;
    const int IW = 32;
    const int ICH = 8;
    const int OH = 3;
    const int OW = 32;
    const int OCH = 16;
    int16_t Im_in[ICH*IH*IW];
    int8_t wt[ICH*OCH];
    int32_t bias_beco[OCH];
    int64_t bias_cmsis[OCH];
    int32_t multiplier[OCH];
    int32_t shift[OCH];

    // Fill input data
#ifdef FILL_RANDOM
    beco_set_random_seed(1);
    beco_vec_fill_random_i16(Im_in, ICH*IH*IW);
    beco_vec_fill_random_i8(wt, ICH*OCH);
#else
    for (int i = 0; i < ICH*IH*IW; i++) { Im_in[i] = i % 100 - 50; }
    for (int i = 0; i < ICH*OCH; i++) { wt[i] = i % 100 - 50; }
#endif
    for (int i = 0; i < OCH; i++) { multiplier[i] = NN_Q31_MAX;
                                    shift[i] = -8;
                                    bias_beco[i] = 500 * (i % 50 - 25);
                                    bias_cmsis[i] = 500 * (i % 50 - 25); }

    // Init Conv1x1 params
    cmsis_nn_context ctx;
    cmsis_nn_conv_params conv_params;
    cmsis_nn_per_channel_quant_params quant_params;
    cmsis_nn_dims input_dims;
    cmsis_nn_dims filter_dims;
    cmsis_nn_dims bias_dims;
    cmsis_nn_dims output_dims;

    input_dims.n = 1;
    input_dims.w = IW;
    input_dims.h = IH;
    input_dims.c = ICH;
    filter_dims.w = 1;
    filter_dims.h = 1;
    filter_dims.c = ICH;
    output_dims.w = OW;
    output_dims.h = OH;
    output_dims.c = OCH;

    conv_params.padding.w = 0;
    conv_params.padding.h = 0;
    conv_params.stride.w = 1;
    conv_params.stride.h = 1;
    conv_params.dilation.w = 1;
    conv_params.dilation.h = 1;

    conv_params.input_offset = 0;
    conv_params.output_offset = 0;
    conv_params.activation.min = NN_Q15_MIN;
    conv_params.activation.max = NN_Q15_MAX;
    quant_params.multiplier = multiplier;
    quant_params.shift = shift;

    /*-------------------------- beco_nn --------------------------*/
    int32_t buf_size = beco_convolve_1x1_s16_get_buffer_size(&input_dims, &filter_dims);
    ctx.buf = malloc(buf_size);
    int16_t out_beco[OCH*IH*IW];

    BECO_GET_TIME_ENTER(1);
    beco_convolve_1x1_s16(&ctx, &conv_params, &quant_params, &input_dims, Im_in,
                          &filter_dims, wt, &bias_dims, bias_beco, &output_dims, out_beco);
    BECO_GET_TIME_EXIT(1, "beco_convolve_1x1_s16");
    free(ctx.buf);

    /*-------------------------- cmsis --------------------------*/
    int16_t out_cmsis[OCH*IH*IW];
    const cmsis_nn_bias_data bias_data = {bias_cmsis, false};
    CMSIS_Input_Transform(Im_in, wt, ICH, IW, IH, OCH, 1, 1);
    buf_size = arm_convolve_s16_get_buffer_size(&input_dims, &filter_dims);
    ctx.buf = malloc(buf_size);
    ctx.size = 0;

    BECO_GET_TIME_ENTER(2);
    arm_convolve_s16(&ctx, &conv_params, &quant_params, &input_dims, Im_in, &filter_dims,
                     wt, &bias_dims, &bias_data, &output_dims, out_cmsis);
    BECO_GET_TIME_EXIT(2, "arm_convolve_s16");
    CMSIS_Output_Transform(out_cmsis, OCH, OW, OH);
    free(ctx.buf);

#ifdef PRINT_RESULTS
    TRACE(3, "beco results:");
    print_matrix(out_beco, OH*OW, OCH, OH*OW);

    TRACE(3, "cmsis results:");
    print_matrix(out_cmsis, OH*OW, OCH, OH*OW);
#endif

    // Compare reference to beco
    bool match = true;
    for (int i = 0; i < OCH*OH*OW; i++) {
        if (abs(out_beco[i] - out_cmsis[i]) > 1) {
            match = false;
            break;
        }
    }
    if (match) {
        TRACE(3, "Beco result MATCH Reference");
    }
    else {
        TRACE(3, "Beco result doesn't match Reference");
    }
}

void compare_convolve_1x1_s16_test01(void)
{
    // Init Conv1x1 params
    cmsis_nn_context ctx;
    cmsis_nn_conv_params conv_params;
    cmsis_nn_per_channel_quant_params quant_params;
    cmsis_nn_dims input_dims;
    cmsis_nn_dims filter_dims;
    cmsis_nn_dims bias_dims = {0, 0, 0, 0};
    cmsis_nn_dims output_dims;

    input_dims.n = 1;
    input_dims.w = CONV_1X1_S16_TEST01_INPUT_W;
    input_dims.h = CONV_1X1_S16_TEST01_INPUT_H;
    input_dims.c = CONV_1X1_S16_TEST01_INPUT_CH;
    filter_dims.w = 1;
    filter_dims.h = 1;
    filter_dims.c = CONV_1X1_S16_TEST01_INPUT_CH;
    output_dims.w = CONV_1X1_S16_TEST01_OUTPUT_W;
    output_dims.h = CONV_1X1_S16_TEST01_OUTPUT_H;
    output_dims.c = CONV_1X1_S16_TEST01_OUTPUT_CH;

    conv_params.padding.w = 0;
    conv_params.padding.h = 0;
    conv_params.stride.w = 1;
    conv_params.stride.h = 1;
    conv_params.dilation.w = 1;
    conv_params.dilation.h = 1;

    conv_params.input_offset = CONV_1X1_S16_TEST01_INPUT_OFFSET;
    conv_params.output_offset = CONV_1X1_S16_TEST01_OUTPUT_OFFSET;
    conv_params.activation.min = NN_Q15_MIN;
    conv_params.activation.max = NN_Q15_MAX;
    quant_params.multiplier = conv_1x1_s16_test01_multiplier;
    quant_params.shift = conv_1x1_s16_test01_shift;

    /*-------------------------- beco_nn --------------------------*/
    int32_t buf_size = beco_convolve_1x1_s16_get_buffer_size(&input_dims, &filter_dims);
    ctx.buf = malloc(buf_size);
    int16_t out_beco[CONV_1X1_S16_TEST01_OUTPUT_CH*CONV_1X1_S16_TEST01_OUTPUT_H*CONV_1X1_S16_TEST01_OUTPUT_W];

    BECO_GET_TIME_ENTER(1);
    beco_convolve_1x1_s16(&ctx, &conv_params, &quant_params, &input_dims,
                          conv_1x1_s16_test01_beco_input, &filter_dims, conv_1x1_s16_test01_beco_weight,
                          &bias_dims, conv_1x1_s16_test01_beco_bias, &output_dims, out_beco);
    BECO_GET_TIME_EXIT(1, "test01 beco_convolve_1x1_s16");
    free(ctx.buf);

    /*-------------------------- cmsis --------------------------*/
    int16_t out_cmsis[CONV_1X1_S16_TEST01_OUTPUT_CH*CONV_1X1_S16_TEST01_OUTPUT_H*CONV_1X1_S16_TEST01_OUTPUT_W];
    const cmsis_nn_bias_data bias_data = {conv_1x1_s16_test01_cmsis_bias, false};
    buf_size = arm_convolve_s16_get_buffer_size(&input_dims, &filter_dims);
    ctx.buf = malloc(buf_size);
    ctx.size = 0;

    BECO_GET_TIME_ENTER(2);
    arm_convolve_s16(&ctx, &conv_params, &quant_params, &input_dims,
                     conv_1x1_s16_test01_cmsis_input, &filter_dims, conv_1x1_s16_test01_cmsis_weight,
                     &bias_dims, &bias_data, &output_dims, out_cmsis);
    BECO_GET_TIME_EXIT(2, "test01 arm_convolve_s16");
    free(ctx.buf);
    CMSIS_Output_Transform(out_cmsis, CONV_1X1_S16_TEST01_OUTPUT_CH, CONV_1X1_S16_TEST01_OUTPUT_W, CONV_1X1_S16_TEST01_OUTPUT_H);

#ifdef PRINT_RESULTS
    TRACE(3, "beco results:");
    print_matrix(out_beco, CONV_1X1_S16_TEST01_OUTPUT_H*CONV_1X1_S16_TEST01_OUTPUT_W,
                 CONV_1X1_S16_TEST01_OUTPUT_CH,
                 CONV_1X1_S16_TEST01_OUTPUT_H*CONV_1X1_S16_TEST01_OUTPUT_W);

    TRACE(3, "cmsis results:");
    print_matrix(out_cmsis, CONV_1X1_S16_TEST01_OUTPUT_H*CONV_1X1_S16_TEST01_OUTPUT_W,
                 CONV_1X1_S16_TEST01_OUTPUT_CH,
                 CONV_1X1_S16_TEST01_OUTPUT_H*CONV_1X1_S16_TEST01_OUTPUT_W);
#endif

    // Compare reference to beco
    bool match = true;
    for (int i = 0; i < CONV_1X1_S16_TEST01_OUTPUT_CH*CONV_1X1_S16_TEST01_OUTPUT_H*CONV_1X1_S16_TEST01_OUTPUT_W; i++) {
        if (abs(out_beco[i] - out_cmsis[i]) > 1) {
            match = false;
            break;
        }
    }
    if (match) {
        TRACE(3, "Beco result MATCH Reference");
    }
    else {
        TRACE(3, "Beco result doesn't match Reference");
    }
}

void compare_convolve_1x1_s16_test02(void)
{
    // Init Conv1x1 params
    cmsis_nn_context ctx;
    cmsis_nn_conv_params conv_params;
    cmsis_nn_per_channel_quant_params quant_params;
    cmsis_nn_dims input_dims;
    cmsis_nn_dims filter_dims;
    cmsis_nn_dims bias_dims = {0, 0, 0, 0};
    cmsis_nn_dims output_dims;

    input_dims.n = 1;
    input_dims.w = CONV_1X1_S16_TEST02_INPUT_W;
    input_dims.h = CONV_1X1_S16_TEST02_INPUT_H;
    input_dims.c = CONV_1X1_S16_TEST02_INPUT_CH;
    filter_dims.w = 1;
    filter_dims.h = 1;
    filter_dims.c = CONV_1X1_S16_TEST02_INPUT_CH;
    output_dims.w = CONV_1X1_S16_TEST02_OUTPUT_W;
    output_dims.h = CONV_1X1_S16_TEST02_OUTPUT_H;
    output_dims.c = CONV_1X1_S16_TEST02_OUTPUT_CH;

    conv_params.padding.w = 0;
    conv_params.padding.h = 0;
    conv_params.stride.w = 1;
    conv_params.stride.h = 1;
    conv_params.dilation.w = 1;
    conv_params.dilation.h = 1;

    conv_params.input_offset = CONV_1X1_S16_TEST02_INPUT_OFFSET;
    conv_params.output_offset = CONV_1X1_S16_TEST02_OUTPUT_OFFSET;
    conv_params.activation.min = NN_Q15_MIN;
    conv_params.activation.max = NN_Q15_MAX;
    quant_params.multiplier = conv_1x1_s16_test02_multiplier;
    quant_params.shift = conv_1x1_s16_test02_shift;

    /*-------------------------- beco_nn --------------------------*/
    int32_t buf_size = beco_convolve_1x1_s16_get_buffer_size(&input_dims, &filter_dims);
    ctx.buf = malloc(buf_size);
    int16_t out_beco[CONV_1X1_S16_TEST02_OUTPUT_CH*CONV_1X1_S16_TEST02_OUTPUT_H*CONV_1X1_S16_TEST02_OUTPUT_W];

    BECO_GET_TIME_ENTER(1);
    beco_convolve_1x1_s16(&ctx, &conv_params, &quant_params, &input_dims,
                          conv_1x1_s16_test02_beco_input, &filter_dims, conv_1x1_s16_test02_beco_weight,
                          &bias_dims, conv_1x1_s16_test02_beco_bias, &output_dims, out_beco);
    BECO_GET_TIME_EXIT(1, "test02 beco_convolve_1x1_s16");
    free(ctx.buf);

    /*-------------------------- cmsis --------------------------*/
    int16_t out_cmsis[CONV_1X1_S16_TEST02_OUTPUT_CH*CONV_1X1_S16_TEST02_OUTPUT_H*CONV_1X1_S16_TEST02_OUTPUT_W];
    const cmsis_nn_bias_data bias_data = {conv_1x1_s16_test02_cmsis_bias, false};
    buf_size = arm_convolve_s16_get_buffer_size(&input_dims, &filter_dims);
    ctx.buf = malloc(buf_size);
    ctx.size = 0;

    BECO_GET_TIME_ENTER(2);
    arm_convolve_s16(&ctx, &conv_params, &quant_params, &input_dims,
                     conv_1x1_s16_test02_cmsis_input, &filter_dims, conv_1x1_s16_test02_cmsis_weight,
                     &bias_dims, &bias_data, &output_dims, out_cmsis);
    BECO_GET_TIME_EXIT(2, "test02 arm_convolve_s16");
    free(ctx.buf);
    CMSIS_Output_Transform(out_cmsis, CONV_1X1_S16_TEST02_OUTPUT_CH, CONV_1X1_S16_TEST02_OUTPUT_W, CONV_1X1_S16_TEST02_OUTPUT_H);

#ifdef PRINT_RESULTS
    TRACE(3, "beco results:");
    print_matrix(out_beco, CONV_1X1_S16_TEST02_OUTPUT_H*CONV_1X1_S16_TEST02_OUTPUT_W,
                 CONV_1X1_S16_TEST02_OUTPUT_CH,
                 CONV_1X1_S16_TEST02_OUTPUT_H*CONV_1X1_S16_TEST02_OUTPUT_W);

    TRACE(3, "cmsis results:");
    print_matrix(out_cmsis, CONV_1X1_S16_TEST02_OUTPUT_H*CONV_1X1_S16_TEST02_OUTPUT_W,
                 CONV_1X1_S16_TEST02_OUTPUT_CH,
                 CONV_1X1_S16_TEST02_OUTPUT_H*CONV_1X1_S16_TEST02_OUTPUT_W);
#endif

    // Compare reference to beco
    bool match = true;
    for (int i = 0; i < CONV_1X1_S16_TEST02_OUTPUT_CH*CONV_1X1_S16_TEST02_OUTPUT_H*CONV_1X1_S16_TEST02_OUTPUT_W; i++) {
        if (abs(out_beco[i] - out_cmsis[i]) > 1) {
            match = false;
            break;
        }
    }
    if (match) {
        TRACE(3, "Beco result MATCH Reference");
    }
    else {
        TRACE(3, "Beco result doesn't match Reference");
    }
}

void compare_convolve_1x1_s16_test03(void)
{
    // Init Conv1x1 params
    cmsis_nn_context ctx;
    cmsis_nn_conv_params conv_params;
    cmsis_nn_per_channel_quant_params quant_params;
    cmsis_nn_dims input_dims;
    cmsis_nn_dims filter_dims;
    cmsis_nn_dims bias_dims = {0, 0, 0, 0};
    cmsis_nn_dims output_dims;

    input_dims.n = 1;
    input_dims.w = CONV_1X1_S16_TEST03_INPUT_W;
    input_dims.h = CONV_1X1_S16_TEST03_INPUT_H;
    input_dims.c = CONV_1X1_S16_TEST03_INPUT_CH;
    filter_dims.w = 1;
    filter_dims.h = 1;
    filter_dims.c = CONV_1X1_S16_TEST03_INPUT_CH;
    output_dims.w = CONV_1X1_S16_TEST03_OUTPUT_W;
    output_dims.h = CONV_1X1_S16_TEST03_OUTPUT_H;
    output_dims.c = CONV_1X1_S16_TEST03_OUTPUT_CH;

    conv_params.padding.w = 0;
    conv_params.padding.h = 0;
    conv_params.stride.w = 1;
    conv_params.stride.h = 1;
    conv_params.dilation.w = 1;
    conv_params.dilation.h = 1;

    conv_params.input_offset = CONV_1X1_S16_TEST03_INPUT_OFFSET;
    conv_params.output_offset = CONV_1X1_S16_TEST03_OUTPUT_OFFSET;
    conv_params.activation.min = NN_Q15_MIN;
    conv_params.activation.max = NN_Q15_MAX;
    quant_params.multiplier = conv_1x1_s16_test03_multiplier;
    quant_params.shift = conv_1x1_s16_test03_shift;

    /*-------------------------- beco_nn --------------------------*/
    int32_t buf_size = beco_convolve_1x1_s16_get_buffer_size(&input_dims, &filter_dims);
    ctx.buf = malloc(buf_size);
    int16_t out_beco[CONV_1X1_S16_TEST03_OUTPUT_CH*CONV_1X1_S16_TEST03_OUTPUT_H*CONV_1X1_S16_TEST03_OUTPUT_W];

    BECO_GET_TIME_ENTER(1);
    beco_convolve_1x1_s16(&ctx, &conv_params, &quant_params, &input_dims,
                          conv_1x1_s16_test03_beco_input, &filter_dims, conv_1x1_s16_test03_beco_weight,
                          &bias_dims, conv_1x1_s16_test03_beco_bias, &output_dims, out_beco);
    BECO_GET_TIME_EXIT(1, "test03 beco_convolve_1x1_s16");
    free(ctx.buf);

    /*-------------------------- cmsis --------------------------*/
    int16_t out_cmsis[CONV_1X1_S16_TEST03_OUTPUT_CH*CONV_1X1_S16_TEST03_OUTPUT_H*CONV_1X1_S16_TEST03_OUTPUT_W];
    const cmsis_nn_bias_data bias_data = {conv_1x1_s16_test03_cmsis_bias, false};
    buf_size = arm_convolve_s16_get_buffer_size(&input_dims, &filter_dims);
    ctx.buf = malloc(buf_size);
    ctx.size = 0;

    BECO_GET_TIME_ENTER(2);
    arm_convolve_s16(&ctx, &conv_params, &quant_params, &input_dims,
                     conv_1x1_s16_test03_cmsis_input, &filter_dims, conv_1x1_s16_test03_cmsis_weight,
                     &bias_dims, &bias_data, &output_dims, out_cmsis);
    BECO_GET_TIME_EXIT(2, "test03 arm_convolve_s16");
    CMSIS_Output_Transform(out_cmsis, CONV_1X1_S16_TEST03_OUTPUT_CH, CONV_1X1_S16_TEST03_OUTPUT_W, CONV_1X1_S16_TEST03_OUTPUT_H);

#ifdef PRINT_RESULTS
    TRACE(3, "beco results:");
    print_matrix(out_beco, CONV_1X1_S16_TEST03_OUTPUT_H*CONV_1X1_S16_TEST03_OUTPUT_W,
                 CONV_1X1_S16_TEST03_OUTPUT_CH,
                 CONV_1X1_S16_TEST03_OUTPUT_H*CONV_1X1_S16_TEST03_OUTPUT_W);

    TRACE(3, "cmsis results:");
    print_matrix(out_cmsis, CONV_1X1_S16_TEST03_OUTPUT_H*CONV_1X1_S16_TEST03_OUTPUT_W,
                 CONV_1X1_S16_TEST03_OUTPUT_CH,
                 CONV_1X1_S16_TEST03_OUTPUT_H*CONV_1X1_S16_TEST03_OUTPUT_W);
#endif

    // Compare reference to beco
    bool match = true;
    for (int i = 0; i < CONV_1X1_S16_TEST03_OUTPUT_CH*CONV_1X1_S16_TEST03_OUTPUT_H*CONV_1X1_S16_TEST03_OUTPUT_W; i++) {
        if (abs(out_beco[i] - out_cmsis[i]) > 1) {
            match = false;
            break;
        }
    }
    if (match) {
        TRACE(3, "Beco result MATCH Reference");
    }
    else {
        TRACE(3, "Beco result doesn't match Reference");
    }
}


void test_beco_convolve_1x1_s16(void)
{
    TRACE(3, "===== test_beco_convolve_1x1_s16 START =====");
    compare_convolve_1x1_s16();
    compare_convolve_1x1_s16_test01();
    compare_convolve_1x1_s16_test02();
    compare_convolve_1x1_s16_test03();
    TRACE(3, "===== test_beco_convolve_1x1_s16 END =====");
}
