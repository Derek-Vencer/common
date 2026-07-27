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
/*****************************header include********************************/
#include "string.h"
#include "app_tws_ibrt_cmd_handler.h"
#include "app_ibrt_customif_cmd.h"
#ifdef BESUI_APP_EN
#include "app_tota_general.h"
#include "app_tota.h"
#include "nvrecord_extension.h"
#include "nvrecord_env.h"
#if defined(CAPSENSOR_ENABLE)
#if defined(CHIP_BEST1306)
#include "capsensor_driver_best1306.h"
#endif
#if defined(CHIP_BEST1501P)
#include "analog_best1501p.h"
#endif
#endif //#if defined(CAPSENSOR_ENABLE)
#endif

#if defined(SPA_AUDIO_ENABLE)
#include "spa_ext_tws_handler.h"
#endif


#ifdef BESUI_KEY_EN
#include "twsui_key.h"
#endif
#ifdef BESUI_BTMSG_EN
#include "twsui_btmsg.h"
#endif
#if defined(EQ_CUSTOM_APP_EN)
#include "app_bt_stream.h"
#endif

#if 1 //defined(BESUI_TWS_EN) || defined(BESUI_STEREO_EN)
#include "apps.h"
#include "besui_common.h"
#endif

#if defined(BESUI_TWS_EN)
#include "twsui_comm.h"
#endif
#include "bts_tws_if.h"
#include "bts_core_if.h"
#include "app_hfp.h"
#include "app_bt.h"
#include "hw_codec_iir_process.h"
#include "audio_process.h"
#include "nvrecord_bt.h"
#include "nvrecord_env.h"
#include "nvrecord_extension.h"
#include "factory_section.h"
#include "app_bt_stream.h"

extern "C" uint8_t app_ibrt_if_get_ui_role(void);

extern const IIR_CFG_T * const POSSIBLY_UNUSED audio_eq_cfg_vol_list[VOL_CTRL_EQ_LIST_NUM];


static uint8_t g_tws_peer_battery_level = 0xFF;
static bool g_tws_peer_battery_valid = false;
static uint8_t g_tws_peer_box_battery_level = 0xFF;
static bool g_tws_peer_box_battery_valid = false;
extern "C" uint8_t app_battery_current_level(void);
uint8_t getBoxChargerBattery(void);
extern "C" void ntt_color_code_nv_set(uint8_t color);
extern "C" uint8_t ntt_color_code_nv_get(void);
extern void ntt_ble_adv_refresh_data(void);

extern bool ntt_color_code_is_valid(uint8_t color);
#ifdef IBRT
extern "C" bool app_ibrt_middleware_is_ui_slave(void);
#endif

extern "C" void app_bt_profile_connect_manager_opening_reconnect(void);
extern "C" void ntt_audio_drc_apply_by_eq_index(uint8_t eq_index);
extern void ntt_tws_reconnect_after_mobile_profiles_ready_check(void);
extern uint8_t out_of_case_reconnect;
uint8_t app_ibrt_customif_get_tws_peer_battery_level(void)
{
    return g_tws_peer_battery_valid ? g_tws_peer_battery_level : 0xFF;
}

uint8_t app_ibrt_customif_get_tws_peer_box_battery_level(void)
{
    return g_tws_peer_box_battery_valid ? g_tws_peer_box_battery_level : 0xFF;
}

#define NTT_COLOR_CODE_BLACK         0x4B
#define NTT_COLOR_CODE_SILVER_WHITE  0x53
#define NTT_COLOR_CODE_GOLD          0x4E

static bool ntt_color_code_is_valid_local(uint8_t color)
{
    return ((color == NTT_COLOR_CODE_BLACK) ||
            (color == NTT_COLOR_CODE_SILVER_WHITE) ||
            (color == NTT_COLOR_CODE_GOLD));
}

#if defined(IBRT)

/*********************external function declaration*************************/

/*********************internal function declaration*************************/
static void app_ibrt_customif_test1_cmd_send(uint8_t *p_buff, uint16_t length);
static void app_ibrt_customif_test1_cmd_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

static void app_ibrt_customif_test2_cmd_send(uint8_t *p_buff, uint16_t length);
static void app_ibrt_customif_test2_cmd_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
static void app_ibrt_customif_test2_cmd_send_rsp_timeout_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
static void app_ibrt_customif_test2_cmd_send_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
static void app_ibrt_customif_test2_cmd_send_tx_done_handler(uint16_t cmdcode, uint16_t rsp_seq, uint8_t *ptrParam, uint16_t paramLen);

static void app_ibrt_customif_music_eq_cmd_send(uint8_t *p_buff, uint16_t length);
static void app_ibrt_customif_music_eq_cmd_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

static void app_ibrt_customif_button_map_cmd_send(uint8_t *p_buff, uint16_t length);
static void app_ibrt_customif_button_map_cmd_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

static void app_ibrt_customif_sync_bt_name_cmd_send(uint8_t *p_buff, uint16_t length);
static void app_ibrt_customif_sync_bt_name_cmd_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

static void ntt_master_sync_eq_to_peer(void);
static void ntt_master_sync_keymap_to_peer(void);
static void ntt_master_sync_bt_name_to_peer(void);
void ntt_master_sync_all_user_settings_to_peer(void);

static void ntt_case_state_sync_send_handler(uint8_t *p_buff,uint16_t length);
static void ntt_case_state_sync_receive_handler(uint16_t rsp_seq,uint8_t *p_buff,uint16_t length);
static void ntt_key_reconnect_request_send_handler(uint8_t *p_buff,uint16_t length);

static void ntt_key_reconnect_request_receive_handler(uint16_t rsp_seq,uint8_t *p_buff,uint16_t length);

#if 1 //def BESUI_TWS_EN
#ifdef BESUI_APP_EN
static void app_ibrt_sync_tota_battery_level(uint8_t *p_buff, uint16_t length)
{
    app_ibrt_send_cmd_without_rsp(APP_TWS_CMD_SEND_TOTA_BATTERY_LEVEL_CMD, p_buff, length);
}
static void app_ibrt_sync_tota_battery_level_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    EARBUDS_TRACE(0,"app_ibrt_sync_tota_battery_level_handler");
    app_tws_sync_battery_2_slave(p_buff, length);
}
static void app_ibrt_sync_tota_battery_level_rsp(uint8_t *p_buff, uint16_t length)
{
    app_ibrt_send_cmd_without_rsp(APP_TWS_CMD_SEND_TOTA_BATTERY_LEVEL_RSP, p_buff, length);
}
static void app_ibrt_sync_tota_battery_level_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    EARBUDS_TRACE(0,"app_ibrt_sync_tota_battery_level_rsp_handler");
    app_tota_sync_battery_level_rsp_handle(p_buff, length);
}

//----------------------------------------------------------------------------------------------------
void twsui_wear_pp_tx(bool role, uint8_t sta)
{
    if(!bts_tws_if_is_tws_link_connected())
    {
        return;        
    }

	uint8_t param[3] = {0};
    param[0] = TWS_SYNC_WEAR_CTRL;
    param[1] = sta;     //1-in  2-out    
    param[2] = role;    //1-master  0-slave
	tws_ctrl_send_cmd(APP_TWS_CMD_CAPSENSOR_WEAR, param, 3);
	EARBUDS_TRACE(0, "[UITWS]%s sta %d, %d",__func__, sta, role);
}

void app_ibrt_tws_capsensor_wear_tx(uint8_t *p_buff, uint16_t length)
{
    app_ibrt_send_cmd_without_rsp(APP_TWS_CMD_CAPSENSOR_WEAR, p_buff, length);
}
void app_ibrt_tws_capsensor_wear_rx_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
#ifdef CAPSENSOR_ENABLE
    if(length == 2)
        EARBUDS_TRACE(0,"[%s]= %d, %d", __func__,p_buff[0], p_buff[1]);
    else if(length == 3)
        EARBUDS_TRACE(0,"[%s]= %d, %d, %d", __func__,p_buff[0], p_buff[1], p_buff[2]);
    else if(length == 4)
        EARBUDS_TRACE(0,"[%s]= %d, %d, %d, %d", __func__,p_buff[0], p_buff[1], p_buff[2], p_buff[3]);  
    else if(length == 5)
        EARBUDS_TRACE(0,"[%s]= %d, %d, %d, %d, %d", __func__,p_buff[0], p_buff[1], p_buff[2], p_buff[3], p_buff[4]);  

    nv_record_env_get(&nvrecord_uienv);

    if(p_buff[0] == TWS_SYNC_WEAR_M2S)       //from master set wear onoff
    {
        nvrecord_uienv->inear_left_onoff = p_buff[1];
        nvrecord_uienv->left_status = p_buff[2];
        nvrecord_uienv->inear_right_onoff = p_buff[3];
        nvrecord_uienv->right_status = p_buff[4];
    }
    else if(p_buff[0] == TWS_SYNC_WEAR_S2M) //from slave sync wear dat
    {
        if(bts_tws_if_is_local_left_side())
        {
            nvrecord_uienv->inear_right_onoff = p_buff[1];
            nvrecord_uienv->right_status = p_buff[2];
            uictl.ear_another_sta = nvrecord_uienv->right_status;
        }
        else
        {
            nvrecord_uienv->inear_left_onoff = p_buff[1];
            nvrecord_uienv->left_status = p_buff[2];
            uictl.ear_another_sta = nvrecord_uienv->left_status;
        }
        app_tota_send_inear_status(&p_buff[1], 2);
    }
    else if(p_buff[0] == TWS_SYNC_WEAR_CTRL)
    {
        uicom.wear_peer_sta = p_buff[1];
        if(p_buff[1])   //1-in  2-out
            uictl.ear_another_sta = 2-p_buff[1];
        if(p_buff[2])   //1-master  0-slave
            peer_wear_sta_msg(p_buff[1]);
        return;
    }  
    else if(p_buff[0] == TWS_SYNC_WEAR_PROMPT) //from master set wear prompt onoff
    {
#if defined(WEAR_DETECT_PROMPT_EN)
        nvrecord_uienv->wear_prompt_onoff = p_buff[1];
        nv_record_env_set(nvrecord_uienv);
        return;
#endif
    }

    nv_record_env_set(nvrecord_uienv);

    EARBUDS_TRACE(0,"[%s], left = %d, %d", __func__, nvrecord_uienv->inear_left_onoff, nvrecord_uienv->left_status);
    EARBUDS_TRACE(0,"[%s], right = %d, %d", __func__, nvrecord_uienv->inear_right_onoff, nvrecord_uienv->right_status); 
