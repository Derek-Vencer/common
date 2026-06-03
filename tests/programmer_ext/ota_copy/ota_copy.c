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
#include "cmsis.h"
#include "cmsis_nvic.h"
#include "hal_bootmode.h"
#include "hal_cache.h"
#include "export_fn_rom.h"
#include "hal_location.h"
#include "hal_norflash.h"
#include "hal_trace.h"
#include "hal_wdt.h"
#include "pmu.h"
#include "string.h"
#include "sys_api_programmer.h"
#include "crc_c.h"
#include "tgt_hardware.h"
#include "plat_addr_map.h"
#ifdef OTA_BIN_COMPRESSED
#include "bes_lzma_api.h"
#endif

typedef void (*FLASH_ENTRY)(void);

extern uint32_t __app_entry_address__[];

#define OTA_IMAGE_COPY_ENABLED
#define BOOT_MAGIC_NUMBER                   0xBE57EC1C
#define COPY_NEW_IMAGE	                    0x5a5a5a5a
//#define NEW_IMAGE_FLASH_OFFSET	            0x200000
#define NORMAL_BOOT		                    BOOT_MAGIC_NUMBER
#define FLASH_SECTOR_SIZE_IN_BYTES          4096
#define OTA_DATA_BUFFER_SIZE_FOR_BURNING    FLASH_SECTOR_SIZE_IN_BYTES
#define OTA_FLASH_LOGIC_ADDR                FLASH_NC_BASE

#define BOOT_WORD_A                     0xAAAAAAAA
#define BOOT_WORD_B                     0xBBBBBBBB
#define BOOT_WORD_C                     0xCCCCCCCC
#define BOOT_WORD_D                     0xDDDDDDDD
#define INVAILD_VALUE                   0xFFFFFFFF

#define NORFLASH_REMAP_ID               HAL_NORFLASH_REMAP_ID_0
#define NORFLASH_SE_REMAP_ID            HAL_NORFLASH_REMAP_ID_1
#define NORFLASH_NSE_REMAP_ID           HAL_NORFLASH_REMAP_ID_0

uint8_t dataBufferForBurning[OTA_DATA_BUFFER_SIZE_FOR_BURNING];

typedef struct
{
    uint32_t magicNumber; // NORMAL_BOOT or COPY_NEW_IMAGE
    uint32_t imageSize;
    uint32_t imageCrc;
    uint32_t newImageFlashOffset;
    uint32_t boot_word;
    uint32_t tzImageSize;
    uint32_t tzImageCrc;
    uint32_t tzNewImageFlashOffset;
} FLASH_OTA_BOOT_INFO_T;

#ifdef OTA_SEC_BOOT_INFO_OFFSET
#define OTA_SEC_BOOT_INFO_ADDR  (FLASH_BASE+OTA_SEC_BOOT_INFO_OFFSET)
#endif

#ifdef OTA_ENABLE
#ifdef OTA_SEC_BOOT_INFO_OFFSET
FLASH_OTA_BOOT_INFO_T otaBootInfoInFlash __attribute((section(".sec_ota_boot_info"))) =
#else
FLASH_OTA_BOOT_INFO_T otaBootInfoInFlash __attribute((section(".ota_boot_info"))) =
#endif
#else
FLASH_OTA_BOOT_INFO_T otaBootInfoInFlash =
#endif
#ifdef FORCE_COPY_NEW_IMAGE
    {COPY_NEW_IMAGE, FLASH_SIZE / 2 - OTA_CODE_OFFSET, 0, FLASH_SIZE / 2, BOOT_WORD_A};
#else
    {NORMAL_BOOT, 0, 0, OTA_CODE_OFFSET, BOOT_WORD_A};
#endif

uint32_t BOOT_TEXT_FLASH_LOC programmer_get_magic_number(void)
{
    uint32_t magic;

    magic = (uint32_t)&otaBootInfoInFlash;

    // First read (and also flush the controller prefetch buffer)
    *(volatile uint32_t *)(magic + 0x400);

    return *(volatile uint32_t *)magic;
}

