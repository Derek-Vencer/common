/***************************************************************************
 *
 * Copyright 2025-2030 BES.
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
#ifdef BT_A2DP_SUPPORT
#ifdef A2DP_SOURCE_MIHC_ON
#include "app_source_codec.h"

// mihc 2.0 (Insert multi-channel lossless version MIHC_MC into MIHC1.0)
#ifdef MIHC_V2_ENABLED
#include "mihc2_enc_interface.h"
// mihc 1.0
#else
#include "mihc_enc_interface.h"
#endif

#ifdef A2DP_ENCODER_CROSS_CORE
#include "a2dp_encoder_cc_bth.h"
#endif

btif_avdtp_codec_t a2dp_source_mihc_avdtpcodec;
const unsigned char a2dp_source_codec_mihc_elements[] = {
    0x8F, 0x03, 0x00, 0x00,     // Vendor ID: 0x0000 038F
    0x31, 0x4D,                 // Codec ID : 0x4D31
    A2DP_MIHC_VERSION_00,       // Version
    (A2DP_MIHC_SR_48000),
    (A2DP_MIHC_BD_16 | A2DP_MIHC_FD_10MS | A2DP_MIHC_CBR),
    (A2DP_MIHC_MODE_LOSSY | A2DP_MIHC_MODE_LOSSLESS | A2DP_MIHC_MODE_LOW_LATENCY | A2DP_MIHC_MODE_ADATPIVE | A2DP_MIHC_CM_MONO | A2DP_MIHC_CM_STEREO),
    A2DP_MIHC_MIN_BR_DEFAULT,   // min-bitrate is version-dependent
    A2DP_MIHC_MD,
    A2DP_MIHC_ALL_BITS_ZERO,
};

typedef struct _mihc_encoder_context
{
    uint32_t sample_rate;
    uint8_t bits_per_sample;
    uint8_t frame_duration;
    uint8_t num_channels;
    uint32_t input_samples;
    uint32_t bit_rate;
    uint32_t delay_mode;
    uint32_t trans_size;
    uint32_t linein_buf_size;
    void *transmit_buffer;
    void *mihc_mem_ptr;
    void *mihc_input;
    void *mihc_encoder;
} mihc_encoder_context_t;

mihc_encoder_context_t mihc_enc_ctx;

// Adaptive Bitrate table
uint32_t bt_table_mihc_16bit[] = {128, 192, 256, 320, 400, 500, 900, 1200, 1400};

uint32_t bt_table_mihc_24bit[] = {128, 192, 256, 320, 400, 500, 1400, 1900, 2100};

POSSIBLY_UNUSED uint32_t bt_table_mihc_multich[] =
{128, 192, 256, 320, 400, 500, 1000, 2000, 3000, 4000, 5000, 6000};

void a2dp_source_mihc_encoder_init(uint32_t sample_rate, uint8_t bits_per_sample, uint8_t frame_duration, uint8_t num_channels)
{
    uint32_t malloc_size = 0;
    uint32_t samples_per_frame = 0;
    uint32_t mtu_size = 500;
    uint32_t *abr_table = NULL;

    if (mihc_enc_ctx.mihc_encoder)
    {
        DEBUG_ERROR(0, "mihc_encoder_init: already init");
        return;
    }

    DEBUG_INFO(0, "mihc_encoder_init");

    mihc_enc_ctx.sample_rate = sample_rate;
    mihc_enc_ctx.bits_per_sample = bits_per_sample;
    mihc_enc_ctx.frame_duration = frame_duration;
    mihc_enc_ctx.num_channels = num_channels;

    // If sample_rate is 44.1khz, set samples as 48khz.
    samples_per_frame = ((sample_rate == 44100 ? 48000 : sample_rate) * frame_duration / 1000);

    mihc_enc_ctx.input_samples = samples_per_frame * num_channels;
    mihc_enc_ctx.trans_size = (mihc_enc_ctx.input_samples * (bits_per_sample / 8));
    mihc_enc_ctx.linein_buf_size = mihc_enc_ctx.trans_size * 2;

    switch (mihc_enc_ctx.frame_duration)
    {
        case 5:
#ifdef MIHC_V2_ENABLED
            mihc_enc_ctx.delay_mode = MIHC_LOW_DELAY;
#else
            mihc_enc_ctx.delay_mode = MLAC_LOW_DELAY;
#endif
            break;
        case 10:
#ifdef MIHC_V2_ENABLED
            mihc_enc_ctx.delay_mode = MIHC_NORMAL;
#else
            mihc_enc_ctx.delay_mode = MLAC_NORMAL;
#endif
            break;
        default:
            DEBUG_ERROR(0, "mihc_encoder_init: invalid frame_duration");
            return;
    }

    DEBUG_INFO(0, "mihc_encoder_init: sample_rate=%d, bits_per_sample=%d, frame_duration=%d, num_channels=%d",
            mihc_enc_ctx.sample_rate, mihc_enc_ctx.bits_per_sample, mihc_enc_ctx.frame_duration, mihc_enc_ctx.num_channels);
    DEBUG_INFO(0, "mihc_encoder_init: input_samples=%d, trans_size=%d, linein_buf_size=%d",
            mihc_enc_ctx.input_samples, mihc_enc_ctx.trans_size, mihc_enc_ctx.linein_buf_size);

    if (mihc_enc_ctx.trans_size && !mihc_enc_ctx.transmit_buffer)
    {
        app_audio_mempool_get_buff((uint8_t **)&mihc_enc_ctx.transmit_buffer, mihc_enc_ctx.trans_size * 2);
    }
    else
    {
        DEBUG_WARNING(0, "mihc_encoder_init: trans_size=%d, transmit_buffer=%x",
                    mihc_enc_ctx.trans_size, mihc_enc_ctx.transmit_buffer);
    }
    if (!mihc_enc_ctx.transmit_buffer)
    {
        DEBUG_ERROR(0, "mihc_encoder_init: transmit_buffer init error");
        return;
    }

#ifdef MIHC_V2_ENABLED
    malloc_size = mihc2_enc_get_malloc_size();
#else
    malloc_size = mlac_enc_get_malloc_size();
#endif
    if (malloc_size && !mihc_enc_ctx.mihc_mem_ptr)
    {
        malloc_size = 150 * 1024;
        app_audio_mempool_get_buff((uint8_t **)&mihc_enc_ctx.mihc_mem_ptr, malloc_size);
    }
    else
    {
        DEBUG_WARNING(0, "mihc_encoder_init: malloc_size=%d, mihc_mem_ptr=%x",
                    malloc_size, mihc_enc_ctx.mihc_mem_ptr);
    }
    if (!mihc_enc_ctx.mihc_mem_ptr)
    {
        DEBUG_ERROR(0, "mihc_encoder_init: encoder mem init error");
        return;
    }

    if (mihc_enc_ctx.bits_per_sample == 16)
    {
        abr_table = bt_table_mihc_16bit;
    }
    else if (mihc_enc_ctx.bits_per_sample == 24)
    {
        abr_table = bt_table_mihc_24bit;
    }
    else
    {
        // 32 bits not currently supported
        DEBUG_ERROR(0, "mihc_encoder_init: nonsupport bits_per_sample %d", mihc_enc_ctx.bits_per_sample);
        return;
    }

#ifdef MIHC_V2_ENABLED
    channel_mask_t channel_type;
    switch (mihc_enc_ctx.num_channels)
    {
        case 1:
            channel_type = CHANNEL_OUT_MONO;
            break;
        case 2:
            channel_type = CHANNEL_OUT_STEREO;
            break;
        default:
            // TODO: other channel type ?
            DEBUG_ERROR(0, "mihc_encoder_init: invalid channel type");
            return;
    }

    mihc2_enc_get_bitrate(abr_table, &mihc_enc_ctx.bit_rate, 4);

    mihc_enc_ctx.mihc_input = mihc2_enc_set_input_info(mihc_enc_ctx.sample_rate, mihc_enc_ctx.bits_per_sample, channel_type,
                                                       mihc_enc_ctx.input_samples, mihc_enc_ctx.bit_rate, mihc_enc_ctx.delay_mode,
                                                       mihc_enc_ctx.mihc_mem_ptr);

    mihc2_enc_set_mtu_size(mihc_enc_ctx.mihc_input, mtu_size);

    mihc_enc_ctx.mihc_encoder = mihc2_enc_init(mihc_enc_ctx.mihc_input);
#else
    // get bit rate based on mihc_abr_mtk_algo_proces.
    mlac_enc_get_bitrate(abr_table, &mihc_enc_ctx.bit_rate, 4);

    // set pcm input format based on input information.
    mihc_enc_ctx.mihc_input = mlac_enc_set_input_info(mihc_enc_ctx.sample_rate, mihc_enc_ctx.bits_per_sample, mihc_enc_ctx.num_channels,
                                                      mihc_enc_ctx.bit_rate, mihc_enc_ctx.delay_mode, mihc_enc_ctx.mihc_mem_ptr);

    // set mtu packet size.
    mlac_enc_set_mtu_size(mihc_enc_ctx.mihc_input, mtu_size);

    // init encoder based on input information.
    mihc_enc_ctx.mihc_encoder = mlac_enc_init(mihc_enc_ctx.mihc_input);

    // set input_len(input_samples)
    mlac_enc_set_input_len(mihc_enc_ctx.mihc_input, mihc_enc_ctx.input_samples);
#endif
}

void a2dp_source_mihc_encoder_deinit(void)
{
    DEBUG_INFO(0, "mihc_encoder_deinit");
    memset(&mihc_enc_ctx, 0, sizeof(mihc_encoder_context_t));
}

static bool a2dp_source_mcu_encode_mihc_packet(a2dp_source_packet_t *source_packet)
{
    bool enc_success = false;
    btif_a2dp_sbc_packet_t *mihc_packet = &source_packet->packet;
    uint8_t streaming_a2dp_id = app_bt_source_get_streaming_a2dp();
    uint32_t enc_stime = 0;
    uint32_t enc_etime = 0;

    if (streaming_a2dp_id == BT_DEVICE_INVALID_ID)
    {
        DEBUG_ERROR(0, "mcu_encode_mihc_packet: invalid streaming a2dp");
        return false;
    }

    enc_stime = hal_fast_sys_timer_get();

#ifdef MIHC_V2_ENABLED
    MIHC_ENC_OUT mihc_output = {0};
    mihc_output.buffer_out = mihc_packet->data;

    mihc2_enc_set_input_len(mihc_enc_ctx.mihc_input, mihc_enc_ctx.input_samples);
    mihc2_enc_set_bitrate(mihc_enc_ctx.mihc_input, mihc_enc_ctx.bit_rate, mihc_enc_ctx.delay_mode, 0);
    mihc2_enc_process(mihc_enc_ctx.mihc_encoder, mihc_enc_ctx.mihc_input, &mihc_output, mihc_enc_ctx.transmit_buffer);

    DEBUG_INFO(0, "mcu_encode_mihc_packet: mihc_success=%d", mihc_output.mihc_success);
    DEBUG_INFO(0, "packt_bytes_out=%d, frame_bytes_out=%d, packt_frame_num=%d, encoded_samples=%d", mihc_output.packt_bytes_out,
                mihc_output.frame_bytes_out, mihc_output.packt_frame_num, mihc_output.encoded_samples);

    if (mihc_output.mihc_success)
    {
        mihc_packet->dataLen = mihc_output.packt_bytes_out;
        mihc_packet->frameNum = mihc_output.packt_frame_num;
        mihc_packet->frameSize = 0xFFFF;
        source_packet->codec_type = BT_A2DP_CODEC_TYPE_NON_A2DP;
        enc_success = true;
    }
#else
    mlac_enc_out mlac_output = {0};
    mlac_output.buffer_out = mihc_packet->data;

    // reset the real time bit_rate and input_len(input_samples)
    mlac_enc_set_input_len(mihc_enc_ctx.mihc_input, mihc_enc_ctx.input_samples);
    mlac_enc_set_bitrate(mihc_enc_ctx.mihc_input, mihc_enc_ctx.bit_rate, mihc_enc_ctx.delay_mode, 0);
    mlac_enc_process(mihc_enc_ctx.mihc_encoder, mihc_enc_ctx.mihc_input, &mlac_output, mihc_enc_ctx.transmit_buffer);

    DEBUG_INFO(0, "mcu_encode_mihc_packet: mlac_success=%d", mlac_output.mlac_success);
    DEBUG_INFO(0, "packt_bytes_out=%d, frame_bytes_out=%d, packt_frame_num=%d, encoded_samples=%d", mlac_output.packt_bytes_out,
                mlac_output.frame_bytes_out, mlac_output.packt_frame_num, mlac_output.encoded_samples);

    if (mlac_output.mlac_success)
    {
        mihc_packet->dataLen = mlac_output.packt_bytes_out;
        mihc_packet->frameNum = mlac_output.packt_frame_num;
        mihc_packet->frameSize = 0xFFFF; // loss mode has same size, but lossless mode(bitrate > 900) each frame has different size.
        source_packet->codec_type = BT_A2DP_CODEC_TYPE_NON_A2DP;
        enc_success = true;
    }
#endif

    enc_etime = hal_fast_sys_timer_get();
    DEBUG_INFO(0, "mihc mcu encode time=%dus", FAST_TICKS_TO_US(enc_etime - enc_stime));

    return enc_success;
}

bool a2dp_source_encode_mihc_packet(a2dp_source_packet_t *source_packet)
{
#ifndef A2DP_ENCODER_CROSS_CORE
    // TODO:
    if (!mihc_enc_ctx.mihc_encoder)
    {
        uint8_t device_id = app_bt_source_get_current_a2dp();
        struct BT_SOURCE_DEVICE_T *curr_device = app_bt_source_get_device(device_id);
        a2dp_source_mihc_encoder_init(curr_device->aud_sample_rate, curr_device->base_device->sample_bit,
                                      curr_device->base_device->a2dp_frame_dr / 10, curr_device->base_device->a2dp_channel_num);
    }
#endif

#if defined(A2DP_ENCODE_CP_ACCEL)
    // TODO:
#elif defined(A2DP_ENCODER_CROSS_CORE)
    // TODO:
#else
    return a2dp_source_mcu_encode_mihc_packet(source_packet);
#endif
}

uint8_t *a2dp_source_mihc_frame_buffer(void)
{
    return (uint8_t *)mihc_enc_ctx.transmit_buffer;
}

uint32_t a2dp_source_mihc_trans_size(void)
{
    return mihc_enc_ctx.trans_size;
}

uint32_t a2dp_source_mihc_linein_buf_size(void)
{
    return mihc_enc_ctx.linein_buf_size;
}

void a2dp_source_register_mihc_codec(btif_a2dp_stream_t *btif_a2dp, btif_avdtp_content_prot_t *sep_cp, uint8_t sep_priority, btif_a2dp_callback callback)
{
    DEBUG_INFO(0, "register_mihc_codec");

    a2dp_source_mihc_avdtpcodec.codecType = BT_A2DP_CODEC_TYPE_NON_A2DP;
    a2dp_source_mihc_avdtpcodec.discoverable = true;
    a2dp_source_mihc_avdtpcodec.elements = (U8 *)&a2dp_source_codec_mihc_elements;
    a2dp_source_mihc_avdtpcodec.elemLen  = sizeof(a2dp_source_codec_mihc_elements);

    btif_a2dp_register(btif_a2dp, BTIF_A2DP_STREAM_TYPE_SOURCE, &a2dp_source_mihc_avdtpcodec, sep_cp, sep_priority, callback);
}

#endif /* A2DP_SOURCE_MIHC_ON */
#endif /* BT_A2DP_SUPPORT */