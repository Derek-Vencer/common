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
#ifndef __SENSOR_DRV_H__
#define __SENSOR_DRV_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "plat_types.h"
#include "hal_trace.h"
#include "sensor_config.h"



#define I2C_TRYCNT_MAX_VAL 3

/* stream config; samplrate for sensor I2S interface */
#if defined(SENSOR_RATE_16K_EN)
#define SENS_AUD_SR_KHZ (16)
#elif defined(SENSOR_RATE_8K_EN)
#define SENS_AUD_SR_KHZ (8)
#else
#error "unsupport sensor sample rate!"
#endif

/* stream config: I2S capture sensor's axis data */
#define SENS_AUD_CAP_CHAN_NUM  (4)
#if defined(LOW_DELAY_SCO)
#define SENS_AUD_CAP_FRM_NUM   (15) // 7.5 ms
#else
#define SENS_AUD_CAP_FRM_NUM   (30) // 15 ms
#endif
#define SENS_AUD_CAP_BUF_SIZE  (SENS_AUD_SR_KHZ*2*SENS_AUD_CAP_CHAN_NUM*SENS_AUD_CAP_FRM_NUM)

/* stream config: DAC playback sensor's axis data */
#define SENS_AUD_PLAY_CHAN_NUM (2)
#define SENS_AUD_PLAY_FRM_NUM  (SENS_AUD_CAP_FRM_NUM)
#define SENS_AUD_PLAY_BUF_SIZE (SENS_AUD_SR_KHZ*2*SENS_AUD_PLAY_CHAN_NUM*SENS_AUD_PLAY_FRM_NUM)

#define SENS_RSPL_CHAN_NUM     (1)
#define SENS_RSPL_FRM_NUM      (4)
#define SENS_RSPL_BUF_SIZE     (512*SENS_RSPL_CHAN_NUM)
#define SENS_RSPL_OUT_BUF_SIZE (SENS_AUD_SR_KHZ*2*SENS_RSPL_CHAN_NUM* \
    SENS_AUD_CAP_FRM_NUM*SENS_RSPL_FRM_NUM/2)

enum AUD_SENSOR_ID {
    AUD_SENSOR_ID_0 = 0,
    AUD_SENSOR_QTY,
};

enum SENSOR_MODE_T {
    SENSOR_MODE_NORMAL,    //normal mode with 16K/8K samprate
    SENSOR_MODE_SPK,       //loop sensor data to speaker
    SENSOR_MODE_DUMP,      //dump sensor data to buffer
    SENSOR_MODE_DUMP_I2S,  //dump sensor data to I2S interface
    SENSOR_MODE_SELF_TEST, //self test mode
    SENSOR_MODE_VB_TEST,   //vibration test mode
    SENSOR_MODE_BW_TEST,   //bandwidth test mode
};

enum SENSOR_SAMPRATE_T {
    SENSOR_SAMPRATE_8K,
    SENSOR_SAMPRATE_16K,
};

enum AUD_SENSOR_CMD_ID {
    AUD_SENSOR_CMD_WHOAMI,     //get chip id
    AUD_SENSOR_CMD_START_RUN,  //enable run()
    AUD_SENSOR_CMD_STOP_RUN,   //disable run()
    AUD_SENSOR_CMD_SLEEP_ON,   //enable sleep process
    AUD_SENSOR_CMD_SLEEP_OFF,  //disable sleep process
    AUD_SENSOR_CMD_SETUP_AXIS, //setup axis data dynamically

    AUD_SENSOR_CMD_QTY,
};

enum SENSOR_AXIS_MAP_T {
    SENSOR_AXIS_MAP_X = (1<<0),
    SENSOR_AXIS_MAP_Y = (1<<1),
    SENSOR_AXIS_MAP_Z = (1<<2),
    SENSOR_AXIS_MAP_ALL   = (SENSOR_AXIS_MAP_X|SENSOR_AXIS_MAP_Y|SENSOR_AXIS_MAP_Z),
};