#endif
}
//----------------------------------------------------------------------------------------------------
#endif

void ntt_master_sync_all_user_settings_to_peer(void)
{
#ifdef IBRT
    if (!bts_tws_if_is_tws_link_connected())
    {
        EARBUDS_TRACE(0, "[NTT_USER_SYNC] skip, tws not connected");
        return;
    }

    EARBUDS_TRACE(0, "[NTT_USER_SYNC] sync all settings to peer");

    ntt_master_sync_eq_to_peer();
    ntt_master_sync_keymap_to_peer();
    ntt_master_sync_bt_name_to_peer();
#endif
}

static void ntt_master_sync_eq_to_peer(void)
{
    struct nvrecord_env_t *nvrecord_env = NULL;
    uint8_t eq_index = 0;

    nv_record_env_get(&nvrecord_env);

    if (nvrecord_env == NULL)
    {
        EARBUDS_TRACE(0, "[NTT_USER_SYNC][EQ] nvrecord_env null");
        return;
    }

    eq_index = nvrecord_env->eq_index_data;

    if (eq_index > 5)
    {
        EARBUDS_TRACE(1, "[NTT_USER_SYNC][EQ] invalid eq_index=%d, force 0", eq_index);
        eq_index = 0;
        nvrecord_env->eq_index_data = eq_index;
        nv_record_env_set(nvrecord_env);
    }

    EARBUDS_TRACE(1,
        "[NTT_USER_SYNC][EQ] send music eq_index=%d",
        eq_index);

    app_ibrt_customif_cmd_sync_music_eq(eq_index);
}

static void ntt_master_sync_keymap_to_peer(void)
{
    struct nvrecord_env_t *nvrecord_env = NULL;
    nv_record_env_get(&nvrecord_env);

    if (nvrecord_env == NULL)
    {
        EARBUDS_TRACE(0, "[NTT_USER_SYNC][KEYMAP] nvrecord_env null");
        return;
    }

    uint8_t key_num = nvrecord_env->key_map_number;

    if (key_num > 21)
    {
        key_num = 21;
    }

    uint8_t data[1 + 21 * 2] = {0};

    data[0] = key_num;

    for (uint8_t i = 0; i < key_num; i++)
    {
        data[i * 2 + 1] = nvrecord_env->key_map_action[i];
        data[i * 2 + 2] = nvrecord_env->key_map_func[i];
    }

    EARBUDS_TRACE(1, "[NTT_USER_SYNC][KEYMAP] count=%d", key_num);

    tws_ctrl_send_cmd(APP_TWS_CMD_SYNC_BUTTON_MAP, data, 1 + key_num * 2);
}

static void ntt_master_sync_bt_name_to_peer(void)
{
    uint8_t *name = factory_section_get_bt_name();

    if (name == NULL)
    {
        EARBUDS_TRACE(0, "[NTT_USER_SYNC][NAME] name null");
        return;
    }

    uint8_t len = strlen((char *)name) + 1;

    if (len > 49)
    {
        len = 49;
    }

    EARBUDS_TRACE(1, "[NTT_USER_SYNC][NAME] len=%d", len);

    app_ibrt_customif_cmd_sync_bt_name(name, len);
}

#ifdef ALGO_INFO_SYNC_EN
void algo_send_request(uint32_t message_id, uint32_t param0, uint32_t param1, uint32_t param2, uint32_t param3, uint32_t ptr);
static void app_ibrt_customif_sync_eq(uint8_t *p_buff, uint16_t length)
{
	app_ibrt_send_cmd_without_rsp(APP_TWS_CMD_SYNC_EQ, p_buff, length);
}

static void app_ibrt_customif_sync_eq_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
	EARBUDS_TRACE(3, "[UITWS][%s]rsp_seq = %d length = %d", __func__, rsp_seq, length);
	DUMP8("%02x ",p_buff,length);

#ifdef EQ_SET_CUSTOMER_EN
#if defined(EQ_CUSTOM_APP_EN)
    if(p_buff[2] == 0xFF && length>=10)
    {
        // struct nvrecord_uienv_t *nvrecord_uienv;
        nv_record_env_get(&nvrecord_uienv);

        nvrecord_uienv->eq_onoff = p_buff[1]; //off
        nvrecord_uienv->eq_mode = p_buff[2];
        memcpy(nvrecord_uienv->eq_custom, p_buff+3, EQBAND_NUM);
        nv_record_env_set(nvrecord_uienv);
        
        EARBUDS_TRACE(0, "[UIEQ]%s, eq_custom =", __func__);
        DUMP8("%d ", nvrecord_uienv->eq_custom, EQBAND_NUM);
    
#if defined(ALGO_INFO_SYNC_EN)
        algo_send_request(ALGO_ID_EQ_CUSTOM, ALGO_ON, 0xFF, 0, 0, 0);
#endif
    }
    else
#endif
    {
        algo_send_request(ALGO_ID_EQ, p_buff[1], p_buff[2], 0, 0, 0);        
    }
#endif
}

static void app_ibrt_customif_sync_besspa(uint8_t *p_buff, uint16_t length)
{
	app_ibrt_send_cmd_without_rsp(APP_TWS_CMD_SYNC_BESSPA, p_buff, length);
}

static void app_ibrt_customif_sync_besspa_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
	EARBUDS_TRACE(3, "[UITWS][%s]rsp_seq = %d length = %d", __func__, rsp_seq, length);
	DUMP8("%02x ",p_buff,length);
#ifdef BESSPA_ONOFF_EN
    algo_send_request(ALGO_ID_BESSPA, p_buff[1], 0, 0, 0, 0);
#endif
}
static void app_ibrt_customif_sync_dolby(uint8_t *p_buff, uint16_t length)
{
	app_ibrt_send_cmd_without_rsp(APP_TWS_CMD_SYNC_DOLBY, p_buff, length);
}

static void app_ibrt_customif_sync_dolby_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
	EARBUDS_TRACE(3, "[UITWS][%s]rsp_seq = %d length = %d", __func__, rsp_seq, length);
	DUMP8("%02x ",p_buff,length);
#ifdef DOLBY_AUDIO_ENABLE
    algo_send_request(ALGO_ID_DOLBY, p_buff[1], p_buff[2], 0, 0, 0);
#endif
}
#endif //#ifdef ALGO_INFO_SYNC_EN

#if defined(OTA_BOOT_SYNC_EN)
static void app_ibrt_customif_sync_otaboot(uint8_t *p_buff, uint16_t length)
{
	app_ibrt_send_cmd_without_rsp(APP_TWS_CMD_SYNC_OTABOOT, p_buff, length);
}

static void app_ibrt_customif_sync_otaboot_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
	EARBUDS_TRACE(3, "[UITWS][%s]rsp_seq = %d length = %d", __func__, rsp_seq, length);
	DUMP8("%02x ",p_buff,length);
    if(p_buff[0] == 0x01)
    {
#ifdef BESUI_KEY_EN
        uictl.tws_recv_otaboot_flag = 1;
        app_key_gui_to_otaboot_or_single(1, 0);
#endif
    }
}
#endif

static void app_ibrt_customif_sync_something(uint8_t *p_buff, uint16_t length)
{
	app_ibrt_send_cmd_without_rsp(APP_TWS_CMD_SYNC_SOMETHING, p_buff, length);
}


static void app_ibrt_customif_sync_something_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
	EARBUDS_TRACE(3, "[UITWS][%s]rsp_seq = %d length = %d", __func__, rsp_seq, length);
	DUMP8("%02x ",p_buff,length);

#ifdef  BESUI_TWS_EN
    if(p_buff[0] == USER_TWS_CMD_ENTER_PAIRMODE)
    {
        besui_bt_msg_put(SLAVE_ENTER_PAIRMODE_EVENT, 0xff, BT_DEVICE_NUM);
    }
    else if(p_buff[0] == USER_TWS_CMD_SLAVE_LINKLOSS)
    {
        besui_bt_msg_put(SLAVE_LINKLOSS_EVENT, 0xff, BT_DEVICE_NUM);
    }
#endif

#ifdef USER_APP_BLE_DIS_EN    
    else if(p_buff[0] == USER_TWS_CMD_RANDOM)
    {
        memcpy((uint8_t *)(uictl.random_peer), (uint8_t *)(p_buff+1), 2);
        //app_tota_switch_role_to_app();
    }
#endif
#ifdef BES_NULTRON_EN    
    else if(p_buff[0] == USER_TWS_CMD_CLEAR_PHONE_NV)
    {
        besui_bt_msg_put(BT_MSG_SOMETHING_EVENT, USER_MSG_CMD_CLEAR_PHONE_NV, 0);
    }
