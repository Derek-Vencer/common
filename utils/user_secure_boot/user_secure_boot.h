/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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
#ifndef USER_SECURE_BOOT_H
#define USER_SECURE_BOOT_H

#ifdef __cplusplus
extern "C" {
#endif

void user_secure_boot_init(void);

void user_secure_checker(void);

void security_boot_jump_to_next_entry(void);

int user_secure_boot_ota_check(uint32_t offset);

#ifdef __cplusplus
}
#endif

#endif
