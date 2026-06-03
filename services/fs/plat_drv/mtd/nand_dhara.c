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
#include "plat_addr_map.h"
#ifdef NANDFLASH_SUPPORT
#ifndef NANDFLASH_MTD_ENABLE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hal_trace.h"
#include "nandflash_drv.h"
#include "dhara/nand.h"
#include "dhara/map.h"
#include "dhara/journal.h"
#include "fs/fs.h"
#include "nandflash.h"

#define LOG_HEAD "[NFTL] "
#define NAND_NFTL_WHERE __FUNCTION__,__LINE__

#ifdef NAND_NFTL_DEBUG
#define NAND_NFTL_DBG(...) TR_INFO(TR_MOD(NONE), LOG_HEAD __VA_ARGS__)
#else
#define NAND_NFTL_DBG(...)
#endif
#define NAND_NFTL_ERR(...)  TR_ERROR(TR_MOD(NONE), LOG_HEAD __VA_ARGS__)
#define NAND_NFTL_INFO(...) TR_INFO(TR_MOD(NONE), LOG_HEAD __VA_ARGS__)

#define BLOCK_BAD_MARK  0x01
#define BLOCK_FAILED    0x02

typedef enum NANDFLASH_RET_T nand_ret_int;
typedef struct nftl_config {
    int flashId;

    uint32_t total_size;

    uint32_t block_size;
    uint32_t log2_block_size;

    uint32_t log2_page_size;
    uint32_t page_size;

    uint32_t log2_ppb;
    uint32_t pages_per_block;

    uint32_t log2_oob_size;
    uint32_t oob_size;

    uint32_t num_blocks;
    int gc_ratio;

    void *page_buff;
    void *page_copy;

    nand_ret_int(*read)(enum HAL_FLASH_ID_T id, uint32_t bno, uint32_t pno, uint32_t off, uint8_t *buffer, uint32_t size);
    nand_ret_int(*prog)(enum HAL_FLASH_ID_T id, uint32_t bno, uint32_t pno, const uint8_t *buffer, uint32_t size);
    nand_ret_int(*erase)(enum HAL_FLASH_ID_T id, uint32_t bno);
    nand_ret_int(*markbad)(enum HAL_FLASH_ID_T id, uint32_t bno);
    int (*isbad)(enum HAL_FLASH_ID_T id, uint32_t bno);
} nftl_config_t;

struct nftlm_nandflash_status {
    int      flashid;
    uint32_t opened;
    void    *mutex;
    uint32_t refs;
    uint32_t partenable;
};

typedef struct nftl_m {
    struct nftlm_nandflash_status *status;
    struct dhara_nand nand;
    struct dhara_map map;
    nftl_config_t conf;
    uint32_t boffset;
    uint32_t bnum;
    uint32_t partidx;
} nftl_m_t;

#define NFTLM_LOG2_PAGE_SIZE(nftlm)        (nftlm->conf.log2_page_size)
#define NFTLM_LOG2_PAGES_PER_BLOCK(nftlm)  (nftlm->conf.log2_ppb)
#define NFTLM_LOG2_BLOCK_SIZE(nftlm)       (nftlm->conf.log2_page_size + nftlm->conf.log2_ppb)
#define NFTLM_NUM_BLOCKS(nftlm)            (nftlm->conf.num_blocks)
#define NFTLM_PAGE_SIZE(nftlm)             (1 << nftlm->conf.log2_page_size)
#define NFTLM_PAGES_PER_BLOCK(nftlm)       (1 << nftlm->conf.log2_ppb)
#define NFTLM_BLOCK_SIZE(nftlm)            (1 << (nftlm->conf.log2_page_size + nftlm->conf.log2_ppb))
#define NFTLM_OOB_SIZE(nftlm)              (1 << nftlm->conf.log2_oob_size)
#define NFTLM_GET(n)                       ((nftl_m_t*)n->context)

static struct nftlm_nandflash_status g_nand_status[HAL_FLASH_ID_NUM];
static nftl_m_t g_nftlm[HAL_FLASH_ID_NUM][MAX_NAND_PART_NUM];

/**************************************************************************
 * Private interface For Dhara
 **************************************************************************/
