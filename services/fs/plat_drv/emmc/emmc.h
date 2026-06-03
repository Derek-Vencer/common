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
#ifndef __AOS_EMMC_BASE_H__
#define __AOS_EMMC_BASE_H__

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

int bes_emmc_slotinitialize(int id, int minor);
int bes_emmc_slotinitialize_partition(int id, int minor, int part, off_t firstsector, off_t nsectors);
void bes_emmc_drivers_register(void);

#ifdef __cplusplus
}
#endif

#endif /*__AOS_EMMC_BASE_H__*/
