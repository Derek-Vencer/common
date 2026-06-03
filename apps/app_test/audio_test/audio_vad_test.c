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
#include "hal_trace.h"
#include "cmsis_os.h"
#include "app_utils.h"
#include "audioflinger.h"
#include "app_overlay.h"
#include "hal_timer.h"
#include "string.h"
#include "audio_dump.h"
#include "speech_memory.h"
#include "nn_speech_vad.h"

// ext_heap provides alloc function
#define HEAP_BUFF_SIZE       (1024 * 150)
#include "ext_heap.h"

// #define VAD_DUMP_ENABLE
#define SAMPLE_BITS         (16)
#define SAMPLE_BYTES        2
#define FRAME_LEN       (256)
#define CHANNEL_NUM     (1)
#define TEMP_BUF_SIZE   (FRAME_LEN * CHANNEL_NUM * SAMPLE_BYTES * 2)
#define CODEC_STREAM_ID     AUD_STREAM_ID_0
#define VAD_TX_SAMPLE_RATE (16000)
#define VAD_RX_SAMPLE_RATE (16000)

static uint8_t POSSIBLY_UNUSED codec_capture_buf[TEMP_BUF_SIZE];
static uint8_t POSSIBLY_UNUSED codec_playback_buf[TEMP_BUF_SIZE];
static uint8_t POSSIBLY_UNUSED codec_queue_buf[TEMP_BUF_SIZE];

static uint32_t POSSIBLY_UNUSED codec_capture_cnt = 0;
static uint32_t POSSIBLY_UNUSED codec_playback_cnt = 0;

static int16_t POSSIBLY_UNUSED mic0_pcm_buf[FRAME_LEN];
// static int16_t POSSIBLY_UNUSED mic1_pcm_buf[FRAME_LEN];
static uint8_t POSSIBLY_UNUSED speech_buf[HEAP_BUFF_SIZE];
static SpeechNNVadState *nn_vad_st = NULL;

static uint32_t codec_capture_callback(uint8_t *buf, uint32_t len)
{
    static uint32_t vad_cnt=0;
    short POSSIBLY_UNUSED *pcm_buf = (short*)buf;
    memcpy(codec_queue_buf, buf, len);

    for (int i = 0; i < FRAME_LEN; i++) {
        mic0_pcm_buf[i] = pcm_buf[CHANNEL_NUM * i + 0];
        // mic1_pcm_buf[i] = pcm_buf[CHANNEL_NUM * i + 1];
    }

#ifdef VAD_DUMP_ENABLE
    audio_dump_clear_up();
    audio_dump_add_channel_data(0, mic0_pcm_buf, FRAME_LEN);
    // audio_dump_add_channel_data(1, mic1_pcm_buf, FRAME_LEN);
    audio_dump_run();
#endif

    int32_t vad_ret = speech_vad_process(nn_vad_st, mic0_pcm_buf, FRAME_LEN);
    if (vad_ret) {
        vad_cnt++;
        APP_TEST_TRACE(2, "[VAD] ret: %d, cnt: %d", vad_ret, vad_cnt);
    }
    return vad_ret;
}

static uint32_t codec_playback_callback(uint8_t *buf, uint32_t len)
{
    short POSSIBLY_UNUSED *pcm_buf = (short*)buf;
    int pcm_len = len / 2;

    memcpy(buf, codec_queue_buf, len);

    return pcm_len * 2;
}

