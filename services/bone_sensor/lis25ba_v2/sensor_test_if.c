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
#include "sensor_drv.h"
#include "sensor_test.h"
#include "sensor_test_if.h"

static sensor_test_result_t sens_res;
static aud_sensor_t *g_sens = NULL;
static enum SENSOR_TC_ID_T sens_tc_id = SENSOR_TC_ID_NONE;
static bool init_test = false;

volatile static uint32_t raw_dummy_cnt = 0;
volatile static bool sens_tst_done = false;

POSSIBLY_UNUSED static void sensor_reply_test_result(void)
{
    int i;
    sensor_test_result_t *p = &sens_res;
    BONE_SENSOR_TRACE(1, "-------- sensor test result --------");
    BONE_SENSOR_TRACE(1, "    ID = %d", p->id);
    BONE_SENSOR_TRACE(1, "    RESULT = %d", p->result);
    for(i = 0; i < p->len; i++) {
        BONE_SENSOR_TRACE(1, "    PARAM[%d] = %d", i, p->param[i]);
    }
    BONE_SENSOR_TRACE(1, "-------- sensor test done  --------");
}

static uint32_t sensor_cap_data_handler(const uint8_t *buf, uint32_t len)
{
    sensor_test_result_t *p = &sens_res;

    int16_t *pd = (int16_t *)buf;
    int16_t x, y, z;

    if (raw_dummy_cnt > 0) {
        raw_dummy_cnt--;
    }
    if (raw_dummy_cnt > 0) {
        return 0;
    }

    x = pd[0];
    y = pd[1];
    z = pd[2];

    memset(p, 0, sizeof(sensor_test_result_t));
    p->id = 4;
    p->result = 7;
    p->param[p->len++] = (uint32_t)x;
    p->param[p->len++] = (uint32_t)y;
    p->param[p->len++] = (uint32_t)z;
    sens_tst_done = true;
    BONE_SENSOR_TRACE(1, "raw_data: [%d, %d, %d]", x, y, z);
    return 0;
}

static uint32_t sensor_self_test_data_handler(int16_t x, int16_t y, int16_t z,
    int16_t st_x, int16_t st_y, int16_t st_z)
{
    sensor_test_result_t *p = &sens_res;

    uint32_t res = 0;
    int32_t abs_x,abs_y,abs_z;
    int32_t fs_x_min_mg = 300;
    int32_t fs_x_max_mg = 950;
    int32_t fs_y_min_mg = 300;
    int32_t fs_y_max_mg = 950;
    int32_t fs_z_min_mg = 500;
    int32_t fs_z_max_mg = 2700;
    float mg = 0.122;

    abs_x = ABS(x-st_x);
    abs_y = ABS(y-st_y);
    abs_z = ABS(z-st_z);

    abs_x = (int)(mg * abs_x);
    abs_y = (int)(mg * abs_y);
    abs_z = (int)(mg * abs_z);

    if ((abs_x >= fs_x_min_mg) && (abs_x <= fs_x_max_mg)) {
        res |= 0x01;
    }
    if ((abs_y >= fs_y_min_mg) && (abs_y <= fs_y_max_mg)) {
        res |= 0x02;
    }
    if ((abs_z >= fs_z_min_mg) && (abs_z <= fs_z_max_mg)) {
        res |= 0x04;
    }

    memset(p, 0, sizeof(sensor_test_result_t));
    p->id = 1;
    p->result = res;
    p->param[p->len++] = (uint32_t)(ABS(x-st_x));
    p->param[p->len++] = (uint32_t)(ABS(y-st_y));
    p->param[p->len++] = (uint32_t)(ABS(z-st_z));
    p->param[p->len++] = (uint32_t)0;
    p->param[p->len++] = (uint32_t)0;
    p->param[p->len++] = (uint32_t)0;
    sens_tst_done = true;
    BONE_SENSOR_TRACE(1, "acc_st: [%d, %d, %d], res=%x", abs_x, abs_y, abs_z, res);
    return 0;
}

