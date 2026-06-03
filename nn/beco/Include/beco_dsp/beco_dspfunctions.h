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

#ifndef _BECO_DSPFUNCTIONS_H_
#define _BECO_DSPFUNCTIONS_H_


#include "beco.h"
#include "beco_l1.h"
#include "beco_types.h"

BECO_C_DECLARATIONS_START

typedef struct
{
    uint16_t numTaps;
    int16_t *pState;
    const int16_t *pCoeffs;
} beco_fir_instance_q15;

beco_state beco_fir_init_q15(beco_fir_instance_q15* S,
                             uint16_t numTaps,
                             const int16_t* pCoeffs,
                             int16_t* pState,
                             uint32_t blockSize);

beco_state beco_fir_q15(const beco_fir_instance_q15* S,
                        const int16_t* pSrc,
                        int16_t* pDst,
                        uint32_t blockSize);

BECO_C_DECLARATIONS_END

#endif
