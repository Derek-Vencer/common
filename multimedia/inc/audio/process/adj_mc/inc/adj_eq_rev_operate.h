#ifndef __ADJ_EQ_REV_OPERATE_H__
#define __ADJ_EQ_REV_OPERATE_H__

#include <stdbool.h>
#include <stdint.h>
#include "custom_allocator.h"
#ifdef __cplusplus
extern "C" {
#endif

struct  AdjEqOperateState_;
typedef struct AdjEqOperateState_ AdjEqOperateState;

AdjEqOperateState *adj_eq_rev_operate_create(int sample_rate, int frame_size, int sample_bit, uint8_t ch_num,const AdjEQConfig *cfg, custom_allocator *allocator);
void adj_eq_rev_operate_destroy(AdjEqOperateState *st);
void adj_eq_rev_operate_process(AdjEqOperateState *st, uint8_t *buf);
void adj_eq_rev_operate_switch(AdjEqOperateState *st, bool val);
void adj_eq_rev_operate_reset(AdjEqOperateState *st);
#ifdef __cplusplus
}
#endif

#endif
