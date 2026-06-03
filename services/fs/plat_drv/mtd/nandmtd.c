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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hal_trace.h"
#include "nandflash_drv.h"
#include "dhara/nand.h"
#include "dhara/map.h"
#include "dhara/journal.h"
#include "fs/fs.h"
#include "fs/fserrno.h"
#include "conf/fs_memory.h"
#include "mtd/mtd.h"
#include "nandflash.h"

/****************************************************************************
 * Private Types
 ****************************************************************************/
#ifndef CONFIG_NANDMTD_ERASESTATE
#define CONFIG_NANDMTD_ERASESTATE 0xff
#endif

/*This type represents the state of the MTD device.*/
typedef struct nand_dev_s {
    int flashId;
    struct mtd_dev_s mtd; /* interface */
    uint32_t total_size;  /* total size */
    uint32_t block_size;  /* block to erase */
    uint32_t page_size;   /* block to write */
    uint32_t pages_per_block; /* page num of a block*/
    uint32_t log2_ppb;
    size_t   nblocks;         /* num of erase blocks*/
    size_t   npages;          /* numm of pages*/
    uint32_t spare_size;
    uint32_t die_num;
} nand_dev_s;

static int fs_nand_log2(unsigned int val)
{
    int shift = 0;
    while (val >= 2) {
        val >>= 1;
        ++shift;
    }
    return shift;
}

/****************************************************************************
* Name: nand_isbad
****************************************************************************/
static int nand_isbad(FAR struct mtd_dev_s *dev, off_t block)
{
    struct nand_dev_s *priv = (struct nand_dev_s *)dev->priv;
    return nandflash_block_is_bad(priv->flashId, block);
}

/****************************************************************************
* Name: nand_markbad
****************************************************************************/
static int nand_markbad(FAR struct mtd_dev_s *dev, off_t block)
{
    struct nand_dev_s *priv = (struct nand_dev_s *)dev->priv;
    return nandflash_mark_bad_block(priv->flashId, block);
}

/****************************************************************************
 * Name: nand_erase
 ****************************************************************************/
static int nand_erase(FAR struct mtd_dev_s *dev, off_t startblock, size_t nblocks)
{
    int ret;
    struct nand_dev_s *priv = (struct nand_dev_s *)dev->priv;
    DEBUGASSERT(dev);
    for (uint32_t i = 0; i < nblocks; i++) {
        ret = nandflash_erase_block(priv->flashId, startblock + i);
        if (ret != NANDFLASH_RET_OK) {
            AOS_FS_TRACE(0, FS_ERR_TAG"bno=%u ret=%d", __FS_ERR_WHERE__, (int)startblock + i, ret);
            return -1;
        }
        AOS_FS_TRACE(1, FS_DEBUG_TAG"bno=%d", __FS_DEBUG_WHERE__, (int)startblock + i);
    }
    return FS_OK;
}

/****************************************************************************
 * Name: nand_bread
 ****************************************************************************/
static ssize_t nand_bread(FAR struct mtd_dev_s *dev,
                          off_t startblock,
                          size_t nblocks,
                          FAR uint8_t *buf)
{
    DEBUGASSERT(dev && buf);
    int ret;
    struct nand_dev_s *priv = (struct nand_dev_s *)dev->priv;
    const int bno = startblock >> priv->log2_ppb;
    const int pno = startblock & ((1 << priv->log2_ppb) - 1);

    for (uint32_t i = 0; i < nblocks; i++) {
        uint8_t *tmp = buf + priv->page_size * i;
        ret = nandflash_read_page(priv->flashId, bno, pno + i, 0, tmp, priv->page_size);
        if (ret != NANDFLASH_RET_OK) {
            if (ret == NANDFLASH_RET_READ_CORRECTED) {
                ret = 0;
            } else {
                AOS_FS_TRACE(0, FS_ERR_TAG"bno=%d pno=%d", __FS_ERR_WHERE__, bno, pno + i);
                return 0;
            }
        }
        AOS_FS_TRACE(1, FS_DEBUG_TAG"bno=%d pno=%d ok", __FS_DEBUG_WHERE__, bno, pno);
    }
    return nblocks;
}

/****************************************************************************
 * Name: ram_bwrite
 ****************************************************************************/

static ssize_t nand_bwrite(FAR struct mtd_dev_s *dev, off_t startblock,
                           size_t nblocks, FAR const uint8_t *buf)
{
    DEBUGASSERT(dev && buf);
    int ret;
    struct nand_dev_s *priv = (struct nand_dev_s *)dev->priv;
    const int bno = startblock >> priv->log2_ppb;
    const int pno = startblock & ((1 << priv->log2_ppb) - 1);
    for (int i = 0; i < nblocks; i++) {
        const uint8_t *tmp = buf + priv->page_size * i;
        ret = nandflash_write_page(priv->flashId, bno, pno + i, tmp, priv->page_size);
        if (ret) {
            AOS_FS_TRACE(0, FS_ERR_TAG"bno=%d pno=%d", __FS_ERR_WHERE__, bno, pno + i);
            return 0;
        }
        AOS_FS_TRACE(1, FS_DEBUG_TAG"bno=%d pno=%d ok", __FS_DEBUG_WHERE__, bno, pno);
    }
    return nblocks;
}

