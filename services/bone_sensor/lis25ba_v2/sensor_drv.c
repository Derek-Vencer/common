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
#include "cmsis.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_i2c.h"
#include "hal_codec.h"
#include "hal_tdm.h"
#include "hal_i2s.h"
#include "hal_iomux.h"
#include "hwtimer_list.h"
#include "audioflinger.h"
#include "sensor_drv.h"
#include "sensor_test.h"
#ifdef BONE_SENSOR_SW_RESAMPLE
#include "sensor_resample.h"
#endif
#if defined(SENSOR_USE_SW_IIC)
#include "twi_master.h"
#endif

//#define SENS_DBG

//#define PLAY_ZERO_DATA_ASSERT

#ifndef PLAY_TDM_SLOT_IDX
#define PLAY_TDM_SLOT_IDX 0
#endif

#ifdef SENSOR_DRV_USE_HEAP
static uint8_t *sens_aud_cap_buff = NULL;
#ifdef BONE_SENSOR_PLAY_TEST
static uint8_t *sens_aud_play_buff = NULL;
#endif /* BONE_SENSOR_PLAY_TEST */
#else
static uint8_t ALIGNED(4) sens_aud_cap_buff[SENS_AUD_CAP_BUF_SIZE];
#ifdef BONE_SENSOR_PLAY_TEST
static uint8_t ALIGNED(4) sens_aud_play_buff[SENS_AUD_PLAY_BUF_SIZE];
#endif /* BONE_SENSOR_PLAY_TEST */
#endif

static uint32_t sens_cap_buff_size  = SENS_AUD_CAP_BUF_SIZE;
#ifdef BONE_SENSOR_PLAY_TEST
static uint32_t sens_play_buff_size = SENS_AUD_PLAY_BUF_SIZE;
static uint32_t sens_wpos = 0;
static uint32_t sens_rpos = 0;
#endif /* BONE_SENSOR_PLAY_TEST */

#ifdef BONE_SENSOR_SW_RESAMPLE
static uint8_t sens_rspl_buff[SENS_RSPL_BUF_SIZE];
static uint8_t sens_rspl_out_buff[SENS_RSPL_OUT_BUF_SIZE];
static uint32_t sens_rspl_buff_size     = SENS_RSPL_BUF_SIZE;
static uint32_t sens_rspl_out_buff_size = SENS_RSPL_OUT_BUF_SIZE;
#endif

static enum SENSOR_MODE_T sens_work_mode;
static sensor_data_t sensors_data[AUD_SENSOR_QTY];
static aud_sensor_t aud_sensors[AUD_SENSOR_QTY];
static aud_sensor_info_t aud_sensor_info;

#define to_sensor(id)       (&aud_sensors[id])
#define to_sensor_info()    (&aud_sensor_info)
#define to_active_sensor()  (aud_sensor_info.act)

static uint8_t gs_bus_id     = HAL_I2C_ID_0;
static uint8_t gs_bus_addr   = 0;

#if defined(SENSOR_USE_SW_IIC)
static const struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux_sw_iic[] =
{
    {SENSOR_BUS_IIC_SCL_PIN, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENALBE},
    {SENSOR_BUS_IIC_SDA_PIN, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENALBE},
};

static void sensor_bus_sw_iic_pin_init(uint8_t id)
{
    hal_iomux_init((struct HAL_IOMUX_PIN_FUNCTION_MAP *)pinmux_sw_iic, ARRAY_SIZE(pinmux_sw_iic));
}

static void sensor_bus_sw_iic_init(uint8_t id, uint8_t addr, int speed_khz)
{
    gs_bus_addr = addr;
    gs_bus_id = id;
    sensor_bus_sw_iic_pin_init(id);

    twi_master_init(1);
}

static int sensor_bus_sw_iic_read(uint8_t reg, uint8_t *val)
{
    bool ret;
    ret = twi_master_read(gs_bus_addr, reg, val);
    if (ret)
        return 0;
    else
        return -1;
}

static int sensor_bus_sw_iic_write(uint8_t reg, uint8_t val)
{
    bool ret;
    ret = twi_master_write(gs_bus_addr, reg, val);
    if (ret)
        return 0;
    else
        return -1;
}

#else
static void sensor_bus_hw_iic_pin_init(uint8_t id)
{
    if (id == HAL_I2C_ID_0) {
        hal_iomux_set_i2c0();
    } else if (id == HAL_I2C_ID_1) {
        hal_iomux_set_i2c1();
    }
}