static uint32_t sensor_vb_test_data_handler(int result, int freq_x, int freq_y, int freq_z,
    int scale_x, int scale_y, int scale_z)
{
    sensor_test_result_t *p = &sens_res;

    memset(p, 0, sizeof(sensor_test_result_t));
    p->id = 2;
    p->result = result;
    p->param[p->len++] = (uint32_t)freq_x;
    p->param[p->len++] = (uint32_t)freq_y;
    p->param[p->len++] = (uint32_t)freq_z;
    p->param[p->len++] = (uint32_t)scale_x;
    p->param[p->len++] = (uint32_t)scale_y;
    p->param[p->len++] = (uint32_t)scale_z;
    sens_tst_done = true;
    BONE_SENSOR_TRACE(1, "acc_vb: freq=[%d, %d, %d] scale=[%d, %d, %d] res=%x",
        freq_x, freq_y, freq_z, scale_x, scale_y, scale_z, result);
    return 0;
}

static uint32_t sensor_bw_test_data_handler(int result, float bw_x, float bw_y, float bw_z)
{
    sensor_test_result_t *p = &sens_res;
    uint32_t vx, vy, vz, px, py, pz;

    vx = (uint32_t)bw_x;
    vy = (uint32_t)bw_y;
    vz = (uint32_t)bw_z;
    px = (uint32_t)(bw_x*1000-vx*1000);
    py = (uint32_t)(bw_y*1000-vy*1000);
    pz = (uint32_t)(bw_z*1000-vz*1000);

    memset(p, 0, sizeof(sensor_test_result_t));
    p->id = 3;
    p->result = result;
    p->param[p->len++] = vx;
    p->param[p->len++] = px;
    p->param[p->len++] = vy;
    p->param[p->len++] = py;
    p->param[p->len++] = vz;
    p->param[p->len++] = pz;
    sens_tst_done = true;
    BONE_SENSOR_TRACE(1, "acc_bw: [x,y,z]=[%d.%d, %d.%d, %d.%d], res=%x",
        vx,px,vy,py,vz,pz,result);
    return 0;
}

POSSIBLY_UNUSED static int sensor_test_start(aud_sensor_t *s, enum SENSOR_MODE_T mode)
{
    int r = 0;
    bool wait = true;
    sensor_stream_cfg_t cfg;

    BONE_SENSOR_TRACE(1, "%s:mode=%d,case_id=%d",__func__, mode,sens_tc_id);

    memset((void *)&cfg, 0, sizeof(cfg));
    cfg.mode = mode;
    cfg.samprate = SENSOR_SAMPRATE_16K;
    cfg.axis_map = SENSOR_AXIS_MAP_ALL;
    cfg.data_handler    = sensor_cap_data_handler;
    cfg.st_data_handler = sensor_self_test_data_handler;
    cfg.vb_data_handler = sensor_vb_test_data_handler;
    cfg.bw_data_handler = sensor_bw_test_data_handler;

    sens_tst_done = false;

    r = s->init(true, AUD_SENSOR_ID_0);
    ASSERT(r==0, "%s: init failed %d", __func__, r);

    r = s->stream_open(&cfg);
    ASSERT(r==0, "%s: open failed %d", __func__, r);

    if (sens_tc_id == SENSOR_TC_ID_ST) {
        lis25ba_st_init();
        lis25ba_st_enable();
    } else if (sens_tc_id == SENSOR_TC_ID_BW) {
        lis25ba_bw_test_enable();
    } else if (sens_tc_id == SENSOR_TC_ID_VB) {
        lis25ba_vb_test_enable();
    } else if (sens_tc_id == SENSOR_TC_ID_RAW_DATA) {
        raw_dummy_cnt = 10;
    } else {
        wait = false;
    }

    r = s->stream_start();
    ASSERT(r==0, "%s: start failed %d", __func__, r);

    if (wait) {
        BONE_SENSOR_TRACE(1, "%s: waiting sens_tst_done set 1...", __func__);
        while (1) {
            if (sens_tst_done) {
                break;
            }
        }
        BONE_SENSOR_TRACE(1, "%s: sens_tst_done=1", __func__);
    } else {
        BONE_SENSOR_TRACE(1, "%s: not wait sens_tst_done", __func__);
    }

    r = s->stream_stop();
    ASSERT(r==0, "%s: stop failed %d", __func__, r);

    if (sens_tc_id == SENSOR_TC_ID_ST) {
        lis25ba_st_disable();
    } else if (sens_tc_id == SENSOR_TC_ID_BW) {
        lis25ba_bw_test_disable();
    } else if (sens_tc_id == SENSOR_TC_ID_VB) {
        lis25ba_vb_test_disable();
    } else if (sens_tc_id == SENSOR_TC_ID_RAW_DATA) {

    } else {
    }

    r = s->stream_close();
    ASSERT(r==0, "%s: close failed %d", __func__, r);

    //deinit sensor for reducing power consumption
    r = s->init(false, AUD_SENSOR_ID_0);

    BONE_SENSOR_TRACE(1, "%s:done",__func__);
    return r;
}

