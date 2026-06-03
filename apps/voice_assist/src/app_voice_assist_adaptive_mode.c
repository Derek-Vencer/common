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
#include "hal_trace.h"
#include "anc_assist.h"
#include "app_anc_assist.h"
#include "app_anc.h"
#include "app_voice_assist_adaptive_mode.h"
#include "ae_math.h"
#include "app_anc_utils.h"

static int32_t _voice_assist_adaptive_mode_callback(void *buf, uint32_t len, void *other);
static bool adaptive_gain_enable = false;

static void adaptive_mode_set_tt_ff_gain(float tt_gain_lin, float ff_gain_lin)
{
    VOICE_ASSIST_TRACE(0, "[%s] tt_gain(x100): %d, ff_gain(x100): %d", __func__, (int)(tt_gain_lin * 100), (int)(ff_gain_lin * 100));

    // TODO: tws sync gain
    app_anc_set_global_gain_f32(ANC_FEEDFORWARD, ff_gain_lin, ff_gain_lin);
    app_anc_set_global_gain_f32(ANC_TALKTHRU, tt_gain_lin, tt_gain_lin);
}

int32_t app_voice_assist_adaptive_mode_init(void)
{
    app_anc_assist_register(ANC_ASSIST_USER_ADAPTIVE_MODE, _voice_assist_adaptive_mode_callback);

    return 0;
}

int32_t app_voice_assist_adaptive_mode_open(void)
{
    app_anc_assist_open(ANC_ASSIST_USER_ADAPTIVE_MODE);
    adaptive_gain_enable = true;

    return 0;
}

int32_t app_voice_assist_adaptive_mode_close(void)
{
    adaptive_gain_enable = false;
    app_anc_assist_close(ANC_ASSIST_USER_ADAPTIVE_MODE);

    adaptive_mode_set_tt_ff_gain(1.0, 1.0); // reset gain
    return 0;
}

int32_t app_voice_assist_adaptive_mode_set_gain_db(float gain_db)
{
    if(gain_db > 0){
        VOICE_ASSIST_TRACE(0, "[%s] gain_db(x10): (%d) should <= 0", __func__, (int)(gain_db * 10));
        return -1;
    }

    VOICE_ASSIST_TRACE(0, "[%s] fixed gain_db (x10): %d", __func__, (int)(gain_db * 10));
    adaptive_gain_enable = false;

    float gain_lin = DB2LIN(gain_db);

    adaptive_mode_set_tt_ff_gain(gain_lin, 1.0 - gain_lin);

    return 0;
}

static int32_t _voice_assist_adaptive_mode_callback(void *buf, uint32_t len, void *other)
{
    assist_adaptive_mode_res_t *res = (assist_adaptive_mode_res_t *)buf;

    if(ASSIST_ADAPTIVE_MODE_STATUS_CHANGED == res->status && adaptive_gain_enable){
        float gain_lin = DB2LIN(res->gain_db);
        adaptive_mode_set_tt_ff_gain(gain_lin, 1.0 - gain_lin);
    }

    return 0;
}

