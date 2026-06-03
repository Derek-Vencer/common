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
#include "stdio.h"
#include "cmsis_os.h"
#include "list.h"
#include "string.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_bootmode.h"
#include "pmu.h"
#include "apps.h"
#include "app_thread.h"
#include "app_media_player.h"
#include "app_utils.h"
#include "norflash_api.h"
#include "nvrecord_env.h"
#include "bts_tws_if.h"
#include "bts_core_if.h"

#ifdef ANC_APP
#include "app_anc.h"
#if defined(USER_NOISE_ADAPTIVE_ANC_EN)
#include "app_voice_assist_noise_adapt_anc.h"
#endif
#endif

#if defined(CAPSENSOR_ENABLE) && defined(BESUI_APP_EN)
#include CHIP_SPECIFIC_HDR(capsensor_driver)
#endif

#ifdef BESUI_CAPSENSOR_FACTORY_EN
#include "capsensor_factory_cal.h"
#endif

#ifdef BESUI_APP_EN
#include "app_tota_general.h"
#include "app_tota.h"
#endif

#ifdef BESUI_TWS_EN
#include "twsui_comm.h"
#include "twsui_uart.h"
#endif
#ifdef BESUI_STEREO_EN
#include "stereoui.h"
#endif
#if defined(BESUI_COMM_EN)
#include "besui_common.h"
#endif

#include "besui_hfp.h"
#include "app_ibrt_internal.h"

#if defined(BESUI_TWS_EN) || defined(BESUI_STEREO_EN)
extern "C" int32_t bt_sco_chain_bypass_tx_algo(uint32_t sel_ch);
extern "C" int32_t bt_sco_chain_enc_onoff(uint32_t sel_ch);

const char at_cmd[][30] = 
{
    "AT+TALK_MIC",
    "AT+FF_MIC",
    "AT+FB_MIC",
    "AT_ENC_MIC",
    "AT+SHIP_MODE",
    "AT+GETSWVER",
    "AT+ANC_ON",
    "AT+ANC_AA",
    "AT+ANC_OFF",

    "AT+ENC_ON",
    "AT+ENC_OFF",
    "AT+BTADDR",
    "AT+BTNAME",
    "AT+BATTERY",
    "AT+SURROUND_ON", 
    "AT+SURROUND_OFF", 
    "AT+INEAR_ON", 
    "AT+INEAR_OFF", 
    "AT+POWER_OFF",
#ifdef BESUI_CAPSENSOR_FACTORY_EN
    "AT+CAP_RESTART",
    "AT+CAP_OFFSET",
    "AT+CAP_DIFF",
#endif
    "AT+NORMAL_MODE",
    "AT+RESET",
    "AT+ADAPT_ANC_ON",
    "AT+ADAPT_ANC_OFF",
};


uint8_t app_at_compare_cmd_handle(char *cmd_data)
{
    uint8_t i = 0;

    BESUI_TRACE(1,"[UIHFP]%s at_cmd len %d", __func__, (sizeof(at_cmd)/sizeof(at_cmd[0])));

    for(i = 0; i < (sizeof(at_cmd)/sizeof(at_cmd[0])); i++)
    {
        BESUI_TRACE(1,"[UIHFP]%s len %d", __func__, strlen(at_cmd[i]));
        if(strncmp(cmd_data, at_cmd[i], strlen(at_cmd[i])) == 0)
        {
            return i;
        }
    }

    return 0xFF;
}

void app_at_cmd_at_talk_mic_handle(uint8_t device_id)
{
    char at_cmd[10] = {'\0'};

    sprintf(at_cmd, "%s\r", ATOK);

    btif_hf_send_at_cmd((btif_hf_channel_t*)app_bt_get_device(device_id)->hf_channel, at_cmd);

#if (SPEECH_CODEC_CAPTURE_CHANNEL_NUM == 2)
    app_common_mic_hfp_enc_control(false, ONLY_OPEN_MASTER_MIC);
#elif (SPEECH_CODEC_CAPTURE_CHANNEL_NUM > 2)
    //bt_sco_chain_bypass_tx_algo(2);
    bt_sco_chain_enc_onoff(2);
#else
    speech_mic_set_onoff(SPEECH_TEST_MIC1);
#ifdef ANC_APP
    //anc_ff_fb_set_onoff(ANC_TEST_FF);
#endif
#endif
}

