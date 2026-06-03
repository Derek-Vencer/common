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
#ifndef __SENSOR_TEST_IF_H__
#define __SENSOR_TEST_IF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "sensor_drv.h"
#include "sensor_test.h"

typedef struct {
    int id;
    uint32_t result;
    uint32_t len;
    uint32_t param[8];
} sensor_test_result_t;

int sensor_test_enable(void);

int sensor_test_disable(void);

int sensor_test_selftest(sensor_test_result_t *res_info);

int sensor_test_bandwidth(sensor_test_result_t *res_info);

int sensor_test_rawdata(sensor_test_result_t *res_info);

int sensor_test_vibration(sensor_test_result_t *res_info);

#ifdef __cplusplus
}
#endif

#endif // __SENSOR_TEST_IF_H__
