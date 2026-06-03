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
#ifndef __VPU_STREAM_H__
#define __VPU_STREAM_H__

#include "hal_aud.h"

#ifdef __cplusplus
extern "C" {
#endif

int32_t vpu_stream_open(uint32_t sample_rate, uint32_t frame_len);
int32_t vpu_stream_close(void);
int32_t vpu_stream_start(void);
int32_t vpu_stream_stop(void);

void vpu_stream_get_data(void *_pcm_buf, uint32_t pcm_len, uint32_t ch, uint32_t bits);

#ifdef __cplusplus
}
#endif

#endif
