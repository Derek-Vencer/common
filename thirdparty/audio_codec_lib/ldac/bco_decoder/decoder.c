
#include "decoder.h"
#include "ldac_buffer_control.h"
#include "ldac_transport_stream.h"
#include "framebuffer_manager.h"
#include "ldacBT.h"
#include <stdint.h>
#include <stdlib.h>
#include "log.h"
#include <time.h>

#define NANOS 1000 
#define tm_now()                                   \
    ({                                             \
        struct time_ldac_spec t;                   \
        uint32_t sys_tick = hal_sys_timer_get();   \
        t.tv_sec = TICKS_TO_MS(sys_tick) / NANOS;  \
        t.tv_nsec = TICKS_TO_MS(sys_tick) % NANOS; \
        t;                                         \
    })

#define tm_add(ts, nanos)                                            \
    ({                                                               \
        const int64_t ns = (int64_t)(ts).tv_nsec + (int64_t)(nanos); \
        struct time_ldac_spec t__;                                   \
        t__.tv_sec = (ts).tv_sec + ns / NANOS;                       \
        t__.tv_nsec = ns % NANOS;                                    \
        t__;                                                         \
    })
#define tm_sub(ts, nsec)                                           \
    ({                                                             \
        struct time_ldac_spec t__;                                 \
        int32_t q = (int32_t)(nsec) / NANOS;                       \
        int32_t r = (int32_t)(nsec) % NANOS;                       \
        if ((ts).tv_nsec < r)                                      \
        {                                                          \
            const int32_t nsec__ = (r - (ts).tv_nsec) / NANOS + 1; \
            r -= nsec__ * NANOS;                                   \
            q += nsec__;                                           \
        }                                                          \
        if ((ts).tv_nsec - r > NANOS)                              \
        {                                                          \
            const int32_t nsec__ = ((ts).tv_nsec - r) / NANOS;     \
            r += nsec__ * NANOS;                                   \
            q -= nsec__;                                           \
        }                                                          \
        t__.tv_sec = (ts).tv_sec - q;                              \
        t__.tv_nsec = (ts).tv_nsec - r;                            \
        /* ts.tv_sec < q if subtract result is negative. */        \
        t__;                                                       \
    })
#define tm_diff_nsec(a, b)                                 \
    (((int64_t)(a).tv_sec - (int64_t)(b).tv_sec) * NANOS + \
     ((int64_t)(a).tv_nsec - (int64_t)(b).tv_nsec))

struct decoder_handle
{
    pthread_t tid;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    pthread_cond_t suspend_cond;

    int32_t quit;
    int32_t suspend_flag; /* 1:when suspended, otherwise 0. */

    struct fm_handle *hFM;

    decoder_callback callback;
    set_stream_configuration_cb config_cb;
    void *callback_context;

    HANDLE_LDAC_BT hdec;
    int32_t configured;        /* 1 when ldac_init_handle_decode()'ed, otherwise 0. */
    int32_t required_bc_reset; /* 1 when required bc_reset, otherwise 0. */
    enum decoder_frequency freq;
    enum decoder_channels chan;
    enum decoder_bits bits;
};

typedef struct decoder_handle HANDLE;

#define num_channels(decoder_channels) (decoder_channels == DEC_CHAN_MONO ? 1 : 2)

#ifdef PRINT_STAT
static void print_stat(HANDLE *h, int32_t passed, int32_t used, const char *bc_stat, double sma);
#endif

struct timing;
static struct timing *timing_new();
static void timing_reset(struct timing *timing, struct time_ldac_spec *base,
                         enum decoder_frequency frequency);

static int32_t frequency(enum decoder_frequency freq)
{
    switch (freq)
    {
    case DEC_FREQ_192000:
        return 192000;
    case DEC_FREQ_176400:
        return 176400;
    default:
    case DEC_FREQ_96000:
        return 96000;
    case DEC_FREQ_88200:
        return 88200;
    case DEC_FREQ_48000:
        return 48000;
    case DEC_FREQ_44100:
        return 44100;
    }
}

#ifdef PRINT_STAT
static int32_t word_length(enum decoder_bits bits)
{
    switch (bits)
    {
    case DEC_BITS_16:
        return 2;
    default:
    case DEC_BITS_24:
        return 3;
    case DEC_BITS_32:
        return 4;
    }
}
#endif

