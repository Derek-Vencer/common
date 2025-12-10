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

#include "bi_gru_s16_test01.h"

// #define PRINT_RESULTS


void beco_bi_gru_s16_test01(void)
{
    int16_t output_data[BI_GRU_S16_PT_TEST01_NUM_TIMES*BI_GRU_S16_PT_TEST01_NUM_BATCHES*2*BI_GRU_S16_PT_TEST01_NUM_OUTPUTS];
    int16_t hidden_state[BI_GRU_S16_PT_TEST01_NUM_LAYERS*BI_GRU_S16_PT_TEST01_NUM_BATCHES*BI_GRU_S16_PT_TEST01_NUM_OUTPUTS];
    int16_t hidden_state_reverse[BI_GRU_S16_PT_TEST01_NUM_LAYERS*BI_GRU_S16_PT_TEST01_NUM_BATCHES*BI_GRU_S16_PT_TEST01_NUM_OUTPUTS];
    memset(hidden_state, 0, BI_GRU_S16_PT_TEST01_NUM_LAYERS*BI_GRU_S16_PT_TEST01_NUM_BATCHES*BI_GRU_S16_PT_TEST01_NUM_OUTPUTS * sizeof(int16_t));
    memset(hidden_state_reverse, 0, BI_GRU_S16_PT_TEST01_NUM_LAYERS*BI_GRU_S16_PT_TEST01_NUM_BATCHES*BI_GRU_S16_PT_TEST01_NUM_OUTPUTS * sizeof(int16_t));
    int32_t buf_size = beco_bi_gru_s16_get_buffer_size(BI_GRU_S16_PT_TEST01_NUM_TIMES,
                                                       BI_GRU_S16_PT_TEST01_NUM_BATCHES,
                                                       BI_GRU_S16_PT_TEST01_NUM_OUTPUTS);
    int16_t *scratch_buffers = (int16_t *)malloc(buf_size);

    BECO_GET_TIME_ENTER(1);
    beco_bi_gru_s16(bi_gru_s16_pt_test01_input,
                    bi_gru_s16_pt_test01_input_weights_0,
                    bi_gru_s16_pt_test01_input_weights_n,
                    bi_gru_s16_pt_test01_recurrent_weight,
                    bi_gru_s16_pt_test01_input_weights_0_reverse,
                    bi_gru_s16_pt_test01_input_weights_n_reverse,
                    bi_gru_s16_pt_test01_recurrent_weight_reverse,
                    BI_GRU_S16_PT_TEST01_NUM_TIMES,
                    BI_GRU_S16_PT_TEST01_NUM_BATCHES,
                    BI_GRU_S16_PT_TEST01_NUM_INPUTS,
                    BI_GRU_S16_PT_TEST01_NUM_OUTPUTS,
                    BI_GRU_S16_PT_TEST01_NUM_LAYERS,
                    bi_gru_s16_pt_test01_input_bias,
                    bi_gru_s16_pt_test01_recurrent_bias,
                    bi_gru_s16_pt_test01_input_bias_reverse,
                    bi_gru_s16_pt_test01_recurrent_bias_reverse,
                    bi_gru_s16_pt_test01_input_multiplier,
                    bi_gru_s16_pt_test01_input_shift,
                    bi_gru_s16_pt_test01_recurrent_multiplier,
                    bi_gru_s16_pt_test01_recurrent_shift,
                    bi_gru_s16_pt_test01_input_multiplier_reverse,
                    bi_gru_s16_pt_test01_input_shift_reverse,
                    bi_gru_s16_pt_test01_recurrent_multiplier_reverse,
                    bi_gru_s16_pt_test01_recurrent_shift_reverse,
                    scratch_buffers,
                    hidden_state,
                    hidden_state_reverse,
                    output_data);
    BECO_GET_TIME_EXIT(1, "test01 beco_bi_gru_s16");
    free(scratch_buffers);

#ifdef PRINT_RESULTS
    TRACE(3, "beco test01 results:%s", "");
    for (int i = BI_GRU_S16_PT_TEST01_NUM_TIMES-1; i < BI_GRU_S16_PT_TEST01_NUM_TIMES; i++) {
        print_matrix(&output_data[i*BI_GRU_S16_PT_TEST01_NUM_BATCHES*2*BI_GRU_S16_PT_TEST01_NUM_OUTPUTS],
                     2*BI_GRU_S16_PT_TEST01_NUM_OUTPUTS,
                     BI_GRU_S16_PT_TEST01_NUM_BATCHES,
                     2*BI_GRU_S16_PT_TEST01_NUM_OUTPUTS);
    }
#endif
}


extern "C"
void test_beco_bi_gru_s16(void)
{
    TRACE(3, "===== test_beco_bi_gru_s16 START =====%s", "");
    beco_bi_gru_s16_test01();
    TRACE(3, "===== test_beco_bi_gru_s16 END =====%s", "");
}
