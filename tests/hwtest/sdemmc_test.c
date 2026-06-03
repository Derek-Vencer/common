/***************************************************************************
 *
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
 *
 ****************************************************************************/
#ifdef SDEMMC_TEST

#include <string.h>
#include <stdlib.h>
#include "hal_cmu.h"
#include "hal_wdt.h"
#include "hal_gpio.h"
#include "hal_sdmmc.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_iomux.h"
#include "hal_sysfreq.h"
#include "hal_location.h"
#ifdef RTOS
    #include "cmsis_os.h"
#endif
#ifdef UTILS_ESHELL_EN
    #include "eshell.h"
#endif
#if defined(AOS_FS_ENABLE) && !defined(UTILS_ESHELL_EN)
    #include "fs/fs.h"
#endif

#if 1
    #define SDEMMC_TRACE(n, s, ...)     HWTEST_TRACE(n, s, ##__VA_ARGS__)
    #define SDEMMC_DUMP8(n, s, ...)     DUMP8(n, s, ##__VA_ARGS__)
    #define SDEMMC_DUMP16(n, s, ...)    DUMP16(n, s, ##__VA_ARGS__)
    #define SDEMMC_DUMP32(n, s, ...)    DUMP32(n, s, ##__VA_ARGS__)
#else
    #define SDEMMC_TRACE(n, s, ...)     TRACE_DUMMY(n, s, ##__VA_ARGS__)
    #define SDEMMC_DUMP8(n, s, ...)     TRACE_DUMMY(n, s, ##__VA_ARGS__)
    #define SDEMMC_DUMP16(n, s, ...)    TRACE_DUMMY(n, s, ##__VA_ARGS__)
    #define SDEMMC_DUMP32(n, s, ...)    TRACE_DUMMY(n, s, ##__VA_ARGS__)
#endif

#if 0
    #define SDEMMC_TRACE_CB(n, s, ...)  TRACE(n, s, ##__VA_ARGS__)
#else
    #define SDEMMC_TRACE_CB(n, s, ...)  TRACE_DUMMY(n, s, ##__VA_ARGS__)
#endif

#if 1
    #define SDEMMC_DELAY_MS(ms)         hal_sys_timer_delay(MS_TO_TICKS(ms))
    #define SDEMMC_DELAY_US(us)         hal_sys_timer_delay_us(us)
#else
    #define SDEMMC_DELAY_MS(ms)         do{}while(0)
    #define SDEMMC_DELAY_US(us)         do{}while(0)
#endif

/*******************************************************************************
Note1:
    The following boards use VIO for emmc power supply. You need to add macro
    PMU_VIO_3V3_ENABLE=1 during compilation to enable vio power supply

    1. BES2700IB (1502x) EVB_Core Board_V1.x
    2. BES2700IMP (1503) EVB_Core Board_V1.x

Note2:
    The following board does not use VIO, but use DC/DC or LDO power supply

    1. BES2710IBP (1502p) EVB_Core Board_V1.x
*******************************************************************************/

/*************************Function configuration area**************************/
#define SDEMMC_ID       HAL_SDMMC_ID_1
#define EMMC_POWER_3V3  HAL_GPIO_PIN_P9_4
#define EMMC_POWER_1V8  HAL_GPIO_PIN_P9_2

#if defined(CHIP_BEST1502P)
#define EMMC_POWER_ON
#else
//#define EMMC_POWER_ON         //used for the boards of QianChen
#endif

#define EMMC_SLEEP_ENABLE
//#define EMMC_CACHE_ENABLE
//#define EMMC_RANDOM_WRITE_ENABLE
//#define EMMC_POLLING_MODE     //default dma mode
//#define EMMC_ERAES_ENABLE
//#define SDMMC_TX_RX_USE_CALLBACK

//#define EMMC_STRESS_TEST
//#define EMMC_OPEN_CLOSE_TEST
//#define EMMC_FORCE_CLOSE_TEST

//#define EMMC_OPEN_WITH_INT_LOCK_TEST

//#define EMMC_RST_OPEN_CLOSE_TEST
//#define EMMC_RST_OPEN_CLOSE_SIMPLE
//#define EMMC_RST_OPEN_CLOSE_RANDOM

//#define EMMC_BUS_WIDTH_4_TEST

//#define EMMC_GET_EXT_CSD_TEST

//#define EMMC_PWROFF_NOTIFICATION_TEST

/******************************************************************************/
#if defined(EMMC_RST_OPEN_CLOSE_SIMPLE) && defined(EMMC_RST_OPEN_CLOSE_RANDOM)
    #error "EMMC_RST_OPEN_CLOSE_SIMPLE and EMMC_RST_OPEN_CLOSE_RANDOM, the 2 macros can only open one of them"
#endif
#if defined(SDMMC_TX_RX_USE_CALLBACK) && defined(EMMC_POLLING_MODE)
    #error "SDMMC_TX_RX_USE_CALLBACK and EMMC_POLLING_MODE, the 2 macros can only open one of them"
#endif

static void rand_generator(uint32_t *dest, uint32_t size)
{
    while (size--) {
        *dest++ = (uint32_t)((rand() % RAND_MAX) / (double)RAND_MAX * 0xFFFFFFFF);
    }
}

#if defined(SDMMC_TX_RX_USE_CALLBACK)
    static volatile uint8_t sdmmc_txrx_done = 0;
#endif

static uint8_t error_flag = 0;
POSSIBLY_UNUSED static void hal_sdmmc_host_error(enum HAL_SDMMC_HOST_ERR error)
{
    error_flag = 1;
    SDEMMC_TRACE(1, "******sdmmc host error:%d", error);

#ifdef EMMC_RST_OPEN_CLOSE_RANDOM
    hal_wdt_stop(HAL_WDT_ID_0);
    while (1) {
        SDEMMC_TRACE(1, "******sdmmc host error:%d", error);
        osDelay(1000);
    }
#endif
}

#if defined(SDMMC_TX_RX_USE_CALLBACK)
static void hal_sdmmc_txrx_done(void)
{
    sdmmc_txrx_done = 1;
    SDEMMC_TRACE_CB(0, "------sdmmc dma txrx done");
}
#endif

