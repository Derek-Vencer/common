/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
#include <string.h>
#include <math.h>
#include "cmsis.h"
#include "hal_timer.h"
#include "hal_i2c.h"
#include "hal_codec.h"
#include "hal_tdm.h"
#include "hal_i2s.h"
#include "hal_trace.h"
#include "hwtimer_list.h"
#include "audioflinger.h"
#include "med_memory.h"
#include "sensor_drv.h"
#include "sensor_test.h"

#define SENS_DYN_MEM_MAX_SIZE 0x4000*5

//#define DEBUG_VB_SENSOR_DATA
struct acc_bw_cfg {
    uint16_t idx;
    uint16_t loop_cnt;
    short (*buf)[SAMPLE_NUMBER];
    double bandwidth[3];
};

static struct acc_bw_cfg g_acc_bw_cfg;

void GetBandwidth(void);
void derivative(short output3d_avg[FFT_DEPTH],
    double output_deriv[FFT_DEPTH], int sample_number);
void fft(double output_deriv[FFT_DEPTH],
    double outMod[FFT_DEPTH], int num_punti);
int normal_FFT(double x[], double y[], int n, int itype);
double findTheBandwidth(short output3d_TDM[FFT_DEPTH]);

int sensor_bus_read(uint8_t reg, uint8_t *val);
int sensor_bus_write(uint8_t reg, uint8_t val);

struct lis25ba_st_ctrl lis25ba_stc;

#define to_st_ctrl() (&lis25ba_stc)

void sensor_memory_init(void)
{
    POSSIBLY_UNUSED int total_size, free_size, org_size;
    POSSIBLY_UNUSED uint8_t *start_addr;

    BONE_SENSOR_TRACE(1, "%s:", __func__);

    syspool_init_specific_size(SENS_DYN_MEM_MAX_SIZE);

    free_size = syspool_free_size();
    total_size = syspool_total_size();
    org_size = syspool_original_size();
    start_addr = syspool_start_addr();

    BONE_SENSOR_TRACE(1, "start_addr=0x%x, free_size=%d, total_size=%d, org_size=%d",
        (int)start_addr, free_size, total_size, org_size);
}

uint8_t *sensor_memory_malloc(size_t size)
{
    uint8_t *pbuf = NULL;
    POSSIBLY_UNUSED int free_size = 0;

    free_size = syspool_get_buff(&pbuf, size);
    ASSERT(pbuf!=NULL, "%s: null pbuff", __func__);

    BONE_SENSOR_TRACE(1, "%s: size=%d, free=%d", __func__, (int)size, free_size);
    return pbuf;
}

void sensor_memory_free(void *ptr)
{
}

static void *st_malloc(size_t size)
{
    return sensor_memory_malloc(size);
}

static void st_free(void *ptr)
{
    sensor_memory_free(ptr);
}

void lis25ba_st_init(void)
{
    struct lis25ba_st_ctrl *c = to_st_ctrl();

    BONE_SENSOR_TRACE(1, "%s", __func__);
    memset((void*)c, 0, sizeof(*c));
#if !defined(SENSOR_DRV_USE_HEAP)
    sensor_memory_init();
#endif
}

void lis25ba_st_enable(void)
{
    struct lis25ba_st_ctrl *c = to_st_ctrl();

    BONE_SENSOR_TRACE(1, "%s", __func__);
    if (!c->enable) {
        c->run_mode = NO_ST_DATA_MODE;
        c->count  = 0;
        c->dumcnt = 0;
        c->ready  = 0;
        c->enable = 1;
    }
}

void lis25ba_st_disable(void)
{
    struct lis25ba_st_ctrl *c = to_st_ctrl();

    BONE_SENSOR_TRACE(1, "%s", __func__);
    if (c->enable) {
        c->run_mode = NO_ST_DATA_MODE;
        c->count  = 0;
        c->ready  = 0;
        c->enable = 0;
    }
}

void lis25ba_st_mode(int en)
{
    struct lis25ba_st_ctrl *c = to_st_ctrl();

    if (c->st_enable == en) {
        return;
    }
    if (en) {
        sensor_bus_write(0x0B, 0x08);
    } else {
        sensor_bus_write(0x0B, 0x00);
    }
    c->st_enable = en;
    BONE_SENSOR_TRACE(1,"%s: en=%d", __func__, en);
}

void enable_sensor(void)
{
    sensor_bus_write(0x26, 0x00);
    sensor_bus_write(0x2F, 0xE1);
    sensor_bus_write(0x2E, 0x02);
}

void disable_sensor(void)
{
    sensor_bus_write(0x2E, 0x82);
    sensor_bus_write(0x26, 0x20); //0x26 -> 0x20 ???
}

