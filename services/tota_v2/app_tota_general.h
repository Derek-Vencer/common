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

#ifndef __APP_TOTA_GENERAL_H__
#define __APP_TOTA_GENERAL_H__

#include "app_battery.h"
#include "factory_section.h"
#include "bluetooth_bt_api.h"
#include "app_bt_stream.h"
#include "app_ibrt_internal.h"
#include "app_bt.h"
#include "app_key.h"
#ifdef IBRT
#include "app_tws_ibrt.h"
#endif

#define BT_BLE_LOCAL_NAME_LEN       32

#ifdef BESUI_APP_EN
#define APP_TOTA_BUTTON_SETTINGS_CONTROL_CMD              0x01
#define APP_TOTA_FACTORY_RESET_CMD                        0x02
#define APP_TOTA_MUSIC_PLAY_SETTINGS_CMD                  0x03
#define APP_TOTA_BATTERY_LEVEL_CMD                        0x04
#define APP_TOTA_EARBUD_FIT_TEST_CMD                      0x05
#define APP_TOAT_EQ_CMD                                   0x06
#define APP_TOTA_INEAR_DETECT_CMD                         0x07
#define APP_TOTA_GET_SPP_STATUS_CMD                       0x08
#define APP_TOTA_GET_BT_ADDR_CMD                          0x09
#define APP_TOTA_GET_CURRENT_BUTTON_STATE                 0x10
#define APP_TOTA_GET_PRODUCT_MODEL                        0x11
#define APP_TOTA_REGULATING_ANC                           0x12
#define APP_TOTA_REGULATING_EQ                            0x13
#define APP_TOTA_REGULATING_DOLBY                         0x14
#define APP_TOTA_REGULATING_BES_SPATIAL                   0x15
#define APP_TOTA_REGULATING_MIMI                          0x16
#define APP_TOTA_REGULATING_CEVA                          0x17
#define APP_TOTA_REGULATING_OTABOOT                       0x18
#define APP_TOTA_TWS_STATUS                               0x19
#define APP_TOTA_MULTIPOINT                               0x1A
#define APP_TOTA_SWITCH_ROLE                              0x1B

//earbuds 0x80~0xF0
#define APP_TOTA_CODEC_TYPE                               0x80
#define APP_TOTA_GAME                                     0x81
#define APP_TOTA_VOLUME                                   0x82

//test
#define APP_TOTA_TEST1                                    0xF1
#define APP_TOTA_ENC_ONOFF                                0xF2
#define APP_TOTA_LINKKEY                                  0xF3
#define APP_TOTA_SENSOR                                   0xF4  
#define APP_TOTA_EQTUNE                                   0xF5  


#define BUTTON_SETTING_LEFT_EARPHONE_CMD       0x01
#define BUTTON_SETTING_RIGHT_EARPHONE_CMD      0x02

#define BUTTON_SETTING_CLICK_CMD               0x01
#define BUTTON_SETTING_DOUBLE_CLICK_CMD        0x02
#define BUTTON_SETTING_TRIPLE_CLICK_CMD        0x03
#define BUTTON_SETTING_LONG_PRESS_CMD          0x04

#define BUTTON_SETTING_LEFT_CLICK_CMD           0x01
#define BUTTON_SETTING_RIGHT_CLICK_CMD          0x02
#define BUTTON_SETTING_LEFT_DOUBLE_CMD          0x03
#define BUTTON_SETTING_RIGHT_DOUBLE_CMD         0x04
#define BUTTON_SETTING_LEFT_TRIPLE_CMD          0x05
#define BUTTON_SETTING_RIGHT_TRIPLE_CMD         0x06
#define BUTTON_SETTING_LEFT_LONG_PRESS_CMD      0x07
#define BUTTON_SETTING_RIGHT_LONG_PRESS_CMD     0x08

#define BUTTON_SETTING_PRE_SONG_CMD            0x01
#define BUTTON_SETING_NEXT_SONG_CMD            0x02
#define BUTTON_SETTING_ANC_CMD                 0x03
#define BUTTON_SETTING_CALL_CMD                0x04
#define BUTTON_SETTING_VOLUME_UP_CMD           0x05
#define BUTTON_SETTING_VOLUME_DOWN_CMD         0x06
#define BUTTON_SETTING_PLAY_MUSIC_CMD          0x07
#define BUTTON_SETTING_PAUSE_MUSIC_CMD         0x08
#define BUTTON_SETTING_VOICE_ASSITANT_CMD      0x09
#define BUTTON_SETTING_PLAY_PAUSE_MUSIC_CMD    0x0a
#define BUTTON_SETTING_GAME_MODE_CMD           0x0b
#define BUTTON_SETTING_ALGO_CMD                0x0c
#define BUTTON_SETTING_ANC_SPEAKTHRU           0x0d
#define BUTTON_SETTING_AIVOICE_WAKEUP          0x0f
#define BUTTON_SETTING_ERROR                   0x00