static int nn_vad_start(bool on)
{
    int ret = 0;
    float nn_vad_thres = 0.5;
    static bool isRun =  false;
    enum APP_SYSFREQ_FREQ_T freq = APP_SYSFREQ_104M;
    struct AF_STREAM_CONFIG_T stream_cfg;

    if (isRun == on) {
        return 0;
    }

    if (on) {
        af_set_priority(AF_USER_TEST, osPriorityHigh);
        freq = APP_SYSFREQ_104M;

        app_sysfreq_req(APP_SYSFREQ_USER_APP_0, freq);
        APP_TEST_TRACE(1,"bt_sco_player: app_sysfreq_req %d", freq);
        APP_TEST_TRACE(1,"sys freq calc : %d\n", hal_sys_timer_calc_cpu_freq(5, 0));

        app_overlay_select(APP_OVERLAY_HFP);

        // Initialize Cqueue
        codec_capture_cnt = 0;
        codec_playback_cnt = 0;

        ext_heap_init(speech_buf);

        nn_vad_st = speech_vad_create(
            VAD_TX_SAMPLE_RATE,
            FRAME_LEN,
            nn_vad_thres,
            &ext_allocator
        );

#ifdef VAD_DUMP_ENABLE
        audio_dump_init(FRAME_LEN, sizeof(short), CHANNEL_NUM);
#endif
        memset(&stream_cfg, 0, sizeof(stream_cfg));
        stream_cfg.channel_num = CHANNEL_NUM;
        stream_cfg.data_size = TEMP_BUF_SIZE;
        stream_cfg.sample_rate = (enum AUD_SAMPRATE_T)VAD_TX_SAMPLE_RATE;
        stream_cfg.bits = (enum AUD_BITS_T)SAMPLE_BITS;;
        stream_cfg.vol = 12;
        stream_cfg.device = AUD_STREAM_USE_INT_CODEC;
        stream_cfg.io_path = AUD_INPUT_PATH_MAINMIC;
        // stream_cfg.channel_map = (enum AUD_CHANNEL_MAP_T)(AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1);
        stream_cfg.handler = codec_capture_callback;
        stream_cfg.data_ptr = codec_capture_buf;

        APP_TEST_TRACE(2,"codec capture sample_rate:%d, data_size:%d",
                stream_cfg.sample_rate,stream_cfg.data_size);
        af_stream_open(CODEC_STREAM_ID, AUD_STREAM_CAPTURE, &stream_cfg);
        ASSERT(ret == 0, "codec capture failed: %d", ret);

        memset(&stream_cfg, 0, sizeof(stream_cfg));
        stream_cfg.channel_num = CHANNEL_NUM;
        stream_cfg.data_size = TEMP_BUF_SIZE;
        stream_cfg.sample_rate = (enum AUD_SAMPRATE_T)VAD_RX_SAMPLE_RATE;
        stream_cfg.bits = (enum AUD_BITS_T)SAMPLE_BITS;;
        stream_cfg.vol = 12;
        stream_cfg.device = AUD_STREAM_USE_INT_CODEC;
        stream_cfg.io_path = AUD_OUTPUT_PATH_SPEAKER;
        stream_cfg.handler = codec_playback_callback;
        stream_cfg.data_ptr = codec_playback_buf;

        APP_TEST_TRACE(2,"codec playback sample_rate:%d, data_size:%d",
                stream_cfg.sample_rate,stream_cfg.data_size);
        af_stream_open(CODEC_STREAM_ID, AUD_STREAM_PLAYBACK, &stream_cfg);

        // Start
        af_stream_start(CODEC_STREAM_ID, AUD_STREAM_PLAYBACK);
        af_stream_start(CODEC_STREAM_ID, AUD_STREAM_CAPTURE);
    } else {
        // Close stream
        af_stream_stop(CODEC_STREAM_ID, AUD_STREAM_CAPTURE);
        af_stream_stop(CODEC_STREAM_ID, AUD_STREAM_PLAYBACK);

        af_stream_close(CODEC_STREAM_ID, AUD_STREAM_CAPTURE);
        af_stream_close(CODEC_STREAM_ID, AUD_STREAM_PLAYBACK);

        // deinitialize stream process
        speech_vad_destroy(nn_vad_st);

        ext_heap_deinit();

        // app_overlay_unloadall();
        app_sysfreq_req(APP_SYSFREQ_USER_APP_0, APP_SYSFREQ_32K);
        af_set_priority(AF_USER_TEST, osPriorityAboveNormal);
    }

    isRun=on;

    return 0;
}

int32_t nn_vad_test(void)
{
    APP_TEST_TRACE(1,"[%s]...", __func__);

    nn_vad_start(true);

    return 0;
}
