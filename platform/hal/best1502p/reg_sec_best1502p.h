#ifndef __REG_SEC_BEST1502P_H__
#define __REG_SEC_BEST1502P_H__

#include "plat_types.h"

struct HAL_SEC_T {
    __IO uint32_t REG_000;
    __IO uint32_t REG_004;
    __IO uint32_t REG_008;
    __IO uint32_t REG_00C;
    __IO uint32_t REG_010;
    __IO uint32_t REG_014;
    __IO uint32_t REG_018;
    __IO uint32_t REG_01C;
    __IO uint32_t REG_020;
    __IO uint32_t REG_024;
    __IO uint32_t REG_028;
    __IO uint32_t REG_02C;
};

// reg_00
#define SEC_CFG_AP_PROT_ROM1                            (1 << 0)
#define SEC_CFG_NONSEC_PROT_ROM1                        (1 << 1)
#define SEC_CFG_SEC_RESP_PROT_ROM1                      (1 << 2)
#define SEC_CFG_NONSEC_BYPASS_PROT_ROM1                 (1 << 3)
#define SEC_CFG_AP_PROT_MCU2SENS                        (1 << 4)
#define SEC_CFG_NONSEC_PROT_MCU2SENS                    (1 << 5)
#define SEC_CFG_SEC_RESP_PROT_MCU2SENS                  (1 << 6)
#define SEC_CFG_NONSEC_BYPASS_PROT_MCU2SENS             (1 << 7)
#define SEC_CFG_INT_VALUE_MPC_SRAM9                     (1 << 8)
#define SEC_CFG_NONSEC_BYPASS_MPC_SRAM9                 (1 << 9)
#define SEC_CFG_INT_VALUE_MPC_SRAM8                     (1 << 10)
#define SEC_CFG_NONSEC_BYPASS_MPC_SRAM8                 (1 << 11)
#define SEC_CFG_INT_VALUE_MPC_SRAM7                     (1 << 12)
#define SEC_CFG_NONSEC_BYPASS_MPC_SRAM7                 (1 << 13)
#define SEC_CFG_INT_VALUE_MPC_SRAM6                     (1 << 14)
#define SEC_CFG_NONSEC_BYPASS_MPC_SRAM6                 (1 << 15)
#define SEC_CFG_INT_VALUE_MPC_SRAM5                     (1 << 16)
#define SEC_CFG_NONSEC_BYPASS_MPC_SRAM5                 (1 << 17)
#define SEC_CFG_INT_VALUE_MPC_SRAM4                     (1 << 18)
#define SEC_CFG_NONSEC_BYPASS_MPC_SRAM4                 (1 << 19)
#define SEC_CFG_INT_VALUE_MPC_SRAM3                     (1 << 20)
#define SEC_CFG_NONSEC_BYPASS_MPC_SRAM3                 (1 << 21)
#define SEC_CFG_INT_VALUE_MPC_SRAM2                     (1 << 22)
#define SEC_CFG_NONSEC_BYPASS_MPC_SRAM2                 (1 << 23)
#define SEC_CFG_INT_VALUE_MPC_SRAM1                     (1 << 24)
#define SEC_CFG_NONSEC_BYPASS_MPC_SRAM1                 (1 << 25)
#define SEC_CFG_INT_VALUE_MPC_SRAM0                     (1 << 26)
#define SEC_CFG_NONSEC_BYPASS_MPC_SRAM0                 (1 << 27)

// reg_04
#define SEC_CFG_AP_PROT_ROM0                            (1 << 0)
#define SEC_CFG_NONSEC_PROT_ROM0                        (1 << 1)
#define SEC_CFG_SEC_RESP_PROT_ROM0                      (1 << 2)
#define SEC_CFG_NONSEC_BYPASS_PROT_ROM0                 (1 << 3)
#define SEC_CFG_INIT_VALUE_MPC_PSRAM                    (1 << 4)
#define SEC_CFG_NONSEC_BYPASS_MPC_PSRAM                 (1 << 5)
#define SEC_CFG_INIT_VALUE_MPC_SPINOR0                  (1 << 6)
#define SEC_CFG_NONSEC_BYPASS_MPC_SPINOR0               (1 << 7)
#define SEC_CFG_INIT_VALUE_MPC_SPINOR1                  (1 << 8)
#define SEC_CFG_NONSEC_BYPASS_MPC_SPINOR1               (1 << 9)
#define SEC_CFG_INIT_VALUE_MPC_C0CC                     (1 << 10)
#define SEC_CFG_NONSEC_BYPASS_MPC_C0CC                  (1 << 11)
#define SEC_CFG_INIT_VALUE_MPC_C0DC                     (1 << 12)
#define SEC_CFG_NONSEC_BYPASS_MPC_C0DC                  (1 << 13)
#define SEC_CFG_INIT_VALUE_MPC_C1CC                     (1 << 14)
#define SEC_CFG_NONSEC_BYPASS_MPC_C1CC                  (1 << 15)
#define SEC_CFG_INIT_VALUE_MPC_C1DC                     (1 << 16)
#define SEC_CFG_NONSEC_BYPASS_MPC_C1DC                  (1 << 17)

