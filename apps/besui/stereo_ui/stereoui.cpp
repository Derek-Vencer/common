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
#include "stereo_led.h"
#include "string.h"
#include "stereo_prompt.h"
#include "apps.h"
#include "app_bt.h"
#include "app_hfp.h"
#include "btapp.h"
#include "ota_spp.h"
#include "bes_gap_api.h"
#include "bts_bt_if.h"
#include "bts_tws_if.h"
#include "bts_core_if.h"
#include "bts_bt_conn.h"
#include "audio_policy.h"

#include "app_thread.h"
#include "app_tws_ibrt.h"
// #include "app_tws_ibrt_conn_api.h"
#include "app_ibrt_conn_evt.h"
#include "app_ui_api.h"
#include "app_battery.h"

#include "pmu.h"
#include "hal_bootmode.h"
#if !defined(CHIP_BEST1501P) && !defined(CHIP_BEST1502X)
#include CHIP_SPECIFIC_HDR(charger)
#endif
#include "app_audio.h"
#include "app_bt_stream.h"

#if defined(ANC_APP)
#include "app_anc.h"
#endif

#ifdef THH_UART_TEST_TOOL_EN
#include "uart_factory.h"
#endif
#include "stereo_key.h"
#include "stereoui.h"
#include "besui_common.h"

extern uint8_t app_poweroff_flag;
osTimerId app_ui_second_timer_id = NULL;

extern bt_status_t LinkDisconnectDirectly(bool PowerOffFlag);

static POSSIBLY_UNUSED void poweroff_timer_stop(void);
int app_pwl_low_level_drive_stop(enum APP_PWL_ID_T id);

//-------------------------------------------------------------------------------------
bool enc_onoff = true;
uint8_t mic_type = OPEN_MASTER_SLAVE_MIC;
void app_common_mic_hfp_enc_control(bool enc, uint8_t mic)
{
    enc_onoff = enc;
    mic_type = mic;
}

bool app_get_mic_hfp_enc_onoff(void)
{
    return enc_onoff;
}

uint8_t app_get_mic_hfp_enc_test_type(void)
{
    return mic_type;
}

uint8_t speech_mic_sta = 0;
void speech_mic_set_onoff(uint8_t sta)
{
    BESUI_TRACE(1,"[UICOM]%s, %d", __func__, speech_mic_sta);
    speech_mic_sta = sta;
    app_audio_sendrequest(APP_BT_STREAM_HFP_PCM, (uint8_t)APP_BT_SETTING_RESTART, 0);
}
uint8_t speech_mic_get_onoff(void)
{
    BESUI_TRACE(1,"[UICOM]%s, %d", __func__, speech_mic_sta);
    return speech_mic_sta;
}
//-------------------------------------------------------------------------------------

#ifdef STEREO_HALL_EN
static bool app_charging_status =  false;
bool app_get_charging_status(void)
{
    BESUI_TRACE(0, "[UICHARGER]%s, %d", __func__, app_charging_status);
    return app_charging_status;
}

void app_set_charging_status(bool flag)
{
    BESUI_TRACE(0, "[UICHARGER]%s, %d", __func__, flag);
    app_charging_status = flag;
}

bool app_ui_charging_io_read(HAL_GPIO_PIN_T hal_pin)
{
    uint8_t cnt = 0;
    bool is_charging_status = hal_gpio_pin_get_val(hal_pin);
    for (uint8_t i = 0; i < 20; i++)
    {
        if (is_charging_status == hal_gpio_pin_get_val(hal_pin)) {
            ++cnt;
        }
        else {
            is_charging_status = hal_gpio_pin_get_val(hal_pin);
            cnt = 0;
        }
        osDelay(1);
        if (cnt > 5) {
            BESUI_TRACE(2, "is_charging_status is %d, i is %d", is_charging_status, i);
            app_set_charging_status(is_charging_status);
            return is_charging_status;
        }
    }
    return is_charging_status;
}

void app_ui_full_charged_gpio_irq_handler(enum HAL_GPIO_PIN_T pin)
{
    BESUI_TRACE(0, "%s HAL_IOMUX_PIN_P0_3  %d", __func__, pin);
    if (pin == (enum HAL_GPIO_PIN_T)HAL_IOMUX_PIN_P0_3) {
        BESUI_TRACE(0, "%s HAL_IOMUX_PIN_P0_3  %d", __func__, pin);
        if(app_ui_charging_io_read(pin)) {
            BESUI_TRACE(1,"FULL_CHARGING!!!");
            app_status_indication_set(APP_STATUS_INDICATION_CHARGED_FULLY);
        }
    }
}

void app_ui_charge_gpio_irq_handler(enum HAL_GPIO_PIN_T pin)
{
    BESUI_TRACE(0, "%s  %d", __func__, pin);
    if (pin == (enum HAL_GPIO_PIN_T)HAL_IOMUX_PIN_P1_7) {
        BESUI_TRACE(0, "%s HAL_IOMUX_PIN_P1_7 %d", __func__, pin);
        if(!app_ui_charging_io_read(pin)) {
            hal_sw_bootmode_clear(HAL_SW_BOOTMODE_REBOOT);
            pmu_shutdown();
        }
    } else if (pin == (enum HAL_GPIO_PIN_T)HAL_IOMUX_PIN_P0_3) {
        BESUI_TRACE(0, "%s HAL_IOMUX_PIN_P0_3  %d", __func__, pin);
        if(app_ui_charging_io_read(pin)) {
            BESUI_TRACE(1,"FULL_CHARGING!!!");
            app_status_indication_set(APP_STATUS_INDICATION_CHARGED_FULLY);
        }
    }
}

const struct HAL_IOMUX_PIN_FUNCTION_MAP app_ui_charge_role_cfg[] = {
        HAL_IOMUX_PIN_P1_7, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL,
        HAL_IOMUX_PIN_P0_3, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL,
    };

void app_ui_charge_gpio_init(void)
{
    hal_iomux_init((struct HAL_IOMUX_PIN_FUNCTION_MAP *)&app_ui_charge_role_cfg[0], 1);
    hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)app_ui_charge_role_cfg[0].pin, HAL_GPIO_DIR_IN, 0);
    hal_iomux_init((struct HAL_IOMUX_PIN_FUNCTION_MAP *)&app_ui_charge_role_cfg[1], 1);
    hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)app_ui_charge_role_cfg[1].pin, HAL_GPIO_DIR_IN, 0);
}

