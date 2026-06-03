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
#include "bluetooth_bt_api.h"
#include "hal_cmu.h"
#include "app_tota_cmd_code.h"
#include "app_tota.h"
#include "app_tota_cmd_handler.h"
#include "cmsis.h"
#include "app_hfp.h"
#include "app_a2dp.h"
#include "app_key.h"
#include "app_tota_general.h"
#include "app_spp_tota.h"
#include "nvrecord_ble.h"
#include "app_tota_if.h"
#include "app_tota_common.h"
#if defined(TOTA_EQ_TUNING)
#include "hal_cmd.h"
#endif
#ifdef IBRT
#include "bts_core_if.h"
#include "bts_tws_if.h"
#ifdef BT_APP_RSSI
#include "app_rssi.h"
#endif
#include "earbud_ux_api.h"
#endif

#ifdef BESUI_APP_EN
#include "app_tws_ibrt_cmd_handler.h"
#include "apps.h"
#include "app_bt_stream.h"
#include "app_utils.h"
#include "app_ibrt_customif_cmd.h"
#include "app_ibrt_customif_ui.h"
#include "app_audio_control.h"
#include "app_capsensor.h"
#ifdef ANC_APP
#include "app_anc.h"
#endif

#include "audio_policy.h"
#include "app_bt_media_manager.h"
#include "nvrecord_extension.h"
#include "nvrecord_env.h"
#endif //#ifdef BESUI_APP_EN

#if defined(CAPSENSOR_ENABLE)
#include CHIP_SPECIFIC_HDR(analog)
#endif

#include "apps.h"
#include "bes_gap_api.h"
#ifdef BESUI_STEREO_EN
#include "stereo_prompt.h"
#include "stereo_key.h"
#include "stereoui.h"
#endif
#if defined(DOLBY_AUDIO_ENABLE)
#include "example.h"
#endif
#ifdef ALGO_INFO_SYNC_EN
#include "app_thread.h"
#endif

#ifdef BESUI_KEY_EN
#include "twsui_key.h"
#endif
#ifdef BESUI_TWS_EN
#include "twsui_comm.h"
#include "twsui_btmsg.h"
#endif
#ifdef USER_TOTA_SPP_SYNC_KEY_EN
#include "ota_spp.h"
#endif
#if defined(BESUI_TWS_EN) || defined(BESUI_STEREO_EN)
#include "besui_common.h"
#endif

#ifdef ALGO_DELAY_ONOFF_EN
#include "app_media_player.h"
uint8_t algo_id_buf = 0;
uint8_t algo_onoff_buf = 0;
uint8_t algo_mode_buf = 0;
osTimerId algo_process_timer_id = NULL;

void algo_set_buf(uint8_t id, uint8_t onoff, uint8_t mode)
{
	BESUI_TRACE(1, "[UIALGO]%s, %d,%d,%d", __func__, id, onoff, mode);

    algo_id_buf = id;
    algo_onoff_buf = onoff;
    algo_mode_buf = mode;
}

void algo_buf_clear(void)
{
	BESUI_TRACE(1, "[UIALGO]%s", __func__);

    algo_id_buf = 0;
    algo_onoff_buf = 0;
    algo_mode_buf = 0;
}

void algo_process_timehandler(void const *param)
{
    uint8_t conn_devices = besui_get_profile_conn_num();

	BESUI_TRACE(1, "[UITIMER]%s conn_devices %d", __func__, conn_devices);
#if defined(DOLBY_AUDIO_ENABLE)
    if(algo_id_buf == ALGO_ID_DOLBY)
    {
        if(algo_onoff_buf == ALGO_ON)
        {
            algo_send_request(ALGO_ID_DOLBY, DOLBY_STATUS_ON, algo_mode_buf, 0, 0, 0);
        }
        else if(algo_onoff_buf == ALGO_OFF)
        {
            algo_send_request(ALGO_ID_DOLBY, DOLBY_STATUS_OFF, uictl.dolby_mode, 0, 0, 0);
        }
    }
#endif
    algo_buf_clear();
    // osTimerDelete(algo_process_timer_id);
    // algo_process_timer_id = NULL;
}
osTimerDef (ALGO_TIMER_DEF, (void (*)(void const *))algo_process_timehandler);
void algo_timer_onoff(bool timer_en)
{
	BESUI_TRACE(1, "[UITIMER]%s timer_en %d ", __func__, timer_en);
    
	if(algo_process_timer_id == NULL)
	{
    	algo_process_timer_id = osTimerCreate(osTimer(ALGO_TIMER_DEF),osTimerOnce,NULL);
	}

    osTimerStop(algo_process_timer_id);
	if(timer_en)
        osTimerStart(algo_process_timer_id, 700);
	// else
	// 	osTimerStop(algo_process_timer_id);
}

#if defined(DOLBY_AUDIO_ENABLE)
void algo_dolby_delay_onoff(uint8_t onoff, uint8_t mode)
{
    algo_set_buf(ALGO_ID_DOLBY, onoff, mode);
#ifdef BESUI_TWS_EN
    if(algo_onoff_buf == ALGO_ON)
        media_PlayAudio(AUD_ID_DOLBY, 0);
    else
        media_PlayAudio(AUD_ID_SIGNATURE, 0);
#endif
    algo_timer_onoff(true);
}
#endif
#endif //#ifdef ALGO_DELAY_ONOFF_EN

#ifdef BESSPA_ONOFF_EN
extern "C" int32_t stereo_surround_status;
#endif

#if (TOTA_GENERAL_ENABLE)

/*------------------------------------------------------------------------------------------------------------------------*/
typedef struct{
    uint32_t    address;
    uint32_t    length;
}TOTA_DUMP_INFO_STRUCT_T;

static inline void sys_memcpy_swap(void *dst, const void *src, size_t length)
{
	uint8_t *pdst = (uint8_t *)dst;
	const uint8_t *psrc = (const uint8_t *)src;

	ASSERT(((psrc < pdst && (psrc + length) <= pdst) ||
		  (psrc > pdst && (pdst + length) <= psrc)),
		 "Source and destination buffers must not overlap");

	psrc += length - 1;

	for (; length > 0; length--) {
		*pdst++ = *psrc--;
	}
}

#define TOTA_CRC_CHECK                      0x54534542
#define LEN_OF_IMAGE_TAIL_TO_FINDKEY_WORD    512

static void general_get_flash_dump_info(TOTA_DUMP_INFO_STRUCT_T * p_flash_info);
static general_info_t general_info;
static void __get_general_info();

typedef struct{
    uint32_t    crc;            //fw crc, generate by generate_crc32_of_image.py
    uint8_t     version[4];     //fw version
    uint8_t     build_date[32]; //fw build data,auto generate by compiler when build
}TOTA_CRC_CHECK_STRUCT_T;

extern const char sys_build_info[];

static const char* image_info_sanity_crc_key_word = "CRC32_OF_IMAGE=0x";
static const char* image_info_build_data = "BUILD_DATE=";

// static int32_t find_key_word(uint8_t* targetArray, uint32_t targetArrayLen, uint8_t* keyWordArray, uint32_t keyWordArrayLen);
// static uint8_t asciiToHex(uint8_t asciiCode);
static APP_TOTA_CMD_RET_STATUS_E tota_get_sanity_crc(uint32_t *sanityCrc32);
static APP_TOTA_CMD_RET_STATUS_E tota_get_build_data(uint8_t *buildData);
#ifdef BESUI_APP_EN
static BUTTON_SET_EVENT_INFO_T keymap_l_tab[4];
static BUTTON_SET_EVENT_INFO_T keymap_r_tab[4];
extern void bt_audio_updata_eq(uint8_t index);
static void button_cmd_execute(uint8_t cmd)
{
    POSSIBLY_UNUSED struct BT_DEVICE_T* a2dp_device = app_bt_get_device(app_bt_audio_get_curr_a2dp_device());
    uint8_t device_id = app_bt_audio_get_device_for_user_action();
    BESUI_TRACE(2,"[UIAPPKEY]%s, cmd = %d", __func__, cmd);
    BESUI_TRACE(0,"[UIAPPKEY]%s, play_pause_flag=%d, streamming=%d", __func__, a2dp_device->a2dp_play_pause_flag, a2dp_device->a2dp_streamming);

    switch(cmd)
    {
        case BUTTON_SETTING_PRE_SONG_CMD:
        a2dp_handleKey(AVRCP_KEY_BACKWARD);
        break;
        case BUTTON_SETING_NEXT_SONG_CMD:
        a2dp_handleKey(AVRCP_KEY_FORWARD);
        break;
#if ANC_APP
        case BUTTON_SETTING_ANC_CMD:
#ifdef BESUI_TWS_EN
        besui_anc_key_switch();
#else
        app_anc_loop_switch();
#endif
        break;
#endif
        case BUTTON_SETTING_CALL_CMD:
        app_ibrt_if_hf_call_redial(device_id);
        break;
        case BUTTON_SETTING_VOLUME_UP_CMD:
        app_ibrt_if_set_local_volume_up();
        break;
        case BUTTON_SETTING_VOLUME_DOWN_CMD:
        app_ibrt_if_set_local_volume_down();
        break;
        case BUTTON_SETTING_PLAY_MUSIC_CMD:
        if(a2dp_device && (a2dp_device->a2dp_play_pause_flag == 0 || a2dp_device->a2dp_streamming == false))
            a2dp_handleKey(AVRCP_KEY_PLAY);
        break;
        case BUTTON_SETTING_PAUSE_MUSIC_CMD:
        if(a2dp_device && (a2dp_device->a2dp_play_pause_flag == 0 || a2dp_device->a2dp_streamming == false))
        {
            //a2dp_handleKey(AVRCP_KEY_PLAY);
        }
        else
        {
            a2dp_handleKey(AVRCP_KEY_PAUSE);
        }
        break;
        case BUTTON_SETTING_PLAY_PAUSE_MUSIC_CMD:
        if(a2dp_device && (a2dp_device->a2dp_play_pause_flag == 0 || a2dp_device->a2dp_streamming == false))
        {
            BESUI_TRACE(0,"[UIAPPKEY]play");
            a2dp_handleKey(AVRCP_KEY_PLAY);
        }
        else
        {
            BESUI_TRACE(0,"[UIAPPKEY]pause");
            a2dp_handleKey(AVRCP_KEY_PAUSE);
        }
        break;
#ifdef SUPPORT_SIRI
        case BUTTON_SETTING_VOICE_ASSITANT_CMD:
#ifdef BESUI_TWS_EN
        app_siri_timer_onoff(true);
#else
        app_ibrt_if_enable_hfp_voice_assistant(true);
#endif
        break;
#endif
        case BUTTON_SETTING_GAME_MODE_CMD:
#ifdef BESUI_GAME_EN
        besui_gamemode_key_switch(true, 0x00, true);
#endif
        break;
        case BUTTON_SETTING_ALGO_CMD:
#ifdef BESUI_TWS_EN
#if defined(__VIRTUAL_SURROUND__) || defined(__VIRTUAL_SURROUND_CP__) || defined(__VIRTUAL_SURROUND_STEREO__)
        if(!besspa_audio_onoff_get()) //out ear off
        {
            if(!bts_tws_if_is_tws_link_connected())
            {
                BESUI_TRACE(0,"[UIAPP]peer ear off");
                break;            
            }
        }
        besspa_key_loop_switch();
#elif defined(DOLBY_AUDIO_ENABLE)
#ifdef ALGO_DELAY_ONOFF_EN
        algo_dolby_delay_onoff(2-(!dolby_audio_onoff_get()), uictl.dolby_mode);
#else
        algo_send_request(ALGO_ID_DOLBY, 2-(!dolby_audio_onoff_get()), uictl.dolby_mode, 0, 0, 0);
#endif
#endif
#endif //#ifdef BESUI_TWS_EN
        break;
        case BUTTON_SETTING_ANC_SPEAKTHRU:
#if defined(ANC_SPEAKTHRU_EN)
#if defined(ANC_APP) && defined(ALGO_INFO_SYNC_EN)
        algo_send_request(ALGO_ID_ANC, APP_ANC_MODE3, 0, 0, 0, 1);
#endif
#endif
        break;
        
        case BUTTON_SETTING_AIVOICE_WAKEUP:
#if defined(BESUI_AIVOICE_EN) && defined(__SMART_VOICE_KWS_AQE__)
            besui_aivoice_onoff();
#endif
        break;
        
        default:
        BESUI_TRACE(0, "[UIAPPKEY]%s, error cmd", __func__);
        break;
    }
}

