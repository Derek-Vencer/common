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

#include "hwtest.h"
#include "beco_test.h"
#include "hal_sysfreq.h"


static void get_coprocess_info(void)
{
    uint32_t cpu_id, beco_id, status;

    cpu_id = get_cpu_id();
    beco_id = beco_cpid();
    status = beco_status();

    TRACE(0, "cpu_id:%d, beco_id:%08x, beco_status:%08x", cpu_id, beco_id, status);
}

static void get_beco_nn_version(void)
{
    TRACE(0, "Beco nn version: %d.%d.%d", BECO_NN_VERSION_MAJOR, BECO_NN_VERSION_MINOR, BECO_NN_VERSION_BUILD);
}

void beco_test(void)
{
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_52M);

    BECO_INIT();

    get_coprocess_info();
    get_beco_nn_version();

#ifdef BECO_DSP_TEST
    test_beco_mat_mult();
    test_beco_fir();
#endif

#ifdef BECO_NN_TEST
    test_beco_fully_connected_s8();
    test_beco_fully_connected_s16();
    test_beco_convolve_1x1_s8();
    test_beco_convolve_1x1_s16();
    test_beco_convolve_s8();
    test_beco_convolve_s16();
    test_beco_transpose_convolve_s8();
    test_beco_transpose_convolve_s16();
    test_beco_depthwise_conv_nx3_s8();
    test_beco_depthwise_conv_nx3_s16();
    test_beco_transpose_depthwise_conv_s8();
    test_beco_transpose_depthwise_conv_s16();
    test_beco_gru_s16_pt();
    test_beco_gru_s16_pc();
    test_beco_bi_gru_s16();
    test_beco_lstm_s16_pt();
    test_beco_lstm_s16_pc();
    test_beco_bi_lstm_s16();
    test_beco_avgpool_s8();
    test_beco_avgpool_s16();
    test_beco_max_pool_s8();
    test_beco_max_pool_s16();
#endif

    BECO_EXIT(0);
}