void app_ui_charge_gpio_iqr_init()
{
    static const struct HAL_GPIO_IRQ_CFG_T charing_gpiocfg = {
        .irq_enable = true,
        .irq_debounce = true,
        .irq_type = HAL_GPIO_IRQ_TYPE_LEVEL_SENSITIVE,
        .irq_polarity = HAL_GPIO_IRQ_POLARITY_LOW_FALLING,
        .irq_handler = app_ui_charge_gpio_irq_handler,
    };

    hal_gpio_setup_irq((enum HAL_GPIO_PIN_T)app_ui_charge_role_cfg[0].pin, &charing_gpiocfg);
    // hal_gpio_pin_set((enum HAL_GPIO_PIN_T)app_ui_charge_role_cfg[i].pin);

    static const struct HAL_GPIO_IRQ_CFG_T fullchaged_gpiocfg = {
        .irq_enable = true,
        .irq_debounce = true,
        .irq_type = HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE,
        .irq_polarity = HAL_GPIO_IRQ_POLARITY_HIGH_RISING,
        .irq_handler = app_ui_full_charged_gpio_irq_handler,
    };

    hal_gpio_setup_irq((enum HAL_GPIO_PIN_T)app_ui_charge_role_cfg[1].pin, &fullchaged_gpiocfg);
}
#endif //#ifdef STEREO_HALL_EN

static void poweroff_process_timehandler(void const *n)
{
    BESUI_TRACE(0, "[UITIMER][%s], %d", __func__, app_poweroff_flag);
    poweroff_timer_stop();
    if (app_poweroff_flag) {
        app_pwl_low_level_drive_stop(APP_PWL_ID_1);
        app_pwl_stop(APP_PWL_ID_0);  //cfg0-blue, cfg1-red
        app_shutdown();
    } else {
        if(!app_bt_audio_count_streaming_a2dp() && !app_bt_audio_count_connected_sco())
            app_status_indication_set(APP_STATUS_INDICATION_PAIRING_CONNECTED);
        stereo_battery_update_to_phone();
    }
}

osTimerDef(APP_THREE_SECOND_TIMER, poweroff_process_timehandler);

static POSSIBLY_UNUSED void poweroff_timer_start(void)
{
    BESUI_TRACE(0, "[UITIMER][%s], %d", __func__, app_poweroff_flag);
	if(app_ui_second_timer_id == NULL)
	{
        app_ui_second_timer_id = osTimerCreate(osTimer(APP_THREE_SECOND_TIMER),osTimerOnce, NULL);
        if(!app_ui_second_timer_id)
             ASSERT(0, "[UITIMER][%s] osTimerCreate error", __func__);
    }
    //osTimerStop(app_ui_second_timer_id);
    if (app_poweroff_flag) {
        osTimerStart(app_ui_second_timer_id, 1500);
    } else {
        osTimerStart(app_ui_second_timer_id, 3*1000);
    }
}

static POSSIBLY_UNUSED void poweroff_timer_stop(void)
{
    if (app_ui_second_timer_id != NULL)
    {
        BESUI_TRACE(0, "[UITIMER][%s]", __func__);
        //osTimerStop(app_ui_second_timer_id);
        osTimerDelete(app_ui_second_timer_id);
        app_ui_second_timer_id = NULL;
    }
}

void app_ui_power_on_event()
{
    BESUI_TRACE(1, "%s ", __func__);
    app_status_indication_set(APP_STATUS_INDICATION_POWER_ON);
    app_voice_report(APP_STATUS_INDICATION_POWER_ON,0);
}

void app_ui_charge_event()
{
    BESUI_TRACE(1, "%s ", __func__);
    app_status_indication_set(APP_STATUS_INDICATION_CHARG);
}

void app_ui_power_off_event()
{
    BESUI_TRACE(1, "%s, app_poweroff_flag=%d", __func__, app_poweroff_flag);
#ifdef STEREO_HALL_EN
    BESUI_TRACE(0, "%s, charging_status=%d", __func__, app_get_charging_status());
    if (app_get_charging_status())
    {
        app_shutdown();
        return;
    }
#endif

    if (!app_poweroff_flag)
    {
        ota_disconnect();
        bes_ble_gap_disconnect_all();
        LinkDisconnectDirectly(true);

        app_voice_report(APP_STATUS_INDICATION_POWER_OFF,0);
        app_status_indication_set(APP_STATUS_INDICATION_POWER_OFF);
        app_poweroff_flag = true;
        poweroff_timer_start();
    }
}

void app_ui_idle_event()
{
    BESUI_TRACE(1, "%s ", __func__);
}

void app_ui_reconnect_a_event()
{
    BESUI_TRACE(1, "%s besui_get_profile_conn_num is %d", __func__, besui_get_profile_conn_num());
    app_status_indication_set(APP_STATUS_INDICATION_PAIRING_MODE);
}

void app_ui_reconnect_b_event()
{
    BESUI_TRACE(1, "%s besui_get_profile_conn_num is  %d", __func__, besui_get_profile_conn_num());

    app_status_indication_set(APP_STATUS_INDICATION_PAIRING_MODE);

}

// bool get_disconnect_08_event();
#ifdef BESUI_STEREO_EN
static bool is_08_disconnect = false;
bool get_disconnect_08_event(void)
{
    return is_08_disconnect;
}
void set_disconect_08_event(bool value)
{
    is_08_disconnect = value;
}
#endif


void app_ui_pair_a_event()
{
    BESUI_TRACE(1, "%s besui_get_profile_conn_num is %d", __func__, besui_get_profile_conn_num());
    poweroff_timer_stop();
    if(!besui_get_profile_conn_num())
    {
        app_status_indication_set(APP_STATUS_INDICATION_PAIRING_MODE);
        if (!get_disconnect_08_event())
        {
            app_start_10_second_timer(APP_SCAN_TIMER_ID);
        }
    }

    if(!app_poweroff_flag)
    {
        app_voice_report(APP_STATUS_INDICATION_PAIRING_MODE, 0);
    }
}

