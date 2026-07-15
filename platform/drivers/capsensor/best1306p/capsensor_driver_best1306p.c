/***************************************************************************
 *
 * Copyright 2024-2024 BES.
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
#ifndef CHIP_SUBSYS_SENS
#include "capsensor_driver_best1306p.h"
#include "analog.h"
#include "pmu.h"
#include "touch_wear_core.h"
#include "cmsis.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "hal_trace.h"
#include "capsensor_spi_best1306p.h"
#include "tgt_hardware_capsensor.h"
#include "capsensor_driver.h"
#ifdef BESUI_CAPSENSOR_FACTORY_EN
#include "capsensor_factory_cal.h"
#endif

void capsensor_driver_init(void)
{
#ifndef CAPSENSOR_READ_DATA_POLLING
    pmu_capsensor_open(CAP_CLK_DIV); //RC
#endif
    analog_capsensor_open();
}

/***************************************************************************
 * @brief capsensor baseline and sar init control function
 *
 ***************************************************************************/
void capsensor_sar_baseline_init(void)
{
}

uint8_t capsensor_ch_num_get(void)
{
    return CAP_CHNUM;
}

uint8_t capsensor_conversion_num_get(void)
{
    return CAP_REPNUM;
}

uint16_t capsensor_samp_fs_get(void)
{
    return CAP_SAMP_FS;
}

uint8_t capsensor_ch_map_get(void)
{
    return (CAP_CH0 | CAP_CH1 | CAP_CH2 | CAP_CH3 | CAP_CH4);
}

void capsensor_gpio_init(void)
{
#ifdef CAP_CHNL_BONDING_WITH_GPIO
    uint8_t cap_ch_map = capsensor_ch_map_get();

    ASSERT(pmu_get_chip_package_type() != PMU_PACKAGE_TYPE_IHC_V1_I2S, "%s: Not support!", __func__);

    if(cap_ch_map & CAP_CH0) {
        hal_iomux_set_io_pull_select(HAL_IOMUX_PIN_P1_0, HAL_IOMUX_PIN_NOPULL);
    }
    if(cap_ch_map & CAP_CH1) {
        hal_iomux_set_io_pull_select(HAL_IOMUX_PIN_P1_1, HAL_IOMUX_PIN_NOPULL);
    }
    if(cap_ch_map & CAP_CH3) {
        hal_iomux_set_io_pull_select(HAL_IOMUX_PIN_P1_3, HAL_IOMUX_PIN_NOPULL);
    }
    if(cap_ch_map & CAP_CH2) {
        hal_iomux_set_analog_io(HAL_IOMUX_PIN_P1_2);
    }
    if(cap_ch_map & CAP_CH4) {
        hal_iomux_set_analog_io(HAL_IOMUX_PIN_P1_4);
    }
#endif
}

/***************************************************************************
 * @brief suspend capsensor functions.
 *
 ***************************************************************************/
void capsensor_suspend(void)
{
    uint32_t lock;

    lock = int_lock();

#ifndef CAPSENSOR_READ_DATA_POLLING
    capsensor_fp_mode_set_mask();
#endif
    analog_capsensor_clk_gate_on();
    int_unlock(lock);
}

/***************************************************************************
 * @brief start capsensor functions.
 *
 ***************************************************************************/
void capsensor_resume(void)
{
    uint32_t lock;

    lock = int_lock();
    analog_capsensor_clk_gate_off();
#ifndef CAPSENSOR_READ_DATA_POLLING
    capsensor_fp_mode_clear_irq();
    capsensor_fp_mode_clr_mask();
#endif
    int_unlock(lock);
}

void capsensor_close(void)
{
    analog_capsensor_reset_set();
}

void capsensor_open(void)
{
    analog_capsensor_reset_clear();
    analog_capsensor_open();
    capsensor_drv_start();
}

void capsensor_sens2mcu_irq_set(void)
{
}

void capsensor_driver_baseline_dr(uint32_t* baseline_value_p, uint32_t* baseline_value_n)
{
    capsensor_baseline_dr(baseline_value_p, baseline_value_n);
}

void capsensor_driver_baseline_reg_read(uint32_t* baseline_value_p, uint32_t* baseline_value_n)
{
    capsensor_baseline_reg_read(baseline_value_p, baseline_value_n);
}

uint16_t capsensor_clk_is_ready(void)
{
    return analog_capsensor_clk_is_ready();
}

#endif
