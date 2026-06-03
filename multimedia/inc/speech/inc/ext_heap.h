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
#include "string.h"
#include "custom_allocator.h"
#include "mm_dbg.h"

#if defined(EXT_HEAP_ENABLE_MULTI_HEAP)
#include "multi_heap.h"
#endif

#ifndef VQE_SIMULATE
#include "hal_trace.h"
#else
#define ASSERT(cond, str, ...)      { if (!(cond)) { fprintf(stderr, str, ##__VA_ARGS__); while(1); } }
#define LOG_I(str, ...)        do { fprintf(stdout, str, ##__VA_ARGS__); fprintf(stdout, "\n"); } while (0)
#ifndef POSSIBLY_UNUSED
#define POSSIBLY_UNUSED
#endif
#include "stdio.h"
#endif

#ifndef HEAP_BUFF_SIZE
#define HEAP_BUFF_SIZE       (1024 * 12)
#endif
static uint8_t *heap_buff = NULL;

#if defined(EXT_HEAP_ENABLE_MULTI_HEAP)
static struct multi_heap_info *g_heap;
#else
static uint32_t heap_buff_size_used;
#endif

static int32_t ext_heap_init(void *buff)
{
    LOG_I("[%s] Heap size = %d", __func__, HEAP_BUFF_SIZE);

    heap_buff = (uint8_t *)buff;
    memset((uint8_t *)heap_buff, 0, HEAP_BUFF_SIZE);

#if defined(EXT_HEAP_ENABLE_MULTI_HEAP)
    g_heap = multi_heap_register(heap_buff, HEAP_BUFF_SIZE);
#else
    heap_buff_size_used = 0;
#endif

    return 0;
}

static POSSIBLY_UNUSED int32_t ext_heap_deinit(void)
{
#if defined(EXT_HEAP_ENABLE_MULTI_HEAP)
    multi_heap_info_t info;
    multi_heap_get_info(g_heap, &info);
    LOG_I("[%s] heap = %d, used = %d, max_used = %d", __func__, info.total_bytes, info.total_allocated_bytes, info.total_bytes - info.minimum_free_bytes);
    ASSERT(info.total_allocated_bytes == 0, "[%s] memleak", __FUNCTION__);
    g_heap = NULL;
#else
    LOG_I("[%s] heap = %d, used = %d, max_used = %d", __func__, HEAP_BUFF_SIZE, heap_buff_size_used, heap_buff_size_used);
#endif

    return 0;
}

static POSSIBLY_UNUSED uint32_t ext_heap_get_used_buff_size()
{
#if defined(EXT_HEAP_ENABLE_MULTI_HEAP)
    multi_heap_info_t info;
    multi_heap_get_info(g_heap, &info);
    return info.total_allocated_bytes;
#else
    return heap_buff_size_used;
#endif
}

static uint32_t ext_heap_get_free_buff_size()
{
#if defined(EXT_HEAP_ENABLE_MULTI_HEAP)
    return multi_heap_free_size(g_heap);
#else
    return HEAP_BUFF_SIZE - heap_buff_size_used;
#endif
}

static void *ext_get_buff(uint32_t size)
{
    uint32_t buff_size_free;

    buff_size_free = ext_heap_get_free_buff_size();

    if (size % 4){
        size = size + (4 - size % 4);
    }

    // LOG_I("[%s] Free: %d; Alloc: %d", __func__, buff_size_free, size);

    ASSERT(size <= buff_size_free, "[%s] size = %d > free size = %d", __func__, size, buff_size_free);

#if defined(EXT_HEAP_ENABLE_MULTI_HEAP)
    uint8_t *buf_ptr = multi_heap_malloc(g_heap, size);
#else
    uint8_t *buf_ptr = &heap_buff[heap_buff_size_used];

    heap_buff_size_used += size;
#endif

    // LOG_I("Allocate %d, now used %d left %d", size, heap_buff_size_used, ext_heap_get_free_buff_size());

    return (void *)buf_ptr;
}

static void *ext_alloc(int size)
{
    void *mem_ptr = ext_get_buff(size);

    memset(mem_ptr, 0, size);

    return mem_ptr;
}

static POSSIBLY_UNUSED void *ext_malloc(uint32_t size)
{
    return ext_alloc(size);
}

static POSSIBLY_UNUSED void *ext_calloc(uint32_t nitems, uint32_t size)
{
    return ext_alloc(nitems * size);
}

static POSSIBLY_UNUSED void ext_free(void *mem_ptr)
{
#if defined(EXT_HEAP_ENABLE_MULTI_HEAP)
    multi_heap_free(g_heap, mem_ptr);
#endif

    ;
}

static POSSIBLY_UNUSED custom_allocator ext_allocator = {
    .malloc = ext_malloc,
    .calloc = ext_calloc,
    .free = ext_free,
};
