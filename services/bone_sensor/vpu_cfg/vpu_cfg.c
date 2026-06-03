/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hal_trace.h"
#include "hal_i2c.h"
#include "hal_iomux.h"

// #define _VPU_CFG_DEBUG

#ifndef VPU_I2C_ID
#define VPU_I2C_ID              HAL_I2C_ID_0
#endif

#define VPU_CFG_LOG_W(str, ...)         TR_WARN(TR_MOD(AUD), "[VPU_CFG]" str, ##__VA_ARGS__)
#define VPU_CFG_LOG_I(str, ...)         TR_INFO(TR_MOD(AUD), "[VPU_CFG]" str, ##__VA_ARGS__)

#if defined(_VPU_CFG_DEBUG)
#define VPU_CFG_LOG_D(str, ...)         TR_INFO(TR_MOD(AUD), "[VPU_CFG]" str, ##__VA_ARGS__)
#else
#define VPU_CFG_LOG_D(str, ...)
#endif

#if VPU_NAME == LSM6DSV16BX
#include "lsm6dsv16bx/lsm6dsv16bx_reg.h"
#define VPU_I2C_ADDR            (LSM6DSV16BX_I2C_ADD_H)
extern int32_t lsm6dsv16bx_tdm_init(void);
extern int32_t lsm6dsv16bx_tdm_deinit(void);
#endif

#define I2C_BUF_LEN             (32)
static uint8_t g_i2c_buf[I2C_BUF_LEN+1];
static uint32_t g_i2c_id = VPU_I2C_ID;
static bool g_i2c_opened = false;

int32_t sensor_read_bytes(uint8_t reg, uint8_t *bufp, uint16_t len)
{
    int32_t ret;

    ASSERT(len <= I2C_BUF_LEN, "[%s] len(%d) > I2C_BUF_LEN", __func__, len);

    g_i2c_buf[0] = reg;

    ret = hal_i2c_recv(VPU_I2C_ID,
                       VPU_I2C_ADDR,
                       g_i2c_buf,
                       1,
                       len,
                       HAL_I2C_RESTART_AFTER_WRITE,
                       0,
                       0);

    if (ret) {
        VPU_CFG_LOG_W("[%s] Read failed: %d. reg[%02x]=%02x", __func__, ret, g_i2c_buf[0], g_i2c_buf[1]);
    } else {
        VPU_CFG_LOG_D("[%s] Read ok. reg[%02x]=%02x", __func__, g_i2c_buf[0], g_i2c_buf[1]);

        memcpy(bufp, &g_i2c_buf[1], len);
    }

    return ret;
}

int32_t sensor_write_bytes(uint8_t reg, const uint8_t *bufp, uint16_t len)
{
    uint8_t buf[2];
    int32_t ret;

    ASSERT(len == 1, "[%s] len != %d", __func__, len);

    buf[0] = reg;
    buf[1] = *bufp;
    ret = hal_i2c_send(VPU_I2C_ID, VPU_I2C_ADDR, buf, 1, 1, 0, 0);
    if (ret) {
        VPU_CFG_LOG_W("[%s] Write failed: %d", __func__, ret);
        return ret;
    } else {
        VPU_CFG_LOG_D("[%s] Write ok. reg[%02x]=%02x", __func__, buf[0], buf[1]);
    }
    buf[0] = reg;
    buf[1] = 0x0;

    ret = hal_i2c_recv(VPU_I2C_ID, VPU_I2C_ADDR, buf, 1, 1, HAL_I2C_RESTART_AFTER_WRITE, 0, 0);
    if (ret) {
        VPU_CFG_LOG_W("[%s] Read failed: %d. reg[%02x]=%02x", __func__, ret, buf[0], buf[1]);
    } else {
        VPU_CFG_LOG_D("[%s] Read ok. reg[%02x]=%02x", __func__, buf[0], buf[1]);
    }

    return ret;
}

int32_t vpu_cfg_init(void)
{
    struct HAL_I2C_CONFIG_T i2c_cfg;
    uint32_t ret;

    VPU_CFG_LOG_I("[%s] I2C_ID: %d", __func__, g_i2c_id);

#if defined(VPU_CFG_ON_SENSOR_HUB)
    if (g_i2c_id == HAL_I2C_ID_0) {
        extern void hal_iomux_set_sens_i2c0(void);
        hal_iomux_set_sens_i2c0();
    } else if (g_i2c_id == HAL_I2C_ID_1) {
        extern void hal_iomux_set_sens_i2c1(void);
        hal_iomux_set_sens_i2c1();
    } else if (g_i2c_id == HAL_I2C_ID_2) {
        extern void hal_iomux_set_sens_i2c2(void);
        hal_iomux_set_sens_i2c2();
    } else if (g_i2c_id == HAL_I2C_ID_3) {
        extern void hal_iomux_set_sens_i2c3(void);
        hal_iomux_set_sens_i2c3();
    } else {
        ASSERT(0, "[%s] IIC ID(%d) is invalid!", __func__, g_i2c_id);
    }
#else
    if (g_i2c_id == HAL_I2C_ID_0) {
        hal_iomux_set_i2c0();
    } else if (g_i2c_id == HAL_I2C_ID_1) {
        hal_iomux_set_i2c1();
    } else {
        ASSERT(0, "[%s] IIC ID(%d) is invalid!", __func__, g_i2c_id);
    }
#endif

    memset(&i2c_cfg, 0, sizeof(i2c_cfg));
    i2c_cfg.mode = HAL_I2C_API_MODE_TASK;
    i2c_cfg.use_dma  = 0;
    i2c_cfg.use_sync = 1;
    i2c_cfg.speed = 400000;
    i2c_cfg.as_master = 1;

    ret = hal_i2c_open(g_i2c_id, &i2c_cfg);
    if (ret) {
        VPU_CFG_LOG_W("[%s] I2C(%d) open failed: %d.", __func__, g_i2c_id, ret);
        return 1;
    } else {
        VPU_CFG_LOG_I("[%s] I2C(%d) open ok.", __func__, g_i2c_id);
    }

    g_i2c_opened = true;

    ret = lsm6dsv16bx_tdm_init();
    if (ret) {
        VPU_CFG_LOG_W("[%s] TDM configure error: %d", __func__, ret);
        return 2;
    }

    VPU_CFG_LOG_I("[%s] OK.", __func__);

    return 0;
}

int32_t vpu_cfg_deinit(void)
{
    uint32_t ret;

    ret = lsm6dsv16bx_tdm_deinit();

    if (ret) {
        VPU_CFG_LOG_W("[%s] lsm6dsv16bx_tdm_deinit error: %d", __func__, ret);
        return 1;
    }

    ret = hal_i2c_close(g_i2c_id);
    if (ret) {
        VPU_CFG_LOG_W("[%s] i2s close failed: %d", __func__, ret);
        return 2;
    }

    g_i2c_id = 0;
    g_i2c_opened = false;

    VPU_CFG_LOG_I("[%s] OK.", __func__);

    return 0;
}

