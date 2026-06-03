#ifndef __ADJ_EQ_REV_H__
#define __ADJ_EQ_REV_H__

#include <stdbool.h>
#include <stdint.h>
#include "iirfilt.h"
#include "custom_allocator.h"
#ifdef __cplusplus
extern "C" {
#endif

#define MAX_ADJ_EQ_NUM (5)
#define MAX_ADJ_EQ_CHANNEL_NUM (2)

typedef struct {
    enum IIR_BIQUARD_TYPE type;
    float freq;
    float Q;
    float gain_cali;
    float min_gain;
    float max_gain;
} adj_eq_cfg_t;

typedef struct 
{
    int debug_en;
    float forget_factor;
    int32_t adj_eq_rev_num;
    adj_eq_cfg_t adj_eq_rev_cfg[MAX_ADJ_EQ_NUM];
    float normalsz_lab[33];
}AdjEQConfig;

struct  AdjEQState_;
typedef struct AdjEQState_ AdjEQState;
AdjEQState *adj_eq_rev_create(int ch_num, uint32_t echo_frame_len, uint32_t echo_sample_rate, const AdjEQConfig *cfg, custom_allocator *allocator);
void adj_eq_rev_destroy(AdjEQState *st);
void adj_eq_rev_filter_estimate(AdjEQState *st, float **fb_buf, float **ref_buf, uint32_t frame_len);
void adj_eq_rev_reset(AdjEQState *st);
void adj_eq_rev_set_cfg(AdjEQState *st,const AdjEQConfig *cfg);

#ifdef __cplusplus
}
#endif

#endif
