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
#ifndef __NN_SPEECH_WAKEUP_H__
#define __NN_SPEECH_WAKEUP_H__

#include <stdint.h>
#include "nn_kws_model_params.h"
#include "custom_allocator.h"

#ifdef __cplusplus
extern "C" {
#endif

struct SpeechNNWakeupState_;
typedef struct SpeechNNWakeupState_ SpeechNNWakeupState;

/**
 * @brief Creates and initializes a SpeechNN wakeup engine instance.
 *
 * This function sets up the internal state for a keyword spotting (KWS)
 * system based on a SpeechNN model.
 *
 * @param model_params  Pointer to the structure holding model data such as weights,
 *                      scale factors, and quantization parameters.
 * @param sample_rate   The sample rate of the input audio in Hz.
 * @param frame_len     The length of each input audio frame, in samples.
 * @param thres         Wakeup threshold. If the model output score exceeds
 *                      this threshold, a wakeup event is triggered.
 * @param allocator     Pointer to a custom memory allocator for managing internal memory.
 *
 * @return a newly created instance if successful, or NULL on failure.
 */
SpeechNNWakeupState *speech_wakeup_create(
    SpeechNNModelParams* model_params,
    uint32_t sample_rate,
    uint32_t frame_len,
    float thres,
    custom_allocator* allocator
);

/**
 * @brief Processes a block of PCM audio data and evaluates the wakeup condition.
 *
 * This function feeds a chunk of 16-bit PCM audio samples into the wakeup engine.
 * It performs inference and returns whether the input contains a wakeup event.
 *
 * @param st        Pointer to the SpeechNNWakeupState instance.
 * @param pcm_in    Pointer to the input audio buffer (signed 16-bit PCM samples).
 * @param pcm_len   Number of audio samples in the input buffer.
 *
 * @return Detection result:
 *         - 1 if a wakeup event (e.g., keyword) is detected,
 *         - 0 if no event is detected,
 *         - negative value if an error occurred.
 */
int32_t speech_wakeup_process(SpeechNNWakeupState *st, int16_t *pcm_in, uint32_t pcm_len);

/**
 * @brief Destroys the wakeup engine instance and releases all associated resources.
 *
 * @param st  Pointer to the SpeechNNWakeupState instance to be destroyed.
 *
 * @return 0 on success, or a negative value on failure.
 */
int32_t speech_wakeup_destroy(SpeechNNWakeupState *st);

/**
 * @brief Resets the internal state of the wakeup engine.
 *
 * This function clears any internal buffers or accumulated state,
 * allowing the engine to restart inference from a clean state .
 *
 * @param st  Pointer to the SpeechNNWakeupState instance to reset.
 */
void speech_wakeup_reset(SpeechNNWakeupState *st);

#ifdef __cplusplus
}
#endif

#endif