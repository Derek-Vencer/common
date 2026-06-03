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
#ifdef CHARGER_1802
#include "tgt_hardware.h"
#include "hal_trace.h"
#include "hal_iomux.h"
#include "hal_i2c.h"
#include CHIP_SPECIFIC_HDR(charger)
#include CHIP_SPECIFIC_HDR(pmu)
#ifdef RTOS
#include "cmsis_os.h"
#else
#include "hal_timer.h"
#endif

extern int pmu_ext_get_efuse(enum PMU_EXT_EFUSE_PAGE_T page, unsigned short *efuse);

#define CHARGER_I2C_SLAVE_ADDRESS    0x1A

enum CHARGER_REG_T {
    CHARGER_REG_01 = 0x01,
    CHARGER_REG_02 = 0x02,
    CHARGER_REG_03 = 0x03,
    CHARGER_REG_04 = 0x04,
    CHARGER_REG_05 = 0x05,
    CHARGER_REG_06 = 0x06,
    CHARGER_REG_07 = 0x07,
    CHARGER_REG_08 = 0x08,
    CHARGER_REG_09 = 0x09,
    CHARGER_REG_0A = 0x0A,
    CHARGER_REG_0B = 0x0B,
    CHARGER_REG_0C = 0x0C,
    CHARGER_REG_0D = 0x0D,
    CHARGER_REG_0E = 0x0E,
    CHARGER_REG_0F = 0x0F,
    CHARGER_REG_10 = 0x10,
    CHARGER_REG_11 = 0x11,
    CHARGER_REG_12 = 0x12,
    CHARGER_REG_13 = 0x13,
    CHARGER_REG_1E = 0x1E,
    CHARGER_REG_1F = 0x1F,
    CHARGER_REG_20 = 0x20,
};

// reg_01
#define CHARGER_PU_OPV_DET                                 (1 << 1)

// reg_02
#define CHARGER_EN_NTC                                     (1 << 5)
#define CHARGER_EN_ISET_RES_EXT                            (1 << 6)
#define CHARGER_PD_ALL                                     (1 << 7)

// reg_04
#define CHARGER_ILIMIT_LEVEL_SEL(n)                        (((n) & 0x7))
#define CHARGER_ILIMIT_LEVEL_SEL_MASK                      (0x7)
#define CHARGER_ILIMIT_LEVEL_SEL_SHIFT                     (0)

// reg_06
#define CHARGER_VREF_RECHARGE_SEL(n)                       (((n) & 0x7) << 4)
#define CHARGER_VREF_RECHARGE_SEL_MASK                     (0x7 << 4)
#define CHARGER_VREF_RECHARGE_SEL_SHIFT                    (4)
#define CHARGER_VSEL_VOREG(n)                              (((n) & 0x3F) << 8)
#define CHARGER_VSEL_VOREG_MASK                            (0x3F << 8)
#define CHARGER_VSEL_VOREG_SHIFT                           (8)

// reg_07
#define CHARGER_ICHARGE_SEL(n)                             (((n) & 0x3F) << 5)
#define CHARGER_ICHARGE_SEL_MASK                           (0x3F << 5)
#define CHARGER_ICHARGE_SEL_SHIFT                          (5)

// reg_08
#define CHARGER_ITERM_SEL(n)                               (((n) & 0x3F))
#define CHARGER_ITERM_SEL_MASK                             (0x3F)
#define CHARGER_ITERM_SEL_SHIFT                            (0)

// reg_09
#define CHARGER_VSEL_PRE2CC(n)                             (((n) & 0x7) << 5)
#define CHARGER_VSEL_PRE2CC_MASK                           (0x7 << 5)
#define CHARGER_VSEL_PRE2CC_SHIFT                          (5)

// reg_0A
#define CHARGER_CHARGING                                   (1 << 0)
#define CHARGER_TEMP_WARM                                  (1 << 3)
#define CHARGER_TEMP_COOL                                  (1 << 4)
#define CHARGER_TEMP_COLD                                  (1 << 5)
#define CHARGER_TIME_OUT                                   (1 << 6)
#define CHARGER_BAT_OVP                                    (1 << 7)
#define CHARGER_TEMP_HOT                                   (1 << 10)
#define CHARGER_TEMP_NORMAL                                (1 << 11)
#define CHARGER_PRE2CC                                     (1 << 12)

