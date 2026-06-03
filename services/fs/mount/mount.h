/****************************************************************************
 * fs/mount/mount.h
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

#ifndef __FS_MOUNT_MOUNT_H
#define __FS_MOUNT_MOUNT_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include "conf/fs_config.h"
#include "fs/ioctl.h"

#ifndef CONFIG_DISABLE_MOUNTPOINT

/****************************************************************************
 * Public Types
 ****************************************************************************/

#define BLKSSZGET  BIOC_BLKSSZGET
#define BLKGETSIZE BIOC_BLKGETSIZE

/* Mount flags */

#define MS_RDONLY 1 /* Mount file system read-only */

/* Un-mount flags
 *
 * These flags control the behavior of umount2() when there are open file
 * references through the mountpoint.  umount2() with no flag bits set is
 * equivalent to umount(), i.e., the umount() will fail with EBUSY if there
 * are open references on the file.
 *
 *   MNT_FORCE - Force unmount even if busy. This can cause data loss.
 *   MNT_DETACH - Perform a lazy unmount: make the mount point unavailable
 *     for new accesses, and actually perform the unmount when the mount
 *     point ceases to be busy.
 *   MNT_EXPIRE - Mark the mount point as expired. If a mount point is not
 *     currently in use, then an initial call to umount2() with this flag
 *     fails with the error EAGAIN, but marks the mount point as expired. The
 *     mount point remains expired as long as it isn't accessed by any
 *     process. A second umount2() call specifying MNT_EXPIRE unmounts an
 *     expired mount point. This flag cannot be specified with either
 *     MNT_FORCE or MNT_DETACH.
 *  UMOUNT_NOFOLLOW (- Don't dereference target if it is a symbolic link.
 *     For Linux, this flag allows security problems to be avoided in
 *     set-user-ID-root programs that allow unprivileged users to unmount
 *     file systems.  For Aos, it is provided only for compatibility
 *
 * Not all options are supported on all file systems.
 */

#define MNT_FORCE       (1 << 0)
#define MNT_DETACH      (1 << 1)
#define MNT_EXPIRE      (1 << 2)
#define UMOUNT_NOFOLLOW (0)

/* umount() is equivalent to umount2() with flags = 0 */
#define umount(t)       umount2(t,0)

struct statfs; /* Forward reference */

/* Callback used by foreach_mountpoints to traverse all mountpoints in the
 * pseudo-file system.
 */

typedef int (*foreach_mountpoint_t)(FAR const char *mountpoint,
                                    FAR struct statfs *statbuf,
                                    FAR void *arg);

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Name: foreach_mountpoint
 *
 * Description:
 *   Visit each mountpoint in the pseudo-file system.  The traversal is
 *   terminated when the callback 'handler' returns a non-zero value, or when
 *   all of the mountpoints have been visited.
 *
 *   This is just a front end "filter" to foreach_inode() that forwards only
 *   mountpoint inodes.  It is intended to support the mount() command to
 *   when the mount command is used to enumerate mounts.
 *
 *   NOTE 1: Use with caution... The pseudo-file system is locked throughout
 *   the traversal.
 *   NOTE 2: The search algorithm is recursive and could, in principle, use
 *   an indeterminant amount of stack space.  This will not usually be a
 *   real work issue.
 *
 ****************************************************************************/

int foreach_mountpoint(foreach_mountpoint_t handler, FAR void *arg);

/****************************************************************************
 * Name: fs_gettype
 *
 * Description:
 *   Given the result of statfs(), return a string representing the type of
 *   the file system.
 *
 * Input Parameters:
 *   statbuf - The result of a previouis statbuf statfs on the file system.
 *
 * Returned Value:
 *   A reference to a string representing the type of the file system.
 *
 ****************************************************************************/

FAR const char *fs_gettype(FAR struct statfs *statbuf);

#endif /* CONFIG_DISABLE_MOUNTPOINT */
#endif /* __FS_MOUNT_MOUNT_H */