POSSIBLY_UNUSED static int sensor_vb_start(aud_sensor_t *s)
{
    sens_tc_id = SENSOR_TC_ID_VB;
    return sensor_test_start(s, SENSOR_MODE_VB_TEST);
}

POSSIBLY_UNUSED static int sensor_bw_start(aud_sensor_t *s)
{
    sens_tc_id = SENSOR_TC_ID_BW;
    return sensor_test_start(s, SENSOR_MODE_BW_TEST);
}

POSSIBLY_UNUSED static int sensor_st_start(aud_sensor_t *s)
{
    sens_tc_id = SENSOR_TC_ID_ST;
    return sensor_test_start(s, SENSOR_MODE_SELF_TEST);
}

POSSIBLY_UNUSED static int sensor_raw_data_test(aud_sensor_t *s)
{
    sens_tc_id = SENSOR_TC_ID_RAW_DATA;
    return sensor_test_start(s, SENSOR_MODE_NORMAL);
}

POSSIBLY_UNUSED static int sensor_stream_play_on(bool on, aud_sensor_t *s)
{
    int r = 0;
    sensor_stream_cfg_t cfg;

    BONE_SENSOR_TRACE(1, "%s: on=%d",__func__, on);
    if (on) {
        memset((void *)&cfg, 0, sizeof(cfg));
        cfg.mode = SENSOR_MODE_SPK;
        cfg.samprate = SENSOR_SAMPRATE_16K;
        cfg.axis_map = SENSOR_AXIS_MAP_ALL;
        cfg.data_handler = sensor_cap_data_handler;
        cfg.st_data_handler = sensor_self_test_data_handler;
        cfg.vb_data_handler = sensor_vb_test_data_handler;
        cfg.bw_data_handler = sensor_bw_test_data_handler;

        r = s->stream_open(&cfg);
        ASSERT(r==0, "%s: open failed %d", __func__, r);
        r = s->stream_start();
        ASSERT(r==0, "%s: start failed %d", __func__, r);
    } else {
        r = s->stream_stop();
        ASSERT(r==0, "%s: stop failed %d", __func__, r);
        r = s->stream_close();
        ASSERT(r==0, "%s: close failed %d", __func__, r);
    }
    return r;
}