uint32_t BOOT_TEXT_FLASH_LOC programmer_get_new_image_offset(void)
{
    return otaBootInfoInFlash.newImageFlashOffset;
}

uint32_t BOOT_TEXT_FLASH_LOC programmer_get_new_image_size(void)
{
    return otaBootInfoInFlash.imageSize;
}

uint32_t BOOT_TEXT_FLASH_LOC programmer_get_tz_image_offset(void)
{
    return otaBootInfoInFlash.tzNewImageFlashOffset;
}

uint32_t BOOT_TEXT_FLASH_LOC programmer_get_tz_image_size(void)
{
    return otaBootInfoInFlash.tzImageSize;
}

#if OTA_ENABLE
bool BOOT_TEXT_FLASH_LOC programmer_need_run_remap(uint32_t boot_word)
{
    bool ret = false;

    if(otaBootInfoInFlash.magicNumber == COPY_NEW_IMAGE && otaBootInfoInFlash.boot_word == boot_word)
    {
        ret = true;
    }

    return ret;
}
#endif

static int BOOT_TEXT_FLASH_LOC programmer_inflash_is_single_line_download_mode(void)
{
#ifdef SINGLE_WIRE_DOWNLOAD
    uint32_t bootmode = hal_sw_bootmode_get();

    return (bootmode & HAL_SW_BOOTMODE_SINGLE_LINE_DOWNLOAD);
#else
#ifdef USB_CDC_DOWNLOAD
    if(hal_sw_bootmode_get() & HAL_SW_BOOTMODE_DLD_USB_CDC)
        return true;
    else
#endif
        return false;
#endif
}

#if OTA_ENABLE
void BOOT_TEXT_FLASH_LOC remap_b_image(void)
{
    uint32_t magicNumber = programmer_get_magic_number();
    uint32_t ota_remap_offset = 0;
    uint32_t lock = 0;
    
    // CAUTION:
    // Trace has NOT been initialized, so no trace can be printed here

    if (COPY_NEW_IMAGE == magicNumber)
    {
        ota_remap_offset = programmer_get_new_image_offset();
        uint32_t dstLogicAddr = FLASH_BASE + OTA_CODE_OFFSET;
        uint32_t len = ota_remap_offset - OTA_CODE_OFFSET;
        uint32_t srcPhysicalAddr = ota_remap_offset;

        // Remap boot
        lock = int_lock_global();
        enum HAL_NORFLASH_RET_T ret = hal_norflash_boot_config_remap(HAL_FLASH_ID_0, NORFLASH_REMAP_ID, dstLogicAddr, len, srcPhysicalAddr);
        if (ret != HAL_NORFLASH_OK) {
            goto _err;
        }
        ret = hal_norflash_boot_enable_remap(HAL_FLASH_ID_0, NORFLASH_REMAP_ID);
        if (ret != HAL_NORFLASH_OK) {
            goto _err;
        }
        // CAUTION:
        // If cache has been enabled, the remap destination address had better be invalidted here.
        // Since the cache invalidation functions are located in .boot_sram_text,
        // the whole boot sram should be initialized before the invalidation.
_err:
        int_unlock_global(lock);
    }
}

