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
#ifndef __NN_SPEECH_VAD_H__
#define __NN_SPEECH_VAD_H__

#include <stdint.h>
#include "custom_allocator.h"

#ifdef __cplusplus
extern "C" {
#endif

struct SpeechNNVadState_;
typedef struct SpeechNNVadState_ SpeechNNVadState;

SpeechNNVadState *speech_vad_create(
    uint32_t sample_rate,
    uint32_t frame_len,
    float thres,
    custom_allocator* allocator
);
int32_t speech_vad_process(SpeechNNVadState *st, int16_t *pcm_in, uint32_t pcm_len);
int32_t speech_vad_destroy(SpeechNNVadState *st);
void speech_vad_reset(SpeechNNVadState *st);
float speech_vad_process_output_prod(SpeechNNVadState *st, int16_t *pcm_in, uint32_t pcm_len);

#ifdef __cplusplus
}
#endif

#endif
