/****************************************************************************
 * fs/vfs/fs_lseek.c
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

#include <assert.h>
#include <sys/types.h>
#include <unistd.h>
#include "conf/fs_config.h"
#include "fs/fserrno.h"
#include "inode/inode.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: file_seek
 *
 * Description:
 *   This is the internal implementation of lseek.  See the comments in
 *   lseek() for further information.
 *
 * Input Parameters:
 *   file     File structure instance
 *   offset   Defines the offset to position to
 *   whence   Defines how to use offset
 *
 * Returned Value:
 *   The resulting offset on success.  A negated errno value is returned on
 *   any failure (see lseek comments).
 *
 ****************************************************************************/

off_t file_seek(FAR struct file *filep, off_t offset, int whence)
{
    FAR struct inode *inode;
    off_t ret;

    DEBUGASSERT(filep);
    inode =  filep->f_inode;

    /* Invoke the file seek method if available */

    if (inode && inode->u.i_ops && inode->u.i_ops->seek) {
        ret = inode->u.i_ops->seek(filep, offset, whence);
        if (ret < 0) {
            AOS_FS_TRACE(0, FS_ERR_TAG"error, offset=%ld whence=%d", __FS_ERR_WHERE__, offset, whence);
            return ret;
        }
    } else {
        /* No... Just set the common file position value */

        switch (whence) {
            case SEEK_CUR:
                offset += filep->f_pos;

            /* FALLTHROUGH */

            case SEEK_SET:
                if (offset >= 0) {
                    filep->f_pos = offset; /* Might be beyond the end-of-file */
                    break;
                } else {
                    return -EINVAL;
                }
                break;

            case SEEK_END:
                return -ENOSYS;

            default:
                return -EINVAL;
        }
    }

    return filep->f_pos;
}

/****************************************************************************
 * Name: bes_aos_seek
 *
 * Description:
 *  bes_aos_seek() function repositions the offset of the open file associated
 *  with the file descriptor fd to the argument 'offset' according to the
 *  directive 'whence'.  bes_aos_seek() is an internal OS function. It is
 *  functionally equivalent to lseek() except that:
 *
 *  - It does not modify the errno variable, and
 *  - It is not a cancellation point.
 *
 ****************************************************************************/

off_t bes_aos_seek(int fd, off_t offset, int whence)
{
    FAR struct file *filep;
    off_t ret;

    /* Get the file structure corresponding to the file descriptor. */

    ret = fs_getfilep(fd, &filep);
    if (ret < 0) {
        return ret;
    }

    DEBUGASSERT(filep != NULL);

    /* Then let file_seek do the real work */

    return file_seek(filep, offset, whence);
}

/****************************************************************************
 * Name: lseek
 *
 * Description:
 *   The lseek() function repositions the offset of the open file associated
 *   with the file descriptor fd to the argument 'offset' according to the
 *   directive 'whence' as follows:
 *
 *   SEEK_SET
 *      The offset is set to offset bytes.
 *   SEEK_CUR
 *      The offset is set to its current location plus offset bytes.
 *   SEEK_END
 *      The offset is set to the size of the file plus offset bytes.
 *
 *  The lseek() function allows the file offset to be set beyond the end of
 *  the file (but this does not change the size of the file). If data is
 *  later written at this point, subsequent reads of the data in the gap (a
 *  "hole") return null bytes ('\0') until data is actually written into the
 *  gap.
 *
 * Input Parameters:
 *   fd       File descriptor of device
 *   offset   Defines the offset to position to
 *   whence   Defines how to use offset
 *
 * Returned Value:
 *   The resulting offset on success. -1 on failure withi errno set properly:
 *
 *   EBADF      fd is not an open file descriptor.
 *   EINVAL     whence  is  not one of SEEK_SET, SEEK_CUR, SEEK_END; or the
 *              resulting file offset would be negative, or beyond the end of
 *              a seekable device.
 *   EOVERFLOW  The resulting file offset cannot be represented in an off_t.
 *   ESPIPE     fd is associated with a pipe, socket, or FIFO.
 *
 ****************************************************************************/

#ifdef AOS_FS_POSIX
off_t lseek(int fd, off_t offset, int whence)
{
    off_t newpos;

    /* Let bes_aos_seek do the real work */
    newpos = bes_aos_seek(fd, offset, whence);
    if (newpos < 0) {
        set_errno(-newpos);
        AOS_FS_TRACE(0, FS_ERR_TAG"offset=%ld whence=%d", __FS_ERR_WHERE__, offset, whence);
        return FS_ERROR;
    }
    AOS_FS_TRACE(2, FS_DEBUG_TAG"offset=%ld whence=%d", __FS_DEBUG_WHERE__, offset, whence);
    return newpos;
}
#endif

#if (BES_AOS_FS_LIBC_ADAPTER == 1)
FUNC_ALIAS(bes_aos_seek, _lseek, (int fd, off_t off, int whence), off_t);
#endif
