/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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
#include "string.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "audioflinger.h"
#include "speech_bone_sensor.h"
#if defined(VPU_STREAM)
#include "vpu_stream/vpu_stream.h"
#if defined(VPU_CFG_ON_SENSOR_HUB)
extern void app_mcu_sensor_hub_ctrl_vpu(bool enable);
#else
#include "vpu_cfg/vpu_cfg.h"
#endif
#else
#if defined(SPEECH_BONE_SENSOR_I2S)
#include "lis25ba/lis25ba_cfg.h"
#include "lis25ba/lis25ba_stream.h"
#else
#include "lis25ba_v2/sensor_drv.h"
#include "audio_dump.h"
#endif
#endif

#if defined(VPU_DEBUG_CODEC_I2S_SYNC)
static uint32_t codec_capture_cnt = 0;
static uint32_t codec_playback_cnt = 0;
static uint32_t i2s_capture_cnt = 0;

static uint32_t codec_capture_tick = 0;
static uint32_t codec_playback_tick = 0;
static uint32_t i2s_capture_tick = 0;

void debug_codec_i2s_stream_sync(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream)
{
    if (id == AUD_STREAM_ID_2) {
        if (stream == AUD_STREAM_CAPTURE) {
            i2s_capture_tick =  hal_fast_sys_timer_get();
            i2s_capture_cnt++;
        }
        // Print info
        BONE_SENSOR_TRACE(0, "[SYNC][SPK] cnt = %d, tick = %d", codec_playback_cnt, codec_playback_tick);
        BONE_SENSOR_TRACE(0, "[SYNC][MIC] cnt = %d, tick = %d", codec_capture_cnt, codec_capture_tick);
        BONE_SENSOR_TRACE(0, "[SYNC][I2S] cnt = %d, tick = %d", i2s_capture_cnt, i2s_capture_tick);
        BONE_SENSOR_TRACE(0, "[SYNC][DIF] diff = %d", i2s_capture_tick - codec_capture_tick);
    } else if (id == AUD_STREAM_ID_0) {
        if (stream == AUD_STREAM_CAPTURE) {
            codec_capture_tick =  hal_fast_sys_timer_get();
            codec_capture_cnt++;
        } else {
            codec_playback_tick =  hal_fast_sys_timer_get();
            codec_playback_cnt++;
        }
    }
}
#endif

#if defined(SPEECH_BONE_SENSOR_TDM) && !defined(VPU_STREAM)
#include "audio_dump.h"

#define CHECK_RES(r, str)    if (r != 0) {BONE_SENSOR_TRACE(0, "[%s] WARNING: %s %d", __func__, str, r); return -1;}
// #define CHECK_RES(r, str)    ASSERT(r==0, "[%s] %s %d", __func__, str, r)

// #define LIS25BA_STREAM_AUDIO_DUMP

#define LIS25BA_AXIS_MAP     (SENSOR_AXIS_MAP_X)
// #define LIS25BA_AXIS_MAP     (SENSOR_AXIS_MAP_X | SENSOR_AXIS_MAP_Y | SENSOR_AXIS_MAP_Z)

#define FRAME_LEN           (SENS_AUD_CAP_BUF_SIZE / SENS_AUD_CAP_CHAN_NUM / 2 / sizeof(int16_t))

static aud_sensor_t *g_bs_tdm_st = NULL;
static short g_capture_pcm_buf[SPEECH_BS_CHANNEL_QTY][FRAME_LEN];