#if defined(SDMMC_USE_SEM) && defined(RTOS)
#define SEM_TIMEOUT_VALUE   5000
static osSemaphoreId_t sem_id = NULL;

static void hal_sdmmc_sem_init(void)
{
    if (sem_id == NULL) {
        sem_id = osSemaphoreNew(1, 0, NULL);
    } else {
        SDEMMC_TRACE(0, "******The value of sem_id is incorrect");
    }
}

static void hal_sdmmc_sem_reset(void)
{
    int cnt, i;

    cnt = osSemaphoreGetCount(sem_id);
    for (i = 0; i < cnt; i++) {
        if (osSemaphoreAcquire(sem_id, SEM_TIMEOUT_VALUE) != osOK) {
            SDEMMC_TRACE(0, "******sem acquire error");
        }
    }
}

static void hal_sdmmc_sem_post(enum HAL_SDMMC_DEVICE_RW dir)
{
    (void)dir;
    if (sem_id) {
        if (osSemaphoreRelease(sem_id) != osOK) {
            SDEMMC_TRACE(0, "******sem release error");
        }
    } else {
        SDEMMC_TRACE(0, "******sem_id is not initialized");
    }
}

static enum HAL_SDMMC_ERR hal_sdmmc_sem_wait(void)
{
    enum HAL_SDMMC_ERR ret = HAL_SDMMC_ERR_SEM;

    if (osSemaphoreAcquire(sem_id, SEM_TIMEOUT_VALUE) != osOK) {
        SDEMMC_TRACE(0, "******sem acquire error");
    } else {
        ret = HAL_SDMMC_ERR_NONE;
    }

    return ret;
}
#endif

POSSIBLY_UNUSED static struct HAL_SDMMC_CB_T sdemmc_test_callback = {
    .hal_sdmmc_signal_voltage_switch = NULL,//emmc is useless
    .hal_sdmmc_host_error = hal_sdmmc_host_error,
#if defined(SDMMC_TX_RX_USE_CALLBACK)
    .hal_sdmmc_txrx_done = hal_sdmmc_txrx_done,
#else
    .hal_sdmmc_txrx_done = NULL,
#endif
// #if defined(SDMMC_USE_SEM) && defined(RTOS)
//     .hal_sdmmc_sem_init = hal_sdmmc_sem_init,
//     .hal_sdmmc_sem_reset = hal_sdmmc_sem_reset,
//     .hal_sdmmc_sem_post = hal_sdmmc_sem_post,
//     .hal_sdmmc_sem_wait = hal_sdmmc_sem_wait,
// #else
//     .hal_sdmmc_sem_init = NULL,
//     .hal_sdmmc_sem_reset = NULL,
//     .hal_sdmmc_sem_post = NULL,
//     .hal_sdmmc_sem_wait = NULL,
// #endif
};

static void sdemmc_dump32(const char *str, uint32_t *buf, uint32_t size)
{
    uint32_t i;

    TRACE_FLUSH();
    if (str) {
        SDEMMC_TRACE(1, "%s, buf=%x, size=%d", str, (uint32_t)buf, size);
    }
    SDEMMC_TRACE(0, " ");
    for (i = 0; i < size / 4; i += 8) {
        SDEMMC_TRACE(1, "%08X %08X %08X %08X %08X %08X %08X %08X",
                     buf[i + 0], buf[i + 1], buf[i + 2], buf[i + 3], buf[i + 4], buf[i + 5], buf[i + 6], buf[i + 7]);
    }
    SDEMMC_TRACE(1, " ");
    TRACE_FLUSH();
}

#define MULTI_TEST_SIZE             (2 * 1024 * 1024)//30MBytes
#define MULTI_TEST_CNT              (MULTI_TEST_SIZE / BUF_SIZE)

#if (RAM_SIZE <= 256 * 1024)
    #define BUF_SIZE 2048
#else
    #define BUF_SIZE (64 * 1024)
#endif
#if (BUF_SIZE < 512)
    #error "BUF_SIZE cannot be less than 512"
#endif