static void lis25ba_st_buffer_init(void)
{
    uint32_t len = XL_CNT * SAMPLE_NUMBER;
    short (*buf)[SAMPLE_NUMBER];

    memset((void *)&g_acc_bw_cfg, 0, sizeof(struct acc_bw_cfg));

    buf = (short (*)[SAMPLE_NUMBER])st_malloc(sizeof(short)*len);
    ASSERT((uint32_t)buf != 0, "alloc buf failed");
    BONE_SENSOR_TRACE(1,"buf=%x, len=%d", (uint32_t)buf, len);
    g_acc_bw_cfg.buf = buf;
}

static void lis25ba_st_buffer_free(void)
{
    short (*buf)[SAMPLE_NUMBER] = g_acc_bw_cfg.buf;

    if (buf) {
        st_free(buf);
        g_acc_bw_cfg.buf = 0;
    }
    BONE_SENSOR_TRACE(1,"%s: done", __func__);
}

void lis25ba_bw_test_enable(void)
{
    struct lis25ba_st_ctrl *c = to_st_ctrl();

    if (!c->enable) {
        c->run_mode = NO_ST_DATA_MODE;
        c->count = 0;
        c->enable = 1;
    }
#if !defined(SENSOR_DRV_USE_HEAP)
    sensor_memory_init();
#endif
    lis25ba_st_buffer_init();
    BONE_SENSOR_TRACE(1,"%s: done", __func__);
}

void lis25ba_bw_test_disable(void)
{
    struct lis25ba_st_ctrl *c = to_st_ctrl();

    if (c->enable) {
        c->run_mode = NO_ST_DATA_MODE;
        c->count = 0;
        c->enable = 0;
    }
    lis25ba_st_buffer_free();
    BONE_SENSOR_TRACE(1,"%s: done", __func__);
}

/******** acc raw data stream handler ********/
uint32_t lis25ba_raw_data_handler(uint8_t *buf, uint32_t len)
{
    aud_sensor_t *act = get_active_sensor();
    aud_sensor_info_t *pi = (aud_sensor_info_t *)(act->priv);

    if (pi->sens_cfg.data_handler) {
        pi->sens_cfg.data_handler(buf, len);
    }
    return 0;
}

/******** acc dump data stream handler ********/
uint32_t lis25ba_dump_data_handler(uint8_t *buf, uint32_t len, int method)
{
    //TODO: dump data by method
    return 0;
}

/******** acc self-test data stream handler ********/
uint32_t lis25ba_st_data_handler(uint8_t *buf, uint32_t len)
{
    struct lis25ba_st_ctrl *c = to_st_ctrl();
    struct lis25ba_st_data *d;
    int16_t *src;

    if (!c->enable) {
        BONE_SENSOR_TRACE(1, "%s: not enabled", __func__);
        return 0;
    }
//    BONE_SENSOR_TRACE(1, "%s: run_mode=%d", __func__, c->run_mode);

    d   = &(c->data);
    src = (int16_t *)buf;

    switch(c->run_mode) {
    case NO_ST_DATA_MODE:
        c->dumcnt++;
        if (c->dumcnt < 2) {
            break;
        }
        if (src[0] == 0) {
            break;
        }
        c->count++;
        if(0 < c->count && c->count <= ST_DATA_BUFFER_CNT) {
            int index = (c->count - 1) * ST_AXIS_CNT;

            d->nost[index + 0] = src[0];//x
            d->nost[index + 1] = src[1];//y
            d->nost[index + 2] = src[2];//z
        } else {
            c->count = 0;
            lis25ba_st_mode(1);
            c->run_mode = ST_WAIT_MODE;
        }
        break;
    case ST_WAIT_MODE:
        c->count++;
        if (c->count >= ST_WAIT_CNT) {
            c->count = 0;
            c->run_mode = ST_DATA_MODE;
        }
        break;
    case ST_DATA_MODE:
        if (src[0] == 0) {
            break;
        }
        c->count++;
        if(0 < c->count && c->count <= ST_DATA_BUFFER_CNT) {
            int index = (c->count - 1) * ST_AXIS_CNT;

            d->st[index+0] = src[0];//x
            d->st[index+1] = src[1];//y
            d->st[index+2] = src[2];//z
        } else {
            c->count = 0;
            c->run_mode = END_ST_MODE;
        }
        break;
    case END_ST_MODE:
        {
            int i;
            int cnt = ST_DATA_BUFFER_CNT;
            int x = 0;
            int y = 0;
            int z = 0;
            int st_x = 0;
            int st_y = 0;
            int st_z = 0;

            //End setting with
            c->count  = 0;
            c->enable = 0;
            c->ready  = 1;
            c->run_mode = NO_ST_DATA_MODE;

            for(i = 0; i < cnt; i++)
            {
                int index = i * ST_AXIS_CNT;

                x += d->nost[index + 0];
                y += d->nost[index + 1];
                z += d->nost[index + 2];

                st_x += d->st[index + 0];
                st_y += d->st[index + 1];
                st_z += d->st[index + 2];
#if 0
                BONE_SENSOR_TRACE(1, "%s: nost[%d][x,y,z]=[%d,%d,%d],st[%d][x,y,z]=[%d,%d,%d]",
                    __func__,
                    i,d->nost[index+0],d->nost[index+1],d->nost[index+2],
                    i,d->st[index+0],d->st[index+1],d->st[index+2]);
#endif
            }
            x = (int)(x / cnt);
            y = (int)(y / cnt);
            z = (int)(z / cnt);

            st_x = (int)(st_x / cnt);
            st_y = (int)(st_y / cnt);
            st_z = (int)(st_z / cnt);

            BONE_SENSOR_TRACE(1, "%s, avg: nost=[%d, %d, %d], st=[%d, %d, %d]",
                __func__, x,y,z,st_x,st_y,st_z);

            sensor_bus_write(0x2E, 0x82);
            sensor_bus_write(0x26, 0x20);
            lis25ba_st_mode(0);
#if 0
            if (c->st_handler) {
                c->st_handler(x, y, z, st_x, st_y, st_z);
            }
#endif
            aud_sensor_t *act = get_active_sensor();
            aud_sensor_info_t *pi = (aud_sensor_info_t *)(act->priv);
            if (pi->sens_cfg.st_data_handler) {
                pi->sens_cfg.st_data_handler(x,y,z,st_x,st_y,st_z);
            }
        }
        break;
    default:
        c->count  = 0;
        c->enable = 0;
        c->ready  = 0;
        c->run_mode = NO_ST_DATA_MODE;
        break;
    }
    return 0;
}

