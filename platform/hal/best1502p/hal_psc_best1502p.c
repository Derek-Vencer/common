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
#include "plat_addr_map.h"
#include "cmsis.h"
#include "hal_chipid.h"
#include "hal_gpio.h"
#include "hal_location.h"
#include "hal_psc.h"
#include "hal_sleep.h"
#include "hal_timer.h"
#include CHIP_SPECIFIC_HDR(reg_psc)
#ifndef CORE_SLEEP_POWER_DOWN
#include CHIP_SPECIFIC_HDR(reg_aoncmu)
#endif

#define PSC_WRITE_ENABLE                    0xCAFE0000

enum PSC_INTR_MASK3_T {
    PSC_INTR_MASK3_GPIO2_15_0   = (0xFFFF << 0),
    PSC_INTR_MASK3_TIMER        = (1 << 16),
    PSC_INTR_MASK3_INTPMU       = (1 << 17),
    PSC_INTR_MASK3_WDT          = (1 << 18),
    PSC_INTR_MASK3_EXTPMU       = (1 << 19),
    PSC_INTR_MASK3_VAD          = (1 << 20),
    PSC_INTR_MASK3_CODEC        = (1 << 21),
    PSC_INTR_MASK3_WAKEUP_BT    = (1 << 22),
    PSC_INTR_MASK3_WAKEUP_SENS  = (1 << 23),
    PSC_INTR_MASK3_WAKEUP_MCU   = (1 << 24),
};

static struct AONPSC_T * const psc = (struct AONPSC_T *)AON_PSC_BASE;
#ifndef CORE_SLEEP_POWER_DOWN
static struct AONCMU_T * const aoncmu = (struct AONCMU_T *)AON_CMU_BASE;
#endif

static uint8_t dis_mod_map;
STATIC_ASSERT(sizeof(dis_mod_map) * 8 >= HAL_PSC_DISPLAY_MOD_QTY, "dis_mod_map size too small");

static uint32_t BOOT_BSS_LOC reg_7c_saved;
static uint32_t BOOT_BSS_LOC reg_f8_saved;
static uint32_t BOOT_BSS_LOC reg_fc_saved;
static uint32_t BOOT_BSS_LOC reg_44_saved;

void hal_psc_jtag_select_mcu(void)
{
    psc->REG_064 &= ~PSC_AON_CODEC_RESERVED(1 << 3);
    psc->REG_064 |= PSC_AON_CODEC_RESERVED(1 << 2);
}

void hal_psc_jtag_select_bt(void)
{
    psc->REG_064 &= ~PSC_AON_CODEC_RESERVED(1 << 3);
    psc->REG_064 &= ~PSC_AON_CODEC_RESERVED(1 << 2);
}

void hal_psc_jtag_select_sens(void)
{
    psc->REG_064 |= PSC_AON_CODEC_RESERVED(1 << 3);
}

#ifndef CORE_SLEEP_POWER_DOWN
void hal_psc_deepsleep_shr_ram_cfg(uint32_t ram_map)
{
    uint32_t val = GET_BITFIELD(aoncmu->SW_INFO, AON_CMU_DEBUG2_DEEPSLEEP_SHM_RETEN);
    val |= ram_map;
    aoncmu->SW_INFO = SET_BITFIELD(aoncmu->SW_INFO, AON_CMU_DEBUG2_DEEPSLEEP_SHM_RETEN, val);
}
#endif