void app_ui_pair_b_event()
{
    BESUI_TRACE(1, "%s ", __func__);

}

void app_ui_connected_a_b_after_music_call_event(void)
{
    app_status_indication_set(APP_STATUS_INDICATION_PAIRING_CONNECTED);
    BESUI_TRACE(2, "%s besui_get_profile_conn_num is %d", __func__, besui_get_profile_conn_num());
}

bool call_end_flag = 0;
void set_disconect_08_event(bool value);
void app_ui_connected_a_event()
{
    BESUI_TRACE(2, "%s besui_get_profile_conn_num is %d", __func__, besui_get_profile_conn_num());
    if(besui_get_profile_conn_num() < 3)
    {
        if(call_end_flag)
            app_status_indication_set(APP_STATUS_INDICATION_PAIRING_CONNECTED);
        else
        	app_status_indication_set(APP_STATUS_INDICATION_PAIRING_SUCCESSFUL);
        call_end_flag = false;
        poweroff_timer_start();
        stereo_battery_update_to_phone();
    }
    set_disconect_08_event(false);
    app_stop_10_second_timer(APP_SCAN_TIMER_ID);
    app_ui_update_scan_type_policy(SCAN_EV_EXIT_PAIRING);
}

void app_ui_connected_b_event()
{
    BESUI_TRACE(2, "%s besui_get_profile_conn_num is %d", __func__, besui_get_profile_conn_num());
    if(besui_get_profile_conn_num() < 3)
    {
        if(call_end_flag)
            app_status_indication_set(APP_STATUS_INDICATION_PAIRING_CONNECTED);
        else
        	app_status_indication_set(APP_STATUS_INDICATION_PAIRING_SUCCESSFUL);
        call_end_flag = false;
        poweroff_timer_start();
        stereo_battery_update_to_phone();
    }
    set_disconect_08_event(false);
    app_stop_10_second_timer(APP_SCAN_TIMER_ID);
    app_ui_update_scan_type_policy(SCAN_EV_EXIT_PAIRING);
}

void app_ui_music_a_event()
{
    BESUI_TRACE(1, "%s ", __func__);
    poweroff_timer_stop();
    app_status_indication_set(APP_STATUS_INDICATION_CALL_MUSIC);
}

void app_ui_music_b_event()
{
    BESUI_TRACE(1, "%s ", __func__);
    poweroff_timer_stop();
    app_status_indication_set(APP_STATUS_INDICATION_CALL_MUSIC);
}

void app_ui_call_a_event()
{
    BESUI_TRACE(1, "%s ", __func__);
    poweroff_timer_stop();
    app_status_indication_set(APP_STATUS_INDICATION_CALL_MUSIC);
}

void app_ui_call_b_event()
{
    BESUI_TRACE(1, "%s ", __func__);
    poweroff_timer_stop();
    app_status_indication_set(APP_STATUS_INDICATION_CALL_MUSIC);
}

void app_ui_disconnected_event()
{
    BESUI_TRACE(1, "%s app_bt_is_any_connection is %d, get_disconnect_08_event %d", __func__, app_bt_is_any_connection(),get_disconnect_08_event());
    poweroff_timer_stop();
    app_voice_report(APP_STATUS_INDICATION_PAIRING_DISCONNECTED, 0);

    if(get_disconnect_08_event() && (!app_bt_is_any_connection()))
    {
        app_status_indication_set(APP_STATUS_INDICATION_PAIRING_LINK_LOSS);
        app_start_10_second_timer(APP_SCAN_TIMER_ID);
    }
}

void app_ui_incoming_event()
{
    poweroff_timer_stop();
    app_voice_report(APP_STATUS_INDICATION_INCOMING_CALL, 0);
    app_status_indication_set(APP_STATUS_INDICATION_CALL_MUSIC);
}

void app_ui_clear_device_list()
{
    app_status_indication_set(APP_STATUS_INDICATION_CLEAR_DEVICE_LIST);
}

void app_ui_reject_prompt(void)
{
    app_voice_report(APP_STATUS_INDICATION_REJECT_INCOMING_CALL, 0);
}

void app_ui_hungup_prompt(void)
{
    app_voice_report(APP_STATUS_INDICATION_END_CALL, 0);
}

static APP_STATUS_TYPE_T cur_status = APP_STATUS_TYPE_NONE;
static app_ui_bt_audio_event_t app_ui_machine_status = APP_UI_MAX_NUM;
static uint8_t sco_cur_deviceid = 255;
static uint8_t a2dp_cur_deviceid = 255;
void app_system_status_set(const APP_STATUS_TYPE_T type)
{
    BESUI_TRACE(2, "%s  The app ui status is %d ", __func__, type);
    if ((cur_status ==  type) && (type != APP_STATUS_TYPE_DISCONNECTED_A))
    {
        BESUI_TRACE(1, "%s is exist", __func__);
        return;
    }

    if (cur_status == APP_STATUS_TYPE_NONE && type != APP_STATUS_TYPE_POWER_ON)
    {
        BESUI_TRACE(1, "%s is fail", __func__);
        return;
    }

    cur_status = type;

    switch(type)
	{
		case APP_STATUS_TYPE_POWER_ON:
            app_ui_power_on_event();
			break;
		case APP_STATUS_TYPE_POWER_OFF:
            app_ui_power_off_event();
			break;
		case APP_STATUS_TYPE_CHARGE:
            app_ui_charge_event();
			break;
		case APP_STATUS_TYPE_IDLE:
            app_ui_idle_event();
			break;
		case APP_STATUS_TYPE_RECONNECT_A:
            app_ui_reconnect_a_event();
			break;
		case APP_STATUS_TYPE_RECONNECT_B:
            app_ui_reconnect_b_event();
			break;
		case APP_STATUS_TYPE_PAIR_A:
            app_ui_pair_a_event();
			break;
		case APP_STATUS_TYPE_PAIR_B:
            app_ui_pair_b_event();
			break;
		case APP_STATUS_TYPE_CONNECTED_A:
            app_ui_connected_a_event();
			break;
        case APP_STATUS_TYPE_CONNECTED_B:
            app_ui_connected_b_event();
			break;
        case APP_STATUS_TYPE_CONNECTED_AFTER_MUSIC_CALL:
            app_ui_connected_a_b_after_music_call_event();
            break;
        case APP_STATUS_TYPE_MUSIC_A:
            app_ui_music_a_event();
			break;
        case APP_STATUS_TYPE_MUSIC_B:
            app_ui_music_b_event();
			break;
        case APP_STATUS_TYPE_CALL_A:
            app_ui_call_a_event();
			break;
        case APP_STATUS_TYPE_CALL_B:
            app_ui_call_b_event();
			break;
        case APP_STATUS_TYPE_DISCONNECTED_A:
            app_ui_disconnected_event();
			break;
        case APP_STATUS_TYPE_DISCONNECTED_B:
            // app_ui_disconnected_b_event();
			break;
        case APP_STATUS_TYPE_CLEAR_DEVICE_LIST:
            app_ui_clear_device_list();
            break;
        case APP_STATUS_TYPE_INCOMMING_CALL:
            app_ui_incoming_event();
            break;
        case APP_STATUS_TYPE_CALL_REJECT_PROMPT:
            app_ui_reject_prompt();
            break; 
        case APP_STATUS_TYPE_CALL_HUNGUP_PROMPT:
            app_ui_hungup_prompt();
            break;                        
		default:
			break;
	}
}

