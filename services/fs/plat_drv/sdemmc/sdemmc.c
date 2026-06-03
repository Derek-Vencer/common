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
#include <stdio.h>
#include "hal_dma.h"
#include "hal_cache.h"
#include "hal_sdmmc.h"
#include "hal_iomux.h"
#include "hal_sysfreq.h"
#include "string.h"
#include "hal_trace.h"
#include "fs/fs.h"
#include "fs_cache.h"

#define MAX_SDMMC_PART_NUM 4

#ifndef AOS_SDMMC_PART_NUM
#define AOS_SDMMC_PART_NUM MAX_SDMMC_PART_NUM
#endif

#if AOS_SDMMC_PART_NUM > MAX_SDMMC_PART_NUM
#  error MAX_SDMMC_PART_NUM is 4
#endif

#ifndef AOS_SDMMCPART1_BLOCK_NUM
#define AOS_SDMMCPART1_BLOCK_NUM 102400
#endif

#ifndef AOS_SDMMCPART2_BLOCK_NUM
#define AOS_SDMMCPART2_BLOCK_NUM 102400
#endif

#ifndef AOS_SDMMCPART3_BLOCK_NUM
#define AOS_SDMMCPART3_BLOCK_NUM 102400
#endif

#ifndef AOS_SDMMCPART4_BLOCK_NUM
#define AOS_SDMMCPART4_BLOCK_NUM 102400
#endif

static uint32_t g_sdmmc_partition[MAX_SDMMC_PART_NUM] = {AOS_SDMMCPART1_BLOCK_NUM, AOS_SDMMCPART2_BLOCK_NUM,
                                                         AOS_SDMMCPART3_BLOCK_NUM, AOS_SDMMCPART4_BLOCK_NUM
                                                        };

struct fs_sdmmc_conf {
    int opened;
    struct HAL_SDMMC_CONFIG_T cfg;
    enum HAL_SDMMC_ID_T id;
    uint32_t ref;
    void *mutex;
#ifdef FS_SDMMC_CACHE_ENABLE
    struct fs_cache_seg_conf cachecfg;
    struct fs_cache_manager cache;
#endif
};

static struct fs_sdmmc_conf *g_sdmmc_priv[HAL_SDMMC_ID_NUM];

static int fs_device_sdmmc_card_write(void *priv, const unsigned char *buffer, blkcnt_t card_offset, unsigned int nsectors);
static int fs_device_sdmmc_card_read(void* priv, unsigned char *buffer, blkcnt_t card_offset, unsigned int nsectors);

static void bes_sdmmc_power_on(void)
{
}

static unsigned int fs_get_sdmmc_card_sector_count(struct inode *inode)
{
    uint32_t sector_count = 0;
    uint32_t sector_size = 0;
    struct fs_sdmmc_conf *conf = (struct fs_sdmmc_conf *)(inode->i_private);

    fs_mutex_lock(conf->mutex);
    hal_sdmmc_info(conf->id, &sector_count, &sector_size);
    fs_mutex_unlock(conf->mutex);

    return sector_count;
}

static unsigned int fs_get_sdmmc_card_sector_size(struct inode *inode)
{
    uint32_t sector_count = 0;
    uint32_t sector_size = 0;
    struct fs_sdmmc_conf *conf = (struct fs_sdmmc_conf *)(inode->i_private);

    fs_mutex_lock(conf->mutex);
    hal_sdmmc_info(conf->id, &sector_count, &sector_size);
    fs_mutex_unlock(conf->mutex);

    return sector_size;
}