static int32_t configure_decoder(HANDLE *h);

HANDLE *decoder_open(decoder_callback cb, set_stream_configuration_cb conf_cb, void *context)
{
    HANDLE *h = malloc(sizeof(HANDLE));
    if (h)
    {
        memset(h, 0, sizeof(*h));
        h->freq = DEC_FREQ_96000;
        h->chan = DEC_CHAN_STEREO;
        h->bits = DEC_BITS_16;

        h->callback = NULL;  //cb ? cb : null_callback;
        h->config_cb = NULL; //conf_cb ? conf_cb : null_config_cb;
        h->callback_context = context;

        h->hFM = fm_get_handle();
        fm_init_handle(h->hFM);

        /* allocate ldacBT decoder library */
        //        open_lib_and_load_func();
        h->hdec = ldacBT_get_handle();

        pthread_mutex_init(&h->mutex, NULL);
        pthread_cond_init(&h->cond, NULL);
        pthread_cond_init(&h->suspend_cond, NULL);
    }
    return h;
}

int32_t decoder_configure(struct decoder_handle *handle, enum decoder_frequency freq,
                          enum decoder_channels chan_mode, enum decoder_bits bits)
{
    int32_t need_reconfigure;

    pthread_mutex_lock(&handle->mutex);
    need_reconfigure = !handle->configured;
    need_reconfigure |= handle->freq != freq || handle->chan != chan_mode;
    handle->freq = freq;
    handle->chan = chan_mode;
    handle->bits = bits;
    if (need_reconfigure)
    {
        configure_decoder(handle);
        handle->required_bc_reset = 1;
    }

    pthread_mutex_unlock(&handle->mutex);
    return need_reconfigure;
}

void decoder_write(struct decoder_handle *handle, const void *data, int32_t length,
                   int32_t frame_num)
{
    struct fm_handle *hFM;
    int32_t ret;
    struct fm_frame_check frame_check;

    pthread_mutex_lock(&handle->mutex);
    hFM = handle->hFM;
    pthread_mutex_unlock(&handle->mutex);

    frame_check.rate = handle->freq;
    frame_check.channel = handle->chan;
    ret = fm_push_frames(hFM, (uint8_t *)data, frame_num, length, &frame_check);
    if (frame_num != ret)
    {
        AUDIO_CODEC_LIB_TRACE(0,"Failed to push frame.");
        fm_dump(hFM);
    }

    pthread_mutex_lock(&handle->mutex);
    pthread_cond_signal(&handle->cond); /* notify new data available. */
    pthread_mutex_unlock(&handle->mutex);
}

void decoder_resume_decode(struct decoder_handle *handle)
{
    pthread_mutex_lock(&handle->mutex);

    fm_clear(handle->hFM);

    handle->quit = 0;
    if (handle->tid == 0)
    {
        pthread_create(&handle->tid, NULL, decoder, handle);
        AUDIO_CODEC_LIB_TRACE(0,"Created Decoder thread\n");
    }
    else
    {
        handle->suspend_flag = 0;
        AUDIO_CODEC_LIB_TRACE(0,"Request to RESUME\n");
        pthread_cond_signal(&handle->suspend_cond);
    }

    pthread_mutex_unlock(&handle->mutex);
}

static int32_t configure_decoder(HANDLE *h)
{
    const int32_t ch_mode = h->chan;
    const int32_t freq = frequency(h->freq);
    int32_t err;

    if (h->configured)
    {
        ldacBT_close_handle(h->hdec);
    }

    err = ldacBT_init_handle_decode(h->hdec, ch_mode, freq, 0, 0, 0);
    h->configured = !err;

    AUDIO_CODEC_LIB_TRACE(0,"Decoder %sconfigured (err = %d): channel mode=%d, frequency=%d",
        h->configured ? "" : "NOT ", err, ch_mode, freq);

    return err;
}