void app_tota_general_button_event_handler(APP_KEY_STATUS *status, bool is_right)
{
    memcpy(keymap_l_tab, nvrecord_uienv->buttonInfo_left, sizeof(keymap_l_tab));
    memcpy(keymap_r_tab, nvrecord_uienv->buttonInfo_right, sizeof(keymap_r_tab));
    BESUI_TRACE(0, "KEY_STATUS  %d %d %d", is_right, status->code, status->event);

    const char *event_str[] = {
        "BUTTON_SETTING_ERROR",
        "BUTTON_SETTING_PRE_SONG_CMD",
        "BUTTON_SETING_NEXT_SONG_CMD",
        "BUTTON_SETTING_ANC_CMD",
        "BUTTON_SETTING_CALL_CMD",
        "BUTTON_SETTING_VOLUME_UP_CMD",
        "BUTTON_SETTING_VOLUME_DOWN_CMD",
        "BUTTON_SETTING_PLAY_MUSIC_CMD",
        "BUTTON_SETTING_PAUSE_MUSIC_CMD",
        "BUTTON_SETTING_VOICE_ASSITANT_CMD",
        "BUTTON_SETTING_PLAY_PAUSE_MUSIC_CMD",
        "BUTTON_SETTING_GAME_MODE_CMD",
        "BUTTON_SETTING_ALGO_CMD",
        "BUTTON_SETTING_ANC_SPEAKTHRU",
        "BUTTON_SETTING_INVALID",
        "BUTTON_SETTING_AIVOICE_WAKEUP",
        "BUTTON_SETTING_MAX",
    };
    for(uint8_t i=0; i<4; i++)
    {
        // BESUI_TRACE(0, "keymap_l_tab left :%d, %d, %d, 0x%02X", keymap_l_tab[i].lr, keymap_l_tab[i].status.code, keymap_l_tab[i].status.event,keymap_l_tab[i].button_set_event);
        // BESUI_TRACE(0, "keymap_r_tab right:%d, %d, %d, 0x%02X", keymap_r_tab[i].lr, keymap_r_tab[i].status.code, keymap_r_tab[i].status.event,keymap_r_tab[i].button_set_event);
        if (((keymap_l_tab[i].lr == BUTTON_SETTING_LEFT_EARPHONE_CMD) && !is_right) 
        || ((keymap_l_tab[i].lr == BUTTON_SETTING_RIGHT_EARPHONE_CMD) && is_right))
        {
            if(keymap_l_tab[i].status.code == status->code)
            {
                if (keymap_l_tab[i].status.event == status->event)
                {
                    // BESUI_TRACE(0, "key left = %d %d %d %d", i, is_right, status->code, status->event);
                    button_cmd_execute(keymap_l_tab[i].button_set_event);
                    if(status->event < 16)
                        BESUI_TRACE(0, "[UIMAP], [%s]", event_str[status->event]);
                }
            }
        }
    
        if (((keymap_r_tab[i].lr == BUTTON_SETTING_LEFT_EARPHONE_CMD) && !is_right) 
        || ((keymap_r_tab[i].lr == BUTTON_SETTING_RIGHT_EARPHONE_CMD) && is_right))
        {
            if(keymap_r_tab[i].status.code == status->code)
            {
                if (keymap_r_tab[i].status.event == status->event)
                {
                    // BESUI_TRACE(0, "key right = %d %d %d %d", i, is_right, status->code, status->event);
                    button_cmd_execute(keymap_r_tab[i].button_set_event);
                    if(status->event < 16)
                        BESUI_TRACE(0, "[UIMAP], [%s]", event_str[status->event]);
                }
            }
        }
    }
}

void app_tota_general_button_event_init(void)
{
    nv_record_env_get(&nvrecord_uienv);

    if(nvrecord_uienv->init_button == 0xA5)
    {
#if defined(DOLBY_AUDIO_ENABLE)
        uictl.dolby_onoff_sta = nvrecord_uienv->dolby_onoff;
        uictl.dolby_mode = nvrecord_uienv->dolby_mode;
#endif
#ifdef BESSPA_ONOFF_EN
        stereo_surround_status = nvrecord_uienv->space_audio_type;
#endif
#if defined(BESUI_LATENCY_EN) && defined(LATENCY_NV_EN) 
        uicom.latency_mode = nvrecord_uienv->remember_game_mode;
#endif
        memcpy(keymap_l_tab, nvrecord_uienv->buttonInfo_left, sizeof(keymap_l_tab));
        memcpy(keymap_r_tab, nvrecord_uienv->buttonInfo_right, sizeof(keymap_r_tab));
        return ;
    }
    BESUI_TRACE(0,"[UITOTA]%s", __func__);
    nvrecord_uienv->init_button = 0xA5;

#if defined(DOLBY_AUDIO_ENABLE)
    nvrecord_uienv->dolby_onoff = 0;
    nvrecord_uienv->dolby_mode = DOLBY_MODE_NATUAL;
#endif

#ifdef BESSPA_ONOFF_EN
    nvrecord_uienv->space_audio_type = 0;
#endif

    nvrecord_uienv->inear_left_onoff = 1;
    nvrecord_uienv->inear_right_onoff = 1;
    
#ifdef APP_MULTIPOINT_ONOFF_EN
    nvrecord_uienv->muiltipoint_onoff = 1;
#endif

#ifdef EQ_SET_CUSTOMER_EN
    nvrecord_uienv->eq_onoff = 2;
    nvrecord_uienv->eq_mode = 0;
#endif

#if defined(WEAR_DETECT_PROMPT_EN)
    nvrecord_uienv->wear_prompt_onoff = 0;
#endif

#if defined(EQ_CUSTOM_APP_EN)
    for(uint8_t i = 0;i < EQBAND_NUM;i ++)
        nvrecord_uienv->eq_custom[i] = (EQBAND_RANGE_DB/2);
#endif

#if defined(ANC_APP) && defined(USER_NOISE_ADAPTIVE_ANC_EN)
    nvrecord_uienv->anc_lmh_mode = 0;
#endif

    keymap_l_tab[0].lr = BUTTON_SETTING_LEFT_EARPHONE_CMD;
    keymap_l_tab[0].status.code = BUTTON_MAP_KEY_TYPE;
    keymap_l_tab[0].status.event = APP_KEY_EVENT_CLICK;
    keymap_l_tab[0].button_set_event = BUTTON_L_CLICK_DEFAULT;

    //init right button
    keymap_r_tab[0].lr = BUTTON_SETTING_RIGHT_EARPHONE_CMD;
    keymap_r_tab[0].status.code = BUTTON_MAP_KEY_TYPE;
    keymap_r_tab[0].status.event = APP_KEY_EVENT_CLICK;
    keymap_r_tab[0].button_set_event = BUTTON_R_CLICK_DEFAULT;

    keymap_l_tab[1].lr = BUTTON_SETTING_LEFT_EARPHONE_CMD;
    keymap_l_tab[1].status.code = BUTTON_MAP_KEY_TYPE;
    keymap_l_tab[1].status.event = APP_KEY_EVENT_DOUBLECLICK;
    keymap_l_tab[1].button_set_event = BUTTON_L_DOUBLE_DEFAULT;

    //init right button
    keymap_r_tab[1].lr = BUTTON_SETTING_RIGHT_EARPHONE_CMD;
    keymap_r_tab[1].status.code = BUTTON_MAP_KEY_TYPE;
    keymap_r_tab[1].status.event = APP_KEY_EVENT_DOUBLECLICK;
    keymap_r_tab[1].button_set_event = BUTTON_R_DOUBLE_DEFAULT;

    keymap_l_tab[2].lr = BUTTON_SETTING_LEFT_EARPHONE_CMD;
    keymap_l_tab[2].status.code = BUTTON_MAP_KEY_TYPE;
    keymap_l_tab[2].status.event = APP_KEY_EVENT_TRIPLECLICK;
    keymap_l_tab[2].button_set_event = BUTTON_L_TRIPLE_DEFAULT;

    //init right button
    keymap_r_tab[2].lr = BUTTON_SETTING_RIGHT_EARPHONE_CMD;
    keymap_r_tab[2].status.code = BUTTON_MAP_KEY_TYPE;
    keymap_r_tab[2].status.event = APP_KEY_EVENT_TRIPLECLICK;
    keymap_r_tab[2].button_set_event = BUTTON_R_TRIPLE_DEFAULT;

    keymap_l_tab[3].lr = BUTTON_SETTING_LEFT_EARPHONE_CMD;
    keymap_l_tab[3].status.code = BUTTON_MAP_KEY_TYPE;
    keymap_l_tab[3].status.event = APP_KEY_EVENT_INITLONGPRESS;
    keymap_l_tab[3].button_set_event = BUTTON_L_LONG_PRESS_DEFAULT;

    //init right button
    keymap_r_tab[3].lr = BUTTON_SETTING_RIGHT_EARPHONE_CMD;
    keymap_r_tab[3].status.code = BUTTON_MAP_KEY_TYPE;
    keymap_r_tab[3].status.event = APP_KEY_EVENT_INITLONGPRESS;
    keymap_r_tab[3].button_set_event = BUTTON_R_LONG_PRESS_DEFAULT;

    memcpy(nvrecord_uienv->buttonInfo_left, keymap_l_tab, sizeof(keymap_l_tab));
    memcpy(nvrecord_uienv->buttonInfo_right, keymap_r_tab, sizeof(keymap_r_tab));
    nv_record_env_set(nvrecord_uienv);
    nv_record_flash_flush();
}

void app_tota_general_button_event_reset(void)
{
    nvrecord_uienv->init_button = 0;
    nv_record_env_set(nvrecord_uienv);
    nv_record_flash_flush();
}

