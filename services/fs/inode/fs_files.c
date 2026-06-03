/****************************************************************************
 * fs/inode/fs_files.c
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
#include <assert.h>
#include "conf/fs_config.h"
#include "conf/fs_memory.h"
#include "conf/fs_mutex.h"
#include "fs/fserrno.h"
#include "fs/fcntl.h"
#include "fs/fs.h"
#include "inode/inode.h"

static struct filelist g_list;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: files_extend
 ****************************************************************************/

static int files_extend(FAR struct filelist *list, size_t row)
{
    FAR struct file **tmp;
    int i;

    if (row <= list->fl_rows) {
        return 0;
    }

    if (row * CONFIG_NFILE_DESCRIPTORS_PER_BLOCK > OPEN_MAX) {
        return -EMFILE;
    }

    tmp = fs_mm_realloc(list->fl_files, sizeof(FAR struct file *) * row);
    DEBUGASSERT(tmp);
    if (tmp == NULL) {
        return -ENFILE;
    }

    i = list->fl_rows;
    do {
        tmp[i] = fs_mm_zalloc(sizeof(struct file) *
                              CONFIG_NFILE_DESCRIPTORS_PER_BLOCK);
        if (tmp[i] == NULL) {
            while (--i >= list->fl_rows) {
                fs_mm_free(tmp[i]);
            }

            fs_mm_free(tmp);
            return -ENFILE;
        }
    } while (++i < row);

    list->fl_files = tmp;
    list->fl_rows = row;

    /* Note: If assertion occurs, the fl_rows has a overflow.
     * And there may be file descriptors leak in system.
     */

    DEBUGASSERT(list->fl_rows == row);
    return 0;
}