static uint32_t sensor_cap_data_handler(const uint8_t *buf, uint32_t len)
{
    ASSERT(len / sizeof(int16_t) / SENS_AUD_CAP_CHAN_NUM == FRAME_LEN, "[%s] len(%d) is invalid", __func__, len);

    int16_t *pcm_buf = (int16_t *)buf;

    for (uint32_t i=0; i<FRAME_LEN; i++) {
#if LIS25BA_AXIS_MAP & SENSOR_AXIS_MAP_X
        g_capture_pcm_buf[SPEECH_BS_CHANNEL_X][i] = pcm_buf[SENS_AUD_CAP_CHAN_NUM * i + SPEECH_BS_CHANNEL_X];
#endif
#if LIS25BA_AXIS_MAP & SENSOR_AXIS_MAP_Y
        g_capture_pcm_buf[SPEECH_BS_CHANNEL_Y][i] = pcm_buf[SENS_AUD_CAP_CHAN_NUM * i + SPEECH_BS_CHANNEL_Y];
#endif
#if LIS25BA_AXIS_MAP & SENSOR_AXIS_MAP_Z
        g_capture_pcm_buf[SPEECH_BS_CHANNEL_Z][i] = pcm_buf[SENS_AUD_CAP_CHAN_NUM * i + SPEECH_BS_CHANNEL_Z];
#endif
    }

#if defined(LIS25BA_STREAM_AUDIO_DUMP)
    audio_dump_clear_up();
    audio_dump_add_channel_data(0, g_capture_pcm_buf[SPEECH_BS_CHANNEL_X], FRAME_LEN);
    audio_dump_add_channel_data(1, g_capture_pcm_buf[SPEECH_BS_CHANNEL_Y], FRAME_LEN);
    audio_dump_add_channel_data(2, g_capture_pcm_buf[SPEECH_BS_CHANNEL_Z], FRAME_LEN);
    audio_dump_run();
#endif

    return 0;
}

int32_t lis25ba_stream_open()
{
    int32_t r = 0;
    uint8_t val = 0;
    aud_sensor_t *s = NULL;
    sensor_stream_cfg_t cfg;

    g_bs_tdm_st = NULL;

    BONE_SENSOR_TRACE(0, "%s: init", __func__);
    s = audio_sensor_init(AUD_SENSOR_ID_0);
    ASSERT(s != NULL, "%s: null pointer", __func__);

    BONE_SENSOR_TRACE(0, "sens: name=%s, id=%x", s->name, s->id);

    r = s->probe(AUD_SENSOR_ID_0);
    CHECK_RES(r, "Proble failed");

    r = s->init(true, AUD_SENSOR_ID_0);
    CHECK_RES(r, "Init failed");

    r = s->read(0xF, &val);
    BONE_SENSOR_TRACE(0, "WHOAMI: reg[0F] = %x", val);

    memset((void *)&cfg, 0, sizeof(cfg));
    cfg.mode = SENSOR_MODE_NORMAL;
    cfg.samprate = SENSOR_SAMPRATE_16K;
    cfg.axis_map = LIS25BA_AXIS_MAP;
    cfg.data_handler    = sensor_cap_data_handler;

    r = s->stream_open(&cfg);
    CHECK_RES(r, "Open failed");

    g_bs_tdm_st = s;

#if defined(LIS25BA_STREAM_AUDIO_DUMP)
    audio_dump_init(FRAME_LEN, sizeof(short), SPEECH_BS_CHANNEL_QTY);
#endif

    return r;
}

int32_t lis25ba_stream_close()
{
    int32_t r = 0;
    aud_sensor_t *s = g_bs_tdm_st;

    if (s == NULL) {
        BONE_SENSOR_TRACE(0, "[%s] WARNING: s is NULL", __func__);
        return -2;
    }

    r = s->stream_close();
    CHECK_RES(r, "Close failed");

    r = s->init(false, AUD_SENSOR_ID_0);

    // TODO: Close IIC

    g_bs_tdm_st = NULL;

    return r;
}

int32_t lis25ba_stream_start()
{
    int32_t r = 0;
    aud_sensor_t *s = g_bs_tdm_st;

    if (s == NULL) {
        BONE_SENSOR_TRACE(0, "[%s] WARNING: s is NULL", __func__);
        return -2;
    }

    r = s->stream_start();
    CHECK_RES(r, "Start failed");

    return r;
}