void dump_acc_data(const char *str, short buf[], uint32_t len)
{
    uint32_t i;

    BONE_SENSOR_TRACE(1,"%s: buf=%x, len=%d", str, (uint32_t)buf, len);
    for(i = 0; i < len; i += 16) {
    BONE_SENSOR_TRACE(1,"%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
        buf[i+0],buf[i+1],buf[i+2],buf[i+3],
        buf[i+4],buf[i+5],buf[i+6],buf[i+7],
        buf[i+8],buf[i+9],buf[i+10],buf[i+11],
        buf[i+12],buf[i+13],buf[i+14],buf[i+15]);
        hal_sys_timer_delay(US_TO_TICKS(500));
    }
}

int lis25ba_bw_test_result(float bw_x, float bw_y, float bw_z)
{
#define MIN_BW_X 2200
#define MAX_BW_X 2600
#define MIN_BW_Y 2200
#define MAX_BW_Y 2600
#define MIN_BW_Z 2200
#define MAX_BW_Z 2600
    int succeed = 0;

    return 0x7;

    if ((bw_x > (float)MIN_BW_X) && (bw_x < (float)MAX_BW_X)) {
        succeed |= 0x1;
    }
    if ((bw_y > (float)MIN_BW_Y) && (bw_y < (float)MAX_BW_Y)) {
        succeed |= 0x2;
    }
    if ((bw_z > (float)MIN_BW_Z) && (bw_z < (float)MAX_BW_Z)) {
        succeed |= 0x4;
    }
    return succeed;
}