void BOOT_TEXT_FLASH_LOC remap_tz_image(void)
{
    uint32_t magicNumber = programmer_get_magic_number();
    enum HAL_NORFLASH_RET_T ret = HAL_NORFLASH_OK;
    uint32_t lock = 0;

    if (COPY_NEW_IMAGE == magicNumber)
    {
        lock = int_lock_global();
        //remap ns bin
        uint32_t dstLogicAddr = FLASH_BASE+ programmer_get_new_image_offset();
        uint32_t len = programmer_get_new_image_size();
        uint32_t srcPhysicalAddr = len;
        ret = hal_norflash_boot_config_remap(HAL_FLASH_ID_0, NORFLASH_NSE_REMAP_ID, dstLogicAddr, len, srcPhysicalAddr);
        if (ret != HAL_NORFLASH_OK) {
            goto _err;
        }
        ret = hal_norflash_boot_enable_remap(HAL_FLASH_ID_0, NORFLASH_NSE_REMAP_ID);
        if (ret != HAL_NORFLASH_OK) {
            goto _err;
        }

        // Remap s bin
        dstLogicAddr = FLASH_BASE+ programmer_get_tz_image_offset();
        len = programmer_get_tz_image_size();
        srcPhysicalAddr = len;
        ret = hal_norflash_boot_config_remap(HAL_FLASH_ID_0, NORFLASH_SE_REMAP_ID, dstLogicAddr, len, srcPhysicalAddr);
        if (ret != HAL_NORFLASH_OK) {
            goto _err;
        }
        ret = hal_norflash_boot_enable_remap(HAL_FLASH_ID_0, NORFLASH_SE_REMAP_ID);
        if (ret != HAL_NORFLASH_OK) {
            goto _err;
        }
_err:
        int_unlock_global(lock);
    }
}

#endif

int BOOT_TEXT_FLASH_LOC ProgrammerInflashCheckBootToApp(void)
{
    bool isEnterSecondaryBootloader = false;
    if (is_ota_remap_enabled())
    {
        if (programmer_inflash_is_single_line_download_mode())
        {
            isEnterSecondaryBootloader = true;
        } else {
#if OTA_ENABLE
            if(programmer_need_run_remap(BOOT_WORD_B))
            {
                remap_b_image();
            }
            else if(programmer_need_run_remap(BOOT_WORD_D))
            {
                remap_tz_image();
            }
#endif
        }
    }
    else
    {
        uint32_t magicNumber = programmer_get_magic_number();
        if (programmer_inflash_is_single_line_download_mode())
        {
            isEnterSecondaryBootloader = true;
        }
        else
        {
            if(0x5a5a5a5a == magicNumber)
            {
                isEnterSecondaryBootloader = true;
            }
        }
    }

    if (isEnterSecondaryBootloader)
    {
        return 0;
    }

    return 1;
}
#if OTA_ENABLE

#ifdef OTA_IMAGE_COPY_ENABLED

void programmer_update_magic_number(uint32_t newMagicNumber)
{
    uint32_t lock;

    uint32_t addr = (uint32_t)&otaBootInfoInFlash;

    lock = int_lock();
    pmu_flash_write_config();
    hal_norflash_erase(HAL_FLASH_ID_0, addr, FLASH_SECTOR_SIZE_IN_BYTES);
    hal_norflash_write(HAL_FLASH_ID_0, addr, (uint8_t*)(&newMagicNumber), 4);
    pmu_flash_read_config();
    int_unlock(lock);
}

static uint32_t ota_check_whole_image_crc(uint32_t srcFlashOffset, uint32_t imageSize)
{
    uint32_t verifiedDataSize = 0;
    uint32_t crc32Value = 0;
    uint32_t verifiedBytes = 0;

    while (verifiedDataSize < imageSize)
    {
        if (imageSize - verifiedDataSize > OTA_DATA_BUFFER_SIZE_FOR_BURNING)
        {
            verifiedBytes = OTA_DATA_BUFFER_SIZE_FOR_BURNING;
        }
        else
        {
            verifiedBytes = imageSize - verifiedDataSize;
        }

        memcpy((void*)dataBufferForBurning, (uint8_t *)(OTA_FLASH_LOGIC_ADDR+srcFlashOffset+verifiedDataSize),
               OTA_DATA_BUFFER_SIZE_FOR_BURNING);

        if (0 == verifiedDataSize)
        {
            if (*(uint32_t *)dataBufferForBurning != NORMAL_BOOT)
            {
                crc32Value = 0;
                return crc32Value;
            }
            else
            {
                *(uint32_t *)dataBufferForBurning  = 0xFFFFFFFF;
            }
        }
        crc32Value = crc32_c(crc32Value, (void*)dataBufferForBurning, verifiedBytes);

        verifiedDataSize += verifiedBytes;
    }

    return crc32Value;
}

