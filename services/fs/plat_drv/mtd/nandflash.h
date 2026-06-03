/***************************************************************************
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
 ***************************************************************************/
#ifndef __AOS_FS_NANDFLASH_H__
#define __AOS_FS_NANDFLASH_H__

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_NAND_PART_NUM 4

#ifndef AOS_NAND_PART_NUM
#define AOS_NAND_PART_NUM MAX_NAND_PART_NUM
#endif

#if AOS_NAND_PART_NUM > MAX_NAND_PART_NUM
#  error MAX_NAND_PART_NUM is 4
#endif

#ifndef AOS_NANDPART1_BLOCK_NUM
#define AOS_NANDPART1_BLOCK_NUM 1024
#endif

#ifndef AOS_NANDPART2_BLOCK_NUM
#define AOS_NANDPART2_BLOCK_NUM 1024
#endif

#ifndef AOS_NANDPART3_BLOCK_NUM
#define AOS_NANDPART3_BLOCK_NUM 100
#endif

#ifndef AOS_NANDPART4_BLOCK_NUM
#define AOS_NANDPART4_BLOCK_NUM 100
#endif

int bes_nandflash_slotinitialize_partition_offset(int idx);
int bes_nandflash_slotinitialize_partition_num(int idx);

#ifdef __cplusplus
}
#endif

#endif /*__AOS_FS_NANDFLASH_H__*/

