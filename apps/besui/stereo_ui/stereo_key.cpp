#ifdef BESUI_STEREO_EN
#include <stdio.h>
#include "cmsis_os.h"
#include "hal_uart.h"
#include "stereo_key.h"
#include "hal_timer.h"
#include "hal_bootmode.h"
#include "hal_norflash.h"
#include "audioflinger.h"
#include "hal_trace.h"
#include "hal_cmu.h"
#include "analog.h"
#include "app_bt_stream.h"
#include "app_bt_func.h"
#include "pmu.h"
#include "cmsis.h"

#include "osif.h"
#include "apps.h"

#include "stereo_led.h"
#include "stereo_prompt.h"

#include "btapp.h"
#include "app_bt.h"
#include "earbud_profiles_api.h"

#include "nvrecord_extension.h"
#include "app_tota_general.h"
#include "nvrecord_env.h"
#include "app_tota.h"

#include "app_ibrt_keyboard.h"

#include "app_ibrt_conn_evt.h"
#include "app_ui_evt.h"
#include "app_ui_param_config.h"
#include "app_ui_api.h"
#include "app_audio_active_device_manager.h"
#include "ota_spp.h"
#include "bes_gap_api.h"
#include "audio_policy.h"
#include "bts_tws_if.h"
#include "bts_core_if.h"
#include "bts_bt_conn.h"

#if defined(ANC_APP)
#include "app_anc.h"
#endif

#if defined(DOLBY_AUDIO_ENABLE)
#include "watchdog.h"
#include "example.h"
#endif

#if defined(BESUI_COMM_EN)
#include "besui_common.h"
#endif

#include "stereoui.h"

#define A2DP_AUDIO_CODEC_TYPE_SBC           (1u<<0)
#define A2DP_AUDIO_CODEC_TYPE_MPEG2_4_AAC   (1u<<1)

static BUTTON_SET_EVENT_INFO_T keymap_l_tab[4];
// static BUTTON_SET_EVENT_INFO_T keymap_r_tab[4];

extern int open_siri_flag;

#ifdef BESSPA_ONOFF_EN
extern "C" int32_t stereo_surround_status;
#endif
extern APP_KEY_STATUS bt_key;
extern "C" void app_bt_key(APP_KEY_STATUS *status, void *param);

extern "C" int app_hfp_siri_voice(bool en);
extern "C" int app_play_audio_onoff(bool onoff, APP_AUDIO_STATUS* status);
extern void app_audio_dynamic_update_dest_packet_mtu_set(uint8_t codec_index, uint8_t packet_mtu, uint8_t user_configure);
extern bt_status_t LinkDisconnectDirectly(bool PowerOffFlag);


const  APP_KEY_HANDLE  app_key_poweron_handle_cfg[] = {
    {{POWER_KEY,APP_KEY_EVENT_CLICK},               "app ui click key",             app_bt_key, NULL},
    {{POWER_KEY,APP_KEY_EVENT_DOUBLECLICK},         "reject call incoming",         app_bt_key, NULL},
    {{POWER_KEY,APP_KEY_EVENT_TRIPLECLICK},         "app activate voice assistant", app_bt_key, NULL},
#ifdef DUT_TEST_EN
    {{POWER_KEY,APP_KEY_EVENT_RAMPAGECLICK},        "stereo enter dut",             app_bt_key, NULL},
#endif
    {{POWER_KEY,APP_KEY_EVENT_LONGPRESS},           "app game mode or music mode",  app_bt_key, NULL}, 
    {{POWER_KEY,APP_KEY_EVENT_LONGLONGPRESS},       "app power off",                app_bt_key, NULL},
    {{POWER_KEY,APP_KEY_EVENT_VERYLONGPRESS},       "app_ibrt_ui_test_key",         app_bt_key, NULL},
	
    {{VOLADD_KEY, APP_KEY_EVENT_CLICK},             "bt volume up key",             app_bt_key, NULL},
    {{VOLADD_KEY, APP_KEY_EVENT_DOUBLECLICK},       "bt volume up key",             app_bt_key, NULL},
    {{VOLADD_KEY, APP_KEY_EVENT_TRIPLECLICK},       "bt volume up key",             app_bt_key, NULL},
    {{VOLADD_KEY, APP_KEY_EVENT_ULTRACLICK},        "bt volume up key",             app_bt_key, NULL},
    {{VOLADD_KEY, APP_KEY_EVENT_LONGPRESS},         "bt play forward key",          app_bt_key, NULL},

    {{VOLSUB_KEY, APP_KEY_EVENT_CLICK},             "bt volume down key",           app_bt_key, NULL},
    {{VOLSUB_KEY, APP_KEY_EVENT_DOUBLECLICK},       "bt volume down key",           app_bt_key, NULL},
    {{VOLSUB_KEY, APP_KEY_EVENT_TRIPLECLICK},       "bt volume down key",           app_bt_key, NULL},
    {{VOLSUB_KEY, APP_KEY_EVENT_ULTRACLICK},        "bt volume down key",           app_bt_key, NULL},
    {{VOLSUB_KEY, APP_KEY_EVENT_LONGPRESS},         "bt play backward key",         app_bt_key, NULL},

    {{ANC_KEY,APP_KEY_EVENT_DOUBLECLICK},           "app anc eq switch",            app_bt_key, NULL},
    {{ANC_KEY,APP_KEY_EVENT_CLICK},                 "app anc click key",            app_bt_key, NULL},

    //{{VOLSUB_KEY | ANC_KEY ,APP_KEY_EVENT_LONGLONGPRESS},"clear paried device list", app_bt_key, NULL},
    //{{VOLADD_KEY | VOLSUB_KEY ,APP_KEY_EVENT_LONGLONGPRESS},"clear paried device list", app_bt_key, NULL},
    // {{ANC_KEY | VOLADD_KEY ,APP_KEY_EVENT_LONGLONGPRESS},"Single line upgrade",      app_bt_key, NULL},    
    {{ANC_KEY | VOLSUB_KEY ,APP_KEY_EVENT_LONGLONGPRESS},"clear paried device list", app_bt_key, NULL},
};