static void task_fssync(FAR struct filelist *list, FAR void *arg)
{
    int i;
    int j;

    fs_mutex_lock(list->fl_lock);

    for (i = 0; i < list->fl_rows; i++) {
        for (j = 0; j < CONFIG_NFILE_DESCRIPTORS_PER_BLOCK; j++) {
            FAR struct file *filep;

            filep = &list->fl_files[i][j];
            if (filep != NULL && filep->f_inode != NULL) {
                file_fsync(filep);
            }
        }
    }

    fs_mutex_unlock(list->fl_lock);
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: files_getlist
 ****************************************************************************/

void *files_getlist()
{
    return &g_list;
}


/****************************************************************************
 * Name: files_initlist
 *
 * Description: Initializes the list of files for a new task
 *
 ****************************************************************************/

void files_initlist(FAR struct filelist *list)
{
    DEBUGASSERT(list);

    int res;

    if (list->inited)
        return;

    list->fl_rows = 0;

    list->fl_lock = fs_mutex_create("fs_list_lock");
    if (list->fl_lock == NULL)
        return;

    res = files_extend(list, 1);
    if (res != 0) {
        fs_mutex_destroy(&list->fl_lock);
        AOS_FS_TRACE(0, FS_ERR_TAG"files_extend error, ret: %d", __FS_ERR_WHERE__, res);
        return;
    }

    list->inited = 1;
    return;
}

/****************************************************************************
 * Name: files_releaselist
 *
 * Description:
 *   Release a reference to the file list
 *
 ****************************************************************************/

void files_releaselist(FAR struct filelist *list)
{
    int i;
    int j;

    DEBUGASSERT(list);

    /* Close each file descriptor .. Normally, you would need take the list
     * mutex, but it is safe to ignore the mutex in this context
     * because there should not be any references in this context.
     */

    for (i = list->fl_rows - 1; i >= 0; i--) {
        for (j = CONFIG_NFILE_DESCRIPTORS_PER_BLOCK - 1; j >= 0; j--) {
            file_close(&list->fl_files[i][j]);
        }

        fs_mm_free(list->fl_files[i]);
    }

    fs_mm_free(list->fl_files);

    /* Destroy the mutex */

    fs_mutex_destroy(&list->fl_lock);
}

/****************************************************************************
 * Name: file_allocate_from_tcb
 *
 * Description:
 *   Allocate a struct files instance and associate it with an inode
 *   instance.
 *
 * Returned Value:
 *     Returns the file descriptor == index into the files array on success;
 *     a negated errno value is returned on any failure.
 *
 ****************************************************************************/

int file_allocate_from_tcb(FAR struct filelist *list, FAR struct inode *inode,
                           int oflags, off_t pos, FAR void *priv, int minfd,
                           bool addref)
{
    int ret;
    int i;
    int j;

    /* Get the file descriptor list.  It should not be NULL in this context. */
    DEBUGASSERT(list != NULL);

    fs_mutex_lock(list->fl_lock);

    /* Calculate minfd whether is in list->fl_files.
     * if not, allocate a new filechunk.
     */

    i = minfd / CONFIG_NFILE_DESCRIPTORS_PER_BLOCK;
    if (i >= list->fl_rows) {
        ret = files_extend(list, i + 1);
        if (ret < 0) {
            fs_mutex_unlock(list->fl_lock);
            return ret;
        }
    }

    /* Find free file */

    j = minfd % CONFIG_NFILE_DESCRIPTORS_PER_BLOCK;
    do {
        do {
            if (!list->fl_files[i][j].f_inode && !(i == 0 && j <= 2)) {
                list->fl_files[i][j].f_oflags = oflags;
                list->fl_files[i][j].f_pos    = pos;
                list->fl_files[i][j].f_inode  = inode;
                list->fl_files[i][j].f_priv   = priv;
                fs_mutex_unlock(list->fl_lock);

                if (addref) {
                    inode_addref(inode);
                }
                return i * CONFIG_NFILE_DESCRIPTORS_PER_BLOCK + j;
            }
        } while (++j < CONFIG_NFILE_DESCRIPTORS_PER_BLOCK);

        j = 0;
    } while (++i < list->fl_rows);

    /* The space of file array isn't enough, allocate a new filechunk */

    ret = files_extend(list, i + 1);
    if (ret < 0) {
        fs_mutex_unlock(list->fl_lock);
        return ret;
    }

    list->fl_files[i][0].f_oflags = oflags;
    list->fl_files[i][0].f_pos    = pos;
    list->fl_files[i][0].f_inode  = inode;
    list->fl_files[i][0].f_priv   = priv;
    fs_mutex_unlock(list->fl_lock);

    if (addref) {
        inode_addref(inode);
    }
    return i * CONFIG_NFILE_DESCRIPTORS_PER_BLOCK;
}

/****************************************************************************
 * Name: file_allocate
 *
 * Description:
 *   Allocate a struct files instance and associate it with an inode
 *   instance.
 *
 * Returned Value:
 *     Returns the file descriptor == index into the files array on success;
 *     a negated errno value is returned on any failure.
 *
 ****************************************************************************/

int file_allocate(FAR struct inode *inode, int oflags, off_t pos,
                  FAR void *priv, int minfd, bool addref)
{
    return file_allocate_from_tcb(files_getlist(), inode, oflags,
                                  pos, priv, minfd, addref);
}

/****************************************************************************
 * Name: files_duplist
 *
 * Description:
 *   Duplicate parent task's file descriptors.
 *
 ****************************************************************************/

int files_duplist(FAR struct filelist *plist, FAR struct filelist *clist)
{
    int ret = 0;
    int i;
    int j;

    DEBUGASSERT(clist);
    DEBUGASSERT(plist);

    fs_mutex_lock(plist->fl_lock);

    for (i = 0; i < plist->fl_rows; i++) {
        for (j = 0; j < CONFIG_NFILE_DESCRIPTORS_PER_BLOCK; j++) {
            FAR struct file *filep;
#ifdef CONFIG_FDCLONE_STDIO

            /* Determine how many file descriptors to clone.  If
             * CONFIG_FDCLONE_DISABLE is set, no file descriptors will be
             * cloned.  If CONFIG_FDCLONE_STDIO is set, only the first
             * three descriptors (stdin, stdout, and stderr) will be
             * cloned.  Otherwise all file descriptors will be cloned.
             */

            if (i * CONFIG_NFILE_DESCRIPTORS_PER_BLOCK + j >= 3) {
                goto out;
            }
#endif

            filep = &plist->fl_files[i][j];
            DEBUGASSERT(filep);

            if (filep && (filep->f_inode == NULL ||
                          (filep->f_oflags & O_CLOEXEC) != 0)) {
                continue;
            }

            ret = files_extend(clist, i + 1);
            if (ret < 0) {
                goto out;
            }

            /* Yes... duplicate it for the child */

            ret = file_dup2(filep, &clist->fl_files[i][j]);
            if (ret < 0) {
                goto out;
            }
        }
    }

out:
    fs_mutex_unlock(plist->fl_lock);
    return ret;
}

/****************************************************************************
 * Name: fs_getfilep
 *
 * Description:
 *   Given a file descriptor, return the corresponding instance of struct
 *   file.
 *
 * Input Parameters:
 *   fd    - The file descriptor
 *   filep - The location to return the struct file instance
 *
 * Returned Value:
 *   Zero (FS_OK) is returned on success; a negated errno value is returned on
 *   any failure.
 *
 ****************************************************************************/

int fs_getfilep(int fd, FAR struct file **filep)
{
    FAR struct filelist *list;
    int ret = FS_OK;

    DEBUGASSERT(filep != NULL);
    *filep = NULL;

    list = files_getlist();

    /* The file list can be NULL under two cases:  (1) One is an obscure
     * cornercase:  When memory management debug output is enabled.  Then
     * there may be attempts to write to stdout from malloc before the group
     * data has been allocated.  The other other is (2) if this is a kernel
     * thread.  Kernel threads have no allocated file descriptors.
     */

    if (list == NULL) {
        AOS_FS_TRACE(0, FS_ERR_TAG"list null err", __FS_ERR_WHERE__);
        return -EAGAIN;
    }

    if (fd < 0 || fd >= list->fl_rows * CONFIG_NFILE_DESCRIPTORS_PER_BLOCK) {
        AOS_FS_TRACE(0, FS_ERR_TAG"bad fd, fd: %d", __FS_ERR_WHERE__, fd);
        return -EBADF;
    }

    /* The descriptor is in a valid range to file descriptor... Get the
     * thread-specific file list.
     */

    /* And return the file pointer from the list */

    fs_mutex_lock(list->fl_lock);

    *filep = &list->fl_files[fd / CONFIG_NFILE_DESCRIPTORS_PER_BLOCK]
             [fd % CONFIG_NFILE_DESCRIPTORS_PER_BLOCK];

    /* if f_inode is NULL, fd was closed */

    if (!(*filep)->f_inode) {
        *filep = NULL;
        AOS_FS_TRACE(0, FS_ERR_TAG"f_inode err", __FS_ERR_WHERE__);
        ret = -EBADF;
    }

    fs_mutex_unlock(list->fl_lock);
    return ret;
}

/****************************************************************************
 * Name: bes_aos_dup2_from_tcb
 *
 * Description:
 *   bes_aos_dup2_from_tcb() is similar to the standard 'dup2' interface
 *   except that is not a cancellation point and it does not modify the
 *   errno variable.
 *
 *   bes_aos_dup2_from_tcb() is an internal Aos interface and should not be
 *   called from applications.
 *
 *   Clone a file descriptor to a specific descriptor number.
 *
 * Returned Value:
 *   fd2 is returned on success; a negated errno value is return on
 *   any failure.
 *
 ****************************************************************************/

int bes_aos_dup2_from_tcb(FAR struct filelist *list, int fd1, int fd2)
{
    FAR struct file *filep;
    FAR struct file  file;
    int ret;

    if (fd1 == fd2) {
        return fd1;
    }

    /* Get the file descriptor list.  It should not be NULL in this context. */

    if (fd1 < 0 || fd1 >= CONFIG_NFILE_DESCRIPTORS_PER_BLOCK * list->fl_rows ||
        fd2 < 0) {
        return -EBADF;
    }

    fs_mutex_lock(list->fl_lock);

    if (fd2 >= CONFIG_NFILE_DESCRIPTORS_PER_BLOCK * list->fl_rows) {
        ret = files_extend(list, fd2 / CONFIG_NFILE_DESCRIPTORS_PER_BLOCK + 1);
        if (ret < 0) {
            fs_mutex_unlock(&list->fl_lock);
            return ret;
        }
    }

    filep = &list->fl_files[fd2 / CONFIG_NFILE_DESCRIPTORS_PER_BLOCK]
            [fd2 % CONFIG_NFILE_DESCRIPTORS_PER_BLOCK];
    memcpy(&file, filep, sizeof(struct file));
    memset(filep, 0,     sizeof(struct file));

    /* Perform the dup2 operation */

    ret = file_dup2(&list->fl_files[fd1 / CONFIG_NFILE_DESCRIPTORS_PER_BLOCK]
                    [fd1 % CONFIG_NFILE_DESCRIPTORS_PER_BLOCK],
                    filep);


    fs_mutex_unlock(list->fl_lock);

    file_close(&file);

    return ret < 0 ? ret : fd2;
}

/****************************************************************************
 * Name: bes_aos_dup2
 *
 * Description:
 *   bes_aos_dup2() is similar to the standard 'dup2' interface except that is
 *   not a cancellation point and it does not modify the errno variable.
 *
 *   bes_aos_dup2() is an internal Aos interface and should not be called from
 *   applications.
 *
 *   Clone a file descriptor to a specific descriptor number.
 *
 * Returned Value:
 *   fd2 is returned on success; a negated errno value is return on
 *   any failure.
 *
 ****************************************************************************/

int bes_aos_dup2(int fd1, int fd2)
{
    return bes_aos_dup2_from_tcb(files_getlist(), fd1, fd2);
}

/****************************************************************************
 * Name: dup2
 *
 * Description:
 *   Clone a file descriptor or socket descriptor to a specific descriptor
 *   number
 *
 ****************************************************************************/

int dup2(int fd1, int fd2)
{
    int ret;
    ret = bes_aos_dup2(fd1, fd2);
    return ret;
}

/****************************************************************************
 * Name: bes_aos_close_from_tcb
 *
 * Description:
 *   bes_aos_close_from_tcb() is similar to the standard 'close' interface
 *   except that is not a cancellation point and it does not modify the
 *   errno variable.
 *
 *   bes_aos_close_from_tcb() is an internal Aos interface and should not
 *   be called from applications.
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

int bes_aos_close_from_tcb(FAR struct filelist *list, int fd)
{
    FAR struct file     *filep;
    FAR struct file      file;

    /* Perform the protected close operation */
    fs_mutex_lock(list->fl_lock);

    /* If the file was properly opened, there should be an inode assigned */

    if (fd < 0 || fd >= list->fl_rows * CONFIG_NFILE_DESCRIPTORS_PER_BLOCK ||
        !list->fl_files[fd / CONFIG_NFILE_DESCRIPTORS_PER_BLOCK]
        [fd % CONFIG_NFILE_DESCRIPTORS_PER_BLOCK].f_inode) {
        fs_mutex_unlock(list->fl_lock);
        return -EBADF;
    }

    filep = &list->fl_files[fd / CONFIG_NFILE_DESCRIPTORS_PER_BLOCK]
            [fd % CONFIG_NFILE_DESCRIPTORS_PER_BLOCK];
    memcpy(&file, filep, sizeof(struct file));
    memset(filep, 0,     sizeof(struct file));

    fs_mutex_unlock(list->fl_lock);
    return file_close(&file);
}

