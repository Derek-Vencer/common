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
#ifndef __SENSOR_TEST_H__
#define __SENSOR_TEST_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "sensor_drv.h"

#define SENSOR_TST_USE_HEAP
//#define SENSOR_DRV_USE_HEAP

#define ST_DATA_BUFFER_CNT      5
#define ST_AXIS_CNT             3
#define ST_DATA_BUFFER_CNT_TOTAL (ST_DATA_BUFFER_CNT * ST_AXIS_CNT)

#define ST_WAIT_CNT 5
#define BW_WAIT_CNT 5

#define VIBR_FREQ_SAMP_CNT      (512 + 1)

enum ACC_ST_STATE {
    NO_ST_DATA_MODE = 0,
    ST_DATA_MODE,
    ST_WAIT_MODE,
    END_ST_MODE,
};

struct lis25ba_st_data {
    int16_t nost[ST_DATA_BUFFER_CNT_TOTAL];
    int16_t st[ST_DATA_BUFFER_CNT_TOTAL];
};

struct lis25ba_st_ctrl {
    uint8_t enable;
    uint8_t ready;
    uint8_t st_enable;
    uint8_t run_mode;
    uint32_t count;
    uint32_t dumcnt;
    struct lis25ba_st_data data;
};

extern struct lis25ba_st_ctrl lis25ba_stc;

#define to_st_ctrl() (&lis25ba_stc)

#define XL_CNT                  3
#define FFT_DEPTH               1024
#define SAMPLE_NUMBER           FFT_DEPTH

#define ACQUISITION_LOOP        1
#define TDM_ODR                 16000

#define SAM_BF_ST               10  //Sample before ST  (NoST count)
#define FIRST_FREQ              800
#define LAST_FREQ               1300

#ifndef TRUE
#define TRUE                    1
#endif

#ifndef FALSE
#define FALSE                   0
#endif

#define M_2PI                   6.28318530717958647692
#define M_SQRT_2                0.707106781186547524401

#define PI                      3.141592
#define MAX(a,b)    (((a) > (b)) ? (a) : (b))

int gsensor_bus_write(uint8_t reg, uint8_t val);

void lis25ba_st_init(void);
void lis25ba_st_enable(void);
void lis25ba_st_disable(void);

void lis25ba_bw_test_enable(void);
void lis25ba_bw_test_disable(void);

void lis25ba_vb_test_enable(void);
void lis25ba_vb_test_disable(void);

uint32_t lis25ba_st_data_handler(uint8_t *buf, uint32_t len);
uint32_t lis25ba_bw_data_handler(uint8_t *buf, uint32_t len);
uint32_t lis25ba_vb_data_handler(uint8_t *buf, uint32_t len);
uint32_t lis25ba_raw_data_handler(uint8_t *buf, uint32_t len);
uint32_t lis25ba_dump_data_handler(uint8_t *buf, uint32_t len, int method);

void sensor_memory_init(void);
uint8_t *sensor_memory_malloc(size_t size);
void sensor_memory_free(void *ptr);

enum SENSOR_TC_ID_T {
    SENSOR_TC_ID_NONE,
    SENSOR_TC_ID_ST,         // self test
    SENSOR_TC_ID_VB,         // vibration test
    SENSOR_TC_ID_BW,         // bandwidth test
    SENSOR_TC_ID_RAW_DATA,   // get sensor raw data test
    SENSOR_TC_ID_LOOP_START, // I2S -> DAC loop test start
    SENSOR_TC_ID_LOOP_STOP,  // I2S -> DAC loop test stop
    SENSOR_TC_ID_IIC_RW,     // I2C write/read test

    SENSOR_TC_QTY,
};

int sensor_exec_test(enum SENSOR_TC_ID_T case_id);

int lis25ba_test(void);

#ifdef __cplusplus
}
#endif

#endif
