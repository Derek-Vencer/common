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

#ifndef _NAIVE_DSPFUNCTIONS_H
#define _NAIVE_DSPFUNCTIONS_H

#include "arm_math.h"

__STATIC_FORCEINLINE void naive_mat_mult_i8xi8_i16(const int8_t *A, const int8_t *B, int16_t *C, int M, int N, int K)
{
    int i,j,p;
    int out;

    for (i = 0; i < M; i++) {
        for (j = 0; j < N; j++) {
            // Calculate the dot product between A.column and B.column:
            out = 0;
            for (p = 0; p < K; p++) {
                out += A[i+p*M] * B[p*N+j]; // A[M][K] * B[K][N]
            }
            C[i*N+j] = (int16_t)__SSAT(out, 16);
        }
    }
}

__STATIC_FORCEINLINE void naive_mat_mult_u8xu8_u16(const uint8_t *A, const uint8_t *B, uint16_t *C, int M, int N, int K)
{
    int i,j,p;
    int out;

    for (i = 0; i < M; i++) {
        for (j = 0; j < N; j++) {
            // Calculate the dot product between A.column and B.column:
            out = 0;
            for (p = 0; p < K; p++) {
                out += A[i+p*M] * B[p*N+j]; // A[M][K] * B[K][N]
            }
            C[i*N+j] = (uint16_t)__USAT(out, 16);
        }
    }
}

__STATIC_FORCEINLINE void naive_fir_q15(const int16_t *pSrc,
                                        const int16_t *pCoeffs,
                                        int16_t *pState,
                                        uint16_t numTaps,
                                        uint16_t blockSize,
                                        int16_t *pDst)
{
    int64_t acc0;
    int16_t *px;
    const int16_t *pb;
    int16_t *state = pState;
    int16_t *pStateCurnt = &pState[numTaps - 1];

    uint16_t blkCnt = blockSize;
    while (blkCnt) {
        *pStateCurnt++ = *pSrc++;
        acc0 = 0;

        px = state;
        pb = pCoeffs;

        uint16_t tapCnt = numTaps >> 1;
        while (tapCnt) {
            acc0 += (int32_t) *px++ * *pb++;
            acc0 += (int32_t) *px++ * *pb++;
            tapCnt--;
        }

        *pDst++ = (int16_t)__SSAT((acc0 >> 15), 16);
        state++;

        blkCnt--;
    }

    pStateCurnt = pState;
    uint16_t tapCnt = (numTaps - 1);
    while (tapCnt) {
        *pStateCurnt++ = *state++;
        tapCnt--;
    }
}

#endif