#ifdef PRINT_STAT
static void print_stat(HANDLE *h, int32_t passed, int32_t used, const char *bc_stat, double sma)
{
    /* mutex accessing handle member */
    pthread_mutex_lock(&h->mutex);
    do
    {
        struct fm_handle *const hFM = h->hFM;
        enum decoder_bits const bits = h->bits;
        char data[LDACBT_MAX_NBYTES];
        int32_t size;
        uint32_t caps;
        uint32_t total;
        struct ldac_ts_frame info;
        char kbps[32] = "---";
        static char msg[2048];
        const int32_t bitrate = ldacBT_get_bitrate(h->hdec);

        pthread_mutex_unlock(&h->mutex);

        if (bitrate != -1)
        {
            sprintf(kbps, 32, "%d", bitrate);
        }
        size = fm_peek_frame(hFM, (uint8_t *)data, sizeof(data));
        total = fm_get_all_size(hFM);
        caps = (FM_PACKET_MAX_SIZE * FM_PACKET_MAX_NUMBER);
        ts_parse_frame(data, size, NULL, &info);

        msg[0] = '\0';
        sprintf(msg, 2048, "fs=%d, cm=%d(%s), frm-stat=%d, frm-sz=%d, (%s kbps)",
                ({
                    int32_t fs;
                    switch (info.rate)
                    {
                    case LDAC_RATE_44100:
                        fs = 44100;
                        break;
                    case LDAC_RATE_48000:
                        fs = 48000;
                        break;
                    case LDAC_RATE_88200:
                        fs = 88200;
                        break;
                    case LDAC_RATE_96000:
                        fs = 96000;
                        break;
                    case LDAC_RATE_176400:
                        fs = 176400;
                        break;
                    case LDAC_RATE_192000:
                        fs = 192000;
                        break;
                    default:
                        fs = -1;
                        break;
                    }
                    fs;
                }) /* fs */,
                ({
                    int32_t cm;
                    switch (info.channel)
                    { /* channel index */
                    case LDAC_CH_STEREO:
                        cm = LDACBT_CHANNEL_MODE_STEREO;
                        break;
                    case LDAC_CH_DUAL:
                        cm = LDACBT_CHANNEL_MODE_DUAL_CHANNEL;
                        break;
                    case LDAC_CH_MONO:
                        cm = LDACBT_CHANNEL_MODE_MONO;
                        break;
                    default:
                        cm = -1;
                        break;
                    }
                    cm;
                }) /* channel mode */,
                ({
                    char *ch;
                    switch (info.channel)
                    {
                    case LDAC_CH_STEREO:
                        ch = "STEREO";
                        break;
                    case LDAC_CH_DUAL:
                        ch = "DUAL";
                        break;
                    case LDAC_CH_MONO:
                        ch = "MONO";
                        break;
                    default:
                        ch = "unknown";
                        break;
                    }
                    ch;
                }) /* channel */,
                info.status /* frame status */, passed /* bytes passed to decode */,
                kbps /* bps in previous decode */
        );

        {
            static char msgv[1024];
            sprintf(msgv, 1024,
                    " SNK STATS: used_byte=%d, output-wl:%d,"
                    " nfrm=%d (%6.2f%% consumed) -- BCO %s (%f)",
                    used /* bytes decoded */, word_length(bits) /* word length */,
                    fm_get_frame_number(hFM) /*  */,
                    100. * total / caps /* queue buffer usage */,
                    bc_stat /* buffer control status */,
                    sma /* simple moving average of frames in the queue */
            );
            strncat(msg, msgv, 1024);
        }

        AUDIO_CODEC_LIB_TRACE(0,"%s", msg);

    } while (0);
}
#endif

struct timing
{
    struct time_ldac_spec base;
    uint32_t frames; /* frames decoded from the time 'base' */
    int32_t duration;
};

static struct timing *timing_new()
{
    struct timing *const timing = malloc(sizeof(*timing));
    if (timing)
    {
        memset(timing, 0, sizeof(*timing));
        timing->duration = 1; /* avoid zero-division error. */
    }
    return timing;
}

static void timing_reset(struct timing *timing, struct time_ldac_spec *base,
                         enum decoder_frequency frequency)
{
    timing->base = *base;
    timing->frames = 0;