POSSIBLY_UNUSED static int sensor_i2c_rw_test(aud_sensor_t *s)
{
    int r = 0;
    uint8_t tmp = 0, val = 0, orgval = 0;

    r = s->read(0x2F, &orgval);
    BONE_SENSOR_TRACE(1, "read0: reg[2F] = %x", orgval);

    val = orgval;
    if (val & (1<<7)) {
        val &= ~(1<<7);
    } else {
        val |= (1<<7);
    }
    r = s->write(0x2F, val);
    BONE_SENSOR_TRACE(1, "write1: reg[2F] = %x", val);

    r = s->read(0x2F, &tmp);
    BONE_SENSOR_TRACE(1, "read2: reg[2F] = %x", tmp);

    r = s->write(0x2F, orgval);
    BONE_SENSOR_TRACE(1, "write2: reg[2F] = %x", orgval);

    if (val != tmp) {
        r = -1;
        BONE_SENSOR_TRACE(1, "%s: error val, should be %x", __func__, val);
    }
    return r;
}

int sensor_exec_test(enum SENSOR_TC_ID_T case_id)
{
    int r = 0;

    BONE_SENSOR_TRACE(1, "%s: case_id=%d",__func__,case_id);

    switch (case_id) {
    case SENSOR_TC_ID_ST:         // self test
        BONE_SENSOR_TRACE(1, "SENSOR_TC_ID_ST");
        r = sensor_st_start(g_sens);
        sensor_reply_test_result();
        break;
    case SENSOR_TC_ID_VB:         // vibration test
        BONE_SENSOR_TRACE(1, "SENSOR_TC_ID_VB");
        r = sensor_vb_start(g_sens);
        sensor_reply_test_result();
        break;
    case SENSOR_TC_ID_BW:         // bandwidth test
        BONE_SENSOR_TRACE(1, "SENSOR_TC_ID_BW");
        r = sensor_bw_start(g_sens);
        sensor_reply_test_result();
        break;
    case SENSOR_TC_ID_RAW_DATA:   // read sensor's raw data test
        BONE_SENSOR_TRACE(1, "SENSOR_TC_ID_RAW_DATA");
        r = sensor_raw_data_test(g_sens);
        sensor_reply_test_result();
        break;
    case SENSOR_TC_ID_LOOP_START: // I2S -> DAC loop test start
        BONE_SENSOR_TRACE(1, "SENSOR_TC_ID_LOOP_START");
        r = sensor_stream_play_on(true, g_sens);
        break;
    case SENSOR_TC_ID_LOOP_STOP:  // I2S -> DAC loop test stop
        BONE_SENSOR_TRACE(1, "SENSOR_TC_ID_LOOP_STOP");
        r = sensor_stream_play_on(false, g_sens);
        break;
    case SENSOR_TC_ID_IIC_RW:     // I2C write/read test
        BONE_SENSOR_TRACE(1, "SENSOR_TC_ID_IIC_RW");
        r = sensor_i2c_rw_test(g_sens);
        break;
    default:
        break;
    }
    BONE_SENSOR_TRACE(1, "%s: test done", __func__);
    return r;
}

int sensor_test_enable(void)
{
    int r = 0;
    uint8_t val = 0;
    aud_sensor_t *s = NULL;

    if (!init_test) {
        BONE_SENSOR_TRACE(0,"%s: init", __func__);
        s = audio_sensor_init(AUD_SENSOR_ID_0);
        ASSERT(s != NULL, "%s: null pointer", __func__);

        BONE_SENSOR_TRACE(1, "sens: name=%s, id=%x", s->name, s->id);

        r = s->probe(AUD_SENSOR_ID_0);
        ASSERT(r==0, "%s: proble failed %d", __func__, r);

        r = s->init(true, AUD_SENSOR_ID_0);
        ASSERT(r==0, "%s: init failed %d", __func__, r);

        r = s->read(0xF, &val);
        BONE_SENSOR_TRACE(1, "WHOAMI: reg[0F] = %x", val);

        r = s->read(0x2F, &val);
        BONE_SENSOR_TRACE(1, "read1: reg[2F] = %x", val);

        g_sens = s;
        init_test = true;
        af_close();
        af_open();
    }
    return r;
}

