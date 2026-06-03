/***************************************************************************
 * Copyright 2015-2024 BES.
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
 ***************************************************************************/
#include "heap_api.h"
#include "hal_cache.h"
#include "conf/fs_config.h"

#ifdef FS_MM_HEAP_SIZE
#define FS_HEAP_BUFF_SIZE  FS_MM_HEAP_SIZE
#else
#define FS_HEAP_BUFF_SIZE  CONFIG_FS_HEAP_DEFAULT_SIZE
#endif
static multi_heap_handle_t g_fs_heap_handle = NULL;
static uint32_t g_fs_heap_buff[FS_HEAP_BUFF_SIZE] ALIGNED(64);

int fs_heap_init()
{
    if (g_fs_heap_handle != NULL)
        return 0;
    g_fs_heap_handle = heap_register((void *)g_fs_heap_buff, sizeof(g_fs_heap_buff));
    if (g_fs_heap_handle == NULL) {
        return -1;
    }
    return 0;
}

void fs_heap_destroy()
{
    g_fs_heap_handle = NULL;
}

void *fs_mm_zalloc(size_t size)
{
    void *ptr = NULL;
#ifdef FS_MM_HEAP_CACHE_ALIGNED
    ptr = heap_malloc_align(g_fs_heap_handle, size, MAX_CACHE_LINE_SIZE);
    if (ptr == NULL) {
        return NULL;
    }
#else
    ptr = heap_malloc(g_fs_heap_handle, size);
    if (ptr == NULL) {
        return NULL;
    }
#endif
    memset(ptr, 0x00, size);
    return ptr;
}

void *fs_mm_realloc(void *p, size_t size)
{
    void *ptr = NULL;
    ptr = heap_realloc(g_fs_heap_handle, p, size);
    if (ptr == NULL) {
        return NULL;
    }
    return ptr;
}

void fs_mm_free(void *ptr)
{
    if (ptr == NULL) {
        return;
    }
#ifdef FS_MM_HEAP_CACHE_ALIGNED
    heap_free_align(g_fs_heap_handle, ptr);
#else
    heap_free(g_fs_heap_handle, ptr);
#endif
}

size_t fs_mm_malloc_size(void *ptr)
{
    return heap_get_allocated_size(g_fs_heap_handle, ptr);
}

char *fs_mm_strdup(const char *s)
{
    size_t len = strlen(s) + 1;
    char *copy = fs_mm_zalloc(len);
    if (copy != NULL) {
        memcpy(copy, s, len);
    }
    return copy;
}

char *fs_mm_strndup(const char *s, size_t size)
{
    size_t len = strnlen(s, size) + 1;
    char *copy = fs_mm_zalloc(len);
    if (copy != NULL) {
        memcpy(copy, s, len);
        copy[len - 1] = '\0';
    }
    return copy;
}
