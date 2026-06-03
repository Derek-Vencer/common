/***************************************************************************
 *
 * Copyright 2015-2023 BES.
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
#ifndef __HAL_SEC_BEST1502P_H__
#define __HAL_SEC_BEST1502P_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "plat_types.h"

enum HAL_SEC_MPC_C1_SEL_T {
    HAL_SEC_MPC_SEL_C1CC,
    HAL_SEC_MPC_SEL_C1DC,
};

void hal_sec_cfg_nonsec_bypass(bool bypass);

void hal_sec_mpc_c1_sel(enum HAL_SEC_MPC_C1_SEL_T sel);

#ifdef __cplusplus
}
#endif

#endif

