/****************************************************************************
 * drivers/drivers_initialize.c
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include "mtd/mtd.h"
#ifdef EMMC_ENABLE
#include "emmc/emmc.h"
#endif
#ifdef SDMMC_ENABLE
#include "sdemmc/sdemmc.h"
#endif
#ifdef USBHOST_MASS_ENABLE
#include "usbhost_mass/usbmass.h"
#endif
#ifdef BES_AOS_RPMSGFS_SERVER
#include "rpmsgfs/rpmsgfs.h"
#endif

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void fs_emmc_drivers_initialize()
{
#ifdef EMMC_ENABLE
    bes_emmc_drivers_register();
#endif
}

void fs_sdmmc_drivers_initialize()
{
#ifdef SDMMC_ENABLE
    bes_sdmmc_drivers_register();
#endif
}

void fs_nand_drivers_initialize()
{
#ifdef NANDFLASH_SUPPORT
    bes_nand_drivers_register();
#endif
}

void fs_usbmass_drivers_initialize()
{
#ifdef USBHOST_MASS_ENABLE
    bes_usbhost_mass_storage_slotinitialize(0);
#endif
}

void fs_norflash_drivers_initialize()
{
#if defined(FLASH_BASE) && defined(NORFLASH_FS_SIZE) && (NORFLASH_FS_SIZE > 0)
    bes_norflash_drivers_register(0, (uint8_t *)NORFLASH_FS_ADDR, NORFLASH_FS_SIZE);
#endif
}

void fs_rpmsgfs_drivers_initialize()
{
#ifdef BES_AOS_RPMSGFS_SERVER
    bes_aos_rpmsgfs_server_init();
#endif
}

/****************************************************************************
 * Name: drivers_initialize
 *
 * Description:
 *   drivers_initialize will be called once during OS initialization after
 *   the basic OS services have been initialized.
 *
 *   drivers_initialize is called after the OS initialized but before the
 *   user initialization logic has been started and before the libraries
 *   have been initialized.  OS services and driver services are available.
 *
 ****************************************************************************/

void drivers_initialize(void)
{
    bes_ram_slotinitialize(0);
    fs_emmc_drivers_initialize();
    fs_sdmmc_drivers_initialize();
    fs_nand_drivers_initialize();
    fs_usbmass_drivers_initialize();
    fs_norflash_drivers_initialize();
    fs_rpmsgfs_drivers_initialize();
}
