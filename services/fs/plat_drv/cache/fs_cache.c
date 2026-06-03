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

#include "fs_cache.h"
#include <string.h>
#include "conf/fs_memory.h"

#define CACHE_PAGE_NUM(ind, cache)    (cache->_fs_cache[ind].buffer_len/cache->sector_size)
#define INDEX_IS_VALID(ind, cache)    (ind < cache->conf.cache_seg_num)
#define DIFF(a,b)                     (a >= b ? a - b: b-a)
#define _FS_CACHE_SET_MAP(page_no,start_page, map) \
    do{map[(page_no - start_page)/32] |= (0x1<<((page_no - start_page) % 32));}while(0)

static uint32_t _lookup_ind(uint32_t page_no, struct fs_cache_manager *cache)
{
    uint32_t ind;
    uint32_t final_ind = 0xff;
    uint32_t candidate_ind[FS_CACHE_SEGMENTATION_NUM_MAX];
    uint32_t n = 0;
    uint32_t i;

    for (ind = 0; ind < cache->conf.cache_seg_num; ind ++) {
        if (page_no >= cache->_fs_cache[ind].start_page
            && page_no < cache->_fs_cache[ind].start_page +  cache->_fs_cache[ind].page_num) {
            AOS_FS_TRACE(1, FS_DEBUG_TAG"page_no=%d, start_page=%d, page_num=%d, ind=%d",
                         __FS_ERR_WHERE__, page_no, cache->_fs_cache[ind].start_page, cache->_fs_cache[ind].page_num, ind);
            return ind;
        }
    }

    for (ind = 0; ind < cache->conf.cache_seg_num; ind ++) {
        if (page_no >=  cache->_fs_cache[ind].base_page && page_no <= cache->_fs_cache[ind].end_page) {
            candidate_ind[n++] = ind;
            AOS_FS_TRACE(1, FS_DEBUG_TAG"page_no = %d, candidate_ind[%d] = %d",
                         __FS_ERR_WHERE__, page_no, n - 1, candidate_ind[n - 1]);
        }
    }

    if (n > 0) {
        final_ind = candidate_ind[0];
        for (i = 0; i < n; i ++) {
            ind = candidate_ind[i];
            if (cache->_fs_cache[ind].page_num == 0) {
                final_ind = ind;
                AOS_FS_TRACE(1, FS_DEBUG_TAG"i = %d break", __FS_ERR_WHERE__, i);
                break;
            } else if (DIFF(page_no, cache->_fs_cache[ind].start_page)
                       <= DIFF(page_no,  cache->_fs_cache[final_ind].start_page)) {
                final_ind = candidate_ind[i];
            }
        }
    }
    AOS_FS_TRACE(1, FS_DEBUG_TAG"page_no = %d, start_page = %d, page_num = %d, final_ind = %d",
                 __FS_ERR_WHERE__, page_no, cache->_fs_cache[ind].start_page, cache->_fs_cache[ind].page_num, final_ind);
    return final_ind;
}

static int fs_cache_seg_offset(int idx, struct fs_cache_seg_conf *conf)
{
    int offset = 0;
    for (int i = 0; i < idx; i++) {
        offset += conf->cache_seg[i];
    }
    return offset;
}