// reg_0C
#define CHARGER_EN_VBAT_DIV_ADC                            (1 << 8)

// reg_1E
#define CHARGER_DIG_EN_SHIP_MODE                           (1 << 0)

#define I2C_RETRY_CNT       3
#define I2C_OPEN_DELAY      2//ms

static struct HAL_I2C_CONFIG_T i2c_cfg = {
    .mode = HAL_I2C_API_MODE_TASK,
    .use_dma  = 0,
    .use_sync = 1,
    .speed = 100000,
    .as_master = 1,
    .rising_time_ns = 0,
    .addr_as_slave = 0,
};

static uint32_t i2c_set_of_read(uint8_t device_addr, uint8_t reg, uint16_t *val)
{
    uint32_t ret;
    uint8_t buf[3];

    buf[0] = reg;
    buf[1] = 0;
    buf[2] = 0;

    ret = hal_i2c_open(HAL_I2C_ID_0, &i2c_cfg);
    if (ret) {
        DRIVERS_TRACE(0, "%s:%d, i2c%d open failed, ret = 0x%x", __func__, __LINE__, HAL_I2C_ID_0, ret);
        return ret;
    }

    ret = hal_i2c_recv(HAL_I2C_ID_0, device_addr, buf, 1, 2, HAL_I2C_RESTART_AFTER_WRITE, 0, 0);
    hal_i2c_close(HAL_I2C_ID_0);
    if (ret) {
        DRIVERS_TRACE(0, "%s:%d, i2c%d recv failed, ret = 0x%x", __func__, __LINE__, HAL_I2C_ID_0, ret);
        return ret;
    }

    if (val) {
        *val = buf[1] << 8 | buf[2];
    }

    return ret;
}

static uint32_t i2c_set_of_write(uint8_t device_addr, uint8_t reg, uint16_t val)
{
    uint32_t ret;
    uint8_t buf[3];

    buf[0] = reg;
    buf[1] = (val >> 8) & 0xFF;
    buf[2] = val & 0xFF;

    ret = (int32_t)hal_i2c_open(HAL_I2C_ID_0, &i2c_cfg);
    if (ret) {
        DRIVERS_TRACE(0, "%s:%d, i2c%d open failed, ret = 0x%x", __func__, __LINE__, HAL_I2C_ID_0, ret);
        return ret;
    }

    ret = hal_i2c_send(HAL_I2C_ID_0, device_addr, buf, 1, 2, 0, 0);
    if (ret) {
        DRIVERS_TRACE(0, "%s:%d, i2c%d send failed, ret = 0x%x", __func__, __LINE__, HAL_I2C_ID_0, ret);
    }
    hal_i2c_close(HAL_I2C_ID_0);

    return ret;
}

static int i2cif_reg_read(uint8_t device_addr, uint8_t reg, uint16_t *val)
{
    int32_t ret;
    uint8_t retry = 0;

    do {
        ret = (int32_t)i2c_set_of_read(device_addr, reg, val);
        if (ret) {
            if (ret == HAL_I2C_ERRCODE_IN_USE) {
                osDelay(I2C_OPEN_DELAY);
            }
            retry++;
            DRIVERS_TRACE(0, "%s, i2c%d Reread cnt: %d", __func__, HAL_I2C_ID_0, retry);
            if ((retry >= I2C_RETRY_CNT)||(ret == HAL_I2C_ERRCODE_INV_PARAM)) {
                break;
            }
        }
    } while (ret);

    hal_iomux_clear_charger_i2c0();
    return ret;
}

static int i2cif_reg_write(uint8_t device_addr, uint8_t reg, uint16_t val)
{
    int32_t ret;
    uint8_t retry = 0;

    do {
        ret = (int32_t)i2c_set_of_write(device_addr, reg, val);
        if (ret) {
            if (ret == HAL_I2C_ERRCODE_IN_USE) {
                osDelay(I2C_OPEN_DELAY);
            }
            retry++;
            DRIVERS_TRACE(0, "%s, i2c%d Rewrite cnt: %d", __func__, HAL_I2C_ID_0, retry);
            if ((retry >= I2C_RETRY_CNT)||(ret == HAL_I2C_ERRCODE_INV_PARAM)) {
                break;
            }
        }
    } while (ret);

    hal_iomux_clear_charger_i2c0();
    return ret;
}

