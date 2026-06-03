/***************************************************************************
 *
 * Copyright 2015-2023 BES.
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
#ifndef __REG_BTCMU_BEST1306P_H__
#define __REG_BTCMU_BEST1306P_H__

#include "plat_types.h"

struct BTCMU_T {
    __IO uint32_t CLK_ENABLE;           // 0x00
    __IO uint32_t CLK_DISABLE;          // 0x04
    __IO uint32_t CLK_MODE;             // 0x08
    __IO uint32_t DIV_TIMER;            // 0x0C
    __IO uint32_t RESET_SET;            // 0x10
    __IO uint32_t RESET_CLR;            // 0x14
    __IO uint32_t DIV_WDT;              // 0x18
    __IO uint32_t RESET_PULSE;          // 0x1C
         uint32_t RESERVED_020[0x24 / 4]; // 0x20
    __IO uint32_t CLK_OUT;              // 0x44
         uint32_t RESERVED_048[2];      // 0x48
    __IO uint32_t ISIRQ_SET;            // 0x50
    __IO uint32_t ISIRQ_CLR;            // 0x54
};

// reg_00
#define BT_CMU_MANUAL_HCLK_ENABLE(n)                        (((n) & 0xFFFFFFFF) << 0)
#define BT_CMU_MANUAL_HCLK_ENABLE_MASK                      (0xFFFFFFFF << 0)
#define BT_CMU_MANUAL_HCLK_ENABLE_SHIFT                     (0)
#define BT_CMU_MANUAL_OCLK_ENABLE(n)                        (((n) & 0xFFFFFFFF) << BT_HCLK_NUM)
#define BT_CMU_MANUAL_OCLK_ENABLE_MASK                      (0xFFFFFFFF << BT_HCLK_NUM)
#define BT_CMU_MANUAL_OCLK_ENABLE_SHIFT                     (BT_HCLK_NUM)

// reg_04
#define BT_CMU_MANUAL_HCLK_DISABLE(n)                       (((n) & 0xFFFFFFFF) << 0)
#define BT_CMU_MANUAL_HCLK_DISABLE_MASK                     (0xFFFFFFFF << 0)
#define BT_CMU_MANUAL_HCLK_DISABLE_SHIFT                    (0)
#define BT_CMU_MANUAL_OCLK_DISABLE(n)                       (((n) & 0xFFFFFFFF) << BT_HCLK_NUM)
#define BT_CMU_MANUAL_OCLK_DISABLE_MASK                     (0xFFFFFFFF << BT_HCLK_NUM)
#define BT_CMU_MANUAL_OCLK_DISABLE_SHIFT                    (BT_HCLK_NUM)

// reg_08
#define BT_CMU_MODE_HCLK(n)                                 (((n) & 0xFFFFFFFF) << 0)
#define BT_CMU_MODE_HCLK_MASK                               (0xFFFFFFFF << 0)
#define BT_CMU_MODE_HCLK_SHIFT                              (0)
#define BT_CMU_MODE_OCLK(n)                                 (((n) & 0xFFFFFFFF) << BT_HCLK_NUM)
#define BT_CMU_MODE_OCLK_MASK                               (0xFFFFFFFF << BT_HCLK_NUM)
#define BT_CMU_MODE_OCLK_SHIFT                              (BT_HCLK_NUM)

// reg_0c
#define BT_CMU_CFG_DIV_TIMER0(n)                            (((n) & 0xFFFF) << 0)
#define BT_CMU_CFG_DIV_TIMER0_MASK                          (0xFFFF << 0)
#define BT_CMU_CFG_DIV_TIMER0_SHIFT                         (0)
#define BT_CMU_CFG_DIV_TIMER1(n)                            (((n) & 0xFFFF) << 16)
#define BT_CMU_CFG_DIV_TIMER1_MASK                          (0xFFFF << 16)
#define BT_CMU_CFG_DIV_TIMER1_SHIFT                         (16)

// reg_10
#define BT_CMU_HRESETN_SET(n)                               (((n) & 0xFFFFFFFF) << 0)
#define BT_CMU_HRESETN_SET_MASK                             (0xFFFFFFFF << 0)
#define BT_CMU_HRESETN_SET_SHIFT                            (0)
#define BT_CMU_ORESETN_SET(n)                               (((n) & 0xFFFFFFFF) << BT_HRST_NUM)
#define BT_CMU_ORESETN_SET_MASK                             (0xFFFFFFFF << BT_HRST_NUM)
#define BT_CMU_ORESETN_SET_SHIFT                            (BT_HRST_NUM)
#define BT_CMU_GLOBAL_RESETN_SET                            (1 << (BT_HRST_NUM+BT_ORST_NUM+1-1))

// reg_14
#define BT_CMU_HRESETN_CLR(n)                               (((n) & 0xFFFFFFFF) << 0)
#define BT_CMU_HRESETN_CLR_MASK                             (0xFFFFFFFF << 0)
#define BT_CMU_HRESETN_CLR_SHIFT                            (0)
#define BT_CMU_ORESETN_CLR(n)                               (((n) & 0xFFFFFFFF) << BT_HRST_NUM)
#define BT_CMU_ORESETN_CLR_MASK                             (0xFFFFFFFF << BT_HRST_NUM)
#define BT_CMU_ORESETN_CLR_SHIFT                            (BT_HRST_NUM)
#define BT_CMU_GLOBAL_RESETN_CLR                            (1 << (BT_HRST_NUM+BT_ORST_NUM+1-1))

// reg_18
#define BT_CMU_CFG_DIV_WDT(n)                               (((n) & 0xFFFF) << 0)
#define BT_CMU_CFG_DIV_WDT_MASK                             (0xFFFF << 0)
#define BT_CMU_CFG_DIV_WDT_SHIFT                            (0)
#define BT_CMU_CFG_HCLK_MCU_OFF_TIMER(n)                    (((n) & 0xFF) << 16)
#define BT_CMU_CFG_HCLK_MCU_OFF_TIMER_MASK                  (0xFF << 16)
#define BT_CMU_CFG_HCLK_MCU_OFF_TIMER_SHIFT                 (16)
#define BT_CMU_UART_BUS_IDLE_TIMER(n)                       (((n) & 0xFF) << 24)
#define BT_CMU_UART_BUS_IDLE_TIMER_MASK                     (0xFF << 24)
#define BT_CMU_UART_BUS_IDLE_TIMER_SHIFT                    (24)

// reg_1c
#define BT_CMU_HRESETN_PULSE(n)                             (((n) & 0xFFFFFFFF) << 0)
#define BT_CMU_HRESETN_PULSE_MASK                           (0xFFFFFFFF << 0)
#define BT_CMU_HRESETN_PULSE_SHIFT                          (0)
#define BT_CMU_ORESETN_PULSE(n)                             (((n) & 0xFFFFFFFF) << BT_HRST_NUM)
#define BT_CMU_ORESETN_PULSE_MASK                           (0xFFFFFFFF << BT_HRST_NUM)
#define BT_CMU_ORESETN_PULSE_SHIFT                          (BT_HRST_NUM)
#define BT_CMU_GLOBAL_RESETN_PULSE                          (1 << (BT_HRST_NUM+BT_ORST_NUM+1-1))

// reg_20
#define BT_CMU_MODE_MCLK(n)                                 (((n) & 0xFFFFFFFF) << 0)
#define BT_CMU_MODE_MCLK_MASK                               (0xFFFFFFFF << 0)
#define BT_CMU_MODE_MCLK_SHIFT                              (0)

// reg_24
#define BT_CMU_DEEPSLEEP_START                              (1 << 0)
#define BT_CMU_DEEPSLEEP_GATE                               (1 << 1)
#define BT_CMU_CAL_LPO_INT_EN                               (1 << 2)
#define BT_CMU_HCLK_MCU_ENABLE                              (1 << 3)
#define BT_CMU_DEEPSLEEP_EN                                 (1 << 4)
#define BT_CMU_DEEPSLEEP_ROMRAM_EN                          (1 << 5)
#define BT_CMU_OSC_EN_BT_EN                                 (1 << 6)
#define BT_CMU_UART_WAKE_EN                                 (1 << 7)
#define BT_CMU_TIMER_WT26(n)                                (((n) & 0xFF) << 8)
#define BT_CMU_TIMER_WT26_MASK                              (0xFF << 8)
#define BT_CMU_TIMER_WT26_SHIFT                             (8)
#define BT_CMU_CPU_IDLE_GATE_EN                             (1 << 16)
#define BT_CMU_MANUAL_RAM_RETN                              (1 << 17)
#define BT_CMU_CPU_IDLE_GATE_DIS                            (1 << 18)
#define BT_CMU_DEEPSLEEP_MODE                               (1 << 24)
#define BT_CMU_PU_OSC                                       (1 << 25)
#define BT_CMU_UART_WAKE                                    (1 << 27)
#define BT_CMU_SLEEP_ALLOW                                  (1 << 29)
#define BT_CMU_DEEPSLEEP_ALLOWED                            (1 << 30)
#define BT_CMU_WAKEUP_DEEPSLEEP_SYNC_N                      (1 << 31)

// reg_28
#define BT_CMU_TARGET_ADDR(n)                               (((n) & 0xFFFFFFFF) << 0)
#define BT_CMU_TARGET_ADDR_MASK                             (0xFFFFFFFF << 0)
#define BT_CMU_TARGET_ADDR_SHIFT                            (0)

// reg_2c
#define BT_CMU_BM_IRQ_CLR                                   (1 << 0)
#define BT_CMU_BM_IRQ_EN                                    (1 << 1)
#define BT_CMU_TARGET_OP                                    (1 << 2)
#define BT_CMU_MANUAL_EN_AON                                (1 << 3)
#define BT_CMU_CACHEC_REMAP                                 (1 << 4)
#define BT_CMU_REG_OSCX2_OPEN                               (1 << 5)
#define BT_CMU_MANUAL_EN_BTC2BTH                            (1 << 6)
#define BT_CMU_MANUAL_EN_BTC2SYS                            (1 << 7)
#define BT_CMU_MANUAL_EN_BTC2WIFI                           (1 << 8)
#define BT_CMU_RF_APBWR_MODE                                (1 << 9)

// reg_30
#define BT_CMU_RAM_EMA(n)                                   (((n) & 0x7) << 0)
#define BT_CMU_RAM_EMA_MASK                                 (0x7 << 0)
#define BT_CMU_RAM_EMA_SHIFT                                (0)
#define BT_CMU_RAM_EMAW(n)                                  (((n) & 0x3) << 3)
#define BT_CMU_RAM_EMAW_MASK                                (0x3 << 3)
#define BT_CMU_RAM_EMAW_SHIFT                               (3)
#define BT_CMU_RAM_RAWL                                     (1 << 5)
#define BT_CMU_RAM_RAWLM(n)                                 (((n) & 0x3) << 6)
#define BT_CMU_RAM_RAWLM_MASK                               (0x3 << 6)
#define BT_CMU_RAM_RAWLM_SHIFT                              (6)
#define BT_CMU_RAM_RET1N0                                   (1 << 8)
#define BT_CMU_RAM_RET2N0                                   (1 << 9)
#define BT_CMU_RAM_PGEN0                                    (1 << 10)
#define BT_CMU_RAM_RET1N1                                   (1 << 11)
#define BT_CMU_RAM_RET2N1                                   (1 << 12)
#define BT_CMU_RAM_PGEN1                                    (1 << 13)
#define BT_CMU_RAM_EMAS                                     (1 << 14)
#define BT_CMU_RAM_WABL                                     (1 << 15)
#define BT_CMU_RAM_WABLM(n)                                 (((n) & 0x7) << 16)
#define BT_CMU_RAM_WABLM_MASK                               (0x7 << 16)
#define BT_CMU_RAM_WABLM_SHIFT                              (16)

// reg_34
#define BT_CMU_RF_EMA(n)                                    (((n) & 0x7) << 0)
#define BT_CMU_RF_EMA_MASK                                  (0x7 << 0)
#define BT_CMU_RF_EMA_SHIFT                                 (0)
#define BT_CMU_RF_EMAW(n)                                   (((n) & 0x3) << 3)
#define BT_CMU_RF_EMAW_MASK                                 (0x3 << 3)
#define BT_CMU_RF_EMAW_SHIFT                                (3)
#define BT_CMU_RF_RAWL                                      (1 << 5)
#define BT_CMU_RF_RAWLM(n)                                  (((n) & 0x3) << 6)
#define BT_CMU_RF_RAWLM_MASK                                (0x3 << 6)
#define BT_CMU_RF_RAWLM_SHIFT                               (6)
#define BT_CMU_RF_RET1N0                                    (1 << 8)
#define BT_CMU_RF_RET2N0                                    (1 << 9)
#define BT_CMU_RF_PGEN0                                     (1 << 10)
#define BT_CMU_RF_RET1N1                                    (1 << 11)
#define BT_CMU_RF_RET2N1                                    (1 << 12)
#define BT_CMU_RF_PGEN1                                     (1 << 13)
#define BT_CMU_RF_EMAS                                      (1 << 14)
#define BT_CMU_RF_WABL                                      (1 << 15)
#define BT_CMU_RF_WABLM(n)                                  (((n) & 0x3) << 16)
#define BT_CMU_RF_WABLM_MASK                                (0x3 << 16)
#define BT_CMU_RF_WABLM_SHIFT                               (16)

// reg_38
#define BT_CMU_BT_CLKSEL(n)                                 (((n) & 0x3F) << 0)
#define BT_CMU_BT_CLKSEL_MASK                               (0x3F << 0)
#define BT_CMU_BT_CLKSEL_SHIFT                              (0)
#define BT_CMU_SEL_SYS_PLL                                  (1 << 6)
#define BT_CMU_SEL_SYS_OSCX2                                (1 << 7)
#define BT_CMU_SEL_SYS_OSCX4                                (1 << 8)
#define BT_CMU_SEL_SYS_32K                                  (1 << 10)
#define BT_CMU_POL_CLK_ADC                                  (1 << 12)
#define BT_CMU_POL_CLK_DAC                                  (1 << 13)
#define BT_CMU_POL_CLK_OSC                                  (1 << 14)
#define BT_CMU_SEL_TIMER_FAST                               (1 << 15)
#define BT_CMU_SEL_32K_TIMER                                (1 << 16)
#define BT_CMU_SEL_WDT_FAST                                 (1 << 17)
#define BT_CMU_SEL_32K_WDT                                  (1 << 18)
#define BT_CMU_EN_BT2M                                      (1 << 19)
#define BT_CMU_FORCE_SYS_CLK_ON                             (1 << 20)
#define BT_CMU_EN_CLK_CPU_EM                                (1 << 21)
#define BT_CMU_FORCE_PU_OFF                                 (1 << 22)
#define BT_CMU_SEL_CALIB_OSCX2                              (1 << 23)
#define BT_CMU_POL_CLK_OSCX2                                (1 << 24)
#define BT_CMU_POL_CLK_OSCX4                                (1 << 25)
#define BT_CMU_EN_BT4M                                      (1 << 26)
#define BT_CMU_EN_ANT_MASTER_GCLK                           (1 << 27)
#define BT_CMU_EN_SLE_MASTER_GCLK                           (1 << 28)

// reg_3c
#define BT_CMU_WAKEUP_IRQ_MASK(n)                           (((n) & 0xFFFFFFFF) << 0)
#define BT_CMU_WAKEUP_IRQ_MASK_MASK                         (0xFFFFFFFF << 0)
#define BT_CMU_WAKEUP_IRQ_MASK_SHIFT                        (0)

// reg_40
#define BT_CMU_WRITE_UNLOCK_H                               (1 << 0)
#define BT_CMU_WRITE_UNLOCK_STATUS                          (1 << 1)

// reg_44
#define BT_CMU_CAL_TIME(n)                                  (((n) & 0xFF) << 0)
#define BT_CMU_CAL_TIME_MASK                                (0xFF << 0)
#define BT_CMU_CAL_TIME_SHIFT                               (0)
#define BT_CMU_SMP_CMU_SEL(n)                               (((n) & 0xF) << 8)
#define BT_CMU_SMP_CMU_SEL_MASK                             (0xF << 8)
#define BT_CMU_SMP_CMU_SEL_SHIFT                            (8)
#define BT_CMU_CFG_CLK_OUT(n)                               (((n) & 0xF) << 12)
#define BT_CMU_CFG_CLK_OUT_MASK                             (0xF << 12)
#define BT_CMU_CFG_CLK_OUT_SHIFT                            (12)
#define BT_CMU_ROM_RTSEL(n)                                 (((n) & 0x3) << 16)
#define BT_CMU_ROM_RTSEL_MASK                               (0x3 << 16)
#define BT_CMU_ROM_RTSEL_SHIFT                              (16)
#define BT_CMU_ROM_PTSEL(n)                                 (((n) & 0x3) << 18)
#define BT_CMU_ROM_PTSEL_MASK                               (0x3 << 18)
#define BT_CMU_ROM_PTSEL_SHIFT                              (18)
#define BT_CMU_ROM_TRB(n)                                   (((n) & 0x3) << 20)
#define BT_CMU_ROM_TRB_MASK                                 (0x3 << 20)
#define BT_CMU_ROM_TRB_SHIFT                                (20)
#define BT_CMU_ROM_PGEN(n)                                  (((n) & 0x1F) << 22)
#define BT_CMU_ROM_PGEN_MASK                                (0x1F << 22)
#define BT_CMU_ROM_PGEN_SHIFT                               (22)
#define BT_CMU_ROM_EMA(n)                                   (((n) & 0x7) << 27)
#define BT_CMU_ROM_EMA_MASK                                 (0x7 << 27)
#define BT_CMU_ROM_EMA_SHIFT                                (27)
#define BT_CMU_ROM_KEN                                      (1 << 30)

// reg_48
#define BT_CMU_CAL_COUNT_VALUE(n)                           (((n) & 0x7FFFFFFF) << 0)
#define BT_CMU_CAL_COUNT_VALUE_MASK                         (0x7FFFFFFF << 0)
#define BT_CMU_CAL_COUNT_VALUE_SHIFT                        (0)
#define BT_CMU_CAL_LPO_DONE                                 (1 << 31)

// reg_4c
#define BT_CMU_CPU_IDLE_FLAG                                (1 << 0)

// reg_50
#define BT_CMU_MCU2BT_DATA_DONE_SET                         (1 << 0)
#define BT_CMU_MCU2BT_DATA1_DONE_SET                        (1 << 1)
#define BT_CMU_BT2MCU_DATA_IND_SET                          (1 << 2)
#define BT_CMU_BT2MCU_DATA1_IND_SET                         (1 << 3)
#define BT_CMU_WAKEUP_BT                                    (1 << 4)
#define BT_CMU_MCU_ALLIRQ_MASK_SET                          (1 << 5)
#define BT_CMU_BT2MCU_ALLIRQ_MASK_SET(n)                    (((n) & 0x1FF) << 6)
#define BT_CMU_BT2MCU_ALLIRQ_MASK_SET_MASK                  (0x1FF << 6)
#define BT_CMU_BT2MCU_ALLIRQ_MASK_SET_SHIFT                 (6)
#define BT_CMU_BT2AON_IRQ_SET                               (1 << 15)
#define BT_CMU_SENS2BT_DATA_DONE_SET                        (1 << 16)
#define BT_CMU_SENS2BT_DATA1_DONE_SET                       (1 << 17)
#define BT_CMU_BT2SENS_DATA_IND_SET                         (1 << 18)
#define BT_CMU_BT2SENS_DATA1_IND_SET                        (1 << 19)
#define BT_CMU_SENS_ALLIRQ_MASK_SET                         (1 << 20)
#define BT_CMU_BT2SENS_ALLIRQ_MASK_SET(n)                   (((n) & 0x1FF) << 21)
#define BT_CMU_BT2SENS_ALLIRQ_MASK_SET_MASK                 (0x1FF << 21)
#define BT_CMU_BT2SENS_ALLIRQ_MASK_SET_SHIFT                (21)

// reg_54
#define BT_CMU_MCU2BT_DATA_DONE_CLR                         (1 << 0)
#define BT_CMU_MCU2BT_DATA1_DONE_CLR                        (1 << 1)
#define BT_CMU_BT2MCU_DATA_IND_CLR                          (1 << 2)
#define BT_CMU_BT2MCU_DATA1_IND_CLR                         (1 << 3)
#define BT_CMU_MCU_ALLIRQ_MASK_CLR                          (1 << 5)
#define BT_CMU_BT2MCU_ALLIRQ_MASK_CLR(n)                    (((n) & 0x1FF) << 6)
#define BT_CMU_BT2MCU_ALLIRQ_MASK_CLR_MASK                  (0x1FF << 6)
#define BT_CMU_BT2MCU_ALLIRQ_MASK_CLR_SHIFT                 (6)
#define BT_CMU_BT2AON_IRQ_CLR                               (1 << 15)
#define BT_CMU_SENS2BT_DATA_DONE_CLR                        (1 << 16)
#define BT_CMU_SENS2BT_DATA1_DONE_CLR                       (1 << 17)
#define BT_CMU_BT2SENS_DATA_IND_CLR                         (1 << 18)
#define BT_CMU_BT2SENS_DATA1_IND_CLR                        (1 << 19)
#define BT_CMU_SENS_ALLIRQ_MASK_CLR                         (1 << 20)
#define BT_CMU_BT2SENS_ALLIRQ_MASK_CLR(n)                   (((n) & 0x1FF) << 21)
#define BT_CMU_BT2SENS_ALLIRQ_MASK_CLR_MASK                 (0x1FF << 21)
#define BT_CMU_BT2SENS_ALLIRQ_MASK_CLR_SHIFT                (21)

// reg_58
#define BT_CMU_MRESETN_MODE(n)                              (((n) & 0xFFFFFFFF) << 0)
#define BT_CMU_MRESETN_MODE_MASK                            (0xFFFFFFFF << 0)
#define BT_CMU_MRESETN_MODE_SHIFT                           (0)

// reg_5c
#define BT_CMU_MRESETN_RXTX(n)                              (((n) & 0xFFFFFFFF) << 0)
#define BT_CMU_MRESETN_RXTX_MASK                            (0xFFFFFFFF << 0)
#define BT_CMU_MRESETN_RXTX_SHIFT                           (0)

// reg_60
#define BT_CMU_MRESETN_SET(n)                               (((n) & 0xFFFFFFFF) << 0)
#define BT_CMU_MRESETN_SET_MASK                             (0xFFFFFFFF << 0)
#define BT_CMU_MRESETN_SET_SHIFT                            (0)

// reg_64
#define BT_CMU_MRESETN_CLR(n)                               (((n) & 0xFFFFFFFF) << 0)
#define BT_CMU_MRESETN_CLR_MASK                             (0xFFFFFFFF << 0)
#define BT_CMU_MRESETN_CLR_SHIFT                            (0)

// reg_68
#define BT_CMU_MANUAL_MCLK_ENABLE(n)                        (((n) & 0xFFFFFFFF) << 0)
#define BT_CMU_MANUAL_MCLK_ENABLE_MASK                      (0xFFFFFFFF << 0)
#define BT_CMU_MANUAL_MCLK_ENABLE_SHIFT                     (0)

// reg_6c
#define BT_CMU_MANUAL_MCLK_DISABLE(n)                       (((n) & 0xFFFFFFFF) << 0)
#define BT_CMU_MANUAL_MCLK_DISABLE_MASK                     (0xFFFFFFFF << 0)
#define BT_CMU_MANUAL_MCLK_DISABLE_SHIFT                    (0)

// reg_70
#define BT_CMU_MRESETN_MODE2(n)                             (((n) & 0xFFFFFFFF) << 0)
#define BT_CMU_MRESETN_MODE2_MASK                           (0xFFFFFFFF << 0)
#define BT_CMU_MRESETN_MODE2_SHIFT                          (0)

// reg_74
#define BT_CMU_MRESETN_RXTX2(n)                             (((n) & 0xFFFFFFFF) << 0)
#define BT_CMU_MRESETN_RXTX2_MASK                           (0xFFFFFFFF << 0)
#define BT_CMU_MRESETN_RXTX2_SHIFT                          (0)

// reg_78
#define BT_CMU_MRESETN_SET2(n)                              (((n) & 0xFFFFFFFF) << 0)
#define BT_CMU_MRESETN_SET2_MASK                            (0xFFFFFFFF << 0)
#define BT_CMU_MRESETN_SET2_SHIFT                           (0)

// reg_7c
#define BT_CMU_MRESETN_CLR2(n)                              (((n) & 0xFFFFFFFF) << 0)
#define BT_CMU_MRESETN_CLR2_MASK                            (0xFFFFFFFF << 0)
#define BT_CMU_MRESETN_CLR2_SHIFT                           (0)

// reg_80
#define BT_CMU_MANUAL_MCLK_ENABLE2(n)                       (((n) & 0xFFFFFFFF) << 0)
#define BT_CMU_MANUAL_MCLK_ENABLE2_MASK                     (0xFFFFFFFF << 0)
#define BT_CMU_MANUAL_MCLK_ENABLE2_SHIFT                    (0)

// reg_84
#define BT_CMU_MANUAL_MCLK_DISABLE2(n)                      (((n) & 0xFFFFFFFF) << 0)
#define BT_CMU_MANUAL_MCLK_DISABLE2_MASK                    (0xFFFFFFFF << 0)
#define BT_CMU_MANUAL_MCLK_DISABLE2_SHIFT                   (0)

// reg_88
#define BT_CMU_MODE_MCLK2(n)                                (((n) & 0xFFFFFFFF) << 0)
#define BT_CMU_MODE_MCLK2_MASK                              (0xFFFFFFFF << 0)
#define BT_CMU_MODE_MCLK2_SHIFT                             (0)

// reg_8c
#define BT_CMU_WAKEUP_IRQ_MASK_EXT(n)                       (((n) & 0xFFFF) << 0)
#define BT_CMU_WAKEUP_IRQ_MASK_EXT_MASK                     (0xFFFF << 0)
#define BT_CMU_WAKEUP_IRQ_MASK_EXT_SHIFT                    (0)

// reg_90
#define BT_CMU_IRQ_MASKED_EXT(n)                            (((n) & 0xFFFF) << 0)
#define BT_CMU_IRQ_MASKED_EXT_MASK                          (0xFFFF << 0)
#define BT_CMU_IRQ_MASKED_EXT_SHIFT                         (0)

// reg_a0
#define BT_CMU_BT2MCU_DATA_MSK_SET                          (1 << 0)
#define BT_CMU_BT2MCU_DATA1_MSK_SET                         (1 << 1)
#define BT_CMU_MCU2BT_DATA_MSK_SET                          (1 << 2)
#define BT_CMU_MCU2BT_DATA1_MSK_SET                         (1 << 3)
#define BT_CMU_BT2MCU_DATA_INTR                             (1 << 4)
#define BT_CMU_BT2MCU_DATA1_INTR                            (1 << 5)
#define BT_CMU_MCU2BT_DATA_INTR                             (1 << 6)
#define BT_CMU_MCU2BT_DATA1_INTR                            (1 << 7)
#define BT_CMU_BT2MCU_DATA_INTR_MSK                         (1 << 8)
#define BT_CMU_BT2MCU_DATA1_INTR_MSK                        (1 << 9)
#define BT_CMU_MCU2BT_DATA_INTR_MSK                         (1 << 10)
#define BT_CMU_MCU2BT_DATA1_INTR_MSK                        (1 << 11)
#define BT_CMU_BT2SENS_DATA_MSK_SET                         (1 << 16)
#define BT_CMU_BT2SENS_DATA1_MSK_SET                        (1 << 17)
#define BT_CMU_SENS2BT_DATA_MSK_SET                         (1 << 18)
#define BT_CMU_SENS2BT_DATA1_MSK_SET                        (1 << 19)
#define BT_CMU_BT2SENS_DATA_INTR                            (1 << 20)
#define BT_CMU_BT2SENS_DATA1_INTR                           (1 << 21)
#define BT_CMU_SENS2BT_DATA_INTR                            (1 << 22)
#define BT_CMU_SENS2BT_DATA1_INTR                           (1 << 23)
#define BT_CMU_BT2SENS_DATA_INTR_MSK                        (1 << 24)
#define BT_CMU_BT2SENS_DATA1_INTR_MSK                       (1 << 25)
#define BT_CMU_SENS2BT_DATA_INTR_MSK                        (1 << 26)
#define BT_CMU_SENS2BT_DATA1_INTR_MSK                       (1 << 27)

// reg_a4
#define BT_CMU_BT2MCU_DATA_MSK_CLR                          (1 << 0)
#define BT_CMU_BT2MCU_DATA1_MSK_CLR                         (1 << 1)
#define BT_CMU_MCU2BT_DATA_MSK_CLR                          (1 << 2)
#define BT_CMU_MCU2BT_DATA1_MSK_CLR                         (1 << 3)
#define BT_CMU_BT2SENS_DATA_MSK_CLR                         (1 << 16)
#define BT_CMU_BT2SENS_DATA1_MSK_CLR                        (1 << 17)
#define BT_CMU_SENS2BT_DATA_MSK_CLR                         (1 << 18)
#define BT_CMU_SENS2BT_DATA1_MSK_CLR                        (1 << 19)

// reg_a8
#define BT_CMU_SEL_PCM_SYS                                  (1 << 0)
#define BT_CMU_SEL_SYS_GT(n)                                (((n) & 0x3) << 1)
#define BT_CMU_SEL_SYS_GT_MASK                              (0x3 << 1)
#define BT_CMU_SEL_SYS_GT_SHIFT                             (1)

// reg_ac
#define BT_CMU_MCU2BT_DATA2_DONE_SET                        (1 << 0)
#define BT_CMU_MCU2BT_DATA3_DONE_SET                        (1 << 1)
#define BT_CMU_BT2MCU_DATA2_IND_SET                         (1 << 2)
#define BT_CMU_BT2MCU_DATA3_IND_SET                         (1 << 3)
#define BT_CMU_MCU2BT_DATA2_MSK_SET                         (1 << 4)
#define BT_CMU_MCU2BT_DATA3_MSK_SET                         (1 << 5)
#define BT_CMU_BT2MCU_DATA2_MSK_SET                         (1 << 6)
#define BT_CMU_BT2MCU_DATA3_MSK_SET                         (1 << 7)
#define BT_CMU_MCU2BT_DATA2_INTR                            (1 << 8)
#define BT_CMU_MCU2BT_DATA3_INTR                            (1 << 9)
#define BT_CMU_BT2MCU_DATA2_INTR                            (1 << 10)
#define BT_CMU_BT2MCU_DATA3_INTR                            (1 << 11)
#define BT_CMU_MCU2BT_DATA2_INTR_MSK                        (1 << 12)
#define BT_CMU_MCU2BT_DATA3_INTR_MSK                        (1 << 13)
#define BT_CMU_BT2MCU_DATA2_INTR_MSK                        (1 << 14)
#define BT_CMU_BT2MCU_DATA3_INTR_MSK                        (1 << 15)

// reg_b0
#define BT_CMU_MCU2BT_DATA2_DONE_CLR                        (1 << 0)
#define BT_CMU_MCU2BT_DATA3_DONE_CLR                        (1 << 1)
#define BT_CMU_BT2MCU_DATA2_IND_CLR                         (1 << 2)
#define BT_CMU_BT2MCU_DATA3_IND_CLR                         (1 << 3)
#define BT_CMU_MCU2BT_DATA2_MSK_CLR                         (1 << 4)
#define BT_CMU_MCU2BT_DATA3_MSK_CLR                         (1 << 5)
#define BT_CMU_BT2MCU_DATA2_MSK_CLR                         (1 << 6)
#define BT_CMU_BT2MCU_DATA3_MSK_CLR                         (1 << 7)

// reg_b8
#define BT_CMU_MTB_SRAM_BASE(n)                             (((n) & 0xFFFFFFFF) << 0)
#define BT_CMU_MTB_SRAM_BASE_MASK                           (0xFFFFFFFF << 0)
#define BT_CMU_MTB_SRAM_BASE_SHIFT                          (0)

// reg_bc
#define BT_CMU_BT_PC_VALUE(n)                               (((n) & 0xFFFFFFFF) << 0)
#define BT_CMU_BT_PC_VALUE_MASK                             (0xFFFFFFFF << 0)
#define BT_CMU_BT_PC_VALUE_SHIFT                            (0)

// reg_c0
#define BT_CMU_WIFI2BT_DATA_DONE_SET                        (1 << 0)
#define BT_CMU_BT2WIFI_DATA_IND_SET                         (1 << 1)
#define BT_CMU_WIFI2BT_DATA_MSK_SET                         (1 << 2)
#define BT_CMU_BT2WIFI_DATA_MSK_SET                         (1 << 3)
#define BT_CMU_WIFI2BT_DATA_INTR                            (1 << 4)
#define BT_CMU_BT2WIFI_DATA_INTR                            (1 << 5)
#define BT_CMU_WIFI2BT_DATA_INTR_MSK                        (1 << 6)
#define BT_CMU_BT2WIFI_DATA_INTR_MSK                        (1 << 7)
#define BT_CMU_WIFI_ALLIRQ_MASK_SET                         (1 << 8)
#define BT_CMU_BT2WIFI_ALLIRQ_MASK_SET(n)                   (((n) & 0x1FF) << 9)
#define BT_CMU_BT2WIFI_ALLIRQ_MASK_SET_MASK                 (0x1FF << 9)
#define BT_CMU_BT2WIFI_ALLIRQ_MASK_SET_SHIFT                (9)

// reg_c4
#define BT_CMU_WIFI2BT_DATA_DONE_CLR                        (1 << 0)
#define BT_CMU_BT2WIFI_DATA_IND_CLR                         (1 << 1)
#define BT_CMU_WIFI2BT_DATA_MSK_CLR                         (1 << 2)
#define BT_CMU_BT2WIFI_DATA_MSK_CLR                         (1 << 3)
#define BT_CMU_WIFI_ALLIRQ_MASK_CLR                         (1 << 4)
#define BT_CMU_BT2WIFI_ALLIRQ_MASK_CLR(n)                   (((n) & 0x1FF) << 5)
#define BT_CMU_BT2WIFI_ALLIRQ_MASK_CLR_MASK                 (0x1FF << 5)
#define BT_CMU_BT2WIFI_ALLIRQ_MASK_CLR_SHIFT                (5)

// reg_c8
#define BT_CMU_IRQ_MASKED(n)                                (((n) & 0xFFFFFFFF) << 0)
#define BT_CMU_IRQ_MASKED_MASK                              (0xFFFFFFFF << 0)
#define BT_CMU_IRQ_MASKED_SHIFT                             (0)

// reg_cc
#define BT_CMU_PVT_RO_EN                                    (1 << 0)
#define BT_CMU_PVT_COUNT_START_EN                           (1 << 1)
#define BT_CMU_PVT_OSC_CLK_EN                               (1 << 2)
#define BT_CMU_PVT_RSTN                                     (1 << 3)
#define BT_CMU_PVT_REG_TIMESET(n)                           (((n) & 0x3FF) << 4)
#define BT_CMU_PVT_REG_TIMESET_MASK                         (0x3FF << 4)
#define BT_CMU_PVT_REG_TIMESET_SHIFT                        (4)

// reg_d0
#define BT_CMU_PVT_COUNT_VALUE(n)                           (((n) & 0x3FFFF) << 0)
#define BT_CMU_PVT_COUNT_VALUE_MASK                         (0x3FFFF << 0)
#define BT_CMU_PVT_COUNT_VALUE_SHIFT                        (0)
#define BT_CMU_PVT_COUNT_DONE                               (1 << 18)

// reg_f0
#define BT_CMU_RESERVED(n)                                  (((n) & 0xFFFFFFFF) << 0)
#define BT_CMU_RESERVED_MASK                                (0xFFFFFFFF << 0)
#define BT_CMU_RESERVED_SHIFT                               (0)

#endif

