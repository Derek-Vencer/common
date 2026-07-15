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
#include "bt_sco_chain_lea_reframe.h"
#include "bt_sco_chain.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "speech_memory.h"
#include "mfifo.h"
#include "cmsis_os.h"

#define BT_SCO_CHAIN_LOG_I(str, ...)        TR_INFO(TR_MOD(AUD), "[BT_SCO_CHAIN]" str, ##__VA_ARGS__)
#if defined(_REFRAME_DEBUG)
#define BT_SCO_CHAIN_LOG_D(str, ...)        TR_DEBUG(TR_MOD(AUD), "[BT_SCO_CHAIN]" str, ##__VA_ARGS__)
static uint32_t g_debug_index = 0;
#else
#define BT_SCO_CHAIN_LOG_D(str, ...)
#endif


#if defined(SPEECH_TX_24BIT)
typedef int     _PCM_T;
#define SAMPLE_BYTES                     (4)
#else
typedef short   _PCM_T;
#define SAMPLE_BYTES                     (2)
#endif

#define PROCESS_FRAME_LEN                (240)
#define PCM_TX_CHANNEL_NUM               (SPEECH_CODEC_CAPTURE_CHANNEL_NUM)
#define PCM_FIFO_FRAME_NUM               (3)

#define PCM_OUT_DELAY                    (PROCESS_FRAME_LEN * (PCM_FIFO_FRAME_NUM - 1))

#define GET_OUT_FIFO_DATA_WAIT_TIME_US   (100)
#define GET_OUT_FIFO_DATA_WAIT_CNT       (10)

// Thread
#define REFRAME_THREAD_STACK_SIZE        (1024 * 4)
static osThreadId reframe_thread_tid;
static void _reframe_thread(void const *argument);
osThreadDef(_reframe_thread, osPriorityAboveNormal, 1, REFRAME_THREAD_STACK_SIZE, "speech_lea_reframe");

// Signal
#define REFRAME_SIGNAL_WRITE             (1)

typedef struct {
    struct mfifo pcm_fifo;
    uint8_t *pcm_fifo_mem;
    struct mfifo ref_fifo;
    uint8_t *ref_fifo_mem;
    struct mfifo out_fifo;
    uint8_t *out_fifo_mem;
} reframe_tx_fifo_t;

typedef struct {
    struct mfifo pcm_fifo;
    uint8_t *pcm_fifo_mem;
    struct mfifo out_fifo;
    uint8_t *out_fifo_mem;
} reframe_rx_fifo_t;

static reframe_tx_fifo_t g_tx_fifo   = {0};
static reframe_rx_fifo_t g_rx_fifo   = {0};
static uint8_t *process_tx_pcm_buf      = NULL;
static uint8_t *process_ref_buf      = NULL;
static uint8_t *process_rx_pcm_buf      = NULL;
static int g_process_status = 0;
static int g_close_flag = 0;

static reframe_tx_fifo_t *get_reframe_tx_fifo(void)
{
    return &g_tx_fifo;
}

static reframe_rx_fifo_t *get_reframe_rx_fifo(void)
{
    return &g_rx_fifo;
}

static bool tx_pcm_fifo_is_ready(void)
{
    reframe_tx_fifo_t *tx_fifo = get_reframe_tx_fifo();
    int32_t pcm_data_size = PROCESS_FRAME_LEN * PCM_TX_CHANNEL_NUM * SAMPLE_BYTES;
    int32_t out_data_size = PROCESS_FRAME_LEN * SAMPLE_BYTES;
    if (mfifo_len(&tx_fifo->pcm_fifo) >= pcm_data_size &&
        mfifo_get_free_space(&tx_fifo->out_fifo) >= out_data_size) {
        return true;
    } else {
        return false;
    }
}

static bool rx_pcm_fifo_is_ready(void)
{
    reframe_rx_fifo_t *rx_fifo = get_reframe_rx_fifo();
    int32_t pcm_data_size = PROCESS_FRAME_LEN * SAMPLE_BYTES;
    int32_t out_data_size = PROCESS_FRAME_LEN * SAMPLE_BYTES;
    if (mfifo_len(&rx_fifo->pcm_fifo) >= pcm_data_size &&
        mfifo_get_free_space(&rx_fifo->out_fifo) >= out_data_size) {
        return true;
    } else {
        return false;
    }
}