int charger_enter_shipping_mode(bool shipping_mode)
{
    uint16_t value = 0;
    int ret = 0;

    ret = i2cif_reg_read(CHARGER_I2C_SLAVE_ADDRESS, CHARGER_REG_1E, &value);
    if (ret)
        goto exit;

    if (shipping_mode)
        value |= CHARGER_DIG_EN_SHIP_MODE;
    else
        value &= ~CHARGER_DIG_EN_SHIP_MODE;

    ret = i2cif_reg_write(CHARGER_I2C_SLAVE_ADDRESS, CHARGER_REG_1E, value);

exit:
    return ret;
}

int charger_status_get(enum CHARGER_STATUS_E *charger_status)
{
    uint16_t value = 0;
    int ret = 0;

    *charger_status = CHARGER_CHARGING_NONE;
    ret = i2cif_reg_read(CHARGER_I2C_SLAVE_ADDRESS, CHARGER_REG_0A, &value);
    if (ret)
        goto exit;

    if ((value & CHARGER_CHARGING) == 0)
        *charger_status = CHARGER_NO_CHARGING;
    else if ((value & CHARGER_PRE2CC) == 0)
        *charger_status = CHARGER_PRE_CHARGING;
    else
        *charger_status = CHARGER_CHARGING_CC;

exit:
    return ret;
}

int charger_full_voltage_set(enum CHARGER_FULL_CHARGER_VOLTAGE_E full_voltage)
{
    uint16_t value = 0;
    int ret = 0;

    ret = i2cif_reg_read(CHARGER_I2C_SLAVE_ADDRESS, CHARGER_REG_06, &value);
    if (ret)
        goto exit;

    value = SET_BITFIELD(value, CHARGER_VSEL_VOREG, full_voltage);
    ret = i2cif_reg_write(CHARGER_I2C_SLAVE_ADDRESS, CHARGER_REG_06, value);
    if (ret)
        goto exit;

exit:
    return ret;
}

int charger_precharging_current_set(enum CHARGER_PRECHARGING_CURRENT_E precharging_current)
{
    uint16_t value = 0;
    int ret = 0;

    ret = i2cif_reg_read(CHARGER_I2C_SLAVE_ADDRESS, CHARGER_REG_0A, &value);
    if (ret)
        goto exit;

    if ((value & CHARGER_PRE2CC) == 0) {
        ret = i2cif_reg_read(CHARGER_I2C_SLAVE_ADDRESS, CHARGER_REG_07, &value);
        if (ret)
            goto exit;

        value = SET_BITFIELD(value, CHARGER_ICHARGE_SEL, precharging_current);
        ret = i2cif_reg_write(CHARGER_I2C_SLAVE_ADDRESS, CHARGER_REG_07, value);
        if (ret)
            goto exit;
    }

exit:
    return ret;
}

int charger_charging_current_set(enum CHARGER_CHARGING_CURRENT_E charging_current)
{
    uint16_t value = 0;
    int ret = 0;

    ret = i2cif_reg_read(CHARGER_I2C_SLAVE_ADDRESS, CHARGER_REG_0A, &value);
    if (ret)
        goto exit;

    if ((value & CHARGER_PRE2CC) == CHARGER_PRE2CC) {
        ret = i2cif_reg_read(CHARGER_I2C_SLAVE_ADDRESS, CHARGER_REG_07, &value);
        if (ret)
            goto exit;

        value = SET_BITFIELD(value, CHARGER_ICHARGE_SEL, charging_current);
        ret = i2cif_reg_write(CHARGER_I2C_SLAVE_ADDRESS, CHARGER_REG_07, value);
        if (ret)
            goto exit;
    }

exit:
    return ret;
}

int charger_vbat_div_adc_enable(bool enable)
{
    uint16_t value = 0;
    int ret = 0;

    ret = i2cif_reg_read(CHARGER_I2C_SLAVE_ADDRESS, CHARGER_REG_0C, &value);
    if (ret)
        goto exit;

    if (enable)
        value |= CHARGER_EN_VBAT_DIV_ADC;
    else
        value &= ~CHARGER_EN_VBAT_DIV_ADC;

    ret = i2cif_reg_write(CHARGER_I2C_SLAVE_ADDRESS, CHARGER_REG_0C, value);
    if (ret)
        goto exit;

exit:
    return ret;
}