void app_tota_general_button_event_info_set(uint8_t* ptrParam, uint16_t paramLen)
{
    uint8_t resData[5];
    uint8_t resLen;
    BESUI_TRACE(1,"[UIAPP], button ptrParam = %d, %d, %d, %d", ptrParam[0], ptrParam[1], ptrParam[2], ptrParam[3]);
   if(BUTTON_SETTING_LEFT_EARPHONE_CMD == ptrParam[1])
   {
        if(!bts_tws_if_is_tws_link_connected()&&!bts_tws_if_is_local_left_side())
        {
            resData[0] = ptrParam[0];
            resData[1] = ptrParam[1];
            resData[2] = ptrParam[2];
            resData[3] = ptrParam[3];
            resData[4] = 0;
            resLen = 0x05;
            app_tota_send_rsp(OP_TOTA_SET_CUSTOMER_CMD, TOTA_NO_ERROR, resData, resLen);
            return ;
        }
       if(BUTTON_SETTING_CLICK_CMD == ptrParam[2])
       {
           keymap_l_tab[0].status.code = BUTTON_MAP_KEY_TYPE;//if you have button parameters,set them here
           keymap_l_tab[0].status.event = APP_KEY_EVENT_CLICK;//8,if you have button event parameters,set them here
           keymap_l_tab[0].lr = ptrParam[1];
           keymap_l_tab[0].button_set_event = ptrParam[3];
       }
       else if(BUTTON_SETTING_DOUBLE_CLICK_CMD == ptrParam[2])
       {
           keymap_l_tab[1].status.code = BUTTON_MAP_KEY_TYPE;//if you have button parameters,set them here
           keymap_l_tab[1].status.event = APP_KEY_EVENT_DOUBLECLICK;//if you have button event parameters,set them here
           keymap_l_tab[1].lr = ptrParam[1];
           keymap_l_tab[1].button_set_event = ptrParam[3];
       }
       else if(BUTTON_SETTING_TRIPLE_CLICK_CMD == ptrParam[2])
       {
           keymap_l_tab[2].status.code = BUTTON_MAP_KEY_TYPE;
           keymap_l_tab[2].status.event = APP_KEY_EVENT_TRIPLECLICK;
           keymap_l_tab[2].lr = ptrParam[1];
           keymap_l_tab[2].button_set_event = ptrParam[3];
       }
       else if(BUTTON_SETTING_LONG_PRESS_CMD == ptrParam[2])
       {
           keymap_l_tab[3].status.code = BUTTON_MAP_KEY_TYPE;
           keymap_l_tab[3].status.event = APP_KEY_EVENT_INITLONGPRESS;
           keymap_l_tab[3].lr = ptrParam[1];
           keymap_l_tab[3].button_set_event = ptrParam[3];
       }
        //write to flash
        BESUI_TRACE(1,"write flash, %d",  sizeof(keymap_l_tab));
        memcpy(nvrecord_uienv->buttonInfo_left, keymap_l_tab, sizeof(keymap_l_tab));
        //DUMP8("%0x2", &(nvrecord_uienv->buttonInfo_left),  sizeof(keymap_l_tab));
        nv_record_env_set(nvrecord_uienv);
        //nv_record_flash_flush();
   }
   else
   {
        if(!bts_tws_if_is_tws_link_connected()&&bts_tws_if_is_local_left_side())
        {
            resData[0] = ptrParam[0];
            resData[1] = ptrParam[1];
            resData[2] = ptrParam[2];
            resData[3] = ptrParam[3];
            resData[4] = 0;
            resLen = 0x05;
            app_tota_send_rsp(OP_TOTA_SET_CUSTOMER_CMD, TOTA_NO_ERROR, resData, resLen);
            return ;
        }
       if(BUTTON_SETTING_CLICK_CMD == ptrParam[2])
       {
           keymap_r_tab[0].status.code = BUTTON_MAP_KEY_TYPE;//if you have button parameters,set them here
           keymap_r_tab[0].status.event = APP_KEY_EVENT_CLICK;//8,if you have button event parameters,set them here
           keymap_r_tab[0].lr = ptrParam[1];
           keymap_r_tab[0].button_set_event = ptrParam[3];
       }
       else if(BUTTON_SETTING_DOUBLE_CLICK_CMD == ptrParam[2])
       {
           keymap_r_tab[1].status.code = BUTTON_MAP_KEY_TYPE;//if you have button parameters,set them here
           keymap_r_tab[1].status.event = APP_KEY_EVENT_DOUBLECLICK;//if you have button event parameters,set them here
           keymap_r_tab[1].lr = ptrParam[1];
           keymap_r_tab[1].button_set_event = ptrParam[3];
       }
       else if(BUTTON_SETTING_TRIPLE_CLICK_CMD == ptrParam[2])
       {
           keymap_r_tab[2].status.code = BUTTON_MAP_KEY_TYPE;
           keymap_r_tab[2].status.event = APP_KEY_EVENT_TRIPLECLICK;
           keymap_r_tab[2].lr = ptrParam[1];
           keymap_r_tab[2].button_set_event = ptrParam[3];
       }
       else if(BUTTON_SETTING_LONG_PRESS_CMD == ptrParam[2])
       {
           keymap_r_tab[3].status.code = BUTTON_MAP_KEY_TYPE;
           keymap_r_tab[3].status.event = APP_KEY_EVENT_INITLONGPRESS;
           keymap_r_tab[3].lr = ptrParam[1];
           keymap_r_tab[3].button_set_event = ptrParam[3];
       }

       /* keymap_r_tab.status.code = APP_KEY_CODE_FN1;//if you have button parameters,set them here
        if(ptrParam[2] == BUTTON_SETTING_CLICK_CMD)
        {
             keymap_r_tab.status.event = APP_KEY_EVENT_CLICK;//8,if you have button event parameters,set them here
        }
        else if(ptrParam[2] == BUTTON_SETTING_DOUBLE_CLICK_CMD)
        {
            keymap_r_tab.status.event = APP_KEY_EVENT_DOUBLECLICK;//if you have button event parameters,set them here
        }
    
        keymap_r_tab.lr = ptrParam[1];
        keymap_r_tab.button_set_event = ptrParam[3];*/
    
        //write to flash
        BESUI_TRACE(1,"write flash, %d",  sizeof(keymap_r_tab));
        memcpy(nvrecord_uienv->buttonInfo_right, keymap_r_tab, sizeof(keymap_r_tab));
       // DUMP8("%0x2", &(nvrecord_uienv->buttonInfo_right),  sizeof(keymap_r_tab));
        nv_record_env_set(nvrecord_uienv);
        //nv_record_flash_flush();
   }

    if(TWS_UI_SLAVE != app_ibrt_if_get_ui_role())
    {
        resData[0] = ptrParam[0];
        resData[1] = ptrParam[1];
        resData[2] = ptrParam[2];
        resData[3] = ptrParam[3];
        resData[4] = 1;
        resLen = 0x05;
        app_tota_send_rsp(OP_TOTA_SET_CUSTOMER_CMD, TOTA_NO_ERROR, resData, resLen);
    }
}
#ifdef SUPPORT_SIRI
void app_tota_general_button_event_sync(uint8_t* ptrParam, uint16_t paramLen)
{
    uint8_t cmd = ptrParam[0];
    button_cmd_execute(cmd);
}
#endif

bool app_inear_get_right_status(void)
{
    BESUI_TRACE(0,"[UIAPP]%s, right_status = %d", __func__, nvrecord_uienv->right_status);
    if(app_ibrt_if_is_right_side())
    {
#if defined(CAPSENSOR_ENABLE)
        return app_inear_get_status();
#else
        return 0;
#endif
    }
    else
    {
        return uictl.ear_another_sta;
    }
}

bool app_inear_get_left_status(void)
{
    BESUI_TRACE(0,"[UIAPP]%s, left_status = %d", __func__, nvrecord_uienv->left_status);
    if(bts_tws_if_is_local_left_side())
    {
#if defined(CAPSENSOR_ENABLE)
        return app_inear_get_status();
#else
        return 0;
#endif
    }
    else
    {
        return uictl.ear_another_sta;
    }
}

bool app_inear_get_myself_status(void)
{
#if defined(CAPSENSOR_ENABLE)
    return app_inear_get_status();
#else
    return 0;
#endif
}

#ifdef ALGO_INFO_SYNC_EN
void app_tota_algo_send_to_app(APP_TOTA_CMD_CODE_E rsp_opCode,uint8_t cmd_type, uint8_t ptrParam1, uint8_t ptrParam2, uint8_t ptrParam3, uint32_t paramLen)
{
    uint8_t resData[5]={0};
    resData[0] = cmd_type;
    resData[1] = ptrParam1;
    resData[2] = ptrParam2;
    resData[3] = ptrParam3;
    BESUI_TRACE(4,"[UIAPP]%s, %02X %02X %02X %02X", __func__, resData[0], resData[1], resData[2], resData[3]);
    app_tota_send_rsp(rsp_opCode, TOTA_NO_ERROR, resData, paramLen);
}
#endif

#ifdef USER_APP_BLE_DIS_EN
void app_tota_switch_role_to_app(void) //send to app before switch role
{
#ifndef FREEMAN_ENABLED_STERO
    BESUI_TRACE(0, "[UIAPP]%s", __func__);
    uint8_t resData[5]={0};
#ifdef USER_TOTA_SPP_SYNC_KEY_EN
    if(spp_connect_sta() != true)
#endif
    {
        resData[0] = APP_TOTA_SWITCH_ROLE;
        memcpy((uint8_t *)(resData+1), (uint8_t *)(uictl.random_local+0), 2);
        memcpy((uint8_t *)(resData+3), (uint8_t *)(uictl.random_peer+0), 2);
        app_tota_send_rsp(OP_TOTA_SET_CUSTOMER_CMD, TOTA_NO_ERROR, resData, 5);
    }
    app_tota_tws_status_to_app(0xFF);
    BESUI_TRACE(0, "[UIAPP]%s, %02X, random=%02X:%02X-%02X:%02X", __func__, resData[0], resData[1], resData[2], resData[3], resData[4]);
#endif
}
#endif

void app_tota_phone_num_to_app(uint8_t num)
{
    uint8_t resData[8]={0};
    resData[0] = APP_TOTA_GET_BT_ADDR_CMD;
    if(num != 0xFF)
        resData[1] = num;
    else
        resData[1] = besui_get_profile_conn_num();

    sys_memcpy_swap(&resData[2],bt_global_addr,6);

    app_tota_send_rsp(OP_TOTA_SET_CUSTOMER_CMD, TOTA_NO_ERROR, resData, 8);
    BESUI_TRACE(0,"[UIAPP]%s", __func__);
    DUMP8("%02X ", resData, 8);
}

void app_tota_tws_status_to_app(uint8_t sta)
{
#ifndef FREEMAN_ENABLED_STERO
    uint8_t resData[3]={0};
    resData[0] = APP_TOTA_TWS_STATUS;
    if(sta != 0xFF)
        resData[1] = sta;
    else
        resData[1] = bts_tws_if_is_tws_link_connected();

    if(bts_tws_if_is_local_left_side())
    {
        if(IBRT_SLAVE != app_ibrt_if_get_ui_role())
            resData[2] = 1;
    }
    else
    {
        if(IBRT_SLAVE != app_ibrt_if_get_ui_role())
            resData[2] = 2;
    }
    BESUI_TRACE(4,"[UIAPP]%s, %02X %02X %02X", __func__, resData[0], resData[1], resData[2]);
    app_tota_send_rsp(OP_TOTA_SET_CUSTOMER_CMD, TOTA_NO_ERROR, resData, 3);
#endif
}

bool wear_det_onoff_get(void)
{
    bool flag = false;
    if(nvrecord_uienv->inear_left_onoff == 0 && bts_tws_if_is_local_left_side())
    {
        flag = true;
    }
    if(nvrecord_uienv->inear_right_onoff == 0 && app_ibrt_if_is_right_side())
    {
        flag = true;
    } 
    BESUI_TRACE(0, "[UIAPP]%s, flag = %d", __func__, flag);  
    return flag;
}

#ifdef ANC_APP
void app_tota_anc_send_to_app(uint8_t param)
{
    BESUI_TRACE(0,"[UIAPP]%s, param = %d", __func__, param);
#if defined(USER_NOISE_ADAPTIVE_ANC_EN)
    if(besui_anc_adapt_get_onoff()) //anc adapt
    {
        param = 0x01;
    }
    else
    {
        if(besui_get_curr_anc_mode() == APP_ANC_MODE1)
            param = 0x05;
        else if(besui_get_curr_anc_mode() == APP_ANC_MODE2)
            param = 0x06;
        else if(besui_get_curr_anc_mode() == APP_ANC_MODE3)
            param = 0x07;
        else if(besui_get_curr_anc_mode() == APP_ANC_MODE4)
            param = 0x02;
    }
#endif
    if(param == 0)
        param = 4;
    app_tota_algo_send_to_app(OP_TOTA_SET_CUSTOMER_CMD, APP_TOTA_REGULATING_ANC, param, 0, 0, 2);
}
#endif