#endif
#ifdef APP_TEST_EN
    else if(p_buff[0] == USER_TWS_CMD_TEST_EQTUNE)
    {
        besui_bt_msg_put(BT_MSG_SOMETHING_EVENT, USER_MSG_CMD_TEST_EQTUNE, 0);
#ifdef SPP_DEBUG_TOOL
	    EARBUDS_TRACE(0, "[EQTUNE][%s]USER_TWS_CMD_TEST_EQTUNE", __func__);
        DUMP8("%02X ", p_buff, length);
        
        uint16_t dataLen = length-1;
        extern bool app_spp_debug_cmd_check(uint8_t *cmd, uint16_t len);
        extern uint8_t *app_spp_debug_cmd_process(uint8_t *cmd, uint16_t len, uint16_t *out_len);
        if (app_spp_debug_cmd_check(p_buff+1, dataLen)) {
            TOTA_LOG_DBG(1,"[EQTUNE]rx = %d", dataLen);
            TOTA_LOG_DUMP("%02X ", p_buff+1, dataLen);
            // uint8_t *ret_buf = 
            app_spp_debug_cmd_process(p_buff+1, dataLen, &dataLen);
            // if (ret_buf != NULL) {
            //     bt_spp_write(param->spp_chan->rfcomm_handle, ret_buf, dataLen);
            // }
            EARBUDS_TRACE(0, "[%s] Bypass TOTA.", __func__);
            // return 0;
        }
#endif
    }
#endif
}

void app_ibrt_customif_cmd_sync_poweroff_shutdown(bool poweroff_flag)
{
    if (!bts_tws_if_is_tws_link_connected())
    {
        return;
    }

	uint8_t cmd_sync_poweroff_shutdown[1];
	cmd_sync_poweroff_shutdown[0] = poweroff_flag;
	EARBUDS_TRACE(2, "[UITWS]%s poweroff_flag %d",__func__, poweroff_flag);
	tws_ctrl_send_cmd(APP_TWS_CMD_POWEROFF_SHUTDOWN_SYNC, cmd_sync_poweroff_shutdown, 1);
}

static void app_ibrt_customif_sync_poweroff_shutdown_send(uint8_t *p_buff, uint16_t length)
{
	app_ibrt_send_cmd_without_rsp(APP_TWS_CMD_POWEROFF_SHUTDOWN_SYNC, p_buff, length);
}

static void app_ibrt_customif_sync_poweroff_shutdown_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
	EARBUDS_TRACE(1, "[UITWS]%s", __func__);
	EARBUDS_TRACE(2, "[UITWS]rsp_seq = %d length = %d", rsp_seq, length);
	DUMP8("%02x ",p_buff,length);

    if(p_buff[0])
    {

#ifdef  BESUI_TWS_EN
        uictl.shutdown_type = SHUTDOWN_SYNC_PEER;
#endif
        app_shutdown();
    }
}

void app_ibrt_customif_cmd_sync_battery_level(uint8_t current_level)
{
    static uint32_t s_last_sync_ms = 0;
    static uint8_t s_last_level = 0xFF;
    static uint8_t s_last_box_level = 0xFF;

    bool tws_connected = bts_tws_if_is_tws_link_connected();
    uint8_t role = app_ibrt_if_get_ui_role();

    if (!tws_connected)
    {
        return;
    }

    if (role == TWS_UI_MASTER)
    {
        return;
    }

    uint8_t box_level = getBoxChargerBattery();
    uint32_t now_ms = GET_CURRENT_MS();

    if ((s_last_level == current_level) &&
        (s_last_box_level == box_level) &&
        ((now_ms - s_last_sync_ms) < 120000))
    {
        return;
    }

    s_last_level = current_level;
    s_last_box_level = box_level;
    s_last_sync_ms = now_ms;

    uint8_t cmd_sync_battery_level[2];
    cmd_sync_battery_level[0] = current_level;
    cmd_sync_battery_level[1] = box_level;

    tws_ctrl_send_cmd(APP_TWS_CMD_BATTERY_LEVEL_SYNC,
                      cmd_sync_battery_level,
                      sizeof(cmd_sync_battery_level));
}

static void app_ibrt_customif_sync_battery_level_send(uint8_t *p_buff,uint16_t length)
{
    if ((p_buff == NULL) || (length == 0))
    {
        EARBUDS_TRACE(1,
                      "[BAT_SYNC][SEND] invalid data, length=%u",
                      length);
        return;
    }

    DUMP8("[BAT_SYNC][SEND] raw data: ", p_buff, length);

    /*
     * 目前擴充格式預期：
     *
     * p_buff[0]：耳機電量資料
     * p_buff[1]：耳機狀態／其他原有電池欄位
     * p_buff[2]：充電盒電量
     *
     * 若你的實際 battery sync payload 超過 3 bytes，
     * 充電盒電量應以接收端 peer_box_raw 使用的索引為準。
     */
    if (length >= 3)
    {
        const uint8_t ear_battery_raw = p_buff[0];
        const uint8_t ear_status_raw  = p_buff[1];
        const uint8_t box_battery     = p_buff[2];

        if (box_battery <= 100)
        {
            EARBUDS_TRACE(3,
                          "[BAT_SYNC][SEND] ear_raw=%u status=0x%02X box=%u%%",
                          ear_battery_raw,
                          ear_status_raw,
                          box_battery);
        }
        else
        {
            EARBUDS_TRACE(3,
                          "[BAT_SYNC][SEND] ear_raw=%u status=0x%02X box=INVALID(0x%02X)",
                          ear_battery_raw,
                          ear_status_raw,
                          box_battery);
        }
    }
    else if (length == 2)
    {
        EARBUDS_TRACE(2,
                      "[BAT_SYNC][SEND] ear_raw=%u status=0x%02X, no box battery",
                      p_buff[0],
                      p_buff[1]);
    }
    else
    {
        EARBUDS_TRACE(1,
                      "[BAT_SYNC][SEND] ear_raw=%u, payload too short",
                      p_buff[0]);
    }

    app_ibrt_send_cmd_without_rsp(APP_TWS_CMD_BATTERY_LEVEL_SYNC,
                                  p_buff,
                                  length);

    EARBUDS_TRACE(1,
                  "[BAT_SYNC][SEND] APP_TWS_CMD_BATTERY_LEVEL_SYNC sent, length=%u",
                  length);
}

uint8_t earbuds_get_profile_conn_num(void)
{
    uint8_t conn_cnt = 0;
    if(app_bt_get_device(BT_DEVICE_ID_1)->profile_mgr.profile_connected)
        conn_cnt ++;
    if(app_bt_get_device(BT_DEVICE_ID_2)->profile_mgr.profile_connected)
        conn_cnt ++;
    EARBUDS_TRACE(0,"%s, conn_cnt = %d", __func__, conn_cnt);

    return conn_cnt;
}

void app_tws_battery_update(bool tws_connect_flag)
{
    uint8_t conn_devices = earbuds_get_profile_conn_num();
    EARBUDS_TRACE(1,"%s tws_connect_flag %d %d ", __func__, tws_connect_flag, conn_devices);


    app_hfp_battery_report_reset(BT_DEVICE_ID_1);
#if (BT_DEVICE_NUM > 1)
    app_hfp_battery_report_reset(BT_DEVICE_ID_2);
#endif

#ifdef  BESUI_TWS_EN
    uint8_t level = 0;
    if(tws_connect_flag)
    {
        if((bts_tws_if_is_tws_link_connected())&&(TWS_UI_MASTER == app_ibrt_if_get_ui_role())&&(conn_devices > 0))
        {
            level = app_battery_level_compare();
            app_hfp_set_battery_level(level);
        }
    }
    else
    {
        if(conn_devices > 0)
        {
            level = twsui_get_bat_level();
            app_hfp_set_battery_level(level);
        }
    }
#else
    EARBUDS_TRACE(1,"%s not impletement!!! ", __func__);
#endif

}

//static bool battery_sync_echo_guard = false;
static void app_ibrt_customif_sync_battery_level_send_handler(
    uint16_t rsp_seq,
    uint8_t *p_buff,
    uint16_t length)
{
    uint8_t peer_raw = 0xFF;
    uint8_t peer_percent = 0xFF;
    uint8_t peer_box_raw = 0xFF;
    uint8_t role = app_ibrt_if_get_ui_role();


    if ((p_buff == NULL) || (length < 1))
    {
        return;
    }

    peer_raw = p_buff[0];
    peer_box_raw = (length > 1) ? p_buff[1] : 0xFF;

    if (peer_raw <= 9)
    {
        peer_percent = (peer_raw >= 9) ? 100 : ((peer_raw + 1) * 10);
    }
    else if (peer_raw <= 100)
    {
        peer_percent = peer_raw;
    }
    else
    {

        return;
    }


    if (role == TWS_UI_MASTER)
    {
        g_tws_peer_battery_level = peer_percent;
        g_tws_peer_battery_valid = true;

        if (peer_box_raw <= 100)
        {
            g_tws_peer_box_battery_level = peer_box_raw;
            g_tws_peer_box_battery_valid = true;
        }

        EARBUDS_TRACE(4,
                    "[BAT_SYNC][MASTER_SAVE_PEER] ear=%d valid=%d box=%d box_valid=%d",
                    g_tws_peer_battery_level,
                    g_tws_peer_battery_valid,
                    g_tws_peer_box_battery_level,
                    g_tws_peer_box_battery_valid);

    #ifdef BESUI_TWS_EN
        set_tws_peer_battery_percent(peer_percent);
        app_battery_set_other_battery_level(peer_percent);
        app_tws_battery_update(true);
    #endif
    }
}

#ifdef BESUI_GAME_EN
void app_ibrt_customif_cmd_sync_game_mode(bool switch_game_flag, uint8_t need_set_ame_mode)
{
    if (!bts_tws_if_is_tws_link_connected())
    {
        return;
    }

	uint8_t cmd_sync_game_mode[2];
	cmd_sync_game_mode[0] = switch_game_flag;
    cmd_sync_game_mode[1] = need_set_ame_mode;
	EARBUDS_TRACE(2, "[UITWS]%s switch_game_flag %d %d",__func__, switch_game_flag, need_set_ame_mode);
	tws_ctrl_send_cmd(APP_TWS_CMD_SWITCH_GAME_MODE_SYNC, cmd_sync_game_mode, 2);
}