void BOOT_TEXT_FLASH_LOC hal_psc_init(void)
{
    uint32_t mcu_wake_mask;
    uint32_t sens_wake_mask;
    uint32_t reg_07c;
    uint32_t reg_0fc;
    uint32_t reg_0f8;
    uint32_t reg_044;
    mcu_wake_mask = 0;
    sens_wake_mask = 0;

#ifdef INTPMU_IRQ_TO_SENS
    sens_wake_mask |= PSC_INTR_MASK3_INTPMU;
#else
    mcu_wake_mask |= PSC_INTR_MASK3_INTPMU;
#endif
#ifdef WDT_IRQ_TO_SENS
    sens_wake_mask |= PSC_INTR_MASK3_WDT;
#else
    mcu_wake_mask |= PSC_INTR_MASK3_WDT;
#endif
#ifdef EXTPMU_IRQ_TO_SENS
    sens_wake_mask |= PSC_INTR_MASK3_EXTPMU;
#else
    mcu_wake_mask |= PSC_INTR_MASK3_EXTPMU;
#endif
#ifdef CODEC_IRQ_TO_SENS
    sens_wake_mask |= PSC_INTR_MASK3_CODEC;
#else
    mcu_wake_mask |= PSC_INTR_MASK3_CODEC;
#endif

    // Setup MCU wakeup mask
    psc->REG_080 = 0;
    psc->REG_084 = 0;
    psc->REG_088 = PSC_INTR_MASK3_TIMER | PSC_INTR_MASK3_WAKEUP_MCU | mcu_wake_mask;

    // Setup BT wakeup mask
    psc->REG_098 = 0;
    psc->REG_09C = 0;
    psc->REG_0A0 = PSC_INTR_MASK3_WAKEUP_BT;

    // Setup SENS wakeup mask
    psc->REG_0B0 = 0;
    psc->REG_0B4 = 0;
    psc->REG_0B8 = PSC_INTR_MASK3_VAD | PSC_INTR_MASK3_WAKEUP_SENS | sens_wake_mask;

    // Setup power down/up timer
    psc->REG_008 = PSC_AON_MCU_POWERDN_TIMER4(3) | PSC_AON_MCU_POWERDN_TIMER5(0);
    psc->REG_00C = PSC_AON_MCU_POWERUP_TIMER2(0)| PSC_AON_MCU_POWERUP_TIMER4(0xa);

    psc->REG_048 = SET_BITFIELD(psc->REG_048, PSC_AON_MCU_RETMEM_PE_POWERDN_TIMER1, 1);

    reg_07c = psc->REG_07C;
    reg_07c |= PSC_AON_SRAM4_ISO_EN_REG | PSC_AON_SRAM5_ISO_EN_REG | PSC_AON_SRAM6_ISO_EN_REG | PSC_AON_SRAM7_ISO_EN_REG |
            PSC_AON_SRAM8_ISO_EN_REG | PSC_AON_SRAM9_ISO_EN_REG | PSC_AON_MCU_RETMEM_ISO_EN_REG;
    reg_07c &= ~(PSC_AON_SRAM4_ISO_EN_DR | PSC_AON_SRAM5_ISO_EN_DR | PSC_AON_SRAM6_ISO_EN_DR | PSC_AON_SRAM7_ISO_EN_DR |
            PSC_AON_SRAM8_ISO_EN_DR | PSC_AON_SRAM9_ISO_EN_DR | PSC_AON_MCU_RETMEM_ISO_EN_DR);
    psc->REG_07C = PSC_WRITE_ENABLE | reg_07c;

    reg_0fc = psc->REG_0FC;
    reg_044 = psc->REG_044;
#if defined(CORE_SLEEP_POWER_DOWN)
    //mcu ram
    reg_0fc |= PSC_AON_MCU_RETMEM_PDEN;
    //share mem sram4-8 256k
    reg_0fc |= PSC_AON_SRAM4_PE_PSW_EN_REG | PSC_AON_SRAM5_PE_PSW_EN_REG | PSC_AON_SRAM6_PE_PSW_EN_REG | PSC_AON_SRAM7_PE_PSW_EN_REG | PSC_AON_SRAM8_PE_PSW_EN_REG;
    reg_0fc &= ~(PSC_AON_SRAM4_CE_PSW_EN_REG | PSC_AON_SRAM5_CE_PSW_EN_REG | PSC_AON_SRAM6_CE_PSW_EN_REG | PSC_AON_SRAM7_CE_PSW_EN_REG | PSC_AON_SRAM8_CE_PSW_EN_REG |
            PSC_AON_SRAM4_PE_PSW_EN_DR | PSC_AON_SRAM5_PE_PSW_EN_DR | PSC_AON_SRAM6_PE_PSW_EN_DR | PSC_AON_SRAM7_PE_PSW_EN_DR | PSC_AON_SRAM8_PE_PSW_EN_DR);
    //share mem sram9 128k
    reg_044 |= PSC_AON_SRAM9_PE_PSW_EN_REG;
    reg_044 &= ~(PSC_AON_SRAM9_CE_PSW_EN_REG | PSC_AON_SRAM9_PE_PSW_EN_DR);
#else
    reg_0fc &= ~PSC_AON_MCU_RETMEM_PDEN;
    reg_044 &= ~PSC_AON_SRAM9_CE_PSW_EN_REG;
#endif
    psc->REG_0FC = PSC_WRITE_ENABLE | reg_0fc;
    psc->REG_044 = PSC_WRITE_ENABLE | reg_044;

    reg_0f8 = psc->REG_0F8;
    reg_0f8 |= PSC_AON_SRAM9_CE_PSW_EN_DR;
    reg_0f8 &= ~(PSC_AON_MCU_RETMEM_PE_PSW_EN_DR | PSC_AON_BTC_RETMEM_PDEN);

    psc->REG_0F8 = PSC_WRITE_ENABLE | reg_0f8;
    psc->REG_0EC = PSC_AON_REG_RAMRET_CFG(0x81);

// #ifndef ARM_CMSE
//     uint32_t ram_map;
//     // Power down unused share ram
//     ram_map = gen_bth_shr_ram_mask(RAM_S_TO_NS(RAM_BASE), RAM_SIZE)
// #if defined(MEM_POOL_BASE) && defined(MEM_POOL_SIZE)
//     | gen_bth_shr_ram_mask(RAM_S_TO_NS(MEM_POOL_BASE), MEM_POOL_SIZE)
// #endif
//     ;
// #ifndef CORE_SLEEP_POWER_DOWN
//     hal_psc_deepsleep_shr_ram_cfg(ram_map);
// #endif
//     ram_map = 0x003F & ~(ram_map);
//     hal_psc_shr_ram_config(ram_map, HAL_PSC_SHR_RAM_FORCE_PD);
// #endif

    hal_psc_bt_disable();
    hal_psc_display_disable(HAL_PSC_DISPLAY_MOD_ALL);
    hal_psc_sens_disable();

    // Save ram psw settings
    reg_7c_saved = psc->REG_07C;
    reg_f8_saved = psc->REG_0F8 & (PSC_AON_MCU_RETMEM_PE_PSW_EN_DR | PSC_AON_MCU_RETMEM_PE_PSW_EN_REG);
    reg_fc_saved = psc->REG_0FC;
    reg_44_saved = psc->REG_044 & (PSC_AON_SRAM9_PE_PSW_EN_DR | PSC_AON_SRAM9_PE_PSW_EN_REG);
}

uint32_t hal_psc_get_power_loop_cycle_cnt(void)
{
    return 6 + 14 + 14; // 1.5 + 4.5 + 14 + 14
}

void SRAM_TEXT_LOC hal_psc_mcu_auto_power_down(void)
{
    psc->REG_018 = PSC_WRITE_ENABLE | 0;
    psc->REG_000 = PSC_WRITE_ENABLE | PSC_AON_MCU_PG_AUTO_EN;
    psc->REG_010 = PSC_WRITE_ENABLE | PSC_AON_MCU_POWERDN_START;
}

void SRAM_TEXT_LOC hal_psc_mcu_auto_power_up(void)
{
    psc->REG_014 = PSC_WRITE_ENABLE | PSC_AON_MCU_POWERUP_START;
}


void BOOT_TEXT_FLASH_LOC hal_psc_display_enable(enum HAL_PSC_DISPLAY_MOD_T mod)
{
    uint32_t lock;
    bool set = true;

    if (mod > HAL_PSC_DISPLAY_MOD_QTY) {
        return;
    };

    lock = int_lock();
    if (mod < HAL_PSC_DISPLAY_MOD_ALL) {
        if (dis_mod_map) {
            set = false;
        }
        dis_mod_map |= (1 << mod);
    }
    int_unlock(lock);

    if (!set) {
        while (psc->REG_028 & PSC_AON_DIS_CLK_STOP_REG) {
			hal_sys_timer_delay(MS_TO_TICKS(1));
        }
        return;
    }

    psc->REG_028 = PSC_WRITE_ENABLE |
        PSC_AON_DIS_MEM_PSW_EN_DR |
        PSC_AON_DIS_PSW_EN_DR | PSC_AON_DIS_PSW_EN_REG |
        PSC_AON_DIS_RESETN_ASSERT_DR | PSC_AON_DIS_RESETN_ASSERT_REG |
        PSC_AON_DIS_ISO_EN_DR | PSC_AON_DIS_ISO_EN_REG |
        PSC_AON_DIS_CLK_STOP_DR | PSC_AON_DIS_CLK_STOP_REG;
    hal_sys_timer_delay(MS_TO_TICKS(1));
    psc->REG_028 = PSC_WRITE_ENABLE |
        PSC_AON_DIS_MEM_PSW_EN_DR |
        PSC_AON_DIS_PSW_EN_DR |
        PSC_AON_DIS_RESETN_ASSERT_DR | PSC_AON_DIS_RESETN_ASSERT_REG |
        PSC_AON_DIS_ISO_EN_DR | PSC_AON_DIS_ISO_EN_REG |
        PSC_AON_DIS_CLK_STOP_DR | PSC_AON_DIS_CLK_STOP_REG;
    hal_sys_timer_delay(MS_TO_TICKS(1));
    psc->REG_028 = PSC_WRITE_ENABLE |
        PSC_AON_DIS_MEM_PSW_EN_DR |
        PSC_AON_DIS_PSW_EN_DR |
        PSC_AON_DIS_RESETN_ASSERT_DR |
        PSC_AON_DIS_ISO_EN_DR | PSC_AON_DIS_ISO_EN_REG |
        PSC_AON_DIS_CLK_STOP_DR | PSC_AON_DIS_CLK_STOP_REG;
    psc->REG_028 = PSC_WRITE_ENABLE |
        PSC_AON_DIS_MEM_PSW_EN_DR |
        PSC_AON_DIS_PSW_EN_DR |
        PSC_AON_DIS_RESETN_ASSERT_DR |
        PSC_AON_DIS_ISO_EN_DR |
        PSC_AON_DIS_CLK_STOP_DR | PSC_AON_DIS_CLK_STOP_REG;
    psc->REG_028 = PSC_WRITE_ENABLE |
        PSC_AON_DIS_MEM_PSW_EN_DR |
        PSC_AON_DIS_PSW_EN_DR |
        PSC_AON_DIS_RESETN_ASSERT_DR |
        PSC_AON_DIS_ISO_EN_DR |
        PSC_AON_DIS_CLK_STOP_DR;
}

