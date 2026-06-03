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
#ifndef __REG_CAPSENSOR_BEST1502P_H__
#define __REG_CAPSENSOR_BEST1502P_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "plat_types.h"

enum CAPSENSOR_T {
    CAP_REG_00 = 0x0,
    CAP_REG_01,
    CAP_REG_02,
    CAP_REG_03,
    CAP_REG_04,
    CAP_REG_05,
    CAP_REG_06,
    CAP_REG_07,
    CAP_REG_08,
    CAP_REG_09,
    CAP_REG_0A,
    CAP_REG_0B,
    CAP_REG_0C,
    CAP_REG_0D,
    CAP_REG_0E,
    CAP_REG_0F,

    CAP_REG_10,
    CAP_REG_11,
    CAP_REG_12,
    CAP_REG_13,
    CAP_REG_14,
    CAP_REG_15,
    CAP_REG_16,
    CAP_REG_17,
    CAP_REG_18,
    CAP_REG_19,
    CAP_REG_1A,
    CAP_REG_1B,
    CAP_REG_1C,
    CAP_REG_1D,
    CAP_REG_1E,
    CAP_REG_1F,

    CAP_REG_20,
    CAP_REG_21,
    CAP_REG_22,
    CAP_REG_23,
    CAP_REG_24,
    CAP_REG_25,
    CAP_REG_26,
    CAP_REG_27,
    CAP_REG_28,
    CAP_REG_29,
    CAP_REG_2A,
    CAP_REG_2B,
    CAP_REG_2C,
    CAP_REG_2D,
    CAP_REG_2E,
    CAP_REG_2F,

    CAP_REG_30,
    CAP_REG_31,
    CAP_REG_32,
    CAP_REG_33,
    CAP_REG_34,
    CAP_REG_35,
    CAP_REG_36,
    CAP_REG_37,
    CAP_REG_38,
    CAP_REG_39,
    CAP_REG_3A,
    CAP_REG_3B,
    CAP_REG_3C,
    CAP_REG_3D,
    CAP_REG_3E,
    CAP_REG_3F,

    CAP_REG_40,
    CAP_REG_41,
    CAP_REG_42,
    CAP_REG_43,
    CAP_REG_44,
    CAP_REG_45,
    CAP_REG_46,
    CAP_REG_47,
    CAP_REG_48,
    CAP_REG_49,
    CAP_REG_4A,
    CAP_REG_4B,
    CAP_REG_4C,
    CAP_REG_4D,
    CAP_REG_4E,
    CAP_REG_4F,

    CAP_REG_50,
    CAP_REG_51,
    CAP_REG_52,
    CAP_REG_53,
    CAP_REG_54,
    CAP_REG_55,
    CAP_REG_56,
    CAP_REG_57,
    CAP_REG_58,
    CAP_REG_59,
    CAP_REG_5A,
    CAP_REG_5B,
    CAP_REG_5C,
    CAP_REG_5D,
    CAP_REG_5E,
    CAP_REG_5F,

    CAP_REG_60,
    CAP_REG_61,
    CAP_REG_62,
    CAP_REG_63,
    CAP_REG_64,
    CAP_REG_65,
    CAP_REG_66,
    CAP_REG_67,
    CAP_REG_68,
    CAP_REG_69,
    CAP_REG_6A,
    CAP_REG_6B,
    CAP_REG_6C,
    CAP_REG_6D,
    CAP_REG_6E,
    CAP_REG_6F,

    CAP_REG_70,
    CAP_REG_71,
    CAP_REG_72,
    CAP_REG_73,
    CAP_REG_74,
    CAP_REG_75,
    CAP_REG_76,
    CAP_REG_77,
    CAP_REG_78,
    CAP_REG_79,
    CAP_REG_7A,
    CAP_REG_7B,
    CAP_REG_7C,
    CAP_REG_7D,
    CAP_REG_7E,
    CAP_REG_7F,