static void app_ibrt_customif_sync_game_mode_send(uint8_t *p_buff, uint16_t length)
{
	app_ibrt_send_cmd_without_rsp(APP_TWS_CMD_SWITCH_GAME_MODE_SYNC, p_buff, length);
}

static void app_ibrt_customif_sync_game_mode_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
	EARBUDS_TRACE(1, "[UITWS]%s", __func__);
	EARBUDS_TRACE(2, "[UITWS]rsp_seq = %d length = %d", rsp_seq, length);
	DUMP8("%02x ",p_buff,length);
    besui_gamemode_key_switch(p_buff[0], p_buff[1], true);
}
#endif

void app_ibrt_customif_cmd_sync_led_mode(uint8_t led_mode, uint8_t call_status)
{
    if (!bts_tws_if_is_tws_link_connected())
    {
        return;
    }

	uint8_t cmd_sync_led_mode[2];
	
	cmd_sync_led_mode[0] = led_mode;
    cmd_sync_led_mode[1] = call_status;


	EARBUDS_TRACE(2, "[UITWS]%s led_mode %d %d",__func__, led_mode, call_status);

	tws_ctrl_send_cmd(APP_TWS_CMD_SWITCH_LED_MODE_SYNC, cmd_sync_led_mode, 2);
}

static void app_ibrt_customif_sync_led_mode_send(uint8_t *p_buff, uint16_t length)
{
	app_ibrt_send_cmd_without_rsp(APP_TWS_CMD_SWITCH_LED_MODE_SYNC, p_buff, length);
}

static void app_ibrt_customif_sync_led_mode_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
	EARBUDS_TRACE(1, "[UITWS]%s", __func__);
	EARBUDS_TRACE(2, "[UITWS]rsp_seq = %d length = %d", rsp_seq, length);
	DUMP8("%02x ",p_buff,length);

#ifdef  BESUI_TWS_EN
    app_tws_set_ledsta_call_slave_process(p_buff[0], p_buff[1]);
#endif

}

void app_ibrt_customif_cmd_sync_btaddr(uint8_t *cur_btaddr)
{
    if (!bts_tws_if_is_tws_link_connected())
    {
        return;
    }

	uint8_t cmd_sync_btaddr[6];
    uint8_t i = 0;
	
    for(i = 0; i < 6; i++)
    {
        cmd_sync_btaddr[i] = cur_btaddr[i];
    }

	EARBUDS_TRACE(0, "[UITWS]%s",__func__);

	tws_ctrl_send_cmd(APP_TWS_CMD_BTADDR_SYNC, cmd_sync_btaddr, 6);
}

static void app_ibrt_customif_sync_btaddr_send(uint8_t *p_buff, uint16_t length)
{
	app_ibrt_send_cmd_without_rsp(APP_TWS_CMD_BTADDR_SYNC, p_buff, length);
}

static void app_ibrt_customif_sync_btaddr_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
	EARBUDS_TRACE(1, "[UITWS]%s", __func__);
	EARBUDS_TRACE(2, "[UITWS]rsp_seq = %d length = %d", rsp_seq, length);
	DUMP8("%02x ",p_buff,length);

#ifdef  BESUI_TWS_EN
    app_common_store_twsaddrto_nv_flash(p_buff);
#endif

}


void app_ibrt_customif_cmd_sync_clear_pairlist(bool clear_tws, bool clear_phone)
{
    if (!bts_tws_if_is_tws_link_connected())
    {
        return;
    }

	uint8_t cmd_sync_clear_pairlist[2];
    cmd_sync_clear_pairlist[0] = clear_tws;
    cmd_sync_clear_pairlist[1] = clear_phone;
	EARBUDS_TRACE(0, "[UITWS]%s clear_tws %d clear_phone %d",__func__, clear_tws, clear_phone);
	tws_ctrl_send_cmd(APP_TWS_CMD_CLEAR_PIARLIST_SYNC, cmd_sync_clear_pairlist, 2);
}

static void app_ibrt_customif_sync_clear_pairlist_send(uint8_t *p_buff, uint16_t length)
{
	app_ibrt_send_cmd_without_rsp(APP_TWS_CMD_CLEAR_PIARLIST_SYNC, p_buff, length);
}

static void app_ibrt_customif_sync_clear_pairlist_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
	EARBUDS_TRACE(1, "[UITWS]%s", __func__);
	EARBUDS_TRACE(2, "[UITWS]rsp_seq = %d length = %d", rsp_seq, length);
	DUMP8("%02x ",p_buff,length);

#ifdef  BESUI_TWS_EN
    app_common_clear_pairlist_process(p_buff[0], p_buff[1], true, false);
#endif

}

void app_ibrt_customif_cmd_sync_space_audio_status(bool switch_flag, uint8_t current_status)
{
    if (!bts_tws_if_is_tws_link_connected())
    {
        return;
    }

	uint8_t param[2];
    param[0] = switch_flag;
    param[1] = current_status;
	EARBUDS_TRACE(0, "[UITWS]%s switch_flag%d current_status %d",__func__, switch_flag, current_status);
	tws_ctrl_send_cmd(APP_TWS_CMD_SPACE_AUDIO_SYNC_STATUS, param, 2);
}

static void app_ibrt_customif_sync_space_audio_status_send(uint8_t *p_buff, uint16_t length)
{
	app_ibrt_send_cmd_without_rsp(APP_TWS_CMD_SPACE_AUDIO_SYNC_STATUS, p_buff, length);
}

static void app_ibrt_customif_sync_space_audio_status_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
	EARBUDS_TRACE(1, "[UITWS]%s", __func__);
	EARBUDS_TRACE(2, "[UITWS]rsp_seq = %d length = %d", rsp_seq, length);
	DUMP8("%02x ",p_buff,length);
}

void app_ibrt_customif_cmd_sync_set_reconnect_status(uint8_t device_id, uint8_t recon_status)
{
    if (!bts_tws_if_is_tws_link_connected())
    {
        return;
    }

	uint8_t cmd_sync_set_reconnect_status[2];
    cmd_sync_set_reconnect_status[0] = device_id;
    cmd_sync_set_reconnect_status[1] = recon_status;
	EARBUDS_TRACE(0, "[UITWS]%s device_id%d recon_status %d",__func__, device_id, recon_status);
	tws_ctrl_send_cmd(APP_TWS_CMD_SET_OPENRECONNET_STATUS, cmd_sync_set_reconnect_status, 2);
}

void app_ibrt_customif_cmd_sync_music_eq(uint8_t index)
{
    if (!bts_tws_if_is_tws_link_connected())
    {
        return;
    }

	uint8_t cmd_sync_music_eq[2];
    cmd_sync_music_eq[0] = index;
	EARBUDS_TRACE(0, "[UITWS]%s index %d ",__func__, index);
	tws_ctrl_send_cmd(APP_TWS_CMD_SYNC_MUSIC_EQ, cmd_sync_music_eq, 2);
}

void app_ibrt_customif_cmd_sync_button_map(uint8_t *p_buff, uint16_t length)
{
    if (!bts_tws_if_is_tws_link_connected())
    {
        return;
    }

	uint8_t cmd_sync_button_map[50] = {0};
	memcpy(cmd_sync_button_map,p_buff,length);
	
	EARBUDS_TRACE(0, "[UITWS]%s index %d ",__func__, length);
	tws_ctrl_send_cmd(APP_TWS_CMD_SYNC_BUTTON_MAP, cmd_sync_button_map, length);
}

static void app_ibrt_customif_sync_set_reconnect_status_send(uint8_t *p_buff, uint16_t length)
{
	app_ibrt_send_cmd_without_rsp(APP_TWS_CMD_SET_OPENRECONNET_STATUS, p_buff, length);
}

static void app_ibrt_customif_sync_set_reconnect_status_send_handler(uint16_t rsp_seq,
                                                                     uint8_t *p_buff,
                                                                     uint16_t length)
{
    uint8_t device_id = 0;
    uint8_t recon_status = 0;

    EARBUDS_TRACE(1, "[UITWS]%s", __func__);
    EARBUDS_TRACE(2, "[UITWS]rsp_seq = %d length = %d", rsp_seq, length);
    DUMP8("%02x ", p_buff, length);

    if (length < 2)
    {
        EARBUDS_TRACE(1,
            "[NTT_RECONNECT_SYNC] invalid length=%d",
            length);
        return;
    }

    device_id = p_buff[0];
    recon_status = p_buff[1];

#ifdef BESUI_TWS_EN
    app_bt_mobile_set_openreconnect_info(device_id, recon_status, false);
#endif

#ifdef IBRT
    if (recon_status == 1)
    {
        if (app_ibrt_middleware_is_ui_slave())
        {
            EARBUDS_TRACE(2,
                "[NTT_RECONNECT_SYNC] slave start opening reconnect dev=%d status=%d",
                device_id,
                recon_status);

            app_bt_profile_connect_manager_opening_reconnect();
        }
        else
        {
            EARBUDS_TRACE(0,
                "[NTT_RECONNECT_SYNC] master recv sync, skip");
        }
    }
#endif
}

