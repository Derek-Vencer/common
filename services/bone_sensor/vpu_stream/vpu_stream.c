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
#include "vpu_stream.h"
#include "audio_dump.h"
#include "hal_tdm.h"
#include "audioflinger.h"
#include "string.h"

// #define VPU_STREAM_AUDIO_DUMP

#define VPU_STREAM_ID       AUD_STREAM_ID_2

#define FRAME_LEN_MAX       (512)       // max support 30ms
#define SAMPLE_RATE         (16000)
#define CHANNEL_NUM         (4)
#define CAPTURE_BUF_SIZE    (FRAME_LEN_MAX * CHANNEL_NUM * sizeof(int16_t) * 2)

typedef enum {
    AXIS_CH_X = 0,
    AXIS_CH_Y,
    AXIS_CH_Z,
    AXIS_CH_QTY
} axis_ch_t;

static uint32_t g_frame_len = 0;
static uint8_t ALIGNED(4) vpu_capture_buf[CAPTURE_BUF_SIZE];
static int16_t ALIGNED(4) g_capture_pcm_buf[AXIS_CH_QTY][FRAME_LEN_MAX];

static uint32_t vpu_capture_callback(uint8_t *buf, uint32_t len)
{
    ASSERT(len / sizeof(int16_t) / CHANNEL_NUM == g_frame_len, "[%s] len(%d) is invalid", __func__, len);

    int16_t *pcm_buf = (int16_t *)buf;

    for (uint32_t i=0; i<g_frame_len; i++) {
        g_capture_pcm_buf[AXIS_CH_X][i] = pcm_buf[CHANNEL_NUM * i + AXIS_CH_X];
        g_capture_pcm_buf[AXIS_CH_Y][i] = pcm_buf[CHANNEL_NUM * i + AXIS_CH_Y];
        g_capture_pcm_buf[AXIS_CH_Z][i] = pcm_buf[CHANNEL_NUM * i + AXIS_CH_Z];
    }

#if defined(VPU_STREAM_AUDIO_DUMP)
    audio_dump_clear_up();
    audio_dump_add_channel_data(0, g_capture_pcm_buf[AXIS_CH_X], g_frame_len);
    audio_dump_add_channel_data(1, g_capture_pcm_buf[AXIS_CH_Y], g_frame_len);
    audio_dump_add_channel_data(2, g_capture_pcm_buf[AXIS_CH_Z], g_frame_len);
    audio_dump_run();
#endif

    return 0;
}

void vpu_stream_get_data(void *_pcm_buf, uint32_t pcm_len, uint32_t ch, uint32_t bits)
{
    ASSERT(pcm_len == g_frame_len, "[%s] pcm_len(%d) != g_frame_len(%d)", __func__, pcm_len, g_frame_len);
    ASSERT(ch < AXIS_CH_QTY, "[%s] ch(%d) is invalid!", __func__, ch);
    
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

int32_t vpu_stream_open(uint32_t sample_rate, uint32_t frame_len)
{
    //BONE_SENSOR_TRACE(0, "[%s] VPU opened", __func__);
    int ret = 0;
    struct AF_STREAM_CONFIG_T stream_cfg;

    ASSERT(sample_rate == SAMPLE_RATE, "[%s] sample_rate(%d) != SAMPLE_RATE !!!", __func__, sample_rate);
    ASSERT(frame_len <= FRAME_LEN_MAX, "[%s] frame_len(%d) != FRAME_LEN_MAX(%d)", \
        __func__, \
        frame_len, \
        FRAME_LEN_MAX);
    g_frame_len = frame_len;

#if defined(VPU_STREAM_AUDIO_DUMP)
    audio_dump_init(g_frame_len, sizeof(int16_t), AXIS_CH_QTY);
#endif

    memset(&stream_cfg, 0, sizeof(stream_cfg));
    stream_cfg.sync_start = false;
    stream_cfg.fs_cycles   = HAL_TDM_FS_CYCLES_16;
    stream_cfg.slot_cycles = HAL_TDM_SLOT_CYCLES_16;
    stream_cfg.fs_edge     = AUD_FS_FIRST_EDGE_POS;
    stream_cfg.align       = AUD_DATA_ALIGN_LEFT_JUSTIFIED;

    stream_cfg.bits = AUD_BITS_16;
    stream_cfg.channel_num = CHANNEL_NUM;
    stream_cfg.channel_map = (enum AUD_CHANNEL_MAP_T)(AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1);
    stream_cfg.sample_rate = SAMPLE_RATE;
    stream_cfg.chan_sep_buf = false;
    stream_cfg.vol = TGT_VOLUME_LEVEL_MAX;
    stream_cfg.device = AUD_STREAM_USE_TDM0_MASTER;
    stream_cfg.io_path = AUD_IO_PATH_NULL;
    stream_cfg.data_size = frame_len * CHANNEL_NUM * sizeof(int16_t) * 2;
    stream_cfg.data_ptr = vpu_capture_buf;
    stream_cfg.handler = vpu_capture_callback;
    BONE_SENSOR_TRACE(3, "[%s] sample_rate=%d, data_size=%d", __func__, stream_cfg.sample_rate, stream_cfg.data_size);
    ret = af_stream_open(VPU_STREAM_ID, AUD_STREAM_CAPTURE, &stream_cfg);
    ASSERT(ret == 0, "i2s capture failed: %d", ret);

    return 0;
}

int32_t vpu_stream_close(void)
{
    BONE_SENSOR_TRACE(1, "[%s] ...", __func__);

    af_stream_close(VPU_STREAM_ID, AUD_STREAM_CAPTURE);

    return 0;
}

int32_t vpu_stream_start(void)
{
    BONE_SENSOR_TRACE(1, "[%s] ...", __func__);

    af_stream_start(VPU_STREAM_ID, AUD_STREAM_CAPTURE);

    return 0;
}

int32_t vpu_stream_stop(void)
{
    BONE_SENSOR_TRACE(1, "[%s] ...", __func__);

    af_stream_stop(VPU_STREAM_ID, AUD_STREAM_CAPTURE);

    return 0;
}