    CAP_REG_80,
    CAP_REG_81,
    CAP_REG_82,
    CAP_REG_83,
    CAP_REG_84,
    CAP_REG_85,
    CAP_REG_86,
    CAP_REG_87,
    CAP_REG_88,
    CAP_REG_89,
    CAP_REG_8A,
    CAP_REG_8B,
    CAP_REG_8C,
    CAP_REG_8D,
    CAP_REG_8E,
    CAP_REG_8F,

    CAP_REG_90,
    CAP_REG_91,
    CAP_REG_92,
    CAP_REG_93,
    CAP_REG_94,
    CAP_REG_95,
    CAP_REG_96,
    CAP_REG_97,
    CAP_REG_98,
};

// REG_01
#define FSM_EN                          (1 << 0)
#define CDC_SDM_MODE_DR                 (1 << 1)
#define CDC_SDM_MODE_REG                (1 << 2)
#define CDC_SAR_N_MODE_REG              (1 << 3)
#define CDC_SAR_P_MODE_DR               (1 << 4)
#define CDC_SAR_P_MODE_REG              (1 << 5)
#define CDC_CAL_N_MODE_DR               (1 << 6)
#define CDC_CAL_N_MODE_REG              (1 << 7)
#define CDC_CAL_P_MODE_DR               (1 << 8)
#define CDC_CAL_P_MODE_REG              (1 << 9)
#define CDC_P_MODE_DR                   (1 << 10)
#define CDC_P_MODE_REG                  (1 << 11)
#define CDC_PU_VREF0P7                  (1 << 12)
#define CDC_PU_LDO                      (1 << 13)
#define CDC_PU_CDC_DR                   (1 << 14)
#define CDC_PU_CDC_REG                  (1 << 15)

// REG_02
#define CDC_PU_P_CHAN_DR                (1 << 2)
#define CDC_PU_P_CHAN_REG               (1 << 3)
#define ACTIVE_CH_NUM_SHIFT             4
#define ACTIVE_CH_NUM_MASK              (0x7 << ACTIVE_CH_NUM_SHIFT)
#define ACTIVE_CH_NUM(n)                BITFIELD_VAL(ACTIVE_CH_NUM, n)
#define POWER_MODE_REG                  (1 << 9)

// REG_03
#define CDC_SAR_P_BIT_IN_DR             (1 << 0)
#define PDAC_MODE_DR                    (1 << 5)
#define PDAC_MODE_REG                   (1 << 6)

// REG_04
#define CH_IDX_1_SAMPLE_POINT_SHIFT     0
#define CH_IDX_1_SAMPLE_POINT_MASK      (0xFF << CH_IDX_1_SAMPLE_POINT_SHIFT)
#define CH_IDX_1_SAMPLE_POINT(n)        BITFIELD_VAL(CH_IDX_1_SAMPLE_POINT, n)
#define CH_IDX_0_SAMPLE_POINT_SHIFT     8
#define CH_IDX_0_SAMPLE_POINT_MASK      (0xFF << CH_IDX_0_SAMPLE_POINT_SHIFT)
#define CH_IDX_0_SAMPLE_POINT(n)        BITFIELD_VAL(CH_IDX_0_SAMPLE_POINT, n)

// REG_05
#define CH_IDX_3_SAMPLE_POINT_SHIFT     0
#define CH_IDX_3_SAMPLE_POINT_MASK      (0xFF << CH_IDX_3_SAMPLE_POINT_SHIFT)
#define CH_IDX_3_SAMPLE_POINT(n)        BITFIELD_VAL(CH_IDX_3_SAMPLE_POINT, n)
#define CH_IDX_2_SAMPLE_POINT_SHIFT     8
#define CH_IDX_2_SAMPLE_POINT_MASK      (0xFF << CH_IDX_2_SAMPLE_POINT_SHIFT)
#define CH_IDX_2_SAMPLE_POINT(n)        BITFIELD_VAL(CH_IDX_2_SAMPLE_POINT, n)