void BOOT_TEXT_FLASH_LOC hal_psc_display_disable(enum HAL_PSC_DISPLAY_MOD_T mod)
{
    uint32_t lock;
    bool set = true;

    if (mod > HAL_PSC_DISPLAY_MOD_QTY) {
        return;
    };

    lock = int_lock();

    if (mod < HAL_PSC_DISPLAY_MOD_ALL) {
        dis_mod_map &= ~(1 << mod);
        if (dis_mod_map) {
            set = false;
        }
    }

    if (set) {
        psc->REG_028 = PSC_WRITE_ENABLE |
            PSC_AON_DIS_MEM_PSW_EN_DR |
            PSC_AON_DIS_PSW_EN_DR |
            PSC_AON_DIS_RESETN_ASSERT_DR |
            PSC_AON_DIS_ISO_EN_DR |
            PSC_AON_DIS_CLK_STOP_DR | PSC_AON_DIS_CLK_STOP_REG;
        psc->REG_028 = PSC_WRITE_ENABLE |
            PSC_AON_DIS_MEM_PSW_EN_DR |
            PSC_AON_DIS_PSW_EN_DR |
            PSC_AON_DIS_RESETN_ASSERT_DR |
            PSC_AON_DIS_ISO_EN_DR | PSC_AON_DIS_ISO_EN_REG |
            PSC_AON_DIS_CLK_STOP_DR | PSC_AON_DIS_CLK_STOP_REG;
        psc->REG_028 = PSC_WRITE_ENABLE |
            PSC_AON_DIS_MEM_PSW_EN_DR |
            PSC_AON_DIS_PSW_EN_DR |
            PSC_AON_DIS_RESETN_ASSERT_DR | PSC_AON_DIS_RESETN_ASSERT_REG |
            PSC_AON_DIS_ISO_EN_DR | PSC_AON_DIS_ISO_EN_REG |
            PSC_AON_DIS_CLK_STOP_DR | PSC_AON_DIS_CLK_STOP_REG;
        psc->REG_028 = PSC_WRITE_ENABLE |
            PSC_AON_DIS_MEM_PSW_EN_DR |
            PSC_AON_DIS_PSW_EN_DR | PSC_AON_DIS_PSW_EN_REG |
            PSC_AON_DIS_RESETN_ASSERT_DR | PSC_AON_DIS_RESETN_ASSERT_REG |
            PSC_AON_DIS_ISO_EN_DR | PSC_AON_DIS_ISO_EN_REG |
            PSC_AON_DIS_CLK_STOP_DR | PSC_AON_DIS_CLK_STOP_REG;
        psc->REG_028 = PSC_WRITE_ENABLE |
            PSC_AON_DIS_MEM_PSW_EN_DR | PSC_AON_DIS_MEM_PSW_EN_REG |
            PSC_AON_DIS_PSW_EN_DR | PSC_AON_DIS_PSW_EN_REG |
            PSC_AON_DIS_RESETN_ASSERT_DR | PSC_AON_DIS_RESETN_ASSERT_REG |
            PSC_AON_DIS_ISO_EN_DR | PSC_AON_DIS_ISO_EN_REG |
            PSC_AON_DIS_CLK_STOP_DR | PSC_AON_DIS_CLK_STOP_REG;
    }

    int_unlock(lock);
}

void BOOT_TEXT_FLASH_LOC hal_psc_codec_enable(void)
{
    psc->REG_078 = PSC_WRITE_ENABLE |
        PSC_AON_CODEC_MEM_PSW_EN_DR |
        PSC_AON_CODEC_PSW_EN_DR | PSC_AON_CODEC_PSW_EN_REG |
        PSC_AON_CODEC_RESETN_ASSERT_DR | PSC_AON_CODEC_RESETN_ASSERT_REG |
        PSC_AON_CODEC_ISO_EN_DR | PSC_AON_CODEC_ISO_EN_REG |
        PSC_AON_CODEC_CLK_STOP_DR | PSC_AON_CODEC_CLK_STOP_REG;
    hal_sys_timer_delay(MS_TO_TICKS(1));
    psc->REG_078 = PSC_WRITE_ENABLE |
        PSC_AON_CODEC_MEM_PSW_EN_DR |
        PSC_AON_CODEC_PSW_EN_DR |
        PSC_AON_CODEC_RESETN_ASSERT_DR | PSC_AON_CODEC_RESETN_ASSERT_REG |
        PSC_AON_CODEC_ISO_EN_DR | PSC_AON_CODEC_ISO_EN_REG |
        PSC_AON_CODEC_CLK_STOP_DR | PSC_AON_CODEC_CLK_STOP_REG;
    hal_sys_timer_delay(MS_TO_TICKS(1));
    psc->REG_078 = PSC_WRITE_ENABLE |
        PSC_AON_CODEC_MEM_PSW_EN_DR |
        PSC_AON_CODEC_PSW_EN_DR |
        PSC_AON_CODEC_RESETN_ASSERT_DR |
        PSC_AON_CODEC_ISO_EN_DR | PSC_AON_CODEC_ISO_EN_REG |
        PSC_AON_CODEC_CLK_STOP_DR | PSC_AON_CODEC_CLK_STOP_REG;
    psc->REG_078 = PSC_WRITE_ENABLE |
        PSC_AON_CODEC_MEM_PSW_EN_DR |
        PSC_AON_CODEC_PSW_EN_DR |
        PSC_AON_CODEC_RESETN_ASSERT_DR |
        PSC_AON_CODEC_ISO_EN_DR |
        PSC_AON_CODEC_CLK_STOP_DR | PSC_AON_CODEC_CLK_STOP_REG;
    psc->REG_078 = PSC_WRITE_ENABLE |
        PSC_AON_CODEC_MEM_PSW_EN_DR |
        PSC_AON_CODEC_PSW_EN_DR |
        PSC_AON_CODEC_RESETN_ASSERT_DR |
        PSC_AON_CODEC_ISO_EN_DR |
        PSC_AON_CODEC_CLK_STOP_DR;
}

