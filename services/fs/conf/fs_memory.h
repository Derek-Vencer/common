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
#ifndef __AOS_FS_MEMORY_H__
#define __AOS_FS_MEMORY_H__

#include <stddef.h>

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

#if defined(__cplusplus)
extern "C"
{
#endif

int fs_heap_init();
void fs_heap_destroy();
void fs_mm_free(void *ptr);
void *fs_mm_zalloc(size_t size);
void *fs_mm_realloc(void *p, size_t size);
size_t fs_mm_malloc_size(void *ptr);
char *fs_mm_strdup(const char *s);
char *fs_mm_strndup(const char *s, size_t size);

#define fs_heap_zalloc      fs_mm_zalloc
#define fs_heap_malloc      fs_mm_zalloc
#define fs_heap_malloc_size fs_mm_malloc_size
#define fs_heap_realloc     fs_mm_realloc
#define fs_heap_free        fs_mm_free
#define fs_heap_strdup      fs_mm_strdup
#define fs_heap_strndup     fs_mm_strndup

#if defined(__cplusplus)
}
#endif

#endif /*__AOS_FS_MEMORY_H__*/
