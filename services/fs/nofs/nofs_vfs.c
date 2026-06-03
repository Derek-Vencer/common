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


/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <string.h>
#include "fs/fs.h"
#include "fs/fs_nofs.h"
#include "mtd/mtd.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Types
 ****************************************************************************/

/* This structure represents the overall mountpoint state. An instance of
 * this structure is retained as inode private data on each mountpoint that
 * is mounted with a littlefs filesystem.
 */

struct bes_nofs_mountpt_s {
    void                 *lock;
    FAR struct inode     *drv;
    struct mtd_geometry_s geo;
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static int     hal_nofs_open(struct file *filep, const char *relpath, int oflags, mode_t mode);
static int     hal_nofs_close(struct file *filep);
static ssize_t hal_nofs_read(struct file *filep, char *buffer, size_t buflen);
static ssize_t hal_nofs_write(struct file *filep, const char *buffer, size_t buflen);
static int     hal_nofs_bind(struct inode *driver, const void *data, void **handle);
static int     hal_nofs_unbind(void *handle, struct inode **driver, unsigned int flags);

/****************************************************************************
 * Public Data
 ****************************************************************************/

/* See fs_mount.c -- this structure is explicitly extern'ed there.
 * We use the old-fashioned kind of initializers so that this will compile
 * with any compiler.
 */

const struct mountpt_operations g_bes_nofs_operations = {
    hal_nofs_open,          /* open */
    hal_nofs_close,         /* close */
    hal_nofs_read,          /* read */
    hal_nofs_write,         /* write */
    NULL,                   /* seek */
    NULL,                   /* ioctl */
#ifdef CONFIG_MOUNTPT_MMAP
    NULL,                   /* mmap */
#endif
    NULL,                   /* truncate */

    NULL,                   /* sync */
    NULL,                   /* dup */
    NULL,                   /* fstat */
    NULL,                   /* fchstat */

    NULL,                   /* opendir */
    NULL,                   /* closedir */
    NULL,                   /* readdir */
    NULL,                   /* rewinddir */

    hal_nofs_bind,          /* bind */
    hal_nofs_unbind,        /* unbind */
    NULL,                   /* statfs */

    NULL,                   /* unlink */
    NULL,                   /* mkdir */
    NULL,                   /* rmdir */
    NULL,                   /* rename */
    NULL,                   /* stat */
    NULL                    /* chstat */
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/


/****************************************************************************
 * Name: hal_nofs_open
 ****************************************************************************/

static int hal_nofs_open(struct file *filep, const char *relpath, int oflags, mode_t mode)
{
    struct bes_nofs_mountpt_s *fs;
    struct bes_nofs_file_s *fp;
    struct mtd_geometry_s *geo;

    fs = filep->f_inode->i_private;
    if (filep->f_priv != NULL) {
        return FS_OK;
    }

    fp = fs_mm_zalloc(sizeof(struct bes_nofs_file_s));
    if (!fp) {
        return -ENOMEM;
    }

    geo = &fs->geo;
    fs_mutex_lock(fs->lock);
    filep->f_priv = fp;
    fp->refs++;
    fp->oflags = oflags;
    fp->data = fs_mm_zalloc(geo->blocksize * 2);
    if (fp->data == NULL) {
        fs_mm_free(fp);
        filep->f_priv = NULL;
        fs_mutex_unlock(fs->lock);
        return -ENOMEM;
    }
    fs_mutex_unlock(fs->lock);
    return FS_OK;
}

/****************************************************************************
 * Name: hal_nofs_close
 ****************************************************************************/

static int hal_nofs_close(struct file *filep)
{
    struct bes_nofs_mountpt_s *fs;
    struct bes_nofs_file_s *fp;
    struct inode *drv;

    fp = filep->f_priv;
    fs = filep->f_inode->i_private;
    drv = fs->drv;

    fs_mutex_lock(fs->lock);
    if (INODE_IS_MTD(drv)) {
        MTD_IOCTL(drv->u.i_mtd, BIOC_FLUSH, 0);
    } else {
        drv->u.i_bops->ioctl(drv, BIOC_FLUSH, 0);
    }
    fp->refs--;
    fs_mutex_unlock(fs->lock);

    if (fp->refs <= 0) {
        fs_mm_free(fp->data);
        fs_mm_free(fp);
        filep->f_priv = NULL;
    }
    return FS_OK;
}

static uint32_t offset_to_start_block(struct mtd_geometry_s *geo, uint32_t offset)
{
    return offset / geo->blocksize;
}

static uint32_t offset_to_end_block(struct mtd_geometry_s *geo, uint32_t offset, uint32_t len)
{
    return (offset + len) / geo->blocksize;
}

/****************************************************************************
 * Name: hal_nofs_block_read
 ****************************************************************************/

static int hal_nofs_block_read(struct bes_nofs_mountpt_s *fs, struct bes_nofs_file_s *fp,
                               size_t buflen, char *buffer)
{
    struct mtd_geometry_s *geo = &fs->geo;
    struct inode *drv = fs->drv;
    int ret;

    if ((buflen / geo->blocksize * geo->blocksize) != buflen) {
        ASSERT(0, FS_ERR_TAG"buflen not aligned, buflen=%u blocksize=%u",
               __FS_ERR_WHERE__, buflen, geo->blocksize);
    }

    AOS_FS_TRACE(1, FS_DEBUG_TAG"read block=%u count=%u len=%u",
                 __FS_ERR_WHERE__, fp->block, buflen / geo->blocksize, buflen);

    if (fp->block >= (geo->erasesize / geo->blocksize * geo->neraseblocks)) {
        ASSERT(0, FS_ERR_TAG"fp->block=%u invalid", __FS_ERR_WHERE__, fp->block);
    }

    if ((fp->block + (buflen / geo->blocksize) - 1) >= (geo->erasesize / geo->blocksize * geo->neraseblocks)) {
        ASSERT(0, FS_ERR_TAG"fp->block=%u bcnt=%u invalid", __FS_ERR_WHERE__, fp->block, buflen / geo->blocksize);
    }

    if (INODE_IS_MTD(drv)) {
        ret = MTD_BREAD(drv->u.i_mtd, fp->block, buflen / geo->blocksize, (void *)buffer);
    } else {
        ret = drv->u.i_bops->read(drv, (uint8_t *)buffer, fp->block, buflen / geo->blocksize);
    }

    return ret >= 0 ? buflen : 0;
}

/****************************************************************************
 * Name: hal_nofs_stream_read
 ****************************************************************************/

static int hal_nofs_stream_read(struct bes_nofs_mountpt_s *fs, struct bes_nofs_file_s *fp,
                                size_t buflen, char *buffer)
{
    struct mtd_geometry_s *geo = &fs->geo;
    uint32_t start_block = offset_to_start_block(geo, fp->offset);
    uint32_t end_block = offset_to_end_block(geo, fp->offset, buflen);
    uint32_t offset_left_start = fp->offset % geo->blocksize;
    uint32_t offset_left_end = (fp->offset + buflen) % geo->blocksize;
    uint32_t need_block = buflen / geo->blocksize;
    uint32_t tmplen = 0;
    char    *tmpbuf = NULL;
    int32_t  ret = -1;

    if (offset_left_start) {
        if (need_block <= 1 && (offset_left_start + buflen <= (geo->blocksize * 2))) {
            fp->block = start_block;
            tmplen = geo->blocksize * 2;
            AOS_FS_TRACE(1, FS_DEBUG_TAG"buflen=%u stream read, block=%u len=%u",
                         __FS_ERR_WHERE__, buflen, fp->block, tmplen);
            ret = hal_nofs_block_read(fs, fp, tmplen, fp->data);
            if (ret <= 0) {
                return ret;
            }
            memcpy(buffer, fp->data + offset_left_start, buflen);
            return buflen;
        } else {
            fp->block = start_block;
            tmplen = geo->blocksize;
            AOS_FS_TRACE(1, FS_DEBUG_TAG"buflen=%u stream read, block=%u len=%u",
                         __FS_ERR_WHERE__, buflen, fp->block, tmplen);
            ret = hal_nofs_block_read(fs, fp, geo->blocksize, fp->data);
            if (ret <= 0) {
                return ret;
            }
            memcpy(buffer, fp->data + offset_left_start, geo->blocksize - offset_left_start);

            fp->block = start_block + 1;
            tmplen = geo->blocksize * ((end_block - start_block) - 1);
            AOS_FS_TRACE(1, FS_DEBUG_TAG"buflen=%u stream read, block=%u len=%u",
                         __FS_ERR_WHERE__, buflen, fp->block, tmplen);
            ret = hal_nofs_block_read(fs, fp, tmplen, buffer + (geo->blocksize - offset_left_start));
            if (ret <= 0) {
                return ret;
            }

            fp->block = end_block;
            tmplen = geo->blocksize;
            AOS_FS_TRACE(1, FS_DEBUG_TAG"buflen=%u stream read, block=%u len=%u",
                         __FS_ERR_WHERE__, buflen, fp->block, tmplen);
            ret = hal_nofs_block_read(fs, fp, tmplen, fp->data);
            if (ret <= 0) {
                return ret;
            }
            tmpbuf = (buffer + (geo->blocksize - offset_left_start)) + (((end_block - start_block) - 1) * geo->blocksize);
            memcpy(tmpbuf, fp->data, offset_left_end);
            return buflen;
        }
        return -1;
    } else {
        if (!(buflen % geo->blocksize)) {
            fp->block = start_block;
            tmplen = geo->blocksize * (end_block - start_block);
            AOS_FS_TRACE(1, FS_DEBUG_TAG"buflen=%u stream read, block=%u len=%u",
                         __FS_ERR_WHERE__, buflen, fp->block, tmplen);
            ret = hal_nofs_block_read(fs, fp, tmplen, buffer);
            if (ret <= 0) {
                return ret;
            }
            return buflen;
        } else {
            if (need_block <= 1) {//less than two block
                fp->block = start_block;
                tmplen = geo->blocksize * 2;
                AOS_FS_TRACE(1, FS_DEBUG_TAG"buflen=%u stream read, block=%u len=%u",
                             __FS_ERR_WHERE__, buflen, fp->block, tmplen);
                ret = hal_nofs_block_read(fs, fp, tmplen, fp->data);
                if (ret <= 0) {
                    return ret;
                }
                memcpy(buffer, fp->data, buflen);
                return buflen;
            } else if (need_block > 1) {//more than two block
                fp->block = start_block;
                tmplen = geo->blocksize * (end_block - start_block);
                AOS_FS_TRACE(1, FS_DEBUG_TAG"buflen=%u stream read, block=%u len=%u",
                             __FS_ERR_WHERE__, buflen, fp->block, tmplen);
                ret = hal_nofs_block_read(fs, fp, tmplen, buffer);
                if (ret <= 0) {
                    return ret;
                }

                fp->block = end_block;
                tmplen = geo->blocksize;
                AOS_FS_TRACE(1, FS_DEBUG_TAG"buflen=%u stream read, block=%u len=%u",
                             __FS_ERR_WHERE__, buflen, fp->block, tmplen);
                ret = hal_nofs_block_read(fs, fp, tmplen, fp->data);
                if (ret <= 0) {
                    return ret;
                }
                memcpy(buffer + ((end_block - start_block)*geo->blocksize), fp->data, (buflen % geo->blocksize));
                return buflen;
            }
        }
    }
    return 0;
}

/****************************************************************************
 * Name: hal_nofs_read
 ****************************************************************************/
static ssize_t hal_nofs_read(struct file *filep, char *buffer, size_t buflen)
{
    struct bes_nofs_mountpt_s *fs;
    struct bes_nofs_file_s *fp;
    int ret;

    fp = filep->f_priv;
    fs = filep->f_inode->i_private;

    if (fp->oflags != O_RDWR && fp->oflags != O_RDONLY) {
        AOS_FS_TRACE(0, FS_ERR_TAG"has no acess to read", __FS_ERR_WHERE__);
        return 0;
    }

    fs_mutex_lock(fs->lock);
    if (fp->procflag == BES_NOFS_PROC_BLOCK) {
        AOS_FS_TRACE(1, FS_DEBUG_TAG"buflen=%u block read, block=%u", __FS_ERR_WHERE__, buflen, fp->block);
        ret = hal_nofs_block_read(fs, fp, buflen, buffer);
    } else {
        AOS_FS_TRACE(1, FS_DEBUG_TAG"buflen=%u stream read, offset=%u", __FS_ERR_WHERE__, buflen, fp->offset);
        ret = hal_nofs_stream_read(fs, fp, buflen, buffer);
    }
    fs_mutex_unlock(fs->lock);

    return ret;
}

/****************************************************************************
 * Name: hal_nofs_block_write
 ****************************************************************************/
static int hal_nofs_block_write(struct bes_nofs_mountpt_s *fs, struct bes_nofs_file_s *fp,
                                size_t buflen, const char *buffer)
{
    struct mtd_geometry_s *geo = &fs->geo;
    struct inode *drv = fs->drv;
    int ret;

    if ((buflen / geo->blocksize * geo->blocksize) != buflen) {
        ASSERT(0, FS_ERR_TAG"buflen not aligned, buflen=%u blocksize=%u",
               __FS_ERR_WHERE__, buflen, geo->blocksize);
    }

    AOS_FS_TRACE(1, FS_DEBUG_TAG"write block=%u count=%u len=%u",
                 __FS_ERR_WHERE__, fp->block, buflen / geo->blocksize, buflen);

    if (fp->block >= (geo->erasesize / geo->blocksize * geo->neraseblocks)) {
        ASSERT(0, FS_ERR_TAG"fp->block=%u invalid", __FS_ERR_WHERE__, fp->block);
    }

    if ((fp->block + (buflen / geo->blocksize) - 1) >= (geo->erasesize / geo->blocksize * geo->neraseblocks)) {
        ASSERT(0, FS_ERR_TAG"fp->block=%u bcnt=%u invalid", __FS_ERR_WHERE__, fp->block, buflen / geo->blocksize);
    }

    if (INODE_IS_MTD(drv)) {
        ret = MTD_BWRITE(drv->u.i_mtd, fp->block, buflen / geo->blocksize, (void *)buffer);
    } else {
        ret = drv->u.i_bops->write(drv, (uint8_t *)buffer, fp->block, buflen / geo->blocksize);
    }

    return ret >= 0 ? buflen : 0;
}

/****************************************************************************
 * Name: hal_nofs_stream_write
 ****************************************************************************/
static int hal_nofs_stream_write(struct bes_nofs_mountpt_s *fs, struct bes_nofs_file_s *fp,
                                 size_t buflen, const char *buffer)
{
    struct mtd_geometry_s *geo = &fs->geo;
    uint32_t start_block = offset_to_start_block(geo, fp->offset);
    uint32_t end_block = offset_to_end_block(geo, fp->offset, buflen);
    uint32_t offset_left_start = fp->offset % geo->blocksize;
    uint32_t offset_left_end = (fp->offset + buflen) % geo->blocksize;
    uint32_t need_block = buflen / geo->blocksize;
    uint32_t tmplen = 0;
    int32_t  ret = -1;
    const char *tmpbuf = NULL;

    if (offset_left_start) {
        if (need_block <= 1 && (offset_left_start + buflen <= geo->blocksize * 2)) {
            //block: 0 or 1 ---> len <= geo->blocksize
            fp->block = start_block;
            tmplen = geo->blocksize * 2;
            ret = hal_nofs_block_read(fs, fp, tmplen, fp->data);
            if (ret <= 0) {
                return ret;
            }
            memcpy(fp->data + offset_left_start, buffer, buflen);
            ret = hal_nofs_block_write(fs, fp, tmplen, fp->data);
            if (ret <= 0) {
                return ret;
            }
            return tmplen;
        } else {
            fp->block = start_block;
            tmplen = geo->blocksize;
            ret = hal_nofs_block_read(fs, fp, tmplen, fp->data);
            if (ret <= 0) {
                return ret;
            }
            memcpy(fp->data + offset_left_start, buffer, geo->blocksize - offset_left_start);
            ret = hal_nofs_block_write(fs, fp, tmplen, fp->data);
            if (ret <= 0) {
                return ret;
            }

            fp->block = start_block + 1;
            tmplen = geo->blocksize * ((end_block - start_block) - 1);
            ret = hal_nofs_block_write(fs, fp, tmplen, buffer + (geo->blocksize - offset_left_start));
            if (ret <= 0) {
                return ret;
            }

            fp->block = end_block;
            tmplen = geo->blocksize;
            ret = hal_nofs_block_read(fs, fp, tmplen, fp->data);
            if (ret <= 0) {
                return ret;
            }
            tmpbuf = (buffer + (geo->blocksize - offset_left_start)) + (((end_block - start_block) - 1) * geo->blocksize);
            memcpy(fp->data, tmpbuf, offset_left_end);
            ret = hal_nofs_block_write(fs, fp, tmplen, fp->data);
            if (ret <= 0) {
                return ret;
            }
            return buflen;
        }
        return -1;
    } else {
        if (buflen % geo->blocksize) { // not align
            if (need_block <= 1) { //less than two block
                fp->block = start_block;
                tmplen = geo->blocksize * 2;
                ret = hal_nofs_block_read(fs, fp, tmplen, fp->data);
                if (ret <= 0) {
                    return ret;
                }
                memcpy(fp->data, buffer, buflen);
                ret = hal_nofs_block_write(fs, fp, tmplen, fp->data);
                if (ret <= 0) {
                    return ret;
                }
                return buflen;
            } else if (need_block > 1) { // more than two block
                fp->block = start_block;
                tmplen = geo->blocksize * (end_block - start_block);
                ret = hal_nofs_block_write(fs, fp, tmplen, buffer);
                if (ret <= 0) {
                    return ret;
                }

                fp->block = end_block;
                tmplen = geo->blocksize;
                ret = hal_nofs_block_read(fs, fp, tmplen, fp->data);
                if (ret <= 0) {
                    return ret;
                }
                memcpy(fp->data, buffer + (end_block - start_block)*geo->blocksize, buflen % geo->blocksize);
                ret = hal_nofs_block_write(fs, fp, tmplen, fp->data);
                if (ret <= 0) {
                    return ret;
                }
                return buflen;
            }
        } else {
            fp->block = start_block;
            tmplen = geo->blocksize * (end_block - start_block);
            ret = hal_nofs_block_write(fs, fp, tmplen, buffer);
            if (ret <= 0) {
                return ret;
            }
            return buflen;
        }
    }
    return 0;
}

/****************************************************************************
 * Name: hal_nofs_write
 ****************************************************************************/
static ssize_t hal_nofs_write(struct file *filep, const char *buffer, size_t buflen)
{
    struct bes_nofs_mountpt_s *fs;
    struct bes_nofs_file_s *fp;
    int ret;

    fp = filep->f_priv;
    fs = filep->f_inode->i_private;

    if (fp->oflags != O_RDWR && fp->oflags != O_WRONLY) {
        AOS_FS_TRACE(0, FS_ERR_TAG"has no acess to write", __FS_ERR_WHERE__);
        return 0;
    }

    fs_mutex_lock(fs->lock);
    if (fp->procflag == BES_NOFS_PROC_BLOCK) {
        ret = hal_nofs_block_write(fs, fp, buflen, buffer);
    } else {
        ret = hal_nofs_stream_write(fs, fp, buflen, buffer);
    }
    fs_mutex_unlock(fs->lock);

    return ret;
}

/****************************************************************************
 * Name: hal_nofs_bind
 ****************************************************************************/

static int hal_nofs_bind(struct inode *driver, const void *data, void **handle)
{
    struct bes_nofs_mountpt_s *fs;
    int ret;

    fs = fs_mm_zalloc(sizeof(*fs));
    if (!fs) {
        ret = -ENOMEM;
        return ret;
    }

    fs->drv = driver;        /* Save the driver reference */
    fs->lock = fs_mutex_create("fs_file_lock"); /* Initialize the access control mutex */

    if (INODE_IS_BLOCK(driver) && driver->u.i_bops->open) {
        ret = driver->u.i_bops->open(driver);
        if (ret < 0) {
            AOS_FS_TRACE(0, FS_ERR_TAG"driver open failed", __FS_ERR_WHERE__);
            goto errout_with_block;
        }
    }

    if (INODE_IS_MTD(driver)) {
        /* Get MTD geometry directly */
        ret = MTD_IOCTL(driver->u.i_mtd, MTDIOC_GEOMETRY,
                        (unsigned long)&fs->geo);
        if (ret < 0) {
            goto errout_with_block;
        }
    } else {
        struct geometry geometry;
        ret = driver->u.i_bops->geometry(driver, &geometry);
        if (ret < 0) {
            AOS_FS_TRACE(0, FS_ERR_TAG"Geometry failed: %d", __FS_ERR_WHERE__, ret);
            goto errout_with_block;
        }
        fs->geo.blocksize    = geometry.geo_sectorsize;
        fs->geo.erasesize    = geometry.geo_sectorsize;
        fs->geo.neraseblocks = geometry.geo_nsectors;
    }

    AOS_FS_TRACE(0, "\n");
    AOS_FS_TRACE(0, "****besnofs driver geometry****");
    AOS_FS_TRACE(0, "    geo.neraseblocks: %u", fs->geo.neraseblocks);
    AOS_FS_TRACE(0, "       geo.blocksize: %u", fs->geo.blocksize);
    AOS_FS_TRACE(0, "       geo.erasesize: %u\n", fs->geo.erasesize);

    *handle = fs;
    return FS_OK;

errout_with_block:
    fs_mutex_destroy(&fs->lock);
    fs_mm_free(fs);
    if (INODE_IS_BLOCK(driver) && driver->u.i_bops->close) {
        driver->u.i_bops->close(driver);
    }
    return ret;
}

/****************************************************************************
 * Name: hal_nofs_unbind
 *
 * Description: This implements the filesystem portion of the umount
 *  operation.
 *
 ****************************************************************************/

static int hal_nofs_unbind(void *handle, struct inode **driver, unsigned int flags)
{
    struct bes_nofs_mountpt_s *fs = handle;
    struct inode *drv = fs->drv;

    if (INODE_IS_BLOCK(drv) && drv->u.i_bops->close) {
        drv->u.i_bops->close(drv);
    }

    /* We hold a reference to the driver but should not but
     * mucking with inodes in this context. So, we will just return
     * our contained reference to the driver inode and let the
     * umount logic dispose of it.
     */
    if (driver) {
        *driver = drv;
    }

    /* Release the mountpoint private data */
    fs_mutex_destroy(&fs->lock);
    fs_mm_free(fs);

    return FS_OK;
}

