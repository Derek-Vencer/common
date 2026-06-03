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

#include "beco_dsp/beco_dspfunctions.h"
#include "beco_bias.hpp"
#include "string.h"

beco_state beco_fir_init_q15(beco_fir_instance_q15* S,
                             uint16_t numTaps,
                             const int16_t* pCoeffs,
                             int16_t* pState,
                             uint32_t blockSize)
{
    beco_state status;

    if (blockSize % 4 || numTaps % 4) {
        status = BECO_ARGUMENT_ERROR;
    } else {
        S->numTaps = numTaps;
        S->pCoeffs = pCoeffs;

        memset(pState, 0, (numTaps + blockSize) * sizeof(int16_t));
        S->pState = pState;
        status = BECO_OK;
    }

    return status;
}

#if defined(CHIP_BEST1603) || defined(CHIP_BEST1306)
#include "arm_math_types.h"
static inline int16_t beco_fir_q15_ssat(int32_t val)
{
    val = val > 524288 ? val - (((val + 524288) >> 20) << 20) : val;
    return (int16_t)__SSAT(val, 16);
}
#endif

static void beco_fir_q15_readout_acc0(beco_vec32_out_t *beco_out)
{
#if defined(CHIP_BEST1603) || defined(CHIP_BEST1306)
    beco_out[0].i16[0] = beco_fir_q15_ssat(beco_read_acc(BECO_ACC0, 0).i32);
    beco_out[0].i16[1] = beco_fir_q15_ssat(beco_read_acc(BECO_ACC0, 4).i32);
    beco_out[1].i16[0] = beco_fir_q15_ssat(beco_read_acc(BECO_ACC0, 8).i32);
    beco_out[1].i16[1] = beco_fir_q15_ssat(beco_read_acc(BECO_ACC0, 12).i32);
#else
    *beco_out++ = beco_read_acc(BECO_ACC0, 0);
    *beco_out++ = beco_read_acc(BECO_ACC0, 8);
#endif
}

static void beco_fir_q15_readout(beco_vec32_out_t *beco_out)
{
#if defined(CHIP_BEST1603) || defined(CHIP_BEST1306)
    beco_out[0].i16[0] = beco_fir_q15_ssat(beco_read_acc(BECO_ACC0, 0).i32);
    beco_out[0].i16[1] = beco_fir_q15_ssat(beco_read_acc(BECO_ACC0, 4).i32);
    beco_out[1].i16[0] = beco_fir_q15_ssat(beco_read_acc(BECO_ACC0, 8).i32);
    beco_out[1].i16[1] = beco_fir_q15_ssat(beco_read_acc(BECO_ACC0, 12).i32);
    beco_out[2].i16[0] = beco_fir_q15_ssat(beco_read_acc(BECO_ACC1, 0).i32);
    beco_out[2].i16[1] = beco_fir_q15_ssat(beco_read_acc(BECO_ACC1, 4).i32);
    beco_out[3].i16[0] = beco_fir_q15_ssat(beco_read_acc(BECO_ACC1, 8).i32);
    beco_out[3].i16[1] = beco_fir_q15_ssat(beco_read_acc(BECO_ACC1, 12).i32);
    beco_out[4].i16[0] = beco_fir_q15_ssat(beco_read_acc(BECO_ACC2, 0).i32);
    beco_out[4].i16[1] = beco_fir_q15_ssat(beco_read_acc(BECO_ACC2, 4).i32);
    beco_out[5].i16[0] = beco_fir_q15_ssat(beco_read_acc(BECO_ACC2, 8).i32);
    beco_out[5].i16[1] = beco_fir_q15_ssat(beco_read_acc(BECO_ACC2, 12).i32);
    beco_out[6].i16[0] = beco_fir_q15_ssat(beco_read_acc(BECO_ACC3, 0).i32);
    beco_out[6].i16[1] = beco_fir_q15_ssat(beco_read_acc(BECO_ACC3, 4).i32);
    beco_out[7].i16[0] = beco_fir_q15_ssat(beco_read_acc(BECO_ACC3, 8).i32);
    beco_out[7].i16[1] = beco_fir_q15_ssat(beco_read_acc(BECO_ACC3, 12).i32);
#else
    *beco_out++ = beco_read_acc(BECO_ACC0, 0);
    *beco_out++ = beco_read_acc(BECO_ACC0, 8);
    *beco_out++ = beco_read_acc(BECO_ACC1, 0);
    *beco_out++ = beco_read_acc(BECO_ACC1, 8);
    *beco_out++ = beco_read_acc(BECO_ACC2, 0);
    *beco_out++ = beco_read_acc(BECO_ACC2, 8);
    *beco_out++ = beco_read_acc(BECO_ACC3, 0);
    *beco_out++ = beco_read_acc(BECO_ACC3, 8);
#endif
}