/****************************************************************************
 * Name: bes_aos_close
 *
 * Description:
 *   bes_aos_close() is similar to the standard 'close' interface except that is
 *   not a cancellation point and it does not modify the errno variable.
 *
 *   bes_aos_close() is an internal Aos interface and should not be called from
 *   applications.
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

int bes_aos_close(int fd)
{
    return bes_aos_close_from_tcb(files_getlist(), fd);
}

/****************************************************************************
 * Name: close
 *
 * Description:
 *   close() closes a file descriptor, so that it no longer refers to any
 *   file and may be reused. Any record locks (see fcntl(2)) held on the file
 *   it was associated with, and owned by the process, are removed
 *   (regardless of the file descriptor that was used to obtain the lock).
 *
 *   If fd is the last copy of a particular file descriptor the resources
 *   associated with it are freed; if the descriptor was the last reference
 *   to a file which has been removed using unlink(2) the file is deleted.
 *
 * Input Parameters:
 *   fd   file descriptor to close
 *
 * Returned Value:
 *   0 on success; -1 on error with errno set appropriately.
 *
 * Assumptions:
 *
 ****************************************************************************/
#ifdef AOS_FS_POSIX
int close(int fd)
{
    int ret;

    /* close() is a cancellation point */
    ret = bes_aos_close(fd);
    return ret;
}
#endif

#if (BES_AOS_FS_LIBC_ADAPTER == 1)
FUNC_ALIAS(bes_aos_close, _close, (int fd), int);
#endif

/****************************************************************************
 * Name: bes_aos_sync
 *
 * Description:
 *   sync() causes all pending modifications to filesystem metadata and
 *   cached file data to be written to the underlying filesystems.
 *
 ****************************************************************************/
void bes_aos_sync(void)
{
    task_fssync(files_getlist(), NULL);
}


/****************************************************************************
 * Name: sync
 *
 * Description:
 *   sync() causes all pending modifications to filesystem metadata and
 *   cached file data to be written to the underlying filesystems.
 *
 ****************************************************************************/
#ifdef AOS_FS_POSIX
void sync(void)
{
    task_fssync(files_getlist(), NULL);
}
#endif