int charger_temp_status_get(void)
{
    uint16_t value = 0;
    int ret = CHARGER_BAT_TEMP_NONE;

    ret = i2cif_reg_read(CHARGER_I2C_SLAVE_ADDRESS, CHARGER_REG_0A, &value);
    if (ret)
        goto exit;

    if (value & CHARGER_TEMP_COLD)
        ret = CHARGER_BAT_TEMP_COLD;
    else if (value & CHARGER_TEMP_COOL)
        ret = CHARGER_BAT_TEMP_COOL;
    else if (value & CHARGER_TEMP_NORMAL)
        ret = CHARGER_BAT_TEMP_NORMAL;
    else if (value & CHARGER_BAT_TEMP_WARM)
        ret = CHARGER_TEMP_WARM;
    else if (value & CHARGER_TEMP_HOT)
        ret = CHARGER_BAT_TEMP_HOT;
    else
        ret = CHARGER_BAT_TEMP_NONE;

exit:
    return ret;
}

int charger_enable_charging(void)
{
    uint16_t value = 0;
    int ret = 0;

    ret = i2cif_reg_read(CHARGER_I2C_SLAVE_ADDRESS, CHARGER_REG_01, &value);
    if (ret)
        goto exit;

    value |= CHARGER_PU_OPV_DET;
    ret = i2cif_reg_write(CHARGER_I2C_SLAVE_ADDRESS, CHARGER_REG_01, value);
    if (ret)
        goto exit;

    ret = i2cif_reg_read(CHARGER_I2C_SLAVE_ADDRESS, CHARGER_REG_02, &value);
    if (ret)
        goto exit;

    value &= ~CHARGER_PD_ALL;
    ret = i2cif_reg_write(CHARGER_I2C_SLAVE_ADDRESS, CHARGER_REG_02, value);
    if (ret)
        goto exit;

exit:
    return ret;
}

int charger_disable_charging(void)
{
    uint16_t value = 0;
    int ret = 0;

    ret = i2cif_reg_read(CHARGER_I2C_SLAVE_ADDRESS, CHARGER_REG_01, &value);
    if (ret)
        goto exit;

    value &= ~CHARGER_PU_OPV_DET;
    ret = i2cif_reg_write(CHARGER_I2C_SLAVE_ADDRESS, CHARGER_REG_01, value);
    if (ret)
        goto exit;

    ret = i2cif_reg_read(CHARGER_I2C_SLAVE_ADDRESS, CHARGER_REG_02, &value);
    if (ret)
        goto exit;

    value |= CHARGER_PD_ALL;
    ret = i2cif_reg_write(CHARGER_I2C_SLAVE_ADDRESS, CHARGER_REG_02, value);
    if (ret)
        goto exit;

exit:
    return ret;
}

bool charger_charging_timeout(void)
{
    uint16_t value = 0;
    bool ret = false;

    ret = i2cif_reg_read(CHARGER_I2C_SLAVE_ADDRESS, CHARGER_REG_0A, &value);
    if (ret)
        goto exit;

    if ((value & CHARGER_TIME_OUT) == CHARGER_TIME_OUT)
        ret = true;

exit:
    return ret;
}

int charger_recharger_voltage_set(enum CHARGER_RECHARGER_VOLTAGE_E recharger_voltage)
{
    uint16_t value = 0;
    int ret = 0;

    ret = i2cif_reg_read(CHARGER_I2C_SLAVE_ADDRESS, CHARGER_REG_06, &value);
    if (ret)
        goto exit;

    value = SET_BITFIELD(value, CHARGER_VREF_RECHARGE_SEL, recharger_voltage);
    ret = i2cif_reg_write(CHARGER_I2C_SLAVE_ADDRESS, CHARGER_REG_06, value);
    if (ret)
        goto exit;

exit:
    return ret;
}

int charger_charging_termination_current_set(enum CHARGER_CHARGING_TERMINATION_CURRENT_E charging_terminaton_current)
{
    uint16_t value = 0;
    int ret = 0;

    ret = i2cif_reg_read(CHARGER_I2C_SLAVE_ADDRESS, CHARGER_REG_08, &value);
    if (ret)
        goto exit;

    value = SET_BITFIELD(value, CHARGER_ITERM_SEL, charging_terminaton_current);
    ret = i2cif_reg_write(CHARGER_I2C_SLAVE_ADDRESS, CHARGER_REG_08, value);
    if (ret)
        goto exit;

exit:
    return ret;
}