static int fs_device_sdmmc_card_open(struct inode *inode)
{
    struct fs_sdmmc_conf *conf = (struct fs_sdmmc_conf *)(inode->i_private);

    fs_mutex_lock(conf->mutex);
    if (conf->opened == 1) {
        fs_mutex_unlock(conf->mutex);
        return 0;
    }

    int32_t ret = 0;
    bes_sdmmc_power_on();
    ret = hal_sdmmc_open(conf->id, &conf->cfg);
    if (ret != 0) {
        AOS_FS_TRACE(0, FS_ERR_TAG"ret: 0x%d", __FS_ERR_WHERE__, ret);
        fs_mutex_unlock(conf->mutex);
        return -1;
    }
    hal_sdmmc_cache_control(conf->id, 1);
    conf->opened = 1;
#ifdef FS_SDMMC_CACHE_ENABLE
    uint32_t sector_count = 0;
    uint32_t sector_size = 0;
    blkcnt_t cache_seg[CONFIG_FS_SDMMC_CACHE_SEG_NUM];
    device_read_f readfp = fs_device_sdmmc_card_read;
    device_write_f writefp = fs_device_sdmmc_card_write;

    hal_sdmmc_info(conf->id, &sector_count, &sector_size);
    CONFIG_FS_EMMC_CACHE_CODE(cache_seg);
    fs_cache_conf_init(&conf->cachecfg, CONFIG_FS_SDMMC_CACHE_LEN, CONFIG_FS_SDMMC_CACHE_SEG_NUM, cache_seg, readfp, writefp, inode);
    ret = fs_cache_init(&conf->cachecfg, &conf->cache, sector_size);
    ASSERT(ret == 0, FS_ERR_TAG"fs_cache_init failed", __FS_ERR_WHERE__);
#endif

    AOS_FS_TRACE(1, FS_DEBUG_TAG"sdmmc %d opened", __FS_ERR_WHERE__, conf->id);
    fs_mutex_unlock(conf->mutex);
    return 0;
}

static int fs_device_sdmmc_card_close(struct inode *inode)
{
    struct fs_sdmmc_conf *conf = (struct fs_sdmmc_conf *)(inode->i_private);

    fs_mutex_lock(conf->mutex);
    conf->ref--;
    AOS_FS_TRACE(1, FS_DEBUG_TAG"priv=%p priv->ref=%d inode=%p", __FS_ERR_WHERE__, conf, conf->ref, inode);
    if (conf->ref == 0) {
        AOS_FS_TRACE(1, FS_DEBUG_TAG"sdmmc %d closed", __FS_ERR_WHERE__, conf->id);
        hal_sdmmc_close(conf->id);
        conf->opened = 0;
#ifdef FS_SDMMC_CACHE_ENABLE
        fs_cache_delinit(&conf->cache);
#endif

    }
    fs_mutex_unlock(conf->mutex);
    return 0;
}

static int fs_device_sdmmc_card_write(void *priv, const unsigned char *buffer, blkcnt_t card_offset, unsigned int nsectors)
{
    int32_t wsize = 0;
    struct inode *inode = (struct inode *)priv;
    struct fs_sdmmc_conf *conf = (struct fs_sdmmc_conf *)(inode->i_private);

#if (PSRAM_SIZE > 0)
    uint32_t sector_size;
    if ((uintptr_t)buffer >= PSRAM_BASE && (uintptr_t)buffer < (PSRAM_BASE + PSRAM_SIZE)) {
        sector_size = fs_get_sdmmc_card_sector_size(inode);
        hal_dma_sync_for_device((const void *)ADDR_ALIGN_TO_CACHE((uintptr_t)buffer),
                            LEN_ALIGN_TO_CACHE(nsectors * sector_size, (uintptr_t)buffer));
    }
#endif

    fs_mutex_lock(conf->mutex);
    wsize = hal_sdmmc_write_blocks(conf->id, card_offset, nsectors, (uint8_t *)buffer);
    fs_mutex_unlock(conf->mutex);

    if (wsize != nsectors)
        return -1;

    return nsectors;
}