static void sensor_bus_hw_iic_init(uint8_t id, uint8_t addr, int speed_khz)
{
    struct HAL_I2C_CONFIG_T cfg;
    uint32_t ret;

    gs_bus_addr = addr;
    gs_bus_id   = id;
    sensor_bus_hw_iic_pin_init(id);
#ifdef SENSOR_I2C_TASK_MODE
    cfg.mode = HAL_I2C_API_MODE_TASK;
#else
    cfg.mode = HAL_I2C_API_MODE_SIMPLE;
#endif
    cfg.use_dma = 0;
    cfg.use_sync = 1;
    cfg.speed = speed_khz * 1000;
    cfg.as_master = 1;
    ret = hal_i2c_open(id, &cfg);
    if(ret != 0)
    {
        BONE_SENSOR_TRACE(1,"%s: hal_i2c_open fail id = %d ret = 0x%x", __func__, id, ret);
    }
}

static void sensor_bus_hw_iic_deinit(uint8_t id)
{
    uint32_t ret;

    ret = hal_i2c_close(id);
    if (ret) {
        BONE_SENSOR_TRACE(0," [%s] WARNING: hal_i2c_close fail id = %d ret = 0x%x", __func__, id, ret);
    } else {
        BONE_SENSOR_TRACE(0, "[%s] hal_i2c_close ok.", __func__);
    }
}

static int sensor_bus_hw_iic_read(uint8_t reg, uint8_t *val)
{
    int ret, trycnt;
    uint8_t rxbuf[4];
    uint8_t txbuf[4];

    rxbuf[0] = 0x0;
    txbuf[0] = reg;
    for (trycnt = I2C_TRYCNT_MAX_VAL; trycnt > 0; trycnt--) {
#ifdef SENSOR_I2C_TASK_MODE
        ret = hal_i2c_task_recv(gs_bus_id, gs_bus_addr, txbuf, 1, rxbuf, 1, 0, NULL);
#else
        ret = hal_i2c_simple_recv(gs_bus_id, gs_bus_addr, txbuf, 1, rxbuf, 1);
#endif
        if (ret == 0) {
            break;
        }
        BONE_SENSOR_TRACE(1, "%s:failed,ret=0x%x,reg=0x%x", __func__, ret, reg);
    }
    if (trycnt == 0) {
        // ASSERT(ret==0,"%s: failed 0x%x", __func__, ret);
        BONE_SENSOR_TRACE(0, "[%s] WARNING: Failed 0x%x", __func__, ret);
        return -1;
    }

    *val = rxbuf[0];
    BONE_SENSOR_TRACE(1,"%s: reg=%2x, val=%2x", __func__, reg, *val);
    return 0;
}

int sensor_bus_hw_iic_write(uint8_t reg, uint8_t val)
{
    int ret, trycnt;
    uint8_t txbuf[4];

    txbuf[0] = reg;
    txbuf[1] = val;
    for (trycnt = I2C_TRYCNT_MAX_VAL; trycnt > 0; trycnt--) {
#ifdef SENSOR_I2C_TASK_MODE
        ret = hal_i2c_task_send(gs_bus_id, gs_bus_addr, txbuf, 2, 0, NULL);
#else
        ret = hal_i2c_simple_send(gs_bus_id, gs_bus_addr, txbuf, 2);
#endif
        if (ret == 0) {
            break;
        }
        BONE_SENSOR_TRACE(1, "%s:failed,ret=0x%x,reg=0x%x", __func__, ret, reg);
    }
    if (trycnt == 0) {
        ASSERT(ret==0,"%s: failed 0x%x", __func__, ret);
    }

    BONE_SENSOR_TRACE(1,"%s: [%2x]=%02x", __func__, reg, val);
    return 0;
}
#endif /* #if defined(SENSOR_USE_SW_IIC) */

static void sensor_bus_init(uint8_t id, uint8_t addr, int speed_khz)
{
#if defined(SENSOR_USE_SW_IIC)
    sensor_bus_sw_iic_init(id, addr, speed_khz);
#else
    sensor_bus_hw_iic_init(id, addr, speed_khz);
#endif
}

static void sensor_bus_deinit(uint8_t id)
{
#if defined(SENSOR_USE_SW_IIC)
    BONE_SENSOR_TRACE(0, "[%s] WARNING: Implement this function...", __func__);
#else
    sensor_bus_hw_iic_deinit(id);
#endif
}