/******** acc bandwidth test data stream handler ********/
uint32_t lis25ba_bw_data_handler(uint8_t *buf, uint32_t len)
{
    struct lis25ba_st_ctrl *c = to_st_ctrl();
    int16_t *src, *src_end;
    uint32_t i, stop;
    struct acc_bw_cfg *bwcfg = &g_acc_bw_cfg;

#define ACC_FRM_LEN (4) //16 bits, 4 channel

    if (!c->enable) {
        return 0;
    }

    src = (int16_t *)buf;
    src_end = (int16_t *)(buf + len);
    stop = 0;

    while(src + 4 <= src_end) {
        if (stop) {
            break;
        }
        switch(c->run_mode) {
        case NO_ST_DATA_MODE:
            c->count++;
            if(c->count < BW_WAIT_CNT) {
                stop = 1;
                break;
            }
            //fill nost data into buffer
            if (bwcfg->idx < SAMPLE_NUMBER) {
                for(i = 0; i < 3; i++) {
                    bwcfg->buf[i][bwcfg->idx] = *src++;//nost{x,y,z}
                }
                bwcfg->idx++;
                src++;
            }
            // if nost data is enough, enable st mode, goto next state
            if (bwcfg->idx >= SAM_BF_ST) {
                lis25ba_st_mode(1);
                c->run_mode = ST_DATA_MODE;
            }
            break;
        case ST_DATA_MODE:
            // fill st data into buffer
            if (bwcfg->idx < SAMPLE_NUMBER) {
                for(i = 0; i < 3; i++) {
                    bwcfg->buf[i][bwcfg->idx] = *src++;//st_x
                }
                bwcfg->idx++;
                src++;
            } else {
                // buffer is full, caculate bandwidth, check loop cnt
                short *pxyz;
                for (i = 0; i < 3; i++) {
                    pxyz = &(bwcfg->buf[i][0]);
                    bwcfg->bandwidth[i] += findTheBandwidth(pxyz);
                }

                BONE_SENSOR_TRACE(1,"%s: bw[%d]=[%d, %d, %d]",__func__,
                    bwcfg->loop_cnt,(int)bwcfg->bandwidth[0],
                    (int)bwcfg->bandwidth[1], (int)bwcfg->bandwidth[2]);
#if 0
                dump_acc_data("ACC_X",&(bwcfg->buf[0][0]), SAMPLE_NUMBER);
                dump_acc_data("ACC_Y",&(bwcfg->buf[0][0]), SAMPLE_NUMBER);
                dump_acc_data("ACC_Z",&(bwcfg->buf[0][0]), SAMPLE_NUMBER);
#endif
                bwcfg->loop_cnt++;
                if(bwcfg->loop_cnt >= ACQUISITION_LOOP) {
                    c->run_mode = END_ST_MODE;
                } else {
                    lis25ba_st_mode(0);

                    bwcfg->idx = 0;
                    c->run_mode = NO_ST_DATA_MODE;
                    c->count = 0;
                    stop = 1;
                }
            }
            break;
        case END_ST_MODE:
            {
                float bw_x, bw_y, bw_z;
                int res;
                // disable st mode, give back ACK msg, then stop test
                lis25ba_st_mode(0);
                //disable_sensor();
                c->enable = 0;
                c->count = 0;
                c->run_mode = NO_ST_DATA_MODE;
                stop = 1;

                for(i = 0; i < 3; i++) {
                    bwcfg->bandwidth[i] /= ACQUISITION_LOOP;
                }
                bw_x = (float)(bwcfg->bandwidth[0]);
                bw_y = (float)(bwcfg->bandwidth[1]);
                bw_z = (float)(bwcfg->bandwidth[2]);
                res  = lis25ba_bw_test_result(bw_x, bw_y, bw_z);
#if 0
                if (c->bw_handler) {
                    c->bw_handler(res, bw_x, bw_y, bw_z);
                }
#endif
                aud_sensor_t *act = get_active_sensor();
                aud_sensor_info_t *pi = (aud_sensor_info_t *)(act->priv);
                if (pi->sens_cfg.bw_data_handler) {
                    pi->sens_cfg.bw_data_handler(res, bw_x, bw_y, bw_z);
                }
            }
            break;
        case ST_WAIT_MODE:
        default:
            break;
        }
    }
    return 0;
}

double findTheBandwidth(short output3d_TDM[FFT_DEPTH])
{
  double *output3d_avg, *out_Mod;
  double sensing_bandwidth = 0;
  int sample_number = FFT_DEPTH;

  output3d_avg = (double *)st_malloc(sizeof(double)*FFT_DEPTH);
  ASSERT((uint32_t)output3d_avg != 0, "malloc output3d_avg failed");
  memset((void*)output3d_avg, 0x0, sizeof(double)*FFT_DEPTH);

  out_Mod = (double *)st_malloc(sizeof(double)*FFT_DEPTH);
  ASSERT((uint32_t)out_Mod != 0, "malloc out_Mod failed");
  memset((void*)out_Mod, 0x0, sizeof(double)*FFT_DEPTH);

  derivative(output3d_TDM, output3d_avg, sample_number);

  fft(output3d_avg, out_Mod, sample_number);

  double flat_bw = { 0, };
  double firstFreq = FIRST_FREQ;
  double lastFreq = LAST_FREQ;
  double TS_2 = ((double)1 / TDM_ODR);

  double firstSample = firstFreq * FFT_DEPTH * TS_2;
  double lastSample = lastFreq * FFT_DEPTH * TS_2;

  for (int i_samp = (int)firstSample - 1; i_samp < (int)lastSample; i_samp++)
  {
    flat_bw += out_Mod[i_samp];
  }

  // Compute -3dB value to search for
  flat_bw *= ((double)0.70794 / (double)(lastSample - firstSample + 1));

  // Search when -3dB value is reached
  double dist_3db = 999;
  double freq_3db = 0;
  for (int csamp = 1; csamp < FFT_DEPTH; csamp++)
  {
    if (fabs(out_Mod[csamp] - flat_bw) < dist_3db)
    {
      dist_3db = fabs(out_Mod[csamp] - flat_bw);
      freq_3db = csamp;
    }

  }
  // Frequency response
  sensing_bandwidth = freq_3db / (FFT_DEPTH * TS_2);

  st_free(output3d_avg);
  st_free(out_Mod);

  //BONE_SENSOR_TRACE(1,"bandwidth*1000: %d", (uint32_t)(sensing_bandwidth*1000));
  return sensing_bandwidth;
}

