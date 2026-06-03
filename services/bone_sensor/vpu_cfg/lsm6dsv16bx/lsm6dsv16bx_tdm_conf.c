/*
 ******************************************************************************
 * @file    read_data_polling.c
 * @author  Sensors Software Solution Team
 * @brief   This file shows how to get data from sensor.
 *
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */

/*
 * This example was developed using the following STMicroelectronics
 * evaluation boards:
 *
 * - STEVAL_MKI109V3 + 
 * - NUCLEO_F411RE + 
 * - DISCOVERY_SPC584B + 
 *
 * Used interfaces:
 *
 * STEVAL_MKI109V3    - Host side:   USB (Virtual COM)
 *                    - Sensor side: SPI(Default) / I2C(supported)
 *
 * NUCLEO_STM32F411RE - Host side: UART(COM) to USB bridge
 *                    - Sensor side: I2C(Default) / SPI(supported)
 *
 * DISCOVERY_SPC584B  - Host side: UART(COM) to USB bridge
 *                    - Sensor side: I2C(Default) / SPI(supported)
 *
 * If you need to run this example on a different hardware platform a
 * modification of the functions: `platform_write`, `platform_read`,
 * `tx_com` and 'platform_init' is required.
 *
 */

/* STMicroelectronics evaluation boards definition
 *
 * Please uncomment ONLY the evaluation boards in use.
 * If a different hardware is used please comment all
 * following target board and redefine yours.
 */

//#define STEVAL_MKI109V3  /* little endian */
//#define NUCLEO_F411RE    /* little endian */
//#define SPC584B_DIS      /* big endian */

/* ATTENTION: By default the driver is little endian. If you need switch
 *            to big endian please see "Endianness definitions" in the
 *            header file of the driver (_reg.h).
 */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdio.h>
#include "lsm6dsv16bx_reg.h"
#include "hal_trace.h"

#define _VPU_DRV_DEBUG

#define VPU_DRV_LOG_W(str, ...)         TR_WARN(TR_MOD(AUD), "[VPU_DRV]" str, ##__VA_ARGS__)
#define VPU_DRV_LOG_I(str, ...)         TR_INFO(TR_MOD(AUD), "[VPU_DRV]" str, ##__VA_ARGS__)

#if defined(_VPU_DRV_DEBUG)
#define VPU_DRV_LOG_D(str, ...)         TR_INFO(TR_MOD(AUD), "[VPU_DRV]" str, ##__VA_ARGS__)
#else
#define VPU_DRV_LOG_D(str, ...)
#endif

static int32_t lsm6dsv16bx_dump_regs(void)
{
#if defined(_VPU_DRV_DEBUG)
    uint8_t regs[][2] = {
        {LSM6DSV16BX_TDM_CFG0,  0x00},
        {LSM6DSV16BX_TDM_CFG1,  0x00},
        {LSM6DSV16BX_TDM_CFG2,  0x00},
    };

    lsm6dsv16bx_read_regs(regs, ARRAY_SIZE(regs));

    BONE_SENSOR_TRACE(1, "########## %s ##########", __func__);
    for (uint32_t i=0; i<ARRAY_SIZE(regs); i++) {
        BONE_SENSOR_TRACE(2, "reg[%02x]=%02x", regs[i][0], regs[i][1]);
    }
    BONE_SENSOR_TRACE(0, "########## END ##########");
#endif

    return 0;
}

#if 0
static int32_t lsm6dsv16bx_check_regs(uint8_t regs[][2], uint32_t len)
{
    uint8_t val = 0;

    for (uint32_t i=0; i<ARRAY_SIZE(regs); i++) {
        lsm6dsv16bx_read_reg(NULL, regs[i][0], &val, 1);
        if (val != regs[i][1]) {
            VPU_DRV_LOG_W("[%s] ERROR: reg[%02x]=%02x", __func__, regs[i][0], regs[i][1]);
            return 1;
        }
    }

    return 0;
}
#endif