bool programmer_copy_new_image(uint32_t srcFlashOffset, uint32_t dstFlashOffset,
                                uint32_t imageCrc, uint32_t imageSize)
{
    uint32_t copiedDataSize = 0;
#ifdef __CRC16__
    uint32_t crcValue = 0xFFFF;
#else
    uint32_t crcValue = 0;
#endif
    uint32_t bytesToCopy = 0;
    uint32_t lock;

#ifdef PROGRAMMER_WATCHDOG
    uint32_t copiedDataSizeSinceLastWdPing = 0;
#endif

    crcValue = ota_check_whole_image_crc(srcFlashOffset, imageSize);
    PROGRAMMER_EXT_TRACE(2,"imageCrc:0x%x,0x%x",imageCrc,crcValue);
    if(imageCrc != crcValue)
    {
        programmer_update_magic_number(NORMAL_BOOT);
        hal_cmu_sys_reboot();
        while (1) {}
    }
#ifdef __CRC16__
    crcValue = 0xFFFF;
#else
    crcValue = 0;
#endif
    while (copiedDataSize < imageSize)
    {
        if (imageSize - copiedDataSize > OTA_DATA_BUFFER_SIZE_FOR_BURNING)
        {
            bytesToCopy = OTA_DATA_BUFFER_SIZE_FOR_BURNING;
        }
        else
        {
            bytesToCopy = imageSize - copiedDataSize;
        }
        lock = int_lock();

        memcpy((uint8_t *)dataBufferForBurning, (uint8_t *)(OTA_FLASH_LOGIC_ADDR + srcFlashOffset + copiedDataSize), bytesToCopy);

        hal_norflash_erase(HAL_FLASH_ID_0, dstFlashOffset + copiedDataSize,
                           OTA_DATA_BUFFER_SIZE_FOR_BURNING);

        hal_norflash_write(HAL_FLASH_ID_0, dstFlashOffset + copiedDataSize,
                           dataBufferForBurning, bytesToCopy);
		
		//dataBufferForBurning's buf is not changed, It's redundant;
        //memcpy((uint8_t *)dataBufferForBurning, (uint8_t *)(OTA_FLASH_LOGIC_ADDR + dstFlashOffset + copiedDataSize), bytesToCopy);

        int_unlock(lock);

        if (0 == copiedDataSize)
        {
            *(uint32_t *)dataBufferForBurning = 0xFFFFFFFF;
        }

        copiedDataSize += bytesToCopy;

#ifdef __CRC16__
        crcValue = crc16ccitt_c(crcValue, (uint8_t *)dataBufferForBurning, 0,
                              bytesToCopy);
#else
        crcValue = crc32_c(crcValue, (uint8_t *)dataBufferForBurning,
                         bytesToCopy);
#endif


#ifdef PROGRAMMER_WATCHDOG
        copiedDataSizeSinceLastWdPing += bytesToCopy;
        if (copiedDataSizeSinceLastWdPing > 20 * OTA_DATA_BUFFER_SIZE_FOR_BURNING)
        {
            programmer_watchdog_ping();
            copiedDataSizeSinceLastWdPing = 0;
        }
#endif
    }

    PROGRAMMER_EXT_TRACE(2, "Original CRC32 is 0x%x Confirmed CRC32 is 0x%x.", imageCrc, crcValue);

#ifdef FORCE_COPY_NEW_IMAGE
    crcValue = imageCrc;
#endif

    if (crcValue == imageCrc)
    {
        return true;
    }

    return false;
}

