#ifdef BONE_SENSOR_SW_RESAMPLE
#include "string.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "resample_coef.h"
#include "sensor_resample.h"

//#define SENS_RSPL_DBG

#define RSPL_OUT_LEN(samprate,ssize,chan,frm) (samprate*ssize*chan*frm/1000)
#define RSPL_FRM_NUM(len, sr, ch, ss)         (len*1000/(sr*ch*ss))

typedef struct {
    bool status;
    int src_samprate;
    int dst_samprate;
    int chan_num;
    int samp_size;
    sbuffer_t rspl;
    sbuffer_t in;
    sbuffer_t out;
    RESAMPLE_ID id;
    struct RESAMPLE_CFG_T cfg;
} sensor_resample_ctrl_t;

static sensor_resample_ctrl_t sens_rc;

static uint8_t sensor_tbuff[SENS_RSPL_TBUF_SIZE];

sbuffer_t *sensor_sw_resample_get_out_buffer(void)
{
    return &sens_rc.out;
}

void sensor_sw_resample_enable(void)
{
    sens_rc.status = true;
}

void sensor_sw_resample_disable(void)
{
    sens_rc.status = false;
}

bool sensor_sw_resample_status(void)
{
    return sens_rc.status;
}

int sensor_sw_resample_open(
    uint32_t dst_samprate,
    uint32_t chan_num, uint32_t bits,
    uint8_t *rspl_buf, uint32_t rspl_buf_size,
    uint8_t *in_buf, uint32_t in_buf_size,
    uint8_t *out_buf, uint32_t out_buf_size)
{
    enum RESAMPLE_STATUS_T ret;
    RESAMPLE_ID rspl_id;
    struct RESAMPLE_CFG_T rspl_cfg;
    const struct RESAMPLE_COEF_T *coef = &resample_coef_any_up256;
    uint32_t rspl_size;

    uint32_t crystal_freq = hal_cmu_get_crystal_freq() / 1000;
    uint32_t pll_freq = CODEC_FREQ_24P576M / 1000;
    uint32_t src_samprate = crystal_freq * dst_samprate / pll_freq;
    float ratio = (float)src_samprate / (float)dst_samprate;

    rspl_size = audio_resample_ex_get_buffer_size(chan_num, bits, coef->phase_coef_num);
    ASSERT(rspl_size <= rspl_buf_size, "%s: error: rspl_size(%d) > rspl_buf_size(%d)",
        __func__, rspl_size, rspl_buf_size);

    BONE_SENSOR_TRACE(1, "%s: src_sr=%d,dst_sr=%d,chan=%d,bits=%d, rspl_size=%d",
        __func__, src_samprate, dst_samprate, chan_num, bits, rspl_size);

    BONE_SENSOR_TRACE(1, "%s: [addr,size]: rspl=[%x,%d], out=[%x,%d], in=[%x,%d]",
        __func__, (int)rspl_buf, rspl_buf_size, (int)out_buf, out_buf_size, (int)in_buf, in_buf_size);

    memset(&sens_rc, 0, sizeof(sens_rc));
    sens_rc.status       = false;
    sens_rc.src_samprate = src_samprate;
    sens_rc.dst_samprate = dst_samprate;
    sens_rc.chan_num     = chan_num;
    sens_rc.samp_size    = 2; //16bits
    sens_rc.rspl.buf     = rspl_buf;
    sens_rc.rspl.size    = rspl_size;
    sens_rc.in.buf       = in_buf;
    sens_rc.in.size      = in_buf_size;
    sens_rc.out.buf      = out_buf;
    sens_rc.out.size     = out_buf_size;

    memset(&rspl_cfg, 0, sizeof(rspl_cfg));
    rspl_cfg.chans = chan_num;
    rspl_cfg.bits  = bits;
    rspl_cfg.ratio_step = ratio;
    rspl_cfg.coef  = coef;
    rspl_cfg.buf   = sens_rc.rspl.buf;
    rspl_cfg.size  = sens_rc.rspl.size;
    memcpy((void *)&(sens_rc.cfg), (void *)&rspl_cfg, sizeof(rspl_cfg));

    BONE_SENSOR_TRACE(1, "%s: rspl_cfg: chan=%d, bits=%d,ratio*1000=%d,buf=%x,size=%d", __func__,
        chan_num, bits, (int)(ratio*1000), (int)(rspl_cfg.buf), rspl_cfg.size);

    ret = audio_resample_ex_open(&rspl_cfg, &rspl_id);
    ASSERT(ret == RESAMPLE_STATUS_OK, "%s: open resample failed %d", __func__, ret);
    sens_rc.id = rspl_id;
    return ret;
}