static bool dhara_check_ff(FAR const uint8_t *buf, size_t size)
{
    const uint8_t *p = buf;
    size_t len;

    for (len = 0; len < 16; len++) {
        if (!size)
            return true;

        if (*p != 0xff)
            return false;

        p++;
        size--;
    }

    return memcmp(buf, p, size) == 0;
}

int dhara_nand_is_free(const struct dhara_nand *n, dhara_page_t p)
{
    nftl_m_t *nftlm = NFTLM_GET(n);
    uint8_t *buf = nftlm->conf.page_copy;
    dhara_error_t err = 0;

    if (dhara_nand_read(n, p, 0, nftlm->conf.page_size, buf, &err) < 0) {
        AOS_FS_TRACE(0, FS_ERR_TAG"Fail to read page for free check err %s",
                     __FS_ERR_WHERE__, dhara_strerror(err));
        return 0;
    }

    return dhara_check_ff(buf, nftlm->conf.page_size);
}

int dhara_nand_is_bad(const struct dhara_nand *n, dhara_block_t bno)
{
    int ret;
    nftl_m_t *nftlm = NFTLM_GET(n);

    if (bno >= NFTLM_NUM_BLOCKS(nftlm)) {
        ASSERT(0, "[%s:%d] block is invalid, bno=%d", NAND_NFTL_WHERE, bno);
    }

    bno += nftlm->boffset;
    ret = nftlm->conf.isbad(nftlm->conf.flashId, bno);
    return ret;
}

void dhara_nand_mark_bad(const struct dhara_nand *n, dhara_block_t bno)
{
    nftl_m_t *nftlm = NFTLM_GET(n);

    if (bno >= NFTLM_NUM_BLOCKS(nftlm)) {
        ASSERT(0, "[%s:%d] block is invalid, bno=%d", NAND_NFTL_WHERE, bno);
    }

    //nand flash driver mark bad
    int32_t ret;
    bno += nftlm->boffset;
    ret = nftlm->conf.markbad(nftlm->conf.flashId, bno);
    if (ret != NANDFLASH_RET_OK) {
        NAND_NFTL_ERR("[%s:%d] markbadError, bno=%d", NAND_NFTL_WHERE, bno);
    }
}

int dhara_nand_erase(const struct dhara_nand *n, dhara_block_t bno, dhara_error_t *err)
{
    int ret;
    nftl_m_t *nftlm = NFTLM_GET(n);

    if (bno >= NFTLM_NUM_BLOCKS(nftlm)) {
        NAND_NFTL_ERR("[%s:%d] block is invalid, bno=%d", NAND_NFTL_WHERE, bno);
        return -1;
    }

    bno += nftlm->boffset;
    NAND_NFTL_DBG("[%s:%d] bno=%d", NAND_NFTL_WHERE, bno);
    ret = nftlm->conf.erase(nftlm->conf.flashId, bno);
    if (ret != NANDFLASH_RET_OK) {
        dhara_set_error(err, DHARA_E_BAD_BLOCK);
        return -1;
    }

    return 0;
}

int dhara_nand_prog(const struct dhara_nand *n, dhara_page_t p,
                    const uint8_t *data, dhara_error_t *err)
{
    int ret;
    nftl_m_t *nftlm = NFTLM_GET(n);
    int bno = p >> NFTLM_LOG2_PAGES_PER_BLOCK(nftlm);
    const int pno = p & ((1 << NFTLM_LOG2_PAGES_PER_BLOCK(nftlm)) - 1);

    if ((bno < 0) || (bno >= NFTLM_NUM_BLOCKS(nftlm))) {
        ASSERT(0, "[%s:%d block] is invalid, bno=%d", NAND_NFTL_WHERE, bno);
    }

    bno += nftlm->boffset;
    NAND_NFTL_DBG("[%s:%d] bno=%d pno=%d", NAND_NFTL_WHERE, bno, pno);
    ret = nftlm->conf.prog(nftlm->conf.flashId, bno, pno, data, NFTLM_PAGE_SIZE(nftlm));
    if (ret) {
        dhara_set_error(err, DHARA_E_BAD_BLOCK);
        NAND_NFTL_ERR("[%s:%d] progError, bno=%d pno=%d", NAND_NFTL_WHERE, bno, pno);
        return -1;
    }

    return 0;
}