void app_at_cmd_at_ff_mic_handle(uint8_t device_id)
{
    char at_cmd[10] = {'\0'};

    sprintf(at_cmd, "%s\r", ATOK);

    btif_hf_send_at_cmd((btif_hf_channel_t*)app_bt_get_device(device_id)->hf_channel, at_cmd);

#if (SPEECH_CODEC_CAPTURE_CHANNEL_NUM == 2)
    app_common_mic_hfp_enc_control(false, ONLY_OPEN_SLAVE_MIC);
#elif (SPEECH_CODEC_CAPTURE_CHANNEL_NUM > 2)
    // bt_sco_chain_bypass_tx_algo(1); 
    bt_sco_chain_enc_onoff(1);
#else
    speech_mic_set_onoff(SPEECH_TEST_MIC1);
#ifdef ANC_APP
    //anc_ff_fb_set_onoff(ANC_TEST_FF);
#endif
#endif
}


void app_at_cmd_at_fb_mic_handle(uint8_t device_id)
{
    char at_cmd[10] = {'\0'};

    sprintf(at_cmd, "%s\r", ATOK);

    btif_hf_send_at_cmd((btif_hf_channel_t*)app_bt_get_device(device_id)->hf_channel, at_cmd);

#if (SPEECH_CODEC_CAPTURE_CHANNEL_NUM == 2)
    app_common_mic_hfp_enc_control(false, ONLY_OPEN_MASTER_MIC);
#elif (SPEECH_CODEC_CAPTURE_CHANNEL_NUM > 2)
    // bt_sco_chain_bypass_tx_algo(0);
    bt_sco_chain_enc_onoff(0);
#else
    speech_mic_set_onoff(SPEECH_TEST_MIC0);
#ifdef ANC_APP
    //anc_ff_fb_set_onoff(ANC_TEST_FF);
#endif
#endif
}


void app_at_cmd_at_enc_mic_handle(uint8_t device_id)
{
    char at_cmd[10] = {'\0'};

    sprintf(at_cmd, "%s\r", ATOK);

    btif_hf_send_at_cmd((btif_hf_channel_t*)app_bt_get_device(device_id)->hf_channel, at_cmd);

    app_common_mic_hfp_enc_control(true, OPEN_MASTER_SLAVE_MIC);
}

void app_at_cmd_at_ship_mode_handle(uint8_t device_id)
{
    char at_cmd[10] = {'\0'};

    sprintf(at_cmd, "%s\r", ATOK);

    btif_hf_send_at_cmd((btif_hf_channel_t*)app_bt_get_device(device_id)->hf_channel, at_cmd);
#ifdef BESUI_TWS_EN
    uicom.ship_mode_flag = true;
    uictl.poweroff_fast_flag = true;
#endif
    uictl.shutdown_type = SHUTDOWN_SHIP_MODE;
    app_shutdown();
}

void app_at_cmd_at_get_software_version_handle(uint8_t device_id)
{
    char at_cmd[30] = {'\0'};

    sprintf(at_cmd, "%s", AT_SW_VER);

    at_cmd[11] = '.';
    at_cmd[12] = SOFTWARE_VERSION_INFO+'0';
    at_cmd[13] = '.';
    at_cmd[14] = SOFTWARE_VERSION_INFO+'0';
    at_cmd[15] = '.';
    at_cmd[16] = SOFTWARE_VERSION_INFO+'0';
    at_cmd[17] = '_';

    const char month[12][4] = {"Jan","Feb","Mar","Apr","May","Jun",
                               "Jul","Aug","Sep","Oct","Nov","Dec",};
    char dat_buf[11] = {0};
    memcpy(dat_buf, __DATE__, sizeof(dat_buf));
    BESUI_TRACE(2, "[UIHFP][%s]Build Date  = %s, %s", __func__, __DATE__, __TIME__);
    for(uint8_t i = 0;i < 12;i ++)
    {
        if(!memcmp(month[i], dat_buf, 3))
        {
            at_cmd[22] = (i+1)/10 + '0';
            at_cmd[23] = (i+1)%10 + '0';
        }
    }

    at_cmd[18] = dat_buf[7];
    at_cmd[19] = dat_buf[8];
    at_cmd[20] = dat_buf[9];
    at_cmd[21] = dat_buf[10];

    at_cmd[24] = dat_buf[4];
    at_cmd[25] = dat_buf[5];

    at_cmd[26] = '\r';

    btif_hf_send_at_cmd((btif_hf_channel_t*)app_bt_get_device(device_id)->hf_channel, at_cmd);

}

