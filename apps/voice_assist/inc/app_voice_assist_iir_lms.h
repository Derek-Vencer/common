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
#ifndef __APP_VOICE_ASSIST_IIR_LMS_H__
#define __APP_VOICE_ASSIST_IIR_LMS_H__

#define FIR_ANC_SYS_FREQ_LOG

#include "plat_types.h"
#include "app_anc.h"
#include "app_anc_assist.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    anc_assist_user_t user;
    uint32_t len;
    uint32_t sub_cmd;
    uint8_t buf[256];
} app_voice_assist_core_interact_data_t;

int32_t app_voice_assist_iir_lms_init(void);
int32_t app_voice_assist_iir_lms_enable_iir(bool enable);
int32_t app_voice_assist_iir_lms_set_iir_status(bool enable);
int32_t app_voice_assist_iir_lms_set_anc_mode(app_anc_mode_t mode);
bool app_voice_assist_iir_lms_get_iir_status(void);

int32_t app_voice_assist_iir_lms_set_fb_gain(float gain);
int32_t app_voice_assist_fir_lms_set_tt_ns_cfg(void *cfg);

int32_t app_voice_assist_iir_lms_start_fir(void);
int32_t app_voice_assist_iir_lms_stop_fir(void);

int voice_assist_iir_lms_open(void *share_list_buf);
int voice_assist_iir_lms_close(void);
int voice_assist_iir_lms_set_cfg(uint32_t ctrl, uint8_t *tgt_cfg, uint32_t ptr_len);

void iir_tool_iir_anc_open(int32_t current_mode);
void iir_tool_iir_anc_close(void);
#ifdef __cplusplus
}
#endif

#endif