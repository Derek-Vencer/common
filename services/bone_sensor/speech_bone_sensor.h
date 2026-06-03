/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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
#ifndef __SPEECH_BONE_SENSOR_H__
#define __SPEECH_BONE_SENSOR_H__

#include "hal_aud.h"

typedef enum {
    SPEECH_BS_CHANNEL_X = 0,
    SPEECH_BS_CHANNEL_Y,
    SPEECH_BS_CHANNEL_Z,
    SPEECH_BS_CHANNEL_QTY
} speech_bs_channel_t;

#ifdef __cplusplus
extern "C" {
#endif

int32_t speech_bone_sensor_open(uint32_t sample_rate, uint32_t frame_len);
int32_t speech_bone_sensor_close(void);

int32_t speech_bone_sensor_start(void);
int32_t speech_bone_sensor_stop(void);

int32_t speech_bone_sensor_get_data(void *_pcm_buf, uint32_t pcm_len, speech_bs_channel_t ch, uint32_t bits);

#ifdef __cplusplus
}
#endif

#endif
