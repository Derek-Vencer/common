#ifndef ADAPTIVE_VOLUME_H
#define ADAPTIVE_VOLUME_H

#include <stdint.h>
#include "custom_allocator.h"

struct AdaptiveVolumeState_
{
    custom_allocator *allocator;
    bool    debug;
    float   CT;
    float   CS;
    float   WT;
    float   WS;
    float   ET;
    float   ES;
    float   makeup_gain;
    float   at;
    float   rt;
    float   G_S;
    float   smooth_factor;
    float   volume_gain;

    bool curr_adaptive_volume_status;
    bool next_adaptive_volume_status;
    bool adaptive_volume_gain_update_flag;
    float adaptive_volume_gain;

};

typedef struct {
    bool    debug;
    float   CT;
    float   CS;
    float   WT;
    float   WS;
    float   ET;
    float   ES;
    float   attack_time;
    float   release_time;
    float   makeup_gain;
    float   smooth_factor;
} AdaptiveVolumeConfig;

typedef struct AdaptiveVolumeState_ AdaptiveVolumeState;

#ifdef __cplusplus
extern "C" {
#endif

AdaptiveVolumeState *adaptive_volume_create(int sample_rate, int frame_size, int sample_bit, int ch_num, const AdaptiveVolumeConfig *config, custom_allocator *allocator);

void adaptive_volume_switch(AdaptiveVolumeState *st);

float adaptive_volume_gain_estimate(AdaptiveVolumeState *st, float ff_noise_energy_db, float ref_energy_db);

void adaptive_volume_destroy(AdaptiveVolumeState* st);

void adaptive_volume_set_config(AdaptiveVolumeState *st, const AdaptiveVolumeConfig *cfg);
#ifdef __cplusplus
}
#endif

#endif