static int fs_device_sdmmc_card_read(void* priv, unsigned char *buffer, blkcnt_t card_offset, unsigned int nsectors)
{
    int32_t rsize = 0;
    struct inode *inode = (struct inode *)priv;
    struct fs_sdmmc_conf *conf = (struct fs_sdmmc_conf *)(inode->i_private);

#if (PSRAM_SIZE > 0)
    uint32_t sector_size;
    if ((uintptr_t)buffer >= PSRAM_BASE && (uintptr_t)buffer < (PSRAM_BASE + PSRAM_SIZE)) {
        sector_size = fs_get_sdmmc_card_sector_size(inode);
        hal_dma_sync_for_cpu((const void *)ADDR_ALIGN_TO_CACHE((uintptr_t)buffer),
                            LEN_ALIGN_TO_CACHE(nsectors * sector_size, (uintptr_t)buffer));
    }
#endif

    rsize = hal_sdmmc_read_blocks(conf->id, card_offset, nsectors, (uint8_t *)buffer);
    if (rsize != nsectors)
        return -1;

    return nsectors;
}

static int sdemmc_card_open(struct inode *inode)
{
    int ret;
    ret = fs_device_sdmmc_card_open(inode);
    return ret;
}

static ssize_t sdemmc_card_write(struct inode *inode,
                                 const unsigned char *buffer,
                                 blkcnt_t start_sector, unsigned int nsectors)
{
    int ret;
    struct fs_sdmmc_conf *conf = (struct fs_sdmmc_conf *)(inode->i_private);

    fs_mutex_lock(conf->mutex);
#ifdef FS_SDMMC_CACHE_ENABLE
    ret = fs_cache_write(&conf->cache, buffer, start_sector, nsectors);
#else
    ret = fs_device_sdmmc_card_write(inode, (void *)buffer, start_sector, nsectors);
#endif
    fs_mutex_unlock(conf->mutex);

    return ret;
}

static ssize_t sdemmc_card_read(struct inode *inode, FAR unsigned char *buffer,
                                blkcnt_t start_sector, unsigned int nsectors)
{
    int ret;
    struct fs_sdmmc_conf *conf = (struct fs_sdmmc_conf *)(inode->i_private);

    fs_mutex_lock(conf->mutex);
#ifdef FS_SDMMC_CACHE_ENABLE
    ret = fs_cache_read(&conf->cache, buffer, start_sector, nsectors);
#else
    ret = fs_device_sdmmc_card_read(inode, buffer, start_sector, nsectors);
#endif
    fs_mutex_unlock(conf->mutex);
    return ret;
}

static int sdemmc_card_close(struct inode *inode)
{
    fs_device_sdmmc_card_close(inode);
    return 0;
}

static int sdemmc_card_ioctl(struct inode *inode, int cmd, unsigned long arg)
{
#ifdef FS_SDMMC_CACHE_ENABLE
    struct fs_sdmmc_conf *conf = (struct fs_sdmmc_conf *)(inode->i_private);
#endif

    switch (cmd) {
        case BIOC_FLUSH:
#ifdef FS_SDMMC_CACHE_ENABLE
            fs_cache_sync(&conf->cache);
#endif
            return 0;
        default:
            break;
    }
    AOS_FS_TRACE(0, FS_ERR_TAG"unknown cmd, cmd: %d", __FS_ERR_WHERE__, cmd);
    return -1;
}

static int sdemmc_card_geometry(struct inode *inode, struct geometry *geometry)
{
    sdemmc_card_open(inode);

    geometry->geo_available     = true;
    geometry->geo_mediachanged  = false;
    geometry->geo_writeenabled  = true;
    geometry->geo_nsectors      = fs_get_sdmmc_card_sector_count(inode);
    geometry->geo_sectorsize    = fs_get_sdmmc_card_sector_size(inode);
    return 0;
}