/****************************************************************************
 * Name: nand_ioctl
 ****************************************************************************/

static int nand_ioctl(FAR struct mtd_dev_s *dev, int cmd, unsigned long arg)
{
    FAR struct nand_dev_s *priv = (FAR struct nand_dev_s *)dev->priv;
    int ret = -EINVAL; /* Assume good command with bad parameters */

    switch (cmd) {
        case MTDIOC_GEOMETRY: {
            FAR struct mtd_geometry_s *geo =
                (FAR struct mtd_geometry_s *)((uintptr_t)arg);
            if (geo) {
                memset(geo, 0, sizeof(*geo));
                /* Populate the geometry structure with information need to
                 * know the capacity and how to access the device.
                 */
                geo->blocksize    = priv->page_size;
                geo->erasesize    = priv->block_size;
                geo->neraseblocks = priv->nblocks;
                ret               = FS_OK;
            }
        }
        break;

        case BIOC_XIPBASE: {
            ret = -ENOTTY; /* Bad command */
            break;
        }
        break;

        case BIOC_PARTINFO: {
            struct partition_info_s *info = (struct partition_info_s *)arg;
            if (info != NULL) {
                info->numsectors  = priv->nblocks * priv->pages_per_block;
                info->sectorsize  = priv->page_size;
                info->startsector = 0;
                info->parent[0]   = '\0';
                ret               = FS_OK;
            }
        }
        break;

        case MTDIOC_BULKERASE: {
            /* Erase the entire device */
            uint8_t *result = (uint8_t *)arg;
            nand_erase(dev, 0, priv->nblocks);
            *result = 0xFF;
            ret = FS_OK;
        }
        break;

        case MTDIOC_ERASESTATE: {
            uint8_t *result = (FAR uint8_t *)arg;
            *result = CONFIG_NANDMTD_ERASESTATE;
            ret = FS_OK;
        }
        break;

        default:
            ret = -ENOTTY; /* Bad command */
            break;
    }

    return ret;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: nandtd_initialize
 *
 * Description:
 *   Create and initialize a RAM MTD device instance.
 *
 * Input Parameters:
 *   start - Address of the beginning of the allocated RAM regions.
 *   size  - The size in bytes of the allocated RAM region.
 *
 ****************************************************************************/
struct mtd_dev_s *nanmtd_initialize(int flashid)
{
    int ret;
    struct nand_dev_s *priv;

    if (flashid < 0 || flashid >= HAL_FLASH_ID_NUM) {
        AOS_FS_TRACE(0, FS_ERR_TAG"param error, flashid: %d", __FS_ERR_WHERE__, flashid);
        return NULL;
    }

    /* Create an instance of the RAM MTD device state structure */
    priv = (struct nand_dev_s *)fs_mm_zalloc(sizeof(struct nand_dev_s));
    if (!priv) {
        AOS_FS_TRACE(0, FS_ERR_TAG"Failed to allocate the RAM MTD state structure", __FS_ERR_WHERE__);
        return NULL;
    }

    /* Force the size to be an even number of the erase block size */
    ret = nandflash_init(flashid);
    ASSERT(ret == NANDFLASH_RET_OK, "Failed to init nandflash: %d", ret);
    nandflash_get_size(flashid, &priv->total_size, &priv->block_size, &priv->page_size, &priv->spare_size, &priv->die_num);
    priv->nblocks = priv->total_size / priv->block_size;
    if (priv->nblocks < 1) {
        AOS_FS_TRACE(0, FS_ERR_TAG"Need to provide at least one full erase block", __FS_ERR_WHERE__);
        fs_mm_free(priv);
        return NULL;
    }

#ifdef NAND_NFTL_CLEAR
    for (int i = 0; i < priv->nblocks; i++) {
        nandflash_erase_block(flashid, i);
    }
#endif

    /* Perform initialization as necessary. (unsupported methods were
     * nullified by fs_mm_zalloc).
     */
    priv->flashId    = flashid;
    priv->pages_per_block = priv->block_size / priv->page_size;
    priv->log2_ppb   = fs_nand_log2(priv->pages_per_block);
    priv->mtd.erase  = nand_erase;
    priv->mtd.bread  = nand_bread;
    priv->mtd.bwrite = nand_bwrite;
    priv->mtd.read   = NULL;
#ifdef CONFIG_MTD_BYTE_WRITE
    priv->mtd.write  = NULL;
#endif
    priv->mtd.isbad   = nand_isbad;
    priv->mtd.markbad = nand_markbad;
    priv->mtd.ioctl   = nand_ioctl;
    priv->mtd.name    = "nandmtd";
    priv->mtd.priv    = priv;
    return &priv->mtd;
}
#endif

/****************************************************************************
 * Name: bes_nand_slotinitialize_dhara_cache
 *
 * Description:
 *   Initialize one slot for operation using the nand interface
 *
 * Input Parameters:
 *   minor - The nand minor device number.  The nand device will be
 *     registered as /nandN where N is the minor number
 *
 ****************************************************************************/
int bes_nand_slotinitialize_dhara_cache(int minor, int flashid)
{
#ifdef NANDFLASH_SUPPORT
    struct mtd_dev_s *mtd = NULL;
    mtd = nanmtd_initialize(flashid);
    if (mtd == NULL) {
        AOS_FS_TRACE(0, FS_ERR_TAG"nanmtd_initialize error", __FS_ERR_WHERE__);
        return -1;
    }
#ifdef NANDFLASH_MTD_ENABLE
    int ret;
    ret = dhara_initialize(minor, mtd);
    if (ret < 0) {
        AOS_FS_TRACE(0, FS_ERR_TAG"dhara_initialize error, ret: %d", __FS_ERR_WHERE__, ret);
        return ret;
    }
#endif
#endif
    return 0;
}

/****************************************************************************
 * Name: bes_nand_slotinitialize_yaffs
 *
 * Description:
 *   Initialize one slot for operation using the nand interface with yaffs
 *
 * Input Parameters:
 *   minor - The nand minor device number.  The nand device will be
 *     registered as /nandN where N is the minor number
 *
 ****************************************************************************/
int bes_nand_slotinitialize_yaffs(int minor, int flashid)
{
#ifdef NANDFLASH_SUPPORT
    struct mtd_dev_s *mtd = NULL;
    mtd = nanmtd_initialize(flashid);
    if (mtd == NULL) {
        AOS_FS_TRACE(0, FS_ERR_TAG"nanmtd_initialize error", __FS_ERR_WHERE__);
        return -1;
    }
#if defined(NANDFLASH_MTD_YAFFS_ENABLE)
    int ret;
    char devname[32];
    snprintf(devname, 32, "/dev/nandblock%d", minor);
    ret = register_mtddriver(devname, mtd, 0, mtd);
    if (ret < 0) {
        AOS_FS_TRACE(0, FS_ERR_TAG"register_mtddriver error, ret: %d", __FS_ERR_WHERE__, ret);
        return ret;
    }
#endif
#endif
    return 0;
}

/****************************************************************************
 * Name: bes_nand_slotinitialize_partition_yaffs
 *
 * Description:
 *   Initialize one slot for operation using the nandflash device for yaffs
 *
 * Input Panorflasheters:
 *   mtd  - mtd dev
 *   minor - The nandflash minor device number. The nandflash device will be
 *     registered as /nandblockN where N is the minor number
 *             registered as /nandblockN_P where N is the minor number
 *             registered as /nandblockN_P where P is the part number
 *
 ****************************************************************************/
int bes_nand_slotinitialize_partition_yaffs(int minor, int flashid)
{
#ifdef NANDFLASH_SUPPORT
#ifdef NANDFLASH_MTD_YAFFS_PARTITION_ENABLE
    int ret;
    int boffset = 0;
    int bnum = 0;
    int tbnum = 0;
    int tblocks = 0;
    int tblocks1 = 0;
    char devname[32];
    char pdevname[32];
    struct mtd_geometry_s geo;
    struct mtd_dev_s *mtd = NULL;

    mtd = nanmtd_initialize(flashid);
    if (mtd == NULL) {
        return -EINVAL;
    }

    snprintf(devname, sizeof(devname) - 1, "/dev/nandblock%d", minor);
    ret = register_mtddriver(devname, mtd, 0, mtd);
    if (ret < 0) {
        if (ret != -EEXIST) {
            AOS_FS_TRACE(0, FS_ERR_TAG"register_mtddriver error, ret: %d", __FS_ERR_WHERE__, ret);
            return ret;
        }
    }

    nand_ioctl(mtd, MTDIOC_GEOMETRY, (unsigned long)&geo);
    for (int i = 0; i < AOS_NAND_PART_NUM; i++) {
        boffset = bes_nandflash_slotinitialize_partition_offset(i);
        bnum = bes_nandflash_slotinitialize_partition_num(i);
        tbnum += bnum;
        tblocks = tbnum / (geo.erasesize / geo.blocksize);
        tblocks1 = tbnum % (geo.erasesize / geo.blocksize);
        if(tblocks > geo.neraseblocks || (tblocks == geo.neraseblocks && tblocks1 > 0)) {
            ASSERT(0, FS_ERR_TAG"WARNNING partition param error!!!", __FS_ERR_WHERE__);
            return -EINVAL;
        }
        memset(pdevname, 0x00, sizeof(pdevname));
        snprintf(pdevname, sizeof(pdevname) - 1, "/dev/nandblock%d_%d", minor, i + 1);
        ret = register_mtdpartition(pdevname, 0, devname, boffset, bnum);
        if (ret < 0) {
            AOS_FS_TRACE(0, FS_ERR_TAG"reg mtdpartition error, ret: %d", __FS_ERR_WHERE__, ret);
            return ret;
        }
    }
#endif
#endif
    return 0;
}