int dhara_nand_read(const struct dhara_nand *n, dhara_page_t p,
                    size_t offset, size_t length,
                    uint8_t *data, dhara_error_t *err)
{
    int ret;
    nftl_m_t *nftlm = NFTLM_GET(n);
    int bno = p >> NFTLM_LOG2_PAGES_PER_BLOCK(nftlm);
    const int pno = p & ((1 << NFTLM_LOG2_PAGES_PER_BLOCK(nftlm)) - 1);

    if ((bno < 0) || (bno >= NFTLM_NUM_BLOCKS(nftlm))) {
        ASSERT(0, "[%s:%d] block is invalid, bno=%d", NAND_NFTL_WHERE, bno);
    }

    if ((offset > NFTLM_PAGE_SIZE(nftlm)) || (length > NFTLM_PAGE_SIZE(nftlm)) ||
        (offset + length > NFTLM_PAGE_SIZE(nftlm))) {
        ASSERT(0, "[%s:%d] invalid range, offset=%d, length=%d",
               NAND_NFTL_WHERE, (int)offset, (int)length);
    }

    bno += nftlm->boffset;
    NAND_NFTL_DBG("[%s:%d] bno=%d pno=%d", NAND_NFTL_WHERE, bno, pno);
    ret = nftlm->conf.read(nftlm->conf.flashId, bno, pno, offset, data, length);
    if (ret != NANDFLASH_RET_OK) {
        if (ret == NANDFLASH_RET_READ_CORRECTED) {
            ret = 0;
        } else {
            NAND_NFTL_ERR("[%s:%d] readError, readlen=%d bno=%d pno=%d failed=%d",
                          NAND_NFTL_WHERE, (int)length, (int)bno, (int)pno, ret);
        }
    }
    if (ret) {
        return -1;
    }
    return 0;
}

int dhara_nand_copy(const struct dhara_nand *n,
                    dhara_page_t src, dhara_page_t dst,
                    dhara_error_t *err)
{
    int res = -1;
    int page_size = 1 << n->log2_page_size;
    nftl_m_t *nftlm = NFTLM_GET(n);
    uint8_t *buf = nftlm->conf.page_copy;

    res = dhara_nand_read(n, src, 0, page_size, buf, err);
    if (res < 0) {
        return -1;
    }

    res = dhara_nand_prog(n, dst, buf, err);
    if (res < 0) {
        return -1;
    }

    return 0;
}


/**************************************************************************
 * Public interface For FS
 **************************************************************************/
int nftlm_sync(nftl_m_t *nftlm)
{
    dhara_error_t err;
    int ret;
    ret = dhara_map_sync(&nftlm->map, &err);
    return ret;
}

int nftlm_init_del(nftl_m_t *nftlm)
{
    dhara_map_clear(&nftlm->map);
    nftlm_sync(nftlm);
    return 0;
}

