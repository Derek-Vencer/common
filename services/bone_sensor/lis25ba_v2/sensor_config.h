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
#ifndef __SENSOR_CONFIG_H__
#define __SENSOR_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

/* SENSOR AUDIO STREAM ID */
#define SENSOR_AUD_STREAM_ID       (AUD_STREAM_ID_2)

/* SENSOR DEVICE TDM INTERFACE CONFIG */
#define SENSOR_TDM_ID            (AUD_STREAM_USE_TDM0_MASTER)

/* SENSOR DEVICE I2C INTERFACE CONFIG */

/* If defined I2C use task mode otherwise is simple mode */
#define SENSOR_I2C_TASK_MODE
#define SENSOR_I2C_BUS_ID        (HAL_I2C_ID_0)
#define SENSOR_I2C_BUS_SPEED_KHZ (100)
#define SENSOR_I2C_DEV_ADDR      (0x18|1)

/* SENSOR DEVICE DRIVER CONFIG */
#define SENSOR_ID            (AUD_SENSOR_ID_0)
#define SENSOR_NAME_STR      ("LIS25BA")
#define SENSOR_CHIP_ID       (0x20)

/* SENSOR TDM DATA DEFAULT CONFIG */
#define SENSOR_AXIS_X_EN
#define SENSOR_AXIS_Y_EN
#define SENSOR_AXIS_Z_EN

// NOTE: For phone call, must use 16k sample rate Currently
#define SENSOR_RATE_16K_EN
//#define SENSOR_RATE_8K_EN

#if defined(SENSOR_USE_SW_IIC)
#define SENSOR_BUS_IIC_SCL_PIN HAL_IOMUX_PIN_P0_4
#define SENSOR_BUS_IIC_SDA_PIN HAL_IOMUX_PIN_P0_5
#endif

#ifdef __cplusplus
}
#endif

#endif
