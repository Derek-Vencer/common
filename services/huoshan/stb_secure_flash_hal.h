
/* Copyright 2024-2026 VE Starburst. All rights reserved.
 *
 * NOTICE：All information contained herein is, and remains the property of VE
 * Starburst. The intellectual and technical concepts contained herein are
 * proprietary to VE Starburst. and may be covered by patents, patents in process,
 * and are protected by trade secret or copyright law. Dissemination of this
 * information or reproduction of this material is strictly forbidden unless prior
 * written permission is obtained from VE Starburst.
 */

#ifndef _STB_SECURE_FLASH_HAL_H
#define _STB_SECURE_FLASH_HAL_H

#include <stdint.h>

/**
 * 大小 512 个字节，要求恢复初始化设置的情况下不能被清除，此区要保留
 */

/**
 * Flash 操作状态类型。
 * STB_FLASH_OK 表示操作成功，STB_FLASH_ERR 表示操作失败。
 */
typedef enum {
    STB_FLASH_OK = 0,
    STB_FLASH_ERR = -1
} stb_flash_status_t;

/**
 * 初始化 Flash Secure 适配层。
 * 返回 STB_FLASH_OK 表示成功，STB_FLASH_ERR 表示失败。
 */
stb_flash_status_t stb_secure_flash_init(void);

/**
 * 从 Flash 中读取数据。
 * @param offset 要读取的偏移地址。
 * @param buffer 存储读取数据的缓冲区指针。
 * @param length 要读取的数据长度。
 * 返回 STB_FLASH_OK 表示成功，STB_FLASH_ERR 表示失败。
 */
stb_flash_status_t stb_secure_flash_read(uint32_t offset, uint8_t *buffer, uint32_t length);

/**
 * 向 Flash 中写入数据。
 * @param offset 要写入的偏移地址。
 * @param buffer 包含要写入数据的缓冲区指针。
 * @param length 要写入的数据长度。
 * 返回 STB_FLASH_OK 表示成功，STB_FLASH_ERR 表示失败。
 */
stb_flash_status_t stb_secure_flash_write(uint32_t offset, const uint8_t *buffer, uint32_t length);

/**
 * 擦除 Flash 的指定数据。
 * @param offset 要擦除的偏移地址。
 * 返回 STB_FLASH_OK 表示成功，STB_FLASH_ERR 表示失败。
 */
stb_flash_status_t stb_secure_flash_erase(uint32_t offset, uint32_t length);

#endif // STB_SECURE_FLASH_HAL_H