void BOOT_TEXT_FLASH_LOC hal_psc_codec_disable(void)
{
    psc->REG_078 = PSC_WRITE_ENABLE |
        PSC_AON_CODEC_MEM_PSW_EN_DR |
        PSC_AON_CODEC_PSW_EN_DR |
        PSC_AON_CODEC_RESETN_ASSERT_DR |
        PSC_AON_CODEC_ISO_EN_DR |
        PSC_AON_CODEC_CLK_STOP_DR | PSC_AON_CODEC_CLK_STOP_REG;
    psc->REG_078 = PSC_WRITE_ENABLE |
        PSC_AON_CODEC_MEM_PSW_EN_DR |
        PSC_AON_CODEC_PSW_EN_DR |
        PSC_AON_CODEC_RESETN_ASSERT_DR |
        PSC_AON_CODEC_ISO_EN_DR | PSC_AON_CODEC_ISO_EN_REG |
        PSC_AON_CODEC_CLK_STOP_DR | PSC_AON_CODEC_CLK_STOP_REG;
    psc->REG_078 = PSC_WRITE_ENABLE |
        PSC_AON_CODEC_MEM_PSW_EN_DR |
        PSC_AON_CODEC_PSW_EN_DR |
        PSC_AON_CODEC_RESETN_ASSERT_DR | PSC_AON_CODEC_RESETN_ASSERT_REG |
        PSC_AON_CODEC_ISO_EN_DR | PSC_AON_CODEC_ISO_EN_REG |
        PSC_AON_CODEC_CLK_STOP_DR | PSC_AON_CODEC_CLK_STOP_REG;
    psc->REG_078 = PSC_WRITE_ENABLE |
        PSC_AON_CODEC_MEM_PSW_EN_DR |
        PSC_AON_CODEC_PSW_EN_DR | PSC_AON_CODEC_PSW_EN_REG |
        PSC_AON_CODEC_RESETN_ASSERT_DR | PSC_AON_CODEC_RESETN_ASSERT_REG |
        PSC_AON_CODEC_ISO_EN_DR | PSC_AON_CODEC_ISO_EN_REG |
        PSC_AON_CODEC_CLK_STOP_DR | PSC_AON_CODEC_CLK_STOP_REG;
    psc->REG_078 = PSC_WRITE_ENABLE |
        PSC_AON_CODEC_MEM_PSW_EN_DR | PSC_AON_CODEC_MEM_PSW_EN_REG |
        PSC_AON_CODEC_PSW_EN_DR | PSC_AON_CODEC_PSW_EN_REG |
        PSC_AON_CODEC_RESETN_ASSERT_DR | PSC_AON_CODEC_RESETN_ASSERT_REG |
        PSC_AON_CODEC_ISO_EN_DR | PSC_AON_CODEC_ISO_EN_REG |
        PSC_AON_CODEC_CLK_STOP_DR | PSC_AON_CODEC_CLK_STOP_REG;
}

void BOOT_TEXT_FLASH_LOC hal_psc_bt_enable(void)
{
    psc->REG_038 = PSC_WRITE_ENABLE |
        PSC_AON_BT_MEM_PSW_EN_DR |
        PSC_AON_BT_PSW_EN_DR | PSC_AON_BT_PSW_EN_REG |
        PSC_AON_BT_RESETN_ASSERT_DR | PSC_AON_BT_RESETN_ASSERT_REG |
        PSC_AON_BT_ISO_EN_DR | PSC_AON_BT_ISO_EN_REG |
        PSC_AON_BT_CLK_STOP_DR | PSC_AON_BT_CLK_STOP_REG;
    hal_sys_timer_delay(MS_TO_TICKS(1));
    psc->REG_038 = PSC_WRITE_ENABLE |
        PSC_AON_BT_MEM_PSW_EN_DR |
        PSC_AON_BT_PSW_EN_DR |
        PSC_AON_BT_RESETN_ASSERT_DR | PSC_AON_BT_RESETN_ASSERT_REG |
        PSC_AON_BT_ISO_EN_DR | PSC_AON_BT_ISO_EN_REG |
        PSC_AON_BT_CLK_STOP_DR | PSC_AON_BT_CLK_STOP_REG;
    hal_sys_timer_delay(MS_TO_TICKS(1));
    psc->REG_038 = PSC_WRITE_ENABLE |
        PSC_AON_BT_MEM_PSW_EN_DR |
        PSC_AON_BT_PSW_EN_DR |
        PSC_AON_BT_RESETN_ASSERT_DR |
        PSC_AON_BT_ISO_EN_DR | PSC_AON_BT_ISO_EN_REG |
        PSC_AON_BT_CLK_STOP_DR | PSC_AON_BT_CLK_STOP_REG;
    psc->REG_038 = PSC_WRITE_ENABLE |
        PSC_AON_BT_MEM_PSW_EN_DR |
        PSC_AON_BT_PSW_EN_DR |
        PSC_AON_BT_RESETN_ASSERT_DR |
        PSC_AON_BT_ISO_EN_DR |
        PSC_AON_BT_CLK_STOP_DR | PSC_AON_BT_CLK_STOP_REG;
    psc->REG_038 = PSC_WRITE_ENABLE |
        PSC_AON_BT_MEM_PSW_EN_DR |
        PSC_AON_BT_PSW_EN_DR |
        PSC_AON_BT_RESETN_ASSERT_DR |
        PSC_AON_BT_ISO_EN_DR |
        PSC_AON_BT_CLK_STOP_DR;

#ifdef JTAG_BT
    hal_psc_jtag_select_bt();
#endif
}