void copy_new_image(void)
{
    FLASH_OTA_BOOT_INFO_T otaBootInfo;
    memcpy((uint8_t *)&otaBootInfo, (uint8_t *)&otaBootInfoInFlash, sizeof(FLASH_OTA_BOOT_INFO_T));
    PROGRAMMER_EXT_TRACE(0, "ota boot info is:");
    PROGRAMMER_EXT_DUMP8("0x%02x ", (uint8_t *)&otaBootInfoInFlash, sizeof(FLASH_OTA_BOOT_INFO_T));

    uint32_t magicNumber = programmer_get_magic_number();
#ifdef OTA_TZ_ENABLE
    uint32_t ota_flash_offset_of_image = FLASH_S_SIZE;
    uint32_t se_ota_flash_offset_of_image = OTA_CODE_OFFSET;
#else
    uint32_t ota_flash_offset_of_image = OTA_CODE_OFFSET;
#endif

    if (COPY_NEW_IMAGE == magicNumber)
    {
        hal_norflash_disable_protection(HAL_FLASH_ID_0);
        //we will reboot once copy success
#ifdef OTA_BIN_COMPRESSED
        PROGRAMMER_EXT_TRACE(2, "Start copying new image from 0x%x to 0x%x...", otaBootInfo.newImageFlashOffset, ota_flash_offset_of_image);
        bool ret = ota_copy_compressed_image(otaBootInfo.newImageFlashOffset, ota_flash_offset_of_image,
                                            otaBootInfo.imageCrc, otaBootInfo.imageSize);
        if (!ret)
        {
            goto exit;
        }
        
        #ifdef OTA_TZ_ENABLE
        if (otaBootInfo.tzNewImageFlashOffset && otaBootInfo.tzNewImageFlashOffset != INVAILD_VALUE)
        {
            PROGRAMMER_EXT_TRACE(2, "Start copying new image from 0x%x to 0x%x...",
                otaBootInfo.tzNewImageFlashOffset, se_ota_flash_offset_of_image);
            ret = ota_copy_compressed_image(otaBootInfo.tzNewImageFlashOffset, se_ota_flash_offset_of_image,
                                            otaBootInfo.tzImageCrc, otaBootInfo.tzImageSize);
            if (!ret)
            {
                goto exit;
            }
        }
        #endif
#else
        PROGRAMMER_EXT_TRACE(2, "Start copying new image from 0x%x to 0x%x...", otaBootInfo.newImageFlashOffset, ota_flash_offset_of_image);
        //we will reboot once copy success
        bool ret = programmer_copy_new_image(otaBootInfo.newImageFlashOffset, ota_flash_offset_of_image,
                                            otaBootInfo.imageCrc, otaBootInfo.imageSize);
        if (!ret)
        {
            goto exit;
        }
        #ifdef OTA_TZ_ENABLE
        if (otaBootInfo.tzNewImageFlashOffset && otaBootInfo.tzNewImageFlashOffset != INVAILD_VALUE)
        {
            PROGRAMMER_EXT_TRACE(2, "Start copying new image from 0x%x to 0x%x...",
                otaBootInfo.tzNewImageFlashOffset, se_ota_flash_offset_of_image);
            ret = programmer_copy_new_image(otaBootInfo.tzNewImageFlashOffset, se_ota_flash_offset_of_image,
                                            otaBootInfo.tzImageCrc, otaBootInfo.tzImageSize);
            if (!ret)
            {
                goto exit;
            }
        }
        #endif
#endif

        programmer_update_magic_number(NORMAL_BOOT);
        NORM_LOG_FLUSH();
        hal_cmu_sys_reboot();
        while (1);

exit:
        //PROGRAMMER_EXT_TRACE(0,"Copying new image failed, enter OTA mode.");
        PROGRAMMER_EXT_TRACE(0, "Copying new image failed, enter Single Download mode.");
    }
}
#endif

static void ota_copy_init(void)
{
    if (is_ota_remap_enabled())
    {
       //return directly.
    }
    else
    {
#if defined(OTA_IMAGE_COPY_ENABLED)
        PROGRAMMER_EXT_TRACE(0,"begin copy_new_image");
        copy_new_image();
        PROGRAMMER_EXT_TRACE(0,"end copy_new_image");
#endif
    }
}

static const CUST_CMD_INIT_T CUST_CMD_INIT_TBL_LOC mod_init[] = {
    ota_copy_init,
};
#endif