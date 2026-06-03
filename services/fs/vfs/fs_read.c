/****************************************************************************
 * fs/vfs/fs_read.c
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
#include <unistd.h>
#include <assert.h>
#include "conf/fs_config.h"
#include "fs/fserrno.h"
#include "fs/fcntl.h"
#include "inode/inode.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: file_read
 *
 * Description:
 *   file_read() is an internal OS interface.  It is functionally similar to
 *   the standard read() interface except:
 *
 *    - It does not modify the errno variable,
 *    - It is not a cancellation point,
 *    - It accepts a file structure instance instead of file descriptor.
 *
 * Input Parameters:
 *   filep  - File structure instance
 *   buf    - User-provided to save the data
 *   nbytes - The maximum size of the user-provided buffer
 *
 * Returned Value:
 *   The positive non-zero number of bytes read on success, 0 on if an
 *   end-of-file condition, or a negated errno value on any failure.
 *
 ****************************************************************************/

ssize_t file_read(FAR struct file *filep, FAR void *buf, size_t nbytes)
{
    FAR struct inode *inode;
    int ret = -EBADF;

    DEBUGASSERT(filep);
    inode = filep->f_inode;
    /* Is a driver or mountpoint registered? If so, does it support the read
     * method?
     */
    if (inode != NULL && inode->u.i_ops && inode->u.i_ops->read) {
        /* Yes.. then let it perform the read.  NOTE that for the case of the
         * mountpoint, we depend on the read methods being identical in
         * signature and position in the operations vtable.
         */
        ret = (int)inode->u.i_ops->read(filep,
                                        (FAR char *)buf,
                                        (size_t)nbytes);
    }

    /* Return the number of bytes read (or possibly an error code) */
    return ret;
}

/****************************************************************************
 * Name: bes_aos_read
 *
 * Description:
 *   bes_aos_read() is an internal OS interface.  It is functionally similar to
 *   the standard read() interface except:
 *
 *    - It does not modify the errno variable, and
 *    - It is not a cancellation point.
 *
 * Input Parameters:
 *   fd     - File descriptor to read from
 *   buf    - User-provided to save the data
 *   nbytes - The maximum size of the user-provided buffer
 *
 * Returned Value:
 *   The positive non-zero number of bytes read on success, 0 on if an
 *   end-of-file condition, or a negated errno value on any failure.
 *
 ****************************************************************************/

ssize_t bes_aos_read(int fd, FAR void *buf, size_t nbytes)
{
    FAR struct file *filep;
    ssize_t ret;

    if (fd < 3) {
        // only read a character at a time from stdin
        // TODO: Read from trace uart input
        return 1;
    }

    /* First, get the file structure.  Note that on failure,
     * fs_getfilep() will return the errno.
     */

    ret = (ssize_t)fs_getfilep(fd, &filep);
    if (ret < 0) {
        AOS_FS_TRACE(0, FS_ERR_TAG"fd=%d wrong", __FS_ERR_WHERE__, fd);
        return ret;
    }

    /* Then let file_read do all of the work. */

    return file_read(filep, buf, nbytes);
}

/****************************************************************************
 * Name: read
 *
 * Description:
 *   The standard, POSIX read interface.
 *
 * Input Parameters:
 *   fd     - File descriptor to read from
 *   buf    - User-provided to save the data
 *   nbytes - The maximum size of the user-provided buffer
 *
 * Returned Value:
 *   The positive non-zero number of bytes read on success, 0 on if an
 *   end-of-file condition, or -1 on failure with errno set appropriately.
 *
 ****************************************************************************/
#ifdef AOS_FS_POSIX
ssize_t read(int fd, FAR void *buf, size_t nbytes)
{
    ssize_t ret;
    /* Let bes_aos_read() do the real work */
    ret = bes_aos_read(fd, buf, nbytes);
    if (ret < 0) {
        AOS_FS_TRACE(0, FS_ERR_TAG"fd=%d read=%d", __FS_ERR_WHERE__, fd, ret);
        set_errno(-ret);
        ret = FS_ERROR;
        return ret;
    }
    AOS_FS_TRACE(2, FS_DEBUG_TAG"fd=%d read=%d", __FS_DEBUG_WHERE__, fd, ret);
    return ret;
}
#endif

#if (BES_AOS_FS_LIBC_ADAPTER == 1)
FUNC_ALIAS(bes_aos_read, _read, (int fd, void *buff, size_t bytes), ssize_t);
#endif
