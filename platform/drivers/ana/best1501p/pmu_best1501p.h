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
#ifndef __PMU_BEST1501P_H__
#define __PMU_BEST1501P_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "hal_cmu.h"

#define ANA_REG(r)                          (((r) & 0xFFF) | 0x1000)
#define RF_REG(r)                           (((r) & 0xFFF) | 0x2000)
#define PMU_REG(r)                          (((r) & 0xFFF) | 0x3000)
#define USBPHY_REG(r)                       (((r) & 0xFFF) | 0x5000)

#define ISPI_PMU_REG(reg)                   (((reg) & 0xFFF) | 0x3000)

#define MAX_VMIC_CH_NUM                     2

enum PMU_EFUSE_PAGE_T {
    PMU_EFUSE_PAGE_SECURITY         = 0x00,
    PMU_EFUSE_PAGE_BOOT             = 0x01,
    PMU_EFUSE_PAGE_FEATURE          = 0x02,
    PMU_EFUSE_PAGE_BATTER_LV        = 0x03,

    PMU_EFUSE_PAGE_BATTER_HV        = 0x04,
    PMU_EFUSE_PAGE_SW_CFG           = 0x05,
    PMU_EFUSE_PAGE_PROD_TEST        = 0x06,
    PMU_EFUSE_PAGE_RESERVED_7       = 0x07,

    PMU_EFUSE_PAGE_BT_POWER         = 0x08,
    PMU_EFUSE_PAGE_DCCALIB2_L       = 0x09,
    PMU_EFUSE_PAGE_DCCALIB2_L_LP    = 0x0A,
    PMU_EFUSE_PAGE_DCCALIB_L        = 0x0B,

    PMU_EFUSE_PAGE_DCCALIB_L_LP     = 0x0C,
    PMU_EFUSE_PAGE_RESERVED_13      = 0x0D,
    PMU_EFUSE_PAGE_MODEL            = 0x0E,
    PMU_EFUSE_PAGE_RESERVED_15      = 0x0F,
    PMU_EFUSE_PAGE_RESERVED_0x10    = 0x10,
    PMU_EFUSE_PAGE_RESERVED_0x11    = 0x11,
    PMU_EFUSE_PAGE_X2_CALIB_0x12    = 0x12,
    PMU_EFUSE_PAGE_TEMPERATURE      = 0x14,
    PMU_EFUSE_PAGE_BATTER_LV_LP     = 0x15,
    PMU_EFUSE_PAGE_BATTER_HV_LP     = 0x16,
    PMU_EFUSE_PAGE_TEMPERATURE_LP   = 0x17,

    PMU_EFUSE_PAGE_0x19             = 0x19,
    PMU_EFUSE_PAGE_0x1A             = 0x1A,
    PMU_EFUSE_PAGE_0x1B             = 0x1B,
    PMU_EFUSE_PAGE_0x1C             = 0x1C,
    PMU_EFUSE_PAGE_0x1D             = 0x1D,
    PMU_EFUSE_PAGE_0x1E             = 0x1E,
    PMU_EFUSE_PAGE_0x1F             = 0x1F,

    PMU_EFUSE_PAGE_QTY              = 0x20,
};

enum PMU_IRQ_TYPE_T {
    PMU_IRQ_TYPE_GPADC,
    PMU_IRQ_TYPE_RTC,
    PMU_IRQ_TYPE_CHARGER,
    PMU_IRQ_TYPE_GPIO,
    PMU_IRQ_TYPE_WDT,
    PMU_IRQ_TYPE_PWRKEY,

    PMU_IRQ_TYPE_QTY
};

enum PMU_PLL_DIV_TYPE_T {
    PMU_PLL_DIV_DIG,
    PMU_PLL_DIV_CODEC,
};

enum PMU_BIG_BANDGAP_USER_T {
    PMU_BIG_BANDGAP_USER_GPADC          = (1 << 0),
};

union SECURITY_VALUE_T {
    struct {
        unsigned short security_en      :1;
        unsigned short mode             :1;
        unsigned short sig_type         :1;
        unsigned short reserved         :1;
        unsigned short key_id           :2;
        unsigned short vendor_id        :5;
        unsigned short flash_id         :1;
        unsigned short chksum           :4;
    } root;
    struct {
        unsigned short security_en      :1;
        unsigned short mode             :1;
        unsigned short sig_type         :1;
        unsigned short skip_romkey      :1;
        unsigned short key_id           :2;
        unsigned short reg_base         :2;
        unsigned short reg_size         :2;
        unsigned short reg_offset       :1;
        unsigned short flash_id         :1;
        unsigned short chksum           :4;
    } otp;
    unsigned short reg;
};
#define SECURITY_VALUE_T                    SECURITY_VALUE_T

struct PMU_CHIP_FACTORY_CTX_T {
    uint16_t wafer_coordinate_x;
    uint16_t wafer_coordinate_y;
    uint16_t wafer_id;
};

enum HAL_FLASH_ID_T pmu_get_boot_flash_ctrl_id(void);

uint8_t pmu_gpio_setup_irq(enum HAL_GPIO_PIN_T pin, const struct HAL_GPIO_IRQ_CFG_T *cfg);

void pmu_codec_mic_bias_set_volt(uint32_t map, uint32_t mv);

void pmu_codec_mic_bias_enable(uint32_t map, int enable);

void pmu_codec_mic_bias_lowpower_mode(uint32_t map, int enable);

int pmu_codec_volt_ramp_up(void);

int pmu_codec_volt_ramp_down(void);

void pmu_pll_div_reset_set(enum HAL_CMU_PLL_T pll);

void pmu_pll_div_reset_clear(enum HAL_CMU_PLL_T pll);

void pmu_pll_div_set(enum HAL_CMU_PLL_T pll, enum PMU_PLL_DIV_TYPE_T type, uint32_t div);

void pmu_pll_freq_reg_set(uint16_t low, uint16_t high, uint16_t high2);

void pmu_led_set_hiz(enum HAL_GPIO_PIN_T pin);

void pmu_led_uart_enable(enum HAL_IOMUX_PIN_T pin);

void pmu_led_uart_disable(enum HAL_IOMUX_PIN_T pin);

void pmu_rf_ana_init(void);

void pmu_big_bandgap_enable(enum PMU_BIG_BANDGAP_USER_T user, int enable);

void pmu_bt_reconn(bool en);

bool pmu_dig_set_volt_value(uint16_t dcdc_volt_mv, uint16_t logic_volt_mv, uint16_t ldo_volt_mv);

uint16_t pmu_ldo_sensor_real_volt_get(void);

void pmu_chip_wafer_ctx_get(struct PMU_CHIP_FACTORY_CTX_T *wafer_ctx);

void bbpll_pll_update(uint64_t pll_cfg_val, bool calib);

void bbpll_set_codec_div(uint32_t div);

void bbpll_codec_clock_enable(bool en);

typedef void (*PMU_NTC_IRQ_HANDLER_T)(uint16_t raw_val, int temperature);

int pmu_volt2temperature(const uint16_t volt);

int pmu_ntc_capture_start(PMU_NTC_IRQ_HANDLER_T cb);

uint16_t pmu_ntc_temperature_reference_get(void);

void bbpll_auto_calib_spd(uint32_t codec_freq, uint32_t div);

bool pmu_ana_volt_is_high(void);

int pmu_1620_deep_sleep_disable(void);

int pmu_1620_deep_sleep_enable(void);

void pmu_ldo_vio_off_pull_down(void);

void pmu_ldo_vio_on_lp_dr(bool dr_lp);

int pmu_wdt_reboot(void);

#ifdef __cplusplus
}
#endif

#endif

