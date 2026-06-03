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
#include "custom_allocator.h"
#include "speech_memory.h"
#include "nn_speech_wakeup.h"
#ifdef NN_KWS_EN
#include "nn_kws_model_beco_en.h"
#else
#ifdef USE_BECO_NN
#include "nn_kws_model_beco_cn.h"
#else
#include "nn_kws_model_cmsis_cn.h"
#endif
#endif

// ext_heap provides alloc function
#define HEAP_BUFF_SIZE       (1024 * 150)
#include "ext_heap.h"

#define KWS_TEST_LOG_I(str, ...)      TR_INFO(TR_MOD(TEST), "[KWS_TEST]" str, ##__VA_ARGS__)

// #define KWS_DUMP_ENABLE
#define SAMPLE_BITS         (16)
#define SAMPLE_BYTES        2
#define FRAME_LEN       (256)
#define CHANNEL_NUM     (2)
#define TEMP_BUF_SIZE   (FRAME_LEN * CHANNEL_NUM * SAMPLE_BYTES * 2)
#define CODEC_STREAM_ID     AUD_STREAM_ID_0
#define KWS_TX_SAMPLE_RATE (16000)
#define KWS_RX_SAMPLE_RATE (16000)

static uint8_t POSSIBLY_UNUSED codec_capture_buf[TEMP_BUF_SIZE];
static uint8_t POSSIBLY_UNUSED codec_playback_buf[TEMP_BUF_SIZE];
static uint8_t POSSIBLY_UNUSED codec_queue_buf[TEMP_BUF_SIZE];

static uint32_t POSSIBLY_UNUSED codec_capture_cnt = 0;
static uint32_t POSSIBLY_UNUSED codec_playback_cnt = 0;

static int16_t POSSIBLY_UNUSED mic0_pcm_buf[FRAME_LEN];
static int16_t POSSIBLY_UNUSED mic1_pcm_buf[FRAME_LEN];
static uint8_t POSSIBLY_UNUSED speech_buf[HEAP_BUFF_SIZE];
static SpeechNNWakeupState *nn_kws_st = NULL;

static uint32_t codec_capture_callback(uint8_t *buf, uint32_t len)
{
    static uint32_t kws_cnt=0;
    short POSSIBLY_UNUSED *pcm_buf = (short*)buf;
    memcpy(codec_queue_buf, buf, len);

    for (int i = 0; i < FRAME_LEN; i++) {
        mic0_pcm_buf[i] = pcm_buf[CHANNEL_NUM * i + 0];
        mic1_pcm_buf[i] = pcm_buf[CHANNEL_NUM * i + 1];
    }

#ifdef KWS_DUMP_ENABLE
    audio_dump_clear_up();
    audio_dump_add_channel_data(0, mic0_pcm_buf, FRAME_LEN);
    audio_dump_add_channel_data(1, mic1_pcm_buf, FRAME_LEN);
    audio_dump_run();
#endif

    int32_t kws_ret = speech_wakeup_process(nn_kws_st, mic0_pcm_buf, FRAME_LEN);
    if (kws_ret) {
        kws_cnt++;
        KWS_TEST_LOG_I("[KWS] ret: %d, cnt: %d", kws_ret, kws_cnt);
    }
    return kws_ret;
}

static uint32_t codec_playback_callback(uint8_t *buf, uint32_t len)
{
    short POSSIBLY_UNUSED *pcm_buf = (short*)buf;
    int pcm_len = len / 2;

    memcpy(buf, codec_queue_buf, len);

    return pcm_len * 2;
}

static int nn_kws_start(bool on)
{
    int ret = 0;
    float nn_kws_thres = 0.8;
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
        KWS_TEST_LOG_I("bt_sco_player: app_sysfreq_req %d", freq);
        KWS_TEST_LOG_I("sys freq calc : %d\n", hal_sys_timer_calc_cpu_freq(5, 0));

        app_overlay_select(APP_OVERLAY_HFP);

        // Initialize Cqueue
        codec_capture_cnt = 0;
        codec_playback_cnt = 0;

        ext_heap_init(speech_buf);

        SpeechNNModelParams model_params;
        model_params.weight_data = (unsigned char *)speech_nn_kws_weight_datas;
        model_params.scale_data = (unsigned char *)speech_nn_kws_scale_datas;
        model_params.quant_params_data = (unsigned char *)speech_nn_kws_quant_params_datas;
        model_params.weight_data_len = sizeof(speech_nn_kws_weight_datas);
        model_params.scale_data_len = sizeof(speech_nn_kws_scale_datas);
        model_params.quant_params_data_len = sizeof(speech_nn_kws_quant_params_datas);
        model_params.out_dim = NN_KWS_OUT_DIM;
        nn_kws_st = speech_wakeup_create(
            &model_params,
            KWS_TX_SAMPLE_RATE,
            FRAME_LEN,
            nn_kws_thres,
            &ext_allocator
        );

#ifdef KWS_DUMP_ENABLE
        audio_dump_init(FRAME_LEN, sizeof(short), CHANNEL_NUM);
#endif
        memset(&stream_cfg, 0, sizeof(stream_cfg));
        stream_cfg.channel_num = CHANNEL_NUM;
        stream_cfg.data_size = TEMP_BUF_SIZE;
        stream_cfg.sample_rate = (enum AUD_SAMPRATE_T)KWS_TX_SAMPLE_RATE;
        stream_cfg.bits = (enum AUD_BITS_T)SAMPLE_BITS;;
        stream_cfg.vol = 12;
        stream_cfg.device = AUD_STREAM_USE_INT_CODEC;
        stream_cfg.io_path = AUD_INPUT_PATH_MAINMIC;
        stream_cfg.channel_map = (enum AUD_CHANNEL_MAP_T)(AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1);
        stream_cfg.handler = codec_capture_callback;
        stream_cfg.data_ptr = codec_capture_buf;

        KWS_TEST_LOG_I("codec capture sample_rate:%d, data_size:%d",
                stream_cfg.sample_rate,stream_cfg.data_size);
        af_stream_open(CODEC_STREAM_ID, AUD_STREAM_CAPTURE, &stream_cfg);
        ASSERT(ret == 0, "codec capture failed: %d", ret);

        memset(&stream_cfg, 0, sizeof(stream_cfg));
        stream_cfg.channel_num = CHANNEL_NUM;
        stream_cfg.data_size = TEMP_BUF_SIZE;
        stream_cfg.sample_rate = (enum AUD_SAMPRATE_T)KWS_RX_SAMPLE_RATE;
        stream_cfg.bits = (enum AUD_BITS_T)SAMPLE_BITS;;
        stream_cfg.vol = 12;
        stream_cfg.device = AUD_STREAM_USE_INT_CODEC;
        stream_cfg.io_path = AUD_OUTPUT_PATH_SPEAKER;
        stream_cfg.handler = codec_playback_callback;
        stream_cfg.data_ptr = codec_playback_buf;

        KWS_TEST_LOG_I("codec playback sample_rate:%d, data_size:%d",
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
        speech_wakeup_destroy(nn_kws_st);

        ext_heap_deinit();

        // app_overlay_unloadall();
        app_sysfreq_req(APP_SYSFREQ_USER_APP_0, APP_SYSFREQ_32K);
        af_set_priority(AF_USER_TEST, osPriorityAboveNormal);
    }

    isRun=on;
    return 0;
}

int32_t nn_kws_test(void)
{
    KWS_TEST_LOG_I("[%s]...", __func__);

    nn_kws_start(true);

    return 0;
}