SYNC_FLAGS_LOC static uint32_t txbuf[BUF_SIZE / 4];
SYNC_FLAGS_LOC static uint32_t rxbuf[BUF_SIZE / 4];
static struct HAL_SDMMC_CARD_INFO_T card_info;
static void emmc_read_write_test(uint32_t sector_size, uint32_t sector_count)
{
    int ret;
    uint32_t i;
    uint64_t val;
    uint32_t blks;
    uint32_t start_addr;
    uint32_t card_offset = 0;
    uint32_t time_start, time[5];
    POSSIBLY_UNUSED uint8_t sleep_status;

    SDEMMC_TRACE(0, "  ");
#ifdef EMMC_SLEEP_ENABLE
    ret = hal_sdmmc_sleep_card(SDEMMC_ID);
    SDEMMC_TRACE(0, "======card sleep, ret:%d", ret);
    hal_sdmmc_get_card_sleep_status(SDEMMC_ID, &sleep_status);
    SDEMMC_TRACE(0, "======card sleep status: %d(%s)", sleep_status, sleep_status ? "sleep" : "awake");
#endif
    rand_generator((uint32_t *)txbuf, BUF_SIZE / sizeof(uint32_t));

#ifdef EMMC_RANDOM_WRITE_ENABLE
    card_offset = (uint32_t)((rand() % RAND_MAX) / (double)RAND_MAX * sector_count);
    SDEMMC_TRACE(0, "------random card offset:0x%X", card_offset);
    if ((uint64_t)card_offset * 512 + MULTI_TEST_SIZE > (uint64_t)sector_count * 512) {
        card_offset = (uint32_t)((uint64_t)sector_count * 512 - MULTI_TEST_SIZE) / 512;
    }
    SDEMMC_TRACE(0, "------final card offset :0x%X", card_offset);
#endif

    SDEMMC_TRACE(0, "  ");
    SDEMMC_TRACE(0, "=================> writing card");
    SDEMMC_TRACE(1, "------card offset=0x%x, buf=0x%x, buf sz=%u", card_offset, (uint32_t)txbuf, BUF_SIZE);
#ifdef EMMC_SLEEP_ENABLE
    ret = hal_sdmmc_awake_card(SDEMMC_ID);
    SDEMMC_TRACE(0, "======card awake, ret:%d", ret);
    hal_sdmmc_get_card_sleep_status(SDEMMC_ID, &sleep_status);
    SDEMMC_TRACE(0, "======card sleep status: %d(%s)", sleep_status, sleep_status ? "sleep" : "awake");
#endif

    //single write test
    SDEMMC_TRACE(1, "------single write start, total size:%u, write size:%u, cnt:1", BUF_SIZE, BUF_SIZE);
    time_start = hal_fast_sys_timer_get();
#if defined(SDMMC_TX_RX_USE_CALLBACK)
    sdmmc_txrx_done = 0;
#endif
#ifdef EMMC_POLLING_MODE
    blks = hal_sdmmc_write_blocks_polling(SDEMMC_ID, card_offset, BUF_SIZE / sector_size, (uint8_t *)txbuf);
#else
    blks = hal_sdmmc_write_blocks(SDEMMC_ID, card_offset, BUF_SIZE / sector_size, (uint8_t *)txbuf);
#endif
    if (error_flag) {
        goto error;
    }
#if defined(SDMMC_TX_RX_USE_CALLBACK)
    while (!sdmmc_txrx_done);
    SDEMMC_TRACE_CB(2, "------write done");
#endif
    time[0] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - time_start);

    //multi write test
    SDEMMC_TRACE(1, "------multi write start,  total size:%uMB, write size:%u, cnt:%u", MULTI_TEST_SIZE / 1024 / 1024, BUF_SIZE, MULTI_TEST_CNT);
    time_start = hal_fast_sys_timer_get();
    for (i = 0; i < MULTI_TEST_CNT - 1; i++) {
        start_addr = card_offset + i * BUF_SIZE / sector_size;

#if defined(SDMMC_TX_RX_USE_CALLBACK)
        sdmmc_txrx_done = 0;
#endif
#ifdef EMMC_POLLING_MODE
        blks = hal_sdmmc_write_blocks_polling(SDEMMC_ID, start_addr, BUF_SIZE / sector_size, (uint8_t *)txbuf);
#else
        blks = hal_sdmmc_write_blocks(SDEMMC_ID, start_addr, BUF_SIZE / sector_size, (uint8_t *)txbuf);
#endif
        if (error_flag) {
            goto error;
        }
#if defined(SDMMC_TX_RX_USE_CALLBACK)
        while (!sdmmc_txrx_done);
        SDEMMC_TRACE_CB(2, "------write done");
#endif
    }
    time[1] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - time_start);
    if (blks == BUF_SIZE / sector_size) {
        SDEMMC_TRACE(1, "------write card complete");
    } else {
        SDEMMC_TRACE(1, "******write card failed, blks=%d", blks);
    }
#ifdef EMMC_SLEEP_ENABLE
    ret = hal_sdmmc_sleep_card(SDEMMC_ID);
    SDEMMC_TRACE(0, "======card sleep, ret:%d", ret);
    hal_sdmmc_get_card_sleep_status(SDEMMC_ID, &sleep_status);
    SDEMMC_TRACE(0, "======card sleep status: %d(%s)", sleep_status, sleep_status ? "sleep" : "awake");
#endif

    SDEMMC_TRACE(0, "  ");
    SDEMMC_TRACE(1, "=================> reading card");
    SDEMMC_TRACE(1, "------card offset=0x%x, buf=0x%x, buf sz=%u", card_offset, (uint32_t)rxbuf, BUF_SIZE);
#ifdef EMMC_SLEEP_ENABLE
    ret = hal_sdmmc_awake_card(SDEMMC_ID);
    SDEMMC_TRACE(0, "======card awake, ret:%d", ret);
    hal_sdmmc_get_card_sleep_status(SDEMMC_ID, &sleep_status);
    SDEMMC_TRACE(0, "======card sleep status: %d(%s)", sleep_status, sleep_status ? "sleep" : "awake");
#endif

    //single read test
    SDEMMC_TRACE(1, "------single read start, total size:%u, read size:%u, cnt:1", BUF_SIZE, BUF_SIZE);
    time_start = hal_fast_sys_timer_get();
#if defined(SDMMC_TX_RX_USE_CALLBACK)
    sdmmc_txrx_done = 0;
#endif
#ifdef EMMC_POLLING_MODE
    blks = hal_sdmmc_read_blocks_polling(SDEMMC_ID, card_offset, BUF_SIZE / sector_size, (uint8_t *)rxbuf);
#else
    blks = hal_sdmmc_read_blocks(SDEMMC_ID, card_offset, BUF_SIZE / sector_size, (uint8_t *)rxbuf);
#endif
    if (error_flag) {
        goto error;
    }
#if defined(SDMMC_TX_RX_USE_CALLBACK)
    while (!sdmmc_txrx_done);
    SDEMMC_TRACE_CB(2, "------read done");
