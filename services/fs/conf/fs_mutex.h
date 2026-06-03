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
#ifndef __AOS_FS_MUTEX_H__
#define __AOS_FS_MUTEX_H__

#if defined(__cplusplus)
extern "C"
{
#endif

void *fs_mutex_create(void *name);
void fs_mutex_lock(void *mutex);
void fs_mutex_unlock(void *mutex);
void fs_mutex_destroy(void **mutex);
void fs_mutex_delete(void *mutex);

#if defined(__cplusplus)
}
#endif

#endif /*__AOS_FS_MUTEX_H__*/
