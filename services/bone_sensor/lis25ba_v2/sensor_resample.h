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
#ifndef __SENSOR_RESAMPLE_H__
#define __SENSOR_RESAMPLE_H__

#ifdef __cplusplus
extern "C" {
#endif

#define SENS_RSPL_SLOT_IDX       (1) //0: axis X; 1: axis Y; 2: axis Z
#define SENS_RSPL_SLOT_NUM       (4)
#define SENS_RSPL_TBUF_SIZE (16*2*1*20)

typedef struct {
    uint8_t *buf;
    uint32_t wpos;
    uint32_t rpos;
    uint32_t size;
} sbuffer_t;

int sensor_sw_resample_open(
    uint32_t dst_samprate,
    uint32_t chan_num, uint32_t bits,
    uint8_t *rspl_buf, uint32_t rspl_buf_size,
    uint8_t *in_buf, uint32_t in_buf_size,
    uint8_t *out_buf, uint32_t out_buf_size);

int sensor_sw_resample_close(void);

void sensor_sw_resample_enable(void);

void sensor_sw_resample_disable(void);

bool sensor_sw_resample_status(void);

uint32_t sensor_sw_resample_process(uint8_t *buf, uint32_t len);

sbuffer_t *sensor_sw_resample_get_out_buffer(void);

#ifdef __cplusplus
}
#endif

#endif
