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
#ifndef __EXPORT_UTILS_FN_ROM_H__
#define __EXPORT_UTILS_FN_ROM_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*__INIT_QUEUE)(CQueue *Q, unsigned int size, CQItemType *buf);
typedef int (*__DEC_QUEUE)(CQueue *Q, CQItemType *e, unsigned int len);
typedef int (*__AVAILABLE_OF_QUEUE)(CQueue *Q);
typedef int (*__ENC_QUEUE)(CQueue *Q, CQItemType *e, unsigned int len);
typedef int (*__LENGTH_OF_CQUEUE)(CQueue *Q);
typedef void (*__RESET_CQUEUE)(CQueue *Q);

typedef multi_heap_handle_t (*__MULTI_HEAP_REGISTER)(void *start, size_t size);
typedef void* (*__MULTI_HEAP_GET_BLOCK_ADDR)(multi_heap_block_handle_t block);
typedef void* (*__MULTI_HEAP_MALLOC)(multi_heap_handle_t heap, size_t size);
typedef void (*__MULTI_HEAP_FREE)(multi_heap_handle_t heap, void *p);
typedef void (*__MULTI_HEAP_GET_INFO)(multi_heap_handle_t heap, multi_heap_info_t *info);
typedef bool (*__MULTI_HEAP_CHECK_SIZE_MALLOC_AVA)(multi_heap_handle_t heap, size_t size);
typedef void *(*__MULTI_HEAP_MALLOC_ALIGN)(multi_heap_handle_t heap, size_t size, size_t align);
typedef void (*__MULTI_HEAP_FREE_ALIGN)(multi_heap_handle_t heap, void *p);

typedef bthost_cfg_t* (*__BT_HOST_GET_CFG)(void);

struct EXPORT_UTILS_FN_ROM_T {
    void *reserved;

    __INIT_QUEUE InitCQueue;
    __DEC_QUEUE DeCQueue;
    __AVAILABLE_OF_QUEUE AvailableOfCQueue;
    __ENC_QUEUE EnCQueue;
    __LENGTH_OF_CQUEUE LengthOfCQueue;
    __RESET_CQUEUE  ResetCQueue;

    __MULTI_HEAP_REGISTER multi_heap_register;
    __MULTI_HEAP_GET_BLOCK_ADDR multi_heap_get_block_address;
    __MULTI_HEAP_MALLOC multi_heap_malloc;
    __MULTI_HEAP_FREE multi_heap_free;
    __MULTI_HEAP_GET_INFO multi_heap_get_info;
    __MULTI_HEAP_CHECK_SIZE_MALLOC_AVA multi_heap_check_size_malloc_available;
    __MULTI_HEAP_MALLOC_ALIGN multi_heap_malloc_align;
    __MULTI_HEAP_FREE_ALIGN multi_heap_free_align;

    __BT_HOST_GET_CFG bt_host_get_cfg;
};

void export_register_utils_fn(const struct EXPORT_UTILS_FN_ROM_T* fn);

#ifdef __cplusplus
}
#endif

#endif /*__EXPORT_UTILS_FN_ROM_H__*/
