/***************************************************************************
 *
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
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <sys/types.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "fs/fs.h"
#include "conf/fs_config.h"
#include "conf/fs_memory.h"
#include "fs/fserrno.h"
#include "fs/ioctl.h"
#include "mtd/mtd.h"
#include "hal_trace.h"

#ifdef FLASH_BASE
#include "app_flash_api.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
#define MAX_NORFLASH_PART_NUM 4

#ifndef AOS_NORFLASH_PART_NUM
#define AOS_NORFLASH_PART_NUM MAX_NORFLASH_PART_NUM
#endif

#if AOS_NORFLASH_PART_NUM > MAX_NORFLASH_PART_NUM
#  error MAX_NORFLASH_PART_NUM is 4
#endif

#ifndef AOS_NORFLASHPART1_BLOCK_NUM
#define AOS_NORFLASHPART1_BLOCK_NUM 1024
#endif

#ifndef AOS_NORFLASHPART2_BLOCK_NUM
#define AOS_NORFLASHPART2_BLOCK_NUM 1024
#endif

#ifndef AOS_NORFLASHPART3_BLOCK_NUM
#define AOS_NORFLASHPART3_BLOCK_NUM 1024
#endif

#ifndef AOS_NORFLASHPART4_BLOCK_NUM
#define AOS_NORFLASHPART4_BLOCK_NUM 1024
#endif

static uint32_t g_norflash_partition[MAX_NORFLASH_PART_NUM] = {
    AOS_NORFLASHPART1_BLOCK_NUM,
    AOS_NORFLASHPART2_BLOCK_NUM,
    AOS_NORFLASHPART3_BLOCK_NUM,
    AOS_NORFLASHPART4_BLOCK_NUM
};

/* Configuration ************************************************************/

#define CONFIG_NORFLASH_BLOCKSIZE norflash_api_get_sector_size(priv->flash_id)
#define CONFIG_NORFLASH_ERASESIZE norflash_api_get_sector_size(priv->flash_id)

#ifndef CONFIG_NORFLASH_ERASESTATE
#define CONFIG_NORFLASH_ERASESTATE 0xff
#endif

#if CONFIG_NORFLASH_ERASESTATE != 0xff && CONFIG_NORFLASH_ERASESTATE != 0x00
#error "Unsupported value for CONFIG_NORFLASH_ERASESTATE"
#endif

#undef  NORFLASH_BLKPER
#define NORFLASH_BLKPER 1

/****************************************************************************
 * Private Types
 ****************************************************************************/

/* This type represents the state of the MTD device.  The struct mtd_dev_s
 * must appear at the beginning of the definition so that you can freely
 * cast between pointers to struct mtd_dev_s and struct norflash_dev_s.
 */

