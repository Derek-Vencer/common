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
#ifndef __PLAT_ADDR_MAP_BEST1502P_H__
#define __PLAT_ADDR_MAP_BEST1502P_H__

#ifdef __cplusplus
extern "C" {
#endif

#if defined(ROM_BUILD)
#define ROM_BASE                                0x26000000
#define ROMX_BASE                               0x04020000
#else
#define ROM_BASE                                0x22000000
#define ROMX_BASE                               0x00020000
#endif

#ifndef ROM_SIZE
#define ROM_SIZE                                0x00010000
#endif
#define ROM_EXT_SIZE                            0x00050000

#define BTH_ROM_BASE                            0x22100000
#define BTH_ROMX_BASE                           0x00100000

#ifndef ROM_SIZE
#define BTH_ROM_SIZE                            0x000A0000
#endif

#define PATCH_ENTRY_NUM                         8
#define PATCH_CTRL_BASE                         0x0008F000
#define PATCH_DATA_BASE                         0x0008F100

#define PATCH1_ENTRY_NUM                        8
#define PATCH1_CTRL_BASE                        0x001CF000
#define PATCH1_DATA_BASE                        0x001CF100

#define RAM_SECURITY_FLAG                       0x04000000

#define SHR_RAM_BLK_SIZE                        0x00040000

#define RAM0_BASE                               0x20000000
#define RAMX0_BASE                              0x00200000
#define RAM1_BASE                               0x20020000
#define RAMX1_BASE                              0x00220000
#define RAM2_BASE                               0x20040000
#define RAMX2_BASE                              0x00240000
#define RAM3_BASE                               0x20060000
#define RAMX3_BASE                              0x00260000
#define RAM4_BASE                               0x200A0000
#define RAMX4_BASE                              0x002A0000
#define RAM5_BASE                               0x200E0000
#define RAMX5_BASE                              0x002E0000
#define RAM6_BASE                               0x20120000
#define RAMX6_BASE                              0x00320000
#define RAM7_BASE                               0x20160000
#define RAMX7_BASE                              0x00360000
#define RAM8_BASE                               0x201A0000
#define RAMX8_BASE                              0x003A0000
#define RAM9_BASE                               0x201E0000
#define RAMX9_BASE                              0x003E0000

#define RAM0_SIZE                               0x00020000
#define RAM1_SIZE                               0x00020000
#define RAM2_SIZE                               0x00020000
#define RAM3_SIZE                               0x00040000
#define RAM4_SIZE                               0x00040000
#define RAM5_SIZE                               0x00040000
#define RAM6_SIZE                               0x00040000
#define RAM7_SIZE                               0x00040000
#define RAM8_SIZE                               0x00040000
#define RAM9_SIZE                               0x00020000

#define RAM_TOTAL_SIZE                          (RAM9_BASE + RAM9_SIZE - RAM0_BASE)

#define SENS_RAM0_BASE                               0x20200000
#define SENS_RAMX0_BASE                              0x00400000
#define SENS_RAM1_BASE                               0x20240000
#define SENS_RAMX1_BASE                              0x00440000
#define SENS_RAM2_BASE                               0x20280000
#define SENS_RAMX2_BASE                              0x00480000
#define SENS_RAM3_BASE                               0x202C0000
#define SENS_RAMX3_BASE                              0x004C0000
#define SENS_RAM4_BASE                               0x20300000
#define SENS_RAMX4_BASE                              0x00500000
#define SENS_RAM5_BASE                               0x20340000
#define SENS_RAMX5_BASE                              0x00540000
#define SENS_RAM6_BASE                               0x20360000
#define SENS_RAMX6_BASE                              0x00560000

#define SENS_RAM0_SIZE                               0x00040000
#define SENS_RAM1_SIZE                               0x00040000
#define SENS_RAM2_SIZE                               0x00040000
#define SENS_RAM3_SIZE                               0x00040000
#define SENS_RAM4_SIZE                               0x00040000
#define SENS_RAM5_SIZE                               0x00020000
#define SENS_RAM6_SIZE                               0x00010000

#define CODEC_VAD_MAX_BUF_SIZE                  0x00018000
#define SENS_MAILBOX_SIZE                       0x10
#if (SENS_FIR_LMS_SIZE > 0)
#define SENS_MAILBOX_BASE                  (SENS_RAM4_BASE + SENS_RAM4_SIZE - SENS_MAILBOX_SIZE)
#else
#define SENS_MAILBOX_BASE                  (SENS_RAM6_BASE + SENS_RAM6_SIZE - SENS_MAILBOX_SIZE)
#endif

#if (SENS_FIR_LMS_SIZE > 0)
#define SENS_FIR_LMS_OFFSET                (SENS_RAM5_SIZE - SENS_FIR_LMS_SIZE)
#define SENS_FIR_LMS_BASE	               (SENS_RAMX5_BASE + SENS_FIR_LMS_OFFSET)
#endif

#ifdef __NuttX__
#define AP_MAILBOX_SIZE                         0x18000
#define CP_SUBSYS_ON_PSRAM
#endif

#ifndef AP_MAILBOX_SIZE
#define AP_MAILBOX_SIZE                         0
#endif

#ifdef LARGE_SENS_RAM
#define SENS_RAM_BASE    SENS_RAM3_BASE
#define SENS_RAMX_BASE    SENS_RAMX3_BASE
#else
#define SENS_RAM_BASE    SENS_RAM4_BASE
#define SENS_RAMX_BASE    SENS_RAMX4_BASE
#endif

#ifndef SENS_RAM_SIZE
#if (SENS_FIR_LMS_SIZE > 0) // for gen share ram
#define SENS_RAM_SIZE    (SENS_RAM5_BASE + SENS_RAM5_SIZE - SENS_RAM_BASE- SENS_MAILBOX_SIZE)
#else
#define SENS_RAM_SIZE    (SENS_RAM6_BASE + SENS_RAM6_SIZE - SENS_RAM_BASE- SENS_MAILBOX_SIZE)
#endif
#endif

#ifdef CHIP_SUBSYS_SENS
#define RAM_BASE    SENS_RAM_BASE
#define RAMX_BASE   SENS_RAMX_BASE
#if (SENS_FIR_LMS_SIZE > 0) // for lds ram
#define RAM_SIZE   (SENS_MAILBOX_BASE - SENS_RAM_BASE)
#else
#define RAM_SIZE    SENS_RAM_SIZE
#endif
#else /* !CHIP_SUBSYS_SENS */

#if defined(CP_IN_SAME_EE) && !defined(MCU_RAM_ONLY)
#ifdef __BT_RAM_DISTRIBUTION__
#ifndef RAMCPX_SIZE
#define RAMCPX_SIZE                             (RAMX7_BASE - RAMX6_BASE)
#endif
#define RAMCPX_BASE                             (RAMX6_BASE)

#ifndef RAMCP_SIZE
#define RAMCP_SIZE                              (RAM8_SIZE + RAMX8_BASE - RAMX7_BASE)
#endif
#define RAMCP_BASE                              (RAMCPX_BASE + RAMCPX_SIZE - RAMX6_BASE + RAM6_BASE)

#define RAMCP_TOP                               (RAMCP_BASE + RAMCP_SIZE)
#else /* !__BT_RAMRUN__ */
#ifndef RAMCPX_SIZE
#define RAMCPX_SIZE                             (RAMX1_BASE - RAMX0_BASE)
#endif
#define RAMCPX_BASE                             (RAMX0_BASE)

#ifndef RAMCP_SIZE
#define RAMCP_SIZE                              (RAM2_BASE - RAM1_BASE)
#endif
#define RAMCP_BASE                              (RAMCPX_BASE + RAMCPX_SIZE - RAMX0_BASE + RAM0_BASE)
#endif

#define RAMCP_TOP                               (RAMCP_BASE + RAMCP_SIZE)
#elif defined(CP_AS_SUBSYS) || defined(CHIP_ROLE_CP)
#define RAMCP_SUBSYS_BASE                       RAM0_BASE
#define RAMCP_SUBSYS_SIZE                       (RAM2_BASE - RAM0_BASE)
#define CP_SUBSYS_MAILBOX_SIZE                  0x20

#if AP_MAILBOX_SIZE == 0
#define CP_SUBSYS_MAILBOX_BASE                  (RAMCP_SUBSYS_BASE + RAMCP_SUBSYS_SIZE - CP_SUBSYS_MAILBOX_SIZE)
#else
#ifdef LARGE_SENS_RAM
#define CP_SUBSYS_MAILBOX_BASE                  (RAM6_BASE + RAM6_SIZE - CP_SUBSYS_MAILBOX_SIZE)
#else
#define CP_SUBSYS_MAILBOX_BASE                  (RAM7_BASE + RAM7_SIZE - CP_SUBSYS_MAILBOX_SIZE)
#endif
#endif

#define RAMCP_SIZE                              0

#ifdef CP_SUBSYS_ON_PSRAM
#define RAMCP_TOP                               RAM0_BASE
#else
#define RAMCP_TOP                               (RAMCP_SUBSYS_BASE + RAMCP_SUBSYS_SIZE)
#endif

#if defined(CHIP_ROLE_CP) && defined(CP_SUBSYS_ON_PSRAM) && (AP_MAILBOX_SIZE>0)
#define RAM_NC_SIZE                             (0x2000-CP_SUBSYS_MAILBOX_SIZE)
#define RAM_NC_BASE                             (CP_SUBSYS_MAILBOX_BASE - RAM_NC_SIZE)
#endif

#else
#define RAMCP_TOP                               RAM0_BASE
#endif


#ifdef CORE_SLEEP_POWER_DOWN

#ifdef __BT_RAMRUN_NEW__
/* RAM0-RAM3: 640K MCU volatile memory
 * RAM4-RAM6: 768K BT share memory
 */
#error "New LDS configuration is needed"
#endif

#ifdef MEM_POOL_BASE
#if (MEM_POOL_BASE < RAMCP_TOP)
#error "Bad MEM_POOL_BASE with CORE_SLEEP_POWER_DOWN"
#endif
#else
#define MEM_POOL_BASE                           RAMCP_TOP
#endif

#ifdef MEM_POOL_SIZE
#if (MEM_POOL_BASE + MEM_POOL_SIZE > RAM_NORM_BASE)
#error "Bad MEM_POOL_SIZE with CORE_SLEEP_POWER_DOWN"
#endif
#else
#define MEM_POOL_SIZE                           (RAM4_BASE - MEM_POOL_BASE)
#endif

#ifdef RAM_NORM_BASE
#if (RAM_NORM_BASE < MEM_POOL_BASE)
#error "Bad RAM_BASE with CORE_SLEEP_POWER_DOWN"
#endif
#else
#define RAM_NORM_BASE                                (MEM_POOL_BASE + MEM_POOL_SIZE)
#endif
#define RAMX_NORM_BASE                               (RAM_NORM_BASE - RAM0_BASE + RAMX0_BASE)

#else /* !CORE_SLEEP_POWER_DOWN */

#ifdef CHIP_ROLE_CP
#define RAM_NORM_BASE                                RAMCP_SUBSYS_BASE
#elif defined(RAM_NORM_BASE)
#if (RAM_NORM_BASE < RAMCP_TOP)
#error "Bad RAM_BASE"
#endif
#else
#define RAM_NORM_BASE                                RAMCP_TOP
#endif
#define RAMX_NORM_BASE                               (RAM_NORM_BASE - RAM0_BASE + RAMX0_BASE)

#ifdef __BT_RAMRUN_NEW__
#if (RAM_NORM_BASE >= RAM4_BASE)
#error "Bad RAM_BASE with BT ramrun"
#endif
#define RAM_NORM_SIZE                                (RAM4_BASE - RAM_NORM_BASE - AP_MAILBOX_SIZE)
#endif

#endif /* !CORE_SLEEP_POWER_DOWN */

#ifdef CHIP_ROLE_CP
#define RAM_NORM_SIZE                                (CP_SUBSYS_MAILBOX_BASE - RAMCP_SUBSYS_BASE)
#elif defined(RAM_NORM_SIZE)
#if (RAM_NORM_BASE + RAM_NORM_SIZE) > (RAM9_BASE + RAM9_SIZE)
#error "Bad RAM_SIZE"
#endif
#else
#ifdef LARGE_SENS_RAM
#define RAM_NORM_SIZE                                (RAM6_BASE + RAM6_SIZE - RAM_NORM_BASE - AP_MAILBOX_SIZE)
#else
#define RAM_NORM_SIZE                                (RAM7_BASE + RAM7_SIZE - RAM_NORM_BASE - AP_MAILBOX_SIZE)
#endif
#endif

#if defined(ARM_CMSE) || defined(ARM_CMNS)
/*MPC: SRAM block size: 0x8000, FLASH block size 0x40000*/
#ifdef LARGE_SE_RAM
#define RAM_S_SIZE                              0x00037000
#else
#define RAM_S_SIZE                              0x00027000
#endif
#define RAM_NSC_SIZE                            0x00001000
#ifndef FLASH_S_SIZE
#define FLASH_S_SIZE                            0x00080000
#endif

#undef RAM_BASE
#undef RAMX_BASE
#undef RAM_SIZE
#undef RAMCP_BASE
#undef RAMCPX_BASE
#undef RAMCP_SIZE
#undef RAMCPX_SIZE
#undef RAM_NORM_BASE
#undef RAMX_NORM_BASE
#undef RAM_NORM_SIZE

#define RAMCPX_BASE                             (RAMX0_BASE + RAM_S_SIZE + RAM_NSC_SIZE)
#define RAMCPX_SIZE                             0x20000
#define RAMCP_BASE                              (RAM0_BASE + RAM_S_SIZE + RAM_NSC_SIZE + RAMCPX_SIZE)
#define RAMCP_SIZE                              0x30000
#define RAM_NS_BASE                             (RAMCP_BASE + RAMCP_SIZE)
#define RAMX_NS_BASE                            (RAMCPX_BASE + RAMCPX_SIZE + RAMCP_SIZE)
#ifdef LARGE_SENS_RAM
#define RAM_NS_SIZE                             (RAM6_BASE + RAM6_SIZE - RAM_NORM_BASE)
#else
#define RAM_NS_SIZE                             (RAM7_BASE + RAM7_SIZE - RAM_NORM_BASE)
#endif
#define RAM_S_BASE                              (RAM_NORM_BASE)
#define RAMX_S_BASE                             (RAMX_NORM_BASE)

#if defined(ARM_CMNS)
#define RAM_NORM_BASE                                RAM_NS_BASE
#define RAMX_NORM_BASE                               RAMX_NS_BASE
#define RAM_NORM_SIZE                                RAM_NS_SIZE
#else
#if ((RAM_S_SIZE + RAM_NSC_SIZE) & (0x8000-1))
#error "RAM_S_SIZE should be 0x8000 aligned"
#endif
#if (FLASH_S_SIZE & (0x40000-1))
#error "FLASH_S_SIZE should be 0x40000 aligned"
#endif

#define RAM_NORM_BASE                                RAM0_BASE
#define RAMX_NORM_BASE                               RAMX0_BASE
#define RAM_NORM_SIZE                                RAM_S_SIZE
#ifndef NS_APP_START_OFFSET
#define NS_APP_START_OFFSET                     (FLASH_S_SIZE)
#endif
#ifndef FLASH_REGION_SIZE
#define FLASH_REGION_SIZE                       FLASH_S_SIZE
#endif
#endif
#else
#define RAM_S_BASE                              RAM_NS_TO_S(RAM_NORM_BASE)
#define RAM_S_SIZE                              RAM_NORM_SIZE
#define RAMX_S_BASE                             RAM_NS_TO_S(RAMX_NORM_BASE)

#define RAM_NS_BASE                             RAM_S_TO_NS(RAM_NORM_BASE)
#define RAM_NS_SIZE                             RAM_NORM_SIZE
#define RAMX_NS_BASE                            RAM_S_TO_NS(RAMX_NORM_BASE)
#endif /* defined(ARM_CMSE) || defined(ARM_CMNS) */

#if defined(NO_TRUSTZONE) || defined(ARM_CMNS) || \
        !(defined(ARM_CMSE) || defined(ROM_BUILD) || defined(PROGRAMMER))
#define RAM_BASE                                RAM_NS_BASE
#define RAM_SIZE                                RAM_NS_SIZE
#define RAMX_BASE                               RAMX_NS_BASE
#else
#define RAM_BASE                                RAM_S_BASE
#define RAM_SIZE                                RAM_S_SIZE
#define RAMX_BASE                               RAMX_S_BASE
#endif
#endif

#define REAL_FLASH_BASE                         0x2C000000
#define REAL_FLASH_NC_BASE                      0x28000000
#define REAL_FLASHX_BASE                        0x0C000000
#define REAL_FLASHX_NC_BASE                     0x08000000

#define REAL_FLASH1_BASE                        0x34000000
#define REAL_FLASH1_NC_BASE                     0x30000000
#define REAL_FLASH1X_BASE                       0x14000000
#define REAL_FLASH1X_NC_BASE                    0x10000000

#define SENS_FLASH_BASE                         0x26000000
#define SENS_FLASH_NC_BASE                      0x22000000
#define SENS_FLASHX_BASE                        0x06000000
#define SENS_FLASHX_NC_BASE                     0x02000000

#define PSRAM_BASE                              0x3C000000
#define PSRAM_NC_BASE                           0x38000000
#define PSRAMX_BASE                             0x1C000000
#define PSRAMX_NC_BASE                          0x18000000

#if !defined(CHIP_ROLE_CP) && !defined(CHIP_SUBSYS_SENS)
#define ICACHE_CTRL_BASE                        0x03FFA000
#define ICACHE_SIZE                             0x00008000
#define DCACHE_CTRL_BASE                        0x23FFA000
#define DCACHE_SIZE                             0x00008000
/*
#define ICACHECP_CTRL_BASE                      ICACHE_CTRL_BASE
#define ICACHECP_SIZE                           ICACHE_SIZE
#define DCACHECP_CTRL_BASE                      DCACHE_CTRL_BASE
#define DCACHECP_SIZE                           DCACHE_SIZE
*/
#endif

#define CMU_BASE                                0x40000000
#define MCU_WDT_BASE                            0x40001000
#define MCU_TIMER0_BASE                         0x40002000
#define MCU_TIMER1_BASE                         0x40003000
#define MCU_TIMER2_BASE                         0x40004000

#ifndef CHIP_ROLE_CP
#define I2C0_BASE                               0x40005000
#define I2C1_BASE                               0x40006000
#define SPI_BASE                                0x40007000
#define TRNG_BASE                               0x40008000
#define ISPI_BASE                               0x40009000
#define UART5_BASE                              0x4000A000
#define UART0_BASE                              0x4000B000
#define UART1_BASE                              0x4000C000
#define UART2_BASE                              0x4000D000
#define BTPCM_BASE                              0x4000E000
#define I2S0_BASE                               0x4000F000
#define SPDIF0_BASE                             0x40010000
#define I2S1_BASE                               0x40011000
#define SEC_ENG_BASE                            0x40020000
#define SEC_CTRL_BASE                           0x40030000
#define MPC_PSRAM0_BASE                         0x40031000
#define MPC_FLASH0_BASE                         0x40032000
#define MPC_C1CC_C1DC_BASE                      0x40033000
#define MPC_SRAM0_BASE                          0x40034000
#define SPY_SRAM0_BASE                          0x40050020
#define MPC_SRAM1_BASE                          0x40035000
#define SPY_SRAM1_BASE                          0x40050040
#define MPC_SRAM2_BASE                          0x40036000
#define SPY_SRAM2_BASE                          0x40050060
#define MPC_SRAM3_BASE                          0x40037000
#define SPY_SRAM3_BASE                          0x40050080
#define MPC_SRAM4_BASE                          0x40038000
#define SPY_SRAM4_BASE                          0x400500A0
#define MPC_SRAM5_BASE                          0x40039000
#define SPY_SRAM5_BASE                          0x400500C0
#define MPC_SRAM6_BASE                          0x4003A000
#define SPY_SRAM6_BASE                          0x400500E0
#define MPC_SRAM7_BASE                          0x4003B000
#define SPY_SRAM7_BASE                          0x40050100
#define MPC_SRAM8_BASE                          0x4003C000
#define SPY_SRAM8_BASE                          0x40050120
#define MPC_SRAM9_BASE                          0x4003D000
#define SPY_SRAM9_BASE                          0x40050140
#define MPC_C0CC_BASE                           0x4003E000
#define MPC_C0DC_BASE                           0x4003F000

#define PAGE_SPY_BASE                           0x40050000
#define I2C2_BASE                               0x40060000
#define I2C3_BASE                               0x40061000
#define I2C4_BASE                               0x40062000
#define I2C5_BASE                               0x40063000
#define I2C6_BASE                               0x40064000
#define I2C7_BASE                               0x40065000
#define UART3_BASE                              0x40066000
#define SPILCD_BASE                             0x40067000
#define SPIDPD_BASE                             0x40068000
#define UART4_BASE                              0x40069000
#elif defined(TRACE_CPU_IMM_OUTPUT)
#define UART0_BASE                              0x4000B000
#define UART1_BASE                              0x4000C000
#define UART2_BASE                              0x4000D000
#endif

#define AON_CMU_BASE                            0x40080000
#define AON_GPIO_BASE                           0x40081000
#define AON_WDT_BASE                            0x40082000
#define AON_PWM_BASE                            0x40083000
#define AON_TIMER0_BASE                         0x40084000
#define AON_PSC_BASE                            0x40085000
#define AON_IOMUX_BASE                          0x40086000
#define I2C_SLAVE_BASE                          0x40087000
#define AON_SEC_CTRL_BASE                       0x40088000
#define AON_GPIO1_BASE                          0x40089000
#define AON_GPIO2_BASE                          0x4008A000
#define AON_TIMER1_BASE                         0x4008B000
#define AON_PWM1_BASE                           0x4008C000

#ifndef CHIP_ROLE_CP
#define REAL_FLASH1_CTRL_BASE                   0x40500000
#define SDMMC0_BASE                             0x40110000
#define AUDMA_BASE                              0x40120000
#define GPDMA_BASE                              0x40130000
#define REAL_FLASH_CTRL_BASE                    0x40140000
#define BTDUMP_BASE                             0x40150000
#define PSRAM_CTRL_BASE                         0x40160000
#define USB_BASE                                0x40180000
#define SEDMA_BASE                              0x401D0000
#define CPUDUMP_BASE                            0x401E0000
#define JPEG_BASE                               0x401F0000

#define LCDC_BASE                               0x40200000
#define GPU_BASE                                0x40220000
#endif

#define CODEC_BASE                              0x40300000
#define SDMMC1_BASE                             0x40400000
#define I3C0_BASE                               0x40640000

#ifndef BTC_ROM_RAM_BASE
#define BTC_ROM_RAM_BASE                        0xA0000000
#endif

#ifndef BTC_ROM_RAM_SIZE
#define BTC_ROM_RAM_SIZE                        0x000C0000
#endif

#define BTC_MIN_ROM_RAM_BASE                    0xA0000000
#define BT_SUBSYS_BASE                          0xA0000000
#define BT_RAM_BASE                             0xC0000000
#define BT_RAM_SIZE                             0x00010000
#define BT_EXCH_MEM_BASE                        0xD0200000
#define BT_EXCH_MEM_SIZE                        0x00010000
#define SLE_EXCH_MEM_BASE                       0xD0290000
#define SLE_EXCH_MEM_SIZE                       0x00006000
#define BT_UART_BASE                            0xD0300000
#define BT_CMU_BASE                             0xD0330000
#define BTC_CPUDUMP_BASE                        0xD0700000

#define SENS_ICACHE_CTRL_BASE                   0x07FFE000

#define SENS_CMU_BASE                           0x50000000
#define SENS_WDT_BASE                           0x50001000
#define SENS_TIMER0_BASE                        0x50002000
#define SENS_TIMER1_BASE                        0x50003000
#define SENS_TIMER2_BASE                        0x50004000
#define SENS_I2C0_BASE                          0x50005000
#define SENS_I2C1_BASE                          0x50006000
#define SENS_SPI_BASE                           0x50007000
#define SENS_SPILCD_BASE                        0x50008000
#define SENS_ISPI_BASE                          0x50009000
#define SENS_UART0_BASE                         0x5000A000
#define SENS_UART1_BASE                         0x5000B000
#define SENS_BTPCM_BASE                         0x5000C000
#define SENS_I2S0_BASE                          0x5000D000
#define SENS_I2C2_BASE                          0x5000E000
#define SENS_I2C3_BASE                          0x5000F000
#define SENS_CAP_SENSOR_BASE                    0x50010000
#define SENS_PAGE_SPY_BASE                      0x50011000

#define SENS_SENSOR_ENG0_BASE                   0x50100000
#define SENS_CODEC_SENSOR_BASE                  0x50110000
#define SENS_SDMA_BASE                          0x50120000
#define SENS_TEP_BASE                           0x50130000
#define SENS_AVS_BASE                           0x50140000
#define SENS_FLASH_CTRL_BASE                    0x50160000

#define SENS_VAD_BASE                           0x50200000

#define CAP_SENSOR_BASE                         SENS_CAP_SENSOR_BASE

#ifdef CHIP_SUBSYS_SENS
/* For sensor hub sub-system */
#undef SPDIF0_BASE
#undef SEC_ENG_BASE
#undef BTDUMP_BASE
#undef TRNG_BASE
#undef SDMMC0_BASE
#undef SDMMC1_BASE
#undef LCDC_BASE
#undef UART2_BASE
#undef UART3_BASE
#undef UART4_BASE
#undef UART5_BASE
#undef JPEG_BASE
#undef I2C4_BASE
#undef I2C5_BASE
#undef I2C6_BASE
#undef I2C7_BASE
#undef SPIDPD_BASE
#undef I3C0_BASE

#undef ICACHE_CTRL_BASE
#undef ICACHE_SIZE
#define ICACHE_CTRL_BASE                        SENS_ICACHE_CTRL_BASE
#define ICACHE_SIZE                             0x00004000
#undef DCACHE_CTRL_BASE
#undef DCACHE_SIZE

#undef I2C0_BASE
#define I2C0_BASE                               SENS_I2C0_BASE
#undef I2C1_BASE
#define I2C1_BASE                               SENS_I2C1_BASE
#undef I2C2_BASE
#define I2C2_BASE                               SENS_I2C2_BASE
#undef I2C3_BASE
#define I2C3_BASE                               SENS_I2C3_BASE
#undef SPI_BASE
#define SPI_BASE                                SENS_SPI_BASE
#undef SPILCD_BASE
#define SPILCD_BASE                             SENS_SPILCD_BASE
#undef ISPI_BASE
#define ISPI_BASE                               SENS_ISPI_BASE
#undef UART0_BASE
#define UART0_BASE                              SENS_UART0_BASE
#undef UART1_BASE
#define UART1_BASE                              SENS_UART1_BASE
#undef BTPCM_BASE
#define BTPCM_BASE                              SENS_BTPCM_BASE
#undef I2S0_BASE
#undef I2S1_BASE
#define I2S0_BASE                               SENS_I2S0_BASE
#undef PAGE_SPY_BASE
#define PAGE_SPY_BASE                           SENS_PAGE_SPY_BASE
#undef SENSOR_ENG_BASE
#define SENSOR_ENG_BASE                         SENS_SENSOR_ENG_BASE
#undef AUDMA_BASE
#define AUDMA_BASE                              SENS_SDMA_BASE
#undef GPDMA_BASE
#undef TEP_BASE
#define TEP_BASE                                SENS_TEP_BASE
#undef AVS_BASE
#define AVS_BASE                                SENS_AVS_BASE

#define TIMER0_BASE                             AON_TIMER1_BASE
#define TIMER1_BASE                             SENS_TIMER1_BASE

#ifdef SUBSYS_FLASH_BOOT
#define FLASH_BASE                              SENS_FLASH_BASE
#define FLASH_NC_BASE                           SENS_FLASH_NC_BASE
#define FLASHX_BASE                             SENS_FLASHX_BASE
#define FLASHX_NC_BASE                          SENS_FLASHX_NC_BASE

#define FLASH_CTRL_BASE                         SENS_FLASH_CTRL_BASE
#else
#define NO_FLASH_BASE_ACCESS
#endif

#define SENSOR_ENG0_BASE                        SENS_SENSOR_ENG0_BASE

#else /* !CHIP_SUBSYS_SENS */

#if defined(CP_AS_SUBSYS) || defined(CHIP_ROLE_CP)
#define CP_SUBSYS_TIMER0_BASE                   MCU_TIMER0_BASE
#define CP_SUBSYS_TIMER1_BASE                   MCU_TIMER2_BASE
#endif

#ifdef CHIP_ROLE_CP
#define TIMER0_BASE                             CP_SUBSYS_TIMER0_BASE
#define TIMER1_BASE                             CP_SUBSYS_TIMER1_BASE
#else /* !CHIP_ROLE_CP */
#define TIMER0_BASE                             AON_TIMER0_BASE
#define TIMER1_BASE                             MCU_TIMER1_BASE
#ifndef CP_AS_SUBSYS
#define TIMER2_BASE                             MCU_TIMER2_BASE
#endif
#endif /* CHIP_ROLE_CP */

#ifndef CHIP_ROLE_CP
#ifdef ALT_BOOT_FLASH
#define FLASH_BASE                              REAL_FLASH1_BASE
#define FLASH_NC_BASE                           REAL_FLASH1_NC_BASE
#define FLASHX_BASE                             REAL_FLASH1X_BASE
#define FLASHX_NC_BASE                          REAL_FLASH1X_NC_BASE

#define FLASH1_BASE                             REAL_FLASH_BASE
#define FLASH1_NC_BASE                          REAL_FLASH_NC_BASE
#define FLASH1X_BASE                            REAL_FLASHX_BASE
#define FLASH1X_NC_BASE                         REAL_FLASHX_NC_BASE

#define FLASH_CTRL_BASE                         REAL_FLASH1_CTRL_BASE
#define FLASH1_CTRL_BASE                        REAL_FLASH_CTRL_BASE
#else
#define FLASH_BASE                              REAL_FLASH_BASE
#define FLASH_NC_BASE                           REAL_FLASH_NC_BASE
#define FLASHX_BASE                             REAL_FLASHX_BASE
#define FLASHX_NC_BASE                          REAL_FLASHX_NC_BASE

#define FLASH1_BASE                             REAL_FLASH1_BASE
#define FLASH1_NC_BASE                          REAL_FLASH1_NC_BASE
#define FLASH1X_BASE                            REAL_FLASH1X_BASE
#define FLASH1X_NC_BASE                         REAL_FLASH1X_NC_BASE

#define FLASH_CTRL_BASE                         REAL_FLASH_CTRL_BASE
#define FLASH1_CTRL_BASE                        REAL_FLASH1_CTRL_BASE
#endif

#if 1 /* defined(PROGRAMMER) && !defined(PROGRAMMER_INFLASH) */
#define FLASH2_BASE                             SENS_FLASH_BASE
#define FLASH2_NC_BASE                          SENS_FLASH_NC_BASE
#define FLASH2X_BASE                            SENS_FLASHX_BASE
#define FLASH2X_NC_BASE                         SENS_FLASHX_NC_BASE

#define FLASH2_CTRL_BASE                        SENS_FLASH_CTRL_BASE
#endif
#else // !CHIP_ROLE_CP

#define NO_FLASH_BASE_ACCESS
#ifndef PSRAMCP_BASE
#define PSRAMCP_BASE                            PSRAM_BASE
#endif
#ifdef CP_SUBSYS_ON_PSRAM
#undef RAM_BASE
#undef RAMX_BASE
#undef RAM_SIZE
#undef RAMX_SIZE
#define RAM_BASE                                PSRAMCP_BASE
#define RAMX_BASE                               PSRAM_TO_PSRAMX(RAM_BASE)
#define RAM_SIZE                                PSRAM_SIZE
#define RAMX_SIZE                               PSRAM_SIZE
#endif
#endif // CHIP_ROLE_CP
#endif /* !CHIP_SUBSYS_SENS */

#define AON_TIMER_FREE_TIMER                    AON_TIMER0_BASE

/*resize fpga ram*/
#ifdef FPGA
#ifdef RAM_BASE
#undef RAM_BASE
#define RAM_BASE                                0x20000000
#endif

#ifdef RAMX_BASE
#undef RAMX_BASE
#define RAMX_BASE                               (RAM_BASE - 0x20000000 + 0x200000)
#endif

#ifndef FPGA_RAM_TOTAL_SIZE
#define FPGA_RAM_TOTAL_SIZE                     0xc0000
#endif

#ifdef RAM_SIZE
#undef RAM_SIZE
#define RAM_SIZE                                FPGA_RAM_TOTAL_SIZE
#endif

#ifdef RAMCP_SIZE
#undef RAMCP_SIZE
#define RAMCP_SIZE                              0
#endif
#endif

#define IOMUX_BASE                              AON_IOMUX_BASE
#define GPIO_BASE                               AON_GPIO_BASE
#define GPIO1_BASE                              AON_GPIO1_BASE
#define GPIO2_BASE                              AON_GPIO2_BASE
#define PWM_BASE                                AON_PWM_BASE
#define PWM1_BASE                               AON_PWM1_BASE
#define WDT_BASE                                AON_WDT_BASE

/* For linker scripts */
#if defined(CHIP_SUBSYS_SENS)
#define VECTOR_SECTION_SIZE                     320
#else
#define VECTOR_SECTION_SIZE                     440
#endif

#define REBOOT_PARAM_SECTION_SIZE               64
#define ROM_BUILD_INFO_SECTION_SIZE             40
#define ROM_EXPORT_FN_SECTION_SIZE              128
#define BT_INTESYS_MEM_OFFSET                   0x00004000

/* For module features */
#define CODEC_FREQ_CRYSTAL                      CODEC_FREQ_24M
#define CODEC_FREQ_EXTRA_DIV                    2
#define CODEC_PLL_DIV                           16
#define CODEC_CMU_DIV                           8
#define CODEC_PLAYBACK_BIT_DEPTH                24
#define CODEC_HAS_FIR
#define GPADC_CTRL_VER                          3
#define GPADC_VALUE_BITS                        16
#define GPADC_HAS_VSYS_DIV
#define GPADC_HAS_EXT_SLOPE_CAL2
#define GPADC_SAR_INPUT_BUF_CTRL
#define GPIO_HAS_BOTH_EDGE_IRQ
#define GPADC_VBAT_VOLT_DIV                     5
#define APP_BATTERY_GPADC_CH_NUM                HAL_GPADC_CHAN_6
#define SEC_ENG_HAS_HASH
#define DCDC_CLOCK_CONTROL
#define PWRKEY_IRQ_IN_PMU
#ifndef AUD_SECTION_STRUCT_VERSION
#define AUD_SECTION_STRUCT_VERSION              4
#endif
#define PAGE_SPY_VER                            2
#define NO_SUBSYS_FLASH_ADDR_ACCESS
#define UARTCOM_USE_LED2

/* TODO: MCU has 64M-byte space but SENS only has 16M-byte space */
#define HAL_NORFLASH_ADDR_MASK                  0x00FFFFFF
#define RMT_IPC_API_ENABLE
#if defined(ROM_BUILD) || (defined(PROGRAMMER) && defined(CHIP_HAS_USB))
#define CPU_TO_DEV_ADDR_REMAP
#endif

#define DUMPPC_TOTAL_NUM                        (0x400/4)
#define DUMPPC_NUM                              100

#if defined (__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U)
#define NO_FLASH_S_ACCESS
#endif

#define PROGRAMMER_HAL_CHIP_INIT

/* For boot struct version */
#ifndef SECURE_BOOT_VER
#define SECURE_BOOT_VER                         5
#endif

/* For ROM export functions */
#define NO_MEMMOVE
#define NO_EXPORT_QSORT
#define NO_EXPORT_BSEARCH
#define NO_EXPORT_VSSCANF

#ifdef CP_FLASH_ACCESS
#define CP_MPU_FLASHCP_ENTRY                   \
    { FLASH_BASE, 0x04000000, MPU_ATTR_READ_EXEC, MEM_ATTR_NORMAL_WRITE_BACK, }, \
    { FLASHX_BASE, 0x04000000, MPU_ATTR_READ_EXEC, MEM_ATTR_NORMAL_WRITE_BACK, },

    /* { BTH_ICACHECP_RAM_BASE, BTH_ICACHECP_SIZE, MPU_ATTR_READ_WRITE_EXEC, MEM_ATTR_NORMAL_NON_CACHEABLE, }, */
#else
#define CP_MPU_FLASHCP_ENTRY
#endif

#define CP_MPU_INIT_TABLE                       \
    CP_MPU_FLASHCP_ENTRY                        \
    { RAM0_BASE,              RAM_TOTAL_SIZE,                            MPU_ATTR_READ_WRITE_EXEC,     MEM_ATTR_INT_SRAM}, \
    { ROMX_BASE,              RAMX0_BASE+RAM_TOTAL_SIZE-ROMX_BASE,       MPU_ATTR_READ_EXEC,           MEM_ATTR_INT_SRAM}, \
    { ROM_BASE,               ROM_TOTAL_SIZE,                            MPU_ATTR_READ_EXEC,           MEM_ATTR_INT_SRAM}, \
    { CP_DEV_REG_START,       CP_DEV_REG_SIZE,                           MPU_ATTR_READ_WRITE,          MEM_ATTR_DEVICE},

    /*{ RAM_NS_TO_S(ROMX_BASE), ROM_TOTAL_SIZE,                            MPU_ATTR_READ_EXEC,           MEM_ATTR_INT_SRAM}, */
    /*{ RAM_NS_TO_S(ROM_BASE),  ROM_TOTAL_SIZE,                            MPU_ATTR_READ_EXEC,           MEM_ATTR_INT_SRAM}, */

#ifdef __cplusplus
}
#endif

#endif
