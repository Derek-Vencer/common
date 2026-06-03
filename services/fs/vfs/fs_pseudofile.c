/****************************************************************************
 * fs/vfs/fs_pseudofile.c
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

#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <sys/param.h>
#include "conf/fs_config.h"
#include "conf/fs_memory.h"
#include "fs/fserrno.h"
#include "fs/fcntl.h"
#include "fs/fs.h"
#include "fs/ioctl.h"
#include "inode/inode.h"

/****************************************************************************
 * Private Types
 ****************************************************************************/

#if UINTPTR_MAX > UINT32_MAX
#define FLS(n) ((n)&UINT64_C(0xffffffff00000000) ? 32 +                         \
                FLS32((size_t)(n) >> 32) \
                : FLS32(n))
#else
#define FLS(n) FLS32(n)
#endif

#define FLS32(n) ((n)&0xffff0000 ? 16 + FLS16((n) >> 16) : FLS16(n))
#define FLS16(n) ((n)&0xff00 ? 8 + FLS8((n) >> 8) : FLS8(n))
#define FLS8(n)  ((n)&0xf0 ? 4 + FLS4((n) >> 4) : FLS4(n))
#define FLS4(n)  ((n)&0xc ? 2 + FLS2((n) >> 2) : FLS2(n))
#define FLS2(n)  ((n)&0x2 ? 1 + FLS1((n) >> 1) : FLS1(n))
#define FLS1(n)  ((n)&0x1 ? 1 : 0)

/* Returns round up and round down value of log2(n). Note: it can be used at * compile time.*/
#define LOG2_CEIL(n)  ((n) & (n - 1) ? FLS(n) : FLS(n) - 1)
#define LOG2_FLOOR(n) (FLS(n) - 1)

struct fs_pseudofile_s {
    void *lock;
    uint8_t crefs;
#ifndef CONFIG_DISABLE_PSEUDOFS_OPERATIONS
    bool unlinked;
#endif
    FAR char *content;
};

/****************************************************************************
 * Private Functions Prototypes
 ****************************************************************************/

static int     pseudofile_open(FAR struct file *filep);
static int     pseudofile_close(FAR struct file *filep);
static ssize_t pseudofile_write(FAR struct file *filep,
                                FAR const char *buffer, size_t buflen);
static ssize_t pseudofile_read(FAR struct file *filep, FAR char *buffer,
                               size_t buflen);
static off_t   pseudofile_seek(FAR struct file *filep, off_t offset,
                               int whence);
#ifdef CONFIG_FILE_MMAP
static int     pseudofile_mmap(FAR struct file *filep,
                               FAR struct mm_map_entry_s *map);