#if defined(BESUI_TWS_EN) || defined(BESUI_STEREO_EN)
void besui_app_clear_all_nvrecord(void)
{
    BESUI_TRACE(0,"[UIAPP]%s", __func__);
    if(bts_tws_if_is_tws_link_connected())
    {
        if(TWS_UI_MASTER == app_ibrt_if_get_ui_role())
            tws_ctrl_send_cmd(APP_TWS_CMD_SYNC_TOTA_FACTORY_RESET, NULL, 0); ///>send to slave        
    }
  
    app_tota_general_button_event_reset();
    nv_record_rebuild(NV_REBUILD_SDK_ONLY);

    ota_disconnect();
    bes_ble_gap_disconnect_all();

    nv_record_env_get(&nvrecord_uienv);
#ifdef LATENCY_NV_EN
    nvrecord_uienv->remember_game_mode = 0;
#endif
#if defined(__VIRTUAL_SURROUND__) || defined(__VIRTUAL_SURROUND_CP__) || defined(__VIRTUAL_SURROUND_STEREO__)
    nvrecord_uienv->space_audio_type = 0;  
#endif
#if defined(ANC_APP) && defined(BESUI_TWS_EN)
    nvrecord_uienv->sync_anc_flag = false;
    nvrecord_uienv->sync_anc_mode = 0;
    nvrecord_uienv->sync_anc_adapt = false;    
#endif
    nv_record_env_set(nvrecord_uienv);

    app_reset_factory_timer_onoff();
}
#endif