int sensor_bus_read(uint8_t reg, uint8_t *val)
{
#if defined(SENSOR_USE_SW_IIC)
    return sensor_bus_sw_iic_read(reg, val);
#else
    return sensor_bus_hw_iic_read(reg, val);
#endif
}

int sensor_bus_write(uint8_t reg, uint8_t val)
{
#if defined(SENSOR_USE_SW_IIC)
    return sensor_bus_sw_iic_write(reg, val);
#else
    return sensor_bus_hw_iic_write(reg, val);
#endif
}

static void sensor_dump_regs(void)
{
    uint8_t val, i;
    uint8_t addr[] = {0x26, 0x2F ,0x2E, 0x24, 0x25};

    for(i = 0; i < ARRAY_SIZE(addr); i++) {
        sensor_bus_read(addr[i], &val);
        BONE_SENSOR_TRACE(1, "reg[%02x]=%02x", addr[i], val);
    }
}

static int sensor_probe(enum AUD_SENSOR_ID id)
{
    aud_sensor_t *s = to_sensor(id);
    int i;
    uint8_t val = 0;

    sensor_bus_init(SENSOR_I2C_BUS_ID, SENSOR_I2C_DEV_ADDR, SENSOR_I2C_BUS_SPEED_KHZ);

    for (i = 0; i < 5; i++) {
        sensor_bus_read(0x0F, &val);
        BONE_SENSOR_TRACE(1,"%s: whoamI=%x", SENSOR_NAME_STR, val);
        if (val == SENSOR_CHIP_ID) {
            break;
        }
    }
    if (i >= 5) {
        BONE_SENSOR_TRACE(1,"%s: error whoamI=%x", __func__, val);
        return -1;
    }
    s->state.probed = 1;

    BONE_SENSOR_TRACE(1,"%s: done", __func__);
    return 0;
}

#ifdef SENSOR_AXIS_X_EN
#define SENSOR_AXIS_X_BIT (1<<5)
#else
#define SENSOR_AXIS_X_BIT (0<<5)
#endif

#ifdef SENSOR_AXIS_Y_EN
#define SENSOR_AXIS_Y_BIT (1<<6)
#else
#define SENSOR_AXIS_Y_BIT (0<<6)
#endif

#ifdef SENSOR_AXIS_Z_EN
#define SENSOR_AXIS_Z_BIT (1<<7)
#else
#define SENSOR_AXIS_Z_BIT (0<<7)
#endif

#define SENSOR_AXIS_CTRL ((SENSOR_AXIS_X_BIT)|(SENSOR_AXIS_Y_BIT)|(SENSOR_AXIS_Z_BIT)|0x1)

#ifdef SENSOR_RATE_16K_EN
#define SENSOR_RATE_CTRL (0x02) //16KHz
#elif defined(SENSOR_RATE_8K_EN)
#define SENSOR_RATE_CTRL (0x00) //8KHz
#else
#define SENSOR_RATE_CTRL (0x02) //16KHz
#endif

static void sensor_reg_set_samprate(enum SENSOR_SAMPRATE_T sr)
{
    uint8_t val;

    BONE_SENSOR_TRACE(1, "%s: sr=%d", __func__, sr);

    sensor_bus_read(0x2E, &val);
    val &= ~(3<<1);
    if (sr == SENSOR_SAMPRATE_8K) {
        val |= 0x0;
    } else if (SENSOR_SAMPRATE_16K) {
        val |= 0x2;
    } else {
        val |= 0x4;
    }
    sensor_bus_write(0x2E, val);
}

static void sensor_reg_set_axis(enum SENSOR_AXIS_MAP_T map)
{
    uint8_t val;

    BONE_SENSOR_TRACE(1, "%s: map=%x", __func__, map);

    sensor_bus_read(0x2F, &val);
    val &= ~(7<<5);
    if (map & SENSOR_AXIS_MAP_X) {
        val |= (1<<5);
    }
    if (map & SENSOR_AXIS_MAP_Y) {
        val |= (1<<6);
    }
    if (map & SENSOR_AXIS_MAP_Z) {
        val |= (1<<7);
    }
    sensor_bus_write(0x2F, val);
}