#endif
    time[2] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - time_start);

    //multi read test
    SDEMMC_TRACE(1, "------multi read start,  total size:%uMB, read size:%d, cnt:%d", MULTI_TEST_SIZE / 1024 / 1024, BUF_SIZE, MULTI_TEST_CNT);
    time_start = hal_fast_sys_timer_get();
    for (i = 0; i < MULTI_TEST_CNT - 1; i++) {
        start_addr = card_offset + i * BUF_SIZE / sector_size;

#if defined(SDMMC_TX_RX_USE_CALLBACK)
        sdmmc_txrx_done = 0;
#endif
#ifdef EMMC_POLLING_MODE
        blks = hal_sdmmc_read_blocks_polling(SDEMMC_ID, start_addr, BUF_SIZE / sector_size, (uint8_t *)rxbuf);
#else
        blks = hal_sdmmc_read_blocks(SDEMMC_ID, start_addr, BUF_SIZE / sector_size, (uint8_t *)rxbuf);
#endif
        if (error_flag) {
            goto error;
        }
#if defined(SDMMC_TX_RX_USE_CALLBACK)
        while (!sdmmc_txrx_done);
        SDEMMC_TRACE_CB(2, "------read done");
#endif
    }
    time[3] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - time_start);
    if (blks == BUF_SIZE / sector_size) {
        SDEMMC_TRACE(1, "------read card complete");
    } else {
        SDEMMC_TRACE(1, "******read card failed, blks=%d", blks);
    }

    //compare data
    SDEMMC_TRACE(0, "  ");
    SDEMMC_TRACE(1, "=================> checking card data");
    time_start = hal_fast_sys_timer_get();
    for (i = 0; i < MULTI_TEST_CNT - 1; i++) {
        start_addr = card_offset + i * BUF_SIZE / sector_size;

#if defined(SDMMC_TX_RX_USE_CALLBACK)
        sdmmc_txrx_done = 0;
#endif
#ifdef EMMC_POLLING_MODE
        hal_sdmmc_read_blocks_polling(SDEMMC_ID, start_addr, BUF_SIZE / sector_size, (uint8_t *)rxbuf);
#else
        hal_sdmmc_read_blocks(SDEMMC_ID, start_addr, BUF_SIZE / sector_size, (uint8_t *)rxbuf);
#endif
        if (error_flag) {
            goto error;
        }
#if defined(SDMMC_TX_RX_USE_CALLBACK)
        while (!sdmmc_txrx_done);
        SDEMMC_TRACE_CB(2, "------read done");
#endif
        ret = memcmp((uint32_t *)txbuf, (uint32_t *)rxbuf, BUF_SIZE / sector_size);
        if (ret) {
            SDEMMC_TRACE(1, "check data failed, ret=%d, line=%d", ret, __LINE__);
            sdemmc_dump32("dump src", txbuf, BUF_SIZE / sector_size);
            sdemmc_dump32("dump dst", rxbuf, BUF_SIZE / sector_size);
            SDEMMC_TRACE(1, "halt for check card data error");
            while (1);
        }
    }
    time[4] = FAST_TICKS_TO_US(hal_fast_sys_timer_get() - time_start);
    SDEMMC_TRACE(1, "=================$ data check is OK!!!");

#if !defined(EMMC_STRESS_TEST) && defined(EMMC_SLEEP_ENABLE)
    ret = hal_sdmmc_sleep_card(SDEMMC_ID);
    SDEMMC_TRACE(0, "======card sleep, ret:%d", ret);
    hal_sdmmc_get_card_sleep_status(SDEMMC_ID, &sleep_status);
    SDEMMC_TRACE(0, "======card sleep status: %d(%s)", sleep_status, sleep_status ? "sleep" : "awake");
#endif

error:
    if (error_flag) {
        error_flag = 0;
        SDEMMC_TRACE(0, "EMMC WRITE/READ ERROR");
    } else {
        //calc speed
        SDEMMC_TRACE(1, "  ");
        SDEMMC_TRACE(1, "==========read and write success==========");
        SDEMMC_TRACE(1, "******************************************");
        SDEMMC_TRACE(1, "TIME COST        : read       [%uus]", time[2]);
        SDEMMC_TRACE(1, "TIME COST        : multi read [%uus]", time[3]);
        SDEMMC_TRACE(1, "TIME COST        : write      [%uus]", time[0]);
        SDEMMC_TRACE(1, "TIME COST        : multi write[%uus]", time[1]);
        SDEMMC_TRACE(1, "TIME COST        : multi cmp  [%uus]", time[4]);
        SDEMMC_TRACE(1, "  ");
        SDEMMC_TRACE(1, "******************************************");
        SDEMMC_TRACE(1, "read size        :%ubytes", BUF_SIZE);
        val = (uint64_t)BUF_SIZE * MULTI_TEST_CNT / 1024;
        SDEMMC_TRACE(1, "multi read size  :%uKbytes=%uMbytes", (uint32_t)val, (uint32_t)(val / 1024));
        SDEMMC_TRACE(1, "write size       :%ubytes", BUF_SIZE);
        SDEMMC_TRACE(1, "multi write size :%uKbytes=%uMbytes", (uint32_t)val, (uint32_t)(val / 1024));
        SDEMMC_TRACE(1, "cmp size         :%ubytes", BUF_SIZE);
        SDEMMC_TRACE(1, "multi cmp size   :%uKbytes=%uMbytes", (uint32_t)val, (uint32_t)(val / 1024));
        SDEMMC_TRACE(1, "  ");

        //speed=MB/s
        //     =size/1024/1024MB/(time*10^-6)s
        //     =size/1024/1024*1000000/time (MB/s)
        //     =size*1000000/1024/1024/time (MB/s)
        //     =size*0.954/time (MB/s)
        //     =size*954/1000/time (MB/s)
        //     =size*95400/1000/time (MB/s) //The result is magnified 100 times and used to display 2 decimal places
        time[2] = (uint32_t)((uint64_t)BUF_SIZE * 95400 / 1000 / time[2]);                  //read
        time[3] = (uint32_t)((uint64_t)BUF_SIZE * 95400 * MULTI_TEST_CNT / 1000 / time[3]); //multi read
        time[0] = (uint32_t)((uint64_t)BUF_SIZE * 95400 / 1000 / time[0]);                  //write
        time[1] = (uint32_t)((uint64_t)BUF_SIZE * 95400 * MULTI_TEST_CNT / 1000 / time[1]); //multi write

        SDEMMC_TRACE(1, "emmc cache       :%s", card_info.cache_ctrl ? "ON" : "OFF");
        SDEMMC_TRACE(1, "  ");
        SDEMMC_TRACE(1, "read speed       :%u.%uMB/s", time[2] / 100, time[2] % 100);
        SDEMMC_TRACE(1, "multi read speed :%u.%uMB/s", time[3] / 100, time[3] % 100);
        SDEMMC_TRACE(1, "write speed      :%u.%uMB/s", time[0] / 100, time[0] % 100);
        SDEMMC_TRACE(1, "multi write speed:%u.%uMB/s", time[1] / 100, time[1] % 100);
        SDEMMC_TRACE(1, "******************************************");
        SDEMMC_TRACE(1, "  ");
    }
}