void app_ui_key_poweron_init(void)
{
    BESUI_TRACE(0, "%s", __func__);
    app_key_handle_clear();

    for (uint8_t i = 0; i < (sizeof(app_key_poweron_handle_cfg) / sizeof(APP_KEY_HANDLE)); i++) {
        app_key_handle_registration(&app_key_poweron_handle_cfg[i]);
    }
    stereo_tota_button_init();
}

HFCALL_MACHINE_ENUM app_get_hfcall_machine(void);

static void app_ui_bt_key_handle_bt_func_click(APP_KEY_STATUS *status)
{
    HFCALL_MACHINE_ENUM hfcall_machine = app_get_hfcall_machine();
    BESUI_TRACE(1,"[UIKEY]%s, %d", __func__, hfcall_machine);
    POSSIBLY_UNUSED struct BT_DEVICE_T* a2dp_device = app_bt_get_device(app_bt_audio_get_curr_a2dp_device());

    switch(hfcall_machine)
    {
        case HFCALL_MACHINE_CURRENT_IDLE:
            stereo_tota_button_event_handler(&bt_key, false);
        break;

        case HFCALL_MACHINE_CURRENT_IDLE_ANOTHER_IDLE:
            stereo_tota_button_event_handler(&bt_key, false);
        break;

#ifdef BT_HFP_SUPPORT
        case HFCALL_MACHINE_CURRENT_INCOMMING:
           hfp_handle_key(HFP_KEY_ANSWER_CALL);
        break;
        case HFCALL_MACHINE_CURRENT_OUTGOING:
            //app_voice_report(APP_STATUS_INDICATION_END_CALL, 0);
            hfp_handle_key(HFP_KEY_HANGUP_CALL);
        break;
        case HFCALL_MACHINE_CURRENT_CALLING:
            //app_voice_report(APP_STATUS_INDICATION_END_CALL, 0);
            hfp_handle_key(HFP_KEY_HANGUP_CALL);
        break;
        case HFCALL_MACHINE_CURRENT_3WAY_INCOMMING:
            // hfp_handle_key(HFP_KEY_THREEWAY_HANGUP_AND_ANSWER);
        break;
        case HFCALL_MACHINE_CURRENT_3WAY_HOLD_CALLING:
            // hfp_handle_key(HFP_KEY_THREEWAY_HOLD_AND_ANSWER);
        break;
#endif /* BT_HFP_SUPPORT */

        case HFCALL_MACHINE_CURRENT_INCOMMING_ANOTHER_IDLE:
            hfp_handle_key(HFP_KEY_ANSWER_CALL);
        break;
        case HFCALL_MACHINE_CURRENT_OUTGOING_ANOTHER_IDLE:
            //app_voice_report(APP_STATUS_INDICATION_END_CALL, 0);
            hfp_handle_key(HFP_KEY_HANGUP_CALL);
        break;
        case HFCALL_MACHINE_CURRENT_CALLING_ANOTHER_IDLE:
            //app_voice_report(APP_STATUS_INDICATION_END_CALL, 0);
            hfp_handle_key(HFP_KEY_HANGUP_CALL);
        break;
        case HFCALL_MACHINE_CURRENT_3WAY_INCOMMING_ANOTHER_IDLE:
            // hfp_handle_key(HFP_KEY_THREEWAY_HANGUP_AND_ANSWER);
        break;
        case HFCALL_MACHINE_CURRENT_3WAY_HOLD_CALLING_ANOTHER_IDLE:
            // hfp_handle_key(HFP_KEY_THREEWAY_HOLD_AND_ANSWER);
        break;
        case HFCALL_MACHINE_CURRENT_INCOMMING_ANOTHER_INCOMMING:
            //hfp_handle_key(HFP_KEY_THREEWAY_HANGUP_AND_ANSWER);
            hfp_handle_key(HFP_KEY_DUAL_HF_HOLD_CURR_ANSWER_ANOTHER);
        break;
        case HFCALL_MACHINE_CURRENT_INCOMMING_ANOTHER_OUTGOING:
            hfp_handle_key(HFP_KEY_DUAL_HF_CHANGETOPHONE_ANSWER_CURR);
        break;
        case HFCALL_MACHINE_CURRENT_CALLING_ANOTHER_CHANGETOPHONE:
            // hfp_handle_key(HFP_KEY_DUAL_HF_HANGUP_ANOTHER_ADDTOEARPHONE);
            //app_voice_report(APP_STATUS_INDICATION_END_CALL, 0);
            hfp_handle_key(HFP_KEY_HANGUP_CALL);
        break;
        case HFCALL_MACHINE_CURRENT_CALLING_ANOTHER_HOLD:
            // hfp_handle_key(HFP_KEY_DUAL_HF_HANGUP_CURR_ANSWER_ANOTHER);
            //app_voice_report(APP_STATUS_INDICATION_END_CALL, 0);
            hfp_handle_key(HFP_KEY_HANGUP_CALL);
        break;
        case HFCALL_MACHINE_CURRENT_INCOMMING_ANOTHER_CALLING:
            hfp_handle_key(HFP_KEY_DUAL_HF_CHANGETOPHONE_ANSWER_CURR);
        break;
#ifdef USER_QUICK_SWITCH_EN
        case HFCALL_MACHINE_CURRENT_INCOMING_ANOTHER_PC_STREAMING:
        case HFCALL_MACHINE_CURRENT_CALLING_ANOTHER_PC_STREAMING:
            stereo_tota_button_event_handler(&bt_key, false);
        break;
#endif
        default:
        break;
    }
}