// reg_08
#define SEC_IRQ_CLR_PPC_ROM1                            (1 << 0)
#define SEC_IRQ_EN_PPC_ROM1                             (1 << 1)
#define SEC_IRQ_CLR_PPC_MCU2SENS                        (1 << 2)
#define SEC_IRQ_EN_PPC_MCU2SENS                         (1 << 3)
#define SEC_IRQ_CLR_PPC_SRAM7                           (1 << 4)
#define SEC_IRQ_EN_PPC_SRAM7                            (1 << 5)
#define SEC_IRQ_CLR_PPC_SRAM6                           (1 << 6)
#define SEC_IRQ_EN_PPC_SRAM6                            (1 << 7)
#define SEC_IRQ_CLR_PPC_SRAM5                           (1 << 8)
#define SEC_IRQ_EN_MPC_SRAM5                            (1 << 9)
#define SEC_IRQ_CLR_PPC_SRAM4                           (1 << 10)
#define SEC_IRQ_EN_MPC_SRAM4                            (1 << 11)
#define SEC_IRQ_CLR_PPC_SRAM3                           (1 << 12)
#define SEC_IRQ_EN_MPC_SRAM3                            (1 << 13)
#define SEC_IRQ_CLR_PPC_SRAM2                           (1 << 14)
#define SEC_IRQ_EN_MPC_SRAM2                            (1 << 15)
#define SEC_IRQ_CLR_PPC_SRAM1                           (1 << 16)
#define SEC_IRQ_EN_MPC_SRAM1                            (1 << 17)
#define SEC_IRQ_CLR_PPC_SRAM0                           (1 << 18)
#define SEC_IRQ_EN_MPC_SRAM0                            (1 << 19)
#define SEC_IRQ_CLR_PPC_ROM0                            (1 << 20)
#define SEC_IRQ_EN_PPC_ROM0                             (1 << 21)
#define SEC_IRQ_EN_MPC_PSRAM                            (1 << 22)
#define SEC_IRQ_EN_MPC_SPINOR0                          (1 << 23)
#define SEC_IRQ_EN_MPC_SPINOR1                          (1 << 24)
#define SEC_IRQ_CLR_PPC_SRAM8                           (1 << 25)
#define SEC_IRQ_EN_PPC_SRAM8                            (1 << 26)
#define SEC_IRQ_CLR_PPC_SRAM9                           (1 << 27)
#define SEC_IRQ_EN_PPC_SRAM9                            (1 << 28)

// reg_0c
#define SEC_CFG_NONSEC_ADMA                             (1 << 0)
#define SEC_CFG_NONSEC_GDMA                             (1 << 1)
#define SEC_CFG_NONSEC_BCM                              (1 << 2)
#define SEC_CFG_NONSEC_USB                              (1 << 3)
#define SEC_CFG_NONSEC_LCDC                             (1 << 4)
#define SEC_CFG_NONSEC_BT2MCU                           (1 << 5)
#define SEC_CFG_NONSEC_SENS2MCU                         (1 << 6)
#define SEC_CFG_NONSEC_BT_TP_DUMP                       (1 << 7)
#define SEC_CFG_NONSEC_EMMC                             (1 << 8)
#define SEC_CFG_NONSEC_DMA2D_M_READ                     (1 << 9)
#define SEC_CFG_NONSEC_DMA2D_M_WRITE                    (1 << 10)
#define SEC_CFG_NONSEC_GPU_NC_READ                      (1 << 11)
#define SEC_CFG_NONSEC_GPU_NC_WRITE                     (1 << 12)
#define SEC_CFG_NONSEC_MMU_M                            (1 << 13)
#define SEC_CFG_NONSEC_CRCCHS                           (1 << 14)
#define SEC_CFG_NONSEC_CORDIC                           (1 << 15)
#define SEC_CFG_NONSEC_CAN                              (1 << 16)
#define SEC_CFG_NONSEC_CAM_SPI                          (1 << 17)
#define SEC_IRQ_CLR_MPC_C0CC                            (1 << 18)
#define SEC_IRQ_EN_MPC_C0CC                             (1 << 19)
#define SEC_IRQ_CLR_MPC_C0DC                            (1 << 20)
#define SEC_IRQ_EN_MPC_C0DC                             (1 << 21)
#define SEC_IRQ_CLR_MPC_C1CC                            (1 << 22)
#define SEC_IRQ_EN_MPC_C1CC                             (1 << 23)
#define SEC_IRQ_CLR_MPC_C1DC                            (1 << 24)
#define SEC_IRQ_EN_MPC_C1DC                             (1 << 25)