static void reframe_tx_process(void)
{
    reframe_tx_fifo_t *tx_fifo = get_reframe_tx_fifo();
    int32_t pcm_data_size = PROCESS_FRAME_LEN * PCM_TX_CHANNEL_NUM * SAMPLE_BYTES;
    int32_t pcm_len = PROCESS_FRAME_LEN * PCM_TX_CHANNEL_NUM;
    int32_t out_data_size = PROCESS_FRAME_LEN * SAMPLE_BYTES;

    if (!(mfifo_len(&tx_fifo->pcm_fifo) >= pcm_data_size &&
        mfifo_get_free_space(&tx_fifo->out_fifo) >= out_data_size)) {
        return;
    }

    mfifo_get(&tx_fifo->pcm_fifo, process_tx_pcm_buf, pcm_data_size);
    mfifo_get(&tx_fifo->ref_fifo, process_ref_buf, pcm_data_size / PCM_TX_CHANNEL_NUM);

    speech_tx_process(process_tx_pcm_buf, process_ref_buf, &pcm_len);

    mfifo_put(&tx_fifo->out_fifo, process_tx_pcm_buf, out_data_size);
}

static void reframe_rx_process(void)
{
    reframe_rx_fifo_t *rx_fifo = get_reframe_rx_fifo();
    int32_t pcm_data_size = PROCESS_FRAME_LEN * SAMPLE_BYTES;
    int32_t pcm_len = PROCESS_FRAME_LEN;
    int32_t out_data_size = PROCESS_FRAME_LEN * SAMPLE_BYTES;

    if (!(mfifo_len(&rx_fifo->pcm_fifo) >= pcm_data_size &&
        mfifo_get_free_space(&rx_fifo->out_fifo) >= out_data_size)) {
        return;
    }

    mfifo_get(&rx_fifo->pcm_fifo, process_rx_pcm_buf, pcm_data_size);

    speech_rx_process(process_rx_pcm_buf, &pcm_len);

    mfifo_put(&rx_fifo->out_fifo, process_rx_pcm_buf, out_data_size);
}

static void _reframe_thread(void const *argument)
{
    osEvent evt;
    uint32_t signals = 0;

    while (1) {
        evt = osSignalWait(0x0, osWaitForever);
        signals = evt.value.signals;

        // BT_SCO_CHAIN_LOG_D("[%s] status = %x, signals = %d", __func__, evt.status, evt.value.signals);

        if (evt.status == osEventSignal) {
            // BT_SCO_CHAIN_LOG_D("signal = %ld", signals);
            if (signals & (1 << REFRAME_SIGNAL_WRITE)) {
                while (1 && !g_close_flag) {
                    g_process_status = 1;
                    if (tx_pcm_fifo_is_ready()) {
                        reframe_tx_process();
                    }
                    if (rx_pcm_fifo_is_ready()) {
                        reframe_rx_process();
                    }
                    if (!tx_pcm_fifo_is_ready() && !rx_pcm_fifo_is_ready()) {
                        break;
                    }
                }
                g_process_status = 0;
            }
        } else {
            ASSERT(0, "[%s] ERROR: evt.status = %d", __func__, evt.status);
            continue;
        }
    }
}