struct norflash_dev_s {
    struct mtd_dev_s mtd;      /* MTD device */
    enum HAL_FLASH_ID_T flash_id;
    FAR uint8_t     *start;    /* Start of NORFLASH */
    size_t           nblocks;  /* Number of erase blocks */
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

/* The NORFLASH MTD driver may be useful just as it is, but another good use of
 * the NORFLASH MTD driver is as a FLASH simulation -- to support testing of FLASH
 * based logic without having FLASH.  CONFIG_NORFLASH_FLASHSIM will add some
 * extra logic to improve the level of FLASH simulation.
 */


/* MTD driver methods */

static int norflash_erase(FAR struct mtd_dev_s *dev,
                     off_t startblock,
                     size_t nblocks);
static ssize_t norflash_bread(FAR struct mtd_dev_s *dev,
                         off_t startblock,
                         size_t nblocks,
                         FAR uint8_t *buf);
static ssize_t norflash_bwrite(FAR struct mtd_dev_s *dev,
                          off_t startblock,
                          size_t nblocks,
                          FAR const uint8_t *buf);
static ssize_t norflash_byteread(FAR struct mtd_dev_s *dev,
                            off_t offset,
                            size_t nbytes, FAR uint8_t *buf);
#ifdef CONFIG_MTD_BYTE_WRITE
static ssize_t norflash_bytewrite(FAR struct mtd_dev_s *dev,
                             off_t offset,
                             size_t nbytes,
                             FAR const uint8_t *buf);
#endif
static int norflash_ioctl(FAR struct mtd_dev_s *dev,
                     int cmd,
                     unsigned long arg);

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static int bes_hal_flash_read(const uint32_t addr, uint8_t *dst, const uint32_t size)
{
    if(NULL == dst) {
        return -1;
    }
    app_flash_read(NORFLASH_API_MODULE_ID_MTD_FS, addr, dst, size);
    return 0;
}

static int bes_hal_flash_write(const uint32_t addr, const uint8_t *src, const uint32_t size)
{
    app_flash_program(NORFLASH_API_MODULE_ID_MTD_FS, addr, (uint8_t *)src, size, false);
    app_flash_flush_pending_op(NORFLASH_API_MODULE_ID_MTD_FS, NORFLASH_API_WRITTING);
    return 0;
}

static int bes_hal_flash_erase(const uint32_t addr, const uint32_t size)
{
    app_flash_erase(NORFLASH_API_MODULE_ID_MTD_FS, addr, size);
    app_flash_flush_pending_op(NORFLASH_API_MODULE_ID_MTD_FS, NORFLASH_API_ERASING);
    return 0;
}

/****************************************************************************
 * Name: norflash_erase
 ****************************************************************************/

static int norflash_erase(FAR struct mtd_dev_s *dev, off_t startblock,
                     size_t nblocks)
{
    FAR struct norflash_dev_s *priv = (FAR struct norflash_dev_s *)dev;
    off_t offset;
    size_t nbytes;

    DEBUGASSERT(dev);

    /* Don't let the erase exceed the size of the norflash buffer */

    if (startblock >= priv->nblocks) {
        return 0;
    }

    if (startblock + nblocks > priv->nblocks) {
        nblocks = priv->nblocks - startblock;
    }

    /* Convert the erase block to a logical block and the number of blocks
     * in logical block numbers
     */

    startblock *= NORFLASH_BLKPER;
    nblocks    *= NORFLASH_BLKPER;

    /* Get the offset corresponding to the first block and the size
     * corresponding to the number of blocks.
     */

    offset = startblock * CONFIG_NORFLASH_BLOCKSIZE;
    nbytes = nblocks * CONFIG_NORFLASH_BLOCKSIZE;

    /* Then erase the data in NORFLASH */

    bes_hal_flash_erase(offset, nbytes);

    return FS_OK;
}

/****************************************************************************
 * Name: norflash_bread
 ****************************************************************************/

static ssize_t norflash_bread(FAR struct mtd_dev_s *dev,
                         off_t startblock,
                         size_t nblocks,
                         FAR uint8_t *buf)
{
    FAR struct norflash_dev_s *priv = (FAR struct norflash_dev_s *)dev;
    off_t offset;
    off_t maxblock;
    size_t nbytes;

    DEBUGASSERT(dev && buf);

    /* Don't let the read exceed the size of the norflash buffer */

    maxblock = priv->nblocks * NORFLASH_BLKPER;
    if (startblock >= maxblock) {
        return 0;
    }

    if (startblock + nblocks > maxblock) {
        nblocks = maxblock - startblock;
    }

    /* Get the offset corresponding to the first block and the size
     * corresponding to the number of blocks.
     */

    offset = startblock * CONFIG_NORFLASH_BLOCKSIZE;
    nbytes = nblocks * CONFIG_NORFLASH_BLOCKSIZE;

    /* Then read the data frp, NORFLASH */

    bes_hal_flash_read(offset, buf, nbytes);

    return nblocks;
}

/****************************************************************************
 * Name: norflash_bwrite
 ****************************************************************************/

static ssize_t norflash_bwrite(FAR struct mtd_dev_s *dev, off_t startblock,
                          size_t nblocks, FAR const uint8_t *buf)
{
    FAR struct norflash_dev_s *priv = (FAR struct norflash_dev_s *)dev;
    off_t offset;
    off_t maxblock;
    size_t nbytes;

    DEBUGASSERT(dev && buf);

    /* Don't let the write exceed the size of the norflash buffer */

    maxblock = priv->nblocks * NORFLASH_BLKPER;
    if (startblock >= maxblock) {
        return 0;
    }

    if (startblock + nblocks > maxblock) {
        nblocks = maxblock - startblock;
    }

    /* Get the offset corresponding to the first block and the size
     * corresponding to the number of blocks.
     */

    offset = startblock * CONFIG_NORFLASH_BLOCKSIZE;
    nbytes = nblocks * CONFIG_NORFLASH_BLOCKSIZE;

    /* Then write the data to NORFLASH */

    bes_hal_flash_write(offset, buf, nbytes);

    return nblocks;
}

/****************************************************************************
 * Name: norflash_byteread
 ****************************************************************************/

static ssize_t norflash_byteread(FAR struct mtd_dev_s *dev, off_t offset,
                            size_t nbytes, FAR uint8_t *buf)
{
    FAR struct norflash_dev_s *priv = (FAR struct norflash_dev_s *)dev;
    off_t maxoffset;

    DEBUGASSERT(dev && buf);

    /* Don't let the read exceed the size of the norflash buffer */

    maxoffset = priv->nblocks * CONFIG_NORFLASH_ERASESIZE;
    if (offset >= maxoffset) {
        return 0;
    }

    if (offset + nbytes > maxoffset) {
        nbytes = maxoffset - offset;
    }

    bes_hal_flash_read(offset, buf, nbytes);

    return nbytes;
}

/****************************************************************************
 * Name: norflash_bytewrite
 ****************************************************************************/

#ifdef CONFIG_MTD_BYTE_WRITE
static ssize_t norflash_bytewrite(FAR struct mtd_dev_s *dev, off_t offset,
                             size_t nbytes, FAR const uint8_t *buf)
{
    FAR struct norflash_dev_s *priv = (FAR struct norflash_dev_s *)dev;
    off_t maxoffset;

    DEBUGASSERT(dev && buf);

    /* Don't let the write exceed the size of the norflash buffer */

    maxoffset = priv->nblocks * CONFIG_NORFLASH_ERASESIZE;
    if (offset >= maxoffset) {
        return 0;
    }

    if (offset + nbytes > maxoffset) {
        nbytes = maxoffset - offset;
    }

    /* Then write the data to NORFLASH */
    bes_hal_flash_write(offset, buf, nbytes);

    return nbytes;
}
#endif

/****************************************************************************
 * Name: norflash_ioctl
 ****************************************************************************/

static int norflash_ioctl(FAR struct mtd_dev_s *dev, int cmd, unsigned long arg)
{
    FAR struct norflash_dev_s *priv = (FAR struct norflash_dev_s *)dev;
    int ret = -EINVAL; /* Assume good command with bad panorflasheters */

    switch (cmd) {
        case MTDIOC_GEOMETRY: {
            FAR struct mtd_geometry_s *geo =
                (FAR struct mtd_geometry_s *)((uintptr_t)arg);
            if (geo) {
                memset(geo, 0, sizeof(*geo));

                /* Populate the geometry structure with information need to
                 * know the capacity and how to access the device.
                 */

                geo->blocksize    = CONFIG_NORFLASH_BLOCKSIZE;
                geo->erasesize    = CONFIG_NORFLASH_ERASESIZE;
                geo->neraseblocks = priv->nblocks;
                ret               = FS_OK;
            }
        }
        break;

        case BIOC_XIPBASE: {
            FAR void **ppv = (FAR void **)((uintptr_t)arg);
            if (ppv) {
                /* Return (void*) base address of device memory */

                *ppv = (FAR void *)priv->start;
                ret  = FS_OK;
            }
        }
        break;

        case BIOC_PARTINFO: {
            FAR struct partition_info_s *info =
                (FAR struct partition_info_s *)arg;
            if (info != NULL) {
                info->numsectors  = priv->nblocks *
                                    CONFIG_NORFLASH_ERASESIZE /
                                    CONFIG_NORFLASH_BLOCKSIZE;
                info->sectorsize  = CONFIG_NORFLASH_BLOCKSIZE;
                info->startsector = 0;
                info->parent[0]   = '\0';
                ret               = FS_OK;
            }
        }
        break;

        case MTDIOC_BULKERASE: {
            size_t size = priv->nblocks * CONFIG_NORFLASH_ERASESIZE;

            /* Erase the entire device */
            bes_hal_flash_erase(0, size);

            ret = FS_OK;
        }
        break;

        case MTDIOC_ERASESTATE: {
            FAR uint8_t *result = (FAR uint8_t *)arg;
            *result = CONFIG_NORFLASH_ERASESTATE;

            ret = FS_OK;
        }
        break;

        case MTDIOC_FACTOR: {
            FAR struct mtd_factor_conf *factor =
                (FAR struct mtd_factor_conf *)((uintptr_t)arg);
            if (factor) {
                memset(factor, 0, sizeof(*factor));
                /* Populate the geometry structure with information need to
                 * know the capacity and how to access the device.
                 */
                factor->program_size_factor = 1;
                factor->read_size_factor    = 1;
                factor->block_size_factor   = 1;
                factor->cache_size_factor   = 1;
                ret                         = FS_OK;
            }
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
 * Name: norflashmtd_initialize
 *
 * Description:
 *   Create and initialize a NORFLASH MTD device instance.
 *
 * Input Panorflasheters:
 *   start - Address of the beginning of the allocated NORFLASH regions.
 *   size  - The size in bytes of the allocated NORFLASH region.
 *
 ****************************************************************************/

FAR struct mtd_dev_s *norflashmtd_initialize(FAR uint8_t *start, size_t size)
{
    FAR struct norflash_dev_s *priv;
    size_t nblocks;
    uint32_t start_addr = (uint32_t)start;

    // check flash id
#ifdef FLASH1_BASE
    if (((start_addr & (~HAL_NORFLASH_ADDR_MASK)) == FLASH1_NC_BASE)
        || ((start_addr & (~HAL_NORFLASH_ADDR_MASK)) == FLASH1_BASE)) {
        // flash1
        start_addr = FLASH1_NC_BASE + (start_addr & HAL_NORFLASH_ADDR_MASK);
    } else
#endif
    {
        // default flash0
        start_addr = FLASH_NC_BASE + (start_addr & HAL_NORFLASH_ADDR_MASK);
    }

    /* Create an instance of the NORFLASH MTD device state structure */

    priv = (FAR struct norflash_dev_s *)fs_mm_zalloc(sizeof(struct norflash_dev_s));
    if (!priv) {
        AOS_FS_TRACE(0, FS_ERR_TAG"Failed to allocate the NORFLASH MTD state structure", __FS_ERR_WHERE__);
        return NULL;
    }

    // register partition
    priv->flash_id   = norflash_api_get_dev_id_by_addr(start_addr);
    app_flash_register_module(NORFLASH_API_MODULE_ID_MTD_FS, priv->flash_id, start_addr, size, 0);

    /* Force the size to be an even number of the erase block size */

    nblocks = (size) / (CONFIG_NORFLASH_ERASESIZE);
    if (nblocks < 1) {
        AOS_FS_TRACE(0, FS_ERR_TAG"Need to provide at least one full erase block", __FS_ERR_WHERE__);
        fs_mm_free(priv);
        return NULL;
    }

    /* Perform initialization as necessary. (unsupported methods were
     * nullified by fs_mm_zalloc).
     */

    priv->mtd.erase  = norflash_erase;
    priv->mtd.bread  = norflash_bread;
    priv->mtd.bwrite = norflash_bwrite;
    priv->mtd.read   = norflash_byteread;
#ifdef CONFIG_MTD_BYTE_WRITE
    priv->mtd.write  = norflash_bytewrite;
#endif
    priv->mtd.ioctl  = norflash_ioctl;
    priv->mtd.name   = "norflashmtd";

    priv->start      = start;
    priv->nblocks    = nblocks;

    return &priv->mtd;
}

/****************************************************************************
 * Name: bes_norflash_slotinitialize
 *
 * Description:
 *   Initialize one slot for operation using the norflash device
 *
 * Input Panorflasheters:
 *   minor - The norflash minor device number.  The norflash device will be
 *     registered as /norflashN where N is the minor number
 *   start - flash start address, such as 0x600000/0x28600000/0x2C600000/0x2A600000/0x2D600000
 *   size - flash size, such as 0x100000
 *
 ****************************************************************************/
int bes_norflash_slotinitialize(int minor, uint8_t *start, uint32_t size)
{
    int ret;
    struct mtd_dev_s *mtd = NULL;
    char devname[16];

    mtd = norflashmtd_initialize(start, size);
    if (mtd == NULL) {
        return -1;
    }
    snprintf(devname, 16, "/dev/nor%d", minor);
    ret = register_mtddriver(devname, mtd, 0, mtd);
    if (ret < 0) {
        AOS_FS_TRACE(0, FS_ERR_TAG"register_mtddriver error, ret: %d", __FS_ERR_WHERE__, ret);
        return ret;
    }

    return 0;
}

/****************************************************************************
 * Name: bes_norflash_slotinitialize_partition
 *
 * Description:
 *   Initialize one slot for operation using the norflash device
 *
 * Input Panorflasheters:
 *   mtd  - mtd dev
 *   minor - The norflash minor device number.  The norflash device will be
 *     registered as /norflashN where N is the minor number
 *   part  - - The norflash partition device number.
 *             registered as /norflashN_N where N is the minor number
 *
 ****************************************************************************/
int bes_norflash_slotinitialize_partition(struct mtd_dev_s *mtd, int minor, int part, off_t firstblock, off_t nblocks)
{
    int ret;
    char devname[16];
    char pdevname[32];

    if (minor < 0 || minor > 255) {
        AOS_FS_TRACE(0, FS_ERR_TAG"param error, minor: %d", __FS_ERR_WHERE__, minor);
        return -EINVAL;
    }

    if (part < 0 || minor > 16) {
        AOS_FS_TRACE(0, FS_ERR_TAG"param error, part: %d", __FS_ERR_WHERE__, minor);
        return -EINVAL;
    }

    snprintf(devname, 16, "/dev/nor%d", minor);
    snprintf(pdevname, sizeof(pdevname) - 1, "/dev/nor%d_%d", minor, part);
    ret = register_mtddriver(devname, mtd, 0, mtd);
    if (ret < 0) {
        if (ret != -EEXIST) {
            AOS_FS_TRACE(0, FS_ERR_TAG"register_mtddriver error, ret: %d", __FS_ERR_WHERE__, ret);
            return ret;
        }
    }

    ret = register_mtdpartition(pdevname, 0, devname, firstblock, nblocks);
    if (ret < 0) {
        AOS_FS_TRACE(0, FS_ERR_TAG"register_mtdpartition error, ret: %d", __FS_ERR_WHERE__, ret);
        return ret;
    }

    return 0;
}

int bes_norflash_slotinitialize_partition_offset(int idx)
{
    int offset = 0;
    for (int i = 0; i < idx; i++) {
        offset += g_norflash_partition[i];
    }
    return offset;
}

void bes_norflash_drivers_register(int minor, uint8_t *start, uint32_t size)
{
#ifdef NORFLASH_PARTITION_ENABLE
    int boffset = 0;
    int bnum = 0;
    struct mtd_dev_s *mtd = NULL;
    mtd = norflashmtd_initialize(start, size);
    if (mtd == NULL) {
        return;
    }
    for (int i = 0; i < AOS_NORFLASH_PART_NUM; i++) {
        boffset = bes_norflash_slotinitialize_partition_offset(i);
        bnum = g_norflash_partition[i];
        bes_norflash_slotinitialize_partition(mtd, minor, i + 1, boffset, bnum); // /dev/nor0_i+1
    }
#else
    bes_norflash_slotinitialize(minor, start, size);
#endif
}

#endif /* FLASH_BASE */