static int sdemmc_card_priv_conf_init(struct HAL_SDMMC_CONFIG_T *cfg, enum HAL_SDMMC_ID_T id, struct fs_sdmmc_conf **ppriv)
{
    struct fs_sdmmc_conf *priv = NULL;

    if (g_sdmmc_priv[id] != NULL) {
        priv = g_sdmmc_priv[id];
        fs_mutex_lock(priv->mutex);
        priv->ref++;
        priv->cfg.ddr_mode = cfg->ddr_mode;
        priv->cfg.bus_speed = cfg->bus_speed;
        priv->cfg.bus_width = cfg->bus_width;
        priv->cfg.device_type = cfg->device_type;
        priv->cfg.callback = NULL;
        fs_mutex_unlock(priv->mutex);
        *ppriv = priv;
        AOS_FS_TRACE(1, FS_DEBUG_TAG"priv=%p priv->ref=%d", __FS_ERR_WHERE__, priv, priv->ref);
        return 0;
    }

    priv = (struct fs_sdmmc_conf *)fs_mm_zalloc(sizeof(struct fs_sdmmc_conf));
    if (priv == NULL) {
        AOS_FS_TRACE(0, FS_ERR_TAG"no more memory", __FS_ERR_WHERE__);
        return -ENOMEM;
    }

    g_sdmmc_priv[id] = priv;
    priv->mutex = fs_mutex_create(NULL);
    fs_mutex_lock(priv->mutex);
    if (cfg == NULL) {
        priv->cfg.ddr_mode = false;
        priv->cfg.bus_speed = 48 * 1000 * 1000;
        priv->cfg.bus_width = HAL_SDMMC_BUS_WIDTH_8;
        priv->cfg.device_type = HAL_SDMMC_DEVICE_TYPE_EMMC;
        priv->cfg.callback = NULL;
        priv->id = id;
    } else {
        priv->cfg.ddr_mode = cfg->ddr_mode;
        priv->cfg.bus_speed = cfg->bus_speed;
        priv->cfg.bus_width = cfg->bus_width;
        priv->cfg.device_type = cfg->device_type;
        priv->cfg.callback = NULL;
        priv->id = id;
    }
    priv->ref++;
    fs_mutex_unlock(priv->mutex);
    *ppriv = priv;
    AOS_FS_TRACE(1, FS_DEBUG_TAG"priv=%p priv->ref=%d", __FS_ERR_WHERE__, priv, priv->ref);
    return 0;
}

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const struct block_operations g_sdemmc_bops = {
    sdemmc_card_open,     /* open     */
    sdemmc_card_close,    /* close    */
    sdemmc_card_read,     /* read     */
    sdemmc_card_write,    /* write    */
    sdemmc_card_geometry, /* geometry */
    sdemmc_card_ioctl     /* ioctl    */
};

#if defined(CONFIG_GPT_PARTITION)
void bes_sdemmc_partition_handler(struct partition_s *part, void *path)
{
    char dev[PATH_MAX];
    int ret;
    snprintf(dev, sizeof(dev), "/%s", part->name);
    ret = register_blockpartition(dev, 0, path, part->firstblock, part->nblocks);
    if (ret < 0) {
        AOS_FS_TRACE(0, FS_ERR_TAG"Failed to register block device: %s %d", __FS_ERR_WHERE__, part->name, ret);
    }
    AOS_FS_TRACE(1, FS_DEBUG_TAG"Register block device:%s, first:%d, nblocks:%d, blocksize:%d",
                 __FS_DEBUG_WHERE__,
                 part->name, part->firstblock, part->nblocks, part->blocksize);
}
#endif

/****************************************************************************
 * Name: bes_sdemmc_slotinitialize
 *
 * Description:
 *   Initialize one slot for operation using the EMMC interface
 *
 * Input Parameters:
 *   minor - The SDEMMC minor device number.  The SDEMMC device will be
 *     registered as /emmcN where N is the minor number
 *   dev - And instance of an SDEMMC interface.  The SDEMMC hardware should
 *     be initialized and ready to use.
 *
 ****************************************************************************/
