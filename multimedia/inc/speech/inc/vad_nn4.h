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

#ifndef __VAD_NN4_H__
#define __VAD_NN4_H__

#include <stdint.h>
#include "custom_allocator.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VadNN4State_ VadNN4State;

VadNN4State *vad_nn4_create(custom_allocator *allocator, uint32_t sample_rate, uint32_t frame_size);
float vad_nn4_process(VadNN4State *st, int16_t *pcm_in, uint32_t pcm_len);
void vad_nn4_destroy(VadNN4State *st);
void vad_nn4_reset(VadNN4State *st);

#ifdef __cplusplus
}
#endif

#endif