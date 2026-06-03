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
#ifndef __AOS_FS_CACHE_H__
#define __AOS_FS_CACHE_H__

#include "conf/fs_config.h"
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FS_CACHE_SEGMENTATION_NUM_MIN        2

#ifdef CONFIG_FS_CACHE_SEGMENTATION_NUM_MAX
#define FS_CACHE_SEGMENTATION_NUM_MAX        CONFIG_FS_CACHE_SEGMENTATION_NUM_MAX
#else
#define FS_CACHE_SEGMENTATION_NUM_MAX        5
#endif

typedef ssize_t (*device_read_f)(void* priv, unsigned char *buffer, blkcnt_t start_sector, unsigned int nsectors);
typedef ssize_t (*device_write_f)(void* priv, const unsigned char *buffer, blkcnt_t start_sector, unsigned int nsectors);

struct fs_cache_seg_conf {
    uint32_t cache_len;     // cache buffer len
    uint32_t cache_seg_num; // cache segmentation num
    blkcnt_t cache_seg[FS_CACHE_SEGMENTATION_NUM_MAX]; // block num for each cache segmentation
    device_read_f read;    //device read
    device_write_f write;  //device write
    void* priv;
};

struct fs_cache {
    uint32_t base_page;       // cache section base page no.
    uint32_t end_page;        // cache section end page no.
    uint32_t start_page;      // cache start page no.
    uint32_t page_num;        // cache page number.
    uint32_t *w_map;
    uint32_t w_map_len;
    uint8_t *buffer;
    uint32_t buffer_len;
};

struct fs_cache_manager {
    bool fs_cache_inited;
    struct fs_cache _fs_cache[FS_CACHE_SEGMENTATION_NUM_MAX];
    uint8_t *_cache_buffer;
    uint32_t *_cache_w_map;
    uint32_t sector_size;
    struct fs_cache_seg_conf conf;
    void *priv;
};

int fs_cache_init(struct fs_cache_seg_conf *conf, struct fs_cache_manager *cache, uint32_t sector_size);
int fs_cache_sync(struct fs_cache_manager *cache);
int fs_cache_read(struct fs_cache_manager *cache, unsigned char *buffer, blkcnt_t start_sector, unsigned int nsectors);
int fs_cache_write(struct fs_cache_manager *cache, const unsigned char *buffer, blkcnt_t start_sector, unsigned int nsectors);
int fs_cache_delinit(struct fs_cache_manager *cache);
int fs_cache_conf_init(struct fs_cache_seg_conf *conf, int cache_len, int cache_seg_num,
                            blkcnt_t * cache_seg, device_read_f read, device_write_f write, void* priv);

#ifdef __cplusplus
}
#endif

#endif  /*__AOS_FS_CACHE_H__*/