int32_t lsm6dsv16bx_tdm_init(void)
{
  lsm6dsv16bx_filt_settling_mask_t filt_settling_mask;
  lsm6dsv16bx_xl_axis_t xl_axis;
  lsm6dsv16bx_reset_t rst;
  stmdev_ctx_t dev_ctx;
  uint8_t whoamI;
#if 0
  uint8_t check_regs[][2] = {
    {LSM6DSV16BX_TDM_CFG0,  0x83},
    {LSM6DSV16BX_TDM_CFG1,  0x30},
    {LSM6DSV16BX_TDM_CFG2,  0x09},
  };
#endif

  VPU_DRV_LOG_I("[%s] Start ...", __func__);

  /* Initialize mems driver interface */
  dev_ctx.write_reg = NULL;
  dev_ctx.read_reg = NULL;
  dev_ctx.handle = NULL;

  /* Check device ID */
  lsm6dsv16bx_device_id_get(&dev_ctx, &whoamI);

  if (whoamI != LSM6DSV16BX_ID) {
    VPU_DRV_LOG_W("[%s] Failed: whoamI=%d", __func__, whoamI);
    return 1;
  }

  /* Restore default configuration */
  lsm6dsv16bx_reset_set(&dev_ctx, LSM6DSV16BX_RESTORE_CTRL_REGS);
  do {
    lsm6dsv16bx_reset_get(&dev_ctx, &rst);
  } while (rst != LSM6DSV16BX_READY);

  /* Enable Block Data Update */
  xl_axis.x = PROPERTY_ENABLE;
  xl_axis.y = PROPERTY_ENABLE;
  xl_axis.z = PROPERTY_ENABLE;
  lsm6dsv16bx_xl_axis_set(&dev_ctx, xl_axis);

  /* Enable Block Data Update */
  lsm6dsv16bx_block_data_update_set(&dev_ctx, PROPERTY_ENABLE);
  /* Set Output Data Rate */
  lsm6dsv16bx_xl_data_rate_set(&dev_ctx, LSM6DSV16BX_XL_ODR_AT_3840Hz);
  lsm6dsv16bx_gy_data_rate_set(&dev_ctx, LSM6DSV16BX_GY_ODR_AT_3840Hz);
  /* Set full scale */
  lsm6dsv16bx_xl_full_scale_set(&dev_ctx, LSM6DSV16BX_2g);
  lsm6dsv16bx_gy_full_scale_set(&dev_ctx, LSM6DSV16BX_2000dps);
  /* Configure filtering chain */
  filt_settling_mask.drdy = PROPERTY_ENABLE;
  filt_settling_mask.irq_xl = PROPERTY_ENABLE;
  filt_settling_mask.irq_g = PROPERTY_ENABLE;
  filt_settling_mask.tdm_excep_code = PROPERTY_ENABLE;
  lsm6dsv16bx_filt_settling_mask_set(&dev_ctx, filt_settling_mask);
  lsm6dsv16bx_filt_gy_lp1_set(&dev_ctx, PROPERTY_ENABLE);
  lsm6dsv16bx_filt_gy_lp1_bandwidth_set(&dev_ctx, LSM6DSV16BX_GY_ULTRA_LIGHT);
  lsm6dsv16bx_filt_xl_lp2_set(&dev_ctx, PROPERTY_ENABLE);
  lsm6dsv16bx_filt_xl_lp2_bandwidth_set(&dev_ctx, LSM6DSV16BX_XL_STRONG);

  /* Configure TDM */
  lsm6dsv16bx_xl_mode_set(&dev_ctx, LSM6DSV16BX_XL_HIGH_PERFORMANCE_TDM_MD);
  lsm6dsv16bx_tdm_dis_wclk_pull_up_set(&dev_ctx, PROPERTY_DISABLE);
  lsm6dsv16bx_tdm_tdmout_pull_up_set(&dev_ctx, PROPERTY_DISABLE);
  lsm6dsv16bx_tdm_wclk_bclk_set(&dev_ctx, LSM6DSV16BX_WCLK_16kHZ_BCLK_1024kHz);
  lsm6dsv16bx_tdm_bclk_edge_set(&dev_ctx, LSM6DSV16BX_BCLK_RISING);
  lsm6dsv16bx_tdm_slot_set(&dev_ctx, LSM6DSV16BX_SLOT_012);
  lsm6dsv16bx_tdm_delayed_conf_set(&dev_ctx, PROPERTY_DISABLE);
  lsm6dsv16bx_tdm_axis_order_set(&dev_ctx, LSM6DSV16BX_TDM_ORDER_XYZ);
  lsm6dsv16bx_tdm_xl_full_scale_set(&dev_ctx, LSM6DSV16BX_TDM_4g);

  lsm6dsv16bx_dump_regs();

//   if (lsm6dsv16bx_check_regs(check_regs, ARRAY_SIZE(check_regs))) {
//       return 2;
//   }

  return 0;

}

int32_t lsm6dsv16bx_tdm_deinit(void)
{
    uint8_t whoamI;
    uint8_t regs[][2] = {
        {LSM6DSV16BX_TDM_CFG0,  0x80},
        {LSM6DSV16BX_TDM_CFG1,  0xE0},
        {LSM6DSV16BX_TDM_CFG2,  0x01},
    };

    VPU_DRV_LOG_I("[%s] Start ...", __func__);

    lsm6dsv16bx_device_id_get(NULL, &whoamI);
    if (whoamI != LSM6DSV16BX_ID) {
        VPU_DRV_LOG_W("[%s] Failed: whoamI=%d", __func__, whoamI);
        return 1;
    }

    lsm6dsv16bx_write_regs(regs, ARRAY_SIZE(regs));
    lsm6dsv16bx_xl_mode_set(NULL, LSM6DSV16BX_XL_HIGH_PERFORMANCE_MD);

    lsm6dsv16bx_xl_data_rate_set(NULL, LSM6DSV16BX_XL_ODR_OFF);
    lsm6dsv16bx_gy_data_rate_set(NULL, LSM6DSV16BX_GY_ODR_OFF);

    lsm6dsv16bx_dump_regs();

    return 0;
}