int charger_enable_NTC(void)
{
    uint16_t value = 0;
    int ret = 0;

    ret = i2cif_reg_read(CHARGER_I2C_SLAVE_ADDRESS, CHARGER_REG_02, &value);
    if (ret)
        goto exit;

    value |= CHARGER_EN_NTC;
    ret = i2cif_reg_write(CHARGER_I2C_SLAVE_ADDRESS, CHARGER_REG_02, value);
    if (ret)
        goto exit;

exit:
    return ret;
}

int charger_disable_NTC(void)
{
    uint16_t value = 0;
    int ret = 0;

    ret = i2cif_reg_read(CHARGER_I2C_SLAVE_ADDRESS, CHARGER_REG_02, &value);
    if (ret)
        goto exit;

    value &= ~CHARGER_EN_NTC;
    ret = i2cif_reg_write(CHARGER_I2C_SLAVE_ADDRESS, CHARGER_REG_02, value);
    if (ret)
        goto exit;

exit:
    return ret;
}

int charger_ilimit_current_set(enum CHARGER_ILIMIT_CURRENT_LEVEL_E ilimit_current)
{
    uint16_t value = 0;
    int ret = 0;

    ret = i2cif_reg_read(CHARGER_I2C_SLAVE_ADDRESS, CHARGER_REG_04, &value);
    if (ret)
        goto exit;

    value = SET_BITFIELD(value, CHARGER_ILIMIT_LEVEL_SEL, ilimit_current);
    ret = i2cif_reg_write(CHARGER_I2C_SLAVE_ADDRESS, CHARGER_REG_04, value);
    if (ret)
        goto exit;

exit:
    return ret;
}

int charger_ovp_get(void)
{
    uint16_t value = 0;
    int ret = 0;

    ret = i2cif_reg_read(CHARGER_I2C_SLAVE_ADDRESS, CHARGER_REG_0A, &value);
    if (ret)
        goto exit;

    if ((value & CHARGER_BAT_OVP) == CHARGER_BAT_OVP)
        ret = 1;
    else
        ret = 0;

exit:
    return ret;
}

int charger_precharging_voltage_set(enum CHARGER_VSEL_PRE2CC_VOLTAGE_E precharging_voltage)
{
    uint16_t value = 0;
    int ret = 0;

    ret = i2cif_reg_read(CHARGER_I2C_SLAVE_ADDRESS, CHARGER_REG_09, &value);
    if (ret)
        goto exit;

    value = SET_BITFIELD(value, CHARGER_VSEL_PRE2CC, precharging_voltage);
    ret = i2cif_reg_write(CHARGER_I2C_SLAVE_ADDRESS, CHARGER_REG_09, value);
    if (ret)
        goto exit;

exit:
    return ret;
}

int charger_full_voltage_get(enum CHARGER_FULL_CHARGER_VOLTAGE_E *full_voltage)
{
    uint16_t value = 0;
    int ret = 0;

    *full_voltage = CHARGER_FULL_CHARGER_VOLTAGE_QTY;
    ret = i2cif_reg_read(CHARGER_I2C_SLAVE_ADDRESS, CHARGER_REG_06, &value);
    if (ret) {
        goto exit;
    }
    *full_voltage = (enum CHARGER_FULL_CHARGER_VOLTAGE_E)GET_BITFIELD(value, CHARGER_VSEL_VOREG);

exit:
    return ret;
}

int charger_precharging_current_get(enum CHARGER_PRECHARGING_CURRENT_E *precharging_current)
{
    uint16_t value = 0;
    int ret = 0;

    *precharging_current = CHARGER_PRECHARGING_CURRENT_QTY;
    ret = i2cif_reg_read(CHARGER_I2C_SLAVE_ADDRESS, CHARGER_REG_0A, &value);
    if (ret)
        goto exit;

    if ((value & CHARGER_PRE2CC) == 0) {
        ret = i2cif_reg_read(CHARGER_I2C_SLAVE_ADDRESS, CHARGER_REG_07, &value);
        if (ret)
            goto exit;

        *precharging_current = (enum CHARGER_PRECHARGING_CURRENT_E)GET_BITFIELD(value, CHARGER_ICHARGE_SEL);
    }
exit:
    return ret;
}

