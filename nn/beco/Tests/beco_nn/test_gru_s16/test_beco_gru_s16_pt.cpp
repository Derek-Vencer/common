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

#include "gru_s16_pt_test01.h"
#include "gru_s16_pt_test02.h"

// #define PRINT_RESULTS


void beco_gru_s16_pt_test01(void)
{
    int16_t output_data[GRU_S16_PT_TEST01_NUM_TIMES * GRU_S16_PT_TEST01_NUM_BATCHES * GRU_S16_PT_TEST01_NUM_OUTPUTS];
    int16_t hidden_state[GRU_S16_PT_TEST01_NUM_BATCHES * GRU_S16_PT_TEST01_NUM_LAYERS * GRU_S16_PT_TEST01_NUM_OUTPUTS];
    memset(hidden_state, 0, GRU_S16_PT_TEST01_NUM_BATCHES * GRU_S16_PT_TEST01_NUM_LAYERS * GRU_S16_PT_TEST01_NUM_OUTPUTS * sizeof(int16_t));
    int32_t buf_size = beco_gru_s16_get_buffer_size(GRU_S16_PT_TEST01_NUM_OUTPUTS);
    int16_t *scratch_buffers = (int16_t *)malloc(buf_size);

    BECO_GET_TIME_ENTER(1);
    beco_gru_s16_pt(gru_s16_pt_test01_input,
                    gru_s16_pt_test01_input_weights_0,
                    gru_s16_pt_test01_input_weights_n,
                    gru_s16_pt_test01_recurrent_weight,
                    GRU_S16_PT_TEST01_NUM_TIMES,
                    GRU_S16_PT_TEST01_NUM_BATCHES,
                    GRU_S16_PT_TEST01_NUM_INPUTS,
                    GRU_S16_PT_TEST01_NUM_OUTPUTS,
                    GRU_S16_PT_TEST01_NUM_LAYERS,
                    gru_s16_pt_test01_input_bias,
                    gru_s16_pt_test01_recurrent_bias,
                    gru_s16_pt_test01_input_multiplier,
                    gru_s16_pt_test01_input_shift,
                    gru_s16_pt_test01_recurrent_multiplier,
                    gru_s16_pt_test01_recurrent_shift,
                    scratch_buffers, hidden_state, output_data);
    BECO_GET_TIME_EXIT(1, "test01 beco_gru_s16_pt");
    free(scratch_buffers);

#ifdef PRINT_RESULTS
    TRACE(3, "beco test01 results:");
    print_matrix(hidden_state,
                 GRU_S16_PT_TEST01_NUM_LAYERS * GRU_S16_PT_TEST01_NUM_OUTPUTS,
                 GRU_S16_PT_TEST01_NUM_BATCHES,
                 GRU_S16_PT_TEST01_NUM_LAYERS * GRU_S16_PT_TEST01_NUM_OUTPUTS);
#endif
}

void beco_gru_s16_pt_test02(void)
{
    int16_t output_data[GRU_S16_PT_TEST02_NUM_TIMES * GRU_S16_PT_TEST02_NUM_BATCHES * GRU_S16_PT_TEST02_NUM_OUTPUTS];
    int16_t hidden_state[GRU_S16_PT_TEST02_NUM_BATCHES * GRU_S16_PT_TEST02_NUM_LAYERS * GRU_S16_PT_TEST02_NUM_OUTPUTS];
    memset(hidden_state, 0, GRU_S16_PT_TEST02_NUM_BATCHES * GRU_S16_PT_TEST02_NUM_LAYERS * GRU_S16_PT_TEST02_NUM_OUTPUTS * sizeof(int16_t));
    int32_t buf_size = beco_gru_s16_get_buffer_size(GRU_S16_PT_TEST02_NUM_OUTPUTS);
    int16_t *scratch_buffers = (int16_t *)malloc(buf_size);

    BECO_GET_TIME_ENTER(1);
    beco_gru_s16_pt(gru_s16_pt_test02_input,
                    gru_s16_pt_test02_input_weights_0,
                    gru_s16_pt_test02_input_weights_n,
                    gru_s16_pt_test02_recurrent_weight,
                    GRU_S16_PT_TEST02_NUM_TIMES,
                    GRU_S16_PT_TEST02_NUM_BATCHES,
                    GRU_S16_PT_TEST02_NUM_INPUTS,
                    GRU_S16_PT_TEST02_NUM_OUTPUTS,
                    GRU_S16_PT_TEST02_NUM_LAYERS,
                    gru_s16_pt_test02_input_bias,
                    gru_s16_pt_test02_recurrent_bias,
                    gru_s16_pt_test02_input_multiplier,
                    gru_s16_pt_test02_input_shift,
                    gru_s16_pt_test02_recurrent_multiplier,
                    gru_s16_pt_test02_recurrent_shift,
                    scratch_buffers, hidden_state, output_data);
    BECO_GET_TIME_EXIT(1, "test02 beco_gru_s16_pt");
    free(scratch_buffers);

#ifdef PRINT_RESULTS
    TRACE(3, "beco test02 results:");
    print_matrix(hidden_state,
                 GRU_S16_PT_TEST02_NUM_LAYERS * GRU_S16_PT_TEST02_NUM_OUTPUTS,
                 GRU_S16_PT_TEST02_NUM_BATCHES,
                 GRU_S16_PT_TEST02_NUM_LAYERS * GRU_S16_PT_TEST02_NUM_OUTPUTS);
#endif
}


void test_beco_gru_s16_pt(void)
{
    TRACE(3, "===== test_beco_gru_s16_pt START =====");
    beco_gru_s16_pt_test01();
    beco_gru_s16_pt_test02();
    TRACE(3, "===== test_beco_gru_s16_pt END =====");
}
