/***************************************************************************
 *
 * Copyright (c) 2015-2025 BES Technic
 *
 * Authored by BES CD team (Blueelf Prj).
 *
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
 *
 ****************************************************************************/
#ifndef __CO_SEM_H__
#define __CO_SEM_H__

/**
 * TYPEDEFINES
 *
 *
 */
typedef uint32_t *co_sem_id_t;

/**
 * FUNCTIONS DECLARATIONS
 *
 *
 */
#ifdef __cplusplus
extern "C" {
#endif

co_sem_id_t co_sem_new(uint32_t max_count, uint32_t initial_count);
int32_t co_sem_acquire(co_sem_id_t co_sem_id, uint32_t wait_ms);
int32_t co_sem_release(co_sem_id_t co_sem_id);
int32_t co_sem_delete(co_sem_id_t co_sem_id);

#ifdef __cplusplus
}
#endif
#endif /// __CO_SEM_H__