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
#include "cmsis.h"
#include "cmsis_os.h"
#include "heap_api.h"
#include "app_utils.h"
#include "hal_trace.h"
#include "bes_mem_api.h"

#define BES_CC_HEAP_DEBUGx

static heap_handle_t bes_cc_heap_handle = NULL;
static private_malloc_cb private_malloc_callback = NULL;
static private_calloc_cb private_calloc_callback = NULL;
static private_free_cb  private_free_callback = NULL;

void bes_cc_heap_init(void *begin_addr, uint32_t size)
{
    bes_cc_heap_handle = heap_register(begin_addr,size);
}

void *bes_cc_heap_malloc(uint32_t size)
{
    uint32_t lock = int_lock();

    size = ((size >> 2) + 1) << 2;
    void *ptr = heap_malloc(bes_cc_heap_handle, size);
    ASSERT(ptr, "%s size:%d", __func__, size);
#ifdef BES_CC_HEAP_DEBUG
    CC_DECODE_LOG_I("ptr=%p size=%u user=%p left/mini left=%u/%u",
        ptr, size, __builtin_return_address(0),
        heap_free_size(bes_cc_heap_handle),
        heap_minimum_free_size(bes_cc_heap_handle));
#endif

    int_unlock(lock);
    return ptr;
}

void *bes_cc_heap_cmalloc(size_t __nmemb, uint32_t size)
{
    uint32_t lock = int_lock();

    size = ((size >> 2) + 1) << 2;
    void *ptr = heap_malloc(bes_cc_heap_handle, size);
    ASSERT(ptr, "%s size:%d", __func__, size);
#ifdef BES_CC_HEAP_DEBUG
    CC_DECODE_LOG_I("ptr=%p size=%u user=%p left/mini left=%u/%u",
        ptr, size, __builtin_return_address(0),
        heap_free_size(bes_cc_heap_handle),
        heap_minimum_free_size(bes_cc_heap_handle));
#endif

    memset(ptr, 0, size);
    int_unlock(lock);
    return ptr;
}

void bes_cc_heap_free(void *rmem)
{
#ifdef BES_CC_HEAP_DEBUG
    CC_DECODE_LOG_I("ptr=%p user=%p", rmem, __builtin_return_address(0));
#endif
    ASSERT(rmem, "%s rmem:%p", __func__, rmem);

    uint32_t lock = int_lock();
    heap_free(bes_cc_heap_handle,rmem);
    int_unlock(lock);
}


void private_malloc_callback_register(private_malloc_cb cb)
{
    private_malloc_callback = cb;
    return;
}

void private_calloc_callback_register(private_calloc_cb cb)
{
    private_calloc_callback = cb;
    return;
}

void private_free_callback_register(private_free_cb cb)
{
    private_free_callback = cb;
    return;
}

void *private_malloc(size_t __size)
{
    if(private_malloc_callback == NULL) {
        return malloc(__size);
    }
    return private_malloc_callback(__size);
}

void *private_calloc(size_t __nmemb, size_t __size)
{
    if(private_calloc_callback == NULL) {
        return calloc(__nmemb, __size);
    }
    return private_calloc_callback(__nmemb, __size);
}

void private_free(void *__ptr)
{
    if(private_free_callback == NULL) {
        return free(__ptr);
    }
    return private_free_callback(__ptr);
}

LDACBT_API void ldac_cc_mem_init()
{
    private_malloc_callback_register(bes_cc_heap_malloc);
    private_calloc_callback_register(bes_cc_heap_cmalloc);
    private_free_callback_register(bes_cc_heap_free);
}