int bes_sdemmc_slotinitialize(struct HAL_SDMMC_CONFIG_T *cfg, enum HAL_SDMMC_ID_T id, int minor)
{
    int ret = -ENOMEM;
    struct fs_sdmmc_conf *priv = NULL;
    char devname[16];

    if (minor < 0 || minor > 255) {
        AOS_FS_TRACE(0, FS_ERR_TAG"param error, minor: %d", __FS_ERR_WHERE__, minor);
        return -EINVAL;
    }

    ret = sdemmc_card_priv_conf_init(cfg, id, &priv);
    if (ret != 0) {
        return ret;
    }

    snprintf(devname, 16, "/dev/emmc%d", minor);
    ret = register_blockdriver(devname, &g_sdemmc_bops, 0, priv);
    if (ret < 0) {
        AOS_FS_TRACE(0, FS_ERR_TAG"register_blockdriver error, ret: %d", __FS_ERR_WHERE__, ret);
        fs_mm_free(priv);
        return ret;
    }
    return 0;
}

int bes_sdemmc_slotinitialize_partition(struct HAL_SDMMC_CONFIG_T *cfg, enum HAL_SDMMC_ID_T id, int minor, int part, off_t firstsector, off_t nsectors)
{
    int ret = -ENOMEM;
    struct fs_sdmmc_conf *priv = NULL;
    char devname[32];
    char pdevname[32];

    if (minor < 0 || minor > 255) {
        AOS_FS_TRACE(0, FS_ERR_TAG"param error, minor: %d", __FS_ERR_WHERE__, minor);
        return -EINVAL;
    }

    if (part < 0 || minor > 16) {
        AOS_FS_TRACE(0, FS_ERR_TAG"param error, part: %d", __FS_ERR_WHERE__, minor);
        return -EINVAL;
    }

    ret = sdemmc_card_priv_conf_init(cfg, id, &priv);
    if (ret != 0) {
        return ret;
    }

    snprintf(devname, sizeof(devname) - 1, "/dev/emmc%d", minor);
    snprintf(pdevname, sizeof(pdevname) - 1, "/dev/emmc%d_%d", minor, part);
    ret = register_blockdriver(devname, &g_sdemmc_bops, 0, priv);
    if (ret < 0) {
        if (ret != -EEXIST) {
            AOS_FS_TRACE(0, FS_ERR_TAG"register_blockdriver error, ret: %d", __FS_ERR_WHERE__, ret);
            fs_mm_free(priv);
            return ret;
        }
    }
    ret = register_blockpartition(pdevname, 0, devname, firstsector, nsectors);
    if (ret < 0) {
        AOS_FS_TRACE(0, FS_ERR_TAG"register_blockpartition error, ret: %d", __FS_ERR_WHERE__, ret);
        fs_mm_free(priv);
        return ret;
    }
    AOS_FS_TRACE(1, FS_DEBUG_TAG"devname=[%s] pdevname=[%s]", __FS_ERR_WHERE__, devname, pdevname);
    return 0;
}

int bes_sdmmc_slotinitialize_partition_offset(int idx)
{
    int offset = 0;
    for (int i = 0; i < idx; i++) {
        offset += g_sdmmc_partition[i];
    }
    return offset;
}

void bes_sdmmc_drivers_register(void)
{
    int minor = 0;
    struct HAL_SDMMC_CONFIG_T cfg;
    enum HAL_SDMMC_ID_T id = HAL_SDMMC_ID_1;

    memset(&cfg, 0, sizeof(struct HAL_SDMMC_CONFIG_T));
    cfg.ddr_mode = false;
    cfg.bus_speed = 48 * 1000 * 1000;
    cfg.bus_width = HAL_SDMMC_BUS_WIDTH_8;
    cfg.device_type = HAL_SDMMC_DEVICE_TYPE_EMMC;
    cfg.callback = NULL;
#ifdef SDEMMC_PARTITION_ENABLE
    int boffset = 0;
    int bnum = 0;
    for (int i = 0; i < AOS_SDMMC_PART_NUM; i++) {
        boffset = bes_sdmmc_slotinitialize_partition_offset(i);
        bnum = g_sdmmc_partition[i];
        bes_sdemmc_slotinitialize_partition(&cfg, id, minor, i + 1, boffset, bnum); // /dev/emmc0_i+1
    }
#else
    bes_sdemmc_slotinitialize(&cfg, id, minor);
#endif
}