    switch (frequency)
    {
    case DEC_FREQ_176400: /* 512 samples, 176.4kHz */
    case DEC_FREQ_88200:  /* 256 samples,  88.2kHz */
    case DEC_FREQ_44100:  /* 128 samples,  44.1kHz */
        timing->duration = 44100;
        break;
    default: /* 512 samples, 192.0kHz */
             /* 256 samples,  96.0kHz */
             /* 128 samples,  48.0kHz */
        timing->duration = 48000;
        break;
    }
}

/**
 * @param curr current time. (absolute)
 * @param next recieves the absolute time next frame to be decoded.
 * @return number of frames to decode.
 */
static int32_t timing_update(struct timing *timing, const struct time_ldac_spec *curr,
                             struct time_ldac_spec *next)
{
    const int64_t elapsed_nsec = tm_diff_nsec(*curr, timing->base);
    const int32_t dur = timing->duration;
    int32_t to_decode =
        (int64_t)elapsed_nsec * (int64_t)dur / 128LL / NANOS - (int64_t)timing->frames + 1LL;

    AUDIO_CODEC_LIB_TRACE(0,"curr %d:%d  base %d:%d", curr->tv_sec, curr->tv_nsec, timing->base.tv_sec, timing->base.tv_nsec);
    AUDIO_CODEC_LIB_TRACE(0,"to_decode_=%d,elapsed %lld, dur %d, frames %lld", to_decode, elapsed_nsec, dur, (int64_t)timing->frames);

    if (to_decode > 0)
    {
        timing->frames += to_decode > 10 ? (to_decode = 10) : to_decode;
        /* when 'dur' frames decoded, elapsed just 128 second. */
        if (timing->frames >= dur)
        {
            timing->base.tv_sec += 128;
            timing->frames -= dur;
        }
    }
    else
    {
        to_decode = 0;
    }

    *next = tm_add(timing->base, (128LL * NANOS * (int64_t)timing->frames / (int64_t)dur));
    AUDIO_CODEC_LIB_TRACE(0,"to decode %d frames, elapsed %lld, wait %lld", to_decode, elapsed_nsec,
        tm_diff_nsec(*next, *curr));
    return to_decode;
}

////////////////////    add for a2dpplay use    /////////////////////////
#include "ldacBT_bco_for_fluoride.h"
extern struct decoder_handle *ldac_BCO_decoder_handle_get(HANDLE_LDAC_BCO hLdacBco);

struct buffer_control *bc = NULL;
struct timing *timing = NULL;
struct fm_handle *hFM = NULL;
struct time_ldac_spec curr;
struct time_ldac_spec next;
int32_t decoding = 0;

HANDLE_LDAC_BCO handle_bco;
struct decoder_handle *handle_decoder;
#include "cmsis.h"
#include "cmsis_os.h"
osMutexDef(bco_buffer_mutex);

osMutexId bco_buffer_mutex;
int inline bco_buffer_mutex_init(void)
{
    if (bco_buffer_mutex == NULL){
        bco_buffer_mutex = osMutexCreate((osMutex(bco_buffer_mutex)));
    }
    return 0;
}
int inline bco_buffer_mutex_lock(void)
{
    osMutexWait((osMutexId)bco_buffer_mutex, osWaitForever);
    return 0;
}

int inline bco_buffer_mutex_unlock(void)
{
    osMutexRelease((osMutexId)bco_buffer_mutex);
    return 0;
}


HANDLE_LDAC_BT ldac_BCO_decoder_get_hdec(struct decoder_handle *handle)
{
    if (NULL == handle_decoder)
        return NULL;
    struct decoder_handle *h = handle;
    return h->hdec;
}

void ldac_BCO_decoder_print(struct decoder_handle *handle)
{
    if (NULL == handle_decoder)
        return;
    AUDIO_CODEC_LIB_TRACE(0,"%s chan=%d, freq=%d, bits=%d",
         __func__, handle->chan, handle->freq, handle->bits);
}

int32_t ldac_BCO_decoder_get_frame_number(void)
{
    return fm_get_frame_number(hFM);
}

void ldac_BCO_decoder_init(void)
{
    bc = bc_new(DECODER_NUM_FRAMES_STARTS_DECODE);
    timing = timing_new();
    bc_reset(bc, true);
    decoding = 0;
}