int charger_charging_current_get(enum CHARGER_CHARGING_CURRENT_E *charging_current)
{
    uint16_t value = 0;
    int ret = 0;

    *charging_current = CHARGER_CHARGING_CURRENT_QTY;
    ret = i2cif_reg_read(CHARGER_I2C_SLAVE_ADDRESS, CHARGER_REG_0A, &value);
    if (ret)
        goto exit;

    if ((value & CHARGER_PRE2CC) == CHARGER_PRE2CC) {
        ret = i2cif_reg_read(CHARGER_I2C_SLAVE_ADDRESS, CHARGER_REG_07, &value);
        if (ret) {
            goto exit;
        }
        *charging_current = (enum CHARGER_CHARGING_CURRENT_E)GET_BITFIELD(value, CHARGER_ICHARGE_SEL);
    }

exit:
    return ret;
}

int charger_recharger_voltage_get(enum CHARGER_RECHARGER_VOLTAGE_E *recharger_voltage)
{
    uint16_t value = 0;
    int ret = 0;

    *recharger_voltage = CHARGER_RECHARGER_VOLTAGE_DROP_QTY;
    ret = i2cif_reg_read(CHARGER_I2C_SLAVE_ADDRESS, CHARGER_REG_06, &value);
    if (ret) {
        goto exit;
    }
    *recharger_voltage = (enum CHARGER_RECHARGER_VOLTAGE_E)GET_BITFIELD(value, CHARGER_VREF_RECHARGE_SEL);

exit:
    return ret;
}

int charger_charging_termination_current_get(enum CHARGER_CHARGING_TERMINATION_CURRENT_E *charging_terminaton_current)
{
    uint16_t value = 0;
    int ret = 0;

    *charging_terminaton_current = CHARGER_CHARGING_TERMINATION_CURRENT_QTY;
    ret = i2cif_reg_read(CHARGER_I2C_SLAVE_ADDRESS, CHARGER_REG_08, &value);
    if (ret) {
        goto exit;
    }
    *charging_terminaton_current = (enum CHARGER_CHARGING_TERMINATION_CURRENT_E)GET_BITFIELD(value, CHARGER_ITERM_SEL);

exit:
    return ret;
}

int charger_ilimit_current_get(enum CHARGER_ILIMIT_CURRENT_LEVEL_E *ilimit_current)
{
    uint16_t value = 0;
    int ret = 0;

    *ilimit_current = CHARGER_ILIMIT_CURRENT_LEVEL_QTY;
    ret = i2cif_reg_read(CHARGER_I2C_SLAVE_ADDRESS, CHARGER_REG_04, &value);
    if (ret) {
        goto exit;
    }
    *ilimit_current = (enum CHARGER_ILIMIT_CURRENT_LEVEL_E)GET_BITFIELD(value, CHARGER_ILIMIT_LEVEL_SEL);

exit:
    return ret;
}

int charger_precharging_voltage_get(enum CHARGER_VSEL_PRE2CC_VOLTAGE_E *precharging_voltage)
{
    uint16_t value = 0;
    int ret = 0;

    *precharging_voltage = CHARGER_VSEL_PRE2CC_VOLTAGE_QTY;
    ret = i2cif_reg_read(CHARGER_I2C_SLAVE_ADDRESS, CHARGER_REG_09, &value);
    if (ret) {
        goto exit;
    }
    *precharging_voltage = (enum CHARGER_VSEL_PRE2CC_VOLTAGE_E)GET_BITFIELD(value, CHARGER_VSEL_PRE2CC);

exit:
    return ret;
}

int charger_enable_charger_vbat(void)
{
    uint16_t value = 0;
    int ret = 0;

    ret = i2cif_reg_read(CHARGER_I2C_SLAVE_ADDRESS, CHARGER_REG_02, &value);
    if (ret)
        goto exit;

    value &= ~CHARGER_PD_ALL;
    ret = i2cif_reg_write(CHARGER_I2C_SLAVE_ADDRESS, CHARGER_REG_02, value);
    if (ret)
        goto exit;

exit:
    return ret;
}