// REG_06
#define CH_IDX_5_SAMPLE_POINT_SHIFT     0
#define CH_IDX_5_SAMPLE_POINT_MASK      (0xFF << CH_IDX_5_SAMPLE_POINT_SHIFT)
#define CH_IDX_5_SAMPLE_POINT(n)        BITFIELD_VAL(CH_IDX_5_SAMPLE_POINT, n)
#define CH_IDX_4_SAMPLE_POINT_SHIFT     8
#define CH_IDX_4_SAMPLE_POINT_MASK      (0xFF << CH_IDX_4_SAMPLE_POINT_SHIFT)
#define CH_IDX_4_SAMPLE_POINT(n)        BITFIELD_VAL(CH_IDX_4_SAMPLE_POINT, n)

// REG_07
#define CH_IDX_7_SAMPLE_POINT_SHIFT     0
#define CH_IDX_7_SAMPLE_POINT_MASK      (0xFF << CH_IDX_7_SAMPLE_POINT_SHIFT)
#define CH_IDX_7_SAMPLE_POINT(n)        BITFIELD_VAL(CH_IDX_7_SAMPLE_POINT, n)
#define CH_IDX_6_SAMPLE_POINT_SHIFT     8
#define CH_IDX_6_SAMPLE_POINT_MASK      (0xFF << CH_IDX_6_SAMPLE_POINT_SHIFT)
#define CH_IDX_6_SAMPLE_POINT(n)        BITFIELD_VAL(CH_IDX_6_SAMPLE_POINT, n)

// REG_0C
#define CDC_BASELINE_P_BIT_IN_CH0_DR    (1 << 10)

// REG_3C
#define LP_SLEEP_TIME_SHIFT             0
#define LP_SLEEP_TIME_MASK              (0x3FFF << LP_SLEEP_TIME_SHIFT)
#define LP_SLEEP_TIME(n)                BITFIELD_VAL(LP_SLEEP_TIME, n)

// REG_3D
#define CDC_BASELINE_N_BIT_IN_CH0_DR    (1 << 6)
#define CDC_BASELINE_N_BIT_IN_CH1_DR    (1 << 7)
#define CDC_BASELINE_N_BIT_IN_CH2_DR    (1 << 8)
#define CDC_BASELINE_N_BIT_IN_CH3_DR    (1 << 9)
#define CDC_BASELINE_N_BIT_IN_CH4_DR    (1 << 10)
#define CDC_BASELINE_N_BIT_IN_CH5_DR    (1 << 11)
#define CDC_BASELINE_N_BIT_IN_CH6_DR    (1 << 12)
#define CDC_BASELINE_N_BIT_IN_CH7_DR    (1 << 13)

// REG_3E
#define FIFO_DATA_NUM_TH_SHIFT          0
#define FIFO_DATA_NUM_TH_MASK           (0xFF << FIFO_DATA_NUM_TH_SHIFT)
#define FIFO_DATA_NUM_TH(n)             BITFIELD_VAL(FIFO_DATA_NUM_TH, n)

// REG_54
#define FIFO_RD_CLK_DR                  (1 << 2)
#define FIFO_CLK_DR                     (1 << 3)
#define FIFO_RD_DIRECTION_REG           (1 << 4)
#define FIFO_RD_START_REG               (1 << 5)

// REG_55
#define CH_CONFIG_IDX_0_SHIFT           0
#define CH_CONFIG_IDX_0_MASK            (0x7F << CH_CONFIG_IDX_0_SHIFT)
#define CH_CONFIG_IDX_0(n)              BITFIELD_VAL(CH_CONFIG_IDX_0, n)
#define CH_CONFIG_IDX_1_SHIFT           8
#define CH_CONFIG_IDX_1_MASK            (0x7F << CH_CONFIG_IDX_1_SHIFT)
#define CH_CONFIG_IDX_1(n)              BITFIELD_VAL(CH_CONFIG_IDX_1, n)