static void app_ui_bt_key_handle_bt_func_doubleclick(APP_KEY_STATUS *status)
{
    HFCALL_MACHINE_ENUM hfcall_machine = app_get_hfcall_machine();
    switch(hfcall_machine){
#ifdef BT_HFP_SUPPORT
        case HFCALL_MACHINE_CURRENT_INCOMMING_ANOTHER_IDLE:
            hfp_handle_key(HFP_KEY_HANGUP_CALL);
            break;
#endif /* BT_HFP_SUPPORT */
        default:
            stereo_tota_button_event_handler(&bt_key, false);
        break;
    }
}

static int stereo_app_hfp_siri_voice(bool en)
{
    HFCALL_MACHINE_ENUM hfcall_machine = app_get_hfcall_machine();
    BESUI_TRACE(0, "[UISIRI][%s], hfcall_machine = %d", __func__, hfcall_machine);

    static int device_id = BT_DEVICE_INVALID_ID;
    struct BT_DEVICE_T* curr_device = NULL;
    device_id = app_audio_adm_get_bt_active_device();

    if(device_id == BT_DEVICE_INVALID_ID)
    {
        BESUI_TRACE(0, "active device is null");
        return -1;
    }

    curr_device = app_bt_get_device(device_id);

    BESUI_TRACE(0, "[UISIRI][%s], en = %d", __func__, en);
    if(en == false)
    {
        if(btif_hf_is_voice_rec_active(curr_device->hf_channel) == false)
        {
            en = true;
            BESUI_TRACE(0, "[UISIRI][%s]siri audo end, need open", __func__);
        }
    }

    if((btif_get_hf_chan_state(curr_device->hf_channel) == BT_HFP_CHAN_STATE_OPEN)) 
    {
        btif_hf_enable_voice_recognition(curr_device->hf_channel, en);
        BESUI_TRACE(0, "[UISIRI]siri send %d", en);
    } 

    BESUI_TRACE(0, "[UISIRI][%s]id =%d/%d/%d", __func__, device_id, open_siri_flag, en);

    return 0;
}

static void stereo_siri_onoff(void)
{
    if(!open_siri_flag){
        BESUI_TRACE(0,"[UISIRI]open siri");
        stereo_app_hfp_siri_voice(true);
        open_siri_flag = 1;
    }else{
        BESUI_TRACE(0,"[UISIRI]close siri");
        stereo_app_hfp_siri_voice(false);
        open_siri_flag = 0;
    }
}

static void app_ui_bt_key_handle_bt_func_tripleclick(void)
{
    stereo_tota_button_event_handler(&bt_key, false);
}

void stereo_game_mode_onoff(bool onoff, bool prompt)
{
    BESUI_TRACE(0, "[UISPA]%s", __func__);

    if (onoff)
    {
        app_audio_dynamic_update_dest_packet_mtu_set(A2DP_AUDIO_CODEC_TYPE_SBC, 15, true);//sbc -10
        app_audio_dynamic_update_dest_packet_mtu_set(A2DP_AUDIO_CODEC_TYPE_MPEG2_4_AAC, 3, true);//aac -2
        if(prompt)
            app_voice_report(APP_STATUS_INDICATION_GAME_MODE, 0);
        BESUI_TRACE(0, "game_mode OPEN");
    }
    else
    {
        app_audio_dynamic_update_dest_packet_mtu_set(A2DP_AUDIO_CODEC_TYPE_SBC, 35, false);//sbc -25s
        app_audio_dynamic_update_dest_packet_mtu_set(A2DP_AUDIO_CODEC_TYPE_MPEG2_4_AAC, 3, false);//aa -4
        if(prompt)
            app_voice_report(APP_STATUS_INDICATION_MUSIC_MODE, 0);
        BESUI_TRACE(0, "game_mode CLOSE");
    }
}
void stereo_game_mode_switch(void)
{
    if(app_bt_audio_count_connected_sco())
        return;

    nv_record_env_get(&nvrecord_uienv);
    BESUI_TRACE(0, "[UISPA]%s", __func__);
    if (besui_get_profile_conn_num())
    {
        nvrecord_uienv->game_mode_onoff = !nvrecord_uienv->game_mode_onoff;
        stereo_game_mode_onoff(nvrecord_uienv->game_mode_onoff, true);
        nv_record_env_set(nvrecord_uienv);
    }
}

void app_ui_bt_key_handle_bt_func_longpress(void)
{
    stereo_tota_button_event_handler(&bt_key, false);
}