APP_STATUS_TYPE_T app_system_status_get(void)
{
	return cur_status;
}

void pairmode_exit_timeout_process(void) //pairing mode exit, auto poweroff
{
    BESUI_TRACE(1,"%s",__func__);
    if(!besui_get_profile_conn_num())
    {
        app_status_indication_set(APP_STATUS_INDICATION_PAIRING_NOT_SUCCESSFUL);
        BESUI_TRACE(1,"app_shutdown");
        app_ui_power_off_event();
    }
    else
    {
        app_stop_10_second_timer(APP_SCAN_TIMER_ID);
    }
}

APP_BATTERY_STATUS_T app_battery_status(void);

void app_ui_state_machine(app_ui_bt_audio_event_t event,uint8_t device_id,uint8_t nv_device_cnt)
{
    uint8_t conn_devices = besui_get_profile_conn_num();
    BESUI_TRACE(0,"%s event=0x%x device_id=%d,conn_devices=%d",__func__,event,device_id, conn_devices);

    if (app_poweroff_flag)
        return;

    if(event == APP_UI_MAX_NUM)
        return ;

    BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    // ibrt_mobile_info_t *p_mobile_info = app_ibrt_conn_get_mobile_info_ext();

    app_ui_machine_status = event;

    sco_cur_deviceid  = app_bt_audio_get_curr_playing_sco();
    a2dp_cur_deviceid = app_bt_audio_get_curr_playing_a2dp();
    BESUI_TRACE(1,"[UISTATE]%s, sco_cur=%d, a2dp_cur=%d", __func__, sco_cur_deviceid, a2dp_cur_deviceid);

    switch (event)
    {
        case APP_UI_IDLE:
            app_system_status_set(APP_STATUS_TYPE_IDLE);
            break;
        case APP_UI_RECONNECTED:
            if(nv_device_cnt == 1)
                app_system_status_set(APP_STATUS_TYPE_RECONNECT_A);
            else if(nv_device_cnt == 2)
                app_system_status_set(APP_STATUS_TYPE_RECONNECT_B);
            break;
        case APP_UI_PAIR:
            app_system_status_set(APP_STATUS_TYPE_PAIR_A);
            break;
        case APP_UI_CHARGING:
            app_system_status_set(APP_STATUS_TYPE_CHARGE);
            break;
        case APP_UI_CONNECTED:
#ifdef CODEC_TYPE_APP_EN
            codec_type_to_app();
#endif
            if(sco_cur_deviceid == 255 && a2dp_cur_deviceid == 255)
            {
                if((curr_device->device_id) == 0)
                    app_system_status_set(APP_STATUS_TYPE_CONNECTED_A);
                else if((curr_device->device_id) == 1)
                    app_system_status_set(APP_STATUS_TYPE_CONNECTED_B);
            }
            break;
        case APP_UI_BT_AUDIO_EVENT_ACL_DISC:
            BESUI_TRACE(0, "APP_UI_BT_AUDIO_EVENT_ACL_DISC is %d", curr_device->device_id);
            if((curr_device->device_id) == 0)
                app_system_status_set(APP_STATUS_TYPE_DISCONNECTED_A);
            else if((curr_device->device_id)==1)
                app_system_status_set(APP_STATUS_TYPE_DISCONNECTED_B);
            break;

        case APP_UI_POWER_OFF:
                app_system_status_set(APP_STATUS_TYPE_POWER_OFF);
            break;
        case APP_UI_POWER_ON:
                app_system_status_set(APP_STATUS_TYPE_POWER_ON);
            break;
        case APP_UI_BT_AUDIO_EVENT_A2DP_STREAM_SUSPEND:
            if(!get_disconnect_08_event() && conn_devices && sco_cur_deviceid == 255 && a2dp_cur_deviceid == 255)
            {
                BESUI_TRACE(0, "APP_STATUS_TYPE_CONNECTED_AFTER_MUSIC_CALL");
                app_system_status_set(APP_STATUS_TYPE_CONNECTED_AFTER_MUSIC_CALL);
            }
            break;
        case APP_UI_BT_AUDIO_EVENT_HUNGUP_INCOMING_CALL:
            app_system_status_set(APP_STATUS_TYPE_CALL_REJECT_PROMPT);
            call_end_flag = true;
            if(curr_device->device_id == 0)
                app_system_status_set(APP_STATUS_TYPE_CONNECTED_A);
            else if(curr_device->device_id == 1)
                app_system_status_set(APP_STATUS_TYPE_CONNECTED_B);
            break;
        case APP_UI_BT_AUDIO_EVENT_HUNGUP_ACTIVE_CALL:
            app_system_status_set(APP_STATUS_TYPE_CALL_HUNGUP_PROMPT); 
            call_end_flag = true; 
            if(curr_device->device_id == 0)
                app_system_status_set(APP_STATUS_TYPE_CONNECTED_A);
            else if(curr_device->device_id == 1)
                app_system_status_set(APP_STATUS_TYPE_CONNECTED_B);
            break;
        case APP_UI_MUSIC_CALL:
            // if(bts_bt_if_is_dev_link_connected(&p_mobile_info->mobile_addr))
            {
                if (sco_cur_deviceid == 0 && a2dp_cur_deviceid == 255)
                    app_system_status_set(APP_STATUS_TYPE_CALL_A);
                else if(sco_cur_deviceid == 1 && a2dp_cur_deviceid == 255)
                    app_system_status_set(APP_STATUS_TYPE_CALL_B);
                else if(a2dp_cur_deviceid == 0 && sco_cur_deviceid == 255)
                    app_system_status_set(APP_STATUS_TYPE_MUSIC_A);
                else if(a2dp_cur_deviceid == 1 && sco_cur_deviceid == 255)
                    app_system_status_set(APP_STATUS_TYPE_MUSIC_B);
                else if(a2dp_cur_deviceid == 1 && sco_cur_deviceid == 1)
                    app_system_status_set(APP_STATUS_TYPE_CALL_B);
            }
            break;
        case APP_UI_IMCONNIG_CALL:
            app_system_status_set(APP_STATUS_TYPE_INCOMMING_CALL);
            break;
        default:
            break;
    }
}