void BOOT_TEXT_FLASH_LOC hal_psc_bt_disable(void)
{
#ifdef JTAG_BT
    hal_psc_jtag_select_mcu();
#endif

    psc->REG_038 = PSC_WRITE_ENABLE |
        PSC_AON_BT_MEM_PSW_EN_DR |
        PSC_AON_BT_PSW_EN_DR |
        PSC_AON_BT_RESETN_ASSERT_DR |
        PSC_AON_BT_ISO_EN_DR |
        PSC_AON_BT_CLK_STOP_DR | PSC_AON_BT_CLK_STOP_REG;
    psc->REG_038 = PSC_WRITE_ENABLE |
        PSC_AON_BT_MEM_PSW_EN_DR |
        PSC_AON_BT_PSW_EN_DR |
        PSC_AON_BT_RESETN_ASSERT_DR |
        PSC_AON_BT_ISO_EN_DR | PSC_AON_BT_ISO_EN_REG |
        PSC_AON_BT_CLK_STOP_DR | PSC_AON_BT_CLK_STOP_REG;
    psc->REG_038 = PSC_WRITE_ENABLE |
        PSC_AON_BT_MEM_PSW_EN_DR |
        PSC_AON_BT_PSW_EN_DR |
        PSC_AON_BT_RESETN_ASSERT_DR | PSC_AON_BT_RESETN_ASSERT_REG |
        PSC_AON_BT_ISO_EN_DR | PSC_AON_BT_ISO_EN_REG |
        PSC_AON_BT_CLK_STOP_DR | PSC_AON_BT_CLK_STOP_REG;
    psc->REG_038 = PSC_WRITE_ENABLE |
        PSC_AON_BT_MEM_PSW_EN_DR |
        PSC_AON_BT_PSW_EN_DR | PSC_AON_BT_PSW_EN_REG |
        PSC_AON_BT_RESETN_ASSERT_DR | PSC_AON_BT_RESETN_ASSERT_REG |
        PSC_AON_BT_ISO_EN_DR | PSC_AON_BT_ISO_EN_REG |
        PSC_AON_BT_CLK_STOP_DR | PSC_AON_BT_CLK_STOP_REG;
    psc->REG_038 = PSC_WRITE_ENABLE |
        PSC_AON_BT_MEM_PSW_EN_DR | PSC_AON_BT_MEM_PSW_EN_REG |
        PSC_AON_BT_PSW_EN_DR | PSC_AON_BT_PSW_EN_REG |
        PSC_AON_BT_RESETN_ASSERT_DR | PSC_AON_BT_RESETN_ASSERT_REG |
        PSC_AON_BT_ISO_EN_DR | PSC_AON_BT_ISO_EN_REG |
        PSC_AON_BT_CLK_STOP_DR | PSC_AON_BT_CLK_STOP_REG;
}

void hal_psc_bt_enable_auto_power_down(void)
{
    psc->REG_020 = PSC_WRITE_ENABLE | PSC_AON_BT_PG_AUTO_EN | PSC_AON_BT_PG_HW_EN;
    psc->REG_038 = PSC_WRITE_ENABLE | 0;

    psc->REG_024 |= PSC_AON_BT_SLEEP_NO_WFI;
}

void hal_psc_rf_inf_enable_auto_power_down(void)
{
    uint32_t reg_044 = 0;

    reg_044 = psc->REG_044;
    reg_044 &= ~(PSC_AON_REG_PU_RF_INF_DR);
    psc->REG_044 = PSC_WRITE_ENABLE | reg_044;
}

void hal_psc_rf_inf_disable_auto_power_down(void)
{
    uint32_t reg_044 = 0;

    reg_044 = psc->REG_044;
    reg_044 |= PSC_AON_REG_PU_RF_INF_DR | PSC_AON_REG_PU_RF_INF;
    psc->REG_044 = PSC_WRITE_ENABLE | reg_044;
}

void BOOT_TEXT_FLASH_LOC hal_psc_sens_sys_enable(void)
{
    psc->REG_058 = PSC_WRITE_ENABLE |
        PSC_AON_SENS_MEM_PSW_EN_DR |
        PSC_AON_SENS_PSW_EN_DR | PSC_AON_SENS_PSW_EN_REG |
        PSC_AON_SENS_RESETN_ASSERT_DR | PSC_AON_SENS_RESETN_ASSERT_REG |
        PSC_AON_SENS_ISO_EN_DR | PSC_AON_SENS_ISO_EN_REG |
        PSC_AON_SENS_CLK_STOP_DR | PSC_AON_SENS_CLK_STOP_REG;
    hal_sys_timer_delay(MS_TO_TICKS(1));
    psc->REG_058 = PSC_WRITE_ENABLE |
        PSC_AON_SENS_MEM_PSW_EN_DR |
        PSC_AON_SENS_PSW_EN_DR |
        PSC_AON_SENS_RESETN_ASSERT_DR | PSC_AON_SENS_RESETN_ASSERT_REG |
        PSC_AON_SENS_ISO_EN_DR | PSC_AON_SENS_ISO_EN_REG |
        PSC_AON_SENS_CLK_STOP_DR | PSC_AON_SENS_CLK_STOP_REG;
    hal_sys_timer_delay(MS_TO_TICKS(1));
    psc->REG_058 = PSC_WRITE_ENABLE |
        PSC_AON_SENS_MEM_PSW_EN_DR |
        PSC_AON_SENS_PSW_EN_DR |
        PSC_AON_SENS_RESETN_ASSERT_DR |
        PSC_AON_SENS_ISO_EN_DR | PSC_AON_SENS_ISO_EN_REG |
        PSC_AON_SENS_CLK_STOP_DR | PSC_AON_SENS_CLK_STOP_REG;
    psc->REG_058 = PSC_WRITE_ENABLE |
        PSC_AON_SENS_MEM_PSW_EN_DR |
        PSC_AON_SENS_PSW_EN_DR |
        PSC_AON_SENS_RESETN_ASSERT_DR |
        PSC_AON_SENS_ISO_EN_DR |
        PSC_AON_SENS_CLK_STOP_DR | PSC_AON_SENS_CLK_STOP_REG;
    psc->REG_058 = PSC_WRITE_ENABLE |
        PSC_AON_SENS_MEM_PSW_EN_DR |
        PSC_AON_SENS_PSW_EN_DR |
        PSC_AON_SENS_RESETN_ASSERT_DR |
        PSC_AON_SENS_ISO_EN_DR |
        PSC_AON_SENS_CLK_STOP_DR;
}