int32_t bt_sco_chain_lea_reframe_open(int tx_sample_rate, int rx_sample_rate,
                                int tx_frame_len, int rx_frame_len,
                                int sco_frame_len,
                                uint8_t *buf, int len)
{
    BT_SCO_CHAIN_LOG_I("[%s] ...", __func__);
    ASSERT(tx_frame_len != PROCESS_FRAME_LEN, "[%s] tx_frame_len(%d) == PROCESS_FRAME_LEN", __func__, tx_frame_len);

    int frame_len = tx_sample_rate / 1000 * SPEECH_PROCESS_FRAME_MS;
    speech_init2(tx_sample_rate, rx_sample_rate, frame_len, PROCESS_FRAME_LEN, PROCESS_FRAME_LEN, buf, len);

    // Tx fifo alloc
    reframe_tx_fifo_t *tx_fifo = get_reframe_tx_fifo();
    uint32_t in_data_size = PROCESS_FRAME_LEN * SAMPLE_BYTES * PCM_TX_CHANNEL_NUM * PCM_FIFO_FRAME_NUM;
    uint32_t ref_date_size = PROCESS_FRAME_LEN * SAMPLE_BYTES * PCM_FIFO_FRAME_NUM;
    uint32_t out_data_size = PROCESS_FRAME_LEN * SAMPLE_BYTES * PCM_FIFO_FRAME_NUM;
    tx_fifo->pcm_fifo_mem = speech_calloc(in_data_size, 1);
    tx_fifo->ref_fifo_mem = speech_calloc(ref_date_size, 1);
    tx_fifo->out_fifo_mem = speech_calloc(out_data_size, 1);
    mfifo_init(&tx_fifo->pcm_fifo, tx_fifo->pcm_fifo_mem, in_data_size);
    mfifo_init(&tx_fifo->ref_fifo, tx_fifo->ref_fifo_mem, ref_date_size);
    mfifo_init(&tx_fifo->out_fifo, tx_fifo->out_fifo_mem, out_data_size);

    // Rx fifo alloc
    reframe_rx_fifo_t *rx_fifo = get_reframe_rx_fifo();
    in_data_size = PROCESS_FRAME_LEN * SAMPLE_BYTES * PCM_FIFO_FRAME_NUM;
    out_data_size = PROCESS_FRAME_LEN * SAMPLE_BYTES * PCM_FIFO_FRAME_NUM;
    rx_fifo->pcm_fifo_mem = speech_calloc(in_data_size, 1);
    rx_fifo->out_fifo_mem = speech_calloc(out_data_size, 1);
    mfifo_init(&rx_fifo->pcm_fifo, rx_fifo->pcm_fifo_mem, in_data_size);
    mfifo_init(&rx_fifo->out_fifo, rx_fifo->out_fifo_mem, out_data_size);

    uint32_t process_data_len = PROCESS_FRAME_LEN * PCM_TX_CHANNEL_NUM;
    process_tx_pcm_buf = speech_calloc(process_data_len, SAMPLE_BYTES);
    process_ref_buf = speech_calloc(PROCESS_FRAME_LEN, SAMPLE_BYTES);
    process_rx_pcm_buf = speech_calloc(PROCESS_FRAME_LEN, SAMPLE_BYTES);

    BT_SCO_CHAIN_LOG_I("[%s] Pust %d pcm into out FIFO", __func__, PCM_OUT_DELAY);
    mfifo_put(&tx_fifo->out_fifo, (uint8_t *)tx_fifo->pcm_fifo_mem, PCM_OUT_DELAY * SAMPLE_BYTES);
    mfifo_put(&rx_fifo->out_fifo, (uint8_t *)rx_fifo->pcm_fifo_mem, PCM_OUT_DELAY * SAMPLE_BYTES);

    if (reframe_thread_tid == NULL) {
        reframe_thread_tid = osThreadCreate(osThread(_reframe_thread), NULL);
        ASSERT(reframe_thread_tid, "%s, reframe_thread create error", __func__);
        osSignalSet(reframe_thread_tid, 0x0);
    }
    g_close_flag = 0;

#if defined(_REFRAME_DEBUG)
    g_debug_index = 0;
#endif

    return 0;
}

int32_t bt_sco_chain_lea_reframe_close(void)
{
    BT_SCO_CHAIN_LOG_I("[%s] ...", __func__);

    // TODO: Call thridpart algo deinit function

    if (reframe_thread_tid) {
        g_close_flag = 1;
        while (g_process_status) {
            osDelay(1);
        }
        osThreadTerminate(reframe_thread_tid);
        reframe_thread_tid = NULL;
    }

    reframe_tx_fifo_t *tx_fifo = get_reframe_tx_fifo();
    reframe_rx_fifo_t *rx_fifo = get_reframe_rx_fifo();

    speech_free(process_rx_pcm_buf);
    speech_free(process_ref_buf);
    speech_free(process_tx_pcm_buf);

    speech_free(rx_fifo->out_fifo_mem);
    speech_free(rx_fifo->pcm_fifo_mem);

    speech_free(tx_fifo->out_fifo_mem);
    speech_free(tx_fifo->ref_fifo_mem);
    speech_free(tx_fifo->pcm_fifo_mem);

    speech_deinit();

    return 0;
}