static int app_ui_state_machine_process(APP_MESSAGE_BODY *msg_body)
{
    app_ui_state_machine((app_ui_bt_audio_event_t)msg_body->message_Param0, msg_body->message_Param1, msg_body->message_Param2);
    return 0;
}

void charger_charge_param_cfg_set_enable_disenable(bool open_close_flag, uint8_t battery_current)
{
#if 0
    struct CHARGER_CHARGE_MODULE_CFG_T charge_cfg;

    charger_charge_module_cfg_get(&charge_cfg);

    BESUI_TRACE(1,"%s %d %d %d %d %d %d %d",__func__,
    charge_cfg.prechg_current,charge_cfg.cc_current,charge_cfg.stop_current,
    charge_cfg.cv_volt,charge_cfg.rechg_volt,
    charge_cfg.rechg_en,charge_cfg.chg_en);

    BESUI_TRACE(1,"%s  %d", __func__, battery_current);
    charge_cfg.cc_current = (CHARGER_CHARGE_CONSTANT_CURRENT_E)battery_current;// battery_current;//CHARGER_CHARGE_CONSTANT_CURRENT_60MA;

    if(open_close_flag)
    {
        charge_cfg.chg_en = 1;
    }
    else
    {
        charge_cfg.chg_en = 0;
    }
    //BESUI_TRACE(1,"%s chg_en %d", __func__, charge_cfg.chg_en);
    charger_charge_module_cfg_set(&charge_cfg);

    charger_charge_module_cfg_get(&charge_cfg);

    BESUI_TRACE(1,"get %s %d %d %d %d %d %d %d",__func__,
    charge_cfg.prechg_current,charge_cfg.cc_current,charge_cfg.stop_current,
    charge_cfg.cv_volt,charge_cfg.rechg_volt,
    charge_cfg.rechg_en,charge_cfg.chg_en);
#endif
}

POSSIBLY_UNUSED static int app_ui_hall_handle_process_charging(APP_BATTERY_STATUS_T status, uint32_t volt, uint32_t charger) //per 10s
{
    static uint16_t under_count = 0;
    BESUI_TRACE(0,"[UILOW]status=%d,volt=%d,charger=%d,count=%d", status, volt, charger, under_count);
    if(uictl.batt_low_flag)
    {
        if(under_count == 0 || (under_count % 3) == 0)  //led per 30s tog 5times 
            app_status_indication_set(APP_STATUS_INDICATION_BATTERY_LOW); //led

        if(under_count == 0 || under_count == 60)       //prompt per 10min
            app_voice_report(APP_STATUS_INDICATION_BATTERY_LOW, 0);
        under_count ++;
        if(under_count >= 6000)
            under_count = 0;
    }
    switch (status)
    {
        case APP_BATTERY_STATUS_UNDERVOLT:  //< 3.52V APP_BATTERY_MIN_MV -low batterys
            uictl.batt_low_flag = 1;
            break;
        case APP_BATTERY_STATUS_PDVOLT: //< 3.2V -poweroff
            BESUI_TRACE(1,"APP_UI PDVOLT-->POWEROFF:%d", volt);
            app_ui_power_off_event();
            break;
        default:
            break;
    }
    return 0;
}


POSSIBLY_UNUSED int stereoui_battery_process(uint32_t status, uint32_t volt) //per 10s
{
    static uint16_t under_count = 0;
    BESUI_TRACE(0,"[UILOW]%s, status=%d,volt=%d,count=%d", __func__, status, volt, under_count);
    if(uictl.batt_low_flag)
    {
        if(under_count == 0 || (under_count % 3) == 0)  //led per 30s tog 5times 
            app_status_indication_set(APP_STATUS_INDICATION_BATTERY_LOW); //led

        if(under_count == 0 || under_count == 60)       //prompt per 10min
            app_voice_report(APP_STATUS_INDICATION_BATTERY_LOW, 0);
        under_count ++;
        if(under_count >= 6000)
            under_count = 0;
    }
    switch (status)
    {
        case APP_BATTERY_STATUS_UNDERVOLT:  //< 3.52V APP_BATTERY_MIN_MV -low batterys
            uictl.batt_low_flag = 1;
            break;
        case APP_BATTERY_STATUS_PDVOLT: //< 3.2V -poweroff
            BESUI_TRACE(1,"APP_UI PDVOLT-->POWEROFF:%d", volt);
            app_ui_power_off_event();
            break;
        default:
            break;
    }
    return 0;
}

#ifdef STEREO_HALL_EN
static bool hall_flag = 0;
void hall_poweroff_set_flag(bool sta)
{
    hall_flag = sta;
}
bool hall_poweroff_get_flag(void)
{
    BESUI_TRACE(0, "[UIHALL]%s, hall_flag = %d", __func__, hall_flag);
    return hall_flag;
}

