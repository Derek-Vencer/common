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
#ifndef __BT_SCO_CHAIN_H__
#define __BT_SCO_CHAIN_H__

#include <stdint.h>
#include <stdbool.h>

#if defined(SPEECH_TX_EQ)

typedef enum
{
    NTT_SPEECH_EQ_MODE_CURRENT = 0,
    NTT_SPEECH_EQ_MODE_NB_8K   = 1,
    NTT_SPEECH_EQ_MODE_WB_16K  = 2,
} NTT_SPEECH_EQ_MODE_T;

typedef struct
{
    uint8_t active;
    uint8_t mode;
    uint8_t bypass;
    uint8_t num;

    uint32_t sample_rate;
    int32_t master_gain_x1000;
} NTT_SPEECH_TX_EQ_INFO_T;

typedef struct
{
    uint8_t index;
    uint8_t type;
    uint16_t reserved;

    uint32_t frequency_hz;
    int32_t gain_x1000;
    uint32_t q_x1000;
} NTT_SPEECH_TX_EQ_BAND_T;

#ifdef __cplusplus
extern "C" {
#endif

int ntt_speech_tx_eq_get_info(NTT_SPEECH_EQ_MODE_T mode,NTT_SPEECH_TX_EQ_INFO_T *info);
int ntt_speech_tx_eq_get_band(NTT_SPEECH_EQ_MODE_T mode,uint8_t index,NTT_SPEECH_TX_EQ_BAND_T *band);
int ntt_speech_tx_eq_set_global(NTT_SPEECH_EQ_MODE_T mode,uint8_t bypass,int32_t master_gain_x1000,uint8_t num);
int ntt_speech_tx_eq_set_band(NTT_SPEECH_EQ_MODE_T mode,const NTT_SPEECH_TX_EQ_BAND_T *band);

    #ifdef __cplusplus
}
#endif

#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint8_t bypass;
    uint8_t type;

    int32_t comp_threshold_x1000;
    int32_t comp_ratio_x1000;

    int32_t expand_threshold_x1000;
    int32_t expand_ratio_x1000;

    int32_t attack_time_x1000;
    int32_t release_time_x1000;

    int32_t makeup_gain_x1000;

    uint32_t delay;

    int32_t tav_x1000;

} NTT_SPEECH_TX_COMPEXP_CFG_T;

int ntt_speech_tx_compexp_get(NTT_SPEECH_TX_COMPEXP_CFG_T *cfg);
int ntt_speech_tx_compexp_set(const NTT_SPEECH_TX_COMPEXP_CFG_T *cfg);

/*
 * Init speech algorithm, init all related states and memory
 * Input:
 *      tx_sample_rate: capture sample rate
 *      rx_sample_rate: playback sample rate
 *      tx_frame_ms:    capture frame size in ms
 *      rx_frame_ms:    playback frame size in ms
 *      sco_frame_ms:   sco frame size in ms(must be multiply of 7.5)
 *      buf:            buffer to be used for algorithms
 *      len:            buffer size
 */
int speech_init(int tx_sample_rate, int rx_sample_rate,int tx_frame_ms, int rx_frame_ms,int sco_frame_ms,uint8_t *buf, int len);
int speech_init2(int tx_sample_rate, int rx_sample_rate,int tx_frame_len, int rx_frame_len,int sco_frame_len,uint8_t *buf, int len);

/*
 * Deinit speech algorithm, free all related states and memory
 */
int speech_deinit(void);

/*
 * Uplink process
 * Input:
 *      pcm_buf: captured buffer by microphone, multi-channel is interleaved.
 *      ref_buf: reference buffer for acoustic echo canceller.
 *               buffee sample num should be tx_sample_rate * tx_frame_ms
 *      pcm_len: capture buffer sample num, should be tx_sample_rate * sco_frame_ms * ch_num
 * Output:
 *      pcm_buf: output buffer of speech algorithm, should be single channel
 *      pcm_len: output buffer sample num, should be tx_sample_rate * sco_frame_ms
 */
int speech_tx_process(void *pcm_buf, void *ref_buf, int *pcm_len);

/*
 * Downlink process
 * Input:
 *      pcm_buf: input buffer to be processed
 *      pcm_len: input buffer sample num, should be rx_sample_rate * sco_frame_ms
 * Output:
 *      pcm_buf: output buffer
 *      pcm_len: output buffer sample num, should be rx_sample_rate * sco_frame_ms
 */
int speech_rx_process(void *pcm_buf, int *pcm_len);

void *speech_get_ext_buff(int size);

/*audio dump test*/
int speech_tx_process_audio_dump(void *pcm_buf, int *_pcm_len, int channel_num);

#ifdef SPEECH_ALGO_DSP
int speech_enable_mcpp(bool en);
#endif

#ifdef __cplusplus
}
#endif

#endif