int32_t lis25ba_stream_stop()
{
    int32_t r = 0;
    aud_sensor_t *s = g_bs_tdm_st;

    if (s == NULL) {
        BONE_SENSOR_TRACE(0, "[%s] WARNING: s is NULL", __func__);
        return -2;
    }

    r = s->stream_stop();
    CHECK_RES(r, "Stop failed");

    return 0;
}

int32_t lis25ba_stream_get_data(void *_pcm_buf, uint32_t pcm_len, speech_bs_channel_t ch, uint32_t bits)
{
    ASSERT(pcm_len == FRAME_LEN, "[%s] pcm_len(%d) != FRAME_LEN(%d)", __func__, pcm_len, FRAME_LEN);
    ASSERT(ch < SPEECH_BS_CHANNEL_QTY, "[%s] ch(%d) is invalid!", __func__, ch);

    if (bits == 24) {
        int32_t *pcm_buf = (int32_t *)_pcm_buf;
        for (uint32_t i=0; i<pcm_len; i++) {
            pcm_buf[i] = ((uint32_t)g_capture_pcm_buf[ch][i]) << 8;
        }
    } else if (bits == 16) {
        int16_t *pcm_buf = (int16_t *)_pcm_buf;
        for (uint32_t i=0; i<pcm_len; i++) {
            pcm_buf[i] = g_capture_pcm_buf[ch][i];
        }
    } else {
        ASSERT(0, "[%s] bits(%d) is not invalid!", __func__, bits);
    }

    return 0;
}
#endif  // #if defined(SPEECH_BONE_SENSOR_I2S)

int32_t speech_bone_sensor_open(uint32_t sample_rate, uint32_t frame_len)
{
#if defined(VPU_DEBUG_CODEC_I2S_SYNC)
    codec_capture_cnt = 0;
    codec_playback_cnt = 0;
    i2s_capture_cnt = 0;

    codec_capture_tick = 0;
    codec_playback_tick = 0;
    i2s_capture_tick = 0;

    af_set_irq_notification(debug_codec_i2s_stream_sync);
#endif

#if defined(VPU_STREAM)
#if defined(VPU_CFG_ON_SENSOR_HUB)
    app_mcu_sensor_hub_ctrl_vpu(true);
#else
    vpu_cfg_init();
#endif
    vpu_stream_open(sample_rate, frame_len);
#else
#if defined(SPEECH_BONE_SENSOR_I2S)
    lis25ba_cfg_init();
#endif
    lis25ba_stream_open();
#endif

    return 0;
}

int32_t speech_bone_sensor_close(void)
{
#if defined(VPU_STREAM)
    vpu_stream_close();
#if defined(VPU_CFG_ON_SENSOR_HUB)
    app_mcu_sensor_hub_ctrl_vpu(false);
#else
    vpu_cfg_deinit();
#endif
#else
    lis25ba_stream_close();
#if defined(SPEECH_BONE_SENSOR_I2S)
    lis25ba_cfg_deinit();
#endif
#endif

#if defined(VPU_DEBUG_CODEC_I2S_SYNC)
    af_set_irq_notification(NULL);
#endif

    return 0;
}

int32_t speech_bone_sensor_start(void)
{
#if defined(VPU_STREAM)
    vpu_stream_start();
#else
    lis25ba_stream_start();
#endif

    return 0;
}

int32_t speech_bone_sensor_stop(void)
{
#if defined(VPU_STREAM)
    vpu_stream_stop();
#else
    lis25ba_stream_stop();
#endif

    return 0;
}

int32_t speech_bone_sensor_get_data(void *_pcm_buf, uint32_t pcm_len, speech_bs_channel_t ch, uint32_t bits)
{
#if defined(VPU_STREAM)
    vpu_stream_get_data(_pcm_buf, pcm_len, ch, bits);
#else
    lis25ba_stream_get_data(_pcm_buf, pcm_len, ch, bits);
#endif

    return 0;
}