static void app_ui_bt_key_handle_func_key(APP_KEY_STATUS *status)
{
    BESUI_TRACE(1,"%s",__func__);
    switch(status->event)
    {
        case  APP_KEY_EVENT_CLICK:
            BESUI_TRACE(0,"APP_KEY_EVENT_CLICK press");
            app_ui_bt_key_handle_bt_func_click(status);
            break;

        case  APP_KEY_EVENT_DOUBLECLICK:
            BESUI_TRACE(0,"APP_KEY_EVENT_DOUBLECLICK press");
            app_ui_bt_key_handle_bt_func_doubleclick(status);
            break;

        case  APP_KEY_EVENT_TRIPLECLICK:
            BESUI_TRACE(0,"APP_KEY_EVENT_TRIPLECLICK press");
#ifdef SUPPORT_SIRI
            app_ui_bt_key_handle_bt_func_tripleclick();
#endif //SUPPORT_SIRI            
            break;
#ifdef DUT_TEST_EN
        case  APP_KEY_EVENT_RAMPAGECLICK:
            BESUI_TRACE(0,"APP_KEY_EVENT_RAMPAGECLICK press");
            hal_sw_bootmode_clear(HAL_SW_BOOTMODE_REBOOT);
            hal_sw_bootmode_set(HAL_SW_BOOTMODE_TEST_MODE|HAL_SW_BOOTMODE_TEST_SIGNALINGMODE);
            pmu_reboot();
            break;
#endif
        case  APP_KEY_EVENT_LONGPRESS:
            BESUI_TRACE(0,"APP_KEY_EVENT_LONGPRESS press");
            app_ui_bt_key_handle_bt_func_longpress();
            break;

        case  APP_KEY_EVENT_VERYLONGPRESS:
            BESUI_TRACE(0,"APP_KEY_EVENT_VERYLONGPRESS press");
            break;
        case  APP_KEY_EVENT_LONGLONGPRESS:
            BESUI_TRACE(0,"long long press");
            app_ui_state_machine(APP_UI_POWER_OFF,1,0);
            break;
        default:
            BESUI_TRACE(1,"unregister down key event=%x",status->event);
            break;
    }
}

void app_ui_bt_key_handle_down_key(enum APP_KEY_EVENT_T event)
{
    BESUI_TRACE(1,"%s",__func__);
    switch(event)
    {
        case  APP_KEY_EVENT_CLICK:
        case  APP_KEY_EVENT_DOUBLECLICK:
        case  APP_KEY_EVENT_TRIPLECLICK:
        case  APP_KEY_EVENT_ULTRACLICK:
            app_ibrt_if_set_local_volume_down();
            break;
#ifdef BT_AVRCP_SUPPORT
        case  APP_KEY_EVENT_UP:
        case  APP_KEY_EVENT_LONGPRESS:
            a2dp_handleKey(AVRCP_KEY_BACKWARD);
            break;
#endif
        default:
            BESUI_TRACE(1,"unregister down key event=%x",event);
            break;
    }
}

void app_ui_open_pairing(void)
{
    BESUI_TRACE(2,"%s besui_get_profile_conn_num is %d ", __func__, besui_get_profile_conn_num());
    app_ui_enter_pairing_mode(IBRT_UI_DISABLE_BT_SCAN_TIMEOUT, true);
    if (besui_get_profile_conn_num() >= 2)
    {
        BESUI_TRACE(0, "besui_get_profile_conn_num is 2!!!!!!");
    }
}

void app_ui_bt_key_handle_up_key(enum APP_KEY_EVENT_T event)
{
    BESUI_TRACE(1,"%s",__func__);
    switch(event)
    {
        case  APP_KEY_EVENT_CLICK:
        case  APP_KEY_EVENT_DOUBLECLICK:
        case  APP_KEY_EVENT_TRIPLECLICK:
        case  APP_KEY_EVENT_ULTRACLICK:
            app_ibrt_if_set_local_volume_up();
            break;
#ifdef BT_AVRCP_SUPPORT
        case  APP_KEY_EVENT_UP:
        case  APP_KEY_EVENT_LONGPRESS:
            a2dp_handleKey(AVRCP_KEY_FORWARD);
            break;
#endif
        default:
            BESUI_TRACE(1,"unregister up key event=%x",event);
            break;
    }
}

extern "C" void app_ibrt_nvrecord_delete_all_mobile_record(void);
static int get_bt_nv_record(void)
{
    btif_device_record_t record1;
    btif_device_record_t record2;
    return nv_record_enum_latest_two_paired_dev(&record1, &record2);
}

static void app_remove_all_paired_list(void)
{
    bt_status_t            retStatus;
    btif_device_record_t   record;
    // ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    int                    paired_dev_count = nv_record_get_paired_dev_count();

    BESUI_TRACE(0,"[UIRST]%s", __func__);
    // BESUI_TRACE(0,"[UICOM]Master addr:");
    // DUMP8("%02x ",p_ibrt_ctrl->local_addr.address, BTIF_BD_ADDR_SIZE);
    // BESUI_TRACE(0,"[UICOM]Slave addr:");
    // DUMP8("%02x ",p_ibrt_ctrl->peer_addr.address, BTIF_BD_ADDR_SIZE);

    ota_disconnect();
    bes_ble_gap_disconnect_all();

    LinkDisconnectDirectly(true);
    //osDelay(200);

    for (int32_t index = paired_dev_count - 1; index >= 0; index--)
    {
        retStatus = nv_record_enum_dev_records(index, &record);
        if (BT_STS_SUCCESS == retStatus)
        {
            BESUI_TRACE(1,"[UICOM]The index %d of nv records:", index);
            DUMP8("%02x ", record.bdAddr.address, BTIF_BD_ADDR_SIZE);
            //if (memcmp(record.bdAddr.address, p_ibrt_ctrl->local_addr.address, BTIF_BD_ADDR_SIZE) &&
            //    memcmp(record.bdAddr.address, p_ibrt_ctrl->peer_addr.address, BTIF_BD_ADDR_SIZE)&&
            //    memcmp(record.bdAddr.address, address_compare, BTIF_BD_ADDR_SIZE))
            {
            	nv_record_ddbrec_delete(&record.bdAddr);
            }
        }
    }
    // app_common_clear_bt_pairlist_flag = true;
    // app_status_indication_set(APP_STATUS_INDICATION_CLEARPAIRLIST);
    // app_ibrt_if_config_keeper_clear();
    // memset(p_ibrt_ctrl->local_addr.address, 0, BTIF_BD_ADDR_SIZE);
    // memset(p_ibrt_ctrl->peer_addr.address, 0, BTIF_BD_ADDR_SIZE);

#if 1
    nv_record_env_get(&nvrecord_uienv);
    // nvrecord_uienv->ibrt_mode.mode = IBRT_UNKNOW,
    // memset(nvrecord_uienv->ibrt_mode.record.bdAddr.address, 0, BTIF_BD_ADDR_SIZE);
    // memset(nvrecord_uienv->tws_con_addr, 0, BTIF_BD_ADDR_SIZE);
    // memset(nvrecord_uienv->another_addr, 0, BTIF_BD_ADDR_SIZE);
    // nvrecord_uienv->need_twspair_flag = false;
#ifdef BESUI_APP_EN
    nvrecord_uienv->init_button = 0x00;
#endif

// #ifdef BESUI_GAME_NV_EN
//     nvrecord_uienv->remember_game_mode = 0;
// #endif
    nvrecord_uienv->game_mode_onoff = false;
#if defined(__VIRTUAL_SURROUND__) || defined(__VIRTUAL_SURROUND_STEREO__)
    nvrecord_uienv->space_audio_type = 0;  
#endif

    nv_record_env_set(nvrecord_uienv);
    // nv_record_flash_flush();
#endif

    nv_record_rebuild(NV_REBUILD_SDK_ONLY); //clear gfps dev name
}