int nftlm_init(nftl_m_t *nftlm, nftl_config_t *conf)
{
    nftlm->conf.flashId = conf->flashId;
    nftlm->conf.log2_oob_size = conf->log2_oob_size;

    nftlm->conf.read = conf->read;
    ASSERT(nftlm->conf.read != NULL, "[%s:%d] read is NULL", NAND_NFTL_WHERE);

    nftlm->conf.prog = conf->prog;
    ASSERT(nftlm->conf.prog != NULL, "[%s:%d] prog is NULL", NAND_NFTL_WHERE);

    nftlm->conf.erase = conf->erase;
    ASSERT(nftlm->conf.erase != NULL, "[%s:%d] erase is NULL", NAND_NFTL_WHERE);

    nftlm->conf.isbad = conf->isbad;
    ASSERT(nftlm->conf.isbad != NULL, "[%s:%d] isbad is NULL", NAND_NFTL_WHERE);

    nftlm->conf.markbad = conf->markbad;
    ASSERT(nftlm->conf.markbad != NULL, "[%s:%d] markbad is NULL", NAND_NFTL_WHERE);

    nftlm->conf.total_size = conf->total_size;
    if (nftlm->conf.total_size <= 0) {
        ASSERT(0, "[%s:%d] nand size error, total_size=%d",
               NAND_NFTL_WHERE, nftlm->conf.total_size);
    }

    nftlm->conf.num_blocks = conf->num_blocks;
    if (nftlm->conf.num_blocks <= 0) {
        ASSERT(0, "[%s:%d] num_blocks err, num_blocks=%d",
               NAND_NFTL_WHERE, conf->num_blocks);
    }

    nftlm->conf.pages_per_block = conf->pages_per_block;
    if (nftlm->conf.pages_per_block <= 0) {
        ASSERT(0, "[%s:%d] pages_per_block err, pages_per_block=%d",
               NAND_NFTL_WHERE, nftlm->conf.pages_per_block);
    }

    nftlm->conf.log2_page_size = conf->log2_page_size;
    if (conf->log2_page_size <= 0 || NFTLM_PAGE_SIZE(nftlm) != conf->page_size) {
        ASSERT(0, "[%s:%d] log2_page_size err, log2_page_size=%d page_size=%d",
               NAND_NFTL_WHERE, conf->log2_page_size, conf->page_size);
    }

    nftlm->conf.log2_ppb = conf->log2_ppb;
    if (conf->log2_ppb <= 0 || NFTLM_PAGES_PER_BLOCK(nftlm) != conf->pages_per_block) {
        ASSERT(0, "[%s:%d] log2_ppb err, log2_ppb=%d pages_per_block=%d",
               NAND_NFTL_WHERE, conf->log2_ppb, conf->pages_per_block);
    }

    nftlm->conf.block_size = conf->block_size;
    ASSERT(nftlm->conf.block_size, "[%s:%d] block_size err, block_size=%d",
           NAND_NFTL_WHERE, conf->block_size);

    nftlm->conf.log2_block_size = conf->log2_block_size;
    if (conf->log2_block_size <= 0 || NFTLM_BLOCK_SIZE(nftlm) != conf->block_size) {
        ASSERT(0, "[%s:%d] block_size err, block_size=%d",
               NAND_NFTL_WHERE, conf->block_size);
    }

    nftlm->conf.page_size = conf->page_size;
    if (nftlm->conf.page_size <= 0) {
        ASSERT(0, "[%s:%d] page_size err, page_size=%d ", NAND_NFTL_WHERE, nftlm->conf.page_size);
    }

    nftlm->nand.log2_page_size = nftlm->conf.log2_page_size;
    nftlm->nand.log2_ppb = nftlm->conf.log2_ppb;
    nftlm->nand.num_blocks = nftlm->conf.num_blocks;
    nftlm->nand.context = nftlm;
    nftlm->conf.page_copy = conf->page_copy;
    nftlm->conf.page_buff = conf->page_buff;
    nftlm->conf.gc_ratio = conf->gc_ratio;
    if (nftlm->conf.gc_ratio <= 0) {
        nftlm->conf.gc_ratio = 4;
    }

#ifdef NAND_NFTL_CLEAR
    for (int i = 0; i < nftlm->nand.num_blocks; i++) {
        nandflash_erase_block(nftlm->conf.flashId, i + nftlm->boffset);
        NAND_NFTL_DBG("[%s:%d] erase i + nftlm->boffset=%d", NAND_NFTL_WHERE, i + nftlm->boffset);
    }
#endif

    NAND_NFTL_INFO("[%s:%d] log2_page_size=%d log2_ppb=%d num_blocks=%d",
                   NAND_NFTL_WHERE, nftlm->nand.log2_page_size,
                   nftlm->nand.log2_ppb, nftlm->nand.num_blocks);

    dhara_map_init(&nftlm->map, &nftlm->nand, nftlm->conf.page_buff, nftlm->conf.gc_ratio);
    dhara_error_t err;
    int ret;
    ret = dhara_map_resume(&nftlm->map, &err);
    NAND_NFTL_INFO("[%s:%d] map->count=%d", NAND_NFTL_WHERE, nftlm->map.count);
    if (ret < 0)
        NAND_NFTL_ERR("[%s:%d] ret=%d err=[%s]", NAND_NFTL_WHERE, ret, dhara_strerror(err));
    return 0;
}

