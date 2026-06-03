
/* Copyright 2024-2026 VE Starburst. All rights reserved.
 *
 * NOTICE：All information contained herein is, and remains the property of VE
 * Starburst. The intellectual and technical concepts contained herein are
 * proprietary to VE Starburst. and may be covered by patents, patents in process,
 * and are protected by trade secret or copyright law. Dissemination of this
 * information or reproduction of this material is strictly forbidden unless prior
 * written permission is obtained from VE Starburst.
 */

// #include "rt_thread.h"
#include "stb_secure_flash_hal.h"
#include "norflash_api.h"

extern uint8_t __huoshan_info_start[];
extern uint8_t __huoshan_info_end[];

#define HUOSHAN_INFO_START_ADDR   (__huoshan_info_start)

stb_flash_status_t stb_secure_flash_init(void)
{
    enum NORFLASH_API_RET_T result;
    enum HAL_FLASH_ID_T flash_id;
    uint32_t sector_size = 0;
    uint32_t block_size = 0;
    uint32_t page_size = 0;

    flash_id = norflash_api_get_dev_id_by_addr((uint32_t)__huoshan_info_start);
    hal_norflash_get_size(flash_id,
                          NULL,
                          &block_size,
                          &sector_size,
                          &page_size);
    result = norflash_api_register(NORFLASH_API_MODULE_ID_HUOSHAN,
                                   flash_id,
                                   ((uint32_t)__huoshan_info_start),
                                   ((uint32_t)__huoshan_info_end - (uint32_t)__huoshan_info_start),
                                   block_size,
                                   sector_size,
                                   page_size,
                                   ((uint32_t)__huoshan_info_end - (uint32_t)__huoshan_info_start),
                                   NULL);
    return (result == NORFLASH_API_OK) ? STB_FLASH_OK : STB_FLASH_ERR;
}

stb_flash_status_t stb_secure_flash_read(uint32_t offset,
    uint8_t *buffer, uint32_t length)
{
    enum NORFLASH_API_RET_T ret = norflash_api_read(NORFLASH_API_MODULE_ID_HUOSHAN,
                                                   (uint32_t)HUOSHAN_INFO_START_ADDR,
                                                   buffer, length);
    return (ret == NORFLASH_API_OK) ? STB_FLASH_OK : STB_FLASH_ERR;
}

stb_flash_status_t stb_secure_flash_write(
    uint32_t offset, const uint8_t *buffer, uint32_t length)
{
    enum NORFLASH_API_RET_T ret = norflash_api_write(NORFLASH_API_MODULE_ID_HUOSHAN,
                             (uint32_t)HUOSHAN_INFO_START_ADDR + offset,
                             (uint8_t *)buffer, length, false);
    return (ret == NORFLASH_API_OK) ? STB_FLASH_OK : STB_FLASH_ERR;
}

stb_flash_status_t stb_secure_flash_erase(uint32_t offset, uint32_t length)
{
    enum NORFLASH_API_RET_T ret = norflash_api_erase(NORFLASH_API_MODULE_ID_HUOSHAN,
                             (uint32_t)HUOSHAN_INFO_START_ADDR + offset, length, false);
    return (ret == NORFLASH_API_OK) ? STB_FLASH_OK : STB_FLASH_ERR;
}