enum HALL_CHARGER_T hall_sta_get(void)
{
    const char hall_tab[3][11] = {"HALL_CLOSE", "HALL_OPEN", "HALL_ERR"};
    enum APP_BATTERY_CHARGER_T charger = APP_BATTERY_CHARGER_QTY;
    charger = (enum APP_BATTERY_CHARGER_T)pmu_charger_get_status();
    if (charger == (enum APP_BATTERY_CHARGER_T)PMU_CHARGER_PLUGIN)
        charger = APP_BATTERY_CHARGER_PLUGIN;
    else
        charger = APP_BATTERY_CHARGER_PLUGOUT;
    BESUI_TRACE(0,"[UIHALL]%s, charger=%d, %s", __func__, charger, hall_tab[charger]);
    return (enum HALL_CHARGER_T)charger;
}

void app_ui_hall_fast_switching(void)
{
    BESUI_TRACE(0, "[UIHALL]%s", __func__);
    if(!hall_poweroff_get_flag()) //if MFB poweroff need not judge hall sta
        return;
    hall_poweroff_set_flag(false);
    if (hall_sta_get() == HALL_STA_OPEN)
    {
        BESUI_TRACE(0,"[UIHALL]%s, hall is open ,need reboot",__func__);
        pmu_reboot();
    }
}

static bool once_hall_in_after_key = 0;
void judge_hall_time(uint8_t charge)
{
    BESUI_TRACE(0,"[UIHALL]%s, charge = %d",__func__, charge);

    static uint8_t before_charge = 101;

    if(charge == before_charge)
        return;

    before_charge = charge;
    if (charge == APP_BATTERY_CHARGER_PLUGIN) //hall open
        once_hall_in_after_key = true;
    BESUI_TRACE(0,"[UIHALL]%s, once_hall_in_after_key = %d",__func__, once_hall_in_after_key);
}

#define USE_HALL_TIMER_POWEROFF     1
#if USE_HALL_TIMER_POWEROFF
static uint8_t hall_poweroff_cnt = 0;
osTimerId hall_poweroff_timer_id = NULL;
void hall_poweroff_timer_onoff(bool timer_en);
void hall_poweroff_process_timehandler(void const *param)
{
	BESUI_TRACE(1, "[UIHALL][UITIMER]%s hall_poweroff_cnt = %d", __func__, hall_poweroff_cnt);
	hall_poweroff_cnt ++;
    if(hall_poweroff_cnt >= 10) //shake poweroff timeout
    {
        hall_poweroff_cnt = 0;
        if (hall_sta_get() == HALL_STA_CLOSE)
        {
            hall_poweroff_set_flag(true);
            app_ui_power_off_event();
        }
        osTimerDelete(hall_poweroff_timer_id);
        hall_poweroff_timer_id = NULL;
    }
    else
    {
        if (hall_sta_get() == HALL_STA_OPEN)
        {
            hall_poweroff_cnt = 0;
            osTimerDelete(hall_poweroff_timer_id);
            hall_poweroff_timer_id = NULL;
        }
        else
        {
            hall_poweroff_timer_onoff(true);            
        }
    }
    if(uictl.prompt_end)
    {
        hall_poweroff_cnt = 0;
    }
}
osTimerDef (HALL_POWEROFF_NAME, (void (*)(void const *))hall_poweroff_process_timehandler);

void hall_poweroff_timer_onoff(bool timer_en)
{
	BESUI_TRACE(1, "[UIHALL][UITIMER]%s timer_en %d ", __func__, timer_en);

	if(hall_poweroff_timer_id == NULL)
	{
    	hall_poweroff_timer_id = osTimerCreate(osTimer(HALL_POWEROFF_NAME),osTimerOnce,NULL);
        if(!hall_poweroff_timer_id)
             ASSERT(0, "[UIHALL][UITIMER][%s] osTimerCreate error", __func__);
	}

	if(timer_en)
    {
		osTimerStart(hall_poweroff_timer_id, 100);        
    }
	else
    {
		osTimerStop(hall_poweroff_timer_id);
        hall_poweroff_cnt = 0;    
    }

}
#endif


#if defined(CHIP_BEST1501) || defined(CHIP_BEST1501P)
enum PMU_BOOT_CAUSE_T pmu_boot_cause_get(void)
{
    return PMU_BOOT_CAUSE_NULL;
}
#endif

void app_ui_hall_handle(APP_BATTERY_MV_T volt, uint8_t currlevel, enum APP_BATTERY_STATUS_T battery_status, uint32_t status, union APP_BATTERY_MSG_PRAMS prams)
{
    BESUI_TRACE(0,"[UIHALL]%s,volt=%d,currlevel=%d,battery_status=%d,status=%d,charger=%d", __func__, volt, currlevel, battery_status, status, prams.charger);

    if(hall_poweroff_get_flag())
        return;

    enum HALL_CHARGER_T hall_charger = HALL_STA_QTY;
    hall_charger = (enum HALL_CHARGER_T)prams.charger;
    if(hall_charger >= HALL_STA_QTY)
    {
        hall_charger = hall_sta_get();
        BESUI_TRACE(0,"[UIHALL]%s, hall_charger=%d", __func__, hall_charger);
    }

    judge_hall_time((enum HALL_CHARGER_T)hall_charger);

    uint16_t boot_type = (pmu_boot_cause_get() & PMU_BOOT_CAUSE_POWER_KEY);
    BESUI_TRACE(0,"[UIHALL]%s, boot_type=0x%04X", __func__, boot_type);
    switch (battery_status)
    {
        case APP_BATTERY_STATUS_NORMAL:
            if (boot_type && !once_hall_in_after_key) //MFB poweron
                break;
#if USE_HALL_TIMER_POWEROFF
            if (hall_charger == HALL_STA_OPEN) //hall open
                hall_poweroff_timer_onoff(false);
            else //if (hall_charger != HALL_STA_OPEN) //hall close
                hall_poweroff_timer_onoff(true);
#else
            hall_poweroff_set_flag(true);
            app_ui_power_off_event();
#endif
            break;

        case APP_BATTERY_STATUS_CHARGING: //hall open
            app_ui_hall_handle_process_charging((APP_BATTERY_STATUS_T)status, volt, prams.charger);
            break;

        default:
            break;
    }
    return;
}
#endif //#ifdef STEREO_HALL_EN

void app_ui_status_init(void)
{
    app_set_threadhandle(APP_MODUAL_UI, app_ui_state_machine_process);
#ifdef STEREO_HALL_EN
    app_battery_register(app_ui_hall_handle);
#endif
}