int nftlm_read(nftl_m_t *nftlm, dhara_sector_t s, uint8_t *data)
{
    dhara_error_t err;
    int ret;
    ret = dhara_map_read(&nftlm->map, s, data, &err);
    return ret;
}

int nftlm_write(nftl_m_t *nftlm, dhara_sector_t s, uint8_t *data)
{
    dhara_error_t err;
    int ret;
    ret = dhara_map_write(&nftlm->map, s, data, &err);
    return ret;
}

/* Obtain the maximum capacity of the map. */
uint32_t nftlm_capacity_free(nftl_m_t *nftlm)
{
    uint32_t ret;
    ret = dhara_map_capacity(&nftlm->map);
    return ret;
}

/* Obtain the current number of allocated sectors. */
uint32_t nftlm_capacity_used(nftl_m_t *nftlm)
{
    uint32_t ret;
    ret = dhara_map_size(&nftlm->map);
    return ret;
}

static nftl_m_t *fs_nand_nftlm_get(int id, int partidx)
{
    return &g_nftlm[id][partidx];
}

static struct nftlm_nandflash_status *fs_nand_status_get(int id)
{
    return &g_nand_status[id];
}

static int fs_nand_log2(unsigned int val)
{
    int shift = 0;
    while (val >= 2) {
        val >>= 1;
        ++shift;
    }
    return shift;
}

static int fs_nand_conf(nftl_config_t *conf, nftl_m_t *nftlm)
{
    uint32_t total_size;
    uint32_t block_size;
    uint32_t page_size;
    uint32_t page_oob_size;
    uint32_t die_num;

    memset(conf, 0x00, sizeof(nftl_config_t));

    nandflash_get_size(nftlm->status->flashid, &total_size, &block_size, &page_size, &page_oob_size, &die_num);

    conf->oob_size = 0;
    conf->page_size = page_size;
    conf->pages_per_block = block_size / page_size;
    conf->block_size = block_size;
    if (nftlm->status->partenable) {
        conf->num_blocks = nftlm->bnum;
        conf->total_size = nftlm->bnum * block_size;
        total_size = conf->total_size;
    } else {
        conf->num_blocks = total_size / block_size;
        conf->total_size = total_size;
        nftlm->bnum = conf->num_blocks;
        nftlm->boffset = 0;
    }

    conf->log2_oob_size = 0;
    conf->log2_page_size = fs_nand_log2(page_size);
    conf->log2_ppb = fs_nand_log2(block_size / page_size);
    conf->log2_block_size = fs_nand_log2(block_size);

    conf->markbad = nandflash_mark_bad_block;
    conf->isbad = nandflash_block_is_bad;
    conf->read = nandflash_read_page;
    conf->prog = nandflash_write_page;
    conf->erase = nandflash_erase_block;

    conf->gc_ratio = 4;
    conf->page_buff = NULL;
    conf->flashId = nftlm->status->flashid;

    conf->page_buff = fs_mm_zalloc(conf->page_size);
    ASSERT(conf->page_buff != NULL, "no memory");
    conf->page_copy = fs_mm_zalloc(conf->page_size);
    ASSERT(conf->page_copy != NULL, "no memory");
    NAND_NFTL_INFO("[%s:%d] totalSize=%d blockSize=%d pageSize=%d",
                   NAND_NFTL_WHERE, total_size, block_size, page_size);
    return 0;
}

int fs_nand_nftlm_open(struct inode *inode)
{
    nftl_m_t *nftlm = NULL;
    nftl_config_t conf;
    int ret;
    int32_t flashid;

    nftlm = (nftl_m_t *)inode->i_private;
    fs_mutex_lock(nftlm->status->mutex);
    nftlm->status->refs++;
    if (nftlm->status->opened == 0) {
        flashid = nftlm->status->flashid;
        ret = nandflash_init(flashid);
        ASSERT(ret == NANDFLASH_RET_OK, "Failed to init nandflash: %d", ret);
        nftlm->status->opened = 1;
    }
    fs_mutex_unlock(nftlm->status->mutex);
    fs_nand_conf(&conf, nftlm);
    nftlm_init(nftlm, &conf);
    NAND_NFTL_INFO("[%s:%d] nand opened inode=%p %p", NAND_NFTL_WHERE, inode, nftlm);
    return 0;
}