static void beco_fir_q15_impl_acc0(beco_vec64_in_t *beco_in,
                                   const int16_t* pCoeffs,
                                   const uint16_t numTaps)
{
    beco_write_reg(BECO_REG0, *beco_in++);
    beco_write_reg(BECO_REG1, *beco_in++);

    uint16_t taps_cnt = numTaps >> 2;
    while (taps_cnt) {
        beco_shift_block5(BECO_REG0, 2);
        beco_mmacgr(BECO_ACC0, ((beco_vec32_in_t){.i16 = {*pCoeffs++, 0}}), BECO_REG0);
        beco_shift_block5(BECO_REG0, 2);
        beco_mmacgr(BECO_ACC0, ((beco_vec32_in_t){.i16 = {*pCoeffs++, 0}}), BECO_REG0);
        beco_shift_block5(BECO_REG0, 2);
        beco_mmacgr(BECO_ACC0, ((beco_vec32_in_t){.i16 = {*pCoeffs++, 0}}), BECO_REG0);
        beco_shift_block5(BECO_REG0, 2);
        beco_mmacgr(BECO_ACC0, ((beco_vec32_in_t){.i16 = {*pCoeffs++, 0}}), BECO_REG0);
        beco_write_reg(BECO_REG1, *beco_in++);
        taps_cnt--;
    }
}

static void beco_fir_q15_impl(beco_vec64_in_t *beco_in,
                              const int16_t* pCoeffs,
                              const uint16_t numTaps)
{
    beco_write_reg(BECO_REG0, *beco_in++);
    beco_write_reg(BECO_REG1, *beco_in++);
    beco_write_reg(BECO_REG2, *beco_in++);
    beco_write_reg(BECO_REG3, *beco_in++);
    beco_write_reg(BECO_REG4, *beco_in++);

    uint16_t taps_cnt = numTaps >> 2;
    while (taps_cnt) {
        beco_shift_block5(BECO_REG0, 2);
        beco_mmacgr4(((beco_vec32_in_t){.i16 = {*pCoeffs++, 0}}), BECO_REG0);
        beco_shift_block5(BECO_REG0, 2);
        beco_mmacgr4(((beco_vec32_in_t){.i16 = {*pCoeffs++, 0}}), BECO_REG0);
        beco_shift_block5(BECO_REG0, 2);
        beco_mmacgr4(((beco_vec32_in_t){.i16 = {*pCoeffs++, 0}}), BECO_REG0);
        beco_shift_block5(BECO_REG0, 2);
        beco_mmacgr4(((beco_vec32_in_t){.i16 = {*pCoeffs++, 0}}), BECO_REG0);
        beco_write_reg(BECO_REG4, *beco_in++);
        taps_cnt--;
    }
}

void beco_fir_q15_process(const int16_t* pSrc,
                          int16_t *pState,
                          int16_t *pDst,
                          const int16_t* pCoeffs,
                          const uint16_t numTaps,
                          const uint16_t blockSize)
{
    memcpy(&pState[numTaps], pSrc, blockSize*sizeof(int16_t));

    const int beco_4acc_nums = blockSize >> 4;
    const int beco_acc_cols = (blockSize & 15) >> 2;
    beco_vec64_in_t *beco_in = (beco_vec64_in_t *)pState;
    beco_vec32_out_t *beco_out = (beco_vec32_out_t *)pDst;

    // cal by beco acc0-acc3
    for (int i = 0; i < beco_4acc_nums; i++) {
        beco_clear_acc(BECO_ACC0);
        beco_clear_acc(BECO_ACC1);
        beco_clear_acc(BECO_ACC2);
        beco_clear_acc(BECO_ACC3);

        beco_fir_q15_impl(beco_in, pCoeffs, numTaps);
        beco_fir_q15_readout(beco_out);

        beco_in += 4;
        beco_out += 8;
    }

    // cal by beco acc0
    for (int i = 0; i < beco_acc_cols; i++) {
        beco_clear_acc(BECO_ACC0);

        beco_fir_q15_impl_acc0(beco_in, pCoeffs, numTaps);
        beco_fir_q15_readout_acc0(beco_out);

        beco_in++;
        beco_out += 2;
    }

    memcpy(pState, &pState[blockSize], numTaps*sizeof(int16_t));
}

beco_state beco_fir_q15(const beco_fir_instance_q15* S,
                        const int16_t* pSrc,
                        int16_t* pDst,
                        uint32_t blockSize)
{
    int16_t* pState = S->pState;
    const int16_t *pCoeffs = S->pCoeffs;
    uint16_t numTaps = S->numTaps;
    if (blockSize % 4 || numTaps % 4) {
        return BECO_ARGUMENT_ERROR;
    }

    uint32_t config =
        BECO_CONF_AMODE_REP16 | BECO_CONF_BMODE_REP64 |
        BECO_CONF_ATYPE_INT16 | BECO_CONF_BTYPE_INT16 |
        BECO_CONF_RSHIFT(15) |
#if defined(CHIP_BEST1603) || defined(CHIP_BEST1306)
        BECO_CONF_PACK_INT32 | BECO_CONF_RD_16x16;
#else
        BECO_CONF_PACK_INT16 | BECO_CONF_RD_16x16;
#endif
    beco_write_config(config);
    beco_fir_q15_process(pSrc, pState, pDst, pCoeffs, numTaps, blockSize);

    return BECO_OK;
}
