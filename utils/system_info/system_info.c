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
#include "stdint.h"
#include "stdbool.h"
#include "plat_types.h"
#include "plat_addr_map.h"
#include "user_secure_boot.h"

#define SYSTEM_INFO_LISTSIZE_LOCATION  __attribute__((section(".system_info_list_size")))
#define SYSTEM_INFO_LIST_LOCATION      __attribute__((section(".system_info_list")))

extern const char sys_build_info[];



const unsigned int SYSTEM_INFO_LIST_LOCATION system_info_list[] ={
(unsigned int)sys_build_info,
};

const unsigned int SYSTEM_INFO_LISTSIZE_LOCATION system_info = sizeof(system_info_list)/sizeof(unsigned int);