int app_ui_send_request(uint32_t message_id, uint32_t param0, uint32_t param1, uint32_t param2, uint32_t ptr)
{
    APP_MESSAGE_BLOCK msg;

    BESUI_TRACE(0,"[UIMSG]%s, 0x%X, 0x%X, 0x%X, 0x%X, 0x%X", __func__, message_id, param0, param1, param2, ptr);

    msg.mod_id = APP_MODUAL_UI;
#if defined(USE_BASIC_THREADS)
    msg.mod_level = APP_MOD_LEVEL_1;
#endif
    msg.msg_body.message_id = message_id;
    msg.msg_body.message_Param0 = param0;
    msg.msg_body.message_Param1 = param1;
    msg.msg_body.message_Param2 = param2;
    msg.msg_body.message_ptr = ptr;

    return app_mailbox_put(&msg);
}

uint8_t batt_curr = 0;
uint8_t batt_last = 0xFF;
static uint8_t temp_level = 10;
static uint16_t volt_curr = 0;
uint8_t stereo_battery_level_process(uint32_t status, uint16_t battery_volt)
{
    volt_curr = battery_volt;

    BESUI_TRACE(0, "[UIBAT]status = %d, volt=%d", status, battery_volt);

    if(battery_volt < 2800) //err sta
    {
        BESUI_TRACE(0, "[UIBAT]%s, battery error! volt=%d", __func__, battery_volt);
        return 10;        
    }

    const uint16_t bat_tab[10] = {3300, 3470, 3540, 3590, 3620, 3650, 3720, 3800, 3900, 4000};
    batt_curr = 0;
    for(uint8_t i = 9;i >= 0;i --)
    {
        if(battery_volt > bat_tab[i])
        {
            batt_curr = i+1;
            break;
        }
    }
    BESUI_TRACE(0, "[UIBAT]%s, volt = %d, cur = %d, last = %d", __func__, battery_volt, batt_curr, batt_last);


     if(batt_curr <= 1) //10%
        uictl.batt_low_flag = 1;

    BESUI_TRACE(0, "[UIBAT]%s, batt_curr=%d, batt_last=%d", __func__, batt_curr, batt_last);

    if(batt_curr != batt_last)
    {
        if((batt_last==0xFF) || ((batt_curr < batt_last) && (batt_last-batt_curr<=2)))
        {
            batt_last = batt_curr;
            temp_level = batt_curr;
            stereo_battery_update_to_phone();
        }
        else
        {
            temp_level = batt_last;
        }
    }
    else
    {
        if(batt_curr != 10)
        {
            temp_level = batt_curr;
        }
    }

    BESUI_TRACE(0,"[UIBAT]%s, battery_volt=%d, temp_level=%d", __func__, battery_volt, temp_level);
    return temp_level;
}

uint16_t battery_volt_level_get(bool sta)
{
    if(sta == true)
        return temp_level;
    else
        return volt_curr;
}

void stereo_battery_update_to_phone(void)
{
    uint8_t batlev = temp_level;
    uint8_t conn_devices = besui_get_profile_conn_num();
    if(batlev)
        batlev --;
    BESUI_TRACE(0, "[UIBAT]%s, level=%d, conn_devices=%d", __func__, batlev, conn_devices);
    app_hfp_battery_report_reset(BT_DEVICE_ID_1);
#if (BT_DEVICE_NUM > 1)
    app_hfp_battery_report_reset(BT_DEVICE_ID_2);
#endif
    if(conn_devices > 0)
    {
        BESUI_TRACE(0, "[UIBAT]%s, batlev=%d", __func__, batlev);
        app_hfp_set_battery_level(batlev);
    }
}

uint8_t stereo_gfps_battery_level(uint8_t charge_status, uint8_t *batteryLevel)
{
    uint8_t batlev = temp_level;
    if(batlev)
        batlev --;

    batteryLevel[0] = (batlev * 10) | (charge_status << 7);
	// batteryLevel[2] = 0x7F; //box
	BESUI_TRACE(0, "[UIBAT]%s, batlev=%d, gfps_level=%d", __func__, batlev, batteryLevel[0]);
    return 1;
}

#if defined(ANC_APP)
static app_anc_mode_t user_anc_mode = APP_ANC_MODE_OFF;
void besui_set_curr_anc_mode(uint8_t sta)
{
    BESUI_TRACE(0, "[UIANC]%s, sta = %d", __func__, sta);
    user_anc_mode = (app_anc_mode_t)sta;
}
uint8_t besui_get_curr_anc_mode(void)
{
    BESUI_TRACE(0, "[UIANC]%s, sta = %d", __func__, user_anc_mode);
    return user_anc_mode;
}
int32_t besui_anc_set_mode(uint8_t sta)
{
    BESUI_TRACE(0, "[UIANC]%s, sta = %d", __func__, sta);
    app_anc_switch((app_anc_mode_t)sta);
    besui_set_curr_anc_mode((app_anc_mode_t)sta);
    return 0;
}

void besui_anc_key_switch(void)
{
    uint8_t mode_buf = APP_ANC_MODE_OFF;
#if defined(ANC_SPEAKTHRU_EN)
    if(besui_get_curr_anc_mode() == APP_ANC_MODE3)
        besui_set_curr_anc_mode(APP_ANC_MODE2);
    else 
#endif
    if(besui_get_curr_anc_mode() == APP_ANC_MODE2)
        besui_set_curr_anc_mode(APP_ANC_MODE1);
    else if(besui_get_curr_anc_mode() == APP_ANC_MODE1)
        besui_set_curr_anc_mode(APP_ANC_MODE_OFF);
    else if(besui_get_curr_anc_mode() == APP_ANC_MODE_OFF)
        besui_set_curr_anc_mode(APP_ANC_MODE2);

    mode_buf = besui_get_curr_anc_mode();
    if(mode_buf == APP_ANC_MODE_OFF)
        mode_buf = 4;
    algo_send_request(ALGO_ID_ANC, mode_buf, 0, 0, 0, 1);

    BESUI_TRACE(0, "[UIANC]%s, anc_mode = %d", __func__, besui_get_curr_anc_mode());
}
#endif