static void app_tota_general_cmd_rx_process(uint8_t* ptrParam, uint32_t paramLen)
{
    uint8_t custom_cmd_type = ptrParam[0];
    uint8_t resData[20]={0};
    uint32_t resLen=1;

    uint8_t device_id = app_bt_audio_get_device_for_user_action();

    BESUI_TRACE(0,"[UIAPP]%s", __func__);
    DUMP8("%02x ", ptrParam, paramLen);

    nv_record_env_get(&nvrecord_uienv);

#ifdef BESUI_TWS_EN
    if(app_ibrt_if_is_ui_slave()) //compatible SPP&BLE
    {
        BESUI_TRACE(0,"[%s], slave return, cmd: %d;", __func__, custom_cmd_type);
        return ;
    }
#endif
    if(APP_TOTA_BUTTON_SETTINGS_CONTROL_CMD == custom_cmd_type)
    {
#ifdef BESUI_TWS_EN
        if(bts_tws_if_is_tws_link_connected())
        {
            tws_ctrl_send_cmd(APP_TWS_CMD_SYNC_TOTA_BUTTON_SETTINGS_CONTROL, ptrParam, (uint16_t)paramLen); ///>send to slave
        }
        app_tota_general_button_event_info_set(ptrParam, (uint16_t)paramLen);
#else
        stereo_tota_button_info_set(ptrParam, (uint16_t)paramLen);
#endif
        return;
    }
    else if( APP_TOTA_FACTORY_RESET_CMD == custom_cmd_type)
    {
        BESUI_TRACE(1,"[UIAPP]custom: factory reset  connect path %d",tota_get_connect_path());
        static bool rst_flag = 1;
        if(rst_flag)
        {
            rst_flag = 0;
            besui_app_clear_all_nvrecord();
        }
        return ;
    }
    else if(APP_TOTA_MUSIC_PLAY_SETTINGS_CMD == custom_cmd_type)
    {
        if(MUSIC_PLAY_SETTINGS_PLAY_CMD == ptrParam[1])
        {
            app_ibrt_if_a2dp_send_play(device_id);
        }
        else if(MUSIC_PLAY_SETTINGS_PAUSE_CMD == ptrParam[1])
        {
            app_ibrt_if_a2dp_send_pause(device_id);
        }
        else if(MUSIC_PLAY_SETTINGS_NEXT_CMD == ptrParam[1])
        {
            app_ibrt_if_a2dp_send_forward(device_id);
        }
        else if(MUSIC_PLAY_SETTINGS_PRE_CMD == ptrParam[1])
        {
            app_ibrt_if_a2dp_send_backward(device_id);
        }
        else
        {
            BESUI_TRACE(0,"[UIAPP]error command");
        }
        return ;
    }
    else if(APP_TOTA_BATTERY_LEVEL_CMD == custom_cmd_type)
    {
        if(ptrParam[1] == INEAR_DETECT_RIGHT_EAR_CMD)
        {
            if(app_ibrt_if_is_right_side())
            {
                uint8_t battery_levels = (app_battery_current_level()+1) * 10;
                if(battery_levels >= 100)
                    battery_levels = 100;
                resData[0] = APP_TOTA_BATTERY_LEVEL_CMD;
                resData[1] = INEAR_DETECT_RIGHT_EAR_CMD;
                resData[2] = battery_levels;
                resLen = 0x03;
                app_tota_send_rsp(OP_TOTA_SET_CUSTOMER_CMD, TOTA_NO_ERROR, resData, resLen);
            }
            else
            {
#ifdef BESUI_TWS_EN
                if(bts_tws_if_is_tws_link_connected())
                {
                   tws_ctrl_send_cmd(APP_TWS_CMD_SEND_TOTA_BATTERY_LEVEL_CMD, &ptrParam[1], 1);
                }
#endif
            }
        }
        else if(ptrParam[1] == INEAR_DETECT_LEFT_EAR_CMD)
        {
            if(bts_tws_if_is_local_left_side())
            {
                uint8_t battery_levels = (app_battery_current_level()+1) * 10;
                if(battery_levels >= 100)
                    battery_levels = 100;
                resData[0] = APP_TOTA_BATTERY_LEVEL_CMD;
                resData[1] = INEAR_DETECT_LEFT_EAR_CMD;
                resData[2] = battery_levels;
                resLen = 0x03;
                app_tota_send_rsp(OP_TOTA_SET_CUSTOMER_CMD, TOTA_NO_ERROR, resData, resLen);
            }
            else
            {
#ifdef BESUI_TWS_EN
                if(bts_tws_if_is_tws_link_connected())
                {
                   tws_ctrl_send_cmd(APP_TWS_CMD_SEND_TOTA_BATTERY_LEVEL_CMD, &ptrParam[1], 1);
                }
#endif
            }
        }
        else if(ptrParam[1] == INEAR_DETECT_BOX_CMD)
        {
            uint8_t battery_levels = 0;
#if defined(BESUI_TWS_EN)
            battery_levels = uicom.bat_box_percent;
#endif
            if(battery_levels >= 100)
                battery_levels = 100;
            resData[0] = APP_TOTA_BATTERY_LEVEL_CMD;
            resData[1] = INEAR_DETECT_BOX_CMD;
            resData[2] = battery_levels;
            resLen = 0x03;
            app_tota_send_rsp(OP_TOTA_SET_CUSTOMER_CMD, TOTA_NO_ERROR, resData, resLen);
        }
        return;
    }
#if defined(VOICE_ASSIST_CUSTOM_LEAK_DETECT)
    else if(APP_TOTA_EARBUD_FIT_TEST_CMD == custom_cmd_type)
    {
        BESUI_TRACE(0, "[UIAPP]APP_TOTA_EARBUD_FIT_TEST_CMD");
        return ;
    }
#endif
    else if(APP_TOAT_EQ_CMD == custom_cmd_type)
    {
        BESUI_TRACE(0,"[UIAPP]APP_TOAT_EQ_CMD");
#if defined(ALGO_INFO_SYNC_EN) && defined(EQ_SET_CUSTOMER_EN)
        ptrParam[2] = ptrParam[1];             
        ptrParam[1] = nvrecord_uienv->eq_onoff;
#ifdef BESUI_TWS_EN
        tws_ctrl_send_cmd(APP_TWS_CMD_SYNC_EQ, ptrParam, 3); ///>send to slave
#endif
        algo_send_request(ALGO_ID_EQ, nvrecord_uienv->eq_onoff, ptrParam[2], 0, 0, 0);
#endif
    }
    else if(APP_TOTA_INEAR_DETECT_CMD == custom_cmd_type)
    {
        resData[0] = APP_TOTA_INEAR_DETECT_CMD;
        if(INEAR_DETECT_STATUS_CMD == ptrParam[1])
        {
            BESUI_TRACE(0,"[UIAPP]INEAR_DETECT_STATUS_CMD");
            resData[1] = INEAR_DETECT_STATUS_CMD;
            if(paramLen == 2) //app get sta
            {
                if(nvrecord_uienv->inear_left_onoff != 1)
                    nvrecord_uienv->inear_left_onoff = 0;
                if(nvrecord_uienv->left_status != 1)
                    nvrecord_uienv->left_status = 0;
                if(nvrecord_uienv->inear_right_onoff != 1)
                    nvrecord_uienv->inear_right_onoff = 0;
                if(nvrecord_uienv->right_status != 1)
                    nvrecord_uienv->right_status = 0;
                resData[2] = nvrecord_uienv->inear_left_onoff;
                resData[3] = app_inear_get_left_status();//nvrecord_uienv->left_status;
                resData[4] = nvrecord_uienv->inear_right_onoff;
                resData[5] = app_inear_get_right_status();//nvrecord_uienv->right_status;
                resLen = 0x06;
                app_tota_send_rsp(OP_TOTA_SET_CUSTOMER_CMD,TOTA_NO_ERROR,resData,resLen);
            }
            else if(paramLen == 3) //app set sta
            {
                if(INEAR_DETECT_ON_CMD == ptrParam[2]) //left and right on
                {
                    BESUI_TRACE(0,"[UIAPP]wear detect all on"); 
                    nvrecord_uienv->inear_left_onoff = INEAR_DETECT_ON_CMD; 
                    nvrecord_uienv->left_status = app_inear_get_left_status();
                    nvrecord_uienv->inear_right_onoff = INEAR_DETECT_ON_CMD;
                    nvrecord_uienv->right_status = app_inear_get_right_status();
                }
                else                                  //left and right off               
                {
                    BESUI_TRACE(0,"[UIAPP]wear detect all off");
                    nvrecord_uienv->inear_left_onoff = 0x00;
                    nvrecord_uienv->left_status = 0x00;

                    nvrecord_uienv->inear_right_onoff = 0x00;
                    nvrecord_uienv->right_status = 0x00;
                }
                nv_record_env_set(nvrecord_uienv);

                if(bts_tws_if_is_tws_link_connected())
                {
                    resData[0] = TWS_SYNC_WEAR_M2S;
                    resData[1] = nvrecord_uienv->inear_left_onoff;
                    resData[2] = nvrecord_uienv->left_status;
                    resData[3] = nvrecord_uienv->inear_right_onoff;
                    resData[4] = nvrecord_uienv->right_status;
#ifdef BESUI_TWS_EN
                    tws_ctrl_send_cmd(APP_TWS_CMD_CAPSENSOR_WEAR, resData, 5);
#endif
                }

                resData[0] = ptrParam[0];
                resData[1] = ptrParam[1];
                resData[2] = nvrecord_uienv->inear_left_onoff;
                resData[3] = nvrecord_uienv->left_status;
                resData[4] = nvrecord_uienv->inear_right_onoff;
                resData[5] = nvrecord_uienv->right_status;
                resLen = 0x06;
                app_tota_send_rsp(OP_TOTA_SET_CUSTOMER_CMD, TOTA_NO_ERROR, resData, resLen);
            }
        }
        else if(INEAR_DETECT_RIGHT_EAR_CMD == ptrParam[1])     //right ear is master
        { 
            BESUI_TRACE(0,"[UIAPP]INEAR_DETECT_RIGHT_EAR_CMD");
            if(bts_tws_if_is_local_left_side())
            {
                if(bts_tws_if_is_tws_link_connected())
                {
                    nvrecord_uienv->inear_right_onoff = ptrParam[2];
                    resData[0] = TWS_SYNC_WEAR_M2S;
                    resData[1] = nvrecord_uienv->inear_left_onoff;
                    resData[2] = nvrecord_uienv->left_status;
                    resData[3] = nvrecord_uienv->inear_right_onoff;
                    resData[4] = nvrecord_uienv->right_status;
#ifdef BESUI_TWS_EN
                    tws_ctrl_send_cmd(APP_TWS_CMD_CAPSENSOR_WEAR, resData, 5);
#endif
                }
                else
                {
                    resData[0] = APP_TOTA_INEAR_DETECT_CMD;
                    resData[1] = INEAR_DETECT_RIGHT_EAR_CMD;
                    resData[2] = 0;
                    resData[3] = 0;
                    resLen = 0x04;
                    app_tota_send_rsp(OP_TOTA_SET_CUSTOMER_CMD, TOTA_NO_ERROR, resData, resLen);
                    return ;
                }
            }

            if(INEAR_DETECT_ON_CMD == ptrParam[2])
            {
                nvrecord_uienv->inear_right_onoff = INEAR_DETECT_ON_CMD;
            }
            else
            {
                nvrecord_uienv->inear_right_onoff = INEAR_DETECT_OFF_CMD;
                nvrecord_uienv->right_status = 0x00;
            }

            resData[0] = APP_TOTA_INEAR_DETECT_CMD;
            resData[1] = INEAR_DETECT_RIGHT_EAR_CMD;
            resData[2] = nvrecord_uienv->inear_right_onoff;
            resData[3] = app_inear_get_right_status();//nvrecord_uienv->right_status;
            resLen = 0x04;
            app_tota_send_rsp(OP_TOTA_SET_CUSTOMER_CMD, TOTA_NO_ERROR, resData, resLen);
            nv_record_env_set(nvrecord_uienv);
        }
        else if(INEAR_DETECT_LEFT_EAR_CMD == ptrParam[1])       //left is slave
        {
            BESUI_TRACE(0,"[UIAPP]INEAR_DETECT_LEFT_EAR_CMD");
            if(app_ibrt_if_is_right_side())
            {
                if(bts_tws_if_is_tws_link_connected())
                {
                    nvrecord_uienv->inear_left_onoff = ptrParam[2];
                    resData[0] = TWS_SYNC_WEAR_M2S;
                    resData[1] = nvrecord_uienv->inear_left_onoff;
                    resData[2] = nvrecord_uienv->left_status;
                    resData[3] = nvrecord_uienv->inear_right_onoff;
                    resData[4] = nvrecord_uienv->right_status;
#ifdef BESUI_TWS_EN
                    tws_ctrl_send_cmd(APP_TWS_CMD_CAPSENSOR_WEAR, resData, 5);
#endif
                }
                else
                {
                    resData[0] = APP_TOTA_INEAR_DETECT_CMD;
                    resData[1] = INEAR_DETECT_LEFT_EAR_CMD;
                    resData[2] = 0;
                    resData[3] = 0;
                    resLen = 0x04;
                    app_tota_send_rsp(OP_TOTA_SET_CUSTOMER_CMD, TOTA_NO_ERROR, resData, resLen);
                    return ;
                }
            }

            if(INEAR_DETECT_ON_CMD == ptrParam[2])
            {
                nvrecord_uienv->inear_left_onoff = INEAR_DETECT_ON_CMD;
            }
            else
            {
                nvrecord_uienv->inear_left_onoff = INEAR_DETECT_OFF_CMD;
                nvrecord_uienv->left_status = 0x00;
            }

            resData[0] = APP_TOTA_INEAR_DETECT_CMD;
            resData[1] = INEAR_DETECT_LEFT_EAR_CMD;
            resData[2] = nvrecord_uienv->inear_left_onoff;
            resData[3] = app_inear_get_left_status();//nvrecord_uienv->left_status;
            resLen = 0x04;
            app_tota_send_rsp(OP_TOTA_SET_CUSTOMER_CMD, TOTA_NO_ERROR, resData, resLen);
            nv_record_env_set(nvrecord_uienv);
        }
        else if(INEAR_DETECT_PROMPT == ptrParam[1])
        {
            BESUI_TRACE(0,"[UIAPP]INEAR_DETECT_PROMPT");
#if defined(WEAR_DETECT_PROMPT_EN)
            resData[1] = INEAR_DETECT_PROMPT;
            if(paramLen == 3) //app set sta
            {
                nvrecord_uienv->wear_prompt_onoff = ptrParam[2]; 
                nv_record_env_set(nvrecord_uienv);
            }
            resData[2] = nvrecord_uienv->wear_prompt_onoff;
            resLen = 3;
            app_tota_send_rsp(OP_TOTA_SET_CUSTOMER_CMD, TOTA_NO_ERROR, resData, resLen);
            if(bts_tws_if_is_tws_link_connected())
            {
                resData[0] = TWS_SYNC_WEAR_PROMPT;
                resData[1] = nvrecord_uienv->wear_prompt_onoff;
                tws_ctrl_send_cmd(APP_TWS_CMD_CAPSENSOR_WEAR, resData, 2);
            }
#endif
        }
        return ;
    }
    else if(APP_TOTA_GET_SPP_STATUS_CMD == custom_cmd_type)
    {
        uint8_t ret = 0;
        // resData[0] = custom_cmd_type;
        if(tota_get_connect_path() == 0)
            ret = 1;
        else
            ret = 2;
        resData[0] = custom_cmd_type;
        resData[1] = ret;
        resLen = 2;
        BESUI_TRACE(0,"[UIAPP]APP_TOTA_GET_SPP_STATUS : %d",ret);
        app_tota_send_rsp(OP_TOTA_SET_CUSTOMER_CMD, TOTA_NO_ERROR, resData, resLen);
        return ;
    }
    else if(APP_TOTA_GET_BT_ADDR_CMD == custom_cmd_type)
    {
        resData[0] = custom_cmd_type;
        resData[1] = besui_get_profile_conn_num();
        sys_memcpy_swap(&resData[2],bt_global_addr,6);
        resLen = 8;
        BESUI_TRACE(0,"[UIAPP]APP_TOTA_GET_APP_ADDR ");
        app_tota_send_rsp(OP_TOTA_SET_CUSTOMER_CMD, TOTA_NO_ERROR, resData, resLen);
        return ;
    }
    else if(APP_TOTA_GET_CURRENT_BUTTON_STATE == custom_cmd_type)
    {
        BESUI_TRACE(0,"[UIAPP]APP_TOTA_GET_CURRENT_BUTTON_STATE ");
        resData[0] = custom_cmd_type;
        resData[1] = BUTTON_SETTING_LEFT_CLICK_CMD;
        resData[2] = nvrecord_uienv->buttonInfo_left[0].button_set_event;
        resData[3] = BUTTON_SETTING_RIGHT_CLICK_CMD;
        resData[4] = nvrecord_uienv->buttonInfo_right[0].button_set_event;
        resData[5] = BUTTON_SETTING_LEFT_DOUBLE_CMD;
        resData[6] = nvrecord_uienv->buttonInfo_left[1].button_set_event;
        resData[7] = BUTTON_SETTING_RIGHT_DOUBLE_CMD;
        resData[8] = nvrecord_uienv->buttonInfo_right[1].button_set_event;
        resData[9] = BUTTON_SETTING_LEFT_TRIPLE_CMD;
        resData[10] = nvrecord_uienv->buttonInfo_left[2].button_set_event;
        resData[11] = BUTTON_SETTING_RIGHT_TRIPLE_CMD;
        resData[12] = nvrecord_uienv->buttonInfo_right[2].button_set_event;
        resData[13] = BUTTON_SETTING_LEFT_LONG_PRESS_CMD;
        resData[14] = nvrecord_uienv->buttonInfo_left[3].button_set_event;
        resData[15] = BUTTON_SETTING_RIGHT_LONG_PRESS_CMD;
        resData[16] = nvrecord_uienv->buttonInfo_right[3].button_set_event;
        
        resLen = 17;
        app_tota_send_rsp(OP_TOTA_SET_CUSTOMER_CMD, TOTA_NO_ERROR, resData, resLen);
        return ;

    }
    else if(APP_TOTA_GET_PRODUCT_MODEL == custom_cmd_type)
    {
        BESUI_TRACE(0,"[UIAPP]GET_PRODUCT_MODEL");
        const char* model_id = BESUI_PROJECT_ID;

        uint8_t  model_id_len = strlen(model_id);
        resLen = model_id_len>57? 57:model_id_len;
        resData[0] = custom_cmd_type;
        resData[1] = (uint8_t) resLen;
        resData[2] = (uint8_t) resLen>>8;
        memcpy(&resData[3], model_id, resLen);
        resLen += 3;
        app_tota_send_rsp(OP_TOTA_SET_CUSTOMER_CMD, TOTA_NO_ERROR, resData, resLen);
        return ;
   
    }
    else if(APP_TOTA_REGULATING_ANC == custom_cmd_type)
    {
        BESUI_TRACE(0,"[UIAPP]APP_TOTA_REGULATING_ANC");
        uint8_t ANC_STATUS = 0;
        if(APP_TOTA_GET_ANC_STATUS == ptrParam[1])
        {
            BESUI_TRACE(0,"[UIAPP]GET_ANC_STATUS");
#if defined(ANC_APP)
            app_tota_anc_send_to_app(app_anc_get_curr_mode());
#endif
        }
        else if(APP_TOTA_SET_ANC_STATUS == ptrParam[1])
        {
            BESUI_TRACE(0,"[UIAPP]SET_ANC_STATUS");
#ifdef ALGO_INFO_SYNC_EN
#if defined(ANC_APP) && defined(USER_NOISE_ADAPTIVE_ANC_EN)
            if(app_bt_audio_count_connected_sco())
            {
                BESUI_TRACE(0,"[UIAPP]sco can not open anc");
                return;
            }
            BESUI_TRACE(0,"[UIAPP]anc_lmh_mode = %d", nvrecord_uienv->anc_lmh_mode);
            if(ptrParam[2] == 0x56) //app click anc mode (adapt low mid high)
                ANC_STATUS = nvrecord_uienv->anc_lmh_mode?nvrecord_uienv->anc_lmh_mode:7;
            else
                ANC_STATUS = ptrParam[2];
            //1-adapt  2-Ambeint  4-off  5-anc low  6-anc mid  7-anc mid
            if(ANC_STATUS == 1)
            {
                if(app_anc_get_curr_mode() == APP_ANC_MODE4 || app_anc_get_curr_mode() == APP_ANC_MODE_OFF)
                    algo_send_request(ALGO_ID_ANC, 1, 3, 1, 0, 1); //ANC mode high voice mode3 voice
                else
                    algo_send_request(ALGO_ID_ANC, 1, 3, 0, 0, 1);
            }
            else
            {
                if((ANC_STATUS>=5 &&ANC_STATUS<=7) && (app_anc_get_curr_mode() == APP_ANC_MODE4 || app_anc_get_curr_mode() == APP_ANC_MODE_OFF))
                    algo_send_request(ALGO_ID_ANC, ANC_STATUS, 0, 1, 0, 1); //need play prompt
                else
                    algo_send_request(ALGO_ID_ANC, ANC_STATUS, 0, 0, 0, 1);
            }
#else
            ANC_STATUS = ptrParam[2];
            algo_send_request(ALGO_ID_ANC, ANC_STATUS, 0, 0, 0, 1);
#endif
#endif
        }
        resData[0] = custom_cmd_type;
        resData[1] = ANC_STATUS;
        resLen = 2;
        app_tota_send_rsp(OP_TOTA_SET_CUSTOMER_CMD, TOTA_NO_ERROR, resData, resLen);
        return ;
    }
    else if(APP_TOTA_REGULATING_EQ == custom_cmd_type)
    {
        BESUI_TRACE(0,"[UIAPP]REGULATING_EQ");
#if defined(ALGO_INFO_SYNC_EN) && defined(EQ_SET_CUSTOMER_EN)
        uint8_t EQ_STATUS = 0;
        uint8_t EQ_MODE = 0;
#endif
        if(APP_TOTA_GET_EQ_ONOFF_STATUS == ptrParam[1])
        {
            BESUI_TRACE(0,"[UIAPP]GET_EQ_STATUS");
#if defined(ALGO_INFO_SYNC_EN) && defined(EQ_SET_CUSTOMER_EN)
            EQ_STATUS = nvrecord_uienv->eq_onoff;
#endif
        }
        else if(APP_TOTA_SET_EQ_ONOFF_STATUS == ptrParam[1])
        {
#ifdef ALGO_DELAY_ONOFF_EN
            algo_buf_clear();
#endif
            BESUI_TRACE(0,"[UIAPP]SET_EQ_STATUS");
#if defined(ALGO_INFO_SYNC_EN) && defined(EQ_SET_CUSTOMER_EN)
            EQ_STATUS = ptrParam[2];
            algo_send_request(ALGO_ID_EQ, EQ_STATUS, nvrecord_uienv->eq_mode, 0, 0, 0);
#if !defined(FREEMAN_ENABLED_STERO)
            ptrParam[1] = EQ_STATUS;
            ptrParam[2] = nvrecord_uienv->eq_mode;
            tws_ctrl_send_cmd(APP_TWS_CMD_SYNC_EQ, ptrParam, 3);
#endif
#endif
        }
        else if(APP_TOTA_SET_EQ_PARAMETER == ptrParam[1])
        {
#ifdef ALGO_DELAY_ONOFF_EN
            algo_buf_clear();
#endif
            BESUI_TRACE(0,"[UIAPP]SET_EQ_MODE");
#if defined(ALGO_INFO_SYNC_EN) && defined(EQ_SET_CUSTOMER_EN)
            EQ_MODE = ptrParam[2];
            algo_send_request(ALGO_ID_EQ, nvrecord_uienv->eq_onoff, EQ_MODE, 0, 0, 0);
#if !defined(FREEMAN_ENABLED_STERO)
            ptrParam[1] = nvrecord_uienv->eq_onoff;
            tws_ctrl_send_cmd(APP_TWS_CMD_SYNC_EQ, ptrParam, 3);
#endif
#endif
        }
#if defined(EQ_CUSTOM_APP_EN)
        else if(APP_TOTA_SET_EQ_CUSTOM == ptrParam[1])
        {
#ifdef ALGO_DELAY_ONOFF_EN
            algo_buf_clear();
#endif
            BESUI_TRACE(0,"[UIAPP]SET_EQ_CUSTOM");
#if defined(ALGO_INFO_SYNC_EN) && defined(EQ_SET_CUSTOMER_EN)
            nvrecord_uienv->eq_onoff = ALGO_OFF;
            nvrecord_uienv->eq_mode = 0xFF;
            memcpy(nvrecord_uienv->eq_custom, &ptrParam[2], EQBAND_NUM);
            nv_record_env_set(nvrecord_uienv);
            
            resData[0] = APP_TOTA_REGULATING_EQ;
            resData[1] = nvrecord_uienv->eq_onoff;
            resData[2] = nvrecord_uienv->eq_mode;
            memcpy(&resData[3], nvrecord_uienv->eq_custom, EQBAND_NUM);
#if !defined(FREEMAN_ENABLED_STERO)
            tws_ctrl_send_cmd(APP_TWS_CMD_SYNC_EQ, resData, 3+EQBAND_NUM);
#endif
            app_tota_send_rsp(OP_TOTA_SET_CUSTOMER_CMD, TOTA_NO_ERROR, resData, 3+EQBAND_NUM);
#if defined(ALGO_INFO_SYNC_EN)
            algo_send_request(ALGO_ID_EQ_CUSTOM, ALGO_ON, 0xFF, 0, 0, 0);
#endif
            return;
#endif
        }
        else if(APP_TOTA_GET_EQ_CUSTOM == ptrParam[1])
        {
            BESUI_TRACE(0,"[UIAPP]GET_EQ_CUSTOM");
#if defined(ALGO_INFO_SYNC_EN) && defined(EQ_SET_CUSTOMER_EN)
            resData[0] = APP_TOTA_REGULATING_EQ;//custom_cmd_type;
            resData[1] = nvrecord_uienv->eq_onoff;
            resData[2] = nvrecord_uienv->eq_mode;
            memcpy(&resData[3], nvrecord_uienv->eq_custom, EQBAND_NUM);
            app_tota_send_rsp(OP_TOTA_SET_CUSTOMER_CMD, TOTA_NO_ERROR, resData, 3+EQBAND_NUM);
#endif
            return;
        }
#endif //#if defined(EQ_CUSTOM_APP_EN)

#if defined(ALGO_INFO_SYNC_EN) && defined(EQ_SET_CUSTOMER_EN)
        resData[0] = custom_cmd_type;
        resData[1] = EQ_STATUS?EQ_STATUS:nvrecord_uienv->eq_onoff;
        resData[2] = EQ_MODE?EQ_MODE:nvrecord_uienv->eq_mode;
#endif
        resLen = 3;
        app_tota_send_rsp(OP_TOTA_SET_CUSTOMER_CMD, TOTA_NO_ERROR, resData, resLen);
        return ;
    }
#ifdef DOLBY_AUDIO_ENABLE
    else if(APP_TOTA_REGULATING_DOLBY == custom_cmd_type)
    {
        BESUI_TRACE(0,"[UIAPP]REGULATING_DOLBY");
        uint8_t DOLBY_STATUS = 0;
        if(APP_TOTA_GET_DOLBY_STATUS == ptrParam[1])
        {
            BESUI_TRACE(0,"[UIAPP]GET_DOLBY_STATUS");
            DOLBY_STATUS = dolby_audio_onoff_get();
            app_tota_algo_send_to_app(OP_TOTA_SET_CUSTOMER_CMD, APP_TOTA_REGULATING_DOLBY, 2-DOLBY_STATUS, uictl.dolby_mode, 0, 3);
        }
        else if(APP_TOTA_SET_DOLBY_STATUS_ONOFF == ptrParam[1])
        {
            BESUI_TRACE(0,"[UIAPP]SET_DOLBY_STATUS_ONOFF");
#if defined(ALGO_INFO_SYNC_EN)
#ifdef ALGO_DELAY_ONOFF_EN
#ifdef BESUI_TWS_EN
            media_PlayAudio(AUD_ID_SIGNATURE, 0);
#endif
            algo_id_buf = ALGO_ID_DOLBY;
            algo_onoff_buf = ALGO_OFF;
            algo_timer_onoff(true);
#else
            uint8_t DOLBY_STATUS_ONOFF = ptrParam[2];
            algo_send_request(ALGO_ID_DOLBY, DOLBY_STATUS_ONOFF, uictl.dolby_mode, 0, 0, 0);
#endif
#endif
        }
        else if(APP_TOTA_SET_DOLBY_STATUS == ptrParam[1])
        {
            BESUI_TRACE(0,"[UIAPP]SET_DOLBY_STATUS");
            DOLBY_STATUS = ptrParam[2];
#if defined(ALGO_INFO_SYNC_EN)
            if(DOLBY_STATUS == ALGO_ON)
            {
                //if(!bts_tws_if_is_tws_link_connected() || !boat_ear_sta_get(false) || !boat_ear_sta_get(true))
                if(!bts_tws_if_is_tws_link_connected())
                {
                    BESUI_TRACE(0,"[UIAPP]peer ear off");
                    app_tota_algo_send_to_app(OP_TOTA_SET_CUSTOMER_CMD, APP_TOTA_REGULATING_DOLBY, ALGO_OFF, uictl.dolby_mode, 0, 3);
                    return;
                }
            }
#ifdef ALGO_DELAY_ONOFF_EN
            algo_dolby_delay_onoff(ALGO_ON, DOLBY_STATUS);
#else
            algo_send_request(ALGO_ID_DOLBY, DOLBY_STATUS_ON, DOLBY_STATUS, 0, 0, 0);
#endif
#endif
        }
        return ;
    }
#endif //#ifdef DOLBY_AUDIO_ENABLE
#ifdef BESSPA_ONOFF_EN 
    else if(APP_TOTA_REGULATING_BES_SPATIAL == custom_cmd_type)
    {
        BESUI_TRACE(0,"[UIAPP]REGULATING_BES_SPATIAL");
        uint8_t BES_SPATIAL_STATUS = 0;
        if(APP_TOTA_GET_BES_SPATIAL_STATUS == ptrParam[1])
        {
            BESUI_TRACE(0,"[UIAPP]GET_BES_SPATIAL_STATUS");
            BES_SPATIAL_STATUS = besspa_audio_onoff_get();
            app_tota_algo_send_to_app(OP_TOTA_SET_CUSTOMER_CMD, APP_TOTA_REGULATING_BES_SPATIAL, 2-BES_SPATIAL_STATUS, 0, 0, 3);
        }
        else if(APP_TOTA_SET_BES_SPATIAL_STATUS == ptrParam[1])
        {
            BESUI_TRACE(0,"[UIAPP]SET_BES_SPATIAL_STATUS");
            BES_SPATIAL_STATUS = ptrParam[2];
#if defined(ALGO_INFO_SYNC_EN) && defined(BESSPA_ONOFF_EN)
            if(BES_SPATIAL_STATUS == ALGO_ON)
            {
                if(!bts_tws_if_is_tws_link_connected())
                {
                    BESUI_TRACE(0,"[UIAPP]peer ear off");
                    app_tota_algo_send_to_app(OP_TOTA_SET_CUSTOMER_CMD, APP_TOTA_REGULATING_BES_SPATIAL, ALGO_OFF, 0, 0, 3);
                    return;
                }
            }
            algo_send_request(ALGO_ID_BESSPA, BES_SPATIAL_STATUS, 0, 0, 0, 0);
#endif
        }    
        return ;
    }
#endif  //#ifdef BESSPA_ONOFF_EN  
    else if(APP_TOTA_REGULATING_MIMI == custom_cmd_type)
    {
        BESUI_TRACE(0,"[UIAPP]REGULATING_MIMI");
        return ;
    }
    else if(APP_TOTA_REGULATING_CEVA == custom_cmd_type)
    {
        BESUI_TRACE(0,"[UIAPP]REGULATING_CEVA");    
        return ;
    }
    else if(APP_TOTA_REGULATING_OTABOOT == custom_cmd_type)
    {
        BESUI_TRACE(0,"[UIAPP]APP_TOTA_REGULATING_OTABOOT");
        resData[0] = custom_cmd_type;
        resLen = 1;
        app_tota_send_rsp(OP_TOTA_SET_CUSTOMER_CMD,TOTA_NO_ERROR,resData,resLen);

#ifdef OTA_BOOT_COPY_EN
        // algo_send_request(ALGO_ID_OTABOOT, 0, 0, 0, 0, 0);
#endif
        return ;
    }
    else if(APP_TOTA_TWS_STATUS == custom_cmd_type)
    {
        BESUI_TRACE(0,"[UIAPP]%s, APP_TOTA_TWS_STATUS", __func__);
        resData[0] = custom_cmd_type;
        resData[1] = bts_tws_if_is_tws_link_connected();
        resData[2] = bts_tws_if_is_local_left_side()?1:2;
        resLen = 3;
        app_tota_send_rsp(OP_TOTA_SET_CUSTOMER_CMD,TOTA_NO_ERROR,resData,resLen);
        return ;
    }
#ifdef APP_MULTIPOINT_ONOFF_EN
    else if(APP_TOTA_MULTIPOINT == custom_cmd_type)
    {
        BESUI_TRACE(0,"[UIAPP]%s, APP_TOTA_MULTIPOINT", __func__);
        resData[0] = custom_cmd_type;
        // if(paramLen == 2) //get
        if(paramLen == 3)
        {
            if(0x00 == ptrParam[1]) //set
            {
                if(0x01 == ptrParam[2])
                    nvrecord_uienv->muiltipoint_onoff = 1;
                else
                    nvrecord_uienv->muiltipoint_onoff = 0;
                nv_record_env_set(nvrecord_uienv);
                algo_send_request(ALGO_ID_MULTIPOINT, nvrecord_uienv->muiltipoint_onoff, 0, 0, 0, 0);
            }
        }

        resData[1] = 0x01;
        resData[2] = nvrecord_uienv->muiltipoint_onoff;
        resLen = 3;
        app_tota_send_rsp(OP_TOTA_SET_CUSTOMER_CMD,TOTA_NO_ERROR,resData,resLen);
        return ;
    }  
#endif
#ifdef CODEC_TYPE_APP_EN
    else if(APP_TOTA_CODEC_TYPE == custom_cmd_type)
    {
        BESUI_TRACE(0,"[UIAPP]%s, APP_TOTA_CODEC_TYPE", __func__);
        codec_type_to_app();
        return ;
    }  
#endif

//------------------------------------------------------------------------------------------------------------
#ifdef APP_SYNC_GAME_EN
    else if(APP_TOTA_GAME == custom_cmd_type)
    {
        BESUI_TRACE(0,"[UIGAME]%s, APP_TOTA_GAME, onoff = %d", __func__, ptrParam[1]);
        if(0xFF == ptrParam[1])
        {
            app_sync_earbuds_latency(besui_latency_mode_get());            
        }
        else
        {
#ifdef BESUI_GAME_EN
            besui_gamemode_key_switch(false, ptrParam[1], true);
#endif
        }
        return ;
    }
#endif

//------------------------------------------------------------------------------------------------------------
#ifdef APP_SYNC_VOLUME_EN
    else if(APP_TOTA_VOLUME == custom_cmd_type)
    {
        BESUI_TRACE(0,"[UIVOL]%s, APP_TOTA_VOLUME, vol = %d", __func__, ptrParam[1]);
        if(0xFF == ptrParam[1]) //get vol
        {
            app_sync_earbuds_volume();     
        }
        else
        {
            uint8_t abs_volume = 8*ptrParam[1];
            if(abs_volume > 127)
                abs_volume = 127;
            app_ibrt_if_set_a2dp_current_abs_volume(0, abs_volume); //0~127  0~16
            BESUI_TRACE(0,"[UIVOL]%s, device_id = %d, vol = %d, abs = %d", __func__, 0, ptrParam[1], abs_volume);
        }
        return ;
    }  
#endif
//------------------------------------------------------------------------------------------------------------
    else
    {
        BESUI_TRACE(0,"[UIAPP]error custom cmd type");
    }
    app_tota_send_rsp(OP_TOTA_SET_CUSTOMER_CMD,TOTA_NO_ERROR,resData,resLen);
} //app_tota_general_cmd_rx_process


