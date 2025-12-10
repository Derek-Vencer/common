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
#include "dsp/matrix_functions.h"

#include "fully_connected_s16_test01.h"
#include "fully_connected_s16_test02.h"
#include "fully_connected_s16_test03.h"

// #define PRINT_RESULTS
#define FILL_RANDOM


void compare_fully_connected_s16(void)
{
    const int DIM_VEC = 128;
    const int DIM_COL = 512;
    int16_t pV[DIM_VEC];
    int8_t pM[DIM_VEC*DIM_COL];
    int8_t pM_cmsis[DIM_VEC*DIM_COL];
    int32_t bias_beco[DIM_COL];
    int64_t bias_cmsis[DIM_COL];

    // Fill input data
#ifdef FILL_RANDOM
    beco_set_random_seed(1);
    beco_vec_fill_random_i16(pV, DIM_VEC);
    beco_vec_fill_random_i8(pM, DIM_VEC*DIM_COL);
#else
    for(int i=0; i< DIM_VEC; i++) { pV[i] = i % 100 - 50; }
    for(int i=0; i< DIM_VEC*DIM_COL; i++) { pM[i] = i % 100 - 50; }
#endif
    for(int i=0; i< DIM_COL; i++) { bias_beco[i] = 500 * (i % 50 - 25);
                                    bias_cmsis[i] = 500 * (i % 50 - 25); }

    // Init FC params
    cmsis_nn_context ctx;
    cmsis_nn_fc_params fc_params;
    cmsis_nn_per_tensor_quant_params quant_params;
    cmsis_nn_dims input_dims;
    cmsis_nn_dims filter_dims;
    cmsis_nn_dims bias_dims;
    cmsis_nn_dims output_dims;

    input_dims.n = 1;
    input_dims.w = 1;
    input_dims.h = 1;
    input_dims.c = DIM_VEC;
    filter_dims.n = DIM_VEC;
    filter_dims.c = DIM_COL;
    output_dims.n = 1;
    output_dims.c = DIM_COL;

    fc_params.input_offset = 0;
    fc_params.filter_offset = 0;
    fc_params.output_offset = 0;
    fc_params.activation.min = NN_Q15_MIN;
    fc_params.activation.max = NN_Q15_MAX;

    quant_params.multiplier = NN_Q31_MAX;
    quant_params.shift = -8;

    /*------------------------------- beco --------------------------*/
    int32_t buf_size = beco_fully_connected_s16_get_buffer_size(&output_dims);
    ctx.buf = malloc(buf_size);
    int16_t pOut_beco[DIM_COL];

    BECO_GET_TIME_ENTER(1);
    beco_fully_connected_s16(
        &ctx, &fc_params, &quant_params, &input_dims, pV, &filter_dims, \
        pM, &bias_dims, bias_beco, &output_dims, pOut_beco);
    BECO_GET_TIME_EXIT(1, "beco_fully_connected_s16");
    free(ctx.buf);

    /*------------------------------- cmsis --------------------------*/
    // weight transpose
    arm_matrix_instance_q7 pISrc;
    pISrc.numRows = DIM_VEC;
    pISrc.numCols = DIM_COL;
    pISrc.pData = pM;
    arm_matrix_instance_q7 pIDst;
    pIDst.numRows = DIM_COL;
    pIDst.numCols = DIM_VEC;
    pIDst.pData = pM_cmsis;
    arm_mat_trans_q7(&pISrc, &pIDst);
    int16_t pOut_cmsis[DIM_COL];
    buf_size = arm_fully_connected_s16_get_buffer_size(&filter_dims);
    ctx.buf = malloc(buf_size);

    BECO_GET_TIME_ENTER(2);
    arm_fully_connected_s16(&ctx, &fc_params, &quant_params, &input_dims, pV, &filter_dims, \
                            pM_cmsis, &bias_dims, bias_cmsis, &output_dims, pOut_cmsis);
    BECO_GET_TIME_EXIT(2, "arm_fully_connected_s16");
    free(ctx.buf);

#ifdef PRINT_RESULTS
    TRACE(3, "beco results:");
    print_matrix(pOut_beco, DIM_COL, 1, DIM_COL);

    TRACE(3, "cmsis results:");
    print_matrix(pOut_cmsis, DIM_COL, 1, DIM_COL);
#endif

    // Compare reference to beco
    bool match = true;
    for (int i = 0; i < DIM_COL; i++) {
        if (abs(pOut_beco[i] - pOut_cmsis[i]) > 1) {
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

void compare_fully_connected_s16_test01(void)
{
    // Init FC params
    cmsis_nn_context ctx = {nullptr, 0};
    cmsis_nn_fc_params fc_params;
    cmsis_nn_per_tensor_quant_params quant_params;
    cmsis_nn_dims input_dims;
    cmsis_nn_dims filter_dims;
    cmsis_nn_dims bias_dims = {0, 0, 0, 0};
    cmsis_nn_dims output_dims;

    input_dims.n = 1;
    input_dims.w = 1;
    input_dims.h = 1;
    input_dims.c = FC_S16_TEST01_DIM_VEC;
    filter_dims.n = FC_S16_TEST01_DIM_VEC;
    filter_dims.c = FC_S16_TEST01_DIM_COL;
    output_dims.n = 1;
    output_dims.c = FC_S16_TEST01_DIM_COL;

    fc_params.input_offset = FC_S16_TEST01_INPUT_OFFSET;
    fc_params.filter_offset = 0;
    fc_params.output_offset = FC_S16_TEST01_OUTPUT_OFFSET;
    fc_params.activation.min = NN_Q15_MIN;
    fc_params.activation.max = NN_Q15_MAX;

    quant_params.multiplier = FC_S16_TEST01_MULTIPLIER;
    quant_params.shift = FC_S16_TEST01_SHIFT;

    /*------------------------------- beco -------------------------------*/
    int32_t buf_size = beco_fully_connected_s16_get_buffer_size(&output_dims);
    ctx.buf = malloc(buf_size);
    int16_t pOut_beco[FC_S16_TEST01_DIM_COL];

    BECO_GET_TIME_ENTER(1);
    beco_fully_connected_s16(
        &ctx, &fc_params, &quant_params, &input_dims, fc_s16_test01_input, &filter_dims, \
        fc_s16_test01_beco_weight, &bias_dims, fc_s16_test01_beco_bias, &output_dims, pOut_beco);
    BECO_GET_TIME_EXIT(1, "test01 beco_fully_connected_s16");

    free(ctx.buf);

    /*------------------------------- cmsis -------------------------------*/
    int16_t pOut_cmsis[FC_S16_TEST01_DIM_COL];
    buf_size = arm_fully_connected_s16_get_buffer_size(&filter_dims);
    ctx.buf = malloc(buf_size);

    BECO_GET_TIME_ENTER(2);
    arm_fully_connected_s16(
        &ctx, &fc_params, &quant_params, &input_dims, fc_s16_test01_input, &filter_dims, \
        fc_s16_test01_cmsis_weight, &bias_dims, fc_s16_test01_cmsis_bias, &output_dims, pOut_cmsis);
    BECO_GET_TIME_EXIT(2, "test01 arm_fully_connected_s16");
    free(ctx.buf);

#ifdef PRINT_RESULTS
    TRACE(3, "beco test01 results:");
    print_matrix(pOut_beco, FC_S16_TEST01_DIM_COL, 1, FC_S16_TEST01_DIM_COL);

    TRACE(3, "cmsis test01 results:");
    print_matrix(pOut_cmsis, FC_S16_TEST01_DIM_COL, 1, FC_S16_TEST01_DIM_COL);
#endif

    // Compare reference to beco
    bool match = true;
    for (int i = 0; i < FC_S16_TEST01_DIM_COL; i++) {
        if (abs(pOut_beco[i] - pOut_cmsis[i]) > 1) {
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

void compare_fully_connected_s16_test02(void)
{
    // Init FC params
    cmsis_nn_context ctx = {nullptr, 0};
    cmsis_nn_fc_params fc_params;
    cmsis_nn_per_tensor_quant_params quant_params;
    cmsis_nn_dims input_dims;
    cmsis_nn_dims filter_dims;
    cmsis_nn_dims bias_dims = {0, 0, 0, 0};
    cmsis_nn_dims output_dims;

    input_dims.n = 1;
    input_dims.w = 1;
    input_dims.h = 1;
    input_dims.c = FC_S16_TEST02_DIM_VEC;
    filter_dims.n = FC_S16_TEST02_DIM_VEC;
    filter_dims.c = FC_S16_TEST02_DIM_COL;
    output_dims.n = 1;
    output_dims.c = FC_S16_TEST02_DIM_COL;

    fc_params.input_offset = FC_S16_TEST02_INPUT_OFFSET;
    fc_params.filter_offset = 0;
    fc_params.output_offset = FC_S16_TEST02_OUTPUT_OFFSET;
    fc_params.activation.min = NN_Q15_MIN;
    fc_params.activation.max = NN_Q15_MAX;

    quant_params.multiplier = FC_S16_TEST02_MULTIPLIER;
    quant_params.shift = FC_S16_TEST02_SHIFT;

    /*------------------------------- beco -------------------------------*/
    int32_t buf_size = beco_fully_connected_s16_get_buffer_size(&output_dims);
    ctx.buf = malloc(buf_size);
    int16_t pOut_beco[FC_S16_TEST02_DIM_COL];

    BECO_GET_TIME_ENTER(1);
    beco_fully_connected_s16(
        &ctx, &fc_params, &quant_params, &input_dims, fc_s16_test02_input, &filter_dims, \
        fc_s16_test02_beco_weight, &bias_dims, fc_s16_test02_beco_bias, &output_dims, pOut_beco);
    BECO_GET_TIME_EXIT(1, "test02 beco_fully_connected_s16");

    free(ctx.buf);

    /*------------------------------- cmsis -------------------------------*/
    int16_t pOut_cmsis[FC_S16_TEST02_DIM_COL];
    buf_size = arm_fully_connected_s16_get_buffer_size(&filter_dims);
    ctx.buf = malloc(buf_size);

    BECO_GET_TIME_ENTER(2);
    arm_fully_connected_s16(
        &ctx, &fc_params, &quant_params, &input_dims, fc_s16_test02_input, &filter_dims, \
        fc_s16_test02_cmsis_weight, &bias_dims, fc_s16_test02_cmsis_bias, &output_dims, pOut_cmsis);
    BECO_GET_TIME_EXIT(2, "test02 arm_fully_connected_s16");
    free(ctx.buf);

#ifdef PRINT_RESULTS
    TRACE(3, "beco test02 results:");
    print_matrix(pOut_beco, FC_S16_TEST02_DIM_COL, 1, FC_S16_TEST02_DIM_COL);

    TRACE(3, "cmsis test02 results:");
    print_matrix(pOut_cmsis, FC_S16_TEST02_DIM_COL, 1, FC_S16_TEST02_DIM_COL);
#endif

    // Compare reference to beco
    bool match = true;
    for (int i = 0; i < FC_S16_TEST02_DIM_COL; i++) {
        if (abs(pOut_beco[i] - pOut_cmsis[i]) > 1) {
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

void compare_fully_connected_s16_test03(void)
{
    // Init FC params
    cmsis_nn_context ctx = {nullptr, 0};
    cmsis_nn_fc_params fc_params;
    cmsis_nn_per_tensor_quant_params quant_params;
    cmsis_nn_dims input_dims;
    cmsis_nn_dims filter_dims;
    cmsis_nn_dims bias_dims = {0, 0, 0, 0};
    cmsis_nn_dims output_dims;

    input_dims.n = 1;
    input_dims.w = 1;
    input_dims.h = 1;
    input_dims.c = FC_S16_TEST03_DIM_VEC;
    filter_dims.n = FC_S16_TEST03_DIM_VEC;
    filter_dims.c = FC_S16_TEST03_DIM_COL;
    output_dims.n = 1;
    output_dims.c = FC_S16_TEST03_DIM_COL;

    fc_params.input_offset = FC_S16_TEST03_INPUT_OFFSET;
    fc_params.filter_offset = 0;
    fc_params.output_offset = FC_S16_TEST03_OUTPUT_OFFSET;
    fc_params.activation.min = NN_Q15_MIN;
    fc_params.activation.max = NN_Q15_MAX;

    quant_params.multiplier = FC_S16_TEST03_MULTIPLIER;
    quant_params.shift = FC_S16_TEST03_SHIFT;

    /*------------------------------- beco -------------------------------*/
    int32_t buf_size = beco_fully_connected_s16_get_buffer_size(&output_dims);
    ctx.buf = malloc(buf_size);
    int16_t pOut_beco[FC_S16_TEST03_DIM_COL];

    BECO_GET_TIME_ENTER(1);
    beco_fully_connected_s16(
        &ctx, &fc_params, &quant_params, &input_dims, fc_s16_test03_input, &filter_dims, \
        fc_s16_test03_beco_weight, &bias_dims, fc_s16_test03_beco_bias, &output_dims, pOut_beco);
    BECO_GET_TIME_EXIT(1, "test03 beco_fully_connected_s16");

    free(ctx.buf);

    /*------------------------------- cmsis -------------------------------*/
    int16_t pOut_cmsis[FC_S16_TEST03_DIM_COL];
    buf_size = arm_fully_connected_s16_get_buffer_size(&filter_dims);
    ctx.buf = malloc(buf_size);

    BECO_GET_TIME_ENTER(2);
    arm_fully_connected_s16(
        &ctx, &fc_params, &quant_params, &input_dims, fc_s16_test03_input, &filter_dims, \
        fc_s16_test03_cmsis_weight, &bias_dims, fc_s16_test03_cmsis_bias, &output_dims, pOut_cmsis);
    BECO_GET_TIME_EXIT(2, "test03 arm_fully_connected_s16");
    free(ctx.buf);

#ifdef PRINT_RESULTS
    TRACE(3, "beco test03 results:");
    print_matrix(pOut_beco, FC_S16_TEST03_DIM_COL, 1, FC_S16_TEST03_DIM_COL);

    TRACE(3, "cmsis test03 results:");
    print_matrix(pOut_cmsis, FC_S16_TEST03_DIM_COL, 1, FC_S16_TEST03_DIM_COL);
#endif

    // Compare reference to beco
    bool match = true;
    for (int i = 0; i < FC_S16_TEST03_DIM_COL; i++) {
        if (abs(pOut_beco[i] - pOut_cmsis[i]) > 1) {
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


void test_beco_fully_connected_s16(void)
{
    TRACE(3, "===== test_beco_fully_connected_s16 START =====");
    compare_fully_connected_s16();
    compare_fully_connected_s16_test01();
    compare_fully_connected_s16_test02();
    compare_fully_connected_s16_test03();
    TRACE(3, "===== test_beco_fully_connected_s16 END =====");
}
