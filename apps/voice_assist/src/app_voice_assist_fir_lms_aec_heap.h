#ifndef APP_VOICE_ASSIST_FIR_LMS_AEC_HEAP_H
#define APP_VOICE_ASSIST_FIR_LMS_AEC_HEAP_H

#include <stdint.h>
#include "custom_allocator.h"

int32_t anc_aec_heap_init(void);

int32_t anc_aec_heap_deinit(void);

custom_allocator *anc_aec_allocator(void);

#endif
