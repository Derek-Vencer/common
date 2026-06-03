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

struct patch_func_item {
    void *old_func;
    void *new_func;
};
static uint32_t ALIGNED(0x100) remap_base[256];

static FRAM_TEXT_LOC void p_OS_MemSet(unsigned char *dest, unsigned char byte, unsigned int len)
{
    unsigned int i = 0;
    while (i < len) {
        dest[i] = byte;
        ++i;
    }
}

static FRAM_TEXT_LOC void *p_OS_memcpy(void *dest, const void *src, size_t count)
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

static struct patch_func_item romsbc_patchs[] = {
    // OS_Memset
    {(void *)0x0003ef85, (void *)p_OS_MemSet},
    {(void *)0x0003f26d, (void *)p_OS_memcpy},
};

static void init_2300_romutils_patch(void)
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
void rom_utils_init(void)
{
    init_2300_romutils_patch();
    //rom1_entry(&ef);
    set__impure_ptr(_impure_ptr);
}