void BOOT_TEXT_FLASH_LOC hal_psc_sens_enable(void)
{
    hal_psc_sens_sys_enable();
    psc->REG_068 = PSC_WRITE_ENABLE |
        PSC_AON_SENSCPU_PSW_EN_DR |
        PSC_AON_SENSCPU_RESETN_ASSERT_DR | PSC_AON_SENSCPU_RESETN_ASSERT_REG |
        PSC_AON_SENSCPU_ISO_EN_DR | PSC_AON_SENSCPU_ISO_EN_REG |
        PSC_AON_SENSCPU_CLK_STOP_DR | PSC_AON_SENSCPU_CLK_STOP_REG;
    hal_sys_timer_delay(MS_TO_TICKS(1));
    psc->REG_068 = PSC_WRITE_ENABLE |
        PSC_AON_SENSCPU_PSW_EN_DR |
        PSC_AON_SENSCPU_RESETN_ASSERT_DR |
        PSC_AON_SENSCPU_ISO_EN_DR | PSC_AON_SENSCPU_ISO_EN_REG |
        PSC_AON_SENSCPU_CLK_STOP_DR | PSC_AON_SENSCPU_CLK_STOP_REG;
    psc->REG_068 = PSC_WRITE_ENABLE |
        PSC_AON_SENSCPU_PSW_EN_DR |
        PSC_AON_SENSCPU_RESETN_ASSERT_DR |
        PSC_AON_SENSCPU_ISO_EN_DR |
        PSC_AON_SENSCPU_CLK_STOP_DR | PSC_AON_SENSCPU_CLK_STOP_REG;
    psc->REG_068 = PSC_WRITE_ENABLE |
        PSC_AON_SENSCPU_PSW_EN_DR |
        PSC_AON_SENSCPU_RESETN_ASSERT_DR |
        PSC_AON_SENSCPU_ISO_EN_DR |
        PSC_AON_SENSCPU_CLK_STOP_DR;
#ifdef JTAG_SENS
    hal_psc_jtag_select_sens();
#endif
}

void BOOT_TEXT_FLASH_LOC hal_psc_sens_disable(void)
{
#ifdef JTAG_SENS
    hal_psc_jtag_select_mcu();
#endif
    psc->REG_068 = PSC_WRITE_ENABLE |
        PSC_AON_SENSCPU_PSW_EN_DR |
        PSC_AON_SENSCPU_RESETN_ASSERT_DR |
        PSC_AON_SENSCPU_ISO_EN_DR |
        PSC_AON_SENSCPU_CLK_STOP_DR | PSC_AON_SENSCPU_CLK_STOP_REG;
    psc->REG_068 = PSC_WRITE_ENABLE |
        PSC_AON_SENSCPU_PSW_EN_DR |
        PSC_AON_SENSCPU_RESETN_ASSERT_DR |
        PSC_AON_SENSCPU_ISO_EN_DR | PSC_AON_SENSCPU_ISO_EN_REG |
        PSC_AON_SENSCPU_CLK_STOP_DR | PSC_AON_SENSCPU_CLK_STOP_REG;
    psc->REG_068 = PSC_WRITE_ENABLE |
        PSC_AON_SENSCPU_PSW_EN_DR |
        PSC_AON_SENSCPU_RESETN_ASSERT_DR | PSC_AON_SENSCPU_RESETN_ASSERT_REG |
        PSC_AON_SENSCPU_ISO_EN_DR | PSC_AON_SENSCPU_ISO_EN_REG |
        PSC_AON_SENSCPU_CLK_STOP_DR | PSC_AON_SENSCPU_CLK_STOP_REG;
    psc->REG_068 = PSC_WRITE_ENABLE |
        PSC_AON_SENSCPU_PSW_EN_DR | PSC_AON_SENSCPU_PSW_EN_REG |
        PSC_AON_SENSCPU_RESETN_ASSERT_DR | PSC_AON_SENSCPU_RESETN_ASSERT_REG |
        PSC_AON_SENSCPU_ISO_EN_DR | PSC_AON_SENSCPU_ISO_EN_REG |
        PSC_AON_SENSCPU_CLK_STOP_DR | PSC_AON_SENSCPU_CLK_STOP_REG;
    psc->REG_058 = PSC_WRITE_ENABLE |
        PSC_AON_SENS_MEM_PSW_EN_DR |
        PSC_AON_SENS_PSW_EN_DR |
        PSC_AON_SENS_RESETN_ASSERT_DR |
        PSC_AON_SENS_ISO_EN_DR |
        PSC_AON_SENS_CLK_STOP_DR | PSC_AON_SENS_CLK_STOP_REG;
    psc->REG_058 = PSC_WRITE_ENABLE |
        PSC_AON_SENS_MEM_PSW_EN_DR |
        PSC_AON_SENS_PSW_EN_DR |
        PSC_AON_SENS_RESETN_ASSERT_DR |
        PSC_AON_SENS_ISO_EN_DR | PSC_AON_SENS_ISO_EN_REG |
        PSC_AON_SENS_CLK_STOP_DR | PSC_AON_SENS_CLK_STOP_REG;
    psc->REG_058 = PSC_WRITE_ENABLE |
        PSC_AON_SENS_MEM_PSW_EN_DR |
        PSC_AON_SENS_PSW_EN_DR |
        PSC_AON_SENS_RESETN_ASSERT_DR | PSC_AON_SENS_RESETN_ASSERT_REG |
        PSC_AON_SENS_ISO_EN_DR | PSC_AON_SENS_ISO_EN_REG |
        PSC_AON_SENS_CLK_STOP_DR | PSC_AON_SENS_CLK_STOP_REG;
    psc->REG_058 = PSC_WRITE_ENABLE |
        PSC_AON_SENS_MEM_PSW_EN_DR |
        PSC_AON_SENS_PSW_EN_DR | PSC_AON_SENS_PSW_EN_REG |
        PSC_AON_SENS_RESETN_ASSERT_DR | PSC_AON_SENS_RESETN_ASSERT_REG |
        PSC_AON_SENS_ISO_EN_DR | PSC_AON_SENS_ISO_EN_REG |
        PSC_AON_SENS_CLK_STOP_DR | PSC_AON_SENS_CLK_STOP_REG;
    psc->REG_058 = PSC_WRITE_ENABLE |
        PSC_AON_SENS_MEM_PSW_EN_DR | PSC_AON_SENS_MEM_PSW_EN_REG |
        PSC_AON_SENS_PSW_EN_DR | PSC_AON_SENS_PSW_EN_REG |
        PSC_AON_SENS_RESETN_ASSERT_DR | PSC_AON_SENS_RESETN_ASSERT_REG |
        PSC_AON_SENS_ISO_EN_DR | PSC_AON_SENS_ISO_EN_REG |
        PSC_AON_SENS_CLK_STOP_DR | PSC_AON_SENS_CLK_STOP_REG;
}

void SRAM_TEXT_LOC hal_psc_sens_auto_power_down(int core_only)
{
    psc->REG_058 = PSC_WRITE_ENABLE | 0;
    if (core_only) {
        psc->REG_040 = PSC_WRITE_ENABLE | 0;
    } else {
        psc->REG_040 = PSC_WRITE_ENABLE | PSC_AON_SENS_PG_AUTO_EN | PSC_AON_SENS_PG_HW_EN;
    }

    psc->REG_068 = PSC_WRITE_ENABLE | 0;
    psc->REG_0E8 = PSC_WRITE_ENABLE | PSC_AON_SENSCPU_PG_AUTO_EN;
    psc->REG_0F0 = PSC_WRITE_ENABLE | PSC_AON_SENSCPU_POWERDN_START;
}

