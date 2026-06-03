/*******************************************************************************
    Copyright 2016-2017 Sony Corporation
*******************************************************************************/
#ifndef DECODER_H__
#define DECODER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "ldac.h"

#define DECODER_BUFFER_SIZE 32768
#define DECODER_NUM_FRAMES_STARTS_DECODE 64
#define DECODER_NUM_FRAMES_MAX 256

enum decoder_frequency {
    DEC_FREQ_192000,
    DEC_FREQ_176400,
    DEC_FREQ_96000,
    DEC_FREQ_88200,
    DEC_FREQ_48000,
    DEC_FREQ_44100,
};

enum decoder_channels {
    DEC_CHAN_STEREO = LDAC_CHANNEL_STEREO,
    DEC_CHAN_DUAL = LDAC_CHANNEL_DUAL,
    DEC_CHAN_MONO = LDAC_CHANNEL_MONO,
};

enum decoder_bits {
    DEC_BITS_32,
    DEC_BITS_24,
    DEC_BITS_16,
};

struct decoder_handle;

/** callback function.
 *
 * called in decoder thread context.
 */
typedef void (*decoder_callback)(void *context, void *data, int32_t length,
                                 enum decoder_frequency frequency, enum decoder_channels channels,
                                 enum decoder_bits bits);

typedef void (*set_stream_configuration_cb)(enum decoder_frequency frequency,
                                            enum decoder_channels channels,
                                            enum decoder_bits bits);

struct decoder_handle *decoder_open(decoder_callback cb, set_stream_configuration_cb config_cb,
                                    void *context);
void decoder_close(struct decoder_handle *handle);

int32_t decoder_configure(struct decoder_handle *handle, enum decoder_frequency frequency,
                          enum decoder_channels channels, enum decoder_bits bits);

void decoder_write(struct decoder_handle *handle, const void *data, int32_t length,
                   int32_t frame_num);

void decoder_resume_decode(struct decoder_handle *handle);

void decoder_suspend_decode(struct decoder_handle *handle);

#ifdef __cplusplus
}
#endif

#endif /* DECODER_H__ */