static int sensor_init(bool init, enum AUD_SENSOR_ID id)
{
    aud_sensor_t *s = to_sensor(id);
    aud_sensor_info_t *pi = to_sensor_info();

    ASSERT(s->state.probed!=0, "%s: not probed", __func__);

    if (init) {
        if (s->state.inited) {
            BONE_SENSOR_TRACE(1,"%s: init done", __func__);
            return 0;
        }
#ifdef SENSOR_DRV_USE_HEAP
        sensor_memory_init();
#endif

        sensor_bus_write(0x26, 0x00);
        sensor_bus_write(0x2E, SENSOR_RATE_CTRL);
        sensor_bus_write(0x2F, SENSOR_AXIS_CTRL);
        sensor_dump_regs();
        s->state.inited = 1;
        pi->act = s;
    } else {
        if (!s->state.inited) {
            BONE_SENSOR_TRACE(1,"%s: deinit done", __func__);
            return 0;
        }
        sensor_bus_write(0x2F, 0x00);
        sensor_bus_write(0x2E, 0x82);
        sensor_bus_write(0x26, 0x20);
        sensor_bus_deinit(SENSOR_I2C_BUS_ID);
        s->state.inited = 0;
        pi->act = NULL;
    }
    BONE_SENSOR_TRACE(1,"%s: init %d done", __func__, init);
    return 0;
}

static int sensor_read(uint8_t reg_addr, uint8_t *reg_val)
{
    int r;
    r = sensor_bus_read(reg_addr, reg_val);
    BONE_SENSOR_TRACE(1, "%s: addr=%x, val=%x", __func__, reg_addr, *reg_val);
    return r;
}

static int sensor_write(uint8_t reg_addr, uint8_t reg_val)
{
    BONE_SENSOR_TRACE(1, "%s: addr=%x, val=%x", __func__, reg_addr, reg_val);
    return sensor_bus_write(reg_addr, reg_val);
}

POSSIBLY_UNUSED static void dump_tdm_data(char *s, uint8_t *buf, uint32_t len)
{
    static uint32_t cnt = 0;

    uint32_t i;
    uint16_t *p16 = (uint16_t *)buf;

    cnt++;
    if ((cnt % 100) != 0) {
        return;
    }
    BONE_SENSOR_TRACE(1, "%s", s);
    for(i = 0; i < 2; i++) {
        BONE_SENSOR_TRACE(1, "%04x %04x %04x %04x %04x %04x %04x %04x",
            p16[0], p16[1], p16[2], p16[3],
            p16[4], p16[5], p16[6], p16[7]);
        p16 += 8;
    }
    BONE_SENSOR_TRACE(1," ");
}

#ifdef BONE_SENSOR_PLAY_TEST
static uint32_t sens_codec_play_data_handler(uint8_t *buf, uint32_t len)
{
    static uint32_t errcnt = 0;
#ifdef SENS_DBG
    uint32_t cur_time = TICKS_TO_MS(hal_sys_timer_get());
#endif
    uint32_t cap_buf_size = sens_cap_buff_size;
    uint8_t *cap_buf = sens_aud_cap_buff;
    uint32_t cap_chnum = SENS_AUD_CAP_CHAN_NUM, ch;
    uint32_t play_chnum = SENS_AUD_PLAY_CHAN_NUM;
    uint32_t sens_rpos_cnt = 0;

#ifdef BONE_SENSOR_SW_RESAMPLE
    sbuffer_t *rspl_out = sensor_sw_resample_get_out_buffer();

    cap_buf      = rspl_out->buf;
    cap_buf_size = rspl_out->size;
    sens_wpos    = rspl_out->wpos;
    sens_rpos    = rspl_out->rpos;
    cap_chnum    = SENS_RSPL_CHAN_NUM;
#endif

    uint32_t avail;
    if (sens_wpos >= sens_rpos) {
        avail = sens_wpos - sens_rpos;
    } else {
        avail = cap_buf_size - sens_rpos + sens_wpos;
    }
    if (avail * play_chnum / cap_chnum >= len) {
        int16_t *src     = (int16_t *)(cap_buf + sens_rpos);
        int16_t *src_end = (int16_t *)(cap_buf + cap_buf_size);
        int16_t *dst     = (int16_t *)(buf);
        int16_t *dst_end = (int16_t *)(buf+len);

        while (dst < dst_end) {
            if (src == src_end) {
                src = (int16_t *)cap_buf;
            }
            for(ch = 0; ch < play_chnum; ch++) {
#ifdef BONE_SENSOR_SW_RESAMPLE
                dst[ch] = src[PLAY_TDM_SLOT_IDX];
#else
                dst[ch] = src[PLAY_TDM_SLOT_IDX];
#endif
            }
            dst += play_chnum;
            src += cap_chnum;
            sens_rpos_cnt += cap_chnum*2;
        }
        sens_rpos += sens_rpos_cnt;
        if (sens_rpos >= cap_buf_size) {
            sens_rpos -= cap_buf_size;
        }
#ifdef BONE_SENSOR_SW_RESAMPLE
        rspl_out->rpos = sens_rpos;
#endif
    } else {
        memset(buf, 0, len);
        BONE_SENSOR_TRACE(1, "CODEC_PLAY: zero data");
        errcnt++;
        BONE_SENSOR_TRACE(0, "%s:error[%d],rpos=%d,wpos=%d,len=%d,avail=%d,play_ch=%d,cap_ch=%d",
            __func__,errcnt,sens_rpos,sens_wpos,len,avail,play_chnum,cap_chnum);
#ifdef PLAY_ZERO_DATA_ASSERT
        if (errcnt > 5) {
            ASSERT(0, "%s: error", __func__);
        }
#endif
    }
#ifdef SENS_DBG
    BONE_SENSOR_TRACE(1,"[%u] DAC_PLAY:buf=%x,len=%d,rpos=%d,wpos=%d,avail=%d",
         cur_time, (int)buf, len, sens_rpos, sens_wpos, avail);
#endif
    return 0;
}