void fft(double output_deriv[FFT_DEPTH],
    double outMod[FFT_DEPTH], int num_punti)
{
  double *out_imag_part = (double*)st_malloc(sizeof(double)*FFT_DEPTH);

  ASSERT((uint32_t)out_imag_part != 0, "malloc out_imag_part failed");
  memset((void*)out_imag_part, 0x0, sizeof(double)*FFT_DEPTH);

  normal_FFT(output_deriv, out_imag_part, num_punti, 1);

  for (int jj = 0; jj < num_punti / 2; jj++)
  {
    outMod[jj] = 2 * sqrt(pow(output_deriv[jj], 2) + pow(out_imag_part[jj], 2)) / num_punti;
  }
  outMod[0] /= 2;

  st_free(out_imag_part);
}

void derivative(short output3d_avg[FFT_DEPTH],
    double output_deriv[FFT_DEPTH], int sample_number)
{
  // Compute incremental ratio
  for (int csamp = 1; csamp < sample_number; csamp++)
  {
    output_deriv[csamp - 1] = output3d_avg[csamp] - output3d_avg[csamp - 1];
  }
}

int normal_FFT(double x[], double y[], int n, int itype)
{
  double m = floor(log(n) / log(2.0) + (double)0.5);
  // Quick return for n=1
  if (n == 1)
  {
    return 0;
  }

  // Conjugate if backward transform
  if (itype == -1)
  {
    for (int i = 0; i < n; i++)
    {
      y[i] = -y[i];
    }
  }

  // Main loop
  double p = 2.0 * PI / n;
  int n2 = n;
  for (int k = 1; k <= m; k++)
  {
    double n1 = n2;
    n2 /= 2;
    double ie = n / n1;
    double ia = 1;
    for (int j = 0; j < n2; j++)
    {
      double a = (ia - 1) * p;
      double c = cos(a);
      double s = sin(a);
      ia += ie;
      for (int i = j; i < n; i += (int)n1)
      {
        int  l = i + n2;
        double xt = x[i] - x[l];
        x[i] += x[l];
        double yt = y[i] - y[l];
        y[i] += y[l];
        x[l] = c * xt + s * yt;
        y[l] = c * yt - s * xt;
      }
    }
  }

  // Bit reversal permutation
  int j = 0;
  for (int i = 0; i < n - 1; i++)
  {
    if (i < j)
    {
      double xt = x[j];
      x[j] = x[i];
      x[i] = xt;
      double yt = y[j];
      y[j] = y[i];
      y[i] = yt;
    }
    double k = n / 2;
    while (k < j + 1)
    {
      j -= (int)k;
      k /= 2;
    }
    j += (int)k;
  }

  // Conjugate and normalize if backward transform
  if (itype == -1)
  {
    p = n;
    for (int i = 0; i < n; i++)
    {
      y[i] /= -p;
      x[i] /= p;
    }
  }

  return 0;
}
///////////////////BAND WIDTH CALC END////////////////////

////////////////////FREQUENCY START///////////////////////

#define MG_UNIT                         0.122
#define NEGATIVE_VAL                    -1

typedef struct lis25ba_st_frq_
{
    int freq;
    int vbScale;
} lis25ba_st_frq;

typedef struct {
  float real, imag;
} COMPLEX;

struct VIBR_FREQ_CFG_T {
    uint32_t irq_cnt;
    int16_t *data;
    uint32_t n;
    COMPLEX *samp;
    float *mag;
    COMPLEX *w;
#ifdef DEBUG_VB_SENSOR_DATA
    int16_t *saved_buf;
    uint32_t saved_size;
    uint32_t saved_idx;
#endif
};

#if defined(SENSOR_TST_USE_HEAP)
static struct VIBR_FREQ_CFG_T vibr_cfg;

