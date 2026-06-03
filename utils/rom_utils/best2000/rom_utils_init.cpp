/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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
#include "hal_chipid.h"
#include "hal_location.h"
#include "hal_trace.h"
#include "patch.h"
#include "cmsis.h"

#include "extern_functions.h"


//TODO: this function is not test by real hardware, but the code should be here
#if defined(__SBC_FUNC_IN_ROM_VBEST2000__)
extern "C" {
#include "stdarg.h"
#include "patch.h"
#include "string.h"
#include "stdio.h"
}

struct patch_func_item {
    void *old_func;
    void *new_func;
};
static uint32_t ALIGNED(0x100) remap_base[8];
char p_romsbc_trace_buffer[128];
SRAM_TEXT_LOC int p_hal_trace_printf_without_crlf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vsprintf(p_romsbc_trace_buffer,fmt,args);
    va_end(args);

    ROM_UTILS_TRACE(1,"%s\n", p_romsbc_trace_buffer);

    return 0;
}
SRAM_TEXT_LOC void p_OS_MemSet(U8 *dest, U8 byte, U32 len)
{
	memset(dest, byte, len);
}
SRAM_TEXT_LOC void p_OS_Assert(const char *expression, const char *file, U16 line)
{
    ASSERT(0,"OS_Assert exp: %s, func: %s, line %d\r\n", expression, file, line);
    while(1);
}
SRAM_TEXT_LOC void* p_memcpy(void *dst,const void *src, size_t length)
{
	return memcpy(dst, src, length);
}
struct patch_func_item romsbc_patchs[] = {
    // OS_Memset
    {(void *)0x0003a54d, (void *)p_OS_MemSet},
    // OS_Assert
    {(void *)0x0003a571, (void *)p_OS_Assert},
    // hal_trace_printf_without_crlf
    {(void *)0x0003a601, (void *)p_hal_trace_printf_without_crlf},
    // memcpy
    {(void *)0x0003a635, (void *)p_memcpy},
};

void init_2200_romprofile_sbc_patch(void)
{
    uint32_t i = 0;
    PATCH_ID id;
    patch_open((uint32_t)remap_base);
    for(i = 0; i < sizeof(romsbc_patchs)/sizeof(struct patch_func_item); ++i) {
        id = patch_enable(PATCH_TYPE_FUNC, (uint32_t)romsbc_patchs[i].old_func, (uint32_t)romsbc_patchs[i].new_func);
        ASSERT(id > 0, "%s: Failed to enable patch %d: %d", __func__, i, id);
    }
}
#endif

void rom_utils_init(void)
{
#if defined(__SBC_FUNC_IN_ROM_VBEST2000__)
    init_2200_romprofile_sbc_patch();
#endif
}
