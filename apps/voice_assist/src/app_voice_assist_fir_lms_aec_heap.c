#include "anc_fir_lms_common.h"

#define EXT_HEAP_ENABLE_MULTI_HEAP
#define HEAP_BUFF_SIZE (28 * 1024 * FIR_CHANNEL_NUM)
#include "ext_heap.h"

static uint8_t g_heap_buff[HEAP_BUFF_SIZE];

int32_t anc_aec_heap_init(void)
{
    return ext_heap_init(g_heap_buff);
}

int32_t anc_aec_heap_deinit(void)
{
    return ext_heap_deinit();
}

custom_allocator *anc_aec_allocator(void)
{
    return &ext_allocator;
}