#if defined(AOS_FS_ENABLE) && !defined(UTILS_ESHELL_EN)
#define MAX_BUFF_SIZE       32768
#define MIN_BUFF_SIZE       128
#define FS_RATE_FILE        "fs_rate.log"
#define FS_RATE_BASE_LENGTH 500*1024
#define FS_MOUNT_NAME_LEN   32
#define TRUNCATE_EMPTY_YES  1
#define TRUNCATE_EMPTY_NO   0
static char g_buff[MAX_BUFF_SIZE + 1] = {'\0'};
static char g_rate_name[NAME_MAX + 1] = {'\0'};

static void fs_rate_read(int32_t single_size)
{
    FILE *fd;
    fd = fopen(g_rate_name, "r");
    if (fd == NULL) {
        SDEMMC_TRACE(0, "%s fail: open err, path=%s", __func__, g_rate_name);
        return;
    }

    int32_t read_len = 0;
    int32_t total_len = FS_RATE_BASE_LENGTH;
    uint32_t tvbegin = hal_sys_timer_get();
    while (total_len > 0) {
        int32_t res;
        int32_t tmp;
        tmp = total_len > single_size ? single_size : total_len;
        res = fread(g_buff, tmp, 1, fd);
        if (res <= 0) {
            SDEMMC_TRACE(0, "%s fail: res=%d", __func__, res);
            break;
        }
        total_len -= tmp;
        read_len += tmp;
    }
    fclose(fd);
    uint32_t tvdone = hal_sys_timer_get();
    if (total_len > 0) {
        SDEMMC_TRACE(0, "%s fail: not reading enough data leftlen:%d read_len:%d",
                     __func__, total_len, read_len);
        return;
    }
    uint32_t ms = TICKS_TO_MS(tvdone - tvbegin);
    if (ms == 0)
        ms = 1;
    SDEMMC_TRACE(0, "%s :  sec=%-5d size=%d(B) rate=%d(KB/s)",
                 __FUNCTION__, single_size, read_len, read_len / ms);
    return;
}

static int fs_safe_write(FILE *fd, char *buff, int32_t total_len, int32_t single_size)
{
    if (fd == NULL || buff == NULL) {
        return -1;
    }

    int32_t written_len = 0;
    while (total_len > 0) {
        int32_t tmp;
        int32_t res;
        tmp = total_len > single_size ? single_size : total_len;
        res = fwrite(buff + written_len, tmp, 1, fd);
        if (res != 1) {
            SDEMMC_TRACE(0, "%s fail: res=%d", __func__, res);
            break;
        }
        total_len -= tmp;
        written_len += tmp;
    }
    return written_len;
}

static void fs_rate_write(int32_t single_size)
{
    FILE *fd;
    fd = fopen(g_rate_name, "ab+");
    if (fd == NULL) {
        SDEMMC_TRACE(0, "%s fail: open err, path=%s", __func__, g_rate_name);
        return;
    }

    int32_t write_len = 0;
    int32_t total_len = FS_RATE_BASE_LENGTH;
    memset(g_buff, 'M', MAX_BUFF_SIZE);
    uint32_t tvbegin = hal_sys_timer_get();
    while (total_len > 0) {
        uint32_t tmp;
        uint32_t res;
        tmp = total_len > MAX_BUFF_SIZE ? MAX_BUFF_SIZE : total_len;
        res = fs_safe_write(fd, g_buff, tmp, single_size);
        if (res != tmp) {
            break;
        }
        total_len -= res;
        write_len += res;
    }
    fclose(fd);
    uint32_t tvdone = hal_sys_timer_get();
    if (total_len > 0) {
        SDEMMC_TRACE(0, "%s fail: not writing enough data", __func__);
        return;
    }
    uint32_t ms = TICKS_TO_MS(tvdone - tvbegin);
    if (ms == 0)
        ms = 1;
    SDEMMC_TRACE(0, "%s:  sec=%-5d size=%d(B) rate=%d(KB/s)",
                 __FUNCTION__, single_size, write_len, write_len / ms);
    return;
}

static void utest_fs_rate(int32_t sec_size)
{
    int32_t secs[] = {128, 512, 1024, 2048, 4096, 8192, 16384, 32768};
    if (sec_size < MIN_BUFF_SIZE || sec_size > MAX_BUFF_SIZE) {
        for (uint32_t i = 0; i < sizeof(secs) / sizeof(int); i++) {
            remove(g_rate_name);
            fs_rate_write(secs[i]);
            fs_rate_read(secs[i]);
        }
        return;
    }
    remove(g_rate_name);
    fs_rate_write(sec_size);
    fs_rate_read(sec_size);
    return;
}
#endif