extern "C" int system_reset(void);
void bt_key_handle_clear_device_key(enum APP_KEY_EVENT_T event)
{
    BESUI_TRACE(1,"[UIRST]%s, event = %d",__func__, event);
    switch(event)
    {
        case APP_KEY_EVENT_LONGLONGPRESS:
            if (get_bt_nv_record() > 0)
            {
                app_system_status_set(APP_STATUS_TYPE_CLEAR_DEVICE_LIST);
                BESUI_TRACE(1,"[UIKEY]get_bt_nv_record is %d", get_bt_nv_record());
                //app_ibrt_nvrecord_delete_all_mobile_record();
                app_remove_all_paired_list();
                osDelay(500);
                system_reset();
            }
            else
            {
                BESUI_TRACE(1,"[UIKEY]nv no record !");
            }
            break;
        default:
            BESUI_TRACE(1,"[UIKEY]unregister up key event=%x",event);
            break;
    }
}

void app_ui_single_line_upgrade(enum APP_KEY_EVENT_T event)
{
    BESUI_TRACE(1,"%s",__func__);
    switch(event)
    {
        case APP_KEY_EVENT_LONGLONGPRESS:
            // hal_sw_bootmode_clear(HAL_SW_BOOTMODE_REBOOT); //single wire mode
            // hal_sw_bootmode_set(HAL_SW_BOOTMODE_SINGLE_LINE_DOWNLOAD);
            // pmu_reboot();
            stereo_ota_boot_or_single_update(0, 1);
            break;
        default:
            BESUI_TRACE(1,"unregister up key event=%x",event);
            break;
    }
}

