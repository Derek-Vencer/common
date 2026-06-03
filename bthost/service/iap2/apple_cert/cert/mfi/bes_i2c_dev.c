
/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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
#include <string.h>
#include <stdlib.h>
#include "hal_trace.h"
#include "hal_i2c.h"
#include "hal_timer.h"
#include "bes_i2c_dev.h"
#include "cmsis_os2.h"
// #include "atime_utils.h"
// #include "airplay_debug.h"
#define I2C_INTERVAL_MS         (2)
#define I2C_MAX_RETRY           (50)
void delayms(uint32_t ms)
{
    hal_sys_timer_delay_us(ms * 1000);
}
static int i2c_init(struct i2c_device *dev)
{
    return hal_i2c_open(dev->port, &dev->cfg);
}
static void i2c_uninit(struct i2c_device *dev)
{
    hal_i2c_close(dev->port);
}
static int i2c_tx(struct i2c_device *dev, const unsigned char reg, const unsigned char *txData, const unsigned int txLength)
{
    int i = 0;
    int ret = 0;
    unsigned int i2cTxLength = txLength + 1;
    unsigned char *i2cTxData = (unsigned char*)malloc(i2cTxLength);
    if(i2cTxData == NULL) {
        TR_INFO(0,"malloc error\n");
        return -1;
    }
    i2cTxData[0] = reg;
    if(txLength) {
        memcpy(i2cTxData + 1, txData, txLength);
    }
    for(i = 0; i < I2C_MAX_RETRY; i++) {
        ret = hal_i2c_simple_send(dev->port, dev->bus_addr, i2cTxData, i2cTxLength);
        if(ret != 0) {
            delayms(I2C_INTERVAL_MS);
            continue;
        } else {
            break;
        }
    }
    free(i2cTxData);
    return ret;
}
static int i2c_rx(struct i2c_device *dev, unsigned char *rxData, const unsigned int rxLength)
{
    int i = 0;
    int ret = 0;
    unsigned char* i2cRxData = rxData;
    unsigned int i2cRxLength = rxLength;
    for(i = 0; i < I2C_MAX_RETRY; i++) {
        ret = hal_i2c_simple_recv(dev->port, dev->bus_addr, NULL, 0, i2cRxData, i2cRxLength);
        if(ret != 0) {
            delayms(I2C_INTERVAL_MS);
            continue;
        } else {
            break;
        }
    }
    return ret;
}
static int i2c_tx_rx(struct i2c_device *dev, const unsigned char reg, const unsigned char *txData, const unsigned int txLength, unsigned char *rxData, const unsigned int rxLength)
{
    int ret = 0;
    ret = i2c_tx(dev, reg, txData, txLength);
    if(ret == 0) {
        delayms(I2C_INTERVAL_MS);
        ret = i2c_rx(dev, rxData, rxLength);
    }
    return ret;
}
int do_i2c(struct i2c_device *dev, const unsigned char reg, const unsigned char *txData, const unsigned int txLength, unsigned char *rxData, unsigned int rxLength)
{
    int ret = 0;
//    unsigned int t = time_utils_now();
    if(dev == NULL) {
        TR_INFO(0,"i2c device is null\n");
        return -1;
    }
    ret = i2c_init(dev);
    if(ret != 0) {
        TR_INFO(0,"I2C don't work !!\n");
        return ret;
    }
    if(rxData != NULL && rxLength > 0) {
        ret = i2c_tx_rx(dev, reg, txData, txLength, rxData, rxLength);
    } else {
        ret = i2c_tx(dev, reg, txData, txLength);
    }
    i2c_uninit(dev);
//    wprt("Reg:%#x  %u ms\n", reg, time_utils_interval_ms(t));
    return ret;
}
int do_i2c_tx(struct i2c_device *dev, const unsigned char *txData, const unsigned int txLength) {
    int i = 0;
    int ret = 0;
    if(dev == NULL || txData == NULL || txLength == 0) {
        TR_INFO(0,"param is error\n");
        return -1;
    }
    ret = i2c_init(dev);
    if(ret != 0) {
        TR_INFO(0,"I2C don't work !!\n");
        return ret;
    }
    for(i = 0; i < I2C_MAX_RETRY; i++) {
        ret = hal_i2c_simple_send(dev->port, dev->bus_addr, txData, txLength);
        if(ret != 0) {
            delayms(I2C_INTERVAL_MS);
            continue;
        } else {
            break;
        }
    }
    i2c_uninit(dev);
    return ret;
}