int fs_nand_nftlm_erase(uint32_t pageid, uint32_t page_count)
{
    int ret = 0;
    NAND_NFTL_DBG("[%s:%d] eraseSectorId=%d", NAND_NFTL_WHERE, pageid);
    return ret;
}

int fs_nand_nftlm_close(struct inode *inode)
{
    nftl_m_t *nftlm = NULL;
    nftlm = (nftl_m_t *)inode->i_private;

    fs_mutex_lock(nftlm->status->mutex);
    nftlm->status->refs--;
    if (nftlm->status->refs == 0) {
        nandflash_close(nftlm->conf.flashId);
        nftlm->status->opened = 0;
    }
    fs_mutex_unlock(nftlm->status->mutex);
    fs_mm_free(nftlm->conf.page_buff);
    nftlm->conf.page_buff = NULL;
    fs_mm_free(nftlm->conf.page_copy);
    nftlm->conf.page_copy = NULL;
    if (nftlm->status->refs == 0)
        fs_mutex_destroy(&nftlm->status->mutex);
    return 0;
}

ssize_t fs_nand_nftlm_prog(struct inode *inode,
                           const unsigned char *buf,
                           blkcnt_t pageid, unsigned int page_count)
{
    int ret;
    nftl_m_t *nftlm = NULL;
    int i = 0;
    uint32_t len;

    nftlm = (nftl_m_t *)inode->i_private;
    len = nftlm->conf.page_size;

    NAND_NFTL_DBG("[%s:%d] progSectorId=%d", NAND_NFTL_WHERE, pageid);
    fs_mutex_lock(nftlm->status->mutex);
    for (i = 0; i < page_count; i++) {
        ret = nftlm_write(nftlm, pageid + i, (uint8_t *)buf + i * len);
        if (ret != 0) {
            break;
        }
    }
    fs_mutex_unlock(nftlm->status->mutex);
    return (ret == 0) ? page_count : -1;
}

ssize_t fs_nand_nftlm_read(struct inode *inode, unsigned char *buf,
                           blkcnt_t pageid, unsigned int page_count)
{
    int ret;
    nftl_m_t *nftlm = NULL;
    int i = 0;

    nftlm = (nftl_m_t *)inode->i_private;
    uint32_t len = nftlm->conf.page_size;
    NAND_NFTL_DBG("[%s:%d] readSectorId=%d", NAND_NFTL_WHERE, pageid);
    fs_mutex_lock(nftlm->status->mutex);
    for (i = 0; i < page_count; i++) {
        ret = nftlm_read(nftlm, pageid + i, buf + len * i);
        if (ret != 0) {
            break;
        }
    }
    fs_mutex_unlock(nftlm->status->mutex);
    return (ret == 0) ? page_count : -1;
}

int fs_nand_nftlm_sync(nftl_m_t *nftlm)
{
    int ret = 0;
    ret = nftlm_sync(nftlm);
    return ret;
}

int fs_nand_bftlm_ioctl(struct inode *inode, int cmd, unsigned long arg)
{
    nftl_m_t *nftlm = NULL;
    nftlm = (nftl_m_t *)inode->i_private;
    switch (cmd) {
        case BIOC_FLUSH:
            fs_nand_nftlm_sync(nftlm);
            return 0;
        default:
            break;
    }
    AOS_FS_TRACE(0, FS_ERR_TAG"unknown cmd, cmd: %d", __FS_ERR_WHERE__, cmd);
    return -1;
}

static int fs_nand_bftlm_geometry(struct inode *inode, struct geometry *geometry)
{
    nftl_m_t *nftlm = NULL;
    nftlm = (nftl_m_t *)inode->i_private;

    geometry->geo_available     = true;
    geometry->geo_mediachanged  = false;
    geometry->geo_writeenabled  = true;
    geometry->geo_nsectors      = nftlm->conf.total_size / nftlm->conf.page_size;
    geometry->geo_sectorsize    = nftlm->conf.page_size;
    return 0;
}

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const struct block_operations g_bops = {
    fs_nand_nftlm_open,     /* open     */
    fs_nand_nftlm_close,    /* close    */
    fs_nand_nftlm_read,     /* read     */
    fs_nand_nftlm_prog,     /* write    */
    fs_nand_bftlm_geometry, /* geometry */
    fs_nand_bftlm_ioctl     /* ioctl    */
};

