/***************************************************************************
 *
 * Copyright 2015-2020 BES.
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
#ifndef __REG_USBPHY_BEST1501_H__
#define __REG_USBPHY_BEST1501_H__

#include "plat_types.h"

// REG_00
#define REVID_SHIFT                         0
#define REVID_MASK                          (0xF << REVID_SHIFT)
#define REVID(n)                            BITFIELD_VAL(REVID, n)
#define CHIPID_SHIFT                        4
#define CHIPID_MASK                         (0xFFF << CHIPID_SHIFT)
#define CHIPID(n)                           BITFIELD_VAL(CHIPID, n)

// REG_01
#define CFG_ANAPHY_RESETN                   (1 << 0)
#define CFG_CKCDR_EN                        (1 << 1)
#define CFG_CKPLL_EN                        (1 << 2)
#define CFG_EN_CLKMAC                       (1 << 3)
#define CFG_POL_CDRCLK                      (1 << 4)
#define CFG_RESETN_MAC                      (1 << 5)
#define CFG_RESETNCDR                       (1 << 6)
#define CFG_RESETNPLL                       (1 << 7)
#define CFG_BYPASS_CDR_ENDRESETN            (1 << 8)
#define CFG_INTR_EN_ALL                     (1 << 9)
#define CFG_LOW_SPEED_MODE                  (1 << 10)
#define CFG_POL_CLKPLL                      (1 << 11)
#define CFG_BYPASS_RDY                      (1 << 12)
#define CFG_MODE_HS_LINES_TX                (1 << 13)
#define CFG_MODE_FS_LINES_TX                (1 << 14)
#define CFG_ENUM_MODE                       (1 << 15)

// REG_02
#define CFG_TARGET_SYNC_TOUT_SHIFT          0
#define CFG_TARGET_SYNC_TOUT_MASK           (0xF << CFG_TARGET_SYNC_TOUT_SHIFT)
#define CFG_TARGET_SYNC_TOUT(n)             BITFIELD_VAL(CFG_TARGET_SYNC_TOUT, n)
#define CFG_SEL_SYNCPATTERN_SHIFT           4
#define CFG_SEL_SYNCPATTERN_MASK            (0x3 << CFG_SEL_SYNCPATTERN_SHIFT)
#define CFG_SEL_SYNCPATTERN(n)              BITFIELD_VAL(CFG_SEL_SYNCPATTERN, n)
#define CFG_BYPASS_SQL_VALID                (1 << 6)
#define CFG_EN_SYNCTOUT                     (1 << 7)
#define CFG_FORCERX                         (1 << 8)
#define CFG_RXRESET                         (1 << 9)
#define CFG_EBUF_THRD_SHIFT                 10
#define CFG_EBUF_THRD_MASK                  (0x1F << CFG_EBUF_THRD_SHIFT)
#define CFG_EBUF_THRD(n)                    BITFIELD_VAL(CFG_EBUF_THRD, n)
#define CFG_EOP_MODE_RX                     (1 << 15)

// REG_03
#define CFG_TARGET_TAIL_SHIFT               0
#define CFG_TARGET_TAIL_MASK                (0xFF << CFG_TARGET_TAIL_SHIFT)
#define CFG_TARGET_TAIL(n)                  BITFIELD_VAL(CFG_TARGET_TAIL, n)
#define CFG_TXPATTERN_SHIFT                 8
#define CFG_TXPATTERN_MASK                  (0xFF << CFG_TXPATTERN_SHIFT)
#define CFG_TXPATTERN(n)                    BITFIELD_VAL(CFG_TXPATTERN, n)

// REG_04
#define CFG_EN_HSTSOF                       (1 << 0)
#define CFG_FORCETX                         (1 << 1)
#define CFG_MODE_BITEN                      (1 << 2)
#define CFG_TXSTATE_RESET                   (1 << 3)
#define CFG_EMPTY_DLY_SEL_SHIFT             4
#define CFG_EMPTY_DLY_SEL_MASK              (0x7 << CFG_EMPTY_DLY_SEL_SHIFT)
#define CFG_EMPTY_DLY_SEL(n)                BITFIELD_VAL(CFG_EMPTY_DLY_SEL, n)
#define RESERVED_0                          (1 << 7)
#define CFG_TP_SEL_SHIFT                    8
#define CFG_TP_SEL_MASK                     (0xFF << CFG_TP_SEL_SHIFT)
#define CFG_TP_SEL(n)                       BITFIELD_VAL(CFG_TP_SEL, n)

// REG_05
#define CFG_ADP_PRB                         (1 << 0)
#define CFG_ADP_SNS                         (1 << 1)
#define CFG_RID_A                           (1 << 2)
#define CFG_RID_B                           (1 << 3)
#define CFG_RID_C                           (1 << 4)
#define CFG_RID_FLOAT                       (1 << 5)
#define CFG_RID_GND                         (1 << 6)
#define CFG_UTMIOTG_AVALID                  (1 << 7)
#define CFG_UTMISRP_BVALID                  (1 << 8)
#define CFG_UTMIOTG_VBUSVALID               (1 << 9)
#define CFG_UTMISRP_SESSEND                 (1 << 10)
#define RESERVED_5_1_SHIFT                  11
#define RESERVED_5_1_MASK                   (0x1F << RESERVED_5_1_SHIFT)
#define RESERVED_5_1(n)                     BITFIELD_VAL(RESERVED_5_1, n)

// REG_06
#define CFG_HS_SRC_SEL                      (1 << 0)
#define CFG_HS_TX_EN                        (1 << 1)
#define CFG_CHIRP_EN                        (1 << 2)
#define CFG_HS_MODE                         (1 << 3)
#define CFG_EN_RHS                          (1 << 4)
#define CFG_HS_DRV_SEL_SHIFT                5
#define CFG_HS_DRV_SEL_MASK                 (0xF << CFG_HS_DRV_SEL_SHIFT)
#define CFG_HS_DRV_SEL(n)                   BITFIELD_VAL(CFG_HS_DRV_SEL, n)
#define CFG_RHS_TRIM_SHIFT                  9
#define CFG_RHS_TRIM_MASK                   (0xF << CFG_RHS_TRIM_SHIFT)
#define CFG_RHS_TRIM(n)                     BITFIELD_VAL(CFG_RHS_TRIM, n)
#define CFG_HS_RCV_PD                       (1 << 13)
#define CFG_EN_HOLD_LAST                    (1 << 14)
#define CFG_EN_HS_S2P                       (1 << 15)

// REG_07
#define CFG_RST_INTP                        (1 << 0)
#define CFG_EN_ZPS                          (1 << 1)
#define CFG_RES_GN_SHIFT                    2
#define CFG_RES_GN_MASK                     (0xF << CFG_RES_GN_SHIFT)
#define CFG_RES_GN(n)                       BITFIELD_VAL(CFG_RES_GN, n)
#define CFG_ISEL_RCV_SHIFT                  6
#define CFG_ISEL_RCV_MASK                   (0x7 << CFG_ISEL_RCV_SHIFT)
#define CFG_ISEL_RCV(n)                     BITFIELD_VAL(CFG_ISEL_RCV, n)
#define CFG_ISEL_SQ_SHIFT                   9
#define CFG_ISEL_SQ_MASK                    (0x7 << CFG_ISEL_SQ_SHIFT)
#define CFG_ISEL_SQ(n)                      BITFIELD_VAL(CFG_ISEL_SQ, n)
#define CFG_INT_FLT_SHIFT                   12
#define CFG_INT_FLT_MASK                    (0x3 << CFG_INT_FLT_SHIFT)
#define CFG_INT_FLT(n)                      BITFIELD_VAL(CFG_INT_FLT, n)
#define CFG_PI_GN_SHIFT                     14
#define CFG_PI_GN_MASK                      (0x3 << CFG_PI_GN_SHIFT)
#define CFG_PI_GN(n)                        BITFIELD_VAL(CFG_PI_GN, n)

// REG_08
#define CFG_CDR_GN_SHIFT                    0
#define CFG_CDR_GN_MASK                     (0x3 << CFG_CDR_GN_SHIFT)
#define CFG_CDR_GN(n)                       BITFIELD_VAL(CFG_CDR_GN, n)
#define CFG_SEL_HS_GAIN                     (1 << 2)
#define CFG_SEL_TERR                        (1 << 3)
#define CFG_SEL_HS_LSTATE                   (1 << 4)
#define CFG_LOOP_BACK                       (1 << 5)
#define CFG_XCVR_SELECT                     (1 << 6)
#define CFG_XTERM_SELECT                    (1 << 7)
#define CFG_USPENDM                         (1 << 8)
#define CFG_DISCON_DET_EN                   (1 << 9)
#define CFG_DISCON_VTHSET_SHIFT             10
#define CFG_DISCON_VTHSET_MASK              (0x7 << CFG_DISCON_VTHSET_SHIFT)
#define CFG_DISCON_VTHSET(n)                BITFIELD_VAL(CFG_DISCON_VTHSET, n)
#define CFG_FS_TX_ON                        (1 << 13)
#define CFG_FS_EN_RFS                       (1 << 14)
#define CFG_FS_EN_R15K                      (1 << 15)

// REG_09
#define CFG_LS_EN_RLS                       (1 << 0)
#define CFG_LS_MODE                         (1 << 1)
#define CFG_ATEST_EN_REG                    (1 << 2)
#define CFG_ATEST_SELX_REG_SHIFT            3
#define CFG_ATEST_SELX_REG_MASK             (0x3 << CFG_ATEST_SELX_REG_SHIFT)
#define CFG_ATEST_SELX_REG(n)               BITFIELD_VAL(CFG_ATEST_SELX_REG, n)
#define CFG_DTESTEN1_REG                    (1 << 5)
#define CFG_DTESTEN2_REG                    (1 << 6)
#define CFG_DTEST_SEL_REG_SHIFT             7
#define CFG_DTEST_SEL_REG_MASK              (0x3 << CFG_DTEST_SEL_REG_SHIFT)
#define CFG_DTEST_SEL_REG(n)                BITFIELD_VAL(CFG_DTEST_SEL_REG, n)
#define CFG_ATEST_EN_DISCON_REG             (1 << 9)
#define CFG_ATEST_SELX_DISCON_REG           (1 << 10)
#define CFG_MODE_LINESTATE                  (1 << 11)
#define RESERVED_8_6_SHIFT                  12
#define RESERVED_8_6_MASK                   (0x7 << RESERVED_8_6_SHIFT)
#define RESERVED_8_6(n)                     BITFIELD_VAL(RESERVED_8_6, n)
#define CFG_BISTEN                          (1 << 15)

// REG_0A
#define CFG_DR_HOSTMODE                     (1 << 0)
#define CFG_DR_FSLS_SEL                     (1 << 1)
#define CFG_DR_SDATA                        (1 << 2)
#define CFG_DR_TERMSEL                      (1 << 3)
#define CFG_DR_XCVRSEL                      (1 << 4)
#define CFG_DR_SON                          (1 << 5)
#define CFG_DR_OPMODE                       (1 << 6)
#define RESERVED_9                          (1 << 7)
#define CFG_DR_HS_TXEN                      (1 << 8)
#define CFG_DR_FSTXEN                       (1 << 9)
#define CFG_DR_FSTXD                        (1 << 10)
#define CFG_DR_FSTXDB                       (1 << 11)
#define CFG_FS_LS_SEL                       (1 << 12)
#define CFG_CLK480M_EDGE_SEL                (1 << 13)
#define CFG_FS_DM_DP_BYPASS                 (1 << 14)

// REG_0B
#define CFG_REG_HOSTMODE                    (1 << 0)
#define CFG_REG_FSLS_SEL                    (1 << 1)
#define CFG_REG_SDATA                       (1 << 2)
#define CFG_REG_TERM                        (1 << 3)
#define CFG_REG_XCVRSEL                     (1 << 4)
#define CFG_REG_SON                         (1 << 5)
#define CFG_REG_OPMODE_SHIFT                6
#define CFG_REG_OPMODE_MASK                 (0x3 << CFG_REG_OPMODE_SHIFT)
#define CFG_REG_OPMODE(n)                   BITFIELD_VAL(CFG_REG_OPMODE, n)
#define CFG_REG_HS_TXEN                     (1 << 8)
#define CFG_REG_FSTXEN                      (1 << 9)
#define CFG_REG_FSTXD                       (1 << 10)
#define CFG_REG_FSTXDB                      (1 << 11)
#define RESERVED_17_14_SHIFT                12
#define RESERVED_17_14_MASK                 (0xF << RESERVED_17_14_SHIFT)
#define RESERVED_17_14(n)                   BITFIELD_VAL(RESERVED_17_14, n)

// REG_0C
#define CFG_RXINTR_MSK_SHIFT                0
#define CFG_RXINTR_MSK_MASK                 (0xFFFF << CFG_RXINTR_MSK_SHIFT)
#define CFG_RXINTR_MSK(n)                   BITFIELD_VAL(CFG_RXINTR_MSK, n)

// REG_0D
#define CFG_HSHOST_DISC_MSK                 (1 << 0)
#define CFG_POL_CKSIE60                     (1 << 1)
#define CFG_RESETN_HSTXP                    (1 << 2)
#define CFG_CKSIE60_EN                      (1 << 3)
#define CFG_TXP_MODE                        (1 << 4)
#define CFG_CKOSC_EN                        (1 << 5)
#define CFG_POL_OSC                         (1 << 6)
#define CFG_CKPRE_EN                        (1 << 7)
#define CFG_RX_P_SEL                        (1 << 8)
#define CFG_RERESTN_HSRXP                   (1 << 9)
#define CFG_POL_RXP                         (1 << 10)
#define CFG_CKRXP_EN                        (1 << 11)
#define CFG_PEBUF_THRD_SHIFT                12
#define CFG_PEBUF_THRD_MASK                 (0xF << CFG_PEBUF_THRD_SHIFT)
#define CFG_PEBUF_THRD(n)                   BITFIELD_VAL(CFG_PEBUF_THRD, n)

// REG_0E
#define RB_RXERRSTATUS_RAW_SHIFT            0
#define RB_RXERRSTATUS_RAW_MASK             (0xFFFF << RB_RXERRSTATUS_RAW_SHIFT)
#define RB_RXERRSTATUS_RAW(n)               BITFIELD_VAL(RB_RXERRSTATUS_RAW, n)

// REG_0F
#define RB_RXERRSTATUS_MSKD_SHIFT           0
#define RB_RXERRSTATUS_MSKD_MASK            (0xFFFF << RB_RXERRSTATUS_MSKD_SHIFT)
#define RB_RXERRSTATUS_MSKD(n)              BITFIELD_VAL(RB_RXERRSTATUS_MSKD, n)

// REG_10
#define RB_HSHOST_DISC_RAW                  (1 << 0)

// REG_11
#define RB_HSHOST_DISC_MSKED                (1 << 0)

// REG_12
#define CFG_EN_SETERR_RX_SHIFT              0
#define CFG_EN_SETERR_RX_MASK               (0xFFFF << CFG_EN_SETERR_RX_SHIFT)
#define CFG_EN_SETERR_RX(n)                 BITFIELD_VAL(CFG_EN_SETERR_RX, n)

// REG_13
#define CFG_TXSTART_DLY_SEL_SHIFT           0
#define CFG_TXSTART_DLY_SEL_MASK            (0x7 << CFG_TXSTART_DLY_SEL_SHIFT)
#define CFG_TXSTART_DLY_SEL(n)              BITFIELD_VAL(CFG_TXSTART_DLY_SEL, n)

// REG_14
#define CALIB_TIME_15_0_SHIFT               0
#define CALIB_TIME_15_0_MASK                (0xFFFF << CALIB_TIME_15_0_SHIFT)
#define CALIB_TIME_15_0(n)                  BITFIELD_VAL(CALIB_TIME_15_0, n)

// REG_15
#define CALIB_TIME_19_16_SHIFT              0
#define CALIB_TIME_19_16_MASK               (0xF << CALIB_TIME_19_16_SHIFT)
#define CALIB_TIME_19_16(n)                 BITFIELD_VAL(CALIB_TIME_19_16, n)
#define RESERVED_49_39_SHIFT                4
#define RESERVED_49_39_MASK                 (0x7FF << RESERVED_49_39_SHIFT)
#define RESERVED_49_39(n)                   BITFIELD_VAL(RESERVED_49_39, n)
#define CALIB_START                         (1 << 15)

// REG_16
#define INTR_MASK_SHIFT                     0
#define INTR_MASK_MASK                      (0x3 << INTR_MASK_SHIFT)
#define INTR_MASK(n)                        BITFIELD_VAL(INTR_MASK, n)
#define RESERVED_63_50_SHIFT                2
#define RESERVED_63_50_MASK                 (0x3FFF << RESERVED_63_50_SHIFT)
#define RESERVED_63_50(n)                   BITFIELD_VAL(RESERVED_63_50, n)

// REG_17
#define O_CNT_PLL_15_0_SHIFT                0
#define O_CNT_PLL_15_0_MASK                 (0xFFFF << O_CNT_PLL_15_0_SHIFT)
#define O_CNT_PLL_15_0(n)                   BITFIELD_VAL(O_CNT_PLL_15_0, n)

// REG_18
#define O_CNT_PLL_24_16_SHIFT               0
#define O_CNT_PLL_24_16_MASK                (0x1FF << O_CNT_PLL_24_16_SHIFT)
#define O_CNT_PLL_24_16(n)                  BITFIELD_VAL(O_CNT_PLL_24_16, n)

// REG_19
#define O_INTR_STATUS_SHIFT                 0
#define O_INTR_STATUS_MASK                  (0x3 << O_INTR_STATUS_SHIFT)
#define O_INTR_STATUS(n)                    BITFIELD_VAL(O_INTR_STATUS, n)

// REG_1A
#define HS_TEST                             (1 << 0)
#define FS_TEST                             (1 << 1)
#define USBINSERT_DET_EN                    (1 << 2)
#define USBINSERT_INTR_EN                   (1 << 3)
#define INTR_CLR                            (1 << 4)
#define POL_USB_RX_DP                       (1 << 5)
#define POL_USB_RX_DM                       (1 << 6)
#define DEBOUNCE_EN                         (1 << 7)
#define NOLS_MODE                           (1 << 8)
#define USB_INSERT_INTR_MSK                 (1 << 9)
#define PHY_TEST_GOON                       (1 << 10)

// REG_1B
#define CFG_HS_PDATA_SHIFT                  0
#define CFG_HS_PDATA_MASK                   (0xFF << CFG_HS_PDATA_SHIFT)
#define CFG_HS_PDATA(n)                     BITFIELD_VAL(CFG_HS_PDATA, n)
#define CFG_HS_P_ON_SHIFT                   8
#define CFG_HS_P_ON_MASK                    (0xFF << CFG_HS_P_ON_SHIFT)
#define CFG_HS_P_ON(n)                      BITFIELD_VAL(CFG_HS_P_ON, n)

// REG_20
#define RB_BISTERR_CAUSE_SHIFT              0
#define RB_BISTERR_CAUSE_MASK               (0x7 << RB_BISTERR_CAUSE_SHIFT)
#define RB_BISTERR_CAUSE(n)                 BITFIELD_VAL(RB_BISTERR_CAUSE, n)

// REG_21
#define RB_BIST_FAIL                        (1 << 0)
#define RB_BIST_DONE                        (1 << 1)
#define RB_CHIRP_ON                         (1 << 2)
#define RB_ADP_SNS_EN                       (1 << 3)
#define RB_ADP_PRB_EN                       (1 << 4)
#define RB_ADP_DISCHRG                      (1 << 5)
#define RB_ADP_CHRG                         (1 << 6)
#define RB_UTMISRP_DISCHRGVBUS              (1 << 7)
#define RB_UTMISRP_CHRGVBUS                 (1 << 8)
#define RB_UTMIOTG_IDPULLUP                 (1 << 9)
#define RB_UTMIOTG_DPPULLDOWN               (1 << 10)
#define RB_UTMIOTG_DMPULLDOWN               (1 << 11)
#define RB_UTMIOTG_DRVVBUS                  (1 << 12)

// REG_22
#define CFG_RESETNTX                        (1 << 0)
#define CFG_RESETNRX                        (1 << 1)
#define CFG_CKTX_EN                         (1 << 2)
#define CFG_CKRX_EN                         (1 << 3)
#define CFG_FORCE_TXCK                      (1 << 4)
#define CFG_FORCE_RXCK                      (1 << 5)
#define CFG_FORCE_CDRCK                     (1 << 6)
#define CFG_MODE_S_ON                       (1 << 7)
#define CFG_RESETNFRE                       (1 << 8)
#define CFG_RESETNOSC                       (1 << 9)
#define CFG_DIG_LOOP                        (1 << 10)
#define CFG_FORCE_CDRPCK                    (1 << 11)
#define CFG_DIG_SLOOP                       (1 << 12)

// REG_23
#define O_CNT_OSC_15_0_SHIFT                0
#define O_CNT_OSC_15_0_MASK                 (0xFFFF << O_CNT_OSC_15_0_SHIFT)
#define O_CNT_OSC_15_0(n)                   BITFIELD_VAL(O_CNT_OSC_15_0, n)

// REG_24
#define O_CNT_OSC_19_16_SHIFT               0
#define O_CNT_OSC_19_16_MASK                (0x1F << O_CNT_OSC_19_16_SHIFT)
#define O_CNT_OSC_19_16(n)                  BITFIELD_VAL(O_CNT_OSC_19_16, n)

// REG_25
#define USB_STATUS_RX_DP                    (1 << 0)
#define USB_STATUS_RX_DM                    (1 << 1)

// REG_30
#define REV_REG30_SHIFT                     0
#define REV_REG30_MASK                      (0xFFFF << REV_REG30_SHIFT)
#define REV_REG30(n)                        BITFIELD_VAL(REV_REG30, n)

// REG_31
#define REV_REG31_SHIFT                     0
#define REV_REG31_MASK                      (0xFFFF << REV_REG31_SHIFT)
#define REV_REG31(n)                        BITFIELD_VAL(REV_REG31, n)

// REG_32
#define REV_REG32_SHIFT                     0
#define REV_REG32_MASK                      (0xFFFF << REV_REG32_SHIFT)
#define REV_REG32(n)                        BITFIELD_VAL(REV_REG32, n)

// REG_33
#define CFG_ANA_REV_SHIFT                   0
#define CFG_ANA_REV_MASK                    (0xFFFF << CFG_ANA_REV_SHIFT)
#define CFG_ANA_REV(n)                      BITFIELD_VAL(CFG_ANA_REV, n)

// REG_40
#define USB_SOF__NUM_SHIFT                  0
#define USB_SOF__NUM_MASK                   (0xFFFF << USB_SOF__NUM_SHIFT)
#define USB_SOF__NUM(n)                     BITFIELD_VAL(USB_SOF__NUM, n)

// REG_41
#define USB_SOF_DETECT_EN                   (1 << 0)
#define USB_SOF_INTR_MASK                   (1 << 1)
#define USB_SOF_INTR_CLR                    (1 << 2)
#define USB_SOF_FRM_NUM_CLR                 (1 << 3)
#define USB_SOF_TOGGLE_EN                   (1 << 4)
#define USB_SOF_HALT                        (1 << 5)

// REG_42
#define USB_SOF_START_NUM_SHIFT             0
#define USB_SOF_START_NUM_MASK              (0x3FFF << USB_SOF_START_NUM_SHIFT)
#define USB_SOF_START_NUM(n)                BITFIELD_VAL(USB_SOF_START_NUM, n)

// REG_43
#define USB_SOF_END_NUM_SHIFT               0
#define USB_SOF_END_NUM_MASK                (0x3FFF << USB_SOF_END_NUM_SHIFT)
#define USB_SOF_END_NUM(n)                  BITFIELD_VAL(USB_SOF_END_NUM, n)

// REG_44
#define USB_SOF_CAL_COUNT_15_0_SHIFT        0
#define USB_SOF_CAL_COUNT_15_0_MASK         (0xFFFF << USB_SOF_CAL_COUNT_15_0_SHIFT)
#define USB_SOF_CAL_COUNT_15_0(n)           BITFIELD_VAL(USB_SOF_CAL_COUNT_15_0, n)

// REG_45
#define USB_SOF_CAL_COUNT_27_16_SHIFT       0
#define USB_SOF_CAL_COUNT_27_16_MASK        (0xFFF << USB_SOF_CAL_COUNT_27_16_SHIFT)
#define USB_SOF_CAL_COUNT_27_16(n)          BITFIELD_VAL(USB_SOF_CAL_COUNT_27_16, n)
#define USB_SOF_INTR                        (1 << 15)

// REG_46
#define USB_EMPHASIS_EN                     (1 << 0)
#define USB_EM_SEL_SHIFT                    1
#define USB_EM_SEL_MASK                     (0x7 << USB_EM_SEL_SHIFT)
#define USB_EM_SEL(n)                       BITFIELD_VAL(USB_EM_SEL, n)
#define USB_DCC_BPS                         (1 << 4)
#define USB_DCC_SEL_SHIFT                   5
#define USB_DCC_SEL_MASK                    (0xF << USB_DCC_SEL_SHIFT)
#define USB_DCC_SEL(n)                      BITFIELD_VAL(USB_DCC_SEL, n)
#define USB_CK60M_EN                        (1 << 9)
#define USB_CK480M_EN                       (1 << 10)
#define USB_CKIN4_EN                        (1 << 11)
#define USB_FS_RCV_PD                       (1 << 12)
#define USB_HS_TX_PD                        (1 << 13)

// REG_47
#define USB_FS_VREF_H_SHIFT                 0
#define USB_FS_VREF_H_MASK                  (0x7 << USB_FS_VREF_H_SHIFT)
#define USB_FS_VREF_H(n)                    BITFIELD_VAL(USB_FS_VREF_H, n)
#define USB_FS_VREF_L_SHIFT                 3
#define USB_FS_VREF_L_MASK                  (0x7 << USB_FS_VREF_L_SHIFT)
#define USB_FS_VREF_L(n)                    BITFIELD_VAL(USB_FS_VREF_L, n)

// REG_51
#define REG_ILDO_DCO_DVDD_SHIFT             0
#define REG_ILDO_DCO_DVDD_MASK              (0x3 << REG_ILDO_DCO_DVDD_SHIFT)
#define REG_ILDO_DCO_DVDD(n)                BITFIELD_VAL(REG_ILDO_DCO_DVDD, n)
#define REG_LDO_VRES_SHIFT                  2
#define REG_LDO_VRES_MASK                   (0xF << REG_LDO_VRES_SHIFT)
#define REG_LDO_VRES(n)                     BITFIELD_VAL(REG_LDO_VRES, n)
#define REG_IPTAT_RES_SHIFT                 6
#define REG_IPTAT_RES_MASK                  (0xF << REG_IPTAT_RES_SHIFT)
#define REG_IPTAT_RES(n)                    BITFIELD_VAL(REG_IPTAT_RES, n)
#define REG_ILDO_DIV_SHIFT                  10
#define REG_ILDO_DIV_MASK                   (0xF << REG_ILDO_DIV_SHIFT)
#define REG_ILDO_DIV(n)                     BITFIELD_VAL(REG_ILDO_DIV, n)

// REG_52
#define REG_ILDO_TDC_SHIFT                  0
#define REG_ILDO_TDC_MASK                   (0xF << REG_ILDO_TDC_SHIFT)
#define REG_ILDO_TDC(n)                     BITFIELD_VAL(REG_ILDO_TDC, n)
#define REG_ILDO_DCO_SHIFT                  4
#define REG_ILDO_DCO_MASK                   (0xF << REG_ILDO_DCO_SHIFT)
#define REG_ILDO_DCO(n)                     BITFIELD_VAL(REG_ILDO_DCO, n)
#define REG_ILDO_DIG_SHIFT                  8
#define REG_ILDO_DIG_MASK                   (0xF << REG_ILDO_DIG_SHIFT)
#define REG_ILDO_DIG(n)                     BITFIELD_VAL(REG_ILDO_DIG, n)

// REG_53
#define REG_FREF_SEL                        (1 << 0)
#define REG_PFD_SIGN_FLIP                   (1 << 1)
#define REG_DLY_LONG                        (1 << 2)
#define REG_PFD_SAMPLE_DLY_SHIFT            3
#define REG_PFD_SAMPLE_DLY_MASK             (0x3 << REG_PFD_SAMPLE_DLY_SHIFT)
#define REG_PFD_SAMPLE_DLY(n)               BITFIELD_VAL(REG_PFD_SAMPLE_DLY, n)
#define REG_PFD_RSTB_IN                     (1 << 5)
#define REG_PFD_RSTB_SEL                    (1 << 6)
#define REG_EN_TDC_CLIPCTRL                 (1 << 7)

// REG_54
#define REG_PU_TDC                          (1 << 0)
#define REG_TDC_SWRC_SHIFT                  1
#define REG_TDC_SWRC_MASK                   (0x3 << REG_TDC_SWRC_SHIFT)
#define REG_TDC_SWRC(n)                     BITFIELD_VAL(REG_TDC_SWRC, n)
#define REG_TDC_GAIN_CAL                    (1 << 3)
#define REG_TDC_RE_EDGE_SEL                 (1 << 4)
#define REG_SEL_CHAN_DR_SHIFT               5
#define REG_SEL_CHAN_DR_MASK                (0x3 << REG_SEL_CHAN_DR_SHIFT)
#define REG_SEL_CHAN_DR(n)                  BITFIELD_VAL(REG_SEL_CHAN_DR, n)
#define TDC_SEL_CHAN                        (1 << 7)
#define TDC_CNT2_SHIFT                      8
#define TDC_CNT2_MASK                       (0x7F << TDC_CNT2_SHIFT)
#define TDC_CNT2(n)                         BITFIELD_VAL(TDC_CNT2, n)
#define TDC_SIGN                            (1 << 15)

// REG_55
#define REG_TDC_COMB_TSFST_SHIFT            0
#define REG_TDC_COMB_TSFST_MASK             (0x3FF << REG_TDC_COMB_TSFST_SHIFT)
#define REG_TDC_COMB_TSFST(n)               BITFIELD_VAL(REG_TDC_COMB_TSFST, n)
#define REG_COMP_DELAY_NUM_SHIFT            10
#define REG_COMP_DELAY_NUM_MASK             (0x3 << REG_COMP_DELAY_NUM_SHIFT)
#define REG_COMP_DELAY_NUM(n)               BITFIELD_VAL(REG_COMP_DELAY_NUM, n)
#define REG_EN_OVER_THR                     (1 << 12)
#define BBPLL_LOCK                          (1 << 14)
#define DWC_OUT_SAT                         (1 << 15)

// REG_56
#define REG_DLF_Z0_SHIFT                    0
#define REG_DLF_Z0_MASK                     (0x7FF << REG_DLF_Z0_SHIFT)
#define REG_DLF_Z0(n)                       BITFIELD_VAL(REG_DLF_Z0, n)
#define REG_DLF_RSTN                        (1 << 11)

// REG_57
#define REG_DLF_P1_SHIFT                    0
#define REG_DLF_P1_MASK                     (0x7FF << REG_DLF_P1_SHIFT)
#define REG_DLF_P1(n)                       BITFIELD_VAL(REG_DLF_P1, n)

// REG_58
#define REG_DLF_P2_SHIFT                    0
#define REG_DLF_P2_MASK                     (0x7FF << REG_DLF_P2_SHIFT)
#define REG_DLF_P2(n)                       BITFIELD_VAL(REG_DLF_P2, n)

// REG_59
#define REG_DLF_G0_15_0_SHIFT               0
#define REG_DLF_G0_15_0_MASK                (0xFFFF << REG_DLF_G0_15_0_SHIFT)
#define REG_DLF_G0_15_0(n)                  BITFIELD_VAL(REG_DLF_G0_15_0, n)

// REG_5A
#define REG_DLF_G0_18_16_SHIFT              0
#define REG_DLF_G0_18_16_MASK               (0x7 << REG_DLF_G0_18_16_SHIFT)
#define REG_DLF_G0_18_16(n)                 BITFIELD_VAL(REG_DLF_G0_18_16, n)
#define TDC_COMB_SHIFT                      4
#define TDC_COMB_MASK                       (0xFFF << TDC_COMB_SHIFT)
#define TDC_COMB(n)                         BITFIELD_VAL(TDC_COMB, n)

// REG_5B
#define REG_PU_DCO                          (1 << 0)
#define REG_DCO_SWRC_SHIFT                  1
#define REG_DCO_SWRC_MASK                   (0xF << REG_DCO_SWRC_SHIFT)
#define REG_DCO_SWRC(n)                     BITFIELD_VAL(REG_DCO_SWRC, n)
#define REG_CBIT_DCO_SHIFT                  5
#define REG_CBIT_DCO_MASK                   (0x3F << REG_CBIT_DCO_SHIFT)
#define REG_CBIT_DCO(n)                     BITFIELD_VAL(REG_CBIT_DCO, n)
#define REG_DCO_DSWRC_SHIFT                 11
#define REG_DCO_DSWRC_MASK                  (0x3 << REG_DCO_DSWRC_SHIFT)
#define REG_DCO_DSWRC(n)                    BITFIELD_VAL(REG_DCO_DSWRC, n)

// REG_5C
#define REG_PU_DIV                          (1 << 0)
#define REG_DIV_SWRC_SHIFT                  1
#define REG_DIV_SWRC_MASK                   (0x3 << REG_DIV_SWRC_SHIFT)
#define REG_DIV_SWRC(n)                     BITFIELD_VAL(REG_DIV_SWRC, n)
#define REG_DIV_DELAY_CAP_SHIFT             3
#define REG_DIV_DELAY_CAP_MASK              (0xF << REG_DIV_DELAY_CAP_SHIFT)
#define REG_DIV_DELAY_CAP(n)                BITFIELD_VAL(REG_DIV_DELAY_CAP, n)
#define REG_DIV_SEL_SYNCLK_SHIFT            7
#define REG_DIV_SEL_SYNCLK_MASK             (0x7 << REG_DIV_SEL_SYNCLK_SHIFT)
#define REG_DIV_SEL_SYNCLK(n)               BITFIELD_VAL(REG_DIV_SEL_SYNCLK, n)
#define REG_DIV_CAL_EN                      (1 << 10)
#define REG_SDM_CLK_SEL                     (1 << 11)
#define REG_BYPASS_ZPS                      (1 << 12)
#define REG_REFCLK_EDGE_SW                  (1 << 13)
#define REG_PLL_CNT_EN                      (1 << 14)
#define REG_DIV_SIMB                        (1 << 15)

// REG_5D
#define REG_FORCE_LK                        (1 << 0)
#define REG_LOAD_SDM_OUT_HI_SHIFT           1
#define REG_LOAD_SDM_OUT_HI_MASK            (0xF << REG_LOAD_SDM_OUT_HI_SHIFT)
#define REG_LOAD_SDM_OUT_HI(n)              BITFIELD_VAL(REG_LOAD_SDM_OUT_HI, n)
#define REG_LOAD_SDM_OUT_LO_SHIFT           5
#define REG_LOAD_SDM_OUT_LO_MASK            (0x3F << REG_LOAD_SDM_OUT_LO_SHIFT)
#define REG_LOAD_SDM_OUT_LO(n)              BITFIELD_VAL(REG_LOAD_SDM_OUT_LO, n)

// REG_5E
#define REG_LD_COM_DTH_SHIFT                0
#define REG_LD_COM_DTH_MASK                 (0xFF << REG_LD_COM_DTH_SHIFT)
#define REG_LD_COM_DTH(n)                   BITFIELD_VAL(REG_LD_COM_DTH, n)
#define DLF_OUT_SHIFT                       9
#define DLF_OUT_MASK                        (0x7F << DLF_OUT_SHIFT)
#define DLF_OUT(n)                          BITFIELD_VAL(DLF_OUT, n)

// REG_5F
#define REG_REFSSC_BYP                      (1 << 0)
#define REG_REFSSC_RSTB                     (1 << 1)
#define REG_SSC_DLY_NUM_SHIFT               2
#define REG_SSC_DLY_NUM_MASK                (0xF << REG_SSC_DLY_NUM_SHIFT)
#define REG_SSC_DLY_NUM(n)                  BITFIELD_VAL(REG_SSC_DLY_NUM, n)
#define REG_REFSSC_RANGE_SHIFT              6
#define REG_REFSSC_RANGE_MASK               (0x3 << REG_REFSSC_RANGE_SHIFT)
#define REG_REFSSC_RANGE(n)                 BITFIELD_VAL(REG_REFSSC_RANGE, n)

// REG_60
#define REG_PI_CLKGEN                       (1 << 0)
#define REG_CLKGEN_SWRC_SHIFT               1
#define REG_CLKGEN_SWRC_MASK                (0x3 << REG_CLKGEN_SWRC_SHIFT)
#define REG_CLKGEN_SWRC(n)                  BITFIELD_VAL(REG_CLKGEN_SWRC, n)
#define REG_CLKGEN_RST                      (1 << 3)
#define REG_SEL_DIG                         (1 << 4)
#define REG_EN_CLK_USB                      (1 << 5)
#define REG_EN_CLK_DIG                      (1 << 6)

// REG_61
#define REG_PU_DIG                          (1 << 0)
#define REG_DIG_SWRC_SHIFT                  1
#define REG_DIG_SWRC_MASK                   (0x3 << REG_DIG_SWRC_SHIFT)
#define REG_DIG_SWRC(n)                     BITFIELD_VAL(REG_DIG_SWRC, n)
#define REG_EN_DWC_IN                       (1 << 3)
#define REG_EN_DWC_OUT                      (1 << 4)
#define REG_DCO_SDM_BYPASS                  (1 << 5)
#define REG_CALIB_BYPASS                    (1 << 6)
#define REG_DCO_SDM_ORDER_SHIFT             7
#define REG_DCO_SDM_ORDER_MASK              (0x3 << REG_DCO_SDM_ORDER_SHIFT)
#define REG_DCO_SDM_ORDER(n)                BITFIELD_VAL(REG_DCO_SDM_ORDER, n)
#define REG_SDM_PRBS_ABS_SHIFT              9
#define REG_SDM_PRBS_ABS_MASK               (0x3 << REG_SDM_PRBS_ABS_SHIFT)
#define REG_SDM_PRBS_ABS(n)                 BITFIELD_VAL(REG_SDM_PRBS_ABS, n)
#define REG_SDM_DR                          (1 << 11)

// REG_62
#define REG_DWC_IN_VAL_15_0_SHIFT           0
#define REG_DWC_IN_VAL_15_0_MASK            (0xFFFF << REG_DWC_IN_VAL_15_0_SHIFT)
#define REG_DWC_IN_VAL_15_0(n)              BITFIELD_VAL(REG_DWC_IN_VAL_15_0, n)

// REG_63
#define REG_DWC_OUT_LO_VAL_SHIFT            0
#define REG_DWC_OUT_LO_VAL_MASK             (0x7FFF << REG_DWC_OUT_LO_VAL_SHIFT)
#define REG_DWC_OUT_LO_VAL(n)               BITFIELD_VAL(REG_DWC_OUT_LO_VAL, n)
#define REG_DWC_IN_VAL_16                   (1 << 15)

// REG_64
#define REG_DWC_OUT_HI_VAL_SHIFT            0
#define REG_DWC_OUT_HI_VAL_MASK             (0xF << REG_DWC_OUT_HI_VAL_SHIFT)
#define REG_DWC_OUT_HI_VAL(n)               BITFIELD_VAL(REG_DWC_OUT_HI_VAL, n)
#define REG_WAIT_SDM_DONE_DLY0_SHIFT        4
#define REG_WAIT_SDM_DONE_DLY0_MASK         (0xF << REG_WAIT_SDM_DONE_DLY0_SHIFT)
#define REG_WAIT_SDM_DONE_DLY0(n)           BITFIELD_VAL(REG_WAIT_SDM_DONE_DLY0, n)
#define REG_WAIT_SDM_DONE_DLY1_SHIFT        8
#define REG_WAIT_SDM_DONE_DLY1_MASK         (0xF << REG_WAIT_SDM_DONE_DLY1_SHIFT)
#define REG_WAIT_SDM_DONE_DLY1(n)           BITFIELD_VAL(REG_WAIT_SDM_DONE_DLY1, n)
#define DIG_PLL_RST                         (1 << 12)
#define DIG_PLL_RST_DR                      (1 << 13)

// REG_65
#define REG_USBPLL_TIMING_T0_SHIFT          0
#define REG_USBPLL_TIMING_T0_MASK           (0xFF << REG_USBPLL_TIMING_T0_SHIFT)
#define REG_USBPLL_TIMING_T0(n)             BITFIELD_VAL(REG_USBPLL_TIMING_T0, n)
#define REG_USBPLL_TIMING_T1_SHIFT          8
#define REG_USBPLL_TIMING_T1_MASK           (0xFF << REG_USBPLL_TIMING_T1_SHIFT)
#define REG_USBPLL_TIMING_T1(n)             BITFIELD_VAL(REG_USBPLL_TIMING_T1, n)

// REG_66
#define REG_USBPLL_TIMING_T2_SHIFT          0
#define REG_USBPLL_TIMING_T2_MASK           (0xFF << REG_USBPLL_TIMING_T2_SHIFT)
#define REG_USBPLL_TIMING_T2(n)             BITFIELD_VAL(REG_USBPLL_TIMING_T2, n)
#define REG_USBPLL_TIMING_T3_SHIFT          8
#define REG_USBPLL_TIMING_T3_MASK           (0xFF << REG_USBPLL_TIMING_T3_SHIFT)
#define REG_USBPLL_TIMING_T3(n)             BITFIELD_VAL(REG_USBPLL_TIMING_T3, n)

// REG_67
#define REG_USBPLL_TIMING_T4_SHIFT          0
#define REG_USBPLL_TIMING_T4_MASK           (0xFF << REG_USBPLL_TIMING_T4_SHIFT)
#define REG_USBPLL_TIMING_T4(n)             BITFIELD_VAL(REG_USBPLL_TIMING_T4, n)
#define REG_USBPLL_TIMING_T5_SHIFT          8
#define REG_USBPLL_TIMING_T5_MASK           (0xFF << REG_USBPLL_TIMING_T5_SHIFT)
#define REG_USBPLL_TIMING_T5(n)             BITFIELD_VAL(REG_USBPLL_TIMING_T5, n)

// REG_68
#define REG_USBPLL_TIMING_T6_SHIFT          0
#define REG_USBPLL_TIMING_T6_MASK           (0xFF << REG_USBPLL_TIMING_T6_SHIFT)
#define REG_USBPLL_TIMING_T6(n)             BITFIELD_VAL(REG_USBPLL_TIMING_T6, n)
#define REG_USBPLL_TIMING_T7_SHIFT          8
#define REG_USBPLL_TIMING_T7_MASK           (0xFF << REG_USBPLL_TIMING_T7_SHIFT)
#define REG_USBPLL_TIMING_T7(n)             BITFIELD_VAL(REG_USBPLL_TIMING_T7, n)

// REG_69
#define REG_USBPLL_LDO_PRECHAR_VALUE_SHIFT  0
#define REG_USBPLL_LDO_PRECHAR_VALUE_MASK   (0xFF << REG_USBPLL_LDO_PRECHAR_VALUE_SHIFT)
#define REG_USBPLL_LDO_PRECHAR_VALUE(n)     BITFIELD_VAL(REG_USBPLL_LDO_PRECHAR_VALUE, n)

// REG_6A
#define REG_USBPLL_LDO_PU_DR                (1 << 0)
#define REG_USBPLL_LDO_PU                   (1 << 1)
#define REG_USBPLL_LDO_PRECHARGE_DR         (1 << 2)
#define REG_USBPLL_LDO_PRECHARGE            (1 << 3)
#define REG_USBPLL_DIV_CAL_EN_DR            (1 << 4)
#define REG_USBPLL_DIV_CAL_EN               (1 << 5)
#define REG_USBPLL_DLF_RSTN_DR              (1 << 6)
#define REG_USBPLL_DLF_RSTN                 (1 << 7)
#define REG_USBPLL_REFSSC_RSTB_DR           (1 << 8)
#define REG_USBPLL_REFSSC_RSTB              (1 << 9)
#define REG_USBPLL_CLKGEN_RST_DR            (1 << 10)
#define REG_USBPLL_CLKGEN_RST               (1 << 11)
#define REG_USBPLL_SDM_CLK_SEL_DR           (1 << 12)
#define REG_USBPLL_SDM_CLK_SEL              (1 << 13)
#define REG_USBPLL_SDM_COMP_BYPASS_DR       (1 << 14)
#define REG_USBPLL_SDM_COMP_BYPASS          (1 << 15)

// REG_6B
#define REG_USBPLL_EN_OVER_THR_DR           (1 << 0)
#define REG_USBPLL_EN_OVER_THR              (1 << 1)
#define REG_USBPLL_VCO_CAL_EN               (1 << 2)
#define REG_USBPLL_SDM_COMP_ACCUM_SIGN      (1 << 3)
#define REG_USBPLL_SDM_GCAL_DELAY_NUM_SHIFT 4
#define REG_USBPLL_SDM_GCAL_DELAY_NUM_MASK  (0x3 << REG_USBPLL_SDM_GCAL_DELAY_NUM_SHIFT)
#define REG_USBPLL_SDM_GCAL_DELAY_NUM(n)    BITFIELD_VAL(REG_USBPLL_SDM_GCAL_DELAY_NUM, n)
#define REG_USBPLL_SDM_COMP_GAIN_SEL_SHIFT  6
#define REG_USBPLL_SDM_COMP_GAIN_SEL_MASK   (0x3 << REG_USBPLL_SDM_COMP_GAIN_SEL_SHIFT)
#define REG_USBPLL_SDM_COMP_GAIN_SEL(n)     BITFIELD_VAL(REG_USBPLL_SDM_COMP_GAIN_SEL, n)
#define REG_USBPLL_VCO_CAL_MANUAL           (1 << 8)
#define REG_USBPLL_CAL_CNT_EN_DR            (1 << 9)
#define REG_USBPLL_CAL_CNT_EN               (1 << 10)
#define REG_USBPLL_CAL_EN_DR                (1 << 11)
#define REG_USBPLL_CAL_EN                   (1 << 12)

// REG_6C
#define REG_USBPLL_INIT_DELAY_SHIFT         0
#define REG_USBPLL_INIT_DELAY_MASK          (0x7 << REG_USBPLL_INIT_DELAY_SHIFT)
#define REG_USBPLL_INIT_DELAY(n)            BITFIELD_VAL(REG_USBPLL_INIT_DELAY, n)
#define REG_USBPLL_CNT_TIME_SHIFT           3
#define REG_USBPLL_CNT_TIME_MASK            (0x7 << REG_USBPLL_CNT_TIME_SHIFT)
#define REG_USBPLL_CNT_TIME(n)              BITFIELD_VAL(REG_USBPLL_CNT_TIME, n)
#define REG_USBPLL_WAIT_TIME_SHIFT          6
#define REG_USBPLL_WAIT_TIME_MASK           (0x7 << REG_USBPLL_WAIT_TIME_SHIFT)
#define REG_USBPLL_WAIT_TIME(n)             BITFIELD_VAL(REG_USBPLL_WAIT_TIME, n)
#define REG_USBPLL_CAL_OPT                  (1 << 9)
#define REG_USBPLL_PRESCALER_DEL_SEL_SHIFT  10
#define REG_USBPLL_PRESCALER_DEL_SEL_MASK   (0xF << REG_USBPLL_PRESCALER_DEL_SEL_SHIFT)
#define REG_USBPLL_PRESCALER_DEL_SEL(n)     BITFIELD_VAL(REG_USBPLL_PRESCALER_DEL_SEL, n)
#define REG_BT_BBPLL_CAL_CLK_EN_DR          (1 << 14)
#define REG_BT_BBPLL_CAL_CLK_EN             (1 << 15)

// REG_6D
#define REG_BT_VCO_CAPBANK_SHIFT            0
#define REG_BT_VCO_CAPBANK_MASK             (0xFFF << REG_BT_VCO_CAPBANK_SHIFT)
#define REG_BT_VCO_CAPBANK(n)               BITFIELD_VAL(REG_BT_VCO_CAPBANK, n)
#define REG_BT_VCO_CAPBANK_DR               (1 << 12)
#define REG_USBPLL_FREQWORD_DEC_SEL_SHIFT   13
#define REG_USBPLL_FREQWORD_DEC_SEL_MASK    (0x7 << REG_USBPLL_FREQWORD_DEC_SEL_SHIFT)
#define REG_USBPLL_FREQWORD_DEC_SEL(n)      BITFIELD_VAL(REG_USBPLL_FREQWORD_DEC_SEL, n)

// REG_6E
#define REG_USBPLL_FREQWORD_15_0_SHIFT      0
#define REG_USBPLL_FREQWORD_15_0_MASK       (0xFFFF << REG_USBPLL_FREQWORD_15_0_SHIFT)
#define REG_USBPLL_FREQWORD_15_0(n)         BITFIELD_VAL(REG_USBPLL_FREQWORD_15_0, n)

// REG_6F
#define REG_USBPLL_FREQWORD_31_16_SHIFT     0
#define REG_USBPLL_FREQWORD_31_16_MASK      (0xFFFF << REG_USBPLL_FREQWORD_31_16_SHIFT)
#define REG_USBPLL_FREQWORD_31_16(n)        BITFIELD_VAL(REG_USBPLL_FREQWORD_31_16, n)

// REG_70
#define REG_USBPLL_FREQWORD_34_32_SHIFT     0
#define REG_USBPLL_FREQWORD_34_32_MASK      (0x7 << REG_USBPLL_FREQWORD_34_32_SHIFT)
#define REG_USBPLL_FREQWORD_34_32(n)        BITFIELD_VAL(REG_USBPLL_FREQWORD_34_32, n)
#define REG_USBPLL_FREQ_OFFSET_EN           (1 << 3)
#define CHIP_ADDR_I2C_SHIFT                 4
#define CHIP_ADDR_I2C_MASK                  (0x7F << CHIP_ADDR_I2C_SHIFT)
#define CHIP_ADDR_I2C(n)                    BITFIELD_VAL(CHIP_ADDR_I2C, n)
#define REG_BT_BBPLL_SDM_RESETN_DR          (1 << 11)
#define REG_BT_BBPLL_SDM_RESETN             (1 << 12)
#define REG_BT_BBPLL_CAL_RESETN_DR          (1 << 13)
#define REG_BT_BBPLL_CAL_RESETN             (1 << 14)

// REG_71
#define REG_USBPLL_FREQ_OFFSET_SHIFT        0
#define REG_USBPLL_FREQ_OFFSET_MASK         (0xFFFF << REG_USBPLL_FREQ_OFFSET_SHIFT)
#define REG_USBPLL_FREQ_OFFSET(n)           BITFIELD_VAL(REG_USBPLL_FREQ_OFFSET, n)

// REG_72
#define REG_USBPLL_SSC_ENABLE               (1 << 0)
#define REG_USBPLL_TRI_FREQ_OFST_SEL_SHIFT  1
#define REG_USBPLL_TRI_FREQ_OFST_SEL_MASK   (0x7 << REG_USBPLL_TRI_FREQ_OFST_SEL_SHIFT)
#define REG_USBPLL_TRI_FREQ_OFST_SEL(n)     BITFIELD_VAL(REG_USBPLL_TRI_FREQ_OFST_SEL, n)
#define REG_USBPLL_TRI_FREQ_SEL_SHIFT       4
#define REG_USBPLL_TRI_FREQ_SEL_MASK        (0x3 << REG_USBPLL_TRI_FREQ_SEL_SHIFT)
#define REG_USBPLL_TRI_FREQ_SEL(n)          BITFIELD_VAL(REG_USBPLL_TRI_FREQ_SEL, n)
#define REG_USBPLL_TRI_STEP_SEL_SHIFT       6
#define REG_USBPLL_TRI_STEP_SEL_MASK        (0x3 << REG_USBPLL_TRI_STEP_SEL_SHIFT)
#define REG_USBPLL_TRI_STEP_SEL(n)          BITFIELD_VAL(REG_USBPLL_TRI_STEP_SEL, n)
#define REG_USBPLL_RSTN_DR                  (1 << 8)
#define REG_USBPLL_RSTN                     (1 << 9)
#define REG_USBPLL_CLK_FBC_EDGE             (1 << 10)
#define REG_USBPLL_INT_DEC_SEL_SHIFT        11
#define REG_USBPLL_INT_DEC_SEL_MASK         (0x7 << REG_USBPLL_INT_DEC_SEL_SHIFT)
#define REG_USBPLL_INT_DEC_SEL(n)           BITFIELD_VAL(REG_USBPLL_INT_DEC_SEL, n)
#define REG_USBPLL_DITHER_BYPASS            (1 << 14)

// REG_73
#define REG_USBPLL_DIV_DR                   (1 << 0)
#define REG_USBPLL_DIV_FRAC_SHIFT           1
#define REG_USBPLL_DIV_FRAC_MASK            (0x3 << REG_USBPLL_DIV_FRAC_SHIFT)
#define REG_USBPLL_DIV_FRAC(n)              BITFIELD_VAL(REG_USBPLL_DIV_FRAC, n)
#define REG_USBPLL_DIV_INT_SHIFT            3
#define REG_USBPLL_DIV_INT_MASK             (0x7F << REG_USBPLL_DIV_INT_SHIFT)
#define REG_USBPLL_DIV_INT(n)               BITFIELD_VAL(REG_USBPLL_DIV_INT, n)

// REG_74
#define REG_USBPLL_SDM_COMP_G0_SHIFT        0
#define REG_USBPLL_SDM_COMP_G0_MASK         (0x1FF << REG_USBPLL_SDM_COMP_G0_SHIFT)
#define REG_USBPLL_SDM_COMP_G0(n)           BITFIELD_VAL(REG_USBPLL_SDM_COMP_G0, n)
#define REG_USBPLL_SDM_COMP_G0_DR           (1 << 9)
#define BBPLL_VCO_CAL_BITS_SHIFT            10
#define BBPLL_VCO_CAL_BITS_MASK             (0x3F << BBPLL_VCO_CAL_BITS_SHIFT)
#define BBPLL_VCO_CAL_BITS(n)               BITFIELD_VAL(BBPLL_VCO_CAL_BITS, n)

// REG_75
#define PLL_CNT_SHIFT                       0
#define PLL_CNT_MASK                        (0x1FFF << PLL_CNT_SHIFT)
#define PLL_CNT(n)                          BITFIELD_VAL(PLL_CNT, n)

// REG_76
#define BBPLL_CAL_FREQ_COMPARE_SHIFT        0
#define BBPLL_CAL_FREQ_COMPARE_MASK         (0xFFFF << BBPLL_CAL_FREQ_COMPARE_SHIFT)
#define BBPLL_CAL_FREQ_COMPARE(n)           BITFIELD_VAL(BBPLL_CAL_FREQ_COMPARE, n)

enum USBPHY_REG_T {
    USBPHY_REG_00   = 0x00,
    USBPHY_REG_01,
    USBPHY_REG_02,
    USBPHY_REG_03,
    USBPHY_REG_04,
    USBPHY_REG_05,
    USBPHY_REG_06,
    USBPHY_REG_07,
    USBPHY_REG_08,
    USBPHY_REG_09,
    USBPHY_REG_0A,
    USBPHY_REG_0B,
    USBPHY_REG_0C,
    USBPHY_REG_0D,
    USBPHY_REG_0E,
    USBPHY_REG_0F,
    USBPHY_REG_10,
    USBPHY_REG_11,
    USBPHY_REG_12,
    USBPHY_REG_13,
    USBPHY_REG_14,
    USBPHY_REG_15,
    USBPHY_REG_16,
    USBPHY_REG_17,
    USBPHY_REG_18,
    USBPHY_REG_19,
    USBPHY_REG_1A,
    USBPHY_REG_1B,

    USBPHY_REG_20   = 0x20,
    USBPHY_REG_21,
    USBPHY_REG_22,
    USBPHY_REG_23,
    USBPHY_REG_24,
    USBPHY_REG_25,

    USBPHY_REG_30   = 0x30,
    USBPHY_REG_31,
    USBPHY_REG_32,

    USBPHY_REG_41   = 0x61,
    USBPHY_REG_42,
    USBPHY_REG_43,
    USBPHY_REG_44,
    USBPHY_REG_45,
    USBPHY_REG_46,
    USBPHY_REG_47,

    USBPHY_REG_51   = 0x51,
    USBPHY_REG_52,
    USBPHY_REG_53,
    USBPHY_REG_54,
    USBPHY_REG_55,
    USBPHY_REG_56,
    USBPHY_REG_57,
    USBPHY_REG_58,
    USBPHY_REG_59,
    USBPHY_REG_5A,
    USBPHY_REG_5B,
    USBPHY_REG_5C,
    USBPHY_REG_5D,
    USBPHY_REG_5E,
    USBPHY_REG_5F,
    USBPHY_REG_60,
    USBPHY_REG_61,
    USBPHY_REG_62,
    USBPHY_REG_63,
    USBPHY_REG_64,
    USBPHY_REG_65,
    USBPHY_REG_66,
    USBPHY_REG_67,
    USBPHY_REG_68,
    USBPHY_REG_69,
    USBPHY_REG_6A,
    USBPHY_REG_6B,
    USBPHY_REG_6C,
    USBPHY_REG_6D,
    USBPHY_REG_6E,
    USBPHY_REG_6F,
    USBPHY_REG_70,
    USBPHY_REG_71,
    USBPHY_REG_72,
    USBPHY_REG_73,
    USBPHY_REG_74,
    USBPHY_REG_75,
};

#endif
