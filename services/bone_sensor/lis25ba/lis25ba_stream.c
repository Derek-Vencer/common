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
#include "plat_types.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "lis25ba_stream.h"
#include "audio_dump.h"
#include "audioflinger.h"
#include "string.h"

// #define LIS25BA_STREAM_AUDIO_DUMP

#define I2S_STREAM_ID       AUD_STREAM_ID_2

#define _SAMPLE_RATE        (16000)
#if defined(LOW_DELAY_SCO)
#define FRAME_LEN           (120)       // 7.5ms
#else
#define FRAME_LEN           (240)       // 15ms
#endif
#define CHANNEL_NUM         (2)
#define CAPTURE_BUF_SIZE    (FRAME_LEN * CHANNEL_NUM * sizeof(int16_t) * 2)

typedef enum {
    AXIS_CHANNEL_X = 0,
    AXIS_CHANNEL_Y,
    AXIS_CHANNEL_Z,
    AXIS_CHANNEL_QTY
} axis_channel_t;

static uint32_t i2s_capture_buf_len = CAPTURE_BUF_SIZE;
static uint8_t  i2s_capture_buf[CAPTURE_BUF_SIZE];

static short g_capture_pcm_buf[AXIS_CHANNEL_QTY][FRAME_LEN];

static uint32_t i2s_capture_callback(uint8_t *buf, uint32_t len)
{
    int sample_idx = 0;
    short POSSIBLY_UNUSED *pcm_buf = (short*)buf;
    int POSSIBLY_UNUSED pcm_len = len / 2;

    // BONE_SENSOR_TRACE(2,"[%s] i2s frame length = %d", __func__, pcm_len / 4);

#if 1
// 2 Channel: X and Y
#define X_OFFSET (2)
#define Y_OFFSET (0)

    for (int i = 0; i < len; i += 4) {
        g_capture_pcm_buf[AXIS_CHANNEL_X][sample_idx] = buf[i + X_OFFSET] + (buf[i + X_OFFSET + 1] << 8);
        g_capture_pcm_buf[AXIS_CHANNEL_Y][sample_idx] = buf[i + Y_OFFSET] + (buf[i + Y_OFFSET + 1] << 8);
        sample_idx++;
    }
#else
    // 3 Channel: X, Y and X. For this case, need to set 32bits I2S
    #define X_OFFSET    (6)
    #define Y_OFFSET    (4)
    #define Z_OFFSET    (2)

    for (int i = 0; i < len; i += 8) {
        i2s_capture_tmp_buf[AXIS_CHANNEL_X][sample_idx] = buf[i+X_OFFSET] + (buf[i+X_OFFSET+1] << 8);
        i2s_capture_tmp_buf[AXIS_CHANNEL_Y][sample_idx] = buf[i+Y_OFFSET] + (buf[i+Y_OFFSET+1] << 8);
        i2s_capture_tmp_buf[AXIS_CHANNEL_Z][sample_idx] = buf[i+Z_OFFSET] + (buf[i+Z_OFFSET+1] << 8);
        sample_idx++;
    }
#endif

#if defined(LIS25BA_STREAM_AUDIO_DUMP)
    audio_dump_clear_up();
    audio_dump_add_channel_data(0, g_capture_pcm_buf[AXIS_CHANNEL_X], FRAME_LEN);
    audio_dump_add_channel_data(1, g_capture_pcm_buf[AXIS_CHANNEL_Y], FRAME_LEN);
    audio_dump_run();
#endif

    return 0;
}

void lis25ba_stream_get_data(void *_pcm_buf, uint32_t pcm_len, uint32_t ch, uint32_t bits)
{
    ASSERT(pcm_len == FRAME_LEN, "[%s] pcm_len(%d) != FRAME_LEN(%d)", __func__, pcm_len, FRAME_LEN);
    ASSERT(ch < AXIS_CHANNEL_QTY, "[%s] ch(%d) is invalid!", __func__, ch);

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
}

int lis25ba_stream_open(void)
{
    BONE_SENSOR_TRACE(1,"[%s] Run...", __func__);
    int ret = 0;
    struct AF_STREAM_CONFIG_T stream_cfg;

#if defined(LIS25BA_STREAM_AUDIO_DUMP)
    audio_dump_init(FRAME_LEN, sizeof(short), 2);
#endif

    memset(&stream_cfg, 0, sizeof(stream_cfg));
    stream_cfg.sync_start = false;
    stream_cfg.bits = AUD_BITS_16;
    stream_cfg.channel_num = AUD_CHANNEL_NUM_2;
    stream_cfg.channel_map = (enum AUD_CHANNEL_MAP_T)(AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1);
    stream_cfg.sample_rate = AUD_SAMPRATE_16000;
    stream_cfg.chan_sep_buf = false;
    stream_cfg.vol = 12;
    stream_cfg.device = AUD_STREAM_USE_I2S0_MASTER;
    stream_cfg.io_path = AUD_IO_PATH_NULL;
    stream_cfg.data_size = i2s_capture_buf_len;
    stream_cfg.data_ptr = i2s_capture_buf;
    stream_cfg.handler = i2s_capture_callback;
    BONE_SENSOR_TRACE(2,"i2s capture sample_rate:%d, data_size:%d",stream_cfg.sample_rate,stream_cfg.data_size);
    ret = af_stream_open(I2S_STREAM_ID, AUD_STREAM_CAPTURE, &stream_cfg);
    ASSERT(ret == 0, "i2s capture failed: %d", ret);

    return 0;
}

int lis25ba_stream_close(void)
{
    BONE_SENSOR_TRACE(1,"[%s] Run...", __func__);

    af_stream_close(I2S_STREAM_ID, AUD_STREAM_CAPTURE);

    return 0;
}

int lis25ba_stream_start(void)
{
    BONE_SENSOR_TRACE(1,"[%s] Run...", __func__);

    af_stream_start(I2S_STREAM_ID, AUD_STREAM_CAPTURE);

    return 0;
}

int lis25ba_stream_stop(void)
{
    BONE_SENSOR_TRACE(1,"[%s] Run...", __func__);

    af_stream_stop(I2S_STREAM_ID, AUD_STREAM_CAPTURE);

    return 0;
}