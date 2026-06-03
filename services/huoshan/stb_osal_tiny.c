
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
#include "stb_osal_tiny.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "heap_api.h"

#define STB_HEAP_SIZE    (4096)
static uint8_t stbHeapBuf[STB_HEAP_SIZE] __attribute__((aligned(4)));
// static uint8_t *stbHeapBuf = NULL;
static heap_handle_t stbHeap;

int rand_generator(uint8_t *dest, unsigned size)
{
    while (size--) {
        *dest++ = (uint8_t)rand();
    }
    return 1;
}

void stb_mem_init(size_t size)
{
    stbHeap = heap_register(stbHeapBuf, STB_HEAP_SIZE);
    return;
}

void *stb_malloc(size_t size)
{
    void *data = NULL;
    // if (!stbHeapBuf)
    // {
    //     syspool_get_buff((uint8_t **)&stbHeapBuf, STB_HEAP_SIZE);
    //     stbHeap = heap_register(stbHeapBuf, STB_HEAP_SIZE);
    // }
    data = heap_malloc(stbHeap, size);
    return data;
}


void stb_free(void *mem)
{
    if (mem)
    {
        heap_free(stbHeap, mem);
    }
}

uint32_t stb_now_ms(void)
{
    uint32_t currentTime = hal_sys_timer_get();
    HUOSHAN_TRACE(0, "%s currentTime is %d", __func__, currentTime);
    return TICKS_TO_MS(currentTime);
}

void stb_rand_init(void)
{
    return;
}

uint32_t stb_rand(void)
{
    uint32_t randValue;
    rand_generator((uint8_t *)&randValue, 4);
    return randValue;
}

void stb_msleep(uint32_t ms)
{
//
}
