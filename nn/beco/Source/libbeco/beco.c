/***************************************************************************
 *
 * Copyright 2015-2025 BES.
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

#include "hal_cmu.h"

int set_beco_coprocessor(int enable);

int beco_init(void)
{
    uint32_t lock;

    lock = int_lock();
#if defined(CHIP_BEST1306P) || defined(CHIP_BEST1502P) || defined(CHIP_BEST1503) || defined(CHIP_BEST1503P) || defined(CHIP_BEST1307P) \
    || defined(CHIP_BEST2007HS)
    if (get_cpu_id() == 1) {
        hal_cmu_cp_beco_enable();
    } else {
        hal_cmu_beco_enable();
    }
#else
    hal_cmu_beco_enable();
#endif
    set_beco_coprocessor(1);
    int_unlock(lock);

    return 0;
}

int beco_exit(void)
{
    uint32_t lock;

    lock = int_lock();
    set_beco_coprocessor(0);
#if defined(CHIP_BEST1306P) || defined(CHIP_BEST1502P) || defined(CHIP_BEST1503) || defined(CHIP_BEST1503P) || defined(CHIP_BEST1307P)
    if (get_cpu_id() == 1) {
        hal_cmu_cp_beco_disable();
    } else {
        hal_cmu_beco_disable();
    }
#else
    hal_cmu_beco_disable();
#endif
    int_unlock(lock);

    return 0;
}