#ifdef USER_QUICK_SWITCH_EN
extern uint8_t app_bt_audio_select_another_call_setup_hfp(uint8_t curr_device_id);
static void quick_switch_process(void)
{
    BESUI_TRACE(0, "%s",__func__);
    uint8_t conn_devices = besui_get_profile_conn_num();
    BESUI_TRACE(0, "%s, conn_devices = %d",__func__, conn_devices);
    if(conn_devices < 2)
    {
        BESUI_TRACE(0, "%s, conn_devices less than 2",__func__);
        return;
    }
    uint8_t sco_current = app_bt_audio_get_curr_playing_sco(); //sco device_id
    uint8_t sco_another = BT_DEVICE_INVALID_ID;
    uint8_t sco_count = app_bt_audio_count_connected_sco();
    if(sco_count > 1)
    {
        if(sco_count < 2)
            sco_another = 1-sco_current;
    } 
    BESUI_TRACE(0, "%s, sco_count=%d, sco_current=%d, sco_another=%d",__func__, sco_count, sco_current, sco_another);

    uint8_t a2dp_current = app_bt_audio_get_curr_playing_a2dp(); //a2dp device_id
    uint8_t a2dp_another = BT_DEVICE_INVALID_ID;
    uint8_t a2dp_count = app_bt_audio_count_connected_a2dp();
    if(a2dp_count > 1)
    {
        if(a2dp_current < 2)
            a2dp_another = 1-a2dp_current;
        if(a2dp_current==BT_DEVICE_INVALID_ID && a2dp_another==BT_DEVICE_INVALID_ID)
        {
            a2dp_another = 1-sco_current;
        }
    }
    BESUI_TRACE(0, "%s, a2dp_count=%d, a2dp_current=%d, a2dp_another=%d",__func__, a2dp_count, a2dp_current, a2dp_another);

    app_bt_manager.hfp_key_handle_curr_id = app_bt_audio_get_hfp_device_for_user_action();
    btif_hf_channel_t* hf_channel_curr = app_bt_get_device(app_bt_manager.hfp_key_handle_curr_id)->hf_channel;

    if(sco_current != BT_DEVICE_INVALID_ID) //A-SCO
    {
        sco_another = app_bt_audio_select_another_call_setup_hfp(sco_current);
        BESUI_TRACE(0, "%s, A-SCO,call_setup_hfp,sco_another=%d",__func__, sco_another);
        if(sco_another == BT_DEVICE_INVALID_ID) //B-SCO
        {
            sco_another = app_bt_audio_select_another_call_active_hfp(sco_current);
            if(sco_another != BT_DEVICE_INVALID_ID) //B-SCO
            {
                btif_hf_disc_audio_link(hf_channel_curr);
                app_bt_HF_CreateAudioLink(app_bt_get_device(sco_another)->hf_channel); //BCC
                //app_bt_hf_create_sco_directly(sco_another); //0x0428
                app_bt_manager.hf_call_next_state = HFCALL_NEXT_STA_CURR_ACCEPT;
                BESUI_TRACE(0, "%s, A-SCO,B-SCO,app_bt_HF_CreateAudioLink1",__func__);
            }
            else
            {
                if(a2dp_another != BT_DEVICE_INVALID_ID) //B-A2DP
                {
                    btif_hf_disc_audio_link(hf_channel_curr);
                    app_ibrt_if_switch_streaming_a2dp();
                    BESUI_TRACE(0, "%s, A-SCO,B-A2DP,app_ibrt_if_switch_streaming_a2dp",__func__);
                }
            }
        }
        else //if(sco_another != BT_DEVICE_INVALID_ID) //B-SCO
        {
            btif_hf_disc_audio_link(hf_channel_curr);
            app_ibrt_if_switch_streaming_sco();
            BESUI_TRACE(0, "%s, A-A2DP,B-SCO,app_ibrt_if_switch_streaming_sco",__func__);
        }
    }
    else
    {
        if(a2dp_current != BT_DEVICE_INVALID_ID) //A-A2DP
        {
            sco_another = app_bt_audio_select_another_call_setup_hfp(a2dp_current);
            BESUI_TRACE(0, "%s, A-A2DP,call_setup_hfp,sco_another=%d",__func__, sco_another);
            if(sco_another == BT_DEVICE_INVALID_ID) //B-SCO
            {
                sco_another = app_bt_audio_select_another_call_active_hfp(a2dp_current);
                BESUI_TRACE(0, "%s, A-A2DP,B-SCO,call_active_hfp2,sco_another=%d",__func__, sco_another);
            }

            if(sco_another != BT_DEVICE_INVALID_ID) //B-SCO
            {
                app_bt_HF_CreateAudioLink(app_bt_get_device(sco_another)->hf_channel); //BCC
                //app_bt_hf_create_sco_directly(sco_another); //0x0428
                app_bt_manager.hf_call_next_state = HFCALL_NEXT_STA_CURR_ACCEPT;
                BESUI_TRACE(0, "%s, A-A2DP,B-SCO,app_bt_HF_CreateAudioLink2",__func__);
            }
            else if(a2dp_another != BT_DEVICE_INVALID_ID) //B-A2DP
            {
                app_ibrt_if_switch_streaming_a2dp();
                BESUI_TRACE(0, "%s, A-A2DP,B-A2DP,app_ibrt_if_switch_streaming_a2dp",__func__);
            }
        }
    }
}
#endif//#ifdef USER_QUICK_SWITCH_EN

void stereo_bes_spatial_onoff(void)
{
    if(app_bt_audio_count_connected_sco())
        return;

#ifdef BESSPA_ONOFF_EN
#if defined(ALGO_INFO_SYNC_EN)
    nv_record_env_get(&nvrecord_uienv);
    nvrecord_uienv->space_audio_type = !besspa_audio_onoff_get();
    if(nvrecord_uienv->space_audio_type)
    {
        algo_send_request(ALGO_ID_BESSPA, ALGO_ON, 0, 0, 0, 0);
    }
    else
    {
        algo_send_request(ALGO_ID_BESSPA, ALGO_OFF, 0, 0, 0, 0);
    }
    nv_record_env_set(nvrecord_uienv);
#endif
    BESUI_TRACE(0, "%s, spa_sta=%d", __func__, nvrecord_uienv->space_audio_type);
#endif
}

#if ANC_APP
static void app_ui_anc_key_handle_func_click(void)
{
    BESUI_TRACE(1,"%s",__func__);

    besui_anc_key_switch();
}
#else
static void app_ui_anc_key_handle_func_click(void)
{
    BESUI_TRACE(0, "%s",__func__);
    if(app_bt_audio_count_connected_sco())
        return;
#ifdef BESSPA_ONOFF_EN
    if (besui_get_profile_conn_num())
    {
        stereo_bes_spatial_onoff();
    }
#endif
}
#endif

static void app_ui_anc_key_handle_func_doubleclick(void)
{
    BESUI_TRACE(1,"%s",__func__);

    if(app_bt_audio_count_connected_sco())
        return;

#if defined(BESSPA_ONOFF_EN)
    if (besui_get_profile_conn_num()) //besui_get_profile_conn_num()
    {
        stereo_bes_spatial_onoff();
    }
#endif
#ifdef USER_QUICK_SWITCH_EN
    quick_switch_process();
#endif 
}

static void app_ui_anc_key_handle_key(enum APP_KEY_EVENT_T event)
{
    BESUI_TRACE(1,"%s",__func__);
    switch(event)
    {
        case  APP_KEY_EVENT_CLICK:
            app_ui_anc_key_handle_func_click();
            break;
        case  APP_KEY_EVENT_DOUBLECLICK:
            app_ui_anc_key_handle_func_doubleclick();
            break;
        case  APP_KEY_EVENT_LONGPRESS:
            break;

        default:
            BESUI_TRACE(1,"unregister down key event=%x",event);
            break;
    }
}