void app_tota_eq_status_event_report(uint8_t status,uint8_t mode)
{
    uint8_t resData[5]={0};
    uint8_t resLen = 0;
    resData[0] = APP_TOTA_REGULATING_EQ;
    resData[1] = status;
    resData[2] = mode;
    resLen = 3;
    app_tota_send_rsp(OP_TOTA_SET_CUSTOMER_CMD, TOTA_NO_ERROR, resData, resLen);
}

void slave_ear_to_master(void)
{
#ifdef BESUI_TWS_EN
    uint8_t resData[3]={0};
    uint8_t resLen = 0;

    if(TWS_UI_SLAVE == app_ibrt_if_get_ui_role())
    {
        BESUI_TRACE(0,"[UIAPP]%s", __func__);
        resData[0] = TWS_SYNC_WEAR_S2M;
        if(bts_tws_if_is_local_left_side())
            resData[1] = nvrecord_uienv->inear_left_onoff;
        else
            resData[1] = nvrecord_uienv->inear_right_onoff;
        resData[2] = app_inear_get_myself_status();
        resLen = 3;
        tws_ctrl_send_cmd(APP_TWS_CMD_CAPSENSOR_WEAR, resData, resLen);
    }
#endif
}

void app_tota_inear_status_event_report(void)
{
    uint8_t resData[5]={0};
    uint8_t resLen = 0;
    BESUI_TRACE(0,"[UIAPP]%s", __func__);
    //if(app_ibrt_if_is_ui_master())
    if(TWS_UI_SLAVE != app_ibrt_if_get_ui_role())
    {
        if(app_ibrt_if_is_right_side())
        {
            if(nvrecord_uienv->inear_right_onoff != 0)
            {
                nvrecord_uienv->right_status = app_inear_get_right_status();
                resData[0] = APP_TOTA_INEAR_DETECT_CMD;
                resData[1] = INEAR_DETECT_RIGHT_EAR_CMD;
                resData[2] = nvrecord_uienv->inear_right_onoff;
                resData[3] = nvrecord_uienv->right_status;
                resLen = 0x04;
                nv_record_env_set(nvrecord_uienv);
                //nv_record_flash_flush();
                app_tota_send_rsp(OP_TOTA_SET_CUSTOMER_CMD,TOTA_NO_ERROR,resData,resLen);
                
            }
        }
        else
        {
            if(nvrecord_uienv->inear_left_onoff != 0)
            {
                nvrecord_uienv->left_status = app_inear_get_left_status();
                resData[0] = APP_TOTA_INEAR_DETECT_CMD;
                resData[1] = INEAR_DETECT_LEFT_EAR_CMD;
                resData[2] = nvrecord_uienv->inear_left_onoff;
                resData[3] = nvrecord_uienv->left_status;
                resLen = 0x04;
                nv_record_env_set(nvrecord_uienv);
                //nv_record_flash_flush();
                app_tota_send_rsp(OP_TOTA_SET_CUSTOMER_CMD,TOTA_NO_ERROR,resData,resLen);
            }
        }
    }
    else
    {
        if(bts_tws_if_is_tws_link_connected())
        {
            resData[0] = TWS_SYNC_WEAR_S2M;
            if(bts_tws_if_is_local_left_side())
                resData[1] = nvrecord_uienv->inear_left_onoff;
            else
                resData[1] = nvrecord_uienv->inear_right_onoff;
            resData[2] = app_inear_get_myself_status();
            resLen = 3;
#ifdef BESUI_TWS_EN
            tws_ctrl_send_cmd(APP_TWS_CMD_CAPSENSOR_WEAR, resData, resLen);
#endif
        }
    }
}