// REG_56
#define CH_CONFIG_IDX_2_SHIFT           0
#define CH_CONFIG_IDX_2_MASK            (0x7F << CH_CONFIG_IDX_2_SHIFT)
#define CH_CONFIG_IDX_2(n)              BITFIELD_VAL(CH_CONFIG_IDX_2, n)
#define CH_CONFIG_IDX_3_SHIFT           8
#define CH_CONFIG_IDX_3_MASK            (0x7F << CH_CONFIG_IDX_1_SHIFT)
#define CH_CONFIG_IDX_3(n)              BITFIELD_VAL(CH_CONFIG_IDX_1, n)

// REG_57
#define CH_CONFIG_IDX_4_SHIFT           0
#define CH_CONFIG_IDX_4_MASK            (0x7F << CH_CONFIG_IDX_4_SHIFT)
#define CH_CONFIG_IDX_4(n)              BITFIELD_VAL(CH_CONFIG_IDX_4, n)
#define CH_CONFIG_IDX_5_SHIFT           8
#define CH_CONFIG_IDX_5_MASK            (0x7F << CH_CONFIG_IDX_5_SHIFT)
#define CH_CONFIG_IDX_5(n)              BITFIELD_VAL(CH_CONFIG_IDX_5, n)

// REG_58
#define CH_CONFIG_IDX_6_SHIFT           0
#define CH_CONFIG_IDX_6_MASK            (0x7F << CH_CONFIG_IDX_6_SHIFT)
#define CH_CONFIG_IDX_6(n)              BITFIELD_VAL(CH_CONFIG_IDX_6, n)
#define CH_CONFIG_IDX_7_SHIFT           8
#define CH_CONFIG_IDX_7_MASK            (0x7F << CH_CONFIG_IDX_7_SHIFT)
#define CH_CONFIG_IDX_7(n)              BITFIELD_VAL(CH_CONFIG_IDX_7, n)

// REG_5F
#define ADC_HBF1_BYPASS_CH0             (1 << 0)
#define ADC_HBF1_BYPASS_CH1             (1 << 1)
#define ADC_HBF1_BYPASS_CH2             (1 << 2)
#define ADC_HBF1_BYPASS_CH3             (1 << 3)
#define ADC_HBF1_BYPASS_CH4             (1 << 4)
#define ADC_HBF1_BYPASS_CH5             (1 << 5)
#define ADC_HBF1_BYPASS_CH6             (1 << 6)
#define ADC_HBF1_BYPASS_CH7             (1 << 7)
#define ADC_HBF2_BYPASS_CH0             (1 << 8)
#define ADC_HBF2_BYPASS_CH1             (1 << 9)
#define ADC_HBF2_BYPASS_CH2             (1 << 10)
#define ADC_HBF2_BYPASS_CH3             (1 << 11)
#define ADC_HBF2_BYPASS_CH4             (1 << 12)
#define ADC_HBF2_BYPASS_CH5             (1 << 13)
#define ADC_HBF2_BYPASS_CH6             (1 << 14)
#define ADC_HBF2_BYPASS_CH7             (1 << 15)

// REG_60
#define DOWN_SEL_CH0_SHIFT              0
#define DOWN_SEL_CH0_MASK               (0x7 << DOWN_SEL_CH0_SHIFT)
#define DOWN_SEL_CH0(n)                 BITFIELD_VAL(DOWN_SEL_CH0, n)
#define DOWN_SEL_CH1_SHIFT              3
#define DOWN_SEL_CH1_MASK               (0x7 << DOWN_SEL_CH1_SHIFT)
#define DOWN_SEL_CH1(n)                 BITFIELD_VAL(DOWN_SEL_CH1, n)
#define DOWN_SEL_CH2_SHIFT              6
#define DOWN_SEL_CH2_MASK               (0x7 << DOWN_SEL_CH2_SHIFT)
#define DOWN_SEL_CH2(n)                 BITFIELD_VAL(DOWN_SEL_CH2, n)
#define DOWN_SEL_CH3_SHIFT              9
#define DOWN_SEL_CH3_MASK               (0x7 << DOWN_SEL_CH3_SHIFT)
#define DOWN_SEL_CH3(n)                 BITFIELD_VAL(DOWN_SEL_CH3, n)
#define DOWN_SEL_CH4_SHIFT              12
#define DOWN_SEL_CH4_MASK               (0x7 << DOWN_SEL_CH4_SHIFT)
#define DOWN_SEL_CH4(n)                 BITFIELD_VAL(DOWN_SEL_CH4, n)
#define FILTER_CONFIG_DR                (1 << 15)

