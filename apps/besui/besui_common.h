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
#ifndef __BESUI_COMMON_H__
#define __BESUI_COMMON_H__

#include "besui_define.h"
#if defined(BESUI_TWS_EN)
#include "tws_ui/twsui_led.h"
#elif defined(BESUI_STEREO_EN)
#include "stereo_ui/stereo_led.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
#define USER_BOOTMODE_FACTORY   (1 << 31)

#ifdef RSSI_SWITCH_ROLE_EN
#define RSSI_MASTER_VALUE       -80//-65 //master low this value
#define RSSI_LOW_TIMES          3//2   //master switch to slave low times
#endif

enum{
    SHUTDOWN_DEFAULT = 0,
    SHUTDOWN_TOUCH_PRESS,
    SHUTDOWN_PAIRMODE_TIMEOUT,
    SHUTDOWN_RECONNECT_TIMEOUT,
    SHUTDOWN_CLEAR_PAIR,
    SHUTDOWN_BAT_CHARGE_FULL,
    SHUTDOWN_BAT_LOW,
    SHUTDOWN_NTC_TEMP,
    SHUTDOWN_FACTORY_RESET,
    SHUTDOWN_SWITCH_ROLE,
    SHUTDOWN_BOX_CMD,
    SHUTDOWN_BOX_RST,
    SHUTDOWN_ADDR_SWAP_OK,
    SHUTDOWN_SHIP_MODE,
    SHUTDOWN_SYNC_PEER,
    SHUTDOWN_DUT_TIMEOUT,
    SHUTDOWN_HFP_CMD,
    SHUTDOWN_LINEIN_INSERT,
    SHUTDOWN_HFP_RST,
 
    SHUTDOWN_MAX,
};

//add to hfp_service.h
enum{
    HFCALL_MACHINE_CURRENT_AUDIO,                                   //21
    HFCALL_MACHINE_CURRENT_AUDIO_OR_ANOTHER_AUDIO,                  //22
#ifdef USER_QUICK_SWITCH_EN
    HFCALL_MACHINE_CURRENT_INCOMING_ANOTHER_PC_STREAMING,           //23
    HFCALL_MACHINE_CURRENT_CALLING_ANOTHER_PC_STREAMING,            //24
#endif
};

// add to bt_common_define.h
enum{
#ifdef BESUI_TWS_EN
    AUD_ID_ANC_ON_MODE = 0x50,
    AUD_ID_ANC_AA_MODE = 0x51,
    AUD_ID_BT_BEASTGAME_MODE = 0x52,
    AUD_ID_SPATIAL_ON = 0x53,
    AUD_ID_SPATIAL_OFF = 0x54,
    AUD_ID_BT_MAX_VOL = 0x55,
	AUD_ID_WEAR_DETECT = 0x56,
    AUD_ID_SIGNATURE = 0x57,
    AUD_ID_DOLBY_ON = 0x58,
    AUD_ID_ANC_ST_MODE = 0x59,
    AUD_ID_GAME_OFF = 0x5A,
    AUD_ID_TWS_CONN = 0x5B,
    AUD_ID_TWS_DISCONN = 0x5C,
    AUD_ID_KEY_VOICE = 0x5D,
#elif BESUI_STEREO_EN
    AUD_ID_MAX_VOLUME=0x50,
    AUD_ID_EQ_OFF = 0x51,
    AUD_ID_EQ_ON = 0x52,
    AUD_ID_GAME_MODE = 0x53,
    AUD_ID_MUSIC_MODE = 0x54,
    AUD_ID_DOLBY_ON = 0x55,
#endif
};

#ifdef ALGO_INFO_SYNC_EN
enum{
    ALGO_ONOFF_DEFAULT = 0x00,
    ALGO_ON = 0x01,
    ALGO_OFF= 0x02,

};
enum{
    ALGO_ID_DEFAULT = 0,
    ALGO_ID_EQ,
    ALGO_ID_MIMI,
    ALGO_ID_DOLBY,
    ALGO_ID_BESSPA,
    ALGO_ID_CEVA,
    ALGO_ID_ANC,
    ALGO_ID_OTABOOT,
    ALGO_ID_MULTIPOINT,
    ALGO_ID_EQ_CUSTOM,

    ALGO_ID_MAX,
};
#endif

#ifdef BESUI_COMM_EN
enum{
    USER_CODEC_AAC  = 0x00,
    USER_CODEC_SBC  = 0x01,
    USER_CODEC_LHDC = 0x02,
    USER_CODEC_LDAC = 0x03,

    USER_CODEC_MAX  = 0x04,

    USER_CODEC_UNKNOWN = 0xBD,
};

#ifdef BESUI_APP_EN
extern struct nvrecord_env_t *nvrecord_uienv;
#endif