void app_tota_send_inear_status(uint8_t *p_buff, uint8_t length)
{
    uint8_t resData[5]={0};
    uint32_t resLen=0x04;
    resData[0] = APP_TOTA_INEAR_DETECT_CMD;
    if(bts_tws_if_is_local_left_side())
    {
        resData[1] = INEAR_DETECT_RIGHT_EAR_CMD;
    }
    else
    {
        resData[1] = INEAR_DETECT_LEFT_EAR_CMD;
    }
    resData[2] = p_buff[0];
    resData[3] = p_buff[1];

    app_tota_send_rsp(OP_TOTA_SET_CUSTOMER_CMD,TOTA_NO_ERROR,resData,resLen);
}

void app_tota_sync_audio_eq_handle(uint8_t *p_buff, uint8_t length)
{
    bt_audio_updata_eq(*p_buff);
}

void app_tws_sync_battery_2_slave(uint8_t *p_buff, uint8_t length)
{
#ifdef BESUI_TWS_EN
    uint8_t resData[5]={0};
    uint8_t resLen = 0;
    uint8_t battery_levels =(app_battery_current_level()+1) * 10;
    if(battery_levels >= 100)
        battery_levels = 100;
    resData[0] = battery_levels;
    resLen = 0x01;
    tws_ctrl_send_cmd(APP_TWS_CMD_SEND_TOTA_BATTERY_LEVEL_RSP, resData, resLen);
#endif
}

void app_tota_sync_battery_level_rsp_handle(uint8_t *p_buff, uint8_t length)
{
    uint8_t resData[5]={0};
    uint8_t resLen = 0;

    resData[0] = APP_TOTA_BATTERY_LEVEL_CMD;
    if(!bts_tws_if_is_local_left_side())
    {
        resData[1] = INEAR_DETECT_LEFT_EAR_CMD;
    }
    else
    {
        resData[1] = INEAR_DETECT_RIGHT_EAR_CMD;
    }
    resData[2] = p_buff[0];
    resLen = 0x03;
    app_tota_send_rsp(OP_TOTA_SET_CUSTOMER_CMD, TOTA_NO_ERROR, resData, resLen);
}
#endif //#ifdef BESUI_APP_EN