int fs_cache_init(struct fs_cache_seg_conf *conf, struct fs_cache_manager *cache, uint32_t sector_size)
{
    uint32_t len_cache_buffer = 0;
    uint32_t len_cache_w_map = 0;
    uint32_t w_map_offs = 0;
    uint32_t buffer_offs = 0;
    uint32_t buffer_len;

    if (conf == NULL || cache == NULL || sector_size == 0) {
        AOS_FS_TRACE(0, FS_ERR_TAG"error param", __FS_ERR_WHERE__);
        return -1;
    }

    if (conf->read == NULL || conf->write == NULL) {
        AOS_FS_TRACE(0, FS_ERR_TAG"error param", __FS_ERR_WHERE__);
        return -1;
    }

    if (conf->cache_seg_num > FS_CACHE_SEGMENTATION_NUM_MAX || conf->cache_seg_num < FS_CACHE_SEGMENTATION_NUM_MIN) {
        AOS_FS_TRACE(0, FS_ERR_TAG"cache_seg_num %d, max=%d min=%d",
                     __FS_ERR_WHERE__, conf->cache_seg_num, FS_CACHE_SEGMENTATION_NUM_MAX, FS_CACHE_SEGMENTATION_NUM_MIN);
        return -1;
    }

    if (conf->cache_len / sector_size == 0 || conf->cache_len % sector_size > 0) {
        AOS_FS_TRACE(0, FS_ERR_TAG"errror cache_len %d, sector size is %d", __FS_ERR_WHERE__, conf->cache_len, sector_size);
        return -1;
    }

    cache->priv = conf->priv;
    cache->sector_size = sector_size;
    memcpy(&cache->conf, conf, sizeof(struct fs_cache_seg_conf));
    len_cache_buffer = conf->cache_seg_num * conf->cache_len;
    cache->_cache_buffer = fs_mm_zalloc(len_cache_buffer);
    ASSERT(cache->_cache_buffer != NULL, "no memory error");

    len_cache_w_map = (len_cache_buffer / sector_size / 32 + conf->cache_seg_num) * 4;
    cache->_cache_w_map = fs_mm_zalloc(len_cache_w_map);
    ASSERT(cache->_cache_w_map != NULL, "no memory error");

    AOS_FS_TRACE(0, "\n");
    AOS_FS_TRACE(0, "****fs cache conf info****");
    AOS_FS_TRACE(0, "cache_len     %d", conf->cache_len);
    AOS_FS_TRACE(0, "cache_seg_num %d", conf->cache_seg_num);
    AOS_FS_TRACE(0, "sector_size   %d", sector_size);
    for (int i = 0; i < conf->cache_seg_num; i++) {
        AOS_FS_TRACE(0, "cache_seg[%d] = %d", i, (uint32_t)conf->cache_seg[i]);
    }

    memset(cache->_fs_cache, 0, sizeof(cache->_fs_cache));

    for (int i = 0; i < conf->cache_seg_num; i++) {
        buffer_len = conf->cache_len;
        cache->_fs_cache[i].base_page = fs_cache_seg_offset(i, conf);
        cache->_fs_cache[i].end_page  = cache->_fs_cache[i].base_page + conf->cache_seg[i] - 1;
        cache->_fs_cache[i].start_page = cache->_fs_cache[i].base_page;
        cache->_fs_cache[i].page_num = 0;
        cache->_fs_cache[i].w_map = (uint32_t *)cache->_cache_w_map + w_map_offs;
        cache->_fs_cache[i].buffer = (uint8_t *)cache->_cache_buffer + buffer_offs;
        cache->_fs_cache[i].w_map_len = (buffer_len) / sector_size / 32 + 1;
        w_map_offs += ((buffer_len)) / sector_size / 32 + 1;
        cache->_fs_cache[i].buffer_len = (buffer_len);
        buffer_offs += (buffer_len);
    }

    for (int ind = 0; ind < conf->cache_seg_num; ind ++) {
        AOS_FS_TRACE(0, "\n");
        AOS_FS_TRACE(0, "****FS_CACHE[%d]:****", ind);
        AOS_FS_TRACE(0, "BASE_PAGE: %d", cache->_fs_cache[ind].base_page);
        AOS_FS_TRACE(0, "END_PAGE:  %d", cache->_fs_cache[ind].end_page);
        AOS_FS_TRACE(0, "START_PAGE:%d", cache->_fs_cache[ind].start_page);
        AOS_FS_TRACE(0, "CACHE_NUM: %d", cache->_fs_cache[ind].page_num);
        AOS_FS_TRACE(0, "W_MAP_LEN: %d", cache->_fs_cache[ind].w_map_len);
        AOS_FS_TRACE(0, "BUFFER_LEN:%d", cache->_fs_cache[ind].buffer_len);
        AOS_FS_TRACE(0, "BUFFER:    0x%x", (uint32_t)cache->_fs_cache[ind].buffer);
        AOS_FS_TRACE(0, "W_MAP:     0x%x", (uint32_t)cache->_fs_cache[ind].w_map);
        AOS_FS_TRACE(0, "\n");
    }

    cache->fs_cache_inited = true;
    return 0;
}