static uint32_t sens_i2s_play_data_handler(uint8_t *buf, uint32_t len)
{
#ifdef SENS_DBG
    BONE_SENSOR_TRACE(1, "I2S_PLAY: buf=%x, len=%d", (int)buf, len);
#endif
    return 0;
}

uint32_t sens_i2s_cap_data_play_handler(uint8_t *buf, uint32_t len)
{
#if !defined(BONE_SENSOR_SW_RESAMPLE)
#ifdef SENS_DBG
    uint32_t cur_time = TICKS_TO_MS(hal_sys_timer_get());
#endif
    uint32_t buf_size = sens_cap_buff_size;

    sens_wpos += len;
    if (sens_wpos >= buf_size) {
        sens_wpos = 0;
    }
#ifdef SENS_DBG
    BONE_SENSOR_TRACE(1,"[%u] I2S_CAP:len=%4u,sens_wpos=%d", cur_time, len, sens_wpos);
#endif
    //dump_tdm_data("I2S_CAP", buf, len);
#endif
    return 0;
}
#endif /* BONE_SENSOR_PLAY_TEST */

static uint32_t sens_i2s_cap_data_handler(uint8_t *buf, uint32_t len)
{
#ifdef BONE_SENSOR_SW_RESAMPLE
    sensor_sw_resample_process(buf, len);
#endif
    switch(sens_work_mode) {
    case SENSOR_MODE_NORMAL:    //normal mode with 16K/8K samprate
        lis25ba_raw_data_handler(buf, len);
        break;
#ifdef BONE_SENSOR_PLAY_TEST
    case SENSOR_MODE_SPK:       //loop sensor data to speaker
        sens_i2s_cap_data_play_handler(buf, len);
        break;
#endif
    case SENSOR_MODE_DUMP:      //dump sensor data to buffer
        lis25ba_dump_data_handler(buf, len, 0);
        break;
    case SENSOR_MODE_DUMP_I2S:  //dump sensor data to I2S interface
        lis25ba_dump_data_handler(buf, len, 1);
        break;
    case SENSOR_MODE_SELF_TEST: //self test mode
        lis25ba_st_data_handler(buf, len);
        break;
    case SENSOR_MODE_VB_TEST:   //vibration test mode
        lis25ba_vb_data_handler(buf, len);
        break;
    case SENSOR_MODE_BW_TEST:   //bandwidth test mode
        lis25ba_bw_data_handler(buf, len);
        break;
    default:
        ASSERT(false, "%s: error sens_work_mode=%d", __func__, sens_work_mode);
        break;
    }
//    BONE_SENSOR_TRACE(1, "%s: buf=%x,len=%d,mode=%d",__func__,(int)buf,len,sens_work_mode);
    return 0;
}