void stereo_tota_button_info_set(uint8_t* ptrParam, uint16_t paramLen)
{
    uint8_t resData[5];
    uint8_t resLen;
        BESUI_TRACE(1,"[UITOTA], button ptrParam = %d, %d, %d, %d", ptrParam[0], ptrParam[1], ptrParam[2], ptrParam[3]);
    if(BUTTON_SETTING_LEFT_EARPHONE_CMD == ptrParam[1])
    {
       if(BUTTON_SETTING_CLICK_CMD == ptrParam[2])
       {
           keymap_l_tab[0].status.code = POWER_KEY;//if you have button parameters,set them here
           keymap_l_tab[0].status.event = APP_KEY_EVENT_CLICK;//8,if you have button event parameters,set them here
           keymap_l_tab[0].lr = ptrParam[1];
           keymap_l_tab[0].button_set_event = ptrParam[3];
       }
       else if(BUTTON_SETTING_DOUBLE_CLICK_CMD == ptrParam[2])
       {
           keymap_l_tab[1].status.code = POWER_KEY;//if you have button parameters,set them here
           keymap_l_tab[1].status.event = APP_KEY_EVENT_DOUBLECLICK;//if you have button event parameters,set them here
           keymap_l_tab[1].lr = ptrParam[1];
           keymap_l_tab[1].button_set_event = ptrParam[3];
       }
       else if(BUTTON_SETTING_TRIPLE_CLICK_CMD == ptrParam[2])
       {
           keymap_l_tab[2].status.code = POWER_KEY;
           keymap_l_tab[2].status.event = APP_KEY_EVENT_TRIPLECLICK;
           keymap_l_tab[2].lr = ptrParam[1];
           keymap_l_tab[2].button_set_event = ptrParam[3];
       }
       else if(BUTTON_SETTING_LONG_PRESS_CMD == ptrParam[2])
       {
           keymap_l_tab[3].status.code = POWER_KEY;
           keymap_l_tab[3].status.event = HAL_KEY_EVENT_LONGPRESS;
           keymap_l_tab[3].lr = ptrParam[1];
           keymap_l_tab[3].button_set_event = ptrParam[3];
       }

        //write to flash
        BESUI_TRACE(1,"write flash, %d",  sizeof(keymap_l_tab));
        memcpy(nvrecord_uienv->buttonInfo_left, keymap_l_tab, sizeof(keymap_l_tab));
        //DUMP8("%0x2", &(nvrecord_uienv->buttonInfo_left),  sizeof(keymap_l_tab));
        nv_record_env_set(nvrecord_uienv);
        nv_record_flash_flush();
   }

    resData[0] = ptrParam[0];
    resData[1] = ptrParam[1];
    resData[2] = ptrParam[2];
    resData[3] = ptrParam[3];
    resData[4] = 1; //0-left  1-right
    resLen = 0x05;
    app_tota_send_rsp(OP_TOTA_SET_CUSTOMER_CMD, TOTA_NO_ERROR, resData, resLen);
}

void stereo_tota_button_init(void)
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
        memcpy(keymap_l_tab, nvrecord_uienv->buttonInfo_left, sizeof(keymap_l_tab));
        //memcpy(keymap_r_tab, nvrecord_uienv->buttonInfo_right, sizeof(keymap_r_tab));
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

#ifdef APP_MULTIPOINT_ONOFF_EN
    nvrecord_uienv->muiltipoint_onoff = 1;
#endif

#ifdef EQ_SET_CUSTOMER_EN
    nvrecord_uienv->eq_onoff = 2;
    nvrecord_uienv->eq_mode = 0;
#endif

#if defined(EQ_CUSTOM_APP_EN)
    for(uint8_t i = 0;i < EQBAND_NUM;i ++)
        nvrecord_uienv->eq_custom[i] = 5;
#endif

#if defined(APP_TOUCH_ONOFF_EN)
    nvrecord_uienv->touch_onoff = true;
#endif

    keymap_l_tab[0].lr = BUTTON_SETTING_LEFT_EARPHONE_CMD;
    keymap_l_tab[0].status.code = POWER_KEY;
    keymap_l_tab[0].status.event = APP_KEY_EVENT_CLICK;
    keymap_l_tab[0].button_set_event = BUTTON_CLICK_DEFAULT;

    keymap_l_tab[1].lr = BUTTON_SETTING_LEFT_EARPHONE_CMD;
    keymap_l_tab[1].status.code = POWER_KEY;
    keymap_l_tab[1].status.event = APP_KEY_EVENT_DOUBLECLICK;
    keymap_l_tab[1].button_set_event = BUTTON_DOUBLE_DEFAULT;

    keymap_l_tab[2].lr = BUTTON_SETTING_LEFT_EARPHONE_CMD;
    keymap_l_tab[2].status.code = POWER_KEY;
    keymap_l_tab[2].status.event = APP_KEY_EVENT_TRIPLECLICK;
    keymap_l_tab[2].button_set_event = BUTTON_TRIPLE_DEFAULT;

    keymap_l_tab[3].lr = BUTTON_SETTING_LEFT_EARPHONE_CMD;
    keymap_l_tab[3].status.code = POWER_KEY;
    keymap_l_tab[3].status.event = HAL_KEY_EVENT_LONGPRESS;
    keymap_l_tab[3].button_set_event = BUTTON_LONG_PRESS_DEFAULT;

    memcpy(nvrecord_uienv->buttonInfo_left, keymap_l_tab, sizeof(keymap_l_tab));
    // memcpy(nvrecord_uienv->buttonInfo_right, keymap_r_tab, sizeof(keymap_r_tab));
    nvrecord_uienv->space_audio_type = 0;
    nv_record_env_set(nvrecord_uienv);
    nv_record_flash_flush();
}

