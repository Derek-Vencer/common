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

#include "aacdecoder_lib.h"
#include "aacenc_lib.h"
#include "FDK_bitbuffer.h"
#include "FDK_tools_rom.h"

struct patch_func_item {
    void *old_func;
    void *new_func;
};
static uint32_t ALIGNED(0x100) remap_base[8];

FRAM_TEXT_LOC void p_OS_MemSet(unsigned char *dest, unsigned char byte, unsigned int len)
{
    unsigned int i = 0;
    while (i < len) {
        dest[i] = byte;
        ++i;
    }
}

FRAM_TEXT_LOC void *p_OS_memcpy(void *dest, const void *src, size_t count)
{
    unsigned char *c_dst, *c_src;

    if (NULL == dest || NULL == src || count <= 0)
        return NULL;
    c_dst = (unsigned char *)dest;
    c_src = (unsigned char *)src;
    while (count--)
        *c_dst++ = *c_src++;
    return c_dst;
}

struct patch_func_item romsbc_patchs[] = {
    // OS_Memset
    {(void *)0x000170f1, (void *)p_OS_MemSet},
    {(void *)0x000170fd, (void *)p_OS_memcpy},
};

void init_1400_romutils_patch(void)
{
    uint32_t i = 0;
    PATCH_ID id;
    patch_open((uint32_t)remap_base);
    for(i = 0; i < sizeof(romsbc_patchs)/sizeof(struct patch_func_item); ++i) {
        id = patch_enable(PATCH_TYPE_FUNC, (uint32_t)romsbc_patchs[i].old_func, (uint32_t)romsbc_patchs[i].new_func);
        ASSERT(id > 0, "%s: Failed to enable patch %d: %d", __func__, i, id);
    }
}

extern "C" void set__impure_ptr(void *p);
#if defined(A2DP_AAC_ON)
extern "C" void fdk_aac_set_BitMask(void *p);
extern "C" void fdk_aac_set_SineTable480(void *p);
extern "C" void fdk_aac_set_SineTable512(void *p);
extern "C" void fdk_aac_set_RotVectorReal240(void *p);
extern "C" void fdk_aac_set_RotVectorImag240(void *p);
extern "C" void fdk_aac_set_RotVectorReal480(void *p);
extern "C" void fdk_aac_set_RotVectorImag480(void *p);
extern "C" void fdk_aac_set_windowSlopes(void *p);
extern "C" void fdk_aac_set_ldIntCoeff(void *p);
extern "C" void fdk_aac_set_invSqrtTab(void *p);
#endif

extern "C" int rom_utils_crash_dump_register(HAL_TRACE_CRASH_DUMP_CB_T cb)
{
    return hal_trace_crash_dump_register(HAL_TRACE_CRASH_DUMP_MODULE_SYS, cb);
}

struct __extern_functions__ ef = {
    /*
    .hal_sys_timer_get = hal_sys_timer_get,

    .hal_trace_crash_dump_register = rom_utils_crash_dump_register,
    .hal_trace_assert_dump_fix_arg = hal_trace_assert_dump_fix_arg,
    .hal_trace_printf_without_crlf_fix_arg = hal_trace_printf_without_crlf_fix_arg,
    .hal_trace_printf_imm_fix_arg = hal_trace_printf_imm_fix_arg,
    .hal_trace_output = hal_trace_output,
    .hal_trace_dump = hal_trace_dump,
    .vsprintf = vsprintf,

    .OS_MemSet = memset,
    .memset = memset,
    .memcpy = memcpy,

    .__aeabi_uldivmod = __aeabi_uldivmod,
    */
};

int hal_trace_printf_imm_fix_arg(const char *fmt)
{
    TR_INFO(TR_ATTR_IMM, "%s", fmt);
    return 0;
}

void hal_trace_assert_dump_fix_arg(const char *fmt)
{
    ASSERT(false, "%s ", fmt);
}

int hal_trace_printf_without_crlf_fix_arg(const char *fmt)
{
    TR_INFO(0, "%s", fmt);
    return 0;
}

void rom_utils_init(void)
{
    ef.hal_sys_timer_get = hal_sys_timer_get;

    ef.hal_trace_crash_dump_register = rom_utils_crash_dump_register;
    ef.hal_trace_assert_dump_fix_arg = hal_trace_assert_dump_fix_arg;
    ef.hal_trace_printf_without_crlf_fix_arg = hal_trace_printf_without_crlf_fix_arg;
    ef.hal_trace_printf_imm_fix_arg = hal_trace_printf_imm_fix_arg;
    ef.hal_trace_output = hal_trace_output;
    ef.hal_trace_dump = hal_trace_dump;
    ef.vsprintf = vsprintf;

    ef.OS_MemSet = p_OS_MemSet;
    ef.memset = memset;
    ef.memcpy = memcpy;

    ef.__aeabi_uldivmod = __aeabi_uldivmod;

    init_1400_romutils_patch();
    rom1_entry(&ef);
    set__impure_ptr(_impure_ptr);
#if defined(A2DP_AAC_ON)
    fdk_aac_set_BitMask((void *)&BitMask);
    fdk_aac_set_SineTable480((void*)&SineTable480);
    fdk_aac_set_SineTable512((void*)&SineTable512);
    fdk_aac_set_RotVectorReal240((void *)&RotVectorReal240);
    fdk_aac_set_RotVectorImag240((void *)&RotVectorImag240);
    fdk_aac_set_RotVectorReal480((void *)&RotVectorReal480);
    fdk_aac_set_RotVectorImag480((void *)&RotVectorImag480);
    fdk_aac_set_windowSlopes((void *)&windowSlopes);
    fdk_aac_set_ldIntCoeff((void *)&ldIntCoeff);
    fdk_aac_set_invSqrtTab((void *)&invSqrtTab);
#endif
}