static void mode_to_stream_state(enum SENSOR_MODE_T mode, bool *cap_on, bool *play_on)
{
    if (0) {
#ifdef BONE_SENSOR_PLAY_TEST
    } else if (mode == SENSOR_MODE_SPK) {
        *play_on = true;
        *cap_on = true;
#endif
    } else if (mode == SENSOR_MODE_SELF_TEST) {
        *cap_on = true;
    } else if (mode == SENSOR_MODE_VB_TEST) {
        *cap_on = true;
    } else if (mode == SENSOR_MODE_BW_TEST) {
        *cap_on = true;
    } else if (mode == SENSOR_MODE_NORMAL) {
        *cap_on = true;
    } else if (mode == SENSOR_MODE_DUMP_I2S) {
        *cap_on = true;
    } else {
        ASSERT(0,"%s:unsupport mode %d", __func__, mode);
    }
}

static int sensor_stream_open(sensor_stream_cfg_t *cfg)
{
    aud_sensor_t *s = to_active_sensor();
    aud_sensor_info_t *pi = to_sensor_info();
    bool play_on = false, cap_on = false;
    enum SENSOR_MODE_T mode = cfg->mode;

    ASSERT(s->state.inited!=0, "%s: not inited", __func__);

    BONE_SENSOR_TRACE(1, "%s:mode=%d", __func__, mode);

    if (s->state.opened) {
        BONE_SENSOR_TRACE(1, "%s: already opened", __func__);
        return 0;
    }
    // init sensor device
    memcpy((void*)&pi->sens_cfg, cfg, sizeof(sensor_stream_cfg_t));

    // if use_deafult_config=false, then set sensor registers
    if (!cfg->use_default_config) {
        sensor_reg_set_samprate(cfg->samprate);
        sensor_reg_set_axis(cfg->axis_map);
        sensor_dump_regs();
    }

    // config stream
    mode_to_stream_state(mode, &cap_on, &play_on);
    sens_work_mode = mode;
    s->priv = pi;

    struct AF_STREAM_CONFIG_T stream_cfg;
    aud_stream_cfg_t *c;

    if (play_on) {
#ifdef BONE_SENSOR_PLAY_TEST
        c = &(pi->aud_cfg[AUD_STREAM_PLAYBACK]);
#ifdef SENSOR_DRV_USE_HEAP
        sens_aud_play_buff = sensor_memory_malloc(sens_play_buff_size);
        c->buf      = sens_aud_play_buff;
        c->buf_size = sens_play_buff_size;
#endif
        memset(c->buf, 0, c->buf_size);
        memset(&stream_cfg, 0, sizeof(stream_cfg));

        sens_wpos = 0;
        sens_rpos = 0;

        stream_cfg.sample_rate = c->samp_rate;
        stream_cfg.bits        = c->samp_bits;
        stream_cfg.channel_num = c->chan_num;
        stream_cfg.channel_map = c->chan_map;
        stream_cfg.data_ptr    = c->buf;
        stream_cfg.data_size   = c->buf_size;
        stream_cfg.vol         = 15;
        stream_cfg.io_path     = AUD_OUTPUT_PATH_SPEAKER;
        if (mode == SENSOR_MODE_SPK) {
            stream_cfg.device      = AUD_STREAM_USE_INT_CODEC;
            stream_cfg.handler     = sens_codec_play_data_handler;
            BONE_SENSOR_TRACE(1,"spk play mode");
        } else {
            stream_cfg.device      = AUD_STREAM_USE_TDM1_MASTER;
            stream_cfg.handler     = sens_i2s_play_data_handler;
            BONE_SENSOR_TRACE(1,"not spk play mode");
        }
        af_stream_open(SENSOR_AUD_STREAM_ID, AUD_STREAM_PLAYBACK, &stream_cfg);
        BONE_SENSOR_TRACE(1, "payback stream opened:buf_size=%d,chan=%d,bits=%d,size=%d,frm=%d",
            c->buf_size,c->chan_num,c->samp_bits,c->samp_size,c->frm_num);
#endif /* BONE_SENSOR_PLAY_TEST */
    }
    if (cap_on) {
        c = &(pi->aud_cfg[AUD_STREAM_CAPTURE]);
#ifdef SENSOR_DRV_USE_HEAP
        sens_aud_cap_buff = sensor_memory_malloc(sens_cap_buff_size);
        c->buf      = sens_aud_cap_buff;
        c->buf_size = sens_cap_buff_size;
#endif
        memset(c->buf, 0, c->buf_size);
        memset(&stream_cfg, 0, sizeof(stream_cfg));

        if (c->samp_bits == AUD_BITS_16) {
            stream_cfg.fs_cycles   = HAL_TDM_FS_CYCLES_16;
            stream_cfg.slot_cycles = HAL_TDM_SLOT_CYCLES_16;
        } else {
            ASSERT(0, "%s: invalid samp bits %d", __func__, c->samp_bits);
        }
        stream_cfg.fs_edge     = AUD_FS_FIRST_EDGE_POS;
        stream_cfg.align       = AUD_DATA_ALIGN_LEFT_JUSTIFIED;
        stream_cfg.sync_start  = false;

        stream_cfg.sample_rate = c->samp_rate;
        stream_cfg.bits        = c->samp_bits;
        stream_cfg.channel_num = c->chan_num;
        stream_cfg.channel_map = c->chan_map;
        stream_cfg.data_ptr    = c->buf;
        stream_cfg.data_size   = c->buf_size;
        stream_cfg.vol         = TGT_VOLUME_LEVEL_7;
        stream_cfg.io_path     = AUD_IO_PATH_NULL;
        stream_cfg.device      = SENSOR_TDM_ID;
        stream_cfg.handler     = sens_i2s_cap_data_handler;
        af_stream_open(SENSOR_AUD_STREAM_ID, AUD_STREAM_CAPTURE, &stream_cfg);
        BONE_SENSOR_TRACE(1, "capture stream opened:buf_size=%d,chan=%d,bits=%d,size=%d,frm=%d",
            c->buf_size,c->chan_num,c->samp_bits,c->samp_size,c->frm_num);
#ifdef BONE_SENSOR_SW_RESAMPLE
        sensor_sw_resample_open(c->samp_rate,
            SENS_RSPL_CHAN_NUM,c->samp_bits,
            sens_rspl_buff, sens_rspl_buff_size,
            NULL, 0,
            sens_rspl_out_buff, sens_rspl_out_buff_size);
#endif
    }
    if (cap_on || play_on) {
        s->state.opened = 1;
    }
    return 0;
}

