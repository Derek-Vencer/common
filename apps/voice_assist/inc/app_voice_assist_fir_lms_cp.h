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
#ifndef __VOICE_ASSIST_FIR_LMS_CP_H__
#define __VOICE_ASSIST_FIR_LMS_CP_H__

#include "event_detection.h"

#ifdef __cplusplus
extern "C" {
#endif

int fir_lms_cp_init(void);
int fir_lms_cp_deinit(void);
void app_voice_assist_fir_lms_cp_process(process_frame_data_t *data_buf, EventController* event_control);

void flr_lms_cp_switch_event_detection_core(bool is_cp);
// void set_flr_lms_cp_status(bool status);

void app_voice_assist_fir_lms_cp_send_stl_to_bth(void *ptr, uint16_t ptr_len);

#ifdef __cplusplus
    }
#endif

#endif//__VOICE_ASSIST_FIR_LMS_CP_H__
