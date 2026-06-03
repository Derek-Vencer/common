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
#ifndef __STEREOUI_H__
#define __STEREOUI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

#define CFG_SW_KEY_LLPRESS_THRESH_MS            5000
#define CFG_SW_KEY_VLPRESS_THRESH_MS            3000
#define CFG_SW_KEY_LPRESS_THRESH_MS             2000
#define CFG_SW_KEY_REPEAT_THRESH_MS             600

#define CFG_SW_KEY_INIT_LPRESS_THRESH_MS        4000
#define CFG_SW_KEY_INIT_LLPRESS_THRESH_MS       10000

#define POWER_ON_HAL_KEY_TIME                   100   // 100*20ms

//-------------------------------------------------------------------------------------
#define ONLY_OPEN_MASTER_MIC                    0x00
#define ONLY_OPEN_SLAVE_MIC                     0x01
#define OPEN_MASTER_SLAVE_MIC                   0x02
#define CLOSE_MASTER_SLAVE_MIC                  0x03

#if defined(ANC_APP)
#define ANC_TEST_FF                             0x01
#define ANC_TEST_FB                             0x02
#endif

#define SPEECH_TEST_MIC0                        0x01
#define SPEECH_TEST_MIC1                        0x02

bool app_get_mic_hfp_enc_onoff(void);
uint8_t app_get_mic_hfp_enc_test_type(void);
void app_common_mic_hfp_enc_control(bool enc, uint8_t mic);

void speech_mic_set_onoff(uint8_t sta);
uint8_t speech_mic_get_onoff(void);
//-------------------------------------------------------------------------------------

enum HALL_CHARGER_T
{
    HALL_STA_CLOSE = 0,
    HALL_STA_OPEN,
    HALL_STA_QTY,
};

#ifdef DC_DETECT_EN
enum DC_CHARGER_T
{
    DC_OUT = 0,
    DC_IN,
    DC_STA_QTY,
};
#endif

typedef enum APP_STATUS_TYPE_T {
    APP_STATUS_TYPE_POWER_ON = 0,
	APP_STATUS_TYPE_POWER_OFF,
    APP_STATUS_TYPE_CHARGE,
	APP_STATUS_TYPE_IDLE,
	APP_STATUS_TYPE_RECONNECT_A,
	APP_STATUS_TYPE_RECONNECT_B, //5
    APP_STATUS_TYPE_PAIR_A,
    APP_STATUS_TYPE_PAIR_B,
    APP_STATUS_TYPE_CONNECTED_A,
    APP_STATUS_TYPE_CONNECTED_B,
    APP_STATUS_TYPE_MUSIC_A, //10
    APP_STATUS_TYPE_MUSIC_B,
    APP_STATUS_TYPE_CALL_A,
    APP_STATUS_TYPE_CALL_B,
    APP_STATUS_TYPE_DISCONNECTED_A,
    APP_STATUS_TYPE_DISCONNECTED_B, //15
    APP_STATUS_TYPE_CLEAR_DEVICE_LIST,
    APP_STATUS_TYPE_CONNECTED_AFTER_MUSIC_CALL,
    APP_STATUS_TYPE_INCOMMING_CALL,
    APP_STATUS_TYPE_CALL_REJECT_PROMPT,
    APP_STATUS_TYPE_CALL_HUNGUP_PROMPT,    
	APP_STATUS_TYPE_NONE,
} APP_STATUS_TYPE_T;