// reg_10
#define SEC_CFG_NONSEC_PROT_AHB1(n)                     (((n) & 0xFFFF) << 0)
#define SEC_CFG_NONSEC_PROT_AHB1_MASK                   (0xFFFF << 0)
#define SEC_CFG_NONSEC_PROT_AHB1_SHIFT                  (0)
#define SEC_CFG_NONSEC_BYPASS_PROT_AHB1(n)              (((n) & 0xFFFF) << 16)
#define SEC_CFG_NONSEC_BYPASS_PROT_AHB1_MASK            (0xFFFF << 16)
#define SEC_CFG_NONSEC_BYPASS_PROT_AHB1_SHIFT           (16)

// reg_14
#define SEC_CFG_NONSEC_PROT_APB0(n)                     (((n) & 0xFFFFFFFF) << 0)
#define SEC_CFG_NONSEC_PROT_APB0_MASK                   (0xFFFFFFFF << 0)
#define SEC_CFG_NONSEC_PROT_APB0_SHIFT                  (0)

// reg_18
#define SEC_CFG_NONSEC_BYPASS_PROT_APB0(n)              (((n) & 0xFFFFFFFF) << 0)
#define SEC_CFG_NONSEC_BYPASS_PROT_APB0_MASK            (0xFFFFFFFF << 0)
#define SEC_CFG_NONSEC_BYPASS_PROT_APB0_SHIFT           (0)

// reg_1c
#define SEC_IDAUEN                                      (1 << 0)
#define SEC_CFG_SEC2NSEC                                (1 << 1)
#define SEC_SPIDEN_CORE0                                (1 << 2)
#define SEC_SPNIDEN_CORE0                               (1 << 3)
#define SEC_SPIDEN_CORE1                                (1 << 4)
#define SEC_SPNIDEN_CORE1                               (1 << 5)

// reg_20
#define SEC_IRQ_PPC_ROM1                                (1 << 0)
#define SEC_IRQ_PPC_MCU2SENS                            (1 << 1)
#define SEC_IRQ_MPC_SRAM9                               (1 << 2)
#define SEC_IRQ_MPC_SRAM8                               (1 << 3)
#define SEC_IRQ_MPC_SRAM7                               (1 << 4)
#define SEC_IRQ_MPC_SRAM6                               (1 << 5)
#define SEC_IRQ_MPC_SRAM5                               (1 << 6)
#define SEC_IRQ_MPC_SRAM4                               (1 << 7)
#define SEC_IRQ_MPC_SRAM3                               (1 << 8)
#define SEC_IRQ_MPC_SRAM2                               (1 << 9)
#define SEC_IRQ_MPC_SRAM1                               (1 << 10)
#define SEC_IRQ_MPC_SRAM0                               (1 << 11)
#define SEC_IRQ_PPC_ROM0                                (1 << 12)
#define SEC_IRQ_MPC_PSRAM                               (1 << 13)
#define SEC_IRQ_MPC_SPINOR0                             (1 << 14)
#define SEC_IRQ_MPC_SPINOR1                             (1 << 15)
#define SEC_IRQ_MPC_C0CC                                (1 << 16)
#define SEC_IRQ_MPC_C0DC                                (1 << 17)
#define SEC_IRQ_MPC_C1CC                                (1 << 18)
#define SEC_IRQ_MPC_C1DC                                (1 << 19)

// reg_24
#define SEC_CFG_NONSEC_PROT_AHB2(n)                     (((n) & 0xFFFF) << 0)
#define SEC_CFG_NONSEC_PROT_AHB2_MASK                   (0xFFFF << 0)
#define SEC_CFG_NONSEC_PROT_AHB2_SHIFT                  (0)
#define SEC_CFG_NONSEC_BYPASS_PROT_AHB2(n)              (((n) & 0xFFFF) << 16)
#define SEC_CFG_NONSEC_BYPASS_PROT_AHB2_MASK            (0xFFFF << 16)
#define SEC_CFG_NONSEC_BYPASS_PROT_AHB2_SHIFT           (16)

// reg_28
#define SEC_CFG_NONSEC_PROT_AHB3(n)                     (((n) & 0xFFFF) << 0)
#define SEC_CFG_NONSEC_PROT_AHB3_MASK                   (0xFFFF << 0)
#define SEC_CFG_NONSEC_PROT_AHB3_SHIFT                  (0)
#define SEC_CFG_NONSEC_BYPASS_PROT_AHB3(n)              (((n) & 0xFFFF) << 16)
#define SEC_CFG_NONSEC_BYPASS_PROT_AHB3_MASK            (0xFFFF << 16)
#define SEC_CFG_NONSEC_BYPASS_PROT_AHB3_SHIFT           (16)

// reg_2C
#define SEC_CFG_SEC_BRIDGE_MPC_C1_SEL                   (1 << 0)

#endif