typedef struct {
    uint8_t poweron_init_ok;
    uint32_t run_time_10s;
#ifdef USE_ALGO_EN
    bool algo_memory_init_flag; //algo ram init flag
    uint32_t audio_sample_rate;
#endif
#if defined(DOLBY_AUDIO_ENABLE)
    uint8_t dolby_onoff_sta;
    uint8_t dolby_mode;
#endif

#ifdef USER_COMPUTER_STREAMING_DYNAMIC
    uint32_t mute_det_timeout;
#endif
#if defined(OTA_BOOT_SYNC_EN)
    uint8_t tws_recv_otaboot_flag;
#endif

    int8_t local_rssi;
    int8_t peer_rssi;
    bool poweroff_start_flag;
    bool auth_start_flag;
    bool role_switch_no;        //1:forbid role switch
    bool user_factory_flag;
    bool poweroff_fast_flag;

#ifdef GFPS_ENABLED
    bool gfps_eneter_pairmode;
#endif
#ifndef FREEMAN_ENABLED_STERO
    bool case_open_flag;
    bool box_sta;
#endif
#ifdef BESUI_APP_EN
    bool ear_current_sta;
    bool ear_another_sta;
#endif
#ifdef USER_APP_BLE_DIS_EN
    uint8_t random_local[2];
    uint8_t random_peer[2];
#endif
#ifdef CAPSENSOR_ENABLE
    uint8_t capsensor_onoff;
#endif
#ifdef USER_TOTA_SPP_SYNC_KEY_EN
    uint8_t aeskey[32];
    bool aesflag;
#endif
#ifdef BESUI_CAPSENSOR_FACTORY_EN
    bool capsensor_uart_factory_mode;
#endif
#ifdef USER_LED_CASE_OPEN_EN
    bool led_caseopen;
    uint8_t led_next;
#endif
    uint8_t usage_ap;
    uint8_t usage_cp;
#ifdef BESUI_STEREO_EN
    bool prompt_end;
    bool batt_low_flag;
#endif
#if defined(UUID_TOTA_USE_OTA_EN) && (defined(BESUI_TWS_EN) || defined(BESUI_STEREO_EN))
    bool spp_conn_type;
#endif
    uint8_t shutdown_type;
#ifdef BESUI_TWS_EN
    bool callend_mute_flag;
#endif
#ifdef BESUI_GAME_EN
    bool game_set_flag;
#endif
#if defined(USER_NOISE_ADAPTIVE_ANC_EN)
    uint8_t adapt_mode;
#endif

#ifdef ANC_PARAM_OTA_EN
    int32_t anc_50p7k[4][2];
#endif
} type_uictl_t;
extern type_uictl_t uictl;
#endif //#ifdef BESUI_COMM_EN

#ifdef ALGO_INFO_SYNC_EN
#ifdef BESSPA_ONOFF_EN
int besspa_audio_onoff_get(void);
void besspa_audio_onoff(uint8_t sta, bool param);
#endif
#ifdef DOLBY_AUDIO_ENABLE
uint8_t dolby_audio_onoff_get(void);
#endif

void algo_send_request(uint32_t message_id, uint32_t param0, uint32_t param1, uint32_t param2, uint32_t param3, uint32_t ptr);
void algo_sync_to_slave(void);
#endif //ALGO_INFO_SYNC_EN

#if GFPS_FIND_VOICE_LOOP_EN
uint8_t app_prompt_gfps_set_volume(uint8_t value);
uint8_t app_prompt_gfps_get_volume(void);
#endif
#ifdef BESUI_APP_EN
bool app_wear_det_onoff_get(void);
#endif

#ifdef USER_APP_BLE_DIS_EN
void app_random_ble_get(void);
void ble_random_add_adv(uint8_t * buf);
#endif

#ifdef BESUI_COMM_EN
void codec_type_stream_set(uint8_t type);
uint8_t codec_type_stream_get(void);
const char *codec_type_stream_get_str(void);
void codec_type_set(uint8_t id, uint8_t type);
void codec_type_to_app(void);
const char *cpu_freq_get_str(void);
void besui_audio_set_channel(bool onoff);
bool besui_audio_get_channel(void);
void besui_algo_set_eq(uint8_t id, bool sta);
void user_bat_volt_set(uint16_t dat);
uint16_t user_bat_volt_get(void);
void besui_algo_mips_trace(uint8_t algo, bool sta);
void user_set_peer_box_sta(uint16_t param);
uint16_t user_get_peer_box_sta(void);
void user_set_putinout_sta(uint8_t param);
uint8_t user_get_putinout_sta(void);
void besui_system_info_trace(void);
void besui_trace_shutdown_type(void);
void besui_common_init(void);
#endif //#ifdef BESUI_COMM_EN

#ifdef APP_SYNC_GAME_EN
void app_sync_earbuds_latency(uint8_t param);
#endif
#ifdef APP_SYNC_VOLUME_EN
void app_sync_earbuds_volume(void);
#endif

uint8_t besui_get_profile_conn_num(void);
bool besui_get_spp_sta(void);
void besui_set_ble_sta(bool param);
bool besui_get_ble_sta(void);

#if defined(USER_NOISE_ADAPTIVE_ANC_EN)
void besui_anc_adapt_set_onoff(bool param);
bool besui_anc_adapt_get_onoff(void);
#endif

#ifdef __cplusplus
}
#endif

#endif//__BESUI_COMMON_H__
