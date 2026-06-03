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
#ifndef __STEREO_LED_H__
#define __STEREO_LED_H__

#ifdef RTOS
#include "cmsis_os.h"
#endif

typedef enum APP_STATUS_INDICATION_T {
    APP_STATUS_INDICATION_POWER_ON =0,
    APP_STATUS_INDICATION_POWER_OFF,
    APP_STATUS_INDICATION_MAX_VOLUME,
    APP_STATUS_INDICATION_MUTE_VOLUME,
    APP_STATUS_INDICATION_INCOMING_CALL,
    APP_STATUS_INDICATION_REJECT_INCOMING_CALL, //5
    APP_STATUS_INDICATION_END_CALL,
    APP_STATUS_INDICATION_Activate_Voice_Assistant,
    APP_STATUS_INDICATION_MUSIC_MODE,
    APP_STATUS_INDICATION_GAME_MODE,
    APP_STATUS_INDICATION_EQ_ON, //10
    APP_STATUS_INDICATION_EQ_OFF,
    APP_STATUS_INDICATION_CLEAR_DEVICE_LIST,
    APP_STATUS_INDICATION_PAIRING_MODE,
    APP_STATUS_INDICATION_PAIRING_SUCCESSFUL,
    APP_STATUS_INDICATION_PAIRING_NOT_SUCCESSFUL, //15
    APP_STATUS_INDICATION_PAIRING_CONNECTED,
    APP_STATUS_INDICATION_PAIRING_CONNECTED_B,
    APP_STATUS_INDICATION_PAIRING_DISCONNECTED,
    APP_STATUS_INDICATION_PAIRING_LINK_LOSS,
    APP_STATUS_INDICATION_CHARG, //20
    APP_STATUS_INDICATION_CHARGED_FULLY,
    APP_STATUS_INDICATION_BATTERY_LOW,
    APP_STATUS_INDICATION_CALL_MUSIC,
    APP_STATUS_INDICATION_ANC_OFF,
    APP_STATUS_INDICATION_ANC_AMBIENT, //25
    APP_STATUS_INDICATION_ANC_ON,

    APP_STATUS_INDICATION_POWEROFF,
    APP_STATUS_INDICATION_PAGESCAN,
    APP_STATUS_INDICATION_BOTHSCAN,
    APP_STATUS_INDICATION_TILE_FIND,
    APP_STATUS_INDICATION_FIND_MY_BUDS,
    APP_STATUS_INDICATION_CHARGING,
    APP_STATUS_INDICATION_FULLCHARGE,
    APP_STATUS_INDICATION_TESTMODE,
    APP_STATUS_INDICATION_TESTMODE1,
    APP_STATUS_INDICATION_NUM,

    APP_STATUS_INDICATION_NUM_MAX
}APP_STATUS_INDICATION_T;


#ifdef __cplusplus
extern "C" {
#endif

uint8_t app_ui_status_indication_init(void);
int app_status_indication_filter_set(APP_STATUS_INDICATION_T status);
APP_STATUS_INDICATION_T app_last_status_indication_get(void);
APP_STATUS_INDICATION_T app_status_indication_get(void);
int app_status_indication_set(APP_STATUS_INDICATION_T status);


#ifdef __cplusplus
}
#endif

#endif

