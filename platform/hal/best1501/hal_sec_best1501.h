/***************************************************************************
 *
 * Copyright 2015-2020 BES.
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
#ifndef __HAL_SEC_BEST1501_H__
#define __HAL_SEC_BEST1501_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "plat_types.h"

void hal_sec_cfg_nonsec_bypass(bool bypass);

void hal_sec_set_sram0(enum HAL_SEC_TYPE_T sec_type);
void hal_sec_set_sram1(enum HAL_SEC_TYPE_T sec_type);
void hal_sec_set_sram2(enum HAL_SEC_TYPE_T sec_type);
void hal_sec_set_sram3(enum HAL_SEC_TYPE_T sec_type);
void hal_sec_set_sram4(enum HAL_SEC_TYPE_T sec_type);
void hal_sec_set_sram5(enum HAL_SEC_TYPE_T sec_type);
void hal_sec_set_sram6(enum HAL_SEC_TYPE_T sec_type);
void hal_sec_set_sram7(enum HAL_SEC_TYPE_T sec_type);
void hal_sec_set_sram8(enum HAL_SEC_TYPE_T sec_type);

#ifdef __cplusplus
}
#endif

#endif

