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

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include "mtd/mtd.h"
#include "hal_cmu.h"
#include "nandflash.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
uint32_t g_nftlm_partition[MAX_NAND_PART_NUM] = {AOS_NANDPART1_BLOCK_NUM, AOS_NANDPART2_BLOCK_NUM,
                                                 AOS_NANDPART3_BLOCK_NUM, AOS_NANDPART4_BLOCK_NUM
                                                };

/****************************************************************************
 * public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: bes_nandflash_slotinitialize_partition_offset
 *
 * Description:
 *   get the offset in block(page) to the partition
 ****************************************************************************/

int bes_nandflash_slotinitialize_partition_offset(int idx)
{
    int offset = 0;
    for (int i = 0; i < idx; i++) {
        offset += g_nftlm_partition[i];
    }
    return offset;
}

/****************************************************************************
 * Name: bes_nandflash_slotinitialize_partition_num
 *
 * Description:
 *   get the number of block(page) in the partition
 ****************************************************************************/

int bes_nandflash_slotinitialize_partition_num(int idx)
{
    return g_nftlm_partition[idx];
}

/****************************************************************************
 * Name: bes_nand_drivers_register
 *
 * Description:
 *   Initialize nandflash driver for fs
 ****************************************************************************/

void bes_nand_drivers_register(void)
{
    int minor = 0;
#if defined(CHIP_BEST1700)
    int flashid = (int)HAL_FLASH_ID_2;
#elif defined(CHIP_BEST1600)
    int flashid = (int)HAL_FLASH_ID_0;
#else
    int flashid = (int)HAL_FLASH_ID_1;
#endif

#if defined(NANDFLASH_MTD_YAFFS_ENABLE)
    /* /dev/nandblock0 */
    bes_nand_slotinitialize_yaffs(minor, flashid);
#elif defined(NANDFLASH_MTD_YAFFS_PARTITION_ENABLE)
    /* /dev/nandblock0_1 ...*/
    bes_nand_slotinitialize_partition_yaffs(minor, flashid);
#elif defined(NANDFLASH_MTD_ENABLE)
    /* /dev/nandblock0 */
    bes_nand_slotinitialize_dhara_cache(minor, flashid);
#elif defined(NANDFLASH_PARTITION_ENABLE)
    for (int i = 0; i < AOS_NAND_PART_NUM; i++) {
        /* /dev/nandblock0_1 ...*/
        bes_nand_slotinitialize_partition_dhara(minor, flashid, i + 1);
    }
#else
    /* /dev/nandblock0 */
    bes_nand_slotinitialize_dhara(minor, flashid);
#endif
}