#define BUTTON_MAP_KEY_TYPE                 APP_KEY_CODE_PWR //button support 
#define BUTTON_L_CLICK_DEFAULT              BUTTON_SETTING_PLAY_PAUSE_MUSIC_CMD
#define BUTTON_R_CLICK_DEFAULT              BUTTON_SETTING_PLAY_PAUSE_MUSIC_CMD
#define BUTTON_L_DOUBLE_DEFAULT             BUTTON_SETTING_PRE_SONG_CMD
#define BUTTON_R_DOUBLE_DEFAULT             BUTTON_SETING_NEXT_SONG_CMD
#if defined(BES_I9CHAT_EN)
#undef BUTTON_L_DOUBLE_DEFAULT
#undef BUTTON_R_DOUBLE_DEFAULT
#define BUTTON_L_DOUBLE_DEFAULT             BUTTON_SETTING_AIVOICE_WAKEUP
#define BUTTON_R_DOUBLE_DEFAULT             BUTTON_SETTING_AIVOICE_WAKEUP
#endif

//3 click
#if defined(BES_IH6_EN) || defined(BES_IH6PRO_EN)
#define BUTTON_L_TRIPLE_DEFAULT             BUTTON_SETTING_GAME_MODE_CMD
#define BUTTON_R_TRIPLE_DEFAULT             BUTTON_SETTING_ANC_SPEAKTHRU
#elif defined(BES_E53B_EN)
#define BUTTON_L_TRIPLE_DEFAULT             BUTTON_SETTING_GAME_MODE_CMD
#define BUTTON_R_TRIPLE_DEFAULT             BUTTON_SETTING_GAME_MODE_CMD
#elif defined(BES_NAZPRO_EN)
#define BUTTON_L_TRIPLE_DEFAULT             BUTTON_SETTING_GAME_MODE_CMD
#define BUTTON_R_TRIPLE_DEFAULT             BUTTON_SETTING_VOICE_ASSITANT_CMD
#elif defined(BESUI_VI_EN)
#define BUTTON_L_TRIPLE_DEFAULT             BUTTON_SETTING_GAME_MODE_CMD
#define BUTTON_R_TRIPLE_DEFAULT             BUTTON_SETTING_VOICE_ASSITANT_CMD
#else
#define BUTTON_L_TRIPLE_DEFAULT             BUTTON_SETTING_ERROR
#define BUTTON_R_TRIPLE_DEFAULT             BUTTON_SETTING_ERROR
#endif

//L long press
#if defined(BES_IM2LDAC_EN) 
#define BUTTON_L_LONG_PRESS_DEFAULT         BUTTON_SETTING_ERROR
#elif defined(BES_I9CHAT_EN) 
#define BUTTON_L_LONG_PRESS_DEFAULT         BUTTON_SETTING_ANC_CMD
#else
#define BUTTON_L_LONG_PRESS_DEFAULT         BUTTON_SETTING_ALGO_CMD
#endif

//R long press
#if defined(BES_IH6_EN) || defined(BES_E53B_EN) || defined(BESUI_VI_EN) || defined(BES_NAZPRO_EN)
#define BUTTON_R_LONG_PRESS_DEFAULT         BUTTON_SETTING_ANC_CMD
#else
#define BUTTON_R_LONG_PRESS_DEFAULT         BUTTON_SETTING_GAME_MODE_CMD
#endif

#if defined(BESUI_STEREO_EN) //stereo
#define BUTTON_CLICK_DEFAULT                BUTTON_SETTING_PLAY_PAUSE_MUSIC_CMD
#define BUTTON_DOUBLE_DEFAULT               BUTTON_SETTING_ERROR
#define BUTTON_TRIPLE_DEFAULT               BUTTON_SETTING_VOICE_ASSITANT_CMD
#define BUTTON_LONG_PRESS_DEFAULT           BUTTON_SETTING_GAME_MODE_CMD
#endif

#define MUSIC_PLAY_SETTINGS_PLAY_CMD                      0x01
#define MUSIC_PLAY_SETTINGS_PAUSE_CMD                     0x02
#define MUSIC_PLAY_SETTINGS_NEXT_CMD                      0x03
#define MUSIC_PLAY_SETTINGS_PRE_CMD                       0x04

#define EARBUD_DETECK_ON_CMD                              0x01
#define EARBUD_DETECK_OFF_CMD                             0x00

#define INEAR_DETECT_STATUS_CMD                           0x00
#define INEAR_DETECT_LEFT_EAR_CMD                         0x01
#define INEAR_DETECT_RIGHT_EAR_CMD                        0x02
#define INEAR_DETECT_BOX_CMD                              0x03
#define INEAR_DETECT_PROMPT                               0x03
#define INEAR_DETECT_OFF_CMD                              0x00
#define INEAR_DETECT_ON_CMD                               0x01

#define APP_TOTA_GET_ANC_STATUS             0x01
#define APP_TOTA_SET_ANC_STATUS             0x02

#define APP_TOTA_GET_EQ_ONOFF_STATUS        0x01
#define APP_TOTA_SET_EQ_ONOFF_STATUS        0x02
#define APP_TOTA_SET_EQ_PARAMETER           0x03
#if defined(EQ_CUSTOM_APP_EN)
#define APP_TOTA_SET_EQ_CUSTOM              0x04
#define APP_TOTA_GET_EQ_CUSTOM              0x05
#endif
#define EQ_TYPE_DEFAULT                     0x01
#define EQ_TYPE_POP                         0x02
#define EQ_TYPE_ROCK                        0x03
#define EQ_TYPE_JAZZ                        0x04
#define EQ_TYPE_CLASSIC                     0x05
#define EQ_TYPE_COUNTRY                     0x06