#ifdef DC_DETECT_EN
enum DC_CHARGER_T charger_usb_sta_get(void)
{
    const char hall_tab[3][7] = {"DC_OUT", "DC_IN", "DC_ERR"};
    enum APP_BATTERY_CHARGER_T charger = APP_BATTERY_CHARGER_QTY;
    charger = (enum APP_BATTERY_CHARGER_T)pmu_charger_get_status();
    if (charger == (enum APP_BATTERY_CHARGER_T)PMU_CHARGER_PLUGIN)
        charger = APP_BATTERY_CHARGER_PLUGIN;
    else
        charger = APP_BATTERY_CHARGER_PLUGOUT;
    BESUI_TRACE(0,"[UIDC]%s, charger=%d, %s", __func__, charger, hall_tab[charger]);
    return (enum DC_CHARGER_T)charger;
}
/*
void dc_det_battery_cb(APP_BATTERY_MV_T volt, uint8_t currlevel, enum APP_BATTERY_STATUS_T battery_status, uint32_t status, union APP_BATTERY_MSG_PRAMS prams)
{
    BESUI_TRACE(0,"[UIDC]%s,volt=%d,currlevel=%d,battery_status=%d,status=%d,charger=%d", __func__, volt, currlevel, battery_status, status, prams.charger);

    if(charger_usb_sta_get() != DC_IN)
    {
        app_shutdown();
    }

    uint16_t boot_type = (pmu_boot_cause_get() & PMU_BOOT_CAUSE_POWER_KEY);
    BESUI_TRACE(0,"[UIDC]%s, boot_type=0x%04X", __func__, boot_type);
    switch (battery_status)
    {
        case APP_BATTERY_STATUS_NORMAL:

            break;

        case APP_BATTERY_STATUS_CHARGING: //DC_IN
            //sapp_ui_hall_handle_process_charging((APP_BATTERY_STATUS_T)status, volt, prams.charger);
            break;

        default:
            break;
    }
    return;
}
*/
#endif //#ifdef DC_DETECT_EN

#ifdef AUX_DETECT_EN
bool aux_sta = false;
void linein_set_sta(bool param)
{
    aux_sta = param;
}
bool linein_get_sta(void)
{
	BESUI_TRACE(0, "[UIAUX]%s, aux_sta = %d", __func__, aux_sta);
    return aux_sta;
}

void aux_capture_irqhandler(uint16_t irq_val, HAL_GPADC_MV_T volt)
{
    //uint32_t meanVolt = 0;
    //BESUI_TRACE(0, "[UIEXT]%s, irq_val=%d, volt=%d",__func__, irq_val, volt);

    if (volt == HAL_GPADC_BAD_VALUE)
        return;
    pmu_ntc_capture_disable();
    static uint16_t volt_buf = 0;
    if(volt_buf != volt)
    {
        volt_buf = volt;
        BESUI_TRACE(0, "[UIAUX]%s, aux_volt=%d",__func__, volt);        
    }
#if 1
    static uint8_t detcnt = 0;
	//BESUI_TRACE(0, "[UIAUX][UITIMER]%s", __func__);
    if(volt > 1470)
    {
        if(detcnt < 2)
        {
            detcnt ++;
            if(detcnt == 2)
            {
                linein_set_sta(true);
                BESUI_TRACE(0, "[UIAUX]%s, aux_sta = 1", __func__);
                uictl.shutdown_type = SHUTDOWN_LINEIN_INSERT;
                poweroff_timer_stop();
                app_pwl_low_level_drive_stop(APP_PWL_ID_1);
                app_pwl_stop(APP_PWL_ID_0);  //cfg0-blue, cfg1-red
                app_shutdown();
            }
        }
    }
    else
    {
        detcnt = 0;
        linein_set_sta(false);
        //BESUI_TRACE(0, "[UIAUX][IRQ]%s, aux_sta = 0", __func__);
    }
#endif
}

void besui_adc_aux_config(void)
{
    pmu_ntc_capture_enable();
    hal_gpadc_open(HAL_GPADC_CHAN_5, HAL_GPADC_ATP_ONESHOT, aux_capture_irqhandler);
}

osTimerId aux_det_timerid = NULL;
void aux_det_timer_onoff(bool timer_en);
void aux_det_process_timehandler(void const *param)
{
    besui_adc_aux_config();
    aux_det_timer_onoff(true);
}
osTimerDef (LINEIN_DET_TIMER_NAME, (void (*)(void const *))aux_det_process_timehandler);

void aux_det_timer_onoff(bool timer_en)
{
	//BESUI_TRACE(1, "[UIAUX]%s timer_en %d ", __func__, timer_en);

	if(aux_det_timerid == NULL)
	{
    	aux_det_timerid = osTimerCreate(osTimer(LINEIN_DET_TIMER_NAME),osTimerOnce,NULL);
        if(!aux_det_timerid)
             ASSERT(0, "[UIAUX][%s] osTimerCreate error", __func__);
	}

	if(timer_en)
		osTimerStart(aux_det_timerid, 1000);
	else
		osTimerStop(aux_det_timerid);
}


void aux_detect_init(void)
{
    BESUI_TRACE(0, "[UIAUX]%s", __func__);
    hal_iomux_init((struct HAL_IOMUX_PIN_FUNCTION_MAP *)&aux_det_cfg, 1);
    hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)aux_det_cfg.pin, HAL_GPIO_DIR_IN, 1);

    besui_adc_aux_config();
    aux_det_timer_onoff(true);
}

#endif //#ifdef AUX_DETECT_EN

void stereoui_init(void)
{
    BESUI_TRACE(0, "%s", __func__);

    app_ui_status_indication_init();
#ifdef STEREO_HALL_EN
    app_ui_charge_gpio_init();
    if (app_ui_charging_io_read((enum HAL_GPIO_PIN_T)HAL_IOMUX_PIN_P1_7)) {
        app_system_status_set(APP_STATUS_TYPE_CHARGE);
        app_ui_charge_gpio_iqr_init();
    }
#endif
#ifdef DC_DETECT_EN
    BESUI_TRACE(0,"[UIDC]%s, boot_type=0x%04X", __func__, pmu_boot_cause_get());
    charger_usb_sta_get();
    //app_battery_register(dc_det_battery_cb);
#endif
#ifdef AUX_DETECT_EN
    aux_detect_init();
#endif
}

#endif //#ifdef BESUI_STEREO_EN