void SRAM_TEXT_LOC hal_psc_core_auto_power_down(int core_only)
{
#ifdef CHIP_SUBSYS_SENS
    hal_psc_sens_auto_power_down(core_only);
#else
#ifdef ARM_CMNS
    TZ_hal_sleep_deep_sleep_S();
#else
    hal_psc_mcu_auto_power_down();
#endif
#endif
}

BOOT_TEXT_FLASH_LOC
uint32_t gen_bth_shr_ram_mask(uint32_t base, uint32_t size)
{
    uint32_t start, end, idx, cnt;
    uint32_t mask;

    if (base <= RAM4_BASE) {
        start = 0;
    } else {
        start = base - RAM4_BASE;
    }

    if (base + size <= RAM4_BASE) {
        end = 0;
    } else {
        end = base + size - RAM4_BASE;
    }
    idx = start / SHR_RAM_BLK_SIZE;
    cnt = ((end - start) + SHR_RAM_BLK_SIZE - 1)/ SHR_RAM_BLK_SIZE;
    mask = ((1 << cnt) - 1) << idx;
    return mask;
}

BOOT_TEXT_FLASH_LOC
uint32_t gen_sens_shr_ram_mask(uint32_t base, uint32_t size)
{
    uint32_t end, idx, cnt;
    uint32_t mask, start;

    if (base < SENS_RAM6_BASE) {
        start = base - SENS_RAM0_BASE;
        end = SENS_RAM6_BASE - SENS_RAM0_BASE + 0x20000;
    } else {
        start = 0;
        end = 0;
    }

    idx = start / SHR_RAM_BLK_SIZE;
    cnt = (end - start) / SHR_RAM_BLK_SIZE;
    mask = ((1 << cnt) - 1) << idx;

    return mask;
}

BOOT_TEXT_SRAM_LOC
void hal_psc_shr_ram_config(uint32_t map, enum HAL_PSC_SHR_RAM_PD_CFG_T cfg)
{
    uint32_t reg_0fc = 0, reg_0f8 = 0, reg_044 = 0;
    uint32_t pe_bfv = 0, pe_bfv_sram9 = 0, ce_bfv = 0, ce_bfv_sram9 = 0;
    uint32_t dr_mask = 0, ce_dr_marsk = 0, dr_mask_sram9 = 0;
    uint32_t i = 0;

    reg_0fc = psc->REG_0FC;
    reg_0f8 = psc->REG_0F8;
    reg_044 = psc->REG_044;

    for (i = 0; i < 6; i++) {
        if (map & (1 << i)) {
            if(i == 5) {
                pe_bfv_sram9 |= (PSC_AON_SRAM9_PE_PSW_EN_REG);
                ce_bfv_sram9 |= (PSC_AON_SRAM9_CE_PSW_EN_REG);
                dr_mask_sram9 |= (PSC_AON_SRAM9_PE_PSW_EN_DR);
                ce_dr_marsk |= (PSC_AON_SRAM9_CE_PSW_EN_DR);
            } else {
                pe_bfv |= (PSC_AON_SRAM4_PE_PSW_EN_REG << i);
                ce_bfv |= (PSC_AON_SRAM4_CE_PSW_EN_REG << i);
                dr_mask |= (PSC_AON_SRAM4_PE_PSW_EN_DR << i);
                ce_dr_marsk |= (PSC_AON_SRAM4_CE_PSW_EN_DR << i);
            }
        }
    }

    if (cfg == HAL_PSC_SHR_RAM_FORCE_PU) {
        //dr=1 reg=0
        reg_0fc &= ~(pe_bfv | ce_bfv);
        reg_0fc |= dr_mask;

        reg_044 &= ~(pe_bfv_sram9 | ce_bfv_sram9);
        reg_0f8 |= ce_dr_marsk;
        reg_044 |= dr_mask_sram9;
    } else if (cfg == HAL_PSC_SHR_RAM_FORCE_PD) {
        //dr=1 reg=1
        reg_0fc |= dr_mask | pe_bfv | ce_bfv;
        reg_044 |= dr_mask_sram9 | pe_bfv_sram9 | ce_bfv_sram9;
        reg_0f8 |= ce_dr_marsk;
    } else if (cfg == HAL_PSC_SHR_RAM_SLEEP_PU) {
        //pe_dr=0 pe_reg=1 ce_dr=1 ce_reg=0
        reg_0fc |= pe_bfv;
        reg_044 |= pe_bfv_sram9;
        reg_0fc &= ~dr_mask;
        reg_044 &= ~dr_mask_sram9;

        reg_0fc &= ~ce_bfv;
        reg_044 &= ~ce_bfv_sram9;
        reg_0f8 |= ce_dr_marsk;
    } else {
        //dr=0 reg=1
        reg_0fc |= pe_bfv | ce_bfv;
        reg_044 |= pe_bfv_sram9 | ce_bfv_sram9;
        reg_0fc &= ~dr_mask;
        reg_0f8 &= ~ce_dr_marsk;
        reg_044 &= ~dr_mask_sram9;
    }

    //sram9_ce_reg must be write before the sram9_dr
    psc->REG_044 = PSC_WRITE_ENABLE | reg_044;
    psc->REG_0FC = PSC_WRITE_ENABLE | reg_0fc;
    psc->REG_0F8 = PSC_WRITE_ENABLE | reg_0f8;
}

void hal_psc_mcu_gpio_irq_enable(enum HAL_GPIO_PIN_T pin)
{
    if (pin < HAL_GPIO_PIN_P4_0) {
        psc->REG_080 |= (1 << pin);
    } else if (pin < HAL_GPIO_PIN_P8_0) {
        psc->REG_084 |= (1 << (pin - HAL_GPIO_PIN_P4_0));
    } else if (pin < HAL_GPIO_PIN_NUM) {
        psc->REG_088 |= (1 << (pin - HAL_GPIO_PIN_P8_0));
    }
}

void hal_psc_mcu_gpio_irq_disable(enum HAL_GPIO_PIN_T pin)
{
    if (pin < HAL_GPIO_PIN_P4_0) {
        psc->REG_080 &= ~(1 << pin);
    } else if (pin < HAL_GPIO_PIN_P8_0) {
        psc->REG_084 &= ~(1 << (pin - HAL_GPIO_PIN_P4_0));
    } else if (pin < HAL_GPIO_PIN_NUM) {
        psc->REG_088 &= ~(1 << (pin - HAL_GPIO_PIN_P8_0));
    }
}

