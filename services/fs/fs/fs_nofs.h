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

#ifndef __INCLUDE_AOS_NOFS_FS_H
#define __INCLUDE_AOS_NOFS_FS_H

#ifdef __cplusplus
extern "C" {
#endif

int hal_aos_nofs_mount(const char *source, const char *target);
int hal_aos_nofs_umount(const char *target);
int hal_aos_nofs_open(const char *path, int oflags, ...);
int hal_aos_nofs_close(int fd);
ssize_t hal_aos_nofs_bwrite(int fd, int sectorid, void *buf, size_t nbytes);
ssize_t hal_aos_nofs_bread(int fd, int sectorid, void *buf, size_t nbytes);
ssize_t hal_aos_nofs_write(int fd, int offset, void *buf, size_t nbytes);
ssize_t hal_aos_nofs_read(int fd, int offset, void *buf, size_t nbytes);

#ifdef __cplusplus
}
#endif

#endif