void app_at_cmd_at_anc_on_handle(uint8_t device_id)
{
    char at_cmd[10] = {'\0'};

    sprintf(at_cmd, "%s\r", ATOK);

    btif_hf_send_at_cmd((btif_hf_channel_t*)app_bt_get_device(device_id)->hf_channel, at_cmd);
    #if defined(ANC_APP)
    besui_anc_set_mode(1);
    #endif
}


void app_at_cmd_at_anc_aa_handle(uint8_t device_id)
{
    char at_cmd[10] = {'\0'};

    sprintf(at_cmd, "%s\r", ATOK);

    btif_hf_send_at_cmd((btif_hf_channel_t*)app_bt_get_device(device_id)->hf_channel, at_cmd);
    #if defined(ANC_APP)
    besui_anc_set_mode(2);
    #endif
}


void app_at_cmd_at_anc_off_handle(uint8_t device_id)
{
    char at_cmd[10] = {'\0'};

    sprintf(at_cmd, "%s\r", ATOK);

    btif_hf_send_at_cmd((btif_hf_channel_t*)app_bt_get_device(device_id)->hf_channel, at_cmd);
    #if defined(ANC_APP)
    besui_anc_set_mode(0);
    #endif
}

void app_at_cmd_at_enc_on_handle(uint8_t device_id)
{
    char at_cmd[10] = {'\0'};

    sprintf(at_cmd, "%s\r", ATOK);

    BESUI_TRACE(0,"[UIHFP]%s,AT send = ", __func__);
    DUMP8("%02x ", at_cmd, sizeof(at_cmd));
    btif_hf_send_at_cmd((btif_hf_channel_t*)app_bt_get_device(device_id)->hf_channel, at_cmd);

#if defined(SPEECH_TX_2MIC_NS8)
    app_common_mic_hfp_enc_control(true, OPEN_MASTER_SLAVE_MIC);
#else
    //bt_sco_chain_bypass_tx_algo(0xFF);
    bt_sco_chain_enc_onoff(0xFF);
#endif
}
void app_at_cmd_at_enc_off_handle(uint8_t device_id)
{
    char at_cmd[10] = {'\0'};

    sprintf(at_cmd, "%s\r", ATOK);
    BESUI_TRACE(0,"[UIHFP]%s,AT send = ", __func__);
    DUMP8("%02x ", at_cmd, sizeof(at_cmd));
    btif_hf_send_at_cmd((btif_hf_channel_t*)app_bt_get_device(device_id)->hf_channel, at_cmd);

#if defined(SPEECH_TX_2MIC_NS8)
    app_common_mic_hfp_enc_control(false, CLOSE_MASTER_SLAVE_MIC);
#else
    //bt_sco_chain_bypass_tx_algo(1);
    bt_sco_chain_enc_onoff(1);
#endif
}
void app_at_cmd_at_btaddr_handle(uint8_t device_id)
{
    BESUI_TRACE(0,"[UIHFP]%s", __func__);

    char at_cmd[25] = {'\0'};
    char hex_tab[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
    sprintf(at_cmd, "%s", "AT+BTADDR,");

    uint8_t *bt_mac_addr;
    ibrt_ctrl_t *p_ibrt_ctrl = app_ibrt_if_get_bt_ctrl_ctx();
    struct nvrecord_env_t *nvrecord_env;
    nv_record_env_get(&nvrecord_env);
    uint8_t i = 0;

    //bt_mac_addr = bt_get_local_address();
#ifdef BESUI_TWS_EN
    if(bts_tws_if_is_tws_link_connected())
    {
        bt_mac_addr = p_ibrt_ctrl->local_addr.address;
    }
    else
    {
        bt_mac_addr = nvrecord_env->tws_con_addr;
    }
#else
    bt_mac_addr = p_ibrt_ctrl->local_addr.address;   
#endif
    for(i = 0; i < 6; i++)
    {
        at_cmd[10+2*i] = hex_tab[((bt_mac_addr[i]>>4)&0x0F)];
        at_cmd[10+2*i+1] = hex_tab[(bt_mac_addr[i]&0x0F)];
    }
    at_cmd[22] = '\r';

    BESUI_TRACE(0,"[UIHFP]%s,AT send = %s,", __func__, at_cmd);
    DUMP8("%02x ", at_cmd, sizeof(at_cmd));
    btif_hf_send_at_cmd((btif_hf_channel_t*)app_bt_get_device(device_id)->hf_channel, at_cmd);
}
void app_at_cmd_at_btname_handle(uint8_t device_id)
{
    BESUI_TRACE(0,"[UIHFP]%s", __func__);
    char at_cmd[30] = {'\0'};
    sprintf(at_cmd, "%s", "AT+BTNAME,");

    const char *localname = bt_get_local_name();
    uint8_t name_len = strlen(localname);
    uint8_t i = 0;

    for(i = 0; i < name_len; i++)
    {
        at_cmd[10+i] = localname[i];
    }
    at_cmd[10+name_len] = '\r';

    BESUI_TRACE(0,"[UIHFP]%s,AT send = ", __func__);
    DUMP8("%02x ", at_cmd, sizeof(at_cmd));
    btif_hf_send_at_cmd((btif_hf_channel_t*)app_bt_get_device(device_id)->hf_channel, at_cmd);
}
void app_at_cmd_at_battery_handle(uint8_t device_id)
{
    BESUI_TRACE(0,"[UIHFP]%s", __func__);
    uint8_t batbuf = 0;
    char at_cmd[15] = {'\0'};
    sprintf(at_cmd, "%s", "AT+BAT=");

    uint8_t ear_battery_level = 10;
#ifdef BESUI_TWS_EN
    if(besui_get_lr_sta() == LEFT_SIDE)
    {
        at_cmd[7] = '0';
    }
    else if(besui_get_lr_sta() == RIGHT_SIDE)
    {
        at_cmd[7] = '1';
    }
#endif
    at_cmd[8] = ',';

    ear_battery_level = app_battery_current_level();
    BESUI_TRACE(1,"[UIHFP]%s EAR level %d", __func__, ear_battery_level);
    batbuf = ((ear_battery_level+1)*10);
    at_cmd[9] = (batbuf/100) + '0';
    at_cmd[10] = (batbuf%100/10) + '0';
    at_cmd[11] = (batbuf%100%10) + '0';
    at_cmd[12] = '\r';

    BESUI_TRACE(0,"[UIHFP]%s,AT send = ", __func__);
    DUMP8("%02x ", at_cmd, sizeof(at_cmd));
    btif_hf_send_at_cmd((btif_hf_channel_t*)app_bt_get_device(device_id)->hf_channel, at_cmd);
}

void app_at_cmd_at_surround_on_handle(uint8_t device_id)
{
    BESUI_TRACE(0,"[UIHFP]%s", __func__);
    char at_cmd[7] = {'\0'};
    sprintf(at_cmd, "%s\r", ATOK);

    BESUI_TRACE(0,"[UIHFP]%s,AT send = ", __func__);
    DUMP8("%02x ", at_cmd, sizeof(at_cmd));
    btif_hf_send_at_cmd((btif_hf_channel_t*)app_bt_get_device(device_id)->hf_channel, at_cmd);

#if defined(BESSPA_ONOFF_EN)
    algo_send_request(ALGO_ID_BESSPA, ALGO_ON, 0, 0, 0, 0);
#elif defined(DOLBY_AUDIO_ENABLE)
    algo_send_request(ALGO_ID_DOLBY, ALGO_ON, uictl.dolby_mode, 0, 0, 0);
#endif
}
void app_at_cmd_at_surround_off_handle(uint8_t device_id)
{
    BESUI_TRACE(0,"[UIHFP]%s", __func__);
    char at_cmd[7] = {'\0'};
    sprintf(at_cmd, "%s\r", ATOK);

    BESUI_TRACE(0,"[UIHFP]%s,AT send = ", __func__);
    DUMP8("%02x ", at_cmd, sizeof(at_cmd));
    btif_hf_send_at_cmd((btif_hf_channel_t*)app_bt_get_device(device_id)->hf_channel, at_cmd);

#if defined(BESSPA_ONOFF_EN)
    algo_send_request(ALGO_ID_BESSPA, ALGO_OFF, 0, 0, 0, 0);
#elif defined(DOLBY_AUDIO_ENABLE)
    algo_send_request(ALGO_ID_DOLBY, ALGO_OFF, uictl.dolby_mode, 0, 0, 0); 
#endif
}

void app_at_cmd_at_inear_on_handle(uint8_t device_id)
{
    BESUI_TRACE(0,"[UIHFP]%s", __func__);
    char at_cmd[7] = {'\0'};
    sprintf(at_cmd, "%s\r", ATOK);

    BESUI_TRACE(0,"[UIHFP]%s,AT send = ", __func__);
    DUMP8("%02x ", at_cmd, sizeof(at_cmd));
    btif_hf_send_at_cmd((btif_hf_channel_t*)app_bt_get_device(device_id)->hf_channel, at_cmd);

#if defined(CAPSENSOR_ENABLE) && defined(CAPSENSOR_WEAR) && defined(BESUI_APP_EN)
#if defined(CHIP_BEST1306)
    capsensor_start();
#elif defined(CHIP_BEST1306P)
    capsensor_resume();
#elif defined(CHIP_BEST1501P)
    analog_capsensor_reopen();
#endif

    struct nvrecord_env_t *nvrecord_env;
    nv_record_env_get(&nvrecord_env);
    nvrecord_env->inear_left_onoff = INEAR_DETECT_ON_CMD; 
    nvrecord_env->inear_right_onoff = INEAR_DETECT_ON_CMD; 
    if(app_inear_get_left_status() == 1 )
        nvrecord_env->left_status = 1;
    else
        nvrecord_env->left_status = 0;

    if(app_inear_get_right_status() == 1 )
        nvrecord_env->right_status = 1;
    else
        nvrecord_env->right_status = 0;

    nv_record_env_set(nvrecord_env);
#endif
}
void app_at_cmd_at_inear_off_handle(uint8_t device_id)
{
    BESUI_TRACE(0,"[UIHFP]%s", __func__);
    char at_cmd[7] = {'\0'};
    sprintf(at_cmd, "%s\r", ATOK);

    BESUI_TRACE(0,"[UIHFP]%s,AT send = ", __func__);
    DUMP8("%02x ", at_cmd, sizeof(at_cmd));
    btif_hf_send_at_cmd((btif_hf_channel_t*)app_bt_get_device(device_id)->hf_channel, at_cmd);

#if defined(CAPSENSOR_ENABLE) && defined(BESUI_APP_EN)
#if defined(CAPSENSOR_ENABLE)    
#if defined(CHIP_BEST1306) || defined(CHIP_BEST1306P)
    capsensor_suspend();
#elif defined(CHIP_BEST1501P)
    analog_capsensor_close();
#endif
#endif

    struct nvrecord_env_t *nvrecord_env;
    nv_record_env_get(&nvrecord_env);
    nvrecord_env->inear_left_onoff = INEAR_DETECT_OFF_CMD; 
    nvrecord_env->inear_right_onoff = INEAR_DETECT_OFF_CMD; 
    nvrecord_env->left_status = 0;
    nvrecord_env->right_status = 0;
    nv_record_env_set(nvrecord_env);
#endif
}

void app_at_cmd_at_shutdown(uint8_t device_id) //shutdown
{
    BESUI_TRACE(0,"[UIHFP]%s", __func__);
    char at_cmd[7] = {'\0'};
    sprintf(at_cmd, "%s\r", ATOK);

    BESUI_TRACE(0,"[UIHFP]%s,AT send = ", __func__);
    DUMP8("%02x ", at_cmd, sizeof(at_cmd));
    btif_hf_send_at_cmd((btif_hf_channel_t*)app_bt_get_device(device_id)->hf_channel, at_cmd);
#ifdef BESUI_TWS_EN
    uictl.poweroff_fast_flag = true;
#endif
    uictl.shutdown_type = SHUTDOWN_HFP_CMD;
    app_shutdown();
}

#ifdef BESUI_CAPSENSOR_FACTORY_EN
int at_dat[11] = {0};
void str_2_int(char *cmd_data)
{
    uint8_t i = 0;
    uint8_t j = 0;
    uint8_t dou_cnt[12] = {0};
    uint8_t str_len = 0;
    uint8_t dat_len = 0;
    uint8_t dats = 0;

    for(i = 0;i < 100;i ++)
    {
        if(cmd_data[i] == '\0')
        {
            str_len = i;
            break;
        }
    }

    BESUI_TRACE(0,"[UIHFP]%s, str_len = %d", __func__, str_len);
    dou_cnt[0] = 14;
    for(i = dou_cnt[0];i < str_len;i ++)
    {
        if(cmd_data[+i] == ',')
        {
            dou_cnt[j+1] = i;
            j ++;
            if(j == 10)
            {
                dou_cnt[11] = str_len;
                break; 
            }
        }
    }
    for(uint8_t i = 0;i < 12;i ++)
    {
        BESUI_TRACE(0,"[UIHFP]%s, dou_cnt[%d] = %d", __func__, i, dou_cnt[i]);
    }
    for(i = 0;i < 11;i ++)
    {
        dat_len = dou_cnt[i+1] - dou_cnt[i] - 1;
        dats = dou_cnt[i] + 1;
        BESUI_TRACE(0,"[UIHFP]%s, dat_len = %d", __func__, dat_len);
        if(dat_len == 0)
            at_dat[i] = 0;
        else if(dat_len == 1)
            at_dat[i] = cmd_data[dats] - '0';
        else if(dat_len == 2)
            at_dat[i] = (cmd_data[dats] - '0')*10 + (cmd_data[dats+1] - '0'); 
        else if(dat_len == 3)
            at_dat[i] = (cmd_data[dats] - '0')*100 + (cmd_data[dats+1] - '0')*10 + (cmd_data[dats+2] - '0'); 
        else if(dat_len == 4)
            at_dat[i] = (cmd_data[dats] - '0')*1000 + (cmd_data[dats+1] - '0')*100 + (cmd_data[dats+2] - '0')*10 + (cmd_data[dats+3] - '0'); 
        else if(dat_len == 5)
            at_dat[i] = (cmd_data[dats] - '0')*10000 + (cmd_data[dats+1] - '0')*1000 + (cmd_data[dats+2] - '0')*100 + (cmd_data[dats+3] - '0')*10 + (cmd_data[dats+4] - '0'); 
        else if(dat_len == 6)
            at_dat[i] = (cmd_data[dats] - '0')*100000 + (cmd_data[dats+1] - '0')*10000 + (cmd_data[dats+2] - '0')*1000 + (cmd_data[dats+3] - '0')*100 + (cmd_data[dats+4] - '0')*10 + (cmd_data[dats+5] - '0'); 
    }
    for(i = 0;i < 11;i ++)
    {
        BESUI_TRACE(0,"[UIHFP]%s, at_dat[%d] = %d", __func__, i, at_dat[i]);
    }
}

void app_at_cmd_at_cap_restart_handle(uint8_t device_id, char *cmd_data)
{
    BESUI_TRACE(0,"[UIHFP]%s, %s", __func__, cmd_data);
    char at_cmd[7] = {'\0'};
    sprintf(at_cmd, "%s\r", ATOK);

    BESUI_TRACE(0,"[UIHFP]%s,AT send = ", __func__);
    DUMP8("%02x ", at_cmd, sizeof(at_cmd));
    btif_hf_send_at_cmd((btif_hf_channel_t*)app_bt_get_device(device_id)->hf_channel, at_cmd);

    //wear range = 8 data = 32byte
    //touch range = 3 data = 12byte
    //AT+CAP_RESTART=
#ifdef CAPSENSOR_ENABLE
    str_2_int(cmd_data);
#if defined(CAPSENSOR_WEAR)
    capsensor_set_wear_cal_range_buf(at_dat);
    capsensor_set_touch_cal_range_buf((int *)(at_dat+8));
#endif
    capsensor_restart(false);
#endif
}
void app_at_cmd_at_cap_offset_handle(uint8_t device_id)
{
    BESUI_TRACE(0,"[UIHFP]%s", __func__);
    char at_cmd[7] = {'\0'};
    sprintf(at_cmd, "%s\r", ATOK);
 
    BESUI_TRACE(0,"[UIHFP]%s,AT send = ", __func__);
    DUMP8("%02x ", at_cmd, sizeof(at_cmd));
    btif_hf_send_at_cmd((btif_hf_channel_t*)app_bt_get_device(device_id)->hf_channel, at_cmd);
#ifdef CAPSENSOR_ENABLE
    capsensor_factory_calculate_flag_set(1, 0); 
#endif
}
uint8_t hfp_device_id = 0;
void app_at_cmd_at_cap_diff_handle(uint8_t device_id)
{
    BESUI_TRACE(0,"[UIHFP]%s", __func__);
    // char at_cmd[10] = {'\0'};
    // sprintf(at_cmd, "%s\r", ATOK);
    // BESUI_TRACE(0,"[UIHFP]%s,AT send = ", __func__);
    // DUMP8("%02x ", at_cmd, sizeof(at_cmd));
    // btif_hf_send_at_cmd((btif_hf_channel_t*)app_bt_get_device(device_id)->hf_channel, at_cmd);
    hfp_device_id = device_id;
#ifdef CAPSENSOR_ENABLE
    capsensor_factory_calculate_flag_set(0, 1);
#endif
}

void app_at_cmd_at_cap_diff_return(uint8_t len, uint8_t *cmd_data) //diff return
{
    BESUI_TRACE(0,"[UIHFP]%s, %s", __func__, cmd_data);
    char at_cmd[100] = {0};
    sprintf(at_cmd, "%s", "AT+CAP_DIFF=");
    for(uint8_t i = 0;i < len;i ++)
    {
        at_cmd[12+i] = cmd_data[i];
    }
    at_cmd[12+len] = '\r';

    BESUI_TRACE(0,"[UIHFP]%s, AT send = ", __func__);
    DUMP8("%02x ", at_cmd, sizeof(at_cmd));
    btif_hf_send_at_cmd((btif_hf_channel_t*)app_bt_get_device(hfp_device_id)->hf_channel, at_cmd);
}
#endif //#ifdef BESUI_CAPSENSOR_FACTORY_EN

void app_at_cmd_at_normal_mode(uint8_t device_id)
{
    BESUI_TRACE(0,"[UIHFP]%s", __func__);
    char at_cmd[7] = {'\0'};
    sprintf(at_cmd, "%s\r", ATOK);
 
    BESUI_TRACE(0,"[UIHFP]%s,AT send = ", __func__);
    DUMP8("%02x ", at_cmd, sizeof(at_cmd));
    btif_hf_send_at_cmd((btif_hf_channel_t*)app_bt_get_device(device_id)->hf_channel, at_cmd);

#ifdef BESSPA_ONOFF_EN
    if(besspa_audio_onoff_get())
        algo_send_request(ALGO_ID_BESSPA, ALGO_OFF, 0, 0, 0, 0);
#endif
#if defined(DOLBY_AUDIO_ENABLE)
    if(dolby_audio_onoff_get())
        algo_send_request(ALGO_ID_DOLBY, ALGO_OFF, uictl.dolby_mode, 0, 0, 0);
#endif
}

void app_at_cmd_at_reset(uint8_t device_id)
{
    BESUI_TRACE(0,"[UIHFP]%s", __func__);
    char at_cmd[7] = {'\0'};
    sprintf(at_cmd, "%s\r", ATOK);
 
    BESUI_TRACE(0,"[UIHFP]%s,AT send = ", __func__);
    DUMP8("%02x ", at_cmd, sizeof(at_cmd));
    btif_hf_send_at_cmd((btif_hf_channel_t*)app_bt_get_device(device_id)->hf_channel, at_cmd);

    uictl.shutdown_type = SHUTDOWN_HFP_RST;
    uictl.poweroff_fast_flag = true;
    app_reset();
}


void app_at_cmd_at_anc_adapt_on(uint8_t device_id)
{
    BESUI_TRACE(0,"[UIHFP]%s", __func__);
    char at_cmd[7] = {'\0'};
    sprintf(at_cmd, "%s\r", ATOK);
 
    BESUI_TRACE(0,"[UIHFP]%s,AT send = ", __func__);
    DUMP8("%02x ", at_cmd, sizeof(at_cmd));
    btif_hf_send_at_cmd((btif_hf_channel_t*)app_bt_get_device(device_id)->hf_channel, at_cmd);

    algo_send_request(ALGO_ID_ANC, 1, 3, 1, 0, 1);
}

void app_at_cmd_at_anc_adapt_off(uint8_t device_id)
{
    BESUI_TRACE(0,"[UIHFP]%s", __func__);
    char at_cmd[7] = {'\0'};
    sprintf(at_cmd, "%s\r", ATOK);
 
    BESUI_TRACE(0,"[UIHFP]%s,AT send = ", __func__);
    DUMP8("%02x ", at_cmd, sizeof(at_cmd));
    btif_hf_send_at_cmd((btif_hf_channel_t*)app_bt_get_device(device_id)->hf_channel, at_cmd);

    algo_send_request(ALGO_ID_ANC, 5, 0, 1, 0, 1);
}

int app_at_msg_handle_process(uint8_t device_id, char *cmd_data)
{
    uint8_t cmd_event = 0xFF;

    BESUI_TRACE(2,"[UIHFP]%s at: %s", __func__, cmd_data);

    cmd_event = app_at_compare_cmd_handle(cmd_data);

    BESUI_TRACE(1,"[UIHFP]%s cmd_event %d", __func__, cmd_event);

    switch(cmd_event)
    {
        case AT_TALK_MIC:
            app_at_cmd_at_talk_mic_handle(device_id);
            break;
        
        case AT_FF_MIC:
            app_at_cmd_at_ff_mic_handle(device_id);
            break;

        case AT_FB_MIC:
            //three mic 
            app_at_cmd_at_fb_mic_handle(device_id);
            break;

        case AT_ENC_MIC:
            app_at_cmd_at_enc_mic_handle(device_id);
            break;

        case AT_SHIP_MODE:
            app_at_cmd_at_ship_mode_handle(device_id);
            break;

        case AT_GET_SWVER:
            app_at_cmd_at_get_software_version_handle(device_id);
            break;

        case AT_ANC_ON:
            app_at_cmd_at_anc_on_handle(device_id);
            break;

        case AT_ANC_AA:
            app_at_cmd_at_anc_aa_handle(device_id);
            break;

        case AT_ANC_OFF:
            app_at_cmd_at_anc_off_handle(device_id);
            break;
//------------------------------------------------------------------------------------------
        case AT_ENC_ON:
            app_at_cmd_at_enc_on_handle(device_id);
            break;
        case AT_ENC_OFF:
            app_at_cmd_at_enc_off_handle(device_id);
            break;
        case AT_BTADDR:
            app_at_cmd_at_btaddr_handle(device_id);
            break;
        case AT_BTNAME:
            app_at_cmd_at_btname_handle(device_id);
            break;
        case AT_BATTERY:
            app_at_cmd_at_battery_handle(device_id);
            break;

        case AT_SURROUND_ON:
            app_at_cmd_at_surround_on_handle(device_id);
            break;
        case AT_SURROUND_OFF:
            app_at_cmd_at_surround_off_handle(device_id);
            break;
        case AT_INEAR_ON:
            app_at_cmd_at_inear_on_handle(device_id);
            break;
        case AT_INEAR_OFF:
            app_at_cmd_at_inear_off_handle(device_id);
            break;
        case AT_POWER_OFF:
            app_at_cmd_at_shutdown(device_id);
            break;

#ifdef BESUI_CAPSENSOR_FACTORY_EN
        case AT_CAP_RESTART:
            app_at_cmd_at_cap_restart_handle(device_id, cmd_data);
            break;
        case AT_CAP_OFFSET:
            app_at_cmd_at_cap_offset_handle(device_id);
            break;
        case AT_CAP_DIFF:
            app_at_cmd_at_cap_diff_handle(device_id);
            break;
#endif
        case AT_NORMAL_MODE:
            app_at_cmd_at_normal_mode(device_id);
            break;
        case AT_RESET:
            app_at_cmd_at_reset(device_id);
            break;
        case AT_ADAPT_ANC_ON:
            app_at_cmd_at_anc_adapt_on(device_id);
            break;
        case AT_ADAPT_ANC_OFF:
            app_at_cmd_at_anc_adapt_off(device_id);
            break;
//------------------------------------------------------------------------------------------
    }

    return 0;
}


#endif //#if defined(BESUI_TWS_EN) || defined(BESUI_STEREO_EN)