void app_ibrt_customif_cmd_sync_color_code(uint8_t color_code)
{
    if (!bts_tws_if_is_tws_link_connected())
    {
        EARBUDS_TRACE(0, "[COLOR_SYNC][TX] skip, tws not connected color=0x%02X", color_code);
        return;
    }

    if (!ntt_color_code_is_valid_local(color_code))
    {
        EARBUDS_TRACE(0, "[COLOR_SYNC][TX] invalid color=0x%02X", color_code);
        return;
    }

    EARBUDS_TRACE(0, "[COLOR_SYNC][TX] color=0x%02X", color_code);

    tws_ctrl_send_cmd(APP_TWS_CMD_SYNC_COLOR_CODE,
                      &color_code,
                      sizeof(color_code));
}

static void app_ibrt_customif_sync_color_code_send(uint8_t *p_buff, uint16_t length)
{
    app_ibrt_send_cmd_without_rsp(APP_TWS_CMD_SYNC_COLOR_CODE, p_buff, length);
}

static void app_ibrt_customif_sync_color_code_received_handler(uint16_t rsp_seq,
                                                               uint8_t *p_buff,
                                                               uint16_t length)
{
    if ((p_buff == NULL) || (length < 1))
    {
        EARBUDS_TRACE(0, "[COLOR_SYNC][RX] invalid len=%d", length);
        return;
    }

    uint8_t color_code = p_buff[0];

    if (!ntt_color_code_is_valid_local(color_code))
    {
        EARBUDS_TRACE(0, "[COLOR_SYNC][RX] invalid color=0x%02X", color_code);
        return;
    }

    EARBUDS_TRACE(0, "[COLOR_SYNC][RX] color=0x%02X", color_code);

    ntt_color_code_nv_set(color_code);
    ntt_ble_adv_refresh_data();
}

#if defined(USER_IMU_SENSORHUB_EN)
static void imu_data_send_handler(uint8_t *p_buff, uint16_t length)
{
	app_ibrt_send_cmd_without_rsp(APP_TWS_CMD_IMU_TX, p_buff, length);
}

static void imu_data_rcv_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
	// EARBUDS_TRACE(1, "[UIIMU]%s", __func__);
	// EARBUDS_TRACE(2, "[UIIMU]rsp_seq = %d length = %d", rsp_seq, length);
	// DUMP8("%02x ",p_buff,length);

    uictl.imu_buf[0] = (p_buff[0]<<24) | (p_buff[1]<<16) | (p_buff[2]<<8)  | (p_buff[3]<<0);
    uictl.imu_buf[1] = (p_buff[4]<<24) | (p_buff[5]<<16) | (p_buff[6]<<8)  | (p_buff[7]<<0);
    uictl.imu_buf[2] = (p_buff[8]<<24) | (p_buff[9]<<16) | (p_buff[10]<<8) | (p_buff[11]<<0);

    uictl.imu_slave_update = true;
}
#endif

#endif //#ifdef BESUI_TWS_EN

#if defined(CUSTOM_BITRATE) && !defined(FREEMAN_ENABLED_STERO)
static void app_ibrt_codec_user_info_sync(uint8_t *p_buff, uint16_t length);
static void app_ibrt_codec_user_info_sync_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
#endif

static const app_tws_cmd_instance_t g_ibrt_custom_cmd_handler_table[]=
{
    {
        APP_IBRT_CUSTOM_CMD_TEST1,                              "TWS_CMD_TEST1",
        app_ibrt_customif_test1_cmd_send,
        app_ibrt_customif_test1_cmd_send_handler,               0,
        app_ibrt_custom_cmd_rsp_timeout_handler_null,           app_ibrt_custom_cmd_rsp_handler_null,
        app_ibrt_custom_cmd_tx_done_handler_null,
        APP_TWS_CMD_PRIO_0
    },
    {
        APP_IBRT_CUSTOM_CMD_TEST2,                              "TWS_CMD_TEST2",
        app_ibrt_customif_test2_cmd_send,
        app_ibrt_customif_test2_cmd_send_handler,               RSP_TIMEOUT_DEFAULT,
        app_ibrt_customif_test2_cmd_send_rsp_timeout_handler,   app_ibrt_customif_test2_cmd_send_rsp_handler,
        app_ibrt_customif_test2_cmd_send_tx_done_handler,
        APP_TWS_CMD_PRIO_0
    },
//-------------------------------------------------------------------------------------------------------
#if 1 //def BESUI_TWS_EN
#ifdef BESUI_APP_EN
    {
        APP_TWS_CMD_SEND_TOTA_BATTERY_LEVEL_CMD,            "SYNC_TOTA_BATTERY_LEVEL",
        app_ibrt_sync_tota_battery_level,
        app_ibrt_sync_tota_battery_level_handler,        0,
        app_ibrt_cmd_rsp_timeout_handler_null,          app_ibrt_cmd_rsp_handler_null
    },
    {
        APP_TWS_CMD_SEND_TOTA_BATTERY_LEVEL_RSP,            "SYNC_TOTA_BATTERY_LEVEL_RSP",
        app_ibrt_sync_tota_battery_level_rsp,
        app_ibrt_sync_tota_battery_level_rsp_handler,        0,
        app_ibrt_cmd_rsp_timeout_handler_null,          app_ibrt_cmd_rsp_handler_null
    },
    {
        APP_TWS_CMD_CAPSENSOR_WEAR,                         "CAPSENSOR_WEAR_TX",
        app_ibrt_tws_capsensor_wear_tx,
        app_ibrt_tws_capsensor_wear_rx_handler,                              0,
        app_ibrt_cmd_rsp_timeout_handler_null,              app_ibrt_cmd_rsp_handler_null,
    },
#endif
#ifdef ALGO_INFO_SYNC_EN
    {
        APP_TWS_CMD_SYNC_EQ,                                    "APP_TWS_CMD_SYNC_EQ",
        app_ibrt_customif_sync_eq,
        app_ibrt_customif_sync_eq_handler,                      0,
        app_ibrt_cmd_rsp_timeout_handler_null,                  app_ibrt_cmd_rsp_handler_null
    },
    {
        APP_TWS_CMD_SYNC_BESSPA,                                "APP_TWS_CMD_SYNC_BESSPA",
        app_ibrt_customif_sync_besspa,
        app_ibrt_customif_sync_besspa_handler,                   0,
        app_ibrt_cmd_rsp_timeout_handler_null,                  app_ibrt_cmd_rsp_handler_null
    },
    {
        APP_TWS_CMD_SYNC_DOLBY,                                "APP_TWS_CMD_SYNC_DOLBY",
        app_ibrt_customif_sync_dolby,
        app_ibrt_customif_sync_dolby_handler,                   0,
        app_ibrt_cmd_rsp_timeout_handler_null,                  app_ibrt_cmd_rsp_handler_null
    },
#endif
    {
        APP_TWS_CMD_POWEROFF_SHUTDOWN_SYNC,                    "APP_TWS_CMD_POWEROFF_SHUTDOWN_SYNC",
        app_ibrt_customif_sync_poweroff_shutdown_send,
        app_ibrt_customif_sync_poweroff_shutdown_send_handler,                   0,
        app_ibrt_cmd_rsp_timeout_handler_null,                  app_ibrt_cmd_rsp_handler_null
    },
    {
        APP_TWS_CMD_BATTERY_LEVEL_SYNC,                    "APP_TWS_CMD_BATTERY_LEVEL_SYNC",
        app_ibrt_customif_sync_battery_level_send,
        app_ibrt_customif_sync_battery_level_send_handler,                   0,
        app_ibrt_cmd_rsp_timeout_handler_null,                  app_ibrt_cmd_rsp_handler_null
    },
#ifdef BESUI_GAME_EN
    {
        APP_TWS_CMD_SWITCH_GAME_MODE_SYNC,                    "APP_TWS_CMD_SWITCH_GAME_MODE_SYNC",
        app_ibrt_customif_sync_game_mode_send,
        app_ibrt_customif_sync_game_mode_send_handler,                   0,
        app_ibrt_cmd_rsp_timeout_handler_null,                  app_ibrt_cmd_rsp_handler_null
    },
#endif
    {
        APP_TWS_CMD_SWITCH_LED_MODE_SYNC,                    "APP_TWS_CMD_SWITCH_LED_MODE_SYNC",
        app_ibrt_customif_sync_led_mode_send,
        app_ibrt_customif_sync_led_mode_send_handler,                   0,
        app_ibrt_cmd_rsp_timeout_handler_null,                  app_ibrt_cmd_rsp_handler_null
    },
    {
        APP_TWS_CMD_BTADDR_SYNC,                    "APP_TWS_CMD_BTADDR_SYNC",
        app_ibrt_customif_sync_btaddr_send,
        app_ibrt_customif_sync_btaddr_send_handler,                   0,
        app_ibrt_cmd_rsp_timeout_handler_null,                  app_ibrt_cmd_rsp_handler_null
    },
    {
        APP_TWS_CMD_CLEAR_PIARLIST_SYNC,                    "APP_TWS_CMD_CLEAR_PIARLIST_SYNC",
        app_ibrt_customif_sync_clear_pairlist_send,
        app_ibrt_customif_sync_clear_pairlist_send_handler,                   0,
        app_ibrt_cmd_rsp_timeout_handler_null,                  app_ibrt_cmd_rsp_handler_null
    },

    {
        APP_TWS_CMD_SPACE_AUDIO_SYNC_STATUS,                    "APP_TWS_CMD_SPACE_AUDIO_SYNC_STATUS",
        app_ibrt_customif_sync_space_audio_status_send,
        app_ibrt_customif_sync_space_audio_status_send_handler,                   0,
        app_ibrt_cmd_rsp_timeout_handler_null,                  app_ibrt_cmd_rsp_handler_null
    },
    {
        APP_TWS_CMD_SET_OPENRECONNET_STATUS,                    "APP_TWS_CMD_SET_OPENRECONNET_STATUS",
        app_ibrt_customif_sync_set_reconnect_status_send,
        app_ibrt_customif_sync_set_reconnect_status_send_handler,                   0,
        app_ibrt_cmd_rsp_timeout_handler_null,                  app_ibrt_cmd_rsp_handler_null
    },
    {
        APP_TWS_CMD_SYNC_SOMETHING,                             "APP_TWS_CMD_SYNC_SOMETHING",
        app_ibrt_customif_sync_something,
        app_ibrt_customif_sync_something_handler,               0,
        app_ibrt_cmd_rsp_timeout_handler_null,                  app_ibrt_cmd_rsp_handler_null
    },
#if defined(OTA_BOOT_SYNC_EN)
    {
        APP_TWS_CMD_SYNC_OTABOOT,                                  "APP_TWS_CMD_SYNC_OTABOOT",
        app_ibrt_customif_sync_otaboot,
        app_ibrt_customif_sync_otaboot_handler,                   0,
        app_ibrt_cmd_rsp_timeout_handler_null,                  app_ibrt_cmd_rsp_handler_null
    },
#endif
#endif //#ifdef BESUI_TWS_EN
	{
        APP_TWS_CMD_SYNC_MUSIC_EQ,                              "TWS_CMD_MUSIC_EQ",
        app_ibrt_customif_music_eq_cmd_send,
        app_ibrt_customif_music_eq_cmd_send_handler,               0,
        app_ibrt_custom_cmd_rsp_timeout_handler_null,           app_ibrt_custom_cmd_rsp_handler_null,
        app_ibrt_custom_cmd_tx_done_handler_null,
        APP_TWS_CMD_PRIO_0
    },
    {
        APP_TWS_CMD_SYNC_BUTTON_MAP,                              "TWS_CMD_BUTTON_MAO",
        app_ibrt_customif_button_map_cmd_send,
        app_ibrt_customif_button_map_cmd_send_handler,               0,
        app_ibrt_custom_cmd_rsp_timeout_handler_null,           app_ibrt_custom_cmd_rsp_handler_null,
        app_ibrt_custom_cmd_tx_done_handler_null,
        APP_TWS_CMD_PRIO_0
    },
    {
        APP_TWS_CMD_SYNC_BT_NAME,                              "TWS_CMD_SYNC_BT_NAME",
        app_ibrt_customif_sync_bt_name_cmd_send,
        app_ibrt_customif_sync_bt_name_cmd_send_handler,               0,
        app_ibrt_custom_cmd_rsp_timeout_handler_null,           app_ibrt_custom_cmd_rsp_handler_null,
        app_ibrt_custom_cmd_tx_done_handler_null,
        APP_TWS_CMD_PRIO_0
    },
    {
        APP_TWS_CMD_SYNC_COLOR_CODE,                            "SYNC_COLOR_CODE",
        app_ibrt_customif_sync_color_code_send,
        app_ibrt_customif_sync_color_code_received_handler,             0,
        app_ibrt_custom_cmd_rsp_timeout_handler_null,           app_ibrt_custom_cmd_rsp_handler_null,
        app_ibrt_custom_cmd_tx_done_handler_null,
        APP_TWS_CMD_PRIO_0
    },
    {
        APP_TWS_CMD_SYNC_CASE_STATE,                        "SYNC_CASE_STATE",
        ntt_case_state_sync_send_handler,
        ntt_case_state_sync_receive_handler,                0,
        app_ibrt_custom_cmd_rsp_timeout_handler_null,       app_ibrt_custom_cmd_rsp_handler_null,
        app_ibrt_custom_cmd_tx_done_handler_null,
        APP_TWS_CMD_PRIO_0
    },
    {
        APP_TWS_CMD_KEY_RECONNECT_REQUEST,                  "KEY_RECONNECT_REQUEST",
        ntt_key_reconnect_request_send_handler,
        ntt_key_reconnect_request_receive_handler,              0,
        app_ibrt_custom_cmd_rsp_timeout_handler_null,       app_ibrt_custom_cmd_rsp_handler_null,
        app_ibrt_custom_cmd_tx_done_handler_null,
        APP_TWS_CMD_PRIO_0
    },
};

