/****************************************************************************
 * fs/vfs/fs_open.c
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <sys/types.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdarg.h>
#include "conf/fs_config.h"
#include "driver/driver.h"
#include "fs/fserrno.h"
#include "fs/stat.h"
#include "fs/fs.h"
#include "fs/fcntl.h"
#include "inode/inode.h"

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: file_vopen
 ****************************************************************************/

/****************************************************************************
 * Name: file_vopen
 *
 * Description:
 *   file_vopen() is similar to the standard 'open' interface except that it
 *   populates an instance of 'struct file' rather than return a file
 *   descriptor.  It also is not a cancellation point and does not modify
 *   the errno variable.
 *
 * Input Parameters:
 *   filep  - The caller provided location in which to return the 'struct
 *            file' instance.
 *   path   - The full path to the file to be opened.
 *   oflags - open flags.
 *   umask  - File mode creation mask. Overrides the open flags.
 *   ap     - Variable argument list, may include 'mode_t mode'
 *
 * Returned Value:
 *   Zero (FS_OK) is returned on success.  On failure, a negated errno value is
 *   returned.
 *
 ****************************************************************************/
static inline int openmode_to_posix(int openmode)
{
    int posix = openmode;
#ifdef __ARMCC_VERSION
    if (openmode & OPEN_PLUS) {
        posix = O_RDWR;
    } else if (openmode & OPEN_W) {
        posix = O_WRONLY;
    } else if (openmode & OPEN_A) {
        posix = O_WRONLY | O_APPEND;
    } else {
        posix = O_RDONLY;
    }
    /* a, w, a+, w+ all create if file does not already exist */
    if (openmode & (OPEN_A | OPEN_W)) {
        posix |= O_CREAT;
    }
    /* w and w+ truncate */
    if (openmode & OPEN_W) {
        posix |= O_TRUNC;
    }
#elif defined(__ICCARM__)
    switch (openmode & _LLIO_RDWRMASK) {
        case _LLIO_RDONLY:
            posix = O_RDONLY;
            break;
        case _LLIO_WRONLY:
            posix = O_WRONLY;
            break;
        case _LLIO_RDWR  :
            posix = O_RDWR  ;
            break;
    }
    if (openmode & _LLIO_CREAT)
        posix |= O_CREAT;
    if (openmode & _LLIO_APPEND)
        posix |= O_APPEND;
    if (openmode & _LLIO_TRUNC)
        posix |= O_TRUNC;
#endif
    return posix;
}