static int fs_cache_flush(struct fs_cache_manager *cache, uint32_t ind)
{
    int ret = 0;
    uint32_t map_len = cache->_fs_cache[ind].w_map_len;//sizeof(cache->_fs_cache[ind].cache_w_map)/sizeof(_fs_cache[ind].cache_w_map[0]);
    uint32_t map;
    uint32_t i, j;
    uint8_t *buffer;
    uint32_t page;
    uint32_t cnt;
    uint32_t wcnt = 0;
    uint32_t tcnt = 0;

    buffer = cache->_fs_cache[ind].buffer;
    page = cache->_fs_cache[ind].start_page;
    cnt = 0;

    for (i = 0; i < map_len; i++) {
        map = cache->_fs_cache[ind].w_map[i];
        for (j = 0; j < 32; j++) {
            if (tcnt >= cache->_fs_cache[ind].page_num) {
                break;
            }

            if (map & (0x1 << j)) {
                cnt ++ ;
            } else {
                if (cnt) {
                    ret = cache->conf.write(cache->priv, buffer, page, cnt);
                    if (ret != cnt) {
                        AOS_FS_TRACE(0, "1 DEVICE WRITE(%d,%d) fail! ret = %d", page, cnt, ret);
                        goto _FUNC_ERR;
                    }
                    page += cnt;
                    wcnt += cnt;
                    cnt = 0;
                    buffer += cnt * cache->sector_size;
                } else {
                    buffer += cache->sector_size;
                    page++;
                }
            }
            tcnt++;
        }
    }

    if (cnt) {
        ret = cache->conf.write(cache->priv, buffer, page, cnt);
        if (ret != cnt) {
            AOS_FS_TRACE(0, "2 DEVICE WRITE(%d,%d) fail! ret = %d", page, cnt, ret);
            goto _FUNC_ERR;
        }
        page += cnt;
        wcnt += cnt;
        cnt = 0;
        buffer += cnt * cache->sector_size;
    }

    if (wcnt > 0) {
        memset((uint8_t *)&cache->_fs_cache[ind].w_map[0], 0, cache->_fs_cache[ind].w_map_len * sizeof(uint32_t));
    }

_FUNC_ERR:
    return ret;
}

int fs_cache_write(struct fs_cache_manager *cache,
                   const unsigned char *buffer,
                   blkcnt_t start_sector, unsigned int nsectors)
{
    uint32_t remain;
    uint8_t *cache_buff;
    uint32_t cur_page;
    uint32_t cur_count;
    uint32_t i;
    uint32_t ind;
    int ret;
    uint32_t page_no = start_sector;
    void *buf = (void *)buffer;
    unsigned int page_count = nsectors;

    AOS_FS_TRACE(1, FS_DEBUG_TAG"page_no = %d, page_count = %d", __FS_ERR_WHERE__, page_no, page_count);
    if (!cache->fs_cache_inited) {
        AOS_FS_TRACE(1, FS_DEBUG_TAG"device cache uninit", __FS_ERR_WHERE__);
        ret = cache->conf.write(cache->priv, buffer, start_sector, nsectors);
        return ret;
    }

    ind = _lookup_ind(page_no, cache);
    if (INDEX_IS_VALID(ind, cache)) {
        AOS_FS_TRACE(1, "IND = %d, start_page = %d, page_num = %d", ind, cache->_fs_cache[ind].start_page, cache->_fs_cache[ind].page_num);
        cache_buff = cache->_fs_cache[ind].buffer;
        remain = page_count;
        cur_page = page_no;
        cur_count = 0;
        while (remain > 0) {
            if (cur_page >= cache->_fs_cache[ind].start_page + CACHE_PAGE_NUM(ind, cache) ||
                cur_page < cache->_fs_cache[ind].start_page) {
                fs_cache_flush(cache, ind);
                cache->_fs_cache[ind].start_page = cur_page;
                cache->_fs_cache[ind].page_num  = 0;
            }
            cur_count = remain <= CACHE_PAGE_NUM(ind, cache) - (cur_page - cache->_fs_cache[ind].start_page)
                        ? remain : CACHE_PAGE_NUM(ind, cache) - (cur_page - cache->_fs_cache[ind].start_page);
            memcpy(cache_buff + (cur_page - cache->_fs_cache[ind].start_page)*cache->sector_size,
                   (uint8_t *)buf + (page_count - remain)*cache->sector_size, cur_count * cache->sector_size);
            cache->_fs_cache[ind].page_num = (cur_page - cache->_fs_cache[ind].start_page) +
                                             (cur_count > cache->_fs_cache[ind].page_num
                                              ? (cur_page - cache->_fs_cache[ind].start_page) + cur_count
                                              : cache->_fs_cache[ind].page_num);
            remain -= cur_count;
            for (i = 0; i <  cur_count; i++) {
                _FS_CACHE_SET_MAP(cur_page, cache->_fs_cache[ind].start_page, cache->_fs_cache[ind].w_map);
                cur_page ++;
            }
        }
        return nsectors;
    } else {
        AOS_FS_TRACE(1, FS_DEBUG_TAG"IND INVALID: %d", __FS_ERR_WHERE__, ind);
        ret = cache->conf.write(cache->priv, buffer, start_sector, nsectors);
        return ret;
    }
}

