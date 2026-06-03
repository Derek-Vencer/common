/****************************************************************************
 * fs/driver/fs_blockproxy.c
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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "conf/fs_config.h"
#include "conf/fs_memory.h"
#include "conf/fs_mutex.h"
#include "driver/driver.h"
#include "plat_drv/drivers.h"
#include "fs/fserrno.h"
#include "fs/fcntl.h"
#include "fs/fs.h"
#include "fs/stat.h"

#if !defined(CONFIG_DISABLE_MOUNTPOINT) && \
    !defined(CONFIG_DISABLE_PSEUDOFS_OPERATIONS)

/****************************************************************************
 * Private Data
 ****************************************************************************/

static uint32_t g_devno;
static void *g_devno_lock = NULL;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

void unique_chardev_lock_init()
{
    if (g_devno_lock == NULL)
        g_devno_lock = fs_mutex_create("unique_chardev_lock");
}

/****************************************************************************
 * Name: unique_chardev
 *
 * Description:
 *   Create a unique temporary device name in the /dev/ directory of the
 *   pseudo-file system.  We cannot use mktemp for this because it will
 *   attempt to open() the file.
 *
 * Input Parameters:
 *   None
 *
 * Returned Value:
 *   The allocated path to the device.  This must be released by the caller
 *   to prevent memory links.  NULL will be returned only the case where
 *   we fail to allocate memory.
 *
 ****************************************************************************/

static FAR char *unique_chardev(void)
{
    struct stat statbuf;
    char devbuf[16];
    uint32_t devno;
    int ret;

    /* Loop until we get a unique device name */

    for (; ;) {
        /* Get the mutex protecting the path number */

        fs_mutex_lock(g_devno_lock);

        /* Get the next device number and release the semaphore */

        devno = ++g_devno;
        fs_mutex_unlock(g_devno_lock);

        /* Construct the full device number */

        devno &= 0xffffff;
        snprintf(devbuf, 16, "/dev/tmpc%06lx", (unsigned long)devno);

        /* Make sure that file name is not in use */

        ret = bes_aos_stat(devbuf, &statbuf, 1);
        if (ret < 0) {
            DEBUGASSERT(ret == -ENOENT);
            return strdup(devbuf);
        }

        /* It is in use, try again */
    }
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: block_proxy
 *
 * Description:
 *   Create a temporary char driver using drivers/bch to mediate character
 *   oriented accessed to the block driver.
 *
 * Input Parameters:
 *   filep  - The caller provided location in which to return the 'struct
 *            file' instance.
 *   blkdev - The path to the block driver
 *   oflags - Character driver open flags
 *
 * Returned Value:
 *   Zero (OK) is returned on success.  On failure, a negated errno value is
 *   returned.
 *
 ****************************************************************************/

int block_proxy(FAR struct file *filep, FAR const char *blkdev, int oflags)
{
    FAR char *chardev;
    bool readonly;
    int ret;

    DEBUGASSERT(blkdev);

    /* Create a unique temporary file name for the character device */

    chardev = unique_chardev();
    if (chardev == NULL) {
        AOS_FS_TRACE(0, FS_ERR_TAG"ERROR: Failed to create temporary device name", __FS_ERR_WHERE__);
        return -ENOMEM;
    }

    /* Should this character driver be read-only? */

    readonly = ((oflags & O_WROK) == 0);

    /* Wrap the block driver with an instance of the BCH driver */

    ret = bchdev_register(blkdev, chardev, readonly);
    if (ret < 0) {
        AOS_FS_TRACE(0, FS_ERR_TAG"ERROR: bchdev_register(%s, %s) failed: %d",
              __FS_ERR_WHERE__, blkdev, chardev, ret);

        goto errout_with_chardev;
    }

    /* Open the newly created character driver */

    oflags &= ~(O_CREAT | O_EXCL | O_APPEND | O_TRUNC);
    ret = file_open(filep, chardev, oflags);
    if (ret < 0) {
        AOS_FS_TRACE(0, FS_ERR_TAG"ERROR: Failed to open %s: %d", __FS_ERR_WHERE__, chardev, ret);
        goto errout_with_bchdev;
    }

    /* Unlink the character device name.  The driver instance will persist,
     * provided that CONFIG_DISABLE_PSEUDOFS_OPERATIONS=y (otherwise, we have
     * a problem here!)
     */

    ret = bes_aos_unlink(chardev);
    if (ret < 0) {
        AOS_FS_TRACE(0, FS_ERR_TAG"ERROR: Failed to unlink %s: %d", __FS_ERR_WHERE__, chardev, ret);
        goto errout_with_chardev;
    }

    /* Free the allocated character driver name. */

    fs_mm_free(chardev);
    return FS_OK;

errout_with_bchdev:
    bes_aos_unlink(chardev);

errout_with_chardev:
    fs_mm_free(chardev);
    return ret;
}

#endif /* !CONFIG_DISABLE_MOUNTPOINT && !CONFIG_DISABLE_PSEUDOFS_OPERATIONS */
