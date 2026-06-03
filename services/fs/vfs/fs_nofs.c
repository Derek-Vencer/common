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

#include "fs/fs.h"
#include "fs/fs_nofs.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: hal_aos_nofs_open
 *
 * Description:
 *   hal_aos_nofs_open() is similar to the standard 'open' interface except that it is
 *   not a cancellation point and it does not modify the errno variable.
 *
 * Input Parameters:
 *   path   - The full path to the file to be opened.
 *   oflags - open flags.
 *   ...    - Variable number of arguments, may include 'mode_t mode'
 *
 * Returned Value:
 *   The new file descriptor is returned on success; a negated errno value is
 *   returned on any failure.
 *
 ****************************************************************************/

int hal_aos_nofs_open(const char *path, int oflags, ...)
{
    if (oflags != O_RDONLY && oflags != O_WRONLY && oflags != O_RDWR) {
        AOS_FS_TRACE(0, FS_ERR_TAG"acess error, oflags=%d", __FS_ERR_WHERE__, oflags);
        return -1;
    }
    return bes_aos_open(path, oflags);
}

/****************************************************************************
 * Name: hal_aos_nofs_close
 *
 * Description:
 *   hal_aos_nofs_close() is similar to the standard 'close' interface except that is
 *   not a cancellation point and it does not modify the errno variable.
 *
 *   Close an inode (if open)
 *
 * Returned Value:
 *   Zero (FS_OK) is returned on success; A negated errno value is returned on
 *   on any failure.
 *
 * Assumptions:
 *   Caller holds the list mutex because the file descriptor will be
 *   freed.
 *
 ****************************************************************************/

int hal_aos_nofs_close(int fd)
{
    return bes_aos_close(fd);
}

/****************************************************************************
 * Name: hal_aos_nofs_write
 *
 * Description:
 *  hal_aos_nofs_bwrite() writes up to nytes bytes to the file referenced by the file
 *  descriptor fd from the buffer starting at buf.
 *
 * Input Parameters:
 *   fd       - file descriptor to write to
 *   sectorid - which sector id to write
 *   buf      - Data to write
 *   nbytes   - Length of data to write
 *
 * Returned Value:
 *  On success, the number of bytes written are returned (zero indicates
 *  nothing was written).  On any failure, a negated errno value is returned
 *  (see comments withwrite() for a description of the appropriate errno
 *   values).
 *
 ****************************************************************************/

ssize_t hal_aos_nofs_bwrite(int fd, int sectorid, void *buf, size_t nbytes)
{
    struct file *filep;
    ssize_t ret;
    struct bes_nofs_file_s *fp = NULL;

    ret = (ssize_t)fs_getfilep(fd, &filep);
    if (ret >= 0) {
        fp = (struct bes_nofs_file_s *)filep->f_priv;
    }

    if (fp == NULL) {
        return 0;
    }

    fp->block = sectorid;
    /*block write*/
    fp->procflag = BES_NOFS_PROC_BLOCK;
    return bes_aos_write(fd, buf, nbytes);
}

/****************************************************************************
 * Name: hal_aos_nofs_write
 *
 * Description:
 *  hal_aos_nofs_write() writes up to nytes bytes to the file referenced by the file
 *  descriptor fd from the buffer starting at buf.
 *
 * Input Parameters:
 *   fd       - file descriptor to write to
 *   offset   - where to write, offset byte from the start block
 *   buf      - Data to write
 *   nbytes   - Length of data to write
 *
 * Returned Value:
 *  On success, the number of bytes written are returned (zero indicates
 *  nothing was written).  On any failure, a negated errno value is returned
 *  (see comments withwrite() for a description of the appropriate errno
 *   values).
 *
 ****************************************************************************/

ssize_t hal_aos_nofs_write(int fd, int offset, void *buf, size_t nbytes)
{
    struct file *filep;
    ssize_t ret;
    struct bes_nofs_file_s *fp = NULL;

    ret = (ssize_t)fs_getfilep(fd, &filep);
    if (ret >= 0) {
        fp = (struct bes_nofs_file_s *)filep->f_priv;
    }

    if (fp == NULL) {
        return 0;
    }

    fp->offset = offset;
    fp->procflag = BES_NOFS_PROC_STREAM;
    return bes_aos_write(fd, buf, nbytes);
}

/****************************************************************************
 * Name: hal_aos_nofs_bread
 *
 * Description:
 *   hal_aos_nofs_bread() is an internal OS interface.  It is functionally similar to
 *   the standard read() interface except:
 *
 * Input Parameters:
 *   fd       - File descriptor to read from
 *   sectorid - which sector id to write
 *   buf      - User-provided to save the data
 *   nbytes   - The maximum size of the user-provided buffer
 *
 * Returned Value:
 *   The positive non-zero number of bytes read on success, 0 on if an
 *   end-of-file condition, or a negated errno value on any failure.
 *
 ****************************************************************************/

ssize_t hal_aos_nofs_bread(int fd, int sectorid, void *buf, size_t nbytes)
{
    struct file *filep;
    ssize_t ret;
    struct bes_nofs_file_s *fp = NULL;

    ret = (ssize_t)fs_getfilep(fd, &filep);
    if (ret >= 0) {
        fp = (struct bes_nofs_file_s *)filep->f_priv;
    }

    if (fp == NULL) {
        return 0;
    }

    fp->block = sectorid;
    fp->procflag = BES_NOFS_PROC_BLOCK;
    return bes_aos_read(fd, buf, nbytes);
}

/****************************************************************************
 * Name: hal_aos_nofs_read
 *
 * Description:
 *   hal_aos_nofs_read() is an internal OS interface.  It is functionally similar to
 *   the standard read() interface except:
 *
 * Input Parameters:
 *   fd       - File descriptor to read from
 *   offset   - where to read, offset byte from the start block
 *   buf      - User-provided to save the data
 *   nbytes   - The maximum size of the user-provided buffer
 *
 * Returned Value:
 *   The positive non-zero number of bytes read on success, 0 on if an
 *   end-of-file condition, or a negated errno value on any failure.
 *
 ****************************************************************************/

ssize_t hal_aos_nofs_read(int fd, int offset, void *buf, size_t nbytes)
{
    struct file *filep;
    ssize_t ret;
    struct bes_nofs_file_s *fp = NULL;

    ret = (ssize_t)fs_getfilep(fd, &filep);
    if (ret >= 0) {
        fp = (struct bes_nofs_file_s *)filep->f_priv;
    }

    if (fp == NULL) {
        return 0;
    }

    fp->offset = offset;
    fp->procflag = BES_NOFS_PROC_STREAM;
    return bes_aos_read(fd, buf, nbytes);
}

/****************************************************************************
 * Name: bes_aos_nofs_mount
 *
 * Description:
 *   bes_aos_nofs_mount() is similar to the standard 'mount' interface except that is
 *   not a cancellation point and it does not modify the errno variable.
 *
 * Returned Value:
 *   Zero is returned on success; a negated value is returned on any failure.
 *
 ****************************************************************************/

int hal_aos_nofs_mount(const char *source, const char *target)
{
    return bes_aos_mount(source, target, "besnofs", 0, "autoformat");
}

/****************************************************************************
 * Name: hal_aos_nofs_umount
 *
 * Description:
 *   bes_aos_nofs_umount() is similar to the standard 'umount2' interface except that
 *   is not a cancellation point and it does not modify the errno variable.
 *
 * Returned Value:
 *   Zero is returned on success; a negated value is returned on any failure.
 *
 ****************************************************************************/

int hal_aos_nofs_umount(const char *target)
{
    return bes_aos_umount2(target, 0);
}