int charger_disable_charger_vbat(void)
{
    uint16_t value = 0;
    int ret = 0;

    ret = i2cif_reg_read(CHARGER_I2C_SLAVE_ADDRESS, CHARGER_REG_02, &value);
    if (ret)
        goto exit;

    value |= CHARGER_PD_ALL;
    ret = i2cif_reg_write(CHARGER_I2C_SLAVE_ADDRESS, CHARGER_REG_02, value);
    if (ret)
        goto exit;

exit:
    return ret;
}

int charger_enable_charger_vbus(void)
{
    uint16_t value = 0;
    int ret = 0;

    ret = i2cif_reg_read(CHARGER_I2C_SLAVE_ADDRESS, CHARGER_REG_02, &value);
    if (ret)
        goto exit;

    value &= ~CHARGER_EN_ISET_RES_EXT;
    ret = i2cif_reg_write(CHARGER_I2C_SLAVE_ADDRESS, CHARGER_REG_02, value);
    if (ret)
        goto exit;

exit:
    return ret;
}

int charger_disable_charger_vbus(void)
{
    uint16_t value = 0;
    int ret = 0;

    ret = i2cif_reg_read(CHARGER_I2C_SLAVE_ADDRESS, CHARGER_REG_02, &value);
    if (ret)
        goto exit;

    value |= CHARGER_EN_ISET_RES_EXT;
    ret = i2cif_reg_write(CHARGER_I2C_SLAVE_ADDRESS, CHARGER_REG_02, value);
    if (ret)
        goto exit;

exit:
    return ret;
}

//charging full when gpio71 high
int charger_is_charging_full(void)
{
    static int init = 0;

    DRIVERS_TRACE(0, "%s", __func__);
    if (init == 0) {
        if (app_battery_ext_charger_detecter_cfg.pin != HAL_IOMUX_PIN_NUM) {
            hal_iomux_init((struct HAL_IOMUX_PIN_FUNCTION_MAP *)&app_battery_ext_charger_detecter_cfg, 1);
            hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)app_battery_ext_charger_detecter_cfg.pin, HAL_GPIO_DIR_IN, 0);
        }
        init = 1;
    }

    return (hal_gpio_pin_get_val((enum HAL_GPIO_PIN_T)app_battery_ext_charger_detecter_cfg.pin));
}

void charger_full_voltage_calib_val_get(uint16_t *vol_4p2, uint16_t *vol_4p35, uint16_t *vol_4p4)
{
    uint16_t efuse_d_v, efuse_f_v = 0;
    uint16_t high_bits, lowbits   = 0;

    pmu_ext_get_efuse(PMU_EXT_EFUSE_PAGE_RESERVED_12, &lowbits);
    pmu_ext_get_efuse(PMU_EXT_EFUSE_PAGE_RESERVED_13, &high_bits);
    //DRIVERS_TRACE(2, "lowbits=0x%x, high_bits=0x%x", lowbits, high_bits);
    efuse_d_v = ((high_bits & 0xFF) << 8) | ((lowbits & 0xFF00) >> 8);
    DRIVERS_TRACE(1, "efuse D:0x%x", efuse_d_v);
    *vol_4p2 = efuse_d_v & 0x3F;             //Bit[5:0]: Voreg4.2V cal

    pmu_ext_get_efuse(PMU_EXT_EFUSE_PAGE_RESERVED_14, &lowbits);
    pmu_ext_get_efuse(PMU_EXT_EFUSE_PAGE_RESERVED_15, &high_bits);
    //DRIVERS_TRACE(2, "lowbits=0x%x, high_bits=0x%x", lowbits, high_bits);
    efuse_f_v = ((high_bits & 0xFF) << 8) | ((lowbits & 0xFF00) >> 8);
    DRIVERS_TRACE(1, "efuse F:0x%x", efuse_f_v);
    *vol_4p4  = efuse_f_v & 0x3F;            //Bit[5:0]: Voreg4.4V cal
    *vol_4p35 = (efuse_f_v >> 6) & 0x3F;     //Bit[6:11]: Voreg4.35V cal

    DRIVERS_TRACE(3, "4p2=0x%x | 4p35=0x%x | 4p4=0x%x.", *vol_4p2, *vol_4p35, *vol_4p4);
}

#endif