static int sensor_stream_start(void)
{
    int r = 0;
    aud_sensor_t *s = to_active_sensor();
    bool play_on = false, cap_on = false;

    ASSERT(s->state.opened!=0, "%s: not inited", __func__);

    if (s->state.sstart) {
        BONE_SENSOR_TRACE(1, "%s: already started", __func__);
        return 0;
    }
    BONE_SENSOR_TRACE(1, "%s:mode=%d", __func__, sens_work_mode);
    mode_to_stream_state(sens_work_mode, &cap_on, &play_on);

    if (play_on) {
#ifdef BONE_SENSOR_PLAY_TEST
        r = af_stream_start(SENSOR_AUD_STREAM_ID, AUD_STREAM_PLAYBACK);
#endif
    }
    if (cap_on) {
#ifdef BONE_SENSOR_SW_RESAMPLE
        sensor_sw_resample_enable();
#endif
        r = af_stream_start(SENSOR_AUD_STREAM_ID, AUD_STREAM_CAPTURE);
    }
    if (play_on || cap_on) {
        s->state.sstart = 1;
    }
    return r;
}

static int sensor_stream_stop(void)
{
    int r = 0;
    aud_sensor_t *s = to_active_sensor();
    bool play = false, cap = false;

    BONE_SENSOR_TRACE(1, "%s:mode=%d", __func__, sens_work_mode);
    ASSERT(s->state.opened!=0, "%s: not inited", __func__);
    if (!s->state.sstart) {
        BONE_SENSOR_TRACE(1, "%s: already stopped", __func__);
        return 0;
    }
    mode_to_stream_state(sens_work_mode, &cap, &play);

    if (play) {
#ifdef BONE_SENSOR_PLAY_TEST
        r = af_stream_stop(SENSOR_AUD_STREAM_ID, AUD_STREAM_PLAYBACK);
#endif
    }
    if (cap) {
        r = af_stream_stop(SENSOR_AUD_STREAM_ID, AUD_STREAM_CAPTURE);
#ifdef BONE_SENSOR_SW_RESAMPLE
        sensor_sw_resample_disable();
#endif
    }
    if (play || cap) {
        s->state.sstart = 0;
    }
    return r;
}