int32_t bt_sco_chain_lea_reframe_tx_process(void *pcm_buf, void *ref_buf, int *pcm_len)
{
    int frame_len = *pcm_len / PCM_TX_CHANNEL_NUM;
#if defined(_REFRAME_DEBUG)
    BT_SCO_CHAIN_LOG_D("[REFRAME] %d: frame_len = %d", g_debug_index++, frame_len);
#endif

    reframe_tx_fifo_t *tx_fifo = get_reframe_tx_fifo();
    int32_t pcm_data_size = frame_len * PCM_TX_CHANNEL_NUM * SAMPLE_BYTES;
    int32_t data_size = frame_len * SAMPLE_BYTES;

    if (mfifo_get_free_space(&tx_fifo->pcm_fifo) >= pcm_data_size) {
        mfifo_put(&tx_fifo->pcm_fifo, (uint8_t *)pcm_buf, pcm_data_size);
        if (ref_buf){
            mfifo_put(&tx_fifo->ref_fifo, (uint8_t *)ref_buf, data_size);
        }
    } else {
        FS_TRACE(0, "[%s] Input buffer is overflow", __func__);
    }
    osSignalSet(reframe_thread_tid, (1 << REFRAME_SIGNAL_WRITE));

    int32_t cnt = 0;
    int32_t out_size = frame_len * SAMPLE_BYTES;
    while (true) {
        if (mfifo_len(&tx_fifo->out_fifo) >= out_size) {
            mfifo_get(&tx_fifo->out_fifo, (uint8_t *)pcm_buf, out_size);
            break;
        } else {
            if (cnt <= GET_OUT_FIFO_DATA_WAIT_CNT) {
                FS_TRACE(0, "[%s] Output buffer is underflow, wait cnt:%d.", __func__, cnt);
                hal_sys_timer_delay_us(GET_OUT_FIFO_DATA_WAIT_TIME_US);
                cnt ++;
            } else {
                FS_TRACE(0, "[%s] Output buffer is underflow, out set to zero!", __func__);
                memset(pcm_buf, 0, out_size);
                break;
            }
        }
    }

    *pcm_len = frame_len;
    return 0;
}

int32_t bt_sco_chain_lea_reframe_rx_process(void *pcm_buf, int *pcm_len)
{
    int frame_len = *pcm_len;
#if defined(_REFRAME_DEBUG)
    BT_SCO_CHAIN_LOG_D("[REFRAME] %d: frame_len = %d", g_debug_index++, frame_len);
#endif

    reframe_rx_fifo_t *rx_fifo = get_reframe_rx_fifo();
    int32_t pcm_data_size = frame_len * SAMPLE_BYTES;

    if (mfifo_get_free_space(&rx_fifo->pcm_fifo) >= pcm_data_size) {
        mfifo_put(&rx_fifo->pcm_fifo, (uint8_t *)pcm_buf, pcm_data_size);
    } else {
        FS_TRACE(0, "[%s] Input buffer is overflow", __func__);
    }
    osSignalSet(reframe_thread_tid, (1 << REFRAME_SIGNAL_WRITE));

    int32_t cnt = 0;
    int32_t out_size = frame_len * SAMPLE_BYTES;
    while (true) {
        if (mfifo_len(&rx_fifo->out_fifo) >= out_size) {
            mfifo_get(&rx_fifo->out_fifo, (uint8_t *)pcm_buf, out_size);
            break;
        } else {
            if (cnt <= GET_OUT_FIFO_DATA_WAIT_CNT) {
                FS_TRACE(0, "[%s] Output buffer is underflow, wait cnt:%d.", __func__, cnt);
                hal_sys_timer_delay_us(GET_OUT_FIFO_DATA_WAIT_TIME_US);
                cnt ++;
            } else {
                FS_TRACE(0, "[%s] Output buffer is underflow, out set to zero!", __func__);
                memset(pcm_buf, 0, out_size);
                break;
            }
        }
    }

    *pcm_len = frame_len;
    return 0;
}