int sensor_sw_resample_close(void)
{
    audio_resample_ex_close(sens_rc.id);
    return 0;
}

static uint32_t sensor_data_extract_pcm16(uint8_t *data, uint32_t len,
    uint8_t *dst, uint32_t dst_size, uint32_t idx, uint32_t chan_num)
{
    int16_t *s = (int16_t *)data;
    int16_t *d = (int16_t *)dst;
    uint32_t i, j = 0;

    ASSERT(len*1/chan_num <= dst_size, "%s: tbuf overflow %d > %d",
        __func__, len*1/chan_num, dst_size);

    for(i = 0; i < len / 2; i += chan_num) {
        d[j++] = s[i+idx];
    }
    return j*2;
}

uint32_t sensor_sw_resample_process(uint8_t *buf, uint32_t len)
{
    enum RESAMPLE_STATUS_T ret = RESAMPLE_STATUS_OK;
    struct RESAMPLE_IO_BUF_T io;
    uint32_t in_size = 0, out_size = 0, out_len = 0, in_len = 0;
    sbuffer_t *out = &sens_rc.out;
    int16_t *in_buf  = (int16_t *)sensor_tbuff;

#ifdef SENS_RSPL_DBG
    uint32_t time = TICKS_TO_MS(hal_sys_timer_get());
#endif

    if (!sens_rc.status) {
        return 0;
    }

    in_len = sensor_data_extract_pcm16(buf, len, (uint8_t *)in_buf, SENS_RSPL_TBUF_SIZE,
        SENS_RSPL_SLOT_IDX, SENS_RSPL_SLOT_NUM);
    out_len = ALIGN(sens_rc.dst_samprate * in_len / sens_rc.src_samprate, 4);

#ifdef SENS_RSPL_DBG
    BONE_SENSOR_TRACE(1, "[%u] RSPL PROC:buf=%x,len=%d,out_len=%d,in_len=%d", time, (int)buf, len, out_len, in_len);
#endif

    io.in               = in_buf;
    io.in_size          = in_len;
    io.out              = out->buf + out->wpos;
    io.out_size         = out_len;
    io.out_cyclic_start = out->buf;
    io.out_cyclic_end   = out->buf + out->size;

    uint32_t time2 = hal_sys_timer_get();
#if 1
    ret = audio_resample_ex_run(sens_rc.id, &io, &in_size, &out_size);
    ASSERT((ret == RESAMPLE_STATUS_IN_EMPTY || ret == RESAMPLE_STATUS_DONE) && io.out_size >= out_size,
                 "%s: Failed to resample: %d io.out_size=%u in_size=%u out_size=%u", __func__,
                 ret, io.out_size, in_size, out_size);
#else
    memcpy(io.out, in_buf, out_len);
    in_size  = in_len;
    out_size = out_len-2;
    in_size  = in_size;
    out_size = out_size;
#endif
    time2 = TICKS_TO_US(hal_sys_timer_get() - time2);

    out->wpos += out_size;
    if (out->wpos >= out->size) {
        out->wpos -= out->size;
    }
#ifdef SENS_RSPL_DBG
    BONE_SENSOR_TRACE(1, "[%d] PROC COST [%d] us: in_size=%d,out_size=%d,wpos=%d,rpos=%d",
        time, time2, in_size,out_size,out->wpos,out->rpos);
#endif
    return ret;
}
#endif