// REG_61
#define DOWN_SEL_CH5_SHIFT              0
#define DOWN_SEL_CH5_MASK               (0x7 << DOWN_SEL_CH5_SHIFT)
#define DOWN_SEL_CH5(n)                 BITFIELD_VAL(DOWN_SEL_CH5, n)
#define DOWN_SEL_CH6_SHIFT              3
#define DOWN_SEL_CH6_MASK               (0x7 << DOWN_SEL_CH6_SHIFT)
#define DOWN_SEL_CH6(n)                 BITFIELD_VAL(DOWN_SEL_CH6, n)
#define DOWN_SEL_CH7_SHIFT              6
#define DOWN_SEL_CH7_MASK               (0x7 << DOWN_SEL_CH7_SHIFT)
#define DOWN_SEL_CH7(n)                 BITFIELD_VAL(DOWN_SEL_CH7, n)

// REG_66
#define CDC_SDM_OP_CIN1_CH0_REG_SHIFT   0
#define CDC_SDM_OP_CIN1_CH0_REG_MASK    (0x7 << CDC_SDM_OP_CIN1_CH0_REG_SHIFT)
#define CDC_SDM_OP_CIN1_CH0_REG(n)      BITFIELD_VAL(CDC_SDM_OP_CIN1_CH0_REG, n)
#define CDC_SDM_OP_CIN1_CH1_REG_SHIFT   3
#define CDC_SDM_OP_CIN1_CH1_REG_MASK    (0x7 << CDC_SDM_OP_CIN1_CH1_REG_SHIFT)
#define CDC_SDM_OP_CIN1_CH1_REG(n)      BITFIELD_VAL(CDC_SDM_OP_CIN1_CH1_REG, n)
#define CDC_SDM_OP_CIN1_CH2_REG_SHIFT   6
#define CDC_SDM_OP_CIN1_CH2_REG_MASK    (0x7 << CDC_SDM_OP_CIN1_CH2_REG_SHIFT)
#define CDC_SDM_OP_CIN1_CH2_REG(n)      BITFIELD_VAL(CDC_SDM_OP_CIN1_CH2_REG, n)
#define CDC_SDM_OP_CIN1_CH3_REG_SHIFT   9
#define CDC_SDM_OP_CIN1_CH3_REG_MASK    (0x7 << CDC_SDM_OP_CIN1_CH3_REG_SHIFT)
#define CDC_SDM_OP_CIN1_CH3_REG(n)      BITFIELD_VAL(CDC_SDM_OP_CIN1_CH3_REG, n)

// REG_67
#define CDC_SDM_OP_CIN1_CH4_REG_SHIFT   0
#define CDC_SDM_OP_CIN1_CH4_REG_MASK    (0x7 << CDC_SDM_OP_CIN1_CH4_REG_SHIFT)
#define CDC_SDM_OP_CIN1_CH4_REG(n)      BITFIELD_VAL(CDC_SDM_OP_CIN1_CH4_REG, n)
#define CDC_SDM_OP_CIN1_CH5_REG_SHIFT   3
#define CDC_SDM_OP_CIN1_CH5_REG_MASK    (0x7 << CDC_SDM_OP_CIN1_CH5_REG_SHIFT)
#define CDC_SDM_OP_CIN1_CH5_REG(n)      BITFIELD_VAL(CDC_SDM_OP_CIN1_CH5_REG, n)
#define CDC_SDM_OP_CIN1_CH6_REG_SHIFT   6
#define CDC_SDM_OP_CIN1_CH6_REG_MASK    (0x7 << CDC_SDM_OP_CIN1_CH6_REG_SHIFT)
#define CDC_SDM_OP_CIN1_CH6_REG(n)      BITFIELD_VAL(CDC_SDM_OP_CIN1_CH6_REG, n)
#define CDC_SDM_OP_CIN1_CH7_REG_SHIFT   9
#define CDC_SDM_OP_CIN1_CH7_REG_MASK    (0x7 << CDC_SDM_OP_CIN1_CH7_REG_SHIFT)
#define CDC_SDM_OP_CIN1_CH7_REG(n)      BITFIELD_VAL(CDC_SDM_OP_CIN1_CH7_REG, n)