#endif
#endif

/****************************************************************************
 * Name: bes_nand_slotinitialize_dhara
 *
 * Description:
 *   Initialize one slot for operation using the nand interface
 *
 * Input Parameters:
 *   minor - The nand minor device number.  The nand device will be
 *     registered as /nandN where N is the minor number
 *
 ****************************************************************************/
int bes_nand_slotinitialize_dhara(int minor, int flashid)
{
#ifdef NANDFLASH_SUPPORT
#ifndef NANDFLASH_MTD_ENABLE
    int ret = -ENOMEM;
    nftl_m_t *nftlm = NULL;
    char devname[32];
    int partidx = 0;

    if (minor < 0 || minor > 255) {
        AOS_FS_TRACE(0, FS_ERR_TAG"param error, minor: %d", __FS_ERR_WHERE__, minor);
        return -EINVAL;
    }

    if (flashid < 0 || flashid >= HAL_FLASH_ID_NUM) {
        AOS_FS_TRACE(0, FS_ERR_TAG"param error, flashid: %d", __FS_ERR_WHERE__, flashid);
        return -EINVAL;
    }

    nftlm = fs_nand_nftlm_get(flashid, partidx);
    nftlm->status = fs_nand_status_get(flashid);
    nftlm->status->partenable = 0;
    nftlm->status->flashid = flashid;
    nftlm->boffset = 0;
    nftlm->bnum = 0;
    if (nftlm->status->mutex == NULL) {
        nftlm->status->mutex = fs_mutex_create(NULL);
    }

    snprintf(devname, 32, "/dev/nandblock%d", minor);
    ret = register_blockdriver(devname, &g_bops, 0, nftlm);
    if (ret < 0) {
        AOS_FS_TRACE(0, FS_ERR_TAG"register_blockdriver error, ret: %d", __FS_ERR_WHERE__, ret);
        return ret;
    }
#else
    /*use mtd interface*/
#endif
#endif
    return 0;
}

int bes_nand_slotinitialize_partition_dhara(int minor, int flashid, int partidx)
{
#ifdef NANDFLASH_SUPPORT
#ifndef NANDFLASH_MTD_ENABLE
    int ret = -ENOMEM;
    nftl_m_t *nftlm = NULL;
    char devname[32];

    if (minor < 0 || minor > 255) {
        AOS_FS_TRACE(0, FS_ERR_TAG"param error, minor: %d", __FS_ERR_WHERE__, minor);
        return -EINVAL;
    }

    if (flashid < 0 || flashid >= HAL_FLASH_ID_NUM) {
        AOS_FS_TRACE(0, FS_ERR_TAG"param error, flashid: %d", __FS_ERR_WHERE__, flashid);
        return -EINVAL;
    }

    if (partidx < 0 || partidx >= AOS_NAND_PART_NUM) {
        AOS_FS_TRACE(0, FS_ERR_TAG"partidx error, partidx: %d", __FS_ERR_WHERE__, partidx);
        return -EINVAL;
    }

    nftlm = fs_nand_nftlm_get(flashid, partidx);
    nftlm->status = fs_nand_status_get(flashid);
    nftlm->status->partenable = 1;
    nftlm->status->flashid = flashid;
    nftlm->bnum = bes_nandflash_slotinitialize_partition_num(partidx);
    nftlm->boffset = bes_nandflash_slotinitialize_partition_offset(partidx);
    if (nftlm->status->mutex == NULL) {
        nftlm->status->mutex = fs_mutex_create(NULL);
    }

    snprintf(devname, 32, "/dev/nandblock%d_%d", minor, partidx);
    ret = register_blockdriver(devname, &g_bops, 0, nftlm);
    if (ret < 0) {
        AOS_FS_TRACE(0, FS_ERR_TAG"register_blockdriver error, ret: %d", __FS_ERR_WHERE__, ret);
        return ret;
    }
#else
    /*use mtd interface*/
#endif
#endif
    return 0;
}