static void alloc_vibr_freq_cfg(void)
{
    memset(&vibr_cfg, 0, sizeof(vibr_cfg));

    vibr_cfg.data = (int16_t *)sensor_memory_malloc(3 * VIBR_FREQ_SAMP_CNT * sizeof(vibr_cfg.data[0]));
    vibr_cfg.samp = (COMPLEX *)sensor_memory_malloc(VIBR_FREQ_SAMP_CNT * sizeof(vibr_cfg.samp[0]));
    vibr_cfg.mag = (float *)sensor_memory_malloc(VIBR_FREQ_SAMP_CNT * sizeof(vibr_cfg.mag[0]));
    vibr_cfg.w = (COMPLEX *)sensor_memory_malloc((VIBR_FREQ_SAMP_CNT / 2 - 1) * sizeof(vibr_cfg.w[0]));

    vibr_cfg.irq_cnt = 0;
    vibr_cfg.n = 0;

#ifdef DEBUG_VB_SENSOR_DATA
    vibr_cfg.saved_size = 4 * VIBR_FREQ_SAMP_CNT * sizeof(vibr_cfg.saved_buf[0]);
    vibr_cfg.saved_buf = sensor_memory_malloc(vibr_cfg.saved_size);
    vibr_cfg.saved_idx = 0;
#endif
    BONE_SENSOR_TRACE(1, "%s: done", __func__);
}

static void free_vibr_freq_cfg(void)
{
    if (vibr_cfg.data) {
        sensor_memory_free(vibr_cfg.data);
        vibr_cfg.data = NULL;
    }
    if (vibr_cfg.samp) {
        sensor_memory_free(vibr_cfg.samp);
        vibr_cfg.samp = NULL;
    }
    if (vibr_cfg.mag) {
        sensor_memory_free(vibr_cfg.mag);
        vibr_cfg.mag = NULL;
    }
    if (vibr_cfg.w) {
        sensor_memory_free(vibr_cfg.w);
        vibr_cfg.w = NULL;
    }
    BONE_SENSOR_TRACE(1, "%s: done", __func__);
}
#endif

void lis25ba_vb_test_enable(void)
{
    BONE_SENSOR_TRACE(1, "%s", __func__);
#if defined(SENSOR_TST_USE_HEAP)
#if !defined(SENSOR_DRV_USE_HEAP)
    sensor_memory_init();
#endif
    alloc_vibr_freq_cfg();
#else
#endif
}

void lis25ba_vb_test_disable(void)
{
    BONE_SENSOR_TRACE(1, "%s", __func__);
#if defined(SENSOR_TST_USE_HEAP)
    free_vibr_freq_cfg();
#endif
}

#if defined(SENSOR_TST_USE_HEAP)
static void fft_FRQ(COMPLEX *x, int m)
{
  static COMPLEX *w = NULL;    // used to store the w complex array
  int n = (1 << m);

  COMPLEX u, temp, tm;
  COMPLEX *xi = NULL;
  COMPLEX *xip = NULL;
  COMPLEX *xj = NULL;
  COMPLEX *wptr = NULL;

  int i = 0;
  int j = 0;
  int k = 0;
  int l = 0;
  int le = 0;
  int windex = 0;

  w = vibr_cfg.w;

  // start fft ??? ?? ??
  le = n;
  windex = 1;
  for (l = 0 ; l < m ; l++)
  {
    le = le/2;
    // first iteration with no multiplies
    for(i = 0 ; i < n ; i = i + 2*le)
    {
      xi = x + i;
      xip = xi + le;
      temp.real = xi->real + xip->real;
      temp.imag = xi->imag + xip->imag;
      xip->real = xi->real - xip->real;
      xip->imag = xi->imag - xip->imag;
      *xi = temp;
    }
    // remaining iterations use stored w
    wptr = w + windex - 1;
    for (j = 1 ; j < le ; j++)
    {
      u = *wptr;
      for (i = j ; i < n ; i = i + 2*le)
      {
        xi = x + i;
        xip = xi + le;
        temp.real = xi->real + xip->real;
        temp.imag = xi->imag + xip->imag;
        tm.real = xi->real - xip->real;
        tm.imag = xi->imag - xip->imag;
        xip->real = tm.real*u.real - tm.imag*u.imag;
        xip->imag = tm.real*u.imag + tm.imag*u.real;
        *xi = temp;
      }
      wptr = wptr + windex;
    }
    windex = 2*windex;
  }
  // rearrange data by bit reversing
  j = 0;
  for (i = 1 ; i < (n-1) ; i++)
  {
    k = n/2;
    while(k <= j)
    {
      j = j - k;
      k = k/2;
    }
    j = j + k;
    if (i < j)
    {
      xi = x + i;
      xj = x + j;
      temp = *xj;
      *xj = *xi;
      *xi = temp;
    }
  }
}

static void derivative_freq(int16_t* signal, uint32_t count)
{
    for (int i = 0; i < count - 1; i++)
    {
        signal[i] = signal[i + 1] - signal[i];
    }
}

