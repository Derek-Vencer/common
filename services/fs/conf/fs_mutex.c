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
#ifdef RTOS
#include "cmsis_os.h"
#else
static int g_mutex = 0;
#endif

void *fs_mutex_create(void *name)
{
#ifdef RTOS
    osMutexAttr_t attr;
    attr.name = (name != NULL) ? name : "unknown";
    attr.attr_bits = osMutexRecursive | osMutexPrioInherit | osMutexRobust;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    return osMutexNew(&attr);
#else
    return &g_mutex;
#endif
}

void fs_mutex_lock(void *mutex)
{
#ifdef RTOS
    if (mutex)
        osMutexAcquire((osMutexId_t)mutex, osWaitForever);
#endif
}

void fs_mutex_unlock(void *mutex)
{
#ifdef RTOS
    if (mutex)
        osMutexRelease((osMutexId_t)mutex);
#endif
}

void fs_mutex_destroy(void **mutex)
{
#ifdef RTOS
    if (mutex && *mutex) {
        osMutexDelete(*mutex);
        *mutex = NULL;
    }
#endif
}

void fs_mutex_delete(void *mutex)
{
#ifdef RTOS
    if (mutex) {
        osMutexDelete(mutex);
    }
#endif
}