typedef struct {
    uint32_t probed   :1;
    uint32_t inited   :1;
    uint32_t opened   :1;
    uint32_t sstart   :1;
    uint32_t run      :1; //run=1: (*run(void)) will be invoked normally;
    uint32_t sleep_en :1; //sleep enable/disable
    uint32_t rsv1     :2; //
    uint32_t wakeup   :8;
    uint32_t rsv      :16;
} sensor_state_t;

typedef struct {
    void *priv_data;
} sensor_data_t;

// normal stream data callback(necessary)
typedef uint32_t (*sensor_data_handler_t)(const uint8_t *buf, uint32_t len);

// selftest callback
typedef uint32_t(*sensor_st_data_handler_t)(int16_t x, int16_t y, int16_t z,
    int16_t st_x, int16_t st_y, int16_t st_z);

// vibration test callback
typedef uint32_t (*sensor_vb_data_handler_t)(int result, int freq_x, int freq_y, int freq_z,
    int scale_x, int scale_y, int scale_z);

// bandwidth test callback
typedef uint32_t (*sensor_bw_data_handler_t)(int result, float x, float y, float z);

// sensor stream configuration
typedef struct {
    bool use_default_config; //if true, {mode, rate, axis} will keep default configuration
    enum SENSOR_MODE_T mode; //work mode, if use_default_config=false, mode will be updated
    enum SENSOR_SAMPRATE_T samprate; //samprate,  if use_default_config=false, rate will be updated
    enum SENSOR_AXIS_MAP_T axis_map; //axis data, if use_default_config=false, axis will be updated
    sensor_data_handler_t  data_handler;      //normal stream data handler
    sensor_st_data_handler_t st_data_handler; //for self-test mode
    sensor_vb_data_handler_t vb_data_handler; //for vibration test
    sensor_bw_data_handler_t bw_data_handler; //for band width test
} sensor_stream_cfg_t;

typedef struct {
    enum AUD_SAMPRATE_T samp_rate;
    enum AUD_BITS_T samp_bits;
    enum AUD_CHANNEL_NUM_T chan_num;
    enum AUD_CHANNEL_MAP_T chan_map;
    uint32_t samp_size;
    uint8_t *buf;
    uint32_t buf_size;
    uint32_t frm_num;
} aud_stream_cfg_t;

typedef struct {
    // sensor identify number
    int id;

    // sensor name string;
    const char *name;

    // sensor internal state
    sensor_state_t state;

    // sensor private data
    sensor_data_t *data;

    // detect sensor if it's connect normally or not
    int (*probe)(enum AUD_SENSOR_ID id);

    // initialize or deinitialize sensor after it is probed done
    int (*init)(bool init, enum AUD_SENSOR_ID id);

    // read sensor's regsiter
    int (*read)(uint8_t reg_addr, uint8_t *reg_val);

    // write sensor's register
    int (*write)(uint8_t reg_addr, uint8_t reg_val);

    // open sensor stream with the format
    int (*stream_open)(sensor_stream_cfg_t *cfg);

    // start sensor stream
    int (*stream_start)(void);

    // stop sensor stream
    int (*stream_stop)(void);

    // close sensor stream
    int (*stream_close)(void);

    // control the sensor's internal status
    int (*cntl)(uint8_t cmd, uint32_t *arg);

    // get audio stream config
    int (*get_aud_stream_cfg)(aud_stream_cfg_t **cfg);

    // get sensor stream config
    int (*get_sensor_stream_cfg)(sensor_stream_cfg_t **cfg);

    // The system invoke run() as a thread
    void (*run)(void);

    void *priv;
} aud_sensor_t;

typedef struct {
    bool inited;
    aud_sensor_t *act;
    sensor_stream_cfg_t sens_cfg;
    aud_stream_cfg_t aud_cfg[AUD_STREAM_NUM];
} aud_sensor_info_t;

aud_sensor_t *audio_sensor_init(enum AUD_SENSOR_ID id);

aud_sensor_t *get_active_sensor(void);

#ifdef __cplusplus
}
#endif

#endif