static int sensor_stream_close(void)
{
    int r = 0;
    aud_sensor_t *s = to_active_sensor();
    bool play = false, cap = false;

    BONE_SENSOR_TRACE(1, "%s:mode=%d", __func__, sens_work_mode);
    ASSERT(s->state.inited!=0, "%s: not inited", __func__);
    if (!s->state.opened) {
        BONE_SENSOR_TRACE(1, "%s: already closed", __func__);
        return 0;
    }
    mode_to_stream_state(sens_work_mode, &cap, &play);

    if (play) {
#ifdef BONE_SENSOR_PLAY_TEST
        r = af_stream_close(SENSOR_AUD_STREAM_ID, AUD_STREAM_PLAYBACK);
#endif
    }
    if (cap) {
        r = af_stream_close(SENSOR_AUD_STREAM_ID, AUD_STREAM_CAPTURE);
#ifdef BONE_SENSOR_SW_RESAMPLE
        sensor_sw_resample_close();
#endif
    }
    if (play || cap) {
        s->state.opened = 0;
    }
    return r;
}

static int sensor_cntl(uint8_t cmd, uint32_t *arg)
{
    int r = 0;
    return r;
}

static void sensor_run(void)
{
#if 0
    aud_sensor_t *s = to_active_sensor();

    if (s->state.run) {
    }
#endif
}

static void audio_sensor_info_init(void)
{
    aud_stream_cfg_t *c;
    aud_sensor_info_t *i = to_sensor_info();

    BONE_SENSOR_TRACE(1, "%s:", __func__);
    if (!i->inited) {
        i->act = NULL;
        i->inited = true;
        memset((&(i->sens_cfg)), 0, sizeof(sensor_stream_cfg_t));

        // init playback stream
        c = &(i->aud_cfg[AUD_STREAM_PLAYBACK]);
        c->samp_rate = AUD_SAMPRATE_16000;
        c->samp_bits = AUD_BITS_16;
        c->samp_size = 2;
        c->chan_num  = SENS_AUD_PLAY_CHAN_NUM;
        c->chan_map  = AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1;
        c->frm_num   = SENS_AUD_PLAY_FRM_NUM;
#ifdef BONE_SENSOR_PLAY_TEST
        c->buf       = sens_aud_play_buff;
        c->buf_size  = sens_play_buff_size;
#endif

        // init capture stream
        c = &(i->aud_cfg[AUD_STREAM_CAPTURE]);
        c->samp_rate = AUD_SAMPRATE_16000;
        c->samp_bits = AUD_BITS_16;
        c->samp_size = 2;
        c->chan_num  = SENS_AUD_CAP_CHAN_NUM;
        c->chan_map  = AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1;
        c->frm_num   = SENS_AUD_CAP_FRM_NUM;
        c->buf       = sens_aud_cap_buff;
        c->buf_size  = sens_cap_buff_size;
    }
}

static int sensor_get_aud_stream_cfg(aud_stream_cfg_t **cfg)
{
    aud_sensor_info_t *info = to_sensor_info();
    aud_stream_cfg_t *c = &(info->aud_cfg[AUD_STREAM_CAPTURE]);

    if (cfg) {
        *cfg = c;
    }
    BONE_SENSOR_TRACE(1, "%s: *cfg=%x", __func__, (uint32_t)c);
    return 0;
}

static int sensor_get_sensor_stream_cfg(sensor_stream_cfg_t **cfg)
{
    aud_sensor_info_t *info = to_sensor_info();
    sensor_stream_cfg_t *c = &(info->sens_cfg);

    if (cfg) {
        *cfg = c;
    }
    BONE_SENSOR_TRACE(1, "%s: *cfg=%x", __func__, (uint32_t)c);
    return 0;
}

aud_sensor_t *audio_sensor_init(enum AUD_SENSOR_ID id)
{
    aud_sensor_t *s = to_sensor(id);

    BONE_SENSOR_TRACE(1, "%s:", __func__);

    audio_sensor_info_init();

    s->id                    = id;
    s->name                  = SENSOR_NAME_STR;
    s->data                  = &sensors_data[id];
    s->probe                 = sensor_probe;
    s->init                  = sensor_init;
    s->read                  = sensor_read;
    s->write                 = sensor_write;
    s->stream_open           = sensor_stream_open;
    s->stream_start          = sensor_stream_start;
    s->stream_stop           = sensor_stream_stop;
    s->stream_close          = sensor_stream_close;
    s->cntl                  = sensor_cntl;
    s->get_aud_stream_cfg    = sensor_get_aud_stream_cfg;
    s->get_sensor_stream_cfg = sensor_get_sensor_stream_cfg;
    s->run                   = sensor_run;

    return s;
}

aud_sensor_t *get_active_sensor(void)
{
    aud_sensor_t *act = to_active_sensor();
    return act;
}