static app_tws_cmd_timer_instance_t *g_ibrt_custom_cmd_handler_var_table[ARRAY_SIZE(g_ibrt_custom_cmd_handler_table)];
/****************************function defination****************************/
int app_ibrt_customif_cmd_table_get(void **cmd_tbl, void ***cmd_var_tbl, uint16_t *cmd_size)
{
    *cmd_tbl = (void *)&g_ibrt_custom_cmd_handler_table;
    *cmd_size = ARRAY_SIZE(g_ibrt_custom_cmd_handler_table);
    *cmd_var_tbl = (void **)&g_ibrt_custom_cmd_handler_var_table;
    return 0;
}

void app_ibrt_customif_cmd_test(ibrt_custom_cmd_test_t *cmd_test)
{
    tws_ctrl_send_cmd(APP_IBRT_CUSTOM_CMD_TEST1, (uint8_t*)cmd_test, sizeof(ibrt_custom_cmd_test_t));
    tws_ctrl_send_cmd(APP_IBRT_CUSTOM_CMD_TEST2, (uint8_t*)cmd_test, sizeof(ibrt_custom_cmd_test_t));
}

static void app_ibrt_customif_test1_cmd_send(uint8_t *p_buff, uint16_t length)
{
    app_ibrt_send_cmd_without_rsp(APP_IBRT_CUSTOM_CMD_TEST1, p_buff, length);
    EARBUDS_TRACE(1, "%s", __func__);
}

static void app_ibrt_customif_test1_cmd_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    EARBUDS_TRACE(1, "%s", __func__);
}

static void app_ibrt_customif_test2_cmd_send(uint8_t *p_buff, uint16_t length)
{
    EARBUDS_TRACE(1, "%s", __func__);
    app_ibrt_send_cmd_with_rsp(APP_IBRT_CUSTOM_CMD_TEST2, p_buff, length);
}

static void app_ibrt_customif_test2_cmd_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    EARBUDS_TRACE(1, "%s", __func__);
    tws_ctrl_send_rsp(APP_IBRT_CUSTOM_CMD_TEST2, rsp_seq, p_buff, length);
}

static void app_ibrt_customif_test2_cmd_send_rsp_timeout_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    EARBUDS_TRACE(1, "%s", __func__);
}

static void app_ibrt_customif_test2_cmd_send_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    EARBUDS_TRACE(1, "%s", __func__);
}

static void app_ibrt_customif_test2_cmd_send_tx_done_handler(uint16_t cmdcode, uint16_t rsp_seq, uint8_t *ptrParam, uint16_t paramLen)
{
    EARBUDS_TRACE(1, "%s", __func__);

    EARBUDS_TRACE(0,
        "[NTT_TWS] role=%s ui_slave=%d tws_connected=%d mobile_connected=%d",
        app_ibrt_middleware_is_ui_slave() ? "SLAVE" : "MASTER",
        app_ibrt_middleware_is_ui_slave(),
        bts_tws_if_is_tws_link_connected(),
        app_bt_ibrt_has_mobile_link_connected());

        if (out_of_case_reconnect)
        {
            ntt_tws_reconnect_after_mobile_profiles_ready_check();
        }

}

static void app_ibrt_customif_music_eq_cmd_send(uint8_t *p_buff, uint16_t length)
{
    app_ibrt_send_cmd_without_rsp(APP_TWS_CMD_SYNC_MUSIC_EQ, p_buff, length);
    EARBUDS_TRACE(1, "%s", __func__);
}

static void app_ibrt_customif_music_eq_cmd_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    EARBUDS_TRACE(1, "%s", __func__);

	struct nvrecord_env_t *nvrecord_env;
	nv_record_env_get(&nvrecord_env);
	uint8_t eq_index = p_buff[0];
	if((eq_index >=0) && (eq_index <= 5))
	{
		if(eq_index != nvrecord_env->eq_index_data){
			nvrecord_env->eq_index_data = eq_index;
			nv_record_env_set(nvrecord_env);
			audio_eq_set_cfg(NULL, audio_eq_cfg_vol_list[eq_index], AUDIO_EQ_TYPE_HW_DAC_IIR);
            ntt_audio_drc_apply_by_eq_index(eq_index);
			
			EARBUDS_TRACE(1, "success_set_eq %s", __func__);
		}
		EARBUDS_TRACE(1, "%s,eq_index:%d", __func__,eq_index);
	}
			
}

static void app_ibrt_customif_button_map_cmd_send(uint8_t *p_buff, uint16_t length)
{
    app_ibrt_send_cmd_without_rsp(APP_TWS_CMD_SYNC_BUTTON_MAP, p_buff, length);
    EARBUDS_TRACE(1, "%s", __func__);
}

static keymap_load_config_cb keymap_load_config_callback = NULL;
void app_ibrt_customifsetbuttonmap_cb(keymap_load_config_cb callback)
{
    keymap_load_config_callback = callback;
}

static void app_ibrt_customif_button_map_cmd_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    EARBUDS_TRACE(1, "%s,length:%d", __func__,length);
	uint8_t button_number = p_buff[0];
	struct nvrecord_env_t *nvrecord_env;
    nv_record_env_get(&nvrecord_env);
	
	nvrecord_env->key_map_number = button_number;
	
	for(int i = 0;i<button_number;i++)
	{
		nvrecord_env->key_map_action[i] = p_buff[i*2+1];
		nvrecord_env->key_map_func[i] = p_buff[i*2+2];
	}

	nv_record_env_set(nvrecord_env);

	if(keymap_load_config_callback)
		keymap_load_config_callback();

#if 0	
	for(int i=0;i<button_number;i++){
		//printf("%02x,",p_buff[i]);
		EARBUDS_TRACE(1, "%02x,%02x,",nvrecord_env->key_map_action[i], nvrecord_env->key_map_func[i]);
	}
	EARBUDS_TRACE(1, "END%s", __func__);