uint32_t hal_psc_mcu_gpio_irq_get_status(uint32_t *status, uint32_t cnt)
{
    volatile uint32_t * const irq_status[] = {
        &psc->REG_08C,
        &psc->REG_090,
        &psc->REG_094,
    };
    const uint32_t irq_tab_sz = ARRAY_SIZE(irq_status);
    uint32_t i;

    if (cnt > irq_tab_sz) {
        cnt = irq_tab_sz;
    }
    for (i = 0; i < cnt; i++) {
        status[i] = *irq_status[i];
        if (i == (irq_tab_sz - 1))
            status[i] &= PSC_INTR_MASK3_GPIO2_15_0;
    }

    return cnt;
}

void hal_psc_sens_gpio_irq_enable(enum HAL_GPIO_PIN_T pin)
{
    if (pin < HAL_GPIO_PIN_P4_0) {
        psc->REG_0B0 |= (1 << pin);
    } else if (pin < HAL_GPIO_PIN_P8_0) {
        psc->REG_0B4 |= (1 << (pin - HAL_GPIO_PIN_P4_0));
    } else if (pin < HAL_GPIO_PIN_NUM) {
        psc->REG_0B8 |= (1 << (pin - HAL_GPIO_PIN_P8_0));
    }
}

void hal_psc_sens_gpio_irq_disable(enum HAL_GPIO_PIN_T pin)
{
    if (pin < HAL_GPIO_PIN_P4_0) {
        psc->REG_0B0 &= ~(1 << pin);
    } else if (pin < HAL_GPIO_PIN_P8_0) {
        psc->REG_0B4 &= ~(1 << (pin - HAL_GPIO_PIN_P4_0));
    } else if (pin < HAL_GPIO_PIN_NUM) {
        psc->REG_0B8 &= ~(1 << (pin - HAL_GPIO_PIN_P8_0));
    }
}

uint32_t hal_psc_sens_gpio_irq_get_status(uint32_t *status, uint32_t cnt)
{
    volatile uint32_t * const irq_status[] = {
        &psc->REG_0BC,
        &psc->REG_0C0,
        &psc->REG_0C4,
    };
    const uint32_t irq_tab_sz = ARRAY_SIZE(irq_status);
    uint32_t i;

    if (cnt > irq_tab_sz) {
        cnt = irq_tab_sz;
    }
    for (i = 0; i < cnt; i++) {
        status[i] = *irq_status[i];
        if (i == (irq_tab_sz - 1))
            status[i] &= PSC_INTR_MASK3_GPIO2_15_0;
    }
    return cnt;
}

void hal_psc_gpio_irq_enable(enum HAL_GPIO_PIN_T pin)
{
#ifdef CHIP_SUBSYS_SENS
    hal_psc_sens_gpio_irq_enable(pin);
#else
    hal_psc_mcu_gpio_irq_enable(pin);
#endif
}

void hal_psc_gpio_irq_disable(enum HAL_GPIO_PIN_T pin)
{
#ifdef CHIP_SUBSYS_SENS
    hal_psc_sens_gpio_irq_disable(pin);
#else
    hal_psc_mcu_gpio_irq_disable(pin);
#endif
}

uint32_t hal_psc_gpio_irq_get_status(uint32_t *status, uint32_t cnt)
{
#ifdef CHIP_SUBSYS_SENS
    return hal_psc_sens_gpio_irq_get_status(status, cnt);
#else
    return hal_psc_mcu_gpio_irq_get_status(status, cnt);
#endif
}

void SRAM_TEXT_LOC hal_psc_mcu_ram_auto_psw_enable(bool enable)
{
    uint32_t val;
    static bool is_auto_mode = true;

    if (!enable && is_auto_mode) {
        // Disable ISO
        val = psc->REG_07C;
        val |= PSC_AON_SRAM4_ISO_EN_DR | PSC_AON_SRAM5_ISO_EN_DR | PSC_AON_SRAM6_ISO_EN_DR | PSC_AON_SRAM7_ISO_EN_DR |
            PSC_AON_SRAM8_ISO_EN_DR | PSC_AON_SRAM9_ISO_EN_DR | PSC_AON_MCU_RETMEM_ISO_EN_DR;
        val &= ~(PSC_AON_SRAM4_ISO_EN_REG | PSC_AON_SRAM5_ISO_EN_REG | PSC_AON_SRAM6_ISO_EN_REG | PSC_AON_SRAM7_ISO_EN_REG |
            PSC_AON_SRAM8_ISO_EN_REG | PSC_AON_SRAM9_ISO_EN_REG | PSC_AON_MCU_RETMEM_ISO_EN_REG);
        psc->REG_07C = PSC_WRITE_ENABLE | val;

        // Force pu PE
        val = psc->REG_0F8;
        val = (val & ~PSC_AON_MCU_RETMEM_PE_PSW_EN_REG) | PSC_AON_MCU_RETMEM_PE_PSW_EN_DR;
        psc->REG_0F8 = PSC_WRITE_ENABLE | val;

        val = psc->REG_0FC;
        val |= PSC_AON_SRAM4_PE_PSW_EN_DR | PSC_AON_SRAM5_PE_PSW_EN_DR | PSC_AON_SRAM6_PE_PSW_EN_DR |
            PSC_AON_SRAM7_PE_PSW_EN_DR | PSC_AON_SRAM8_PE_PSW_EN_DR;
        val &= ~(PSC_AON_SRAM4_PE_PSW_EN_REG | PSC_AON_SRAM5_PE_PSW_EN_REG | PSC_AON_SRAM6_PE_PSW_EN_REG |
            PSC_AON_SRAM7_PE_PSW_EN_REG | PSC_AON_SRAM8_PE_PSW_EN_REG);
        psc->REG_0FC = PSC_WRITE_ENABLE | val;

        val = psc->REG_044;
        val = (val & ~PSC_AON_SRAM9_PE_PSW_EN_REG) | PSC_AON_SRAM9_PE_PSW_EN_DR;
        psc->REG_044 = PSC_WRITE_ENABLE | val;

        is_auto_mode = false;
    } else if (enable && !is_auto_mode) {
        // Resume ram psw settings
        psc->REG_07C = PSC_WRITE_ENABLE | reg_7c_saved;

        val = psc->REG_0F8;
        val &= ~(PSC_AON_MCU_RETMEM_PE_PSW_EN_REG | PSC_AON_MCU_RETMEM_PE_PSW_EN_DR);
        val |= reg_f8_saved;
        psc->REG_0F8 = PSC_WRITE_ENABLE | val;

        psc->REG_0FC = PSC_WRITE_ENABLE | reg_fc_saved;

        val = psc->REG_044;
        val &= ~(PSC_AON_SRAM9_PE_PSW_EN_REG | PSC_AON_SRAM9_PE_PSW_EN_DR);
        val |= reg_44_saved;
        psc->REG_044 = PSC_WRITE_ENABLE | val;

        is_auto_mode = true;
    }
}