int sensor_test_disable(void)
{
    aud_sensor_t *s = g_sens;

    if(!s) {
        return -1;
    }
    if (init_test) {
        BONE_SENSOR_TRACE(0,"%s: deinit", __func__);
        g_sens = NULL;
        init_test = false;
    }
    return 0;
}

int sensor_test_selftest(sensor_test_result_t *res_info)
{
    int r;
    aud_sensor_t *s = g_sens;

    if(!s) {
        return -1;
    }
    if (!init_test) {
        BONE_SENSOR_TRACE(0,"%s: uninit!", __func__);
        return -2;
    }
    r = sensor_st_start(g_sens);
    if(r == 0) {
        sensor_reply_test_result();
        if (res_info) {
            *res_info = sens_res;
        }
    }
    return r;
}

int sensor_test_bandwidth(sensor_test_result_t *res_info)
{
    int r;
    aud_sensor_t *s = g_sens;

    if(!s) {
        return -1;
    }
    if (!init_test) {
        BONE_SENSOR_TRACE(0,"%s: uninit!", __func__);
        return -2;
    }
    r = sensor_bw_start(g_sens);
    if(r == 0) {
        sensor_reply_test_result();
        if (res_info) {
            *res_info = sens_res;
        }
    }
    return r;
}

int sensor_test_rawdata(sensor_test_result_t *res_info)
{
    int r;
    aud_sensor_t *s = g_sens;

    if(!s) {
        return -1;
    }
    if (!init_test) {
        BONE_SENSOR_TRACE(0,"%s: uninit!", __func__);
        return -2;
    }
    r = sensor_raw_data_test(g_sens);
    if(r == 0) {
        sensor_reply_test_result();
        if (res_info) {
            *res_info = sens_res;
        }
    }
    return r;
}

int sensor_test_vibration(sensor_test_result_t *res_info)
{
    int r;
    aud_sensor_t *s = g_sens;

    if(!s) {
        return -1;
    }
    if (!init_test) {
        BONE_SENSOR_TRACE(0,"%s: uninit!", __func__);
        return -2;
    }
    r = sensor_vb_start(g_sens);
    if(r == 0) {
        sensor_reply_test_result();
        if (res_info) {
            *res_info = sens_res;
        }
    }
    return r;
}

int lis25ba_test(void)
{
    sensor_test_enable();

#if 0
    sensor_test_selftest(NULL);
    sensor_test_bandwidth(NULL);
    sensor_test_rawdata(NULL);
    sensor_test_vibration(NULL);
#endif
//    sensor_exec_test(SENSOR_TC_ID_IIC_RW);
    sensor_exec_test(SENSOR_TC_ID_LOOP_START);

#if 0
    sensor_exec_test(SENSOR_TC_ID_ST);
    hal_sys_timer_delay(MS_TO_TICKS(1000));

    sensor_exec_test(SENSOR_TC_ID_ST);
    hal_sys_timer_delay(MS_TO_TICKS(1000));

    sensor_exec_test(SENSOR_TC_ID_RAW_DATA);
    hal_sys_timer_delay(MS_TO_TICKS(1000));

    sensor_exec_test(SENSOR_TC_ID_RAW_DATA);
    hal_sys_timer_delay(MS_TO_TICKS(1000));
#endif
#if 0
    sensor_exec_test(SENSOR_TC_ID_BW);
    hal_sys_timer_delay(MS_TO_TICKS(1000));

    sensor_exec_test(SENSOR_TC_ID_BW);
    hal_sys_timer_delay(MS_TO_TICKS(1000));
#endif
#if 0
    sensor_exec_test(SENSOR_TC_ID_VB);
    hal_sys_timer_delay(MS_TO_TICKS(1000));

    sensor_exec_test(SENSOR_TC_ID_VB);
    hal_sys_timer_delay(MS_TO_TICKS(1000));
#endif
    while(1);
}