#endif			
}

static void app_ibrt_customif_sync_bt_name_cmd_send(uint8_t *p_buff, uint16_t length)
{
    app_ibrt_send_cmd_without_rsp(APP_TWS_CMD_SYNC_BT_NAME, p_buff, length);
    EARBUDS_TRACE(1, "%s", __func__);
}

#define NTT_BT_NAME_MAX_LEN                 50
#define NTT_BT_NAME_DELAY_WRITE_MS          10000

static char ntt_pending_sync_bt_name[NTT_BT_NAME_MAX_LEN + 1] = {0};
static uint16_t ntt_pending_sync_bt_name_len = 0;
static bool ntt_pending_sync_bt_name_valid = false;
static osTimerId ntt_sync_bt_name_write_timer = NULL;

static void ntt_sync_bt_name_write_timer_handler(void const *param);

osTimerDef(NTT_SYNC_BT_NAME_WRITE_TIMER,
           ntt_sync_bt_name_write_timer_handler);

static bool ntt_is_a2dp_streaming_now(void)
{
#ifdef MEDIA_PLAYER_SUPPORT
    if (app_bt_stream_isrun(APP_BT_STREAM_A2DP_SBC) || app_bt_stream_isrun(APP_BT_STREAM_A2DP_AAC))
    {
        return true;
    }
#endif

    return false;
}

static void ntt_sync_bt_name_delay_write_start(void)
{
    if (ntt_sync_bt_name_write_timer == NULL)
    {
        ntt_sync_bt_name_write_timer =
            osTimerCreate(osTimer(NTT_SYNC_BT_NAME_WRITE_TIMER),
                          osTimerOnce,
                          NULL);
    }

    if (ntt_sync_bt_name_write_timer)
    {
        osTimerStop(ntt_sync_bt_name_write_timer);
        osTimerStart(ntt_sync_bt_name_write_timer,
                     NTT_BT_NAME_DELAY_WRITE_MS);
    }
}

static void ntt_sync_bt_name_write_timer_handler(void const *param)
{
    if (!ntt_pending_sync_bt_name_valid)
    {
        return;
    }

    if (ntt_is_a2dp_streaming_now())
    {
        EARBUDS_TRACE(0,
            "[BT_NAME_SYNC] A2DP streaming, delay factory write again");

        ntt_sync_bt_name_delay_write_start();
        return;
    }

    EARBUDS_TRACE(2,
        "[BT_NAME_SYNC] factory write name=%s len=%d",
        ntt_pending_sync_bt_name,
        ntt_pending_sync_bt_name_len);

    if (factory_section_set_bt_name(ntt_pending_sync_bt_name,
                                    ntt_pending_sync_bt_name_len))
    {
        EARBUDS_TRACE(0,
            "[BT_NAME_SYNC] factory_section_set_bt_name failed");

        ntt_sync_bt_name_delay_write_start();
        return;
    }

    ntt_pending_sync_bt_name_valid = false;

    EARBUDS_TRACE(0,
        "[BT_NAME_SYNC] factory write done");
}

static void app_ibrt_customif_sync_bt_name_cmd_send_handler(uint16_t rsp_seq,
                                                            uint8_t *p_buff,
                                                            uint16_t length)
{
    EARBUDS_TRACE(1, "%s,length:%d", __func__, length);

    char nameBuffer[NTT_BT_NAME_MAX_LEN + 1] = {0};
    uint16_t name_len = 0;

    if (p_buff == NULL || length == 0)
    {
        EARBUDS_TRACE(0, "[BT_NAME_SYNC] invalid name data");
        return;
    }

    name_len = (length > NTT_BT_NAME_MAX_LEN) ?
               NTT_BT_NAME_MAX_LEN : length;

    memcpy(nameBuffer, p_buff, name_len);
    nameBuffer[name_len] = '\0';

    const char *old_name = (const char *)factory_section_get_bt_name();

    if (old_name && strcmp(old_name, nameBuffer) == 0)
    {
        EARBUDS_TRACE(1,
            "[BT_NAME_SYNC] same name, skip factory write: %s",
            nameBuffer);
        return;
    }

    EARBUDS_TRACE(2,
        "[BT_NAME_SYNC] pending update name: old=%s new=%s",
        old_name ? old_name : "NULL",
        nameBuffer);

    memset(ntt_pending_sync_bt_name, 0, sizeof(ntt_pending_sync_bt_name));
    memcpy(ntt_pending_sync_bt_name, nameBuffer, name_len + 1);

    ntt_pending_sync_bt_name_len = name_len + 1;
    ntt_pending_sync_bt_name_valid = true;

    ntt_sync_bt_name_delay_write_start();
}

void app_ibrt_customif_cmd_sync_bt_name(uint8_t *p_buff, uint16_t length)
{
	if (!bts_tws_if_is_tws_link_connected())
    {
        return;
    }

	uint8_t cmd_sync_bt_name[50] = {0};
	memcpy(cmd_sync_bt_name,p_buff,length);
	
	EARBUDS_TRACE(0, "[UITWS]%s index %d ",__func__, length);
	tws_ctrl_send_cmd(APP_TWS_CMD_SYNC_BT_NAME, cmd_sync_bt_name, length);
}

#define NTT_CASE_SYNC_VERSION       1
#define NTT_CASE_SYNC_MAGIC         0xCA

typedef struct __attribute__((packed))
{
    uint8_t magic;
    uint8_t version;
    uint8_t case_state;
    uint8_t sequence;
} NTT_CASE_SYNC_PACKET_T;

static volatile NTT_CASE_STATE_E g_ntt_local_case_state = NTT_CASE_STATE_UNKNOWN;
static volatile NTT_CASE_STATE_E g_ntt_peer_case_state = NTT_CASE_STATE_UNKNOWN;
static uint8_t g_ntt_case_state_sequence = 0;


/*
 * Optional callback。
 *
 * 可在 app_ibrt_customif_ui.cpp 提供強實作，
 * 當本機 case state 改變時執行 reconnect、role switch 等流程。
 */
/*
 * Optional callback.
 *
 * app_ibrt_customif_ui.cpp 可以提供同名 strong function。
 */
extern "C" void ntt_case_state_local_changed_callback(
    NTT_CASE_STATE_E state) __attribute__((weak));

extern "C" void ntt_case_state_peer_changed_callback(
    NTT_CASE_STATE_E state) __attribute__((weak));


extern "C" void ntt_case_state_local_changed_callback(
    NTT_CASE_STATE_E state)
{
    EARBUDS_TRACE(1,"[NTT_CASE_SYNC][WEAK_LOCAL_CB] state=%d",state);
}


extern "C" void ntt_case_state_peer_changed_callback(
    NTT_CASE_STATE_E state)
{
    EARBUDS_TRACE(1,"[NTT_CASE_SYNC][WEAK_PEER_CB] state=%d",state);
}

/*
 * Optional callback。
 *
 * Peer 狀態更新時呼叫。
 */

static const char *ntt_case_state_to_string(NTT_CASE_STATE_E state)
{
    switch (state)
    {
        case NTT_CASE_STATE_IN_CASE:
            return "IN_CASE";

        case NTT_CASE_STATE_OUT_CASE:
            return "OUT_CASE";

        default:
            return "UNKNOWN";
    }
}


static bool ntt_case_state_is_valid(uint8_t state)
{
    return state == NTT_CASE_STATE_IN_CASE || state == NTT_CASE_STATE_OUT_CASE;
}


/*
 * 真正透過 IBRT link 傳送封包。
 */
static bool ntt_case_state_send_to_peer(NTT_CASE_STATE_E state)
{
    NTT_CASE_SYNC_PACKET_T packet;

    if (!ntt_case_state_is_valid((uint8_t)state))
    {
        EARBUDS_TRACE(1,"[NTT_CASE_SYNC] invalid send state=%d",state);
        return false;
    }

    if (!bts_tws_if_is_tws_link_connected())
    {
        EARBUDS_TRACE(2,"[NTT_CASE_SYNC] TWS disconnected, save only state=%s(%d)",ntt_case_state_to_string(state),state);
        return false;
    }

    packet.magic = NTT_CASE_SYNC_MAGIC;
    packet.version = NTT_CASE_SYNC_VERSION;
    packet.case_state = (uint8_t)state;
    packet.sequence = ++g_ntt_case_state_sequence;

    EARBUDS_TRACE(3,"[NTT_CASE_SYNC][TX] state=%s(%d) seq=%u",ntt_case_state_to_string(state),state,packet.sequence);
    tws_ctrl_send_cmd(APP_TWS_CMD_SYNC_CASE_STATE,(uint8_t *)&packet,sizeof(packet));
    return true;
}


/*
 * Battery debounce 完成後的主要入口。
 */
extern "C"
void ntt_case_state_sync_local_update(bool in_case)
{
    NTT_CASE_STATE_E new_state = in_case ? NTT_CASE_STATE_IN_CASE : NTT_CASE_STATE_OUT_CASE;
    NTT_CASE_STATE_E old_state = g_ntt_local_case_state;
    if (old_state == new_state)
    {
        EARBUDS_TRACE(2,"[NTT_CASE_SYNC] duplicate local ignored state=%s(%d)",ntt_case_state_to_string(new_state),new_state);
        /*
         * 即使狀態未改變，若 TWS 剛恢復連線，
         * resend() 會負責重新傳送。
         */
        return;
    }

    g_ntt_local_case_state = new_state;

    EARBUDS_TRACE(
        3,
        "[NTT_CASE_SYNC] local changed %s(%d) -> %s(%d)",
        ntt_case_state_to_string(old_state),
        old_state,
        ntt_case_state_to_string(new_state),
        new_state);

    /*
     * 先執行本機流程。
     */
    ntt_case_state_local_changed_callback(new_state);

    /*
     * 再同步給另一耳。
     */
    ntt_case_state_send_to_peer(new_state);
}