#define APP_TOTA_GET_DOLBY_STATUS           0x01
#define APP_TOTA_SET_DOLBY_STATUS_ONOFF     0x02
#define APP_TOTA_SET_DOLBY_STATUS           0x03
#define DOLBY_STATUS_ON                     0x01
#define DOLBY_STATUS_OFF                    0x02

#define APP_TOTA_GET_BES_SPATIAL_STATUS     0x01
#define APP_TOTA_SET_BES_SPATIAL_STATUS     0x02

#define APP_TOTA_GET_MIMI_ONOFF_STATUS      0x01
#define APP_TOTA_SET_MIMI_ONOFF_STATUS      0x02
#define APP_TOTA_SET_MIMI_PRESET            0x03
#define APP_TOTA_SET_MIMI_INTENSTIY         0x04
#define APP_TOTA_GET_MIMI_TECH_LEVEL        0x05
#define MIMI_STATUS_ON                      0x01
#define MIMI_STATUS_OFF                     0x02
 

#define APP_TOTA_GET_CEVA_STATUS            0x01
#define APP_TOTA_SET_CEVA_STATUS            0x02
#endif //#ifdef BESUI_APP_EN

typedef struct{
    /* bt-ble info */
    char btName[BT_BLE_LOCAL_NAME_LEN];         // 32 bytes
    char bleName[BT_BLE_LOCAL_NAME_LEN];        // 32 bytes
    bt_bdaddr_t btLocalAddr;                    // 6 bytes
    bt_bdaddr_t btPeerAddr;                     // 6 bytes
    bt_bdaddr_t bleLocalAddr;                   // 6 bytes
    bt_bdaddr_t blePeerAddr;                    // 6 bytes

    /* ibrt info */
    uint8_t ibrtRole;                       // 1 byte

    /* crystal info */
    uint32_t crystal_freq;                      // 4 bytes
    uint32_t xtal_fcap;                         // 4 bytes
    
    /* battery info */
    APP_BATTERY_MV_T        battery_volt;       // 2 bytes
    uint8_t                 battery_level;      // 1 byte
    APP_BATTERY_STATUS_T    battery_status;     // 4 bytes

    /* firmware info */
    uint8_t fw_version[4];                      // 4 bytes
    
    /* ear location info */
    uint8_t          ear_location;       // APP_TWS_SIDE_T

    /* rssi info */
    uint8_t                 rssi[48];           // 48 bytes
    uint32_t                rssi_len;           // 4 bytes
} general_info_t;


/*--functions from other file--*/
#ifdef FIRMWARE_REV
extern "C" void system_get_info(uint8_t *fw_rev_0, uint8_t *fw_rev_1, uint8_t *fw_rev_2, uint8_t *fw_rev_3);
#endif

extern int app_bt_stream_local_volume_get(void);
extern uint8_t app_audio_get_eq();
extern int app_audio_set_eq(uint8_t index);
extern bool app_meridian_eq(bool onoff);
extern bool app_is_meridian_on();

#ifdef BLE
extern unsigned char *bt_get_ble_local_address(void);
#endif
/*--functions from other file--*/


/*--interface--*/
void app_tota_general_init();
void app_tota_sync_leak_deteck_handle(uint8_t *p_buff, uint8_t length);
void app_tota_sync_audio_eq_handle(uint8_t *p_buff, uint8_t length);

void app_ibrt_send_leak_deteck_status_handle();
void app_tota_start_leak_detect(void);
bool app_tota_get_detect_flag();
void app_tota_set_leak_detect_value();
void app_tota_general_button_event_handler(APP_KEY_STATUS *status, bool is_right);
void app_tota_general_button_event_init(void);
void app_tota_general_button_event_info_set(uint8_t* ptrParam, uint16_t paramLen);
void app_tws_sync_battery_2_slave(uint8_t *p_buff, uint8_t length);
void app_tota_sync_battery_level_rsp_handle(uint8_t *p_buff, uint8_t length);
void app_tota_general_button_event_sync(uint8_t* ptrParam, uint16_t paramLen);
void app_tota_inear_detect_set_on_ear(void);
void app_tota_inear_detect_set_off_ear(void);
bool app_inear_get_right_status(void);
bool app_inear_get_left_status(void);
bool app_inear_get_myself_status(void);
void app_tota_send_inear_status(uint8_t *p_buff, uint8_t length);
void app_tota_general_button_event_reset(void);

#ifdef ALGO_INFO_SYNC_EN
#include "app_tota_cmd_code.h"
void app_tota_algo_send_to_app(APP_TOTA_CMD_CODE_E rsp_opCode,uint8_t cmd_type, uint8_t ptrParam1, uint8_t ptrParam2, uint8_t ptrParam3, uint32_t paramLen);
#endif

#endif

