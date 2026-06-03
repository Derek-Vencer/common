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
#ifdef BESUI_STEREO_EN
#include "cmsis_os.h"
#include "stdbool.h"
#include "hal_trace.h"
#include "app_pwl.h"
#include "stereo_prompt.h"
#include "string.h"
#include "app_media_player.h"
#include "besui_common.h"
#include "app_anc.h"

static APP_STATUS_INDICATION_T app_voice_status = APP_STATUS_INDICATION_NUM_MAX;
int app_voice_report_handler(APP_STATUS_INDICATION_T status, uint8_t device_id, uint8_t isMerging)
{
    BESUI_TRACE(0,"%s,%d",__func__,status);

    static uint8_t tone_time = 0;
    if (((hal_sys_timer_get() - tone_time < 1200)) && ((hal_sys_timer_get() - tone_time) > 0))
    {
        osDelay(hal_sys_timer_get() - tone_time);
    }
    tone_time = hal_sys_timer_get();
    BESUI_TRACE(0, "[UIPROMPT]%s, tone_time is %d", __func__, tone_time);

    if ((APP_STATUS_INDICATION_PAIRING_MODE == app_voice_status) && (APP_STATUS_INDICATION_PAIRING_MODE == status) )
    {
        BESUI_TRACE(1, "%s APP_STATUS_INDICATION_PAIRING_MODE is exist", __func__);
        return 0;
    }
    app_voice_status = status;

    uint16_t id = MAX_RECORD_NUM;
    switch (status) {
        case APP_STATUS_INDICATION_POWER_ON:
            id = AUD_ID_POWER_ON;
            break;
        case APP_STATUS_INDICATION_POWER_OFF:
            id = AUD_ID_POWER_OFF;
            break;
        case APP_STATUS_INDICATION_MAX_VOLUME:
            id = AUD_ID_MAX_VOLUME;
            break;
        case APP_STATUS_INDICATION_INCOMING_CALL:
            id = AUD_ID_BT_CALL_INCOMING_CALL;
            break;
        case APP_STATUS_INDICATION_REJECT_INCOMING_CALL:
            osDelay(100);
            id = AUD_ID_BT_CALL_REFUSE;
            break;
        case APP_STATUS_INDICATION_END_CALL:
            id = AUD_ID_BT_CALL_OVER;
            break;
        case APP_STATUS_INDICATION_Activate_Voice_Assistant:
            break;
        case APP_STATUS_INDICATION_MUSIC_MODE:
            id = AUD_ID_MUSIC_MODE;
            break;
        case APP_STATUS_INDICATION_GAME_MODE:
            id = AUD_ID_GAME_MODE;
            break;
        case APP_STATUS_INDICATION_EQ_ON:
            id = AUD_ID_EQ_ON;
            break;
        case APP_STATUS_INDICATION_EQ_OFF:
            id = AUD_ID_EQ_OFF;
            break;
        case APP_STATUS_INDICATION_BATTERY_LOW:
            id = AUD_ID_BT_CHARGE_PLEASE;
            break;
        case APP_STATUS_INDICATION_PAIRING_MODE:
            id = AUD_ID_BT_PAIRING;
            break;
        case APP_STATUS_INDICATION_PAIRING_SUCCESSFUL:
            id = AUD_ID_BT_PAIRING_SUC;
            break;
        case APP_STATUS_INDICATION_PAIRING_DISCONNECTED:
            id = AUD_ID_BT_DIS_CONNECT;
            break;
        case APP_STATUS_INDICATION_PAIRING_CONNECTED:
            id = AUD_ID_BT_PAIRING_SUC;
            break;
        case APP_STATUS_INDICATION_PAIRING_CONNECTED_B:
            id = AUD_ID_BT_PAIRING_SUC;
            break;
        case APP_STATUS_INDICATION_ANC_OFF:
            id = AUD_ID_ANC_MUTE;
            break;
        case APP_STATUS_INDICATION_ANC_AMBIENT:
            id = AUD_ID_ANC_MODE1;
            break;
        case APP_STATUS_INDICATION_ANC_ON:
            id = AUD_ID_ANC_MODE0;
            break;
        default:
            break;
        }

    media_PlayAudio((AUD_ID_ENUM)id, device_id);

    return 0;
}

int app_voice_report(APP_STATUS_INDICATION_T status, uint8_t device_id)
{
    return app_voice_report_handler(status, device_id, true);
}

void app_ui_max_vol_warning(void)
{
    app_voice_report(APP_STATUS_INDICATION_MAX_VOLUME, 0);
}

void app_ui_mute_vol_warning(void)
{
    app_voice_report(APP_STATUS_INDICATION_MUTE_VOLUME, 0);
}


app_anc_mode_t app_anc_get_curr_mode(void);
void app_ui_anc_tone(void)
{
#ifdef ANC_APP
    uint8 ancmode = (app_anc_get_curr_mode() + 1) % APP_ANC_MODE_QTY;
    BESUI_TRACE(1,"app_anc_get_curr_mode is %d", ancmode);
    if(ancmode == APP_ANC_MODE1)
    {
        app_voice_report(APP_STATUS_INDICATION_ANC_ON,0);
    }
    else if (ancmode == APP_ANC_MODE2)
    {
        app_voice_report(APP_STATUS_INDICATION_ANC_AMBIENT,0);
    }
    else if (ancmode == APP_ANC_MODE_OFF)
    {
        app_voice_report(APP_STATUS_INDICATION_ANC_OFF,0);
    }
#endif
}

#endif //BESUI_STEREO_EN