// REG_7F
#define CH_SAMPLE_START_SHIFT           0
#define CH_SAMPLE_START_MASK            (0xFF << CH_SAMPLE_START_SHIFT)
#define CH_SAMPLE_START(n)              BITFIELD_VAL(CH_SAMPLE_START, n)
#define CH_SAMPLE_CNT_SEL_SHIFT         8
#define CH_SAMPLE_CNT_SEL_MASK          (0x7 << CH_SAMPLE_CNT_SEL_SHIFT)
#define CH_SAMPLE_CNT_SEL(n)            BITFIELD_VAL(CH_SAMPLE_CNT_SEL, n)
#define CH_SAMPLE_MODE                  (1 << 11)

// REG_80
#define FIFO_TAG_EN                     (1 << 0)
#define FIFO_DATA_SDM_ONLY              (1 << 13)

// REG_82
#define RD_ADDR_PTR_SHIFT               8
#define RD_ADDR_PTR_MASK                (0xFF << RD_ADDR_PTR_SHIFT)
#define RD_ADDR_PTR(n)                  BITFIELD_VAL(RD_ADDR_PTR, n)

// REG_84
#define PU_OSC_CAP_SENSOR_DR            (1 << 4)
#define PU_OSC_CAP_SENSOR_REG           (1 << 5)
#define RD_PTR_DR                       (1 << 8)

// REG_87
#define FP_MODE_RD_INT_RAW_EN_REG       (1 << 4)
#define PRESS_INT_EN_REG                (1 << 5)

// REG_88
#define PRESS_INT_MASK                  (1 << 10)
#define PRESS_INT_CLR_REG               (1 << 11)
#define FP_MODE_RD_INT_MASK             (1 << 14)
#define FP_MODE_RD_INT_CLR_REG          (1 << 15)

// REG_8B
#define CDC_PD_IDLE_CHAN_SHIFT          1
#define CDC_PD_IDLE_CHAN_MASK           (0xFF << CDC_PD_IDLE_CHAN_SHIFT)
#define CDC_PD_IDLE_CHAN(n)             BITFIELD_VAL(CDC_PD_IDLE_CHAN, n)

// REG_8C
#define SAR_OVERFLOW_DR                 (1 << 1)
#define FIFO_SAR_DATA_SEL               (1 << 3)
#define CH_SCAN_CNT_MATCHED_REG_SHIFT   4
#define CH_SCAN_CNT_MATCHED_REG_MASK    (0xF << CH_SCAN_CNT_MATCHED_REG_SHIFT)
#define CH_SCAN_CNT_MATCHED_REG(n)      BITFIELD_VAL(CH_SCAN_CNT_MATCHED_REG, n)
#define CDC_CK_DELAY_SHIFT              14
#define CDC_CK_DELAY_MASK               (0x3 << CDC_CK_DELAY_SHIFT)
#define CDC_CK_DELAY(n)                 BITFIELD_VAL(CDC_CK_DELAY, n)

// REG_91
#define RESETN_FILTER_REG_DR            (1 << 0)
#define RESETN_FIFO_REG                 (1 << 1)
#define RESETN_FSM_CTRL_REG             (1 << 2)
#define RESETN_CH_CTRL_REG              (1 << 3)
#define RESETN_FILTER_OUT_REG           (1 << 4)

#ifdef __cplusplus
}
#endif

#endif // __REG_CAPSENSOR_BEST1502P_H__