#endif
static int     pseudofile_truncate(FAR struct file *filep, off_t length);
#ifndef CONFIG_DISABLE_PSEUDOFS_OPERATIONS
static int     pseudofile_unlink(FAR struct inode *inode);
#endif

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const struct file_operations g_pseudofile_ops = {
    pseudofile_open,     /* open */
    pseudofile_close,    /* close */
    pseudofile_read,     /* read */
    pseudofile_write,    /* write */
    pseudofile_seek,     /* seek */
    NULL,                /* ioctl */
#ifdef CONFIG_FILE_MMAP
    pseudofile_mmap,     /* mmap */
#endif
    pseudofile_truncate, /* truncate */
#ifdef CONFIG_FILE_POLL
    NULL,                /* poll */
#endif
#ifndef CONFIG_DISABLE_PSEUDOFS_OPERATIONS
    pseudofile_unlink,   /* unlink */
#endif
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static int pseudofile_open(FAR struct file *filep)
{
    FAR struct inode *node = filep->f_inode;
    FAR struct fs_pseudofile_s *pf = node->i_private;
    int ret;

    fs_mutex_lock(pf->lock);

    if (pf->crefs >= 255) {
        ret = -EMFILE;
    } else {
        pf->crefs += 1;
        ret = FS_OK;
    }

#ifdef CONFIG_PSEUDOFS_ATTRIBUTES
    node->i_atime.tv_sec = time(NULL);
#endif
    fs_mutex_unlock(pf->lock);
    return ret;
}

static void pseudofile_remove(FAR struct fs_pseudofile_s *pf)
{
    fs_mutex_unlock(pf->lock);
    fs_mutex_destroy(&pf->lock);
    fs_mm_free(pf->content);
    fs_mm_free(pf);
}

static int pseudofile_close(FAR struct file *filep)
{
    FAR struct inode *node = filep->f_inode;
    FAR struct fs_pseudofile_s *pf = node->i_private;

    fs_mutex_lock(pf->lock);

    pf->crefs--;
#ifndef CONFIG_DISABLE_PSEUDOFS_OPERATIONS
    if (pf->crefs <= 0 && pf->unlinked)
#else
    if (pf->crefs <= 0)
#endif
    {
        pseudofile_remove(pf);
        return FS_OK;
    }

    fs_mutex_unlock(pf->lock);
    return FS_OK;
}

static int pseudofile_expand(FAR struct inode *node,
                             size_t size)
{
    FAR struct fs_pseudofile_s *pf = node->i_private;
    FAR void *tmp;

    if (pf->content && fs_mm_malloc_size(pf->content) >= size) {
        node->i_size = size;
        return 0;
    }

    tmp = fs_mm_realloc(pf->content, 1 << LOG2_CEIL(size));
    if (tmp == NULL) {
        return -ENOMEM;
    }

    pf->content = tmp;
    node->i_size = size;
    return 0;
}

static ssize_t pseudofile_write(FAR struct file *filep,
                                FAR const char *buffer, size_t buflen)
{
    FAR struct inode *node = filep->f_inode;
    FAR struct fs_pseudofile_s *pf = node->i_private;
    int ret;

    fs_mutex_lock(pf->lock);

    if (filep->f_oflags & O_APPEND) {
        ret = pseudofile_expand(node, node->i_size + buflen);
        if (ret < 0) {
            fs_mutex_unlock(pf->lock);
            return ret;
        }

        filep->f_pos = node->i_size - buflen;
    } else {
        ret = pseudofile_expand(node, filep->f_pos + buflen);
        if (ret < 0) {
            fs_mutex_unlock(pf->lock);
            return ret;
        }
    }

    memcpy(pf->content + filep->f_pos, buffer, buflen);
    filep->f_pos += buflen;
#ifdef CONFIG_PSEUDOFS_ATTRIBUTES
    node->i_mtime.tv_sec = time(NULL);
#endif

    fs_mutex_unlock(pf->lock);
    return buflen;
}

static ssize_t pseudofile_read(FAR struct file *filep, FAR char *buffer,
                               size_t buflen)
{
    FAR struct inode *node = filep->f_inode;
    FAR struct fs_pseudofile_s *pf = node->i_private;


    if (buffer == NULL || node->i_size < filep->f_pos) {
        return -EINVAL;
    }

    fs_mutex_lock(pf->lock);

    buflen = MIN(node->i_size - filep->f_pos, buflen);
    memcpy(buffer, pf->content + filep->f_pos, buflen);
    filep->f_pos += buflen;

#ifdef CONFIG_PSEUDOFS_ATTRIBUTES
    node->i_atime.tv_sec = time(NULL);
#endif
    fs_mutex_unlock(pf->lock);

    return buflen;
}

static off_t pseudofile_seek(FAR struct file *filep, off_t offset,
                             int whence)
{
    FAR struct inode *node = filep->f_inode;
    FAR struct fs_pseudofile_s *pf = node->i_private;


    fs_mutex_lock(pf->lock);

    /* Map the offset according to the whence option */

    switch (whence) {
        case SEEK_SET:
            break;

        case SEEK_CUR:
            offset += filep->f_pos;
            break;

        case SEEK_END:
            offset += node->i_size;
            break;

        default:
            fs_mutex_unlock(pf->lock);
            return -EINVAL;
    }

    if (offset < 0) {
        fs_mutex_unlock(pf->lock);
        return -EINVAL;
    }

    filep->f_pos = offset;
#ifdef CONFIG_PSEUDOFS_ATTRIBUTES
    node->i_atime.tv_sec = time(NULL);
#endif
    fs_mutex_unlock(pf->lock);
    return offset;
}

#ifdef CONFIG_FILE_MMAP
static int pseudofile_mmap(FAR struct file *filep,
                           FAR struct mm_map_entry_s *map)
{
    FAR struct inode *node = filep->f_inode;
    FAR struct fs_pseudofile_s *pf = node->i_private;

    if (map->offset >= 0 && map->offset < node->i_size &&
        map->length != 0 && map->offset + map->length <= node->i_size) {
        map->vaddr = pf->content + map->offset;
        return FS_OK;
    }

    return -EINVAL;
}
#endif

static int pseudofile_truncate(FAR struct file *filep, off_t length)
{
    FAR struct inode *node = filep->f_inode;
    FAR struct fs_pseudofile_s *pf = node->i_private;
    int ret = FS_OK;

    fs_mutex_lock(pf->lock);

    if (length < node->i_size) {
        FAR void *tmp;

        tmp = fs_mm_realloc(pf->content, length);
        if (tmp == NULL) {
            ret = -ENOMEM;
            goto out;
        }

        pf->content = tmp;
        node->i_size = length;
    } else {
        ret = pseudofile_expand(node, length);
        if (ret < 0) {
            goto out;
        }

        memset(pf->content + node->i_size, 0, length - node->i_size);
    }

#ifdef CONFIG_PSEUDOFS_ATTRIBUTES
    node->i_mtime.tv_sec = time(NULL);
#endif

out:
    fs_mutex_unlock(pf->lock);
    return ret;
}

#ifndef CONFIG_DISABLE_PSEUDOFS_OPERATIONS
static int pseudofile_unlink(FAR struct inode *node)
{
    FAR struct fs_pseudofile_s *pf = node->i_private;


    fs_mutex_lock(pf->lock);

    if (pf->crefs <= 0) {
        pseudofile_remove(pf);
        return FS_OK;
    }

    pf->unlinked = true;
    fs_mutex_unlock(pf->lock);
    return FS_OK;
}
#endif

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: pseudofile_create
 *
 * Description:
 *   Create the pseudo-file with specified path and mode, and alloc inode
 *   of this pseudo-file.
 *
 ****************************************************************************/

int pseudofile_create(FAR struct inode **node, FAR const char *path,
                      mode_t mode)
{
    FAR struct fs_pseudofile_s *pf;
    int ret;

    if (node == NULL || path == NULL) {
        return -EINVAL;
    }

    pf = fs_mm_zalloc(sizeof(struct fs_pseudofile_s));
    if (pf == NULL) {
        return -ENOMEM;
    }

    pf->lock = fs_mutex_create("pseudofile_lock");

    ret = inode_lock();
    if (ret < 0) {
        goto lock_err;
    }

    ret = inode_reserve(path, mode, node);
    if (ret < 0) {
        goto reserve_err;
    }

    (*node)->i_crefs = 0;
    (*node)->i_flags = 1;
    (*node)->u.i_ops = &g_pseudofile_ops;
    (*node)->i_private = pf;

    inode_unlock();
    return 0;

reserve_err:
    inode_unlock();
lock_err:
    fs_mutex_destroy(&pf->lock);
    fs_mm_free(pf);
    return ret;
}

/****************************************************************************
 * Name: inode_is_pseudofile
 *
 * Description:
 *    Check inode whether is a pseudo file.
 *
 ****************************************************************************/

bool inode_is_pseudofile(FAR struct inode *inode)
{
    if (inode != NULL && inode->u.i_ops == &g_pseudofile_ops) {
        return true;
    }

    return false;
}
