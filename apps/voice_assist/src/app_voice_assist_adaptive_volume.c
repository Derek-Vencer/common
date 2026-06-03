
/***************************************************************************
 *
 * Copyright 2015-2024 BES.
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
#include "app_anc_assist.h"
#include "app_voice_assist_adaptive_volume.h"
#include "adaptive_volume.h"
#include "anc_assist.h"
#include "hal_trace.h"

#ifdef AUDIO_ADAPTIVE_VOLUME

extern void audio_adaptive_volume_set_gain_db(float gain);
extern AdaptiveVolumeState *get_audio_adaptive_volume_st(void);
static AdaptiveVolumeState *audio_adaptive_volume_st = NULL;
static bool audio_adaptive_volume_is_running = 0;

extern void speech_adaptive_volume_set_gain_db(float gain);
extern AdaptiveVolumeState *get_speech_adaptive_volume_st(void);
static AdaptiveVolumeState *speech_adaptive_volume_st = NULL;

static AdaptiveVolumeState *adaptive_volume_st = NULL;

static int32_t _voice_assist_adaptive_volume_callback(void *buf, uint32_t len, void *other);

int32_t app_voice_assist_adaptive_volume_init(void)
{
    app_anc_assist_register(ANC_ASSIST_USER_ADA_VOLUME, _voice_assist_adaptive_volume_callback);
    return 0;
}

int32_t app_voice_assist_adaptive_volume_open(void)
{
    VOICE_ASSIST_TRACE(0, "[%s] adaptive volume start stream", __func__);
    audio_adaptive_volume_st = get_audio_adaptive_volume_st();
    speech_adaptive_volume_st = get_speech_adaptive_volume_st();

    if (audio_adaptive_volume_st != NULL && speech_adaptive_volume_st == NULL) {
        adaptive_volume_st = audio_adaptive_volume_st;
    } else if (audio_adaptive_volume_st == NULL && speech_adaptive_volume_st != NULL) {
        adaptive_volume_st = speech_adaptive_volume_st;
    } else {
        ASSERT(1, "%s warning, aadaptive_volume_st is not created", __func__);
    }
    app_anc_assist_open(ANC_ASSIST_USER_ADA_VOLUME);
    audio_adaptive_volume_is_running = 1;
    return 0;
}

int32_t app_voice_assist_adaptive_volume_close(void)
{
    if(audio_adaptive_volume_is_running == 1){
        VOICE_ASSIST_TRACE(0, "[%s] adaptive volume close stream", __func__);
        audio_adaptive_volume_is_running = 0;
        app_anc_assist_close(ANC_ASSIST_USER_ADA_VOLUME);
        audio_adaptive_volume_set_gain_db(0.0f);
        speech_adaptive_volume_set_gain_db(0.0f);
    }

    return 0;
}

static float adaptive_gain = 0.f;

static int32_t _voice_assist_adaptive_volume_callback(void *buf, uint32_t len, void *other)
{
    if(audio_adaptive_volume_is_running != 1){
        return 0;
    }

    // ff mic noise engery
    float *res = (float *)buf;
    float ff_mic_noise_engery_db = res[0];
    // VOICE_ASSIST_TRACE(0, "[%s] ff_mic_noise_engery_db(x10): %d", __func__, (int)(ff_mic_noise_engery_db * 10));

    // ref
    POSSIBLY_UNUSED float ref_engery_db = res[1];
    POSSIBLY_UNUSED float *ref = (float *)other;
    POSSIBLY_UNUSED uint32_t ref_frame_len = len;

    ref_engery_db = ref_engery_db - adaptive_gain;
    // VOICE_ASSIST_TRACE(0, "[%s] ref_engery_db(x10): %d", __func__, (int)(ref_engery_db * 10));

    if(adaptive_volume_st != NULL){ 
        float adaptive_volume_gain_db = adaptive_volume_gain_estimate(adaptive_volume_st, ff_mic_noise_engery_db, ref_engery_db);
        if(audio_adaptive_volume_st){
            audio_adaptive_volume_set_gain_db(adaptive_volume_gain_db);
        }else if(speech_adaptive_volume_st){
            speech_adaptive_volume_set_gain_db(adaptive_volume_gain_db);
        }
        adaptive_gain = adaptive_volume_gain_db;
    }else{
        // VOICE_ASSIST_TRACE(0, "%s warning, audio_process.adaptive_volume is not created",__func__);
    }

    return 0;
}
#endif