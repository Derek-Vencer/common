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
#ifndef __STEREO_PROMPT_H__
#define __STEREO_PROMPT_H__
#include "stereo_led.h"

int app_voice_report_handler(APP_STATUS_INDICATION_T status, uint8_t device_id, uint8_t isMerging);
int app_voice_report(APP_STATUS_INDICATION_T status, uint8_t device_id);
void app_ui_max_vol_warning(void);

void app_ui_mute_vol_warning(void);

void app_ui_anc_tone(void);

#endif