/***************************************************************************
 *
 * Copyright 2022-2023 BES.
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

#ifndef _CAPSENSOR_DRIVER_BEST1503_H_
#define _CAPSENSOR_DRIVER_BEST1503_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "stdint.h"
#include "stdbool.h"

void capsensor_driver_baseline_dr(uint32_t* baseline_value_p, uint32_t* baseline_value_n);

void capsensor_driver_baseline_reg_read(uint32_t* baseline_value_p, uint32_t* baseline_value_n);

void capsensor_suspend(void);

void capsensor_resume(void);

void capsensor_close(void);

void capsensor_open(void);

#ifdef __cplusplus
}
#endif

#endif /* _CAPSENSOR_DRIVER_BEST1503_H_ */