HANDLE_LDAC_BT ldac_BCO_init_and_start(int32_t sample_rate, int32_t bits_per_sample, int32_t channel_mode)
{
    handle_bco = ldac_BCO_init(NULL);
    handle_decoder = ldac_BCO_decoder_handle_get(handle_bco);
    hFM = handle_decoder->hFM;
    AUDIO_CODEC_LIB_TRACE(0,"a2dp_audio_init sample Rate 00 =%d, channel_mode = %d\n", sample_rate, channel_mode);
    ldac_BCO_configure(handle_bco, sample_rate, 16, channel_mode);
    ldac_BCO_start(handle_bco);
    ldac_BCO_decoder_init();
    ldac_BCO_decoder_print(handle_decoder);
    bco_buffer_mutex_init();
    return ldac_BCO_decoder_get_hdec(handle_decoder);
}

void ldac_BCO_deinit(void)
{
    bco_buffer_mutex_unlock();
    ldac_BCO_cleanup(handle_bco);
    handle_bco = NULL;
    handle_decoder = NULL;
}

int32_t ldac_BCO_decoder_packet_fill(int32_t frames, void *data, int32_t length)
{
    if (NULL == handle_decoder)
        return 0;

    int32_t frames_in_bco;
    bco_buffer_mutex_lock();
    decoder_write(handle_decoder, data, length, frames);
    bco_buffer_mutex_unlock();
    frames_in_bco = ldac_BCO_decoder_get_frame_number();
    return frames_in_bco;
}

static ssize_t frames_to_decode_peek = 0;
int32_t ldac_BCO_data_peek_one_frame(uint8_t *p_buffer, uint16_t buffer_max_len, uint8_t update_bc)
{
    if (NULL == handle_decoder)
        return -(LDAC_BCO_DECODER_ERROR);

    struct decoder_handle *h = handle_decoder;
    ssize_t frames = 0;

    if (!decoding || (frames_to_decode_peek <= 0))
    {
        if (h->required_bc_reset)
        {
            bc_reset(bc, true);
            h->required_bc_reset = 0;
        }
        if (!decoding)
        {
            bc_reset(bc, false);
        }

        hFM = h->hFM;
        pthread_mutex_unlock(&h->mutex);

        curr = tm_now();
        frames = fm_get_frame_number(hFM);

        if (!decoding && frames >= DECODER_NUM_FRAMES_STARTS_DECODE)
        {
            AUDIO_CODEC_LIB_TRACE(0,"START decoding");
            decoding = 1;
            bc_reset(bc, false);
            timing_reset(timing, &curr, h->freq);
            //frames_to_decode_peek = DECODER_NUM_FRAMES_STARTS_DECODE / 4;
            // bc_update(bc, 0, &curr);
        }
        /* loop until certain number of frames queued in the buffer. */
        if (!decoding)
            return -(LDAC_BCO_DECODER_CACHING);

        frames_to_decode_peek = timing_update(timing, &curr, &next);
        if (frames_to_decode_peek <= 0)
        {
            return -(LDAC_BCO_DECODER_FRAMES_TO_DECODE_IS_0);
        }
    }

    bco_buffer_mutex_lock();
    if(update_bc)
    {
        curr = tm_now();
        frames = fm_get_frame_number(hFM);
        bc_update(bc, frames, &curr);
    }
    frames = fm_get_frame_number(hFM);

#if BCO_TRACE_OPEN
    struct time_ldac_spec time_look = tm_now();
    AUDIO_CODEC_LIB_TRACE(0,"%s: bco_frames=%d, tv_s=%d, tv_ns=%d", __func__, fm_get_frame_number(hFM), time_look.tv_sec, (uint32_t)time_look.tv_nsec);
#endif
    int32_t size;
    size = bc_peek(bc, hFM, p_buffer, buffer_max_len);

    bco_buffer_mutex_unlock();
    if (size < 0)
    {
        AUDIO_CODEC_LIB_TRACE(0,"Buffer underflow");
        bc_burst(bc, 2L * NANOS, &curr); /* 2sec */
        decoding = 0;
        return -(LDAC_BCO_DECODER_BUFFER_UNDERFLOW);
    }
    return size;
}

int32_t ldac_BCO_bc_pop_one_frame(void)
{
    frames_to_decode_peek--;
    return bc_pop(bc, hFM); /* drop frame */
}