int fs_cache_read(struct fs_cache_manager *cache, unsigned char *buffer,
                  blkcnt_t start_sector, unsigned int nsectors)
{
    uint32_t remain;
    uint8_t *cache_buff;
    uint32_t cur_page;
    uint32_t cur_count;
    uint32_t ind;
    int ret = 0;

    uint32_t page_no = start_sector;
    void *buf = (void *)buffer;
    unsigned int page_count = nsectors;

    AOS_FS_TRACE(1, FS_DEBUG_TAG"page_no = %d, page_count = %d", __FS_ERR_WHERE__, page_no, page_count);
    if (!cache->fs_cache_inited) {
        AOS_FS_TRACE(1, FS_DEBUG_TAG"fatfs device cache uninit", __FS_ERR_WHERE__);
        ret = cache->conf.read(cache->priv, buffer, start_sector, nsectors);
        return  ret;
    }

    ind = _lookup_ind(page_no, cache);
    if (INDEX_IS_VALID(ind, cache)) {
        AOS_FS_TRACE(1, FS_DEBUG_TAG"IND = %d, start_page = %d, page_num = %d",
                     __FS_ERR_WHERE__, ind, cache->_fs_cache[ind].start_page, cache->_fs_cache[ind].page_num);
        cache_buff = cache->_fs_cache[ind].buffer;
        remain = page_count;
        cur_page = page_no;
        cur_count = 0;
        while (remain > 0) {
            if (cur_page >= cache->_fs_cache[ind].start_page + cache->_fs_cache[ind].page_num ||
                cur_page < cache->_fs_cache[ind].start_page) {
                fs_cache_flush(cache, ind);
                cache->_fs_cache[ind].start_page = cur_page;
                ret = cache->conf.read(cache->priv, cache_buff, cache->_fs_cache[ind].start_page, CACHE_PAGE_NUM(ind, cache));
                if (ret == -1) {
                    AOS_FS_TRACE(1, FS_DEBUG_TAG"DEVICE READ(%d,%d) fail! ret = %d",
                                 __FS_ERR_WHERE__, cache->_fs_cache[ind].start_page, CACHE_PAGE_NUM(ind, cache), ret);
                    break;
                } else {
                    AOS_FS_TRACE(1, FS_DEBUG_TAG"DEVICE READ(%d,%d) done",
                                 __FS_ERR_WHERE__, cache->_fs_cache[ind].start_page, CACHE_PAGE_NUM(ind, cache));
                }
                cache->_fs_cache[ind].page_num = CACHE_PAGE_NUM(ind, cache);
            }
            cur_count = remain <= cache->_fs_cache[ind].page_num - (cur_page - cache->_fs_cache[ind].start_page)
                        ? remain : cache->_fs_cache[ind].page_num - (cur_page - cache->_fs_cache[ind].start_page);
            memcpy((uint8_t *)buf + (page_count - remain)*cache->sector_size,
                   cache_buff + (cur_page - cache->_fs_cache[ind].start_page)*cache->sector_size,
                   cur_count * cache->sector_size);
            cur_page += cur_count;
            remain -= cur_count;
        }
        return nsectors;
    } else {
        AOS_FS_TRACE(1, FS_DEBUG_TAG"IND INVALID: %d", __FS_ERR_WHERE__, ind);
        ret = cache->conf.read(cache->priv, buffer, start_sector, nsectors);
        return ret;
    }
}

int fs_cache_sync(struct fs_cache_manager *cache)
{
    int32_t ret;
    uint32_t ind;

    //AOS_FS_TRACE(0, "\n%s devType = %d", __func__, devtype);
    if (!cache->fs_cache_inited) {
        AOS_FS_TRACE(1, FS_DEBUG_TAG"fatfs device cache uninit", __FS_ERR_WHERE__);
        return 0;
    }

    for (ind = 0; ind < cache->conf.cache_seg_num; ind ++) {
        ret = fs_cache_flush(cache, ind);
        if (ret) {
            break;
        }
    }
    //AOS_FS_TRACE(0, "%s ret = %d", __func__, ret);
    return ret;
}

int fs_cache_delinit(struct fs_cache_manager *cache)
{
    if (cache->fs_cache_inited == false) {
        return 0;
    }
    fs_mm_free(cache->_cache_buffer);
    fs_mm_free(cache->_cache_w_map);
    memset(cache, 0x00, sizeof(struct fs_cache_manager));
    cache->fs_cache_inited = false;
    return 0;
}

int fs_cache_conf_init(struct fs_cache_seg_conf *conf, int cache_len,
                       int cache_seg_num, blkcnt_t *cache_seg,
                       device_read_f read, device_write_f write,
                       void *priv)
{
    conf->cache_len = cache_len;
    conf->cache_seg_num = cache_seg_num;
    conf->read = read;
    conf->write = write;
    for (int i = 0; i < cache_seg_num; i++) {
        conf->cache_seg[i] = cache_seg[i];
    }
    conf->priv = priv;
    return 0;
}