typedef enum app_ui_bt_audio_event_t {
    APP_UI_BT_AUDIO_EVENT_AVRCP_PLAY_STATUS_CHANGED                  = 0x1,
    APP_UI_BT_AUDIO_EVENT_AVRCP_PLAY_STATUS_NOTIFY_RSP               = 0x2,
    APP_UI_BT_AUDIO_EVENT_AVRCP_PLAY_STATUS                          = 0x3,
    APP_UI_BT_AUDIO_EVENT_AVRCP_PLAY_STATUS_MOCK                     = 0x4,
    APP_UI_BT_AUDIO_EVENT_AVRCP_IS_REALLY_PAUSED                     = 0x5,
    APP_UI_BT_AUDIO_EVENT_AVRCP_MEDIA_PLAYING                        = 0x6,
    APP_UI_BT_AUDIO_EVENT_AVRCP_MEDIA_PAUSED                         = 0x7,
    APP_UI_BT_AUDIO_EVENT_A2DP_STREAM_OPEN                           = 0x8,
    APP_UI_BT_AUDIO_EVENT_A2DP_STREAM_CLOSE                          = 0x9,
    APP_UI_BT_AUDIO_EVENT_A2DP_STREAM_START                          = 0xA,
    APP_UI_BT_AUDIO_EVENT_A2DP_STREAM_MOCK_START                     = 0xB,
    APP_UI_BT_AUDIO_EVENT_A2DP_STREAM_SUSPEND                        = 0xC,
    APP_UI_BT_AUDIO_EVENT_A2DP_STREAM_SWITCH                         = 0xD,
    APP_UI_BT_AUDIO_EVENT_A2DP_STREAM_RESELECT                       = 0xE,
    APP_UI_BT_AUDIO_EVENT_AVRCP_CONNECTED                            = 0xF,
    APP_UI_BT_AUDIO_EVENT_HFP_SERVICE_CONNECTED                      = 0x10,
    APP_UI_BT_AUDIO_EVENT_HFP_SERVICE_MOCK_CONNECTED                 = 0x11,
    APP_UI_BT_AUDIO_EVENT_HFP_SERVICE_DISCONNECTED                   = 0x12,
    APP_UI_BT_AUDIO_EVENT_HFP_SCO_CONNECT_REQ                        = 0x13,
    APP_UI_BT_AUDIO_EVENT_HFP_SCO_CONNECTED                          = 0x14,
    APP_UI_BT_AUDIO_EVENT_HFP_SCO_DISCONNECTED                       = 0x15,
    APP_UI_BT_AUDIO_EVENT_HFP_CCWA_IND                               = 0x16,
    APP_UI_BT_AUDIO_EVENT_HFP_RING_IND                               = 0x17,
    APP_UI_BT_AUDIO_EVENT_HFP_CLCC_IND                               = 0x18,
    APP_UI_BT_AUDIO_EVENT_HFP_CALL_IND                               = 0x19,  //call hang up or active indication
    APP_UI_BT_AUDIO_EVENT_HFP_CALLSETUP_IND                          = 0x1A,  //call setup
    APP_UI_BT_AUDIO_EVENT_HFP_CALLHELD_IND                           = 0x1B,
    APP_UI_BT_AUDIO_EVENT_PROFILE_EXCHANGED                          = 0x1C,
    APP_UI_BT_AUDIO_EVENT_IBRT_STARTED                               = 0x1D,
    APP_UI_BT_AUDIO_EVENT_REPLAY_WHEN_PAUSE_COME_AFTER_SCO_DISC      = 0x1E,
    APP_UI_BT_AUDIO_EVENT_REPLAY_WHEN_SUSPEND_COME_AFTER_SCO_DISC    = 0x1F,
    APP_UI_BT_AUDIO_EVENT_ROUTE_CALL_TO_LE                           = 0x20,
    APP_UI_BT_AUDIO_EVENT_ACL_CONNECTED                              = 0x21,
    APP_UI_BT_AUDIO_EVENT_ACL_DISC                                   = 0x22,
    APP_UI_BT_AUDIO_EVENT_AUTHENTICATED                              = 0x23,
    APP_UI_BT_AUDIO_EVENT_HFP_CALL_MOCK                              = 0x24,
    APP_UI_BT_AUDIO_EVENT_HFP_CALLSETUP_MOCK                         = 0x25,
    APP_UI_BT_AUDIO_EVENT_HFP_CALLHOLD_MOCK                          = 0x26,
    APP_UI_BT_AUDIO_EVENT_HUNGUP_INCOMING_CALL                       = 0x27,
    APP_UI_BT_AUDIO_EVENT_HUNGUP_ACTIVE_CALL                         = 0x28,
    APP_UI_BT_AUDIO_HOLD_SWICTH_BACKGROUND                           = 0x29,
    APP_UI_BT_AUDIO_BG_TO_FG_COMPLETE                                = 0x2A,
    APP_UI_BT_AUDIO_EVENT_AVRCP_PLAY_STATUS_CHANGED_MOCK             = 0x2B,

    APP_UI_RECONNECTED                                               = 0x30,
    APP_UI_PAIR                                                      = 0x31,
    APP_UI_IDLE                                                      = 0x32,
    APP_UI_CHARGING                                                  = 0x33,
    APP_UI_POWER_ON                                                  = 0x34,
    APP_UI_POWER_OFF                                                 = 0x35,
    APP_UI_CONNECTED                                                 = 0x36,
    APP_UI_MUSIC_CALL                                                = 0x37,
    APP_UI_IMCONNIG_CALL                                             = 0x38,
    APP_UI_MAX_NUM                                                   = 0x40,
} app_ui_bt_audio_event_t;


bool get_disconnect_08_event(void);
void set_disconect_08_event(bool value);
void app_system_status_set(APP_STATUS_TYPE_T type);
void pairmode_exit_timeout_process(void);
APP_STATUS_TYPE_T app_system_status_get(void);
void app_ui_state_machine(app_ui_bt_audio_event_t event,uint8_t device_id,uint8_t nv_device_cnt);
void app_ui_status_init(void);
void app_ui_status_indication_close(void);
int app_ui_send_request(uint32_t message_id, uint32_t param0, uint32_t param1, uint32_t param2,uint32_t ptr);
int app_ui_charg_send_request(uint32_t message_id,  uint32_t param0, uint32_t param1);
#if defined(STEREO_HALL_EN)
void app_ui_hall_fast_switching(void);
#endif
void hall_poweroff_set_flag(void);

uint8_t stereo_battery_level_process(uint32_t status, uint16_t battery_volt);
void stereo_battery_update_to_phone(void);
uint8_t stereo_gfps_battery_level(uint8_t charge_status, uint8_t *batteryLevel);
uint16_t battery_volt_level_get(bool sta);
POSSIBLY_UNUSED int stereoui_battery_process(uint32_t status, uint32_t volt);

#if defined(ANC_APP)
void besui_set_curr_anc_mode(uint8_t sta);
uint8_t besui_get_curr_anc_mode(void);
int32_t besui_anc_set_mode(uint8_t sta);
void besui_anc_key_switch(void);
#endif
#ifdef DC_DETECT_EN
enum DC_CHARGER_T charger_usb_sta_get(void);
#endif
void stereoui_init(void);

#ifdef __cplusplus
}
#endif

#endif //#ifndef __STEREOUI_H__