void sdemmc_test(void)
{
    #ifdef EMMC_POWER_ON
    static const struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux_gpio[] = {
        {EMMC_POWER_3V3, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {EMMC_POWER_1V8, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
    };
    hal_iomux_init(pinmux_gpio, ARRAY_SIZE(pinmux_gpio));
    hal_gpio_pin_set_dir(EMMC_POWER_3V3, HAL_GPIO_DIR_OUT, 1);//output high
    hal_gpio_pin_set_dir(EMMC_POWER_1V8, HAL_GPIO_DIR_OUT, 1);//output high
    hal_gpio_pin_set((enum HAL_GPIO_PIN_T)(EMMC_POWER_3V3));
    hal_gpio_pin_set((enum HAL_GPIO_PIN_T)(EMMC_POWER_1V8));
#endif
#if defined(AOS_FS_ENABLE) && !defined(UTILS_ESHELL_EN)
    int err;
    SDEMMC_TRACE(0, "  ");
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_104M);
    SDEMMC_TRACE(1, "CPU frequency changed to: %uM", hal_sys_timer_calc_cpu_freq(5, 0) / 1000000);
    SDEMMC_TRACE(0, "---------emmc fs test started---------");

    err = mount("/dev/emmc0", "/data/emmc0",  "fatfs", 0, "forceformat");
    if (err) {
        SDEMMC_TRACE(0, "%s, mount fs failed %d", __func__, err);
        return;
    }
    memset(g_rate_name, 0x00, sizeof(g_rate_name));
    snprintf(g_rate_name, sizeof(g_rate_name) - 1, "/data/emmc0/%s", FS_RATE_FILE);
    utest_fs_rate(123);
    SDEMMC_TRACE(0, "---------emmc fs test ends---------");
    return;
#endif

    POSSIBLY_UNUSED int32_t run_cnt, run_ctrl;
    enum HAL_SDMMC_ERR ret;
    uint32_t sector_size = 0;
    uint32_t sector_count = 0;
    POSSIBLY_UNUSED struct HAL_SDMMC_CONFIG_T emmc_cfg;

    SDEMMC_TRACE(0, "  ");
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_208M);
    SDEMMC_TRACE(1, "CPU frequency changed to: %uM", hal_sys_timer_calc_cpu_freq(5, 0) / 1000000);
    SDEMMC_TRACE(0, "-------emmc test started,2022-06-01------");
#ifdef EMMC_POLLING_MODE
    SDEMMC_TRACE(0, "--------------polling mode---------------");
#else
    SDEMMC_TRACE(0, "----------------dma mode-----------------");
#endif
    SDEMMC_TRACE(0, "  ");

#ifndef FPGA
    /* Waiting for ldo, dc/dc to be stable after power on */
    osDelay(100);
#endif

#ifdef EMMC_RST_OPEN_CLOSE_TEST
#define EMMC_RST_PIN                    HAL_GPIO_PIN_P0_6
#define CLEAR_FLAG_ADDR                 0x20120000
#define PASS_RESULT_ADDR                0x20121000
#define ERROR_RESULT_ADDR               0x20122000
#define HW_WRITE_REG(_Reg,_Value)       (*((volatile uint32_t*)(_Reg)) = (uint32_t)(_Value))
#define HW_READ_REG(_Reg)               (*((volatile uint32_t*)(_Reg)))

    if (HW_READ_REG(CLEAR_FLAG_ADDR)) {
        HW_WRITE_REG(CLEAR_FLAG_ADDR, 0);
        HW_WRITE_REG(PASS_RESULT_ADDR, 0);
        HW_WRITE_REG(ERROR_RESULT_ADDR, 0);
    }

    uint32_t err_cnt;
    uint32_t pass_cnt;
    static const struct HAL_IOMUX_PIN_FUNCTION_MAP rst_gpio[] = {
        {EMMC_RST_PIN, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
    };
    hal_iomux_init(rst_gpio, ARRAY_SIZE(rst_gpio));
    hal_gpio_pin_set_dir(EMMC_RST_PIN, HAL_GPIO_DIR_OUT, 0);//output high

    //reset emmc
    hal_gpio_pin_clr(EMMC_RST_PIN);
    SDEMMC_TRACE(0, "---set emmc rst pin low");
    osDelay(5);
    hal_gpio_pin_set(EMMC_RST_PIN);
    SDEMMC_TRACE(0, "---set emmc rst pin high");
    SDEMMC_TRACE(0, "    ");
    osDelay(50);
#endif

#ifdef EMMC_RST_OPEN_CLOSE_RANDOM
    SDEMMC_TRACE(0, "  ");
    SDEMMC_TRACE(0, "===wdt reset/sdemmc success cnt:%d, err cnt:%d", HW_READ_REG(PASS_RESULT_ADDR), HW_READ_REG(ERROR_RESULT_ADDR));
    SDEMMC_TRACE(0, "  ");

    uint8_t random_time;
    //gen [5,22]ms random time
    srand(hal_sys_timer_get() * hal_fast_sys_timer_get());
    random_time = (uint8_t)(rand() % (22 - 5 + 1) + 5);
    SDEMMC_TRACE(0, "  ");
    SDEMMC_TRACE(0, "===random_time:%d", random_time);
    SDEMMC_TRACE(0, "  ");

    //wdt reset
    hal_wdt_set_timeout_ms(HAL_WDT_ID_0, random_time);
    hal_wdt_start(HAL_WDT_ID_0);
    SDEMMC_TRACE(0, "Wait DIG WDT reset start...");
    SDEMMC_TRACE(0, "  ");
#endif

    memset(&emmc_cfg, 0, sizeof(struct HAL_SDMMC_CONFIG_T));
#if defined(SDMMC_USE_SEM) && defined(RTOS)
    emmc_cfg.yield = true;
#else
    emmc_cfg.yield = false;
#endif
    emmc_cfg.ddr_mode = false;
    emmc_cfg.volt_switch = false;//useless
    emmc_cfg.bus_speed = 48 * 1000 * 1000;//sdr52
#ifdef EMMC_BUS_WIDTH_4_TEST
    emmc_cfg.bus_width = HAL_SDMMC_BUS_WIDTH_4;
#else
    emmc_cfg.bus_width = HAL_SDMMC_BUS_WIDTH_8;
#endif
    emmc_cfg.device_type = HAL_SDMMC_DEVICE_TYPE_EMMC;
    emmc_cfg.callback = &sdemmc_test_callback;

#ifdef EMMC_OPEN_WITH_INT_LOCK_TEST
#ifdef RTOS
#error "Interrupts cannot be turned off when RTOS=1"
#endif
    uint32_t lock = int_lock_global();
#endif

#ifdef EMMC_FORCE_CLOSE_TEST
    SDEMMC_TRACE(0, "===Call close before open, start");
    hal_sdmmc_force_close(SDEMMC_ID);
    SDEMMC_TRACE(0, "===Call close before open, end");
#endif

    ret = hal_sdmmc_open(SDEMMC_ID, &emmc_cfg);

#ifdef EMMC_OPEN_WITH_INT_LOCK_TEST
    int_unlock_global(lock);
#endif

#ifdef EMMC_RST_OPEN_CLOSE_TEST
    if (ret) {
        err_cnt = HW_READ_REG(ERROR_RESULT_ADDR);
        err_cnt++;
        HW_WRITE_REG(ERROR_RESULT_ADDR, err_cnt);

        SDEMMC_TRACE(0, "    ");
        SDEMMC_TRACE(0, "***hal_sdmmc_open fail:%d", ret);
        hal_sdmmc_close(SDEMMC_ID);
        SDEMMC_TRACE(0, "---hal_sdmmc_close");
        SDEMMC_TRACE(0, "    ");
    } else {
        pass_cnt = HW_READ_REG(PASS_RESULT_ADDR);
        pass_cnt++;
        HW_WRITE_REG(PASS_RESULT_ADDR, pass_cnt);

        SDEMMC_TRACE(0, "    ");
        SDEMMC_TRACE(0, "---hal_sdmmc_open success:%d", ret);
        SDEMMC_TRACE(0, "    ");
    }
#ifdef EMMC_RST_OPEN_CLOSE_RANDOM
    while (1) {
        SDEMMC_TRACE(0, "Wait DIG WDT reset end...");
    }
#endif
#else
    SDEMMC_TRACE(0, "-------sdmmc clk final speed:%d", hal_sdmmc_get_bus_speed(SDEMMC_ID));
    SDEMMC_TRACE(0, "  ");
    if (!ret) {
        hal_sdmmc_info(SDEMMC_ID, &sector_count, &sector_size);
        hal_sdmmc_dump(SDEMMC_ID);
        SDEMMC_TRACE(2, "------sdmmc info:sector_count=0x%X,sector_size=%d", sector_count, sector_size);
        SDEMMC_TRACE(0, "    ");
    } else {
        ASSERT(0, "%s:%d,sdmmc initialization failed,result=%d", __func__, __LINE__, ret);
    }
#endif

#ifdef EMMC_RST_OPEN_CLOSE_SIMPLE
    SDEMMC_TRACE(0, "  ");
    SDEMMC_TRACE(0, "===wdt reset/sdemmc success cnt:%d, err cnt:%d", HW_READ_REG(PASS_RESULT_ADDR), HW_READ_REG(ERROR_RESULT_ADDR));
    SDEMMC_TRACE(0, "  ");

    //wdt reset
    hal_wdt_set_timeout_ms(HAL_WDT_ID_0, 100);
    hal_wdt_start(HAL_WDT_ID_0);
    while (1) {
        SDEMMC_TRACE(0, "Wait DIG WDT reset...");
        osDelay(100);
    }
#endif

#ifdef EMMC_CACHE_ENABLE
    ret = hal_sdmmc_cache_control(SDEMMC_ID, 1);
    SDEMMC_TRACE(0, "emmc cache open result: %d(%s)", ret, ret ? "failed" : "success");
#endif

#ifdef EMMC_GET_EXT_CSD_TEST
    uint32_t ext_csd[512 / 4];

    ret = hal_sdmmc_get_card_ext_csd(SDEMMC_ID, (uint8_t *)ext_csd);
    if (ret) {
        SDEMMC_TRACE(0, "******get ext csd data err:%d, line:%d", ret, __LINE__);
    } else {
        sdemmc_dump32("ext_csd data", ext_csd, 512 / 4);
    }
#endif

#ifdef EMMC_PWROFF_NOTIFICATION_TEST
    enum HAL_SDMMC_POWER_OFF_MODE pon_mode;

    ret = hal_sdmmc_get_power_off_notification_status(SDEMMC_ID, &pon_mode);
    if (ret) {
        SDEMMC_TRACE(0, "******get power_off_notification status err:%d, line:%d", ret, __LINE__);
        return;
    }
    SDEMMC_TRACE(0, "------PWR_OFF_NOTIFY before:%d", pon_mode);

    if (pon_mode == HAL_SDMMC_NO_POWER_NOTIFICATION) {
        ret = hal_sdmmc_power_off_notification_enable(SDEMMC_ID);
        if (ret) {
            SDEMMC_TRACE(0, "******send power_off_notification err:%d, line:%d", ret, __LINE__);
            return;
        }
        ret = hal_sdmmc_get_power_off_notification_status(SDEMMC_ID, &pon_mode);
        if (ret) {
            SDEMMC_TRACE(0, "******get power_off_notification status err:%d, line:%d", ret, __LINE__);
            return;
        }
    }
    SDEMMC_TRACE(0, "------PWR_OFF_NOTIFY after :%d", pon_mode);

    if (pon_mode == HAL_SDMMC_POWERED_ON) {
        SDEMMC_TRACE(0, "++++++power off notification enable success");
    } else {
        SDEMMC_TRACE(0, "******power off notification enable fail");
        return;
    }

    ret = hal_sdmmc_power_off_notification_config(SDEMMC_ID, HAL_SDMMC_SLEEP_NOTIFICATION);
    if (ret) {
        SDEMMC_TRACE(0, "******power off notification config err:%d, line:%d", ret, __LINE__);
    } else {
        SDEMMC_TRACE(0, "++++++power off notification config successful, you can turn off the 3.3V and 1.8V power supply");
    }
    return;
#endif

    //card info read
    hal_sdmmc_get_card_info(SDEMMC_ID, &card_info);
    hal_sdmmc_dump_card_info(&card_info);
    hal_sdmmc_dump_reg(SDEMMC_ID);

#ifdef EMMC_ERAES_ENABLE
#define ERASE_START_BLK     (0)
#define ERASE_BLKS_CNT      (2)

    uint32_t blks;

    SDEMMC_TRACE(0, "  ");
    SDEMMC_TRACE(0, "--------------------emmc earse test--------------------");

    //data buffer populates random
    rand_generator((uint32_t *)txbuf, BUF_SIZE / sizeof(uint32_t));

    //write
    SDEMMC_TRACE(0, "----------write %d bytes", sizeof(txbuf));
#if defined(SDMMC_TX_RX_USE_CALLBACK)
    sdmmc_txrx_done = 0;
#endif
#ifdef EMMC_POLLING_MODE
    blks = hal_sdmmc_write_blocks_polling(SDEMMC_ID, ERASE_START_BLK, BUF_SIZE / sector_size, (uint8_t *)txbuf);
#else
    blks = hal_sdmmc_write_blocks(SDEMMC_ID, ERASE_START_BLK, BUF_SIZE / sector_size, (uint8_t *)txbuf);
#endif
#if defined(SDMMC_TX_RX_USE_CALLBACK)
    while (!sdmmc_txrx_done);
    SDEMMC_TRACE_CB(2, "------write done");
#endif

    //read
    SDEMMC_TRACE(0, "----------read %d bytes", sizeof(rxbuf));
#if defined(SDMMC_TX_RX_USE_CALLBACK)
    sdmmc_txrx_done = 0;
#endif
#ifdef EMMC_POLLING_MODE
    blks = hal_sdmmc_read_blocks_polling(SDEMMC_ID, ERASE_START_BLK, BUF_SIZE / sector_size, (uint8_t *)rxbuf);
#else
    blks = hal_sdmmc_read_blocks(SDEMMC_ID, ERASE_START_BLK, BUF_SIZE / sector_size, (uint8_t *)rxbuf);
#endif
#if defined(SDMMC_TX_RX_USE_CALLBACK)
    while (!sdmmc_txrx_done);
    SDEMMC_TRACE_CB(2, "------read done");
#endif
    sdemmc_dump32("emmc data", (uint32_t *)((uint32_t)rxbuf + sector_size * 0), sector_size);
    sdemmc_dump32("emmc data", (uint32_t *)((uint32_t)rxbuf + sector_size * 1), sector_size);
    sdemmc_dump32("emmc data", (uint32_t *)((uint32_t)rxbuf + sector_size * 2), sector_size);
    sdemmc_dump32("emmc data", (uint32_t *)((uint32_t)rxbuf + sector_size * 3), sector_size);

    //erase
    SDEMMC_TRACE(0, "----------erase blk: %d ~ %d", ERASE_START_BLK, ERASE_START_BLK + ERASE_BLKS_CNT - 1);
    blks = hal_sdmmc_erase_dma(SDEMMC_ID, ERASE_START_BLK, ERASE_BLKS_CNT);
    SDEMMC_TRACE(0, "erase blocks cnt:%d", blks);

    //read
    SDEMMC_TRACE(0, "----------read %d bytes", sizeof(rxbuf));
    memset((void *)rxbuf, 0, sizeof(rxbuf));
#if defined(SDMMC_TX_RX_USE_CALLBACK)
    sdmmc_txrx_done = 0;
#endif
#ifdef EMMC_POLLING_MODE
    blks = hal_sdmmc_read_blocks_polling(SDEMMC_ID, ERASE_START_BLK, BUF_SIZE / sector_size, (uint8_t *)rxbuf);
#else
    blks = hal_sdmmc_read_blocks(SDEMMC_ID, ERASE_START_BLK, BUF_SIZE / sector_size, (uint8_t *)rxbuf);
#endif
#if defined(SDMMC_TX_RX_USE_CALLBACK)
    while (!sdmmc_txrx_done);
    SDEMMC_TRACE_CB(2, "------read done");
#endif
    sdemmc_dump32("emmc data", (uint32_t *)((uint32_t)rxbuf + sector_size * 0), sector_size);
    sdemmc_dump32("emmc data", (uint32_t *)((uint32_t)rxbuf + sector_size * 1), sector_size);
    sdemmc_dump32("emmc data", (uint32_t *)((uint32_t)rxbuf + sector_size * 2), sector_size);
    sdemmc_dump32("emmc data", (uint32_t *)((uint32_t)rxbuf + sector_size * 3), sector_size);

    SDEMMC_TRACE(0, "----------erase test successful");
    SDEMMC_TRACE(0, " ");
#endif

    run_cnt = 0;
    run_ctrl = 1;
    srand(hal_sys_timer_get() * hal_fast_sys_timer_get());
    while (run_ctrl--) {
        emmc_read_write_test(sector_size, sector_count);
#if defined(EMMC_STRESS_TEST) || defined(EMMC_OPEN_CLOSE_TEST) || defined(EMMC_FORCE_CLOSE_TEST)
        SDEMMC_TRACE(0, "----------------------------------------------->run_cnt:%d", ++run_cnt);
#endif

#ifdef EMMC_STRESS_TEST
        run_ctrl = 1;
#endif

#ifdef EMMC_OPEN_CLOSE_TEST
        hal_sdmmc_close(SDEMMC_ID);
        SDEMMC_TRACE(0, "  ");
        SDEMMC_TRACE(0, "------------------------------------");
        SDEMMC_TRACE(0, "---hal_sdmmc_close");
        SDEMMC_TRACE(0, "------------------------------------");
        ret = hal_sdmmc_open(SDEMMC_ID, &emmc_cfg);
        SDEMMC_TRACE(0, "------------------------------------");
        if (ret) {
            SDEMMC_TRACE(0, "***hal_sdmmc_open fail:%d", ret);
        } else {
            run_ctrl = 1;
            SDEMMC_TRACE(0, "---hal_sdmmc_open success:%d", ret);
        }
        SDEMMC_TRACE(0, "------------------------------------");
        SDEMMC_TRACE(0, "  ");
#endif

#ifdef EMMC_FORCE_CLOSE_TEST
        hal_sdmmc_force_close(SDEMMC_ID);
        SDEMMC_TRACE(0, "  ");
        SDEMMC_TRACE(0, "------------------------------------");
        SDEMMC_TRACE(0, "---hal_sdmmc_force_close, call 1");
        SDEMMC_TRACE(0, "------------------------------------");
        hal_sdmmc_force_close(SDEMMC_ID);
        SDEMMC_TRACE(0, "------------------------------------");
        SDEMMC_TRACE(0, "---hal_sdmmc_force_close, call 2");
        SDEMMC_TRACE(0, "------------------------------------");
        ret = hal_sdmmc_open(SDEMMC_ID, &emmc_cfg);
        SDEMMC_TRACE(0, "------------------------------------");
        if (ret) {
            SDEMMC_TRACE(0, "***hal_sdmmc_open fail:%d", ret);
        } else {
            run_ctrl = 1;
            SDEMMC_TRACE(0, "---hal_sdmmc_open success:%d", ret);
        }
        SDEMMC_TRACE(0, "------------------------------------");
        SDEMMC_TRACE(0, "  ");
#endif
        if (run_ctrl) {
            osDelay(500);
        }
    }
    SDEMMC_TRACE(0, "---------emmc read and write test ends---------");
    // hal_sdmmc_dump_run_record(SDEMMC_ID);
}

#ifdef UTILS_ESHELL_EN
static void watch_sdemmc_test_event(int argc, char *argv[])
{
    eshell_putstring_nl("Auto Test command:sdemmc_test\n");
    sdemmc_test();
}

ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_AUTO_TEST, "sdemmc_test", "sdemmc_test", watch_sdemmc_test_event);
#endif

#endif
