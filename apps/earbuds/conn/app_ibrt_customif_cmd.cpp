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

#if defined(BESUI_TWS_EN) || defined(BESUI_STEREO_EN)
#include "apps.h"
#include "besui_common.h"
#endif

#if defined(BESUI_TWS_EN)
#include "twsui_comm.h"
#endif
#include "bts_tws_if.h"
#include "bts_core_if.h"

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

#ifdef BESUI_TWS_EN
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
    if(p_buff[0] == USER_TWS_CMD_ENTER_PAIRMODE)
    {
        besui_bt_msg_put(SLAVE_ENTER_PAIRMODE_EVENT, 0xff, BT_DEVICE_NUM);
    }
    else if(p_buff[0] == USER_TWS_CMD_SLAVE_LINKLOSS)
    {
        besui_bt_msg_put(SLAVE_LINKLOSS_EVENT, 0xff, BT_DEVICE_NUM);
    }
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
        uictl.shutdown_type = SHUTDOWN_SYNC_PEER;
        app_shutdown();
    }
}

void app_ibrt_customif_cmd_sync_battery_level(uint8_t current_level)
{
    if (!bts_tws_if_is_tws_link_connected())
    {
        return;
    }

	uint8_t cmd_sync_battery_level[1];
	cmd_sync_battery_level[0] = current_level;
	EARBUDS_TRACE(2, "[UITWS]%s current_level %d",__func__, current_level);
	tws_ctrl_send_cmd(APP_TWS_CMD_BATTERY_LEVEL_SYNC, cmd_sync_battery_level, 1);
}

static void app_ibrt_customif_sync_battery_level_send(uint8_t *p_buff, uint16_t length)
{
	app_ibrt_send_cmd_without_rsp(APP_TWS_CMD_BATTERY_LEVEL_SYNC, p_buff, length);
}

static void app_ibrt_customif_sync_battery_level_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
	EARBUDS_TRACE(1, "[UITWS]%s", __func__);
	EARBUDS_TRACE(2, "[UITWS]rsp_seq = %d length = %d", rsp_seq, length);
	DUMP8("%02x ",p_buff,length);

    app_battery_set_other_battery_level(p_buff[0]);
    app_tws_battery_update(true);
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

    app_tws_set_ledsta_call_slave_process(p_buff[0], p_buff[1]);
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

    app_common_store_twsaddrto_nv_flash(p_buff);
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
    app_common_clear_pairlist_process(p_buff[0], p_buff[1], true, false);
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

static void app_ibrt_customif_sync_set_reconnect_status_send(uint8_t *p_buff, uint16_t length)
{
	app_ibrt_send_cmd_without_rsp(APP_TWS_CMD_SET_OPENRECONNET_STATUS, p_buff, length);
}

static void app_ibrt_customif_sync_set_reconnect_status_send_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
	EARBUDS_TRACE(1, "[UITWS]%s", __func__);
	EARBUDS_TRACE(2, "[UITWS]rsp_seq = %d length = %d", rsp_seq, length);
	DUMP8("%02x ",p_buff,length);
    app_bt_mobile_set_openreconnect_info(p_buff[0], p_buff[1], false);
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
#ifdef BESUI_TWS_EN
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
}
#endif /* IBRT */