static void app_ui_button_cmd_event(uint8_t cmd)
{
    uint8_t device_id = app_bt_audio_get_device_for_user_action();
    POSSIBLY_UNUSED struct BT_DEVICE_T* a2dp_device = app_bt_get_device(app_bt_audio_get_curr_a2dp_device());

    BESUI_TRACE(2,"[UIAPPKEY]%s, cmd = %d", __func__, cmd);
    BESUI_TRACE(0,"[UIAPPKEY]%s, play_pause_flag=%d, streamming=%d", __func__, a2dp_device->a2dp_play_pause_flag, a2dp_device->a2dp_streamming);

    switch(cmd)
    {
        case BUTTON_SETTING_PRE_SONG_CMD:
            app_ibrt_if_a2dp_send_backward(device_id);
        break;
        case BUTTON_SETING_NEXT_SONG_CMD:
            app_ibrt_if_a2dp_send_forward(device_id);
        break;
#if ANC_APP
        case BUTTON_SETTING_ANC_CMD:
            app_ui_anc_key_handle_func_click();
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
            stereo_siri_onoff();
        break;
#endif
        case BUTTON_SETTING_GAME_MODE_CMD:
            stereo_game_mode_switch();
        break;
        case BUTTON_SETTING_ALGO_CMD:
#ifdef USE_ALGO_EN
#if defined(__VIRTUAL_SURROUND__) || defined(__VIRTUAL_SURROUND_STEREO__)
            stereo_bes_spatial_onoff();
#elif defined(DOLBY_AUDIO_ENABLE)
            algo_send_request(ALGO_ID_DOLBY, 2-(!dolby_audio_onoff_get()), uictl.dolby_mode, 0, 0, 0);
#endif
#endif //#ifdef USE_ALGO_EN
        break;
        default:
        BESUI_TRACE(0, "[UIAPPKEY]%s, error cmd", __func__);
        break;
    }
}

void stereo_tota_button_event_handler(APP_KEY_STATUS *status, bool is_right)
{
    memcpy(keymap_l_tab, nvrecord_uienv->buttonInfo_left, sizeof(keymap_l_tab));
    BESUI_TRACE(0, "KEY_STATUS        %d %d %d", is_right, status->code, status->event);
    for(uint8_t i = 0;i < 4;i ++)
    {
        BESUI_TRACE(0, "button_event_info left :%d %d %d %d", keymap_l_tab[i].lr, keymap_l_tab[i].status.code, keymap_l_tab[i].status.event,keymap_l_tab[i].button_set_event);
        if (keymap_l_tab[i].lr == BUTTON_SETTING_LEFT_EARPHONE_CMD)
        {
            if(keymap_l_tab[i].status.code == status->code)
            {
                if (keymap_l_tab[i].status.event == status->event)
                {
                    BESUI_TRACE(0, "key left = %d %d %d %d", i, is_right, status->code, status->event);
                    app_ui_button_cmd_event(keymap_l_tab[i].button_set_event);
                }
            }
        }
    }
}

void app_ui_key_handle(void)
{
    if(bt_key.code != 0xff)
    {
        BESUI_TRACE(3,"[UIKEY]%s code:%d evt:%d",__func__, bt_key.code, bt_key.event);
        switch(bt_key.code)
        {
            case POWER_KEY:
                app_ui_bt_key_handle_func_key(&bt_key);
                break;
            case VOLADD_KEY:
                app_ui_bt_key_handle_up_key((enum APP_KEY_EVENT_T)bt_key.event);
                break;
            case VOLSUB_KEY:
                app_ui_bt_key_handle_down_key((enum APP_KEY_EVENT_T)bt_key.event);
                break;

            //case (VOLSUB_KEY | ANC_KEY) :
            //case (VOLADD_KEY | VOLSUB_KEY) :
            case (ANC_KEY | VOLSUB_KEY) :            
                bt_key_handle_clear_device_key((enum APP_KEY_EVENT_T)bt_key.event);
                break;

            case ANC_KEY:
                app_ui_anc_key_handle_key((enum APP_KEY_EVENT_T)bt_key.event);
                break;

            // case (ANC_KEY | VOLADD_KEY) :
            //     //app_ui_single_line_upgrade((enum APP_KEY_EVENT_T)bt_key.event);
            //     break;
            default:
                BESUI_TRACE(0,"[UIKEY]bt_key_handle  undefined key");
                break;
        }
        bt_key.code = 0xff;
    }
}

#if defined(OTA_BOOT_COPY_EN) || defined(BESUI_STEREO_EN)
void stereo_ota_boot_or_single_update(bool ota_flag, bool single_flag)
{
    BESUI_TRACE(1,"[UIKEY]%s ota_flag %d",__func__, ota_flag);

    if((ota_flag)&&(single_flag))
        return;

    osDelay(100);
    if(ota_flag)
    {
        hal_sw_bootmode_clear(HAL_SW_BOOTMODE_REBOOT); //ota_boot
        hal_norflash_disable_protection(HAL_FLASH_ID_0);

        hal_sw_bootmode_set(HAL_SW_BOOTMODE_ENTER_HIDE_BOOT);
        #ifdef __KMATE106__
        app_status_indication_set(APP_STATUS_INDICATION_OTA);
        media_PlayAudio(AUD_ID_BT_WARNING, 0);
        osDelay(1200);
        #endif
        #ifdef SECURE_BOOT_WORKAROUND_SOLUTION
        app_wdt_reopen(10);
        hal_cmu_sys_reboot();
        #else
        pmu_reboot();
        #endif
    }
    else if(single_flag)
    {
        hal_sw_bootmode_clear(HAL_SW_BOOTMODE_REBOOT); //single wire mode
        hal_sw_bootmode_set(HAL_SW_BOOTMODE_SINGLE_LINE_DOWNLOAD);
        pmu_reboot();
    }
}
#endif //#ifdef OTA_BOOT_COPY_EN
#endif //BESUI_STEREO_EN