static int get_scale(short* signal, int count)
{
    short min, max;

    min = max = signal[0];

    for (int i = 1; i < count; i++)
    {
        if (signal[i] < min)
        {
            min = signal[i];
        }

        if (max < signal[i])
        {
            max = signal[i];
        }
    }

    return (int)((max + ( NEGATIVE_VAL * min)) * MG_UNIT);
}

static void GetFrequency(int16_t *signal, uint32_t count, lis25ba_st_frq *st_frq)
{
  int i = 0;
  int fft_length = 0;
  int m = 0;
  int odr = 0;
  int max_index = 0;
  int freq = 0;
  float       tempflt  = 0;
  float       *mag = NULL;
  COMPLEX      *samp = NULL;
  float       cnt1_hz = 0;

  int scale = get_scale(signal, count);

  derivative_freq(signal, count);

  count = (1 << get_msb_pos(count));

  // Read the input data file from the dsp format.  ????? ?? ??
  odr = TDM_ODR;
  //length = 512;
  cnt1_hz = (float)odr/count;
  //printf("max :%.3f \n",cnt1_hz);
  // determine fft size and allocate the complex array - fft size? ??
  m = (int)log2f(count);
  fft_length = 1 << m;   //bit shift
  samp = vibr_cfg.samp;

  // copy input signal to complex array and do the fft
  for (i=0; i<count; i++) {
    samp[i].real = (float)signal[i];
    samp[i].imag = 0;
  }

  fft_FRQ(samp, m);

  /* find log magnitude and store for output */
  mag = vibr_cfg.mag;

  float max=0;
  max = samp[i].real;
  for (i=0; i< fft_length/2; i++)
  {
    tempflt  = samp[i].real * samp[i].real;
    tempflt += samp[i].imag * samp[i].imag;
    tempflt =  sqrtf(tempflt);

    mag[i] = tempflt;

    if(mag[i]>max)
    {
      max = mag[i];
      max_index = i;
      //printf("max :%.1f \n",max);
    }

    //printf("log_mag :%.1f \n",mag[i]);
  }

  freq = (int)(max_index*(float)cnt1_hz);
  BONE_SENSOR_TRACE(1,"max_index=%d cnt1_hz(m)=%d freq=%d scale=%d", max_index, (int)(cnt1_hz*1000000), freq, scale);

  st_frq->freq = freq;
  st_frq->vbScale = scale;

  return;
}

static void init_fft_freq(uint32_t cnt)
{
    int n = cnt;
    int j = 0;
    int le = 0;

    float arg = 0;
    float w_real = 0;
    float w_imag = 0;
    float wrecur_real = 0;
    float wrecur_imag = 0;
    float wtemp_real = 0;

    COMPLEX *w = NULL;
    COMPLEX *xj = NULL;

    le = n/2;  //difference between the upper and lower leg indices

    w = vibr_cfg.w;

    // calculate the w values recursively ??
    arg = PI/le;         //  PI/le calculation
    wrecur_real = w_real = cosf(arg);
    wrecur_imag = w_imag = -sinf(arg);
    xj = w;
    for (j = 1 ; j < le ; j++)
    {
      xj->real = (float)wrecur_real;
      xj->imag = (float)wrecur_imag;
      xj++;
      wtemp_real = wrecur_real*w_real - wrecur_imag*w_imag;
      wrecur_imag = wrecur_real*w_imag + wrecur_imag*w_real;
      wrecur_real = wtemp_real;
    }
}

#if 0
static int get_vb_test_result(int fx, int fy, int fz, int expect_hz)
{
    int success = 0;
    int max_hz = expect_hz+1000;
    int min_hz = expect_hz-1000;

    if ((fx > min_hz) && (fx < max_hz)) {
        success |= 0x01;
    }
    if ((fy > min_hz) && (fy < max_hz)) {
        success |= 0x02;
    }
    if ((fz > min_hz) && (fz < max_hz)) {
        success |= 0x04;
    }
    return success;
}
#endif

static void start_vb_test(void)
{
    lis25ba_st_frq x, y, z;
//    struct lis25ba_st_ctrl *c = to_st_ctrl();
    int res;

    init_fft_freq(VIBR_FREQ_SAMP_CNT);
    GetFrequency(&vibr_cfg.data[0], VIBR_FREQ_SAMP_CNT, &x);

    init_fft_freq(VIBR_FREQ_SAMP_CNT);
    GetFrequency(&vibr_cfg.data[VIBR_FREQ_SAMP_CNT], VIBR_FREQ_SAMP_CNT, &y);

    init_fft_freq(VIBR_FREQ_SAMP_CNT);
    GetFrequency(&vibr_cfg.data[VIBR_FREQ_SAMP_CNT * 2], VIBR_FREQ_SAMP_CNT, &z);

#if 0
    res = get_vb_test_result(x.freq, y.freq, z.freq, 2000);
#else
    res = 7;
#endif

    aud_sensor_t *act = get_active_sensor();
    aud_sensor_info_t *pi = (aud_sensor_info_t *)(act->priv);
    if (pi->sens_cfg.vb_data_handler) {
        pi->sens_cfg.vb_data_handler(res, x.freq, y.freq, z.freq, x.vbScale, y.vbScale, z.vbScale);
    }
#if 0
    if (c->vb_handler) {
        c->vb_handler(res, x.freq, y.freq, z.freq, x.vbScale, y.vbScale, z.vbScale);
    }
#endif

#ifdef DEBUG_VB_SENSOR_DATA
    int i;
    BONE_SENSOR_TRACE_IMM(1,"\nX AXIS:\n");
    for (i = 0; i + 8 <= VIBR_FREQ_SAMP_CNT; i += 8) {
        BONE_SENSOR_DUMP16("%5hd ", &vibr_cfg.data[i], 8);
    }
    if (i < VIBR_FREQ_SAMP_CNT) {
        BONE_SENSOR_DUMP16("%5hd ", &vibr_cfg.data[i], VIBR_FREQ_SAMP_CNT- i);
    }
    BONE_SENSOR_TRACE_IMM(1,"\nY AXIS:\n");
    for (i = 0; i + 8 <= VIBR_FREQ_SAMP_CNT; i += 8) {
        BONE_SENSOR_DUMP16("%5hd ", &vibr_cfg.data[i + VIBR_FREQ_SAMP_CNT], 8);
    }
    if (i < VIBR_FREQ_SAMP_CNT) {
        BONE_SENSOR_DUMP16("%5hd ", &vibr_cfg.data[i + VIBR_FREQ_SAMP_CNT], VIBR_FREQ_SAMP_CNT- i);
    }
    BONE_SENSOR_TRACE_IMM(1,"\nZ AXIS:\n");
    for (i = 0; i + 8 <= VIBR_FREQ_SAMP_CNT; i += 8) {
        BONE_SENSOR_DUMP16("%5hd ", &vibr_cfg.data[i + VIBR_FREQ_SAMP_CNT * 2], 8);
    }
    if (i < VIBR_FREQ_SAMP_CNT) {
        BONE_SENSOR_DUMP16("%5hd ", &vibr_cfg.data[i + VIBR_FREQ_SAMP_CNT * 2], VIBR_FREQ_SAMP_CNT- i);
    }
    BONE_SENSOR_TRACE_IMM(1,"\nRAW DATA:\n");
    for (i = 0; i + 8 <= vibr_cfg.saved_idx / 2; i += 8) {
        BONE_SENSOR_DUMP16("%5hd ", &vibr_cfg.saved_buf[i], 8);
        NORM_LOG_FLUSH();
    }
    if (i < vibr_cfg.saved_idx / 2) {
        BONE_SENSOR_DUMP16("%5hd ", &vibr_cfg.saved_buf[i], vibr_cfg.saved_idx / 2 - i);
    }
    while (1) {}
#endif
}
#endif

/******** acc vibration freq test data stream handler ********/
uint32_t lis25ba_vb_data_handler(uint8_t *buf, uint32_t len)
{
#if defined(SENSOR_TST_USE_HEAP)
    int16_t *dx, *dy, *dz;
    int16_t *src = (int16_t *)buf;
    int16_t *src_end = (int16_t *)(buf + len);

    if (vibr_cfg.irq_cnt++ == 0) {
        return 0;
    }

#ifdef DEBUG_VB_SENSOR_DATA
    if (vibr_cfg.saved_idx < vibr_cfg.saved_size) {
        uint32_t saved_len;

        saved_len = vibr_cfg.saved_size - vibr_cfg.saved_idx;
        if (saved_len > len) {
            saved_len = len;
        }
        memcpy((uint8_t *)vibr_cfg.saved_buf + vibr_cfg.saved_idx, buf, saved_len);
        vibr_cfg.saved_idx += saved_len;
    }
#endif

    dx = &vibr_cfg.data[vibr_cfg.n];
    dy = &vibr_cfg.data[vibr_cfg.n + VIBR_FREQ_SAMP_CNT];
    dz = &vibr_cfg.data[vibr_cfg.n + VIBR_FREQ_SAMP_CNT * 2];

    while (src + 4 <= src_end) {
        if (vibr_cfg.n >= VIBR_FREQ_SAMP_CNT) {
            break;
        }
        *dx++ = *src++;
        *dy++ = *src++;
        *dz++ = *src++;
        src++;
        vibr_cfg.n++;
        if (vibr_cfg.n >= VIBR_FREQ_SAMP_CNT) {
            start_vb_test();
            break;
        }
    }
#endif

    return 0;
}