static void __tota_general_cmd_handle(APP_TOTA_CMD_CODE_E funcCode, uint8_t* ptrParam, uint32_t paramLen)
{
    TOTA_V2_TRACE(3,"[%s]: opCode:0x%x, paramLen:%d", __func__, funcCode, paramLen);
    DUMP8("%02x ", ptrParam, paramLen);

    uint8_t resData[60]={0};
    uint32_t resLen=1;
    uint8_t volume_level;
#ifdef CAPSENSOR_TOTA_DATA
    struct BES_BT_DEVICE_T *curr_device = NULL;
    ibrt_mobile_info_t *p_mobile_info = NULL;
    bt_bdaddr_t *mobile_addr = NULL;
#endif

    switch (funcCode)
    {
        case OP_TOTA_GENERAL_INFO_CMD:
            __get_general_info();
            app_tota_v2_send_data(funcCode, (uint8_t*)&general_info, sizeof(general_info_t));
            return ;
        case OP_TOTA_EXCHANGE_MTU_CMD:
            tota_set_trans_MTU(*(uint16_t *)ptrParam);
            #if(BLE_APP_OTA_OVER_TOTA)
            ota_basic_mtu_set(*(uint16_t *)ptrParam - TOTA_PACKET_VERIFY_SIZE);
            #endif
           break;
        case OP_TOTA_GET_RSSI_CMD:
#ifdef BT_APP_RSSI
        {
            rssi_pkt_t pkt;
            app_rssi_update_date_pkt(&pkt);
            app_tota_v2_send_data(funcCode, (uint8_t *)&pkt, sizeof(rssi_pkt_t));
        }
#endif
            return;
        case OP_TOTA_VOLUME_PLUS_CMD:
            app_bt_volumeup();
            volume_level = app_bt_stream_local_volume_get();
            // resData[0] = volume_level;
            TOTA_V2_TRACE(1,"volume = %d",volume_level);
            break;
        case OP_TOTA_VOLUME_DEC_CMD:
            app_bt_volumedown();
            volume_level = app_bt_stream_local_volume_get();
            // resData[0] = volume_level;
            // resLen = 1;
            TOTA_V2_TRACE(1,"volume = %d",volume_level);
            break;
        case OP_TOTA_VOLUME_SET_CMD:
            //uint8_t scolevel = ptrParam[0];
            //uint8_t a2dplevel = ptrParam[1];
            app_bt_set_volume(APP_BT_STREAM_HFP_PCM,ptrParam[0]);
            app_bt_set_volume(APP_BT_STREAM_A2DP_SBC,ptrParam[1]);
            btapp_hfp_report_speak_gain();
            btapp_a2dp_report_speak_gain();
            break;
        case OP_TOTA_VOLUME_GET_CMD:
            resData[0] = app_bt_stream_hfpvolume_get();
            resData[1] = app_bt_stream_a2dpvolume_get();
            resLen = 2;
            app_tota_v2_send_data(funcCode, resData, resLen);
            return;
        case OP_TOTA_RAW_DATA_SET_CMD:
            // app_ibrt_debug_parse(ptrParam, paramLen);
            break;
        case OP_TOTA_GET_DUMP_INFO_CMD:
        {
            TOTA_DUMP_INFO_STRUCT_T dump_info;
            general_get_flash_dump_info(&dump_info);
            app_tota_send_rsp(funcCode,TOTA_NO_ERROR,(uint8_t*)&dump_info, sizeof(TOTA_DUMP_INFO_STRUCT_T));
            return ;
        }
        case OP_TOTA_SET_CUSTOMER_CMD:
#ifdef BESUI_APP_EN
            app_tota_general_cmd_rx_process(ptrParam, paramLen);
#endif
            return ;
        case OP_TOTA_GET_CUSTOMER_CMD:
            TOTA_V2_TRACE(0, "[%s] OP_TOTA_GET_CUSTOMER_CMD ...", __func__);
            break;
#ifdef BESUI_APP_EN
        case OP_TOTA_CRC_CHECK_CMD:
        {
            APP_TOTA_CMD_RET_STATUS_E status = TOTA_NO_ERROR;
            BESUI_TRACE(1, "[%s] OP_TOTA_CRC_CHECK_CMD ...", __func__);

            if(*(int*)ptrParam == TOTA_CRC_CHECK)
            {
                TOTA_CRC_CHECK_STRUCT_T *pdata = (TOTA_CRC_CHECK_STRUCT_T *)resData;
                status = tota_get_sanity_crc(&pdata->crc);
                status = tota_get_build_data(pdata->build_date);
#ifdef FIRMWARE_REV
                system_get_info(&(pdata->version[0]),&(pdata->version[1]),&(pdata->version[2]),&(pdata->version[3]));
#endif
                BESUI_TRACE(0, "|--------------------------------|");
                BESUI_TRACE(1, "CRC: 0x%x", pdata->crc);
                BESUI_TRACE(1, "Version: %d.%d.%d.%d", pdata->version[0],pdata->version[1],pdata->version[2],pdata->version[3]);
                BESUI_TRACE(1, "Build Data: %s", pdata->build_date);
                BESUI_TRACE(0, "|--------------------------------|");
            }
            else
            {
                status =TOTA_INVALID_DATA_PACKET;
            }

            resLen = sizeof(TOTA_CRC_CHECK_STRUCT_T);
            app_tota_send_rsp(funcCode, status, resData, resLen);
            return;
        }
#endif
        default:
            // TOTA_V2_TRACE(1,"wrong cmd 0x%x",funcCode);
            // resData[0] = -1;
            return;
    }
    app_tota_send_rsp(funcCode,TOTA_NO_ERROR,resData,resLen);
}

/* get general info */
static void __get_general_info()
{
    /* get bt-ble name */
    uint8_t* factory_name_ptr =factory_section_get_bt_name();
    if ( factory_name_ptr != NULL )
    {
        uint16_t valid_len = strlen((char*)factory_name_ptr) > BT_BLE_LOCAL_NAME_LEN? BT_BLE_LOCAL_NAME_LEN:strlen((char*)factory_name_ptr);
        memcpy(general_info.btName,factory_name_ptr,valid_len);
    }

    factory_name_ptr =factory_section_get_ble_name();
    if ( factory_name_ptr != NULL )
    {
        uint16_t valid_len = strlen((char*)factory_name_ptr) > BT_BLE_LOCAL_NAME_LEN? BT_BLE_LOCAL_NAME_LEN:strlen((char*)factory_name_ptr);
        memcpy(general_info.bleName,factory_name_ptr,valid_len);
    }

    /* get bt-ble peer addr */
//    ibrt_config_t addrInfo;
//    app_ibrt_ui_test_config_load(&addrInfo);
//    general_info.ibrtRole = addrInfo.nv_role;
//    memcpy(general_info.btLocalAddr.address, addrInfo.local_addr.address, 6);
//    memcpy(general_info.btPeerAddr.address, addrInfo.peer_addr.address, 6);

    #ifdef BLE
    memcpy(general_info.bleLocalAddr.address, bt_get_ble_local_address(), 6);
    memcpy(general_info.blePeerAddr.address, nv_record_tws_get_peer_ble_addr(), 6);
    #endif

    /* get crystal info */
    general_info.crystal_freq = hal_cmu_get_crystal_freq();

    /* factory_section_xtal_fcap_get */
    factory_section_xtal_fcap_get(&general_info.xtal_fcap);

    /* get battery info (volt level)*/
    app_battery_get_info(&general_info.battery_volt,&general_info.battery_level,&general_info.battery_status);

    /* get firmware version */
#ifdef FIRMWARE_REV
    system_get_info(&general_info.fw_version[0],&general_info.fw_version[1],&general_info.fw_version[2],&general_info.fw_version[3]);
    TOTA_V2_TRACE(4,"firmware version = %d.%d.%d.%d",general_info.fw_version[0],general_info.fw_version[1],general_info.fw_version[2],general_info.fw_version[3]);
#endif

    /* get ear location info */
    if ( app_ibrt_if_is_right_side() )      general_info.ear_location = EAR_SIDE_RIGHT;
    else if ( bts_tws_if_is_local_left_side() )  general_info.ear_location = EAR_SIDE_LEFT;
    else                                general_info.ear_location = EAR_SIDE_UNKNOWN;

    general_info.rssi[0] = app_tota_get_rssi_value();
    general_info.rssi_len = 1;
}

static int32_t find_key_word(uint8_t* targetArray, uint32_t targetArrayLen, uint8_t* keyWordArray, uint32_t keyWordArrayLen)
{
    if ((keyWordArrayLen > 0) && (targetArrayLen >= keyWordArrayLen))
    {
        uint32_t index = 0, targetIndex = 0;
        for (targetIndex = 0;targetIndex < targetArrayLen;targetIndex++)
        {
            for (index = 0;index < keyWordArrayLen;index++)
            {
                if (targetArray[targetIndex + index] != keyWordArray[index])
                {
                    break;
                }
            }

            if (index == keyWordArrayLen)
            {
                return targetIndex;
            }
        }

        return -1;
    }
    else
    {
        return -1;
    }
}

static uint8_t asciiToHex(uint8_t asciiCode)
{
    if ((asciiCode >= '0') && (asciiCode <= '9'))
    {
        return asciiCode - '0';
    }
    else if ((asciiCode >= 'a') && (asciiCode <= 'f'))
    {
        return asciiCode - 'a' + 10;
    }
    else if ((asciiCode >= 'A') && (asciiCode <= 'F'))
    {
        return asciiCode - 'A' + 10;
    }
    else
    {
        return 0xff;
    }
}

POSSIBLY_UNUSED static APP_TOTA_CMD_RET_STATUS_E tota_get_sanity_crc(uint32_t *sanityCrc32) 
{
    if(NULL == sanityCrc32){
        return TOTA_CMD_HANDLING_FAILED;
    }

    int32_t found = find_key_word((uint8_t*)&sys_build_info,
        LEN_OF_IMAGE_TAIL_TO_FINDKEY_WORD,
        (uint8_t*)image_info_sanity_crc_key_word,
        strlen(image_info_sanity_crc_key_word));
    if (-1 == found){
        return TOTA_CMD_HANDLING_FAILED;
    }

    uint8_t* crcString = (uint8_t*)&sys_build_info+found+strlen(image_info_sanity_crc_key_word);

    for (uint8_t index = 0;index < 4;index++)
    {
        *sanityCrc32 |= (asciiToHex(crcString[2*index]) << (8*index+4)) + (asciiToHex(crcString[2*index+1]) << (8*index));
    }

    TOTA_V2_TRACE(1,"sanityCrc32 is 0x%x", *sanityCrc32);

    return TOTA_NO_ERROR;
}

POSSIBLY_UNUSED static APP_TOTA_CMD_RET_STATUS_E tota_get_build_data(uint8_t *buildData) 
{
    if(NULL == buildData){
        return TOTA_CMD_HANDLING_FAILED;
    }
    int32_t found = find_key_word((uint8_t*)&sys_build_info,
        LEN_OF_IMAGE_TAIL_TO_FINDKEY_WORD,
        (uint8_t*)image_info_build_data,
        strlen(image_info_build_data));
    if (-1 == found){
        return TOTA_CMD_HANDLING_FAILED;
    }

    memcpy(buildData, (uint8_t*)&sys_build_info+found+strlen(image_info_build_data), 20);

    TOTA_V2_TRACE(1,"buildData is 0x%s", buildData);

    return TOTA_NO_ERROR;
}

/* general command */
TOTA_COMMAND_TO_ADD(OP_TOTA_GENERAL_INFO_CMD, __tota_general_cmd_handle, false, 0, NULL );
TOTA_COMMAND_TO_ADD(OP_TOTA_VOLUME_PLUS_CMD, __tota_general_cmd_handle, false, 0, NULL );
TOTA_COMMAND_TO_ADD(OP_TOTA_VOLUME_DEC_CMD, __tota_general_cmd_handle, false, 0, NULL );
TOTA_COMMAND_TO_ADD(OP_TOTA_VOLUME_SET_CMD, __tota_general_cmd_handle, false, 0, NULL );
TOTA_COMMAND_TO_ADD(OP_TOTA_VOLUME_GET_CMD, __tota_general_cmd_handle, false, 0, NULL );
TOTA_COMMAND_TO_ADD(OP_TOTA_RAW_DATA_SET_CMD, __tota_general_cmd_handle, false, 0, NULL );
TOTA_COMMAND_TO_ADD(OP_TOTA_GET_RSSI_CMD, __tota_general_cmd_handle, false, 0, NULL );
TOTA_COMMAND_TO_ADD(OP_TOTA_GET_DUMP_INFO_CMD, __tota_general_cmd_handle, false, 0, NULL );
TOTA_COMMAND_TO_ADD(OP_TOTA_SET_CUSTOMER_CMD, __tota_general_cmd_handle, false, 0, NULL );
TOTA_COMMAND_TO_ADD(OP_TOTA_GET_CUSTOMER_CMD, __tota_general_cmd_handle, false, 0, NULL );
#ifdef BESUI_APP_EN
TOTA_COMMAND_TO_ADD(OP_TOTA_CRC_CHECK_CMD, __tota_general_cmd_handle, false, 0, NULL );
#endif

#ifdef DUMP_LOG_ENABLE
extern uint32_t __log_dump_start[];
#endif

static void general_get_flash_dump_info(TOTA_DUMP_INFO_STRUCT_T * p_flash_info)
{
#ifdef DUMP_LOG_ENABLE
    p_flash_info->address =  (uint32_t)&__log_dump_start;
    p_flash_info->length = LOG_DUMP_SECTION_SIZE;
#else
    p_flash_info->address =  0;
    p_flash_info->length = 0;
#endif
}
#endif