static int file_vopen(FAR struct file *filep, FAR const char *path,
                      int oflags, mode_t umask, va_list ap)
{
    struct inode_search_s desc;
    FAR struct inode *inode;
    mode_t mode = 0666;
    int ret;

    if (path == NULL) {
        return -EINVAL;
    }

#ifndef CONFIG_DISABLE_MOUNTPOINT

    /* If the file is opened for creation, then get the mode bits */

    if ((oflags & (O_WRONLY | O_CREAT)) != 0) {
        mode = va_arg(ap, mode_t);
    }

    mode &= ~umask;
#endif

    /* Get an inode for this file */

    SETUP_SEARCH(&desc, path, (oflags & O_NOFOLLOW) != 0);

    ret = inode_find(&desc);
    if (ret < 0) {
#ifdef CONFIG_PSEUDOFS_FILE
        if ((oflags & O_CREAT) != 0) {
            ret = pseudofile_create(&desc.node, path, mode);
        }
#endif

        if (ret < 0) {
            /* "O_CREAT is not set and the named file does not exist.  Or, a
             * directory component in pathname does not exist or is a dangling
             * symbolic link."
             */
            AOS_FS_TRACE(0, FS_ERR_TAG"inode_find %s error", __FS_ERR_WHERE__, path);
            goto errout_with_search;
        }
    }

    /* Get the search results */

    inode = desc.node;
    DEBUGASSERT(inode != NULL);

    if (desc.nofollow && INODE_IS_SOFTLINK(inode)) {
        return -ELOOP;
    }

#if defined(CONFIG_BCH) && \
    !defined(CONFIG_DISABLE_MOUNTPOINT) && \
    !defined(CONFIG_DISABLE_PSEUDOFS_OPERATIONS)
    /* If the inode is block driver, then we may return a character driver
     * proxy for the block driver.  block_proxy() will instantiate a BCH
     * character driver wrapper around the block driver, open(), then
     * unlink() the character driver.
     *
     * NOTE: This will recurse to open the character driver proxy.
     */

    if (INODE_IS_BLOCK(inode) || INODE_IS_MTD(inode)) {
        /* Release the inode reference */

        inode_release(inode);
        RELEASE_SEARCH(&desc);

        /* Get the file structure of the opened character driver proxy */

        return block_proxy(filep, path, oflags);
    }
#endif

    /* Make sure that the inode supports the requested access */

    ret = inode_checkflags(inode, oflags);
    if (ret < 0) {
        goto errout_with_inode;
    }

    /* Associate the inode with a file structure */

    memset(filep, 0, sizeof(*filep));
    filep->f_oflags = oflags;
    filep->f_inode  = inode;

    /* Perform the driver open operation.  NOTE that the open method may be
     * called many times.  The driver/mountpoint logic should handle this
     * because it may also be closed that many times.
     */

    if (oflags & O_DIRECTORY) {
        ret = dir_allocate(filep, desc.relpath);
    }
#ifndef CONFIG_DISABLE_MOUNTPOINT
    else if (INODE_IS_MOUNTPT(inode)) {
        if (inode->u.i_mops->open != NULL) {
            ret = inode->u.i_mops->open(filep, desc.relpath, oflags, mode);
        }
    }
#endif
    else if (INODE_IS_DRIVER(inode) || INODE_IS_PIPE(inode)) {
        if (inode->u.i_ops->open != NULL) {
            ret = inode->u.i_ops->open(filep);
        }
    } else {
        ret = -ENXIO;
    }

    if (ret == -EISDIR) {
        ret = dir_allocate(filep, desc.relpath);
    }

    if (ret < 0) {
        goto errout_with_inode;
    }

    RELEASE_SEARCH(&desc);
    return FS_OK;

errout_with_inode:
    filep->f_inode = NULL;
    inode_release(inode);

errout_with_search:
    RELEASE_SEARCH(&desc);
    return ret;
}

/****************************************************************************
 * Name: bes_aos_vopen
 *
 * Description:
 *   bes_aos_vopen() is similar to the standard 'open' interface except that it
 *   is not a cancellation point and it does not modify the errno variable.
 *
 *   bes_aos_open() is an internal Aos interface and should not be called from
 *   applications.
 *
 * Input Parameters:
 *   tcb    - Address of the task's TCB
 *   path   - The full path to the file to be opened.
 *   oflags - open flags.
 *   ap     - Variable argument list, may include 'mode_t mode'
 *
 * Returned Value:
 *   The new file descriptor is returned on success; a negated errno value is
 *   returned on any failure.
 *
 ****************************************************************************/

static mode_t fs_getumask(void)
{
    mode_t mask = 0;//umask( 0 );
    //umask(mask);
    return mask;
}

static int bes_aos_vopen(FAR struct filelist *list,
                    FAR const char *path, int oflags, va_list ap)
{
    struct file filep;
    int ret;
    int fd;

    /* Let file_vopen() do all of the work */

    ret = file_vopen(&filep, path, oflags, fs_getumask(), ap);
    if (ret < 0) {
        AOS_FS_TRACE(0, FS_ERR_TAG"file_vopen error, ret: %d", __FS_ERR_WHERE__, ret);
        return ret;
    }

    /* Allocate a new file descriptor for the inode */

    fd = file_allocate_from_tcb(list, filep.f_inode, filep.f_oflags,
                                filep.f_pos, filep.f_priv, 0, false);
    if (fd < 0) {
        file_close(&filep);
        AOS_FS_TRACE(0, FS_ERR_TAG"file_allocate_from_tcb error, fd: %d", __FS_ERR_WHERE__, fd);
        return fd;
    }

    return fd;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: inode_checkflags
 *
 * Description:
 *   Check if the access described by 'oflags' is supported on 'inode'
 *
 *   inode_checkflags() is an internal Aos interface and should not be
 *   called from applications.
 *
 * Input Parameters:
 *   inode  - The inode to check
 *   oflags - open flags.
 *
 * Returned Value:
 *   Zero (FS_OK) is returned on success.  On failure, a negated errno value is
 *   returned.
 *
 ****************************************************************************/

int inode_checkflags(FAR struct inode *inode, int oflags)
{
    FAR const struct file_operations *ops = inode->u.i_ops;

    if (INODE_IS_PSEUDODIR(inode)) {
        return FS_OK;
    }

    if (ops == NULL) {
        return -ENXIO;
    }

    if (((oflags & O_RDOK) != 0 && !ops->read && !ops->ioctl) ||
        ((oflags & O_WROK) != 0 && !ops->write && !ops->ioctl)) {
        return -EACCES;
    } else {
        return FS_OK;
    }
}

/****************************************************************************
 * Name: file_open
 *
 * Description:
 *   file_open() is similar to the standard 'open' interface except that it
 *   populates an instance of 'struct file' rather than return a file
 *   descriptor.  It also is not a cancellation point and does not modify
 *   the errno variable.
 *
 * Input Parameters:
 *   filep  - The caller provided location in which to return the 'struct
 *            file' instance.
 *   path   - The full path to the file to be opened.
 *   oflags - open flags.
 *   ...    - Variable number of arguments, may include 'mode_t mode'
 *
 * Returned Value:
 *   Zero (FS_OK) is returned on success.  On failure, a negated errno value is
 *   returned.
 *
 ****************************************************************************/

int file_open(FAR struct file *filep, FAR const char *path, int oflags, ...)
{
    va_list ap;
    int ret;

    va_start(ap, oflags);
    ret = file_vopen(filep, path, oflags, 0, ap);
    va_end(ap);

    return ret;
}

/****************************************************************************
 * Name: bes_aos_open_from_tcb
 *
 * Description:
 *   bes_aos_open_from_tcb() is similar to the standard 'open' interface except
 *   that it is not a cancellation point and it does not modify the errno
 *   variable.
 *
 *   bes_aos_open_from_tcb() is an internal Aos interface and should not be
 *   called from applications.
 *
 * Input Parameters:
 *   tcb    - Address of the task's TCB
 *   path   - The full path to the file to be opened.
 *   oflags - open flags.
 *   ...    - Variable number of arguments, may include 'mode_t mode'
 *
 * Returned Value:
 *   The new file descriptor is returned on success; a negated errno value is
 *   returned on any failure.
 *
 ****************************************************************************/

int bes_aos_open_from_tcb(FAR struct filelist *list,
                     FAR const char *path, int oflags, ...)
{
    va_list ap;
    int fd;

    /* Let bes_aos_vopen() do all of the work */

    va_start(ap, oflags);
    fd = bes_aos_vopen(list, path, oflags, ap);
    va_end(ap);

    return fd;
}

/****************************************************************************
 * Name: bes_aos_open
 *
 * Description:
 *   bes_aos_open() is similar to the standard 'open' interface except that it is
 *   not a cancellation point and it does not modify the errno variable.
 *
 *   bes_aos_open() is an internal Aos interface and should not be called from
 *   applications.
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

int bes_aos_open(FAR const char *path, int oflags, ...)
{
    va_list ap;
    int fd;

    /* Let bes_aos_vopen() do all of the work */

    va_start(ap, oflags);
    fd = bes_aos_vopen(files_getlist(), path, oflags, ap);
    va_end(ap);

    return fd;
}

/****************************************************************************
 * Name: open
 *
 * Description:
 *   Standard 'open' interface
 *
 * Returned Value:
 *   The new file descriptor is returned on success; -1 (FS_ERROR) is returned
 *   on any failure with the errno value set appropriately.
 *
 ****************************************************************************/

#ifdef AOS_FS_POSIX
int open(FAR const char *path, int oflags, ...)
{
    va_list ap;
    int fd;
    int openmode;

    openmode = openmode_to_posix(oflags);

    /* Let bes_aos_vopen() do most of the work */
    va_start(ap, oflags);
    fd = bes_aos_vopen(files_getlist(), path, openmode, ap);
    va_end(ap);

    /* Set the errno value if any errors were reported by bes_aos_open() */
    if (fd < 0) {
        set_errno(-fd);
        fd = FS_ERROR;
    }
    return fd;
}
#endif

#if (BES_AOS_FS_LIBC_ADAPTER == 1)
FUNC_ALIAS(bes_aos_open, _open, (const char *path, int flags, ...), int);
#endif
