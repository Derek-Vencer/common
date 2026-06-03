/***************************************************************************
 *
 * Copyright 2023-2024 BES.
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
#ifdef PROGRAMMER

#include "cmsis_nvic.h"
#include "hal_cmu.h"
#include "main_entry.h"
#include "mpu_cfg.h"
#include "system_utils.h"
#include "tool_msg.h"

extern uint32_t __StackTop[];
extern uint32_t __StackLimit[];
extern uint32_t __pgm_bss_start__[];
extern uint32_t __pgm_bss_end__[];
#ifdef RAM_NC_BASE
extern uint32_t __sram_nc_data_start_flash__[];
extern uint32_t __sram_nc_data_end_flash__[];
extern uint32_t __sram_nc_data_start__[];
extern uint32_t __sram_nc_data_end__[];
extern uint32_t __sram_nc_bss_start__[];
extern uint32_t __sram_nc_bss_end__[];
#endif
#ifdef AON_MAILBOX_BASE
extern uint32_t __aon_mbx_data_start_load__[];
extern uint32_t __aon_mbx_data_end_load__[];
extern uint32_t __aon_mbx_data_start__[];
extern uint32_t __aon_mbx_bss_start__[];
extern uint32_t __aon_mbx_bss_end__[];
#endif

#define EXEC_STRUCT_LOC                 __attribute__((section(".exec_struct")))

#ifdef PROGRAMMER_LOAD_RAM
#ifndef LOAD_RAM_BASE
#error "PROGRAMMER_LOAD_RAM needs LOAD_RAM_BASE"
#endif
extern uint32_t __exec_load_start[];
extern uint32_t __exec_load_end[];
extern uint32_t __exec_data_start[];

#define PGM_START_LOC                   __attribute__((section(".pgm_start")))
#endif

extern int MAIN_ENTRY(void);

#ifdef PROGRAMMER_BOOT_ENTRY_HOOK
extern void boot_loader_entry_hook(void);
#endif

bool task_yield(void)
{
    return true;
}

// app_ramrun_start() and all the codes that it invokes will be located
// in real exec space, and not in load space
NOINLINE
void app_ramrun_start(void)
{
    uint32_t *dst;
    POSSIBLY_UNUSED uint32_t *src;

#ifdef PROGRAMMER_BOOT_ENTRY_HOOK
    boot_loader_entry_hook();
    // Now PGM_START_LOC code and data might be invalid
#endif

    // Init Got Base
    GotBaseInit();

    // Set stack pointer
    __set_MSP((uint32_t)__StackTop);
#ifdef __ARM_ARCH_8M_MAIN__
    __set_MSPLIM((uint32_t)__StackLimit);
#endif
    // Always use MSP and set privileged mode
    __set_CONTROL(0);
    __ISB();

    for (dst = __pgm_bss_start__; dst < __pgm_bss_end__; dst++) {
        *dst = 0;
    }

    NVIC_InitVectors();
#ifdef UNALIGNED_ACCESS
    SystemInit();
#endif
#ifdef MPU_INIT_TABLE
    // Init memory map
    mpu_boot_cfg();
#endif
    hal_cmu_setup();
#if defined(_VENDOR_MSG_SUPPT_)
    // 1600/1700 BTH RAMRUN:
    // Avoid SYS flash pll clock missing when init flash for ANC_CMD_OPEN command
    hal_cmu_programmer_enable_pll();
#endif

#ifdef RAM_NC_BASE
    // Init ram nc after mpu_boot_cfg() sets ram nc attribute
    for (dst = __sram_nc_data_start__, src = __sram_nc_data_start_flash__;
            src < __sram_nc_data_end_flash__;
            dst++, src++) {
        *dst = *src;
    }

    for (dst = __sram_nc_bss_start__; dst < __sram_nc_bss_end__; dst++) {
        *dst = 0;
    }
#endif

#ifdef AON_MAILBOX_BASE
    // Init aon mailbox after hal_cmu_programmer_setup() enables aon ram
    for (dst = __aon_mbx_data_start__, src = __aon_mbx_data_start_load__;
            src < __aon_mbx_data_end_load__;
            dst++, src++) {
        *dst = *src;
    }

    for (dst = __aon_mbx_bss_start__; dst < __aon_mbx_bss_end__; dst++) {
        *dst = 0;
    }
#endif

    MAIN_ENTRY();
}

#if defined(PROGRAMMER_LOAD_RAM) && defined(LOAD_RAM_BASE)
PGM_START_LOC
void app_load_start(void)
{
    uint32_t *dst;
    uint32_t *src;

    GotBaseInit();

    for (dst = __exec_data_start, src = __exec_load_start;
            src < __exec_load_end;
            dst++, src++) {
        *dst = *src;
    }

    app_ramrun_start();
}
void programmer_start(void) __attribute__((weak,alias("app_load_start")));
#else
void programmer_start(void) __attribute__((weak,alias("app_ramrun_start")));
#endif

const struct exec_struct_t EXEC_STRUCT_LOC ramrun_struct = {
    .entry = (uint32_t)programmer_start,
    .param = 0,
    .sp = 0,
    .exec_addr = (uint32_t)&ramrun_struct,
};

#endif // PROGRAMMER