/*
 * TWS 連線建立後重新同步一次。
 */
extern "C"
void ntt_case_state_sync_resend(void)
{
    NTT_CASE_STATE_E state = g_ntt_local_case_state;

    EARBUDS_TRACE(
        3,
        "[NTT_CASE_SYNC] resend local=%s(%d) tws=%d",
        ntt_case_state_to_string(state),
        state,
        bts_tws_if_is_tws_link_connected());

    if (state == NTT_CASE_STATE_UNKNOWN)
    {
        EARBUDS_TRACE(0,"[NTT_CASE_SYNC] resend ignored: local unknown");
        return;
    }

    ntt_case_state_send_to_peer(state);
}


/*
 * Custom command 發送 handler。
 *
 * tws_ctrl_send_cmd() 最後會進到這裡。
 */
static void ntt_case_state_sync_send_handler(uint8_t *p_buff,uint16_t length)
{
    if (p_buff == NULL || length != sizeof(NTT_CASE_SYNC_PACKET_T))
    {
        EARBUDS_TRACE(2,"[NTT_CASE_SYNC][SEND] invalid buffer=%p length=%u",p_buff,length);
        return;
    }

    app_ibrt_send_cmd_without_rsp(APP_TWS_CMD_SYNC_CASE_STATE,p_buff,length);
}


/*
 * Peer 收到命令後的 handler。
 */
static void ntt_case_state_sync_receive_handler(uint16_t rsp_seq,uint8_t *p_buff,uint16_t length)
{
    NTT_CASE_SYNC_PACKET_T packet;
    NTT_CASE_STATE_E new_peer_state;
    NTT_CASE_STATE_E old_peer_state;

    (void)rsp_seq;

    if (p_buff == NULL || length != sizeof(NTT_CASE_SYNC_PACKET_T))
    {
        EARBUDS_TRACE(2,"[NTT_CASE_SYNC][RX] invalid buffer=%p length=%u",p_buff,length);
        return;
    }

    memcpy(&packet,p_buff,sizeof(packet));

    if (packet.magic != NTT_CASE_SYNC_MAGIC)
    {
        EARBUDS_TRACE(1,"[NTT_CASE_SYNC][RX] invalid magic=0x%02X",packet.magic);
        return;
    }

    if (packet.version != NTT_CASE_SYNC_VERSION)
    {
        EARBUDS_TRACE(
            2,
            "[NTT_CASE_SYNC][RX] unsupported version=%u expected=%u",
            packet.version,
            NTT_CASE_SYNC_VERSION);

        return;
    }

    if (!ntt_case_state_is_valid(packet.case_state))
    {
        EARBUDS_TRACE(1,"[NTT_CASE_SYNC][RX] invalid state=%u",packet.case_state);
        return;
    }

    new_peer_state = (NTT_CASE_STATE_E)packet.case_state;
    old_peer_state = g_ntt_peer_case_state;
    EARBUDS_TRACE(
        4,
        "[NTT_CASE_SYNC][RX] peer state=%s(%d) seq=%u old=%s(%d)",
        ntt_case_state_to_string(new_peer_state),
        new_peer_state,
        packet.sequence,
        ntt_case_state_to_string(old_peer_state),
        old_peer_state);

    if (old_peer_state == new_peer_state)
    {
        EARBUDS_TRACE(1,"[NTT_CASE_SYNC][RX] duplicate peer ignored");
        return;
    }

    g_ntt_peer_case_state = new_peer_state;
    ntt_case_state_peer_changed_callback(new_peer_state);
    EARBUDS_TRACE(
        4,
        "[NTT_CASE_SYNC] result local=%s(%d) peer=%s(%d)",
        ntt_case_state_to_string(g_ntt_local_case_state),
        g_ntt_local_case_state,
        ntt_case_state_to_string(g_ntt_peer_case_state),
        g_ntt_peer_case_state);
}


extern "C"
NTT_CASE_STATE_E ntt_case_state_get_local(void)
{
    return g_ntt_local_case_state;
}


extern "C"
NTT_CASE_STATE_E ntt_case_state_get_peer(void)
{
    return g_ntt_peer_case_state;
}


extern "C"
bool ntt_case_state_is_local_out(void)
{
    return g_ntt_local_case_state == NTT_CASE_STATE_OUT_CASE;
}


extern "C"
bool ntt_case_state_is_peer_out(void)
{
    return g_ntt_peer_case_state == NTT_CASE_STATE_OUT_CASE;
}


extern "C"
bool ntt_case_state_are_both_out(void)
{
    return g_ntt_local_case_state == NTT_CASE_STATE_OUT_CASE && g_ntt_peer_case_state == NTT_CASE_STATE_OUT_CASE;
}

#define NTT_KEY_RECONNECT_LOCK_MS 3000

static bool g_ntt_key_reconnect_running = false;

static void ntt_key_reconnect_unlock_handler(
    void const *param);

osTimerDef(
    NTT_KEY_RECONNECT_UNLOCK_TIMER,
    ntt_key_reconnect_unlock_handler);

static osTimerId g_ntt_key_reconnect_unlock_timer = NULL;

static void ntt_key_reconnect_unlock_handler(
    void const *param)
{
    (void)param;

    g_ntt_key_reconnect_running = false;

    EARBUDS_TRACE(
        0,
        "[NTT_KEY_RECONNECT] unlock");
}

static void ntt_key_reconnect_lock_start(void)
{
    if (g_ntt_key_reconnect_unlock_timer == NULL)
    {
        g_ntt_key_reconnect_unlock_timer =
            osTimerCreate(
                osTimer(
                    NTT_KEY_RECONNECT_UNLOCK_TIMER),
                osTimerOnce,
                NULL);
    }

    if (g_ntt_key_reconnect_unlock_timer != NULL)
    {
        osTimerStop(
            g_ntt_key_reconnect_unlock_timer);

        osTimerStart(
            g_ntt_key_reconnect_unlock_timer,
            NTT_KEY_RECONNECT_LOCK_MS);
    }
}

static void ntt_key_master_start_reconnect(void)
{
    uint8_t role =
        app_ibrt_if_get_ui_role();

    uint8_t conn_devices =
        app_bt_count_connected_device();

    EARBUDS_TRACE(
        3,
        "[NTT_KEY_RECONNECT] execute role=%d conn=%d running=%d",
        role,
        conn_devices,
        g_ntt_key_reconnect_running);

    if (role != TWS_UI_MASTER)
    {
        EARBUDS_TRACE(
            0,
            "[NTT_KEY_RECONNECT] reject: not MASTER");

        return;
    }

    if (conn_devices != 0)
    {
        EARBUDS_TRACE(
            0,
            "[NTT_KEY_RECONNECT] reject: mobile connected");

        return;
    }

    if (g_ntt_key_reconnect_running)
    {
        EARBUDS_TRACE(
            0,
            "[NTT_KEY_RECONNECT] duplicate ignored");

        return;
    }

    g_ntt_key_reconnect_running = true;

    ntt_key_reconnect_lock_start();

    EARBUDS_TRACE(
        0,
        "[NTT_KEY_RECONNECT] MASTER opening reconnect");

    app_bt_profile_connect_manager_opening_reconnect();
}

static void ntt_key_reconnect_request_send_handler(
    uint8_t *p_buff,
    uint16_t length)
{
    app_ibrt_send_cmd_without_rsp(
        APP_TWS_CMD_KEY_RECONNECT_REQUEST,
        p_buff,
        length);
}

static void ntt_key_reconnect_request_receive_handler(
    uint16_t rsp_seq,
    uint8_t *p_buff,
    uint16_t length)
{
    (void)rsp_seq;
    (void)p_buff;
    (void)length;

    EARBUDS_TRACE(
        2,
        "[NTT_KEY_RECONNECT][RX] role=%d",
        app_ibrt_if_get_ui_role());

    /*
     * 只有 Master 處理 Slave 的回連要求。
     */
    if (app_ibrt_if_get_ui_role() != TWS_UI_MASTER)
    {
        EARBUDS_TRACE(
            0,
            "[NTT_KEY_RECONNECT][RX] ignore: not MASTER");

        return;
    }

    ntt_key_master_start_reconnect();
}

extern "C"
void ntt_key_request_mobile_reconnect(void)
{
    uint8_t role =
        app_ibrt_if_get_ui_role();

    uint8_t conn_devices =
        app_bt_count_connected_device();

    EARBUDS_TRACE(
        3,
        "[NTT_KEY_RECONNECT] request role=%d conn=%d tws=%d",
        role,
        conn_devices,
        bts_tws_if_is_tws_link_connected());

    if (conn_devices != 0)
    {
        EARBUDS_TRACE(
            0,
            "[NTT_KEY_RECONNECT] mobile already connected");

        return;
    }

    /*
     * Master 本機按鍵，直接執行。
     */
    if (role == TWS_UI_MASTER)
    {
        ntt_key_master_start_reconnect();
        return;
    }

    /*
     * Slave 按鍵，通知 Master。
     */
    if (bts_tws_if_is_tws_link_connected())
    {
        uint8_t request = 1;

        EARBUDS_TRACE(
            0,
            "[NTT_KEY_RECONNECT] SLAVE send request to MASTER");

        tws_ctrl_send_cmd(
            APP_TWS_CMD_KEY_RECONNECT_REQUEST,
            &request,
            sizeof(request));

        return;
    }

    /*
     * Slave 且沒有 TWS：
     * 目前不允許直接連手機，避免雙耳競爭。
     */
    EARBUDS_TRACE(
        0,
        "[NTT_KEY_RECONNECT] SLAVE no TWS, request ignored");
}

#endif /* IBRT */
