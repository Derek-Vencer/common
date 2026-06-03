#ifdef BESUI_TWS_EN
#include "stdlib.h"
#include "math.h"
#include "pmu.h"
#include "twsui_comm.h"
#include "app_thread.h"
#include "tgt_hardware.h"
#include "apps.h"
#include "app_bt.h"
#include "hal_trace.h"
#include "app_ibrt_internal.h"
#include "earbud_ux_api.h"
#include "app_ui_param_config.h"
#include "bts_tws_if.h"
#include "bts_core_if.h"
#include "bts_bt_conn.h"
#include "app_audio.h"
#include "app_ibrt_customif_cmd.h"
#include "app_media_player.h"
#include "app_bt_stream.h"
#include "nvrecord_env.h"
#include "app_battery.h"

#if !defined(CHIP_BEST1501)
#include CHIP_SPECIFIC_HDR(reg_pmu)
#endif
#if !defined(CHIP_BEST1501P) && !defined(CHIP_BEST1502X)
#include CHIP_SPECIFIC_HDR(charger)
#endif
#include "app_factory.h"
#include "app_hfp.h"
#include "hfp_service.h"
#if defined(ANC_APP)
#include "app_anc.h"
#include "app_anc_sync.h"
#endif
#include "besbt.h"
#ifdef GFPS_ENABLED
#include "ble_gfps.h"
#include "gfps.h"
#endif
#include "audio_policy.h"

#ifdef BESUI_CHARGE_EN
#include "twsui_charge.h"
#endif
#ifdef BESUI_1WIRE_EN
#include "twsui_uart.h"
#endif
#ifdef BESUI_KEY_EN
#include "twsui_key.h"
#endif
#ifdef BESUI_BTMSG_EN
#include "twsui_btmsg.h"
#endif
#include "communication_svr.h"
#include "hal_bootmode.h"
#ifdef USER_APP_BLE_DIS_EN
#include "app_tota.h"
#endif
#ifdef TOTA_v2
#include "app_tota_common.h"
#endif
#include "ota_spp.h"
#include "bes_gap_api.h"
#ifdef CEVA_AUTO_ONOFF_EN
#include "app_capsensor.h"
#endif

#include "besui_common.h"


type_uicom_t uicom;

void twsui_param_clear(void)
{
    BESUI_TRACE(1,"[UICOM]%s", __func__);

    uicom.bat_curr_level = 10;
#ifdef BESUI_1WIRE_EN
    if(uicom.box_type != BOX_CMD_CASE_CLOSE)
#endif
        uicom.bat_peer_level = 10;

    uicom.bat_low_prompt_flag = false;
    uicom.bat_low_prompt_cnt = 0;
    uicom.bat_low_prompt_sta = DEFAULT_MODE;

    uicom.box_open_bat_det_flag = false;
    uicom.led_box_open_flag = false;

    uicom.led_bt_conn_flag = false;
    
    uictl.poweroff_fast_flag = false;

    app_bt_mobile_connect_info_init();
    app_bt_mobile_reconnect_info_init();
    app_bt_mobile_open_reconnect_info_init();

    app_common_mic_hfp_enc_control(true, OPEN_MASTER_SLAVE_MIC);

    app_call_varible_init();
}

void twsui_param_init(void)
{
    BESUI_TRACE(0, "[UIBOX]%s", __func__);
    memset(&uicom, 0, sizeof(uicom));

    uicom.bat_curr_level = 10;
    uicom.bat_peer_level = 10;

    uicom.bat_low_prompt_flag = false;
    uicom.bat_low_prompt_sta = DEFAULT_MODE;
    uicom.bat_low_prompt_cnt = 0;
    uicom.bat_low_led_cnt = 0;

    uicom.poweron_bat_det_flag = false;
    uicom.poweron_box_flag = false;
    uictl.poweroff_fast_flag = false;
    uicom.power_off2on_flag = false;
    
    uicom.box_type = 0xff;
    uicom.box_open_bat_det_flag = false;
    uicom.box_put_sta = UNKNOW_STATUS;

    uicom.wear_curr_sta = OUTEAR_STATUS;
    uicom.wear_peer_sta = OUTEAR_STATUS;

    uicom.key_func_onoff = false;

    uicom.bt_pairlist_clear_flag = false;

    uicom.charge_ma_val = CHARGER_CHARGE_CONSTANT_CURRENT_80MA;

    uicom.phone_pair_cnt = 0;

    uicom.led_box_open_flag = false;
    uicom.led_recover_last_flag = false;

    uicom.call_prompt_type = 0;
    uicom.call_end_prompt_type = DEFAULT_TYPE;
    uicom.call_end_type = DEFAULT_TYPE;

    uicom.call_incoming_flag = false;
    uicom.call_out_flag = false;

    uicom.ear_lr_side = UNKNOW_SIDE;

#ifdef BESUI_GAME_EN
    uicom.latency_mode = LATENCY_OFF;

    uicom.latency_delay_aac_mtu = 12;
    uicom.latency_delay_aac_us = (23000*uicom.latency_delay_aac_mtu);
//#ifdef A2DP_PLAYER_PLAYBACK_WATER_LINE
    uicom.latency_max_waterline_aac_mtu = 24;
    uicom.latency_min_waterline_aac_mtu = 0;
// #endif

    uicom.latency_delay_sbc_mtu = 45;
    uicom.latency_delay_sbc_frame_mtu = 7;
    uicom.latency_delay_sbc_us = ((2800)*uicom.latency_delay_sbc_mtu);
//#ifdef A2DP_PLAYER_PLAYBACK_WATER_LINE
    uicom.latency_max_waterline_sbc_mtu = 25*uicom.latency_delay_sbc_frame_mtu;//A2DP_PLAYER_PLAYBACK_DELAY_SBC_FRAME_MTU;
    uicom.latency_min_waterline_sbc_mtu = 0*uicom.latency_delay_sbc_frame_mtu;//A2DP_PLAYER_PLAYBACK_DELAY_SBC_FRAME_MTU;
// #endif
#endif

#ifdef BESUI_1WIRE_EN
    memset(&uiuart, 0, sizeof(uiuart));
    uiuart.event_type = 0xFF;
#endif
}

//------------------------------------------------------------------------------------------
//besui common timer
/*
uint8_t comm_timer_type = 0;
osTimerId comm_timer_id = NULL;
void besui_comm_timer_set_type(uint8_t param)
{
    comm_timer_type = param;
}
uint8_t besui_comm_timer_get_type(void)
{
    return comm_timer_type;
}

void besui_comm_timer_handler(void const *param)
{
    uint8_t conn_devices = besui_get_profile_conn_num();
	BESUI_TRACE(0, "[UITIMER]%s, conn_devices = %d", __func__, conn_devices);

    //timer operate

    osTimerDelete(comm_timer_id);
    comm_timer_id = NULL;
}

osTimerDef (BESUI_COMM_TIMER_NAME, (void (*)(void const *))besui_comm_timer_handler);
void besui_comm_timer_onoff(bool timer_en, uint32_t param)
{
 	BESUI_TRACE(1, "[UITIMER]%s, %d ", __func__, timer_en);
 
 	if(comm_timer_id == NULL)
 	{
     	comm_timer_id = osTimerCreate(osTimer(BESUI_COMM_TIMER_NAME), osTimerOnce, NULL);
 	}

 	if(timer_en)
        osTimerStart(comm_timer_id, param);
 	else
 		osTimerStop(comm_timer_id);
}
*/
//------------------------------------------------------------------------------------------

bool app_open_hfp_enc_flag = true;
uint8_t app_open_mic_type = OPEN_MASTER_SLAVE_MIC;
void app_common_mic_hfp_enc_control(bool enc_open_flag, uint8_t open_mic_type)
{
    app_open_hfp_enc_flag = enc_open_flag;
    app_open_mic_type = open_mic_type;
}

bool app_get_mic_hfp_enc_onoff(void)
{
    return app_open_hfp_enc_flag;
}

uint8_t app_get_mic_hfp_enc_test_type(void)
{
    return app_open_mic_type;
}

#if defined(ANC_APP)
uint8_t anc_mic_sta = 0;
void anc_ff_fb_set_onoff(uint8_t sta)
{
    anc_mic_sta = sta;
    if(app_anc_get_curr_mode() != APP_ANC_MODE_OFF)
    {
        static app_anc_mode_t anc_buf = app_anc_get_curr_mode();
        app_anc_switch(APP_ANC_MODE_OFF);
        app_anc_switch(anc_buf);
    }
}
uint8_t anc_ff_fb_get_onoff(void)
{
    return anc_mic_sta;
}
#endif

uint8_t speech_mic_sta = 0;
void speech_mic_set_onoff(uint8_t sta)
{
    BESUI_TRACE(1,"[UIMIC]%s, %d", __func__, speech_mic_sta);
    speech_mic_sta = sta;
    app_audio_sendrequest(APP_BT_STREAM_HFP_PCM, (uint8_t)APP_BT_SETTING_RESTART, 0);
}
uint8_t speech_mic_get_onoff(void)
{
    BESUI_TRACE(1,"[UIMIC]%s, %d", __func__, speech_mic_sta);
    return speech_mic_sta;
}


#ifdef BESUI_LR_IODET_EN
void besui_fixed_lr_det_init(void)
{
    uint8_t high_level_cnt = 0;
    uint8_t low_level_cnt = 0;

    if (app_fixed_left_right_side_cfg.pin != HAL_IOMUX_PIN_NUM)
    {
        hal_iomux_init((struct HAL_IOMUX_PIN_FUNCTION_MAP *)&app_fixed_left_right_side_cfg, 1);
        hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)app_fixed_left_right_side_cfg.pin, HAL_GPIO_DIR_IN, 1);
    
        while(1)
        {
            if (hal_gpio_pin_get_val((enum HAL_GPIO_PIN_T)app_fixed_left_right_side_cfg.pin))
            {
                high_level_cnt ++;
                low_level_cnt = 0;
            }
            else
            {
                low_level_cnt ++;
                high_level_cnt = 0;
            }

            if((high_level_cnt >= 5) || (low_level_cnt >= 5))
                break;
            osDelay(10);
        }

        if(high_level_cnt >= 5)
            uicom.ear_lr_side = LEFT_SIDE;
        else if(low_level_cnt >= 5)
            uicom.ear_lr_side = RIGHT_SIDE;

        BESUI_TRACE(0, "[UILR]%s, uicom.ear_lr_side %d", __func__, uicom.ear_lr_side);
    }

    // uicom.ear_lr_side = 1 - uicom.ear_lr_side;

#if defined(USER_BURN_ADDR_PAIR_EN)
    uint8_t *bt_local_addr;
    bt_local_addr = (uint8_t *)bt_get_local_address();
    if((bt_local_addr[0]%2) == 0)
        uicom.ear_lr_side = RIGHT_SIDE;
    else
        uicom.ear_lr_side = LEFT_SIDE;

    BESUI_TRACE(0, "[UILR]%s, uicom.ear_lr_side %d", __func__, uicom.ear_lr_side);
#endif
}
#endif

uint8_t besui_get_lr_sta(void)
{
    return uicom.ear_lr_side;
}

//get tws pairlist
bool app_ibrt_get_history_paired_device(void)
{
    bt_status_t            retStatus;
    btif_device_record_t   record;
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    int                    paired_dev_count = nv_record_get_paired_dev_count(); //7 phone record+1 tws record

    BESUI_TRACE(0,"[UIPAIRLIST]%s, paired_dev_count=%d", __func__, paired_dev_count);
    BESUI_TRACE(0,"[UIPAIRLIST]Master addr:");
    DUMP8("%02x ",p_ibrt_ctrl->local_addr.address, BT_ADDR_OUTPUT_PRINT_NUM);
    BESUI_TRACE(0,"[UIPAIRLIST]Slave addr:");
    DUMP8("%02x ",p_ibrt_ctrl->peer_addr.address, BT_ADDR_OUTPUT_PRINT_NUM);

    if(paired_dev_count <= MAX_BT_PAIRED_DEVICE_COUNT)
    {
        for (int32_t index = paired_dev_count - 1; index >= 0; index--)
        {
            retStatus = nv_record_enum_dev_records(index, &record);
            if (BT_STS_SUCCESS == retStatus)
            {
                BESUI_TRACE(0,"[UIPAIRLIST]The index %d of nv records:", index);
                DUMP8("%02x ", record.bdAddr.address, BT_ADDR_OUTPUT_PRINT_NUM);
                if (!memcmp(record.bdAddr.address, p_ibrt_ctrl->local_addr.address, BTIF_BD_ADDR_SIZE) ||
                    !memcmp(record.bdAddr.address, p_ibrt_ctrl->peer_addr.address, BTIF_BD_ADDR_SIZE))
                {
                    BESUI_TRACE(0,"[UIPAIRLIST]have tws pairlist");
                    return true;
                }
            }
        }
    }

    BESUI_TRACE(0,"[UIPAIRLIST]have no tws pairlist");
    return false;
}

bool app_get_history_phone_paired_device(void) //get phone pairlist
{
    bt_status_t            retStatus;
    btif_device_record_t   record;
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    int                    paired_dev_count = nv_record_get_paired_dev_count();
    uint8_t address_compare[6] = {0};

    BESUI_TRACE(0,"[UIPAIRLIST]%s, paired_dev_count = %d", __func__, paired_dev_count);
    BESUI_TRACE(0,"[UIPAIRLIST]Master addr:");
    DUMP8("%02x ",p_ibrt_ctrl->local_addr.address, BTIF_BD_ADDR_SIZE);
    BESUI_TRACE(0,"[UIPAIRLIST]Slave addr:");
    DUMP8("%02x ",p_ibrt_ctrl->peer_addr.address, BTIF_BD_ADDR_SIZE);

    if(paired_dev_count < MAX_BT_PAIRED_DEVICE_COUNT)
    {
        for (int32_t index = paired_dev_count - 1; index >= 0; index--)
        {
            retStatus = nv_record_enum_dev_records(index, &record);
            if (BT_STS_SUCCESS == retStatus)
            {
                BESUI_TRACE(0,"[UIPAIRLIST]The index %d of nv records:", index);
                DUMP8("%02x ", record.bdAddr.address, BTIF_BD_ADDR_SIZE);
                if (memcmp(record.bdAddr.address, p_ibrt_ctrl->local_addr.address, BTIF_BD_ADDR_SIZE) &&
                    memcmp(record.bdAddr.address, p_ibrt_ctrl->peer_addr.address, BTIF_BD_ADDR_SIZE)&&
                    memcmp(record.bdAddr.address, address_compare, BTIF_BD_ADDR_SIZE))
                {
                    BESUI_TRACE(0,"[UIPAIRLIST]have phone paired list");
                    return true;
                }
            }
        }
    }

	BESUI_TRACE(0,"[UIPAIRLIST]have no phone paired list");
	return false;
}

void app_get_history_phone_paired_num(void)
{
    bt_status_t            retStatus;
    btif_device_record_t   record;
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    int                    paired_dev_count = nv_record_get_paired_dev_count();
    uint8_t address_compare[6] = {0};
    uint8_t temp_index = 0;
    bool tws_flag = false;

    uicom.phone_pair_cnt = 0;

    BESUI_TRACE(0,"[UICOM]%s, paired_dev_count = %d", __func__, paired_dev_count);
    BESUI_TRACE(0,"[UICOM]Master addr:");
    DUMP8("%02x ",p_ibrt_ctrl->local_addr.address, BTIF_BD_ADDR_SIZE);
    BESUI_TRACE(0,"[UICOM]Slave addr:");
    DUMP8("%02x ",p_ibrt_ctrl->peer_addr.address, BTIF_BD_ADDR_SIZE);

    if(paired_dev_count > MAX_BT_PAIRED_DEVICE_COUNT)
    {
        return;
    }

    for (int32_t index = paired_dev_count - 1; index >= 0; index--)
    {
        retStatus = nv_record_enum_dev_records(index, &record);
        if (BT_STS_SUCCESS == retStatus)
        {
            BESUI_TRACE(1,"[UICOM]The index %d of nv records:", index);
            DUMP8("%02x ", record.bdAddr.address, BTIF_BD_ADDR_SIZE);
            if (memcmp(record.bdAddr.address, p_ibrt_ctrl->local_addr.address, BTIF_BD_ADDR_SIZE) &&
                memcmp(record.bdAddr.address, p_ibrt_ctrl->peer_addr.address, BTIF_BD_ADDR_SIZE)&&
                memcmp(record.bdAddr.address, address_compare, BTIF_BD_ADDR_SIZE))
            {
            	BESUI_TRACE(0,"[UICOM]have phone paired list");
                uicom.phone_pair_cnt++;
                BESUI_TRACE(1,"[UICOM]pairlist_count %d", uicom.phone_pair_cnt);
                /*************get true addr and set device id***************/
                if(paired_dev_count >= 3)
                {
                    if(tws_flag)
                    {
                        if(index > 1)
                            temp_index = index-1;
                        else
                            temp_index = index;
                    }
                    else
                    {
                        if(index > 0)
                            temp_index = index-1;
                        else
                            temp_index = index;
                    }
                }
                else
                {
                    if(index > 0)
                        temp_index = index-1;
                    else
                        temp_index = index;
                }
                app_bt_set_mobile_reconnect_addr(temp_index, record.bdAddr.address);
            }
            else if(!memcmp(record.bdAddr.address, p_ibrt_ctrl->local_addr.address, BTIF_BD_ADDR_SIZE)||
            !memcmp(record.bdAddr.address, p_ibrt_ctrl->peer_addr.address, BTIF_BD_ADDR_SIZE))
            {
                tws_flag = true;
            }
        }
    }
}

void app_clear_tws_pairlist_process(void) //clear tws pairlist
{
    bt_status_t            retStatus;
    btif_device_record_t   record;
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    int                    paired_dev_count = nv_record_get_paired_dev_count();

    BESUI_TRACE(0,"[UICOM]app_clear_tws_pairlist_process");
    BESUI_TRACE(0,"[UICOM]Master addr:");
    DUMP8("%02x ",p_ibrt_ctrl->local_addr.address, BT_ADDR_OUTPUT_PRINT_NUM);
    BESUI_TRACE(0,"[UICOM]Slave addr:");
    DUMP8("%02x ",p_ibrt_ctrl->peer_addr.address, BT_ADDR_OUTPUT_PRINT_NUM);

    for (int32_t index = paired_dev_count - 1; index >= 0; index--)
    {
        retStatus = nv_record_enum_dev_records(index, &record);
        if (BT_STS_SUCCESS == retStatus)
        {
            BESUI_TRACE(1,"[UICOM]The index %d of nv records:", index);
            DUMP8("%02x ", record.bdAddr.address, BT_ADDR_OUTPUT_PRINT_NUM);
            if (!memcmp(record.bdAddr.address, p_ibrt_ctrl->local_addr.address, BTIF_BD_ADDR_SIZE) ||
                !memcmp(record.bdAddr.address, p_ibrt_ctrl->peer_addr.address, BTIF_BD_ADDR_SIZE))
            {
                BESUI_TRACE(0,"[UICOM]have tws paired list, delete it");
                nv_record_ddbrec_delete(&record.bdAddr);
                memset(record.linkKey, 0, 16);
            }
        }
    }

    memset(p_ibrt_ctrl->local_addr.address, 0, BTIF_BD_ADDR_SIZE);
    memset(p_ibrt_ctrl->peer_addr.address, 0, BTIF_BD_ADDR_SIZE);
}

void app_clear_phone_pairlist_process(void) //clear phone pairlist
{
    bt_status_t            retStatus;
    btif_device_record_t   record;
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    int                    paired_dev_count = nv_record_get_paired_dev_count();
    uint8_t address_compare[6] = {0};

    BESUI_TRACE(0,"[UICOM]app_clear_phone_pairlist_process");
    BESUI_TRACE(0,"[UICOM]Master addr:");
    DUMP8("%02x ",p_ibrt_ctrl->local_addr.address, BTIF_BD_ADDR_SIZE);
    BESUI_TRACE(0,"[UICOM]Slave addr:");
    DUMP8("%02x ",p_ibrt_ctrl->peer_addr.address, BTIF_BD_ADDR_SIZE);

    for (int32_t index = paired_dev_count - 1; index >= 0; index--)
    {
        retStatus = nv_record_enum_dev_records(index, &record);
        if (BT_STS_SUCCESS == retStatus)
        {
            BESUI_TRACE(1,"[UICOM]The index %d of nv records:", index);
            DUMP8("%02x ", record.bdAddr.address, BTIF_BD_ADDR_SIZE);
            if (memcmp(record.bdAddr.address, p_ibrt_ctrl->local_addr.address, BTIF_BD_ADDR_SIZE) &&
                memcmp(record.bdAddr.address, p_ibrt_ctrl->peer_addr.address, BTIF_BD_ADDR_SIZE)&&
                memcmp(record.bdAddr.address, address_compare, BTIF_BD_ADDR_SIZE))
            {
            	BESUI_TRACE(0,"[UICOM]clear phone paired list");
            	nv_record_ddbrec_delete(&record.bdAddr);
                memset(record.linkKey, 0, 16);
            }
        }
    }
}

uint8_t app_battery_level_tran_process(uint16_t battery_volt)
{
    static uint8_t first_det = 1;
    static uint8_t batt_last = 10;
    uint8_t level_buf = 10;

    uictl.run_time_10s ++;
    BESUI_TRACE(0, "[UISYS]%s, run_time_10s = %d", __func__, uictl.run_time_10s);
#ifdef USER_EXT_VOLTAGE_DET_EN
    besui_ext_voltage_adc_config();
#endif

    if(battery_volt < 2800) //error
    {
        BESUI_TRACE(2,"[UIBAT]%s, batteryerror volt=%d", __func__, battery_volt);
        return 10;
    }

    const uint16_t bat_tab[9] = {3500, 3600, 3650, 3700, 3750, 3800, 3850, 3950, 4050};

    level_buf = 0;
    for(uint8_t i = 9;i > 0;i --)
    {
        if(battery_volt > bat_tab[i-1])
        {
            level_buf = i;
            break;
        }
    }
    BESUI_TRACE(0, "[UIBAT]%s, volt = %d, cur = %d, last = %d", __func__, battery_volt, level_buf, batt_last);

    if(uictl.case_open_flag == true)
    {
        uictl.case_open_flag = false;
        batt_last = level_buf;
        uicom.bat_curr_level = level_buf;
        app_ibrt_customif_cmd_sync_battery_level(twsui_get_bat_level());
        BESUI_TRACE(0,"[UIBAT]%s, case_open update!!!", __func__);
        goto CASE_OPEN_UPDATE;
    }

    APP_BATTERY_STATUS_T charge_sta;
    app_battery_get_info(NULL, NULL, &charge_sta);
    BESUI_TRACE(0,"[UIBAT]%s, charge_sta = %d", __func__, charge_sta);

#if defined(CHARGE_FULL_POWEROFF_EN)
    #define BAT_FULL_TIMEOUT        (6*60*1) //1 hour
    static uint16_t bat_timeout = 0;
    if(charge_sta == APP_BATTERY_STATUS_CHARGING)
    {
        if(battery_volt > 4100) //>4.10V full
        {
            bat_timeout ++;
            BESUI_TRACE(0, "[UIBAT]%s, bat_timeout = %d", __func__, bat_timeout);

            if(bat_timeout >= BAT_FULL_TIMEOUT && uictl.shutdown_type != SHUTDOWN_BAT_CHARGE_FULL)
            {
                bat_timeout = 0;
                BESUI_TRACE(0, "[UIBAT]%s, bat_volt = %d, full_poweroff!!!", __func__, battery_volt);
                //charger_param_set_onoff(false, uicom.charge_ma_val); //fast no voltage
                uictl.poweroff_fast_flag = true;
                uictl.shutdown_type = SHUTDOWN_BAT_CHARGE_FULL;
#if defined(CHIP_BEST1306)
                if (hal_get_chip_metal_id() >= HAL_CHIP_METAL_ID_2) //1306 C version support hardware shipmode
                {
                    BESUI_TRACE(0, "[UIBAT]%s, metal_id = %d,hardware shipmode", __func__, hal_get_chip_metal_id());
                    pmu_shipment_mode_enable(true);
                }
                else
#endif
                {
                    uicom.ship_mode_flag = true; //1306 software shipmode,need hardware gpio control enter shipmode
                }
                app_shutdown();
            }
        }
        else
        {
            bat_timeout = 0;
        }
    }
    else
    {
        bat_timeout = 0;
    }
#endif

    if(charge_sta == APP_BATTERY_STATUS_CHARGING)
    {
        if(first_det)
            batt_last = 0;
        if(level_buf != batt_last)
        {
            if(level_buf > batt_last)
            {
                batt_last = level_buf;
                uicom.bat_curr_level = level_buf;
                app_ibrt_customif_cmd_sync_battery_level(twsui_get_bat_level());
            }
            else
            {
                uicom.bat_curr_level = batt_last;
            }
        }
        else
        {
            if(level_buf != 10)
            {
                uicom.bat_curr_level = level_buf;
            }
        }     
    }
    else
    {
        if(level_buf != batt_last)
        {
            if(level_buf < batt_last)
            {
                batt_last = level_buf;
                uicom.bat_curr_level = level_buf;
                app_ibrt_customif_cmd_sync_battery_level(twsui_get_bat_level());
            }
            else
            {
                uicom.bat_curr_level = batt_last;
            }
        }
        else
        {
            if(level_buf != 10)
            {
                uicom.bat_curr_level = level_buf;
            }
        }        
    }
    first_det = 0;
CASE_OPEN_UPDATE:
    BESUI_TRACE(0,"[UIBAT]%s, bat_curr_level=%d, charge_sta=%d", __func__, uicom.bat_curr_level, charge_sta);
    return uicom.bat_curr_level;
}

uint8_t twsui_get_bat_level(void)
{
    uint8_t batflag = uicom.bat_curr_level;
    if(batflag > 9)
    {
        batflag = app_battery_current_level();
    }
    if(batflag > 9)
    {
        batflag = 9;
    }
    return batflag;
}

void app_battery_low_voice_enable_set(uint8_t battery_level)
{
    BESUI_TRACE(3,"[UICOM]%s level %d flag %d status %d", __func__, battery_level, uicom.bat_low_prompt_flag, uicom.bat_low_prompt_sta);
    if(battery_level < 1)
    {
        if(!uicom.bat_low_prompt_flag)
        {
            uicom.bat_low_prompt_flag = true;
        }

        if(uicom.bat_low_prompt_sta == DEFAULT_MODE)
        {
            uicom.bat_low_prompt_cnt = 0;
            uicom.bat_low_led_cnt = 0;
            uicom.bat_low_prompt_sta = VOICE_PLAY_START;
        }
    }
}

uint8_t app_battery_level_compare(void)
{
    BESUI_TRACE(1,"[UICOM]%s, current = %d, peer = %d", __func__, twsui_get_bat_level(), uicom.bat_peer_level);
    if(bts_tws_if_is_tws_link_connected())
    {
        if(TWS_UI_SLAVE != app_ibrt_if_get_ui_role())
        {
            if(twsui_get_bat_level() > uicom.bat_peer_level)
                return uicom.bat_peer_level;
            else
                return twsui_get_bat_level();
        }
    }
    else
    {
        return twsui_get_bat_level();
    }

    return 0xff;
}

void app_battery_low_voice_play_process(void)
{
    BESUI_TRACE(1,"[UICOM]%s voice_flag %d voice_sta %d voice_cnt %d", __func__, uicom.bat_low_prompt_flag, 
        uicom.bat_low_prompt_sta, uicom.bat_low_prompt_cnt);

    if((uicom.bat_low_prompt_flag)&&(uicom.bat_low_prompt_sta == VOICE_PLAY_START))
    {
        uicom.bat_low_prompt_cnt++;        
        if(uicom.bat_low_prompt_cnt == 1)
        {
            media_PlayAudio_single_play(AUD_ID_BT_CHARGE_PLEASE, 0);
        }
        if(uicom.bat_low_prompt_cnt >= BAT_LOW_PROMPT_CNT)
            uicom.bat_low_prompt_cnt = 0;

        if(uicom.bat_low_led_cnt >= LED_DISPLAY_SPACE)
            uicom.bat_low_led_cnt = 0;
    }
}

#ifdef BATTERY_SWITCH_ROLE_EN
void besui_battery_role_switch(void) //per 10s
{
    #define SWITCH_ROLE_TIMEOUT         (6*5)
    static uint16_t switch_cnt = 0;
    if(switch_cnt < SWITCH_ROLE_TIMEOUT)
    {
        switch_cnt ++;
        return;
    }
    switch_cnt = 0;

    bud_box_state  local_box_state = app_ui_get_local_box_state();
    bud_box_state  peer_box_state = app_ui_get_peer_box_state();
    // app_ui_config_t* p_app_ui_config = app_ui_get_config();
    uint8_t conn_devices = besui_get_profile_conn_num();

    // int8_t rssi_d_value_diff = 0;
    // rssi_d_value_diff = abs(abs(uictl.peer_rssi) - abs(uictl.local_rssi));
    if(conn_devices == 0)
        return;

    // BESUI_TRACE(3,"[UICOM]ibrt_ui_log:tws rssi %d %d %d", uictl.local_rssi, uictl.peer_rssi, rssi_d_value_diff);    
    // if(app_bt_audio_count_connected_sco())
    //     return;

    if(uictl.role_switch_no) //forbid role switch
    {
        BESUI_TRACE(0,"[UICOM]%s, Wait bt msg process end...", __func__);
        switch_cnt = SWITCH_ROLE_TIMEOUT-1;
        return;
    }
    if((bts_tws_if_is_tws_link_connected())&&(TWS_UI_MASTER == app_ibrt_if_get_ui_role()))
    {
        BESUI_TRACE(2,"[UICOM]%s cur level %d other %d", __func__, twsui_get_bat_level(), uicom.bat_peer_level);
        
        if((twsui_get_bat_level() != 10)&&(uicom.bat_peer_level != 10))
        {
            if(twsui_get_bat_level() < uicom.bat_peer_level)
            {
                if((uicom.bat_peer_level - twsui_get_bat_level()) >= 1)
                {
                    // if(rssi_d_value_diff < p_app_ui_config->rssi_threshold) //rssi ok
                    {
                        BESUI_TRACE(2,"[UICOM]%s local_box_state %d peer_box_state %d", __func__, local_box_state, peer_box_state);
                        if((local_box_state == IBRT_OUT_BOX)&&(peer_box_state == IBRT_OUT_BOX))
                        {
                            BESUI_TRACE(0,"[UICOM]rssi is ok, battery switch role");
                            app_ui_user_role_switch(false);
                        }
                    }
                }
            }
        }
    }
}
#endif

void app_battery_set_other_battery_level(uint8_t battery_level)
{
    uicom.bat_peer_level = battery_level;
    BESUI_TRACE(1,"[UICOM]%s battery_level %d", __func__, uicom.bat_peer_level);
}

void app_tws_battery_update(bool tws_connect_flag)
{
    uint8_t conn_devices = besui_get_profile_conn_num();
    uint8_t level = 0;
    BESUI_TRACE(1,"[UICOM]%s tws_connect_flag %d", __func__, tws_connect_flag);
    app_hfp_battery_report_reset(BT_DEVICE_ID_1);
#if (BT_DEVICE_NUM > 1)
    app_hfp_battery_report_reset(BT_DEVICE_ID_2);
#endif
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
}

#ifdef GFPS_ENABLED
uint8_t app_gfps_renew_battery_level(uint8_t charge_status, uint8_t *batteryLevel)
{
    uint8_t count = 3;
	BESUI_TRACE(3,"[UICOM]%s, bat_cur=%d, bat_other=%d", __func__, app_battery_current_level(), uicom.bat_peer_level);
    if(bts_tws_if_is_tws_link_connected())
    {
        if(besui_get_lr_sta() == LEFT_SIDE)
        {
            batteryLevel[0] = ((app_battery_current_level()+1) * 10) | (charge_status << 7);
			
			if(uicom.bat_peer_level != 10)
			{
				batteryLevel[1] = ((uicom.bat_peer_level+1) * 10) | (charge_status << 7);
			}
			else
			{
				batteryLevel[1] = 0x7F;
			}
        }
        else if(besui_get_lr_sta() == RIGHT_SIDE)
        {
            batteryLevel[1] = ((app_battery_current_level()+1) * 10) | (charge_status << 7);
			
			if(uicom.bat_peer_level != 10)
			{
				batteryLevel[0] = ((uicom.bat_peer_level+1) * 10) | (charge_status << 7);
			}
			else
			{
				batteryLevel[0] = 0x7F;
			}
        }
    }
    else
    {
        if(besui_get_lr_sta() == LEFT_SIDE)
		{
			batteryLevel[0] = ((app_battery_current_level()+1) * 10) | (charge_status << 7);
			batteryLevel[1] = 0x7F;
		}
		else if(besui_get_lr_sta() == RIGHT_SIDE)
		{
			batteryLevel[0] = 0x7F;
			batteryLevel[1] = ((app_battery_current_level()+1) * 10) | (charge_status << 7);
		}
    }

    if(uicom.bat_box_percent != 0xff)
    {
        batteryLevel[2] = (uicom.bat_box_percent) | (charge_status << 7);
    }
    else
    {
		batteryLevel[2] = 0x7F;
    }

    if(batteryLevel[2] == 0)
    {
        batteryLevel[2] = 0x7F;
        BESUI_TRACE(0,"[UICOM]%s, box bat level error, set 0x7F", __func__);
    }

#if 1
    static uint8_t batbuf[3] = {0};
    if(memcmp(batbuf, batteryLevel, 3) && TWS_UI_SLAVE != app_ibrt_if_get_ui_role())
    {
        memcpy(batbuf, batteryLevel, 3);
        bes_ble_gap_refresh_adv_state(BLE_ADVERTISING_INTERVAL);
        BESUI_TRACE(0,"[UIGFPS]%s, bat change, refresh adv", __func__);
    }
#endif

	BESUI_TRACE(0,"[UICOM]%s,count=%d,gfps = %d,%d,%d", __func__, count,batteryLevel[0],batteryLevel[1],batteryLevel[2]);
    return count;
}
#endif

static int app_poweroff_shutdown_ui_handle_process(APP_MESSAGE_BODY *msg_body);

void app_poweroff_shutdown_ui_modual_init(void)
{
    app_set_threadhandle(APP_MODUAL_POWEROFF_SHUTDOWN_UI, app_poweroff_shutdown_ui_handle_process);
}

void app_poweroff_shutdown_ui_post_msg(bool slave_key_flag, uint8_t poweroff_type)
{
    BESUI_TRACE(2,"[UICOM]%s slave_key_flag: %d  poweroff_type %d", __func__, slave_key_flag, poweroff_type);

    APP_MESSAGE_BLOCK msg;

    msg.mod_id = APP_MODUAL_POWEROFF_SHUTDOWN_UI;

    msg.msg_body.message_id = poweroff_type;
    msg.msg_body.message_Param0 = slave_key_flag;

    app_mailbox_put(&msg);
}

static int app_poweroff_shutdown_ui_handle_process(APP_MESSAGE_BODY *msg_body)
{
    bool slave_key_flag = (bool)(msg_body->message_Param0);
    uint8_t poweroff_type = (uint8_t)(msg_body->message_id);

    app_common_poweroff_process(slave_key_flag, poweroff_type);

    return 0;
}

osTimerId app_after_switch_role_poweroff_process_timer = NULL;

void app_switch_role_poweroff_timehandler(void const *param)
{
	BESUI_TRACE(1, "[UITIMER]%s", __func__);
#ifdef BESUI_COMM_EN
    uictl.poweroff_start_flag = 0;
    BESUI_TRACE(1,"[UIAPP]%s, uictl.poweroff_start_flag2 = %d",__func__, uictl.poweroff_start_flag);
#endif

#ifdef USER_APP_BLE_DIS_EN
    app_tota_switch_role_to_app();
#endif

    uictl.shutdown_type = SHUTDOWN_SWITCH_ROLE;
    app_ui_shutdown();
}

osTimerDef (APP_AFTER_SWITCH_ROLE_POWEROFF_PROCESS_TIMER, (void (*)(void const *))app_switch_role_poweroff_timehandler);

void app_switch_role_poweroff_timer_onoff(bool timer_en)
{
	BESUI_TRACE(1, "[UITIMER]%s timer_en %d ", __func__, timer_en);

	if(app_after_switch_role_poweroff_process_timer == NULL)
	{
    	app_after_switch_role_poweroff_process_timer = osTimerCreate(osTimer(APP_AFTER_SWITCH_ROLE_POWEROFF_PROCESS_TIMER),osTimerOnce,NULL);
        if(!app_after_switch_role_poweroff_process_timer)
        {
             ASSERT(0, "[UITIMER][%s] osTimerCreate error", __func__);
        }
	}

	if(timer_en)
	{
		osTimerStart(app_after_switch_role_poweroff_process_timer, 800);
	}
	else
	{
		osTimerStop(app_after_switch_role_poweroff_process_timer);
	}
}


void app_common_poweroff_process(bool slave_key_flag, uint8_t poweroff_type)
{
    uint8_t conn_devices = besui_get_profile_conn_num();

    BESUI_TRACE(4,"[UICOM]%s tws con %d slave key %d poweroff type %d conn_devices %d", __func__, 
        bts_tws_if_is_tws_link_connected(), slave_key_flag, poweroff_type, conn_devices);

    if(bts_tws_if_is_tws_link_connected())
    {
        if(conn_devices > 0)
        {
            if(AUTO_POWEROFF == poweroff_type)
            {
                //nothing
            }
            else if(LOWBATTERY_POWEROFF == poweroff_type)
            {
                uictl.shutdown_type = SHUTDOWN_BAT_LOW;
                if(TWS_UI_MASTER == app_ibrt_if_get_ui_role())
                {
                    app_switch_role_poweroff_timer_onoff(true); //delay poweroff
                }
                else
                {
                    app_shutdown();
                }
            }
            else if(KEY_FUNCTION_POWEROFF == poweroff_type)
            {
                if(slave_key_flag)
                {
                    app_ibrt_customif_cmd_sync_poweroff_shutdown(true);
                }
                else
                {
                    uictl.shutdown_type = SHUTDOWN_TOUCH_PRESS;
                    app_switch_role_poweroff_timer_onoff(true); //delay_ poweroff
                }
            }
            else if(CLEARPAIRLIST_POWEROFF == poweroff_type)
            {
                uictl.shutdown_type = SHUTDOWN_CLEAR_PAIR;
                app_shutdown();
            }
        }
        else
        {
            if(CLEARPAIRLIST_POWEROFF == poweroff_type)
            {
                uictl.shutdown_type = SHUTDOWN_CLEAR_PAIR;
                app_shutdown();
            }
            else
            {
                //sync shutdown
                if(KEY_FUNCTION_POWEROFF == poweroff_type)
                {
                    if(slave_key_flag)
                    {
                        app_ibrt_customif_cmd_sync_poweroff_shutdown(true);
                    }
                    else
                    {
                        uictl.shutdown_type = SHUTDOWN_TOUCH_PRESS;
                        app_shutdown();
                    }
                }
                else if(LOWBATTERY_POWEROFF == poweroff_type)
                {
                    uictl.shutdown_type = SHUTDOWN_BAT_LOW;
                    app_shutdown();
                }
                else
                {
                    app_ibrt_customif_cmd_sync_poweroff_shutdown(true);
                    osDelay(150);
                    uictl.shutdown_type = SHUTDOWN_RECONNECT_TIMEOUT;
                    app_shutdown();
                }
            }
        }
    }
    else
    {
        uictl.shutdown_type = SHUTDOWN_DEFAULT;
        app_shutdown();
    }
}

#ifdef BESUI_GAME_EN
#ifdef BESUI_GAME_NV_EN
void app_gamemode_nv_write(uint8_t param)
{
    nv_record_env_get(&nvrecord_uienv);
    nvrecord_uienv->remember_game_mode = param;
    nv_record_env_set(nvrecord_uienv);
}
#endif

void app_gamemode_off(bool param)
{
    uicom.latency_mode = LATENCY_OFF;
    if(param)
        app_gamemode_set_param(uicom.latency_mode);
}

void app_gamemode_key_switch(bool switch_flag, uint8_t game_mode)
{
    if(switch_flag)
    {
        uicom.latency_mode++;
        if(uicom.latency_mode > LATENCY_ON)
        {
            uicom.latency_mode = LATENCY_OFF;
        }
    }
    else
    {
        uicom.latency_mode = game_mode;
    }
}

uint8_t besui_latency_mode_get(void)
{
    return uicom.latency_mode;
}

void app_gamemode_set_param(uint8_t gamemode_type)
{
    static uint8_t modebuf = 0xFF;
    if(modebuf == gamemode_type)
        return;
    modebuf = gamemode_type;

    if(gamemode_type == LATENCY_OFF)
    {
        uicom.latency_delay_sbc_frame_mtu = 7;
        uicom.latency_delay_sbc_mtu = 35;
        uicom.latency_max_waterline_sbc_mtu = 25*uicom.latency_delay_sbc_frame_mtu;
        uicom.latency_min_waterline_sbc_mtu = 0*uicom.latency_delay_sbc_frame_mtu;
        uicom.latency_delay_sbc_us = ((2800)*uicom.latency_delay_sbc_mtu);

        uicom.latency_delay_aac_mtu = 12;
        uicom.latency_max_waterline_aac_mtu = 24;
        uicom.latency_min_waterline_aac_mtu = 0;
        uicom.latency_delay_aac_us = (23000*uicom.latency_delay_aac_mtu);
    }
    else if(gamemode_type == LATENCY_ON)
    {
        uicom.latency_delay_sbc_frame_mtu = 0;
        uicom.latency_delay_sbc_mtu = 15; //7-92ms, 15-180ms
        uicom.latency_max_waterline_sbc_mtu = 25*uicom.latency_delay_sbc_frame_mtu;
        uicom.latency_min_waterline_sbc_mtu = 0*uicom.latency_delay_sbc_frame_mtu;
        uicom.latency_delay_sbc_us = ((2800)*uicom.latency_delay_sbc_mtu);

        uicom.latency_delay_aac_mtu = 3;
        uicom.latency_max_waterline_aac_mtu = 24;
        uicom.latency_min_waterline_aac_mtu = 0;
        uicom.latency_delay_aac_us = (23000*uicom.latency_delay_aac_mtu);
    }
    app_ibrt_if_force_audio_retrigger(RETRIGGER_BY_STREAM_RESTART); //pop
    BESUI_TRACE(4,"[UICOM]%s,gamemode_type = %d, sbc_frame_mtu=%d, sbc_mtu=%d", __func__, 
    gamemode_type, uicom.latency_delay_sbc_frame_mtu, uicom.latency_delay_sbc_mtu);
}

bool onoff_buckups = false;
uint8_t next_mode_backups = 0;
void besui_gamemode_param_backups(bool onoff, uint8_t next_mode)
{
    onoff_buckups = onoff;
    next_mode_backups = next_mode;
}

void besui_gamemode_param_set(bool onoff, uint8_t next_mode)
{
    if(TWS_UI_SLAVE == app_ibrt_if_get_ui_role())
    {
        app_gamemode_key_switch(false, next_mode);
#ifdef BESUI_GAME_NV_EN
        app_gamemode_nv_write(next_mode);
#endif
        app_gamemode_set_param(next_mode);
    }
    else //if(TWS_UI_MASTER == app_ibrt_if_get_ui_role())
    {
        app_gamemode_key_switch(onoff, next_mode);
        app_gamemode_set_param(uicom.latency_mode);
#ifdef BESUI_GAME_NV_EN
        app_gamemode_nv_write(uicom.latency_mode);
#endif
        app_ibrt_customif_cmd_sync_game_mode(onoff, uicom.latency_mode);

#ifdef APP_SYNC_GAME_EN
        app_sync_earbuds_latency(besui_latency_mode_get());
#endif
    }
}
void besui_gamemode_key_switch(bool onoff, uint8_t next_mode, bool voice_flag)
{
    uint8_t conn_devices = besui_get_profile_conn_num();
    BESUI_TRACE(2,"[UIGAME]%s, next=%d, curr=%d, conn_devices = %d", __func__, onoff, next_mode, conn_devices);
    if(TWS_UI_SLAVE != app_ibrt_if_get_ui_role())
    {
        if(voice_flag)
        {
            if(uicom.latency_mode == LATENCY_ON)
            {
                media_PlayAudio((AUD_ID_ENUM)AUD_ID_GAME_OFF, 0);
            }
            else //if(uicom.latency_mode == LATENCY_OFF)
            {
                media_PlayAudio((AUD_ID_ENUM)AUD_ID_BT_BEASTGAME_MODE, 0);
            }
        }
    }
    besui_gamemode_param_backups(onoff, next_mode);
}

osTimerId gamemode_timer_id = NULL;
uint8_t game_cnt = 0;
void gamemode_timer_timehandler(void const *param)
{
	BESUI_TRACE(0, "[UIGAME][UITIMER]%s, game_cnt = %d", __func__, game_cnt);
    game_cnt ++;
    gamemode_timer_onoff(true);
    if(game_cnt == 4)
    {
        uictl.game_set_flag = true;
    }
    else if(game_cnt >= 5)
    {
        game_cnt = 0;
        besui_gamemode_param_set(onoff_buckups, next_mode_backups);
        gamemode_timer_onoff(false);
        osTimerDelete(gamemode_timer_id);
        gamemode_timer_id = NULL;
    }
}
osTimerDef (GAMEMODE_TIMER_NAME, (void (*)(void const *))gamemode_timer_timehandler);

void gamemode_timer_onoff(bool timer_en)
{
	BESUI_TRACE(1, "[UIGAME][UITIMER]%s timer_en %d ", __func__, timer_en);

	if(gamemode_timer_id == NULL)
	{
        game_cnt = 0;
    	gamemode_timer_id = osTimerCreate(osTimer(GAMEMODE_TIMER_NAME),osTimerOnce,NULL);
        if(!gamemode_timer_id)
             ASSERT(0, "[UIGAME][UITIMER][%s] osTimerCreate error", __func__);
	}

	if(timer_en)
		osTimerStart(gamemode_timer_id, 150);
	else
		osTimerStop(gamemode_timer_id);
}
#endif

#if defined(ANC_APP)
static bool user_set_flag = false;
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

void user_anc_set_judge(bool sta)
{
    user_set_flag = sta;
}

bool user_anc_get_judge(void)
{
    return user_set_flag;
}

int32_t besui_anc_set_mode(uint8_t sta)
{
    BESUI_TRACE(0, "[UIANC]%s, sta = %d", __func__, sta);
    user_anc_set_judge(true);
    app_anc_switch((app_anc_mode_t)sta);
    besui_set_curr_anc_mode((app_anc_mode_t)sta);
    return 0;
}

void app_tws_sync_anc_mode(uint8_t sta)
{
    BESUI_TRACE(0, "[UIANC]%s, sta = %d", __func__, sta);
    app_anc_sync_mode((app_anc_mode_t)sta);
}

#if 1
static osTimerId anc_store_timerId = NULL;
static void anc_store_timer_handler(void const *param);
void anc_store_timer_onoff(bool timer_en);
osTimerDef (ANC_STROE_TIMER_NAME, (void (*)(void const *))anc_store_timer_handler);

bool open_close_anc_buf = false;
bool store_last_anc_mode_flag_buf = false;

static void anc_store_timer_handler(void const *param) //1s one times
{
	BESUI_TRACE(0, "[BESUI-TIMER]%s", __func__);

    app_anc_open_close(open_close_anc_buf, store_last_anc_mode_flag_buf);
    
    osTimerDelete(anc_store_timerId);
    anc_store_timerId = NULL;
}
void anc_store_timer_onoff(bool timer_en, uint16_t param)
{
	BESUI_TRACE(1, "[UIANC][UITIMER]%s timer_en %d ", __func__, timer_en);

	if(anc_store_timerId == NULL)
	{
    	anc_store_timerId = osTimerCreate(osTimer(ANC_STROE_TIMER_NAME),osTimerOnce,NULL);
        if(!anc_store_timerId)
             ASSERT(0, "[UIANC][UITIMER][%s] osTimerCreate error", __func__);
	}

	if(timer_en)
		osTimerStart(anc_store_timerId, param);
	else
		osTimerStop(anc_store_timerId);
}
#endif

void app_common_tws_open_close_anc(bool open_close_anc, bool store_last_anc_mode_flag)
{
    BESUI_TRACE(0, "[UIANC][UITIMER]%s", __func__);
    if(open_close_anc==false && open_close_anc_buf==true && anc_store_timerId) //double off
    {
        open_close_anc_buf = false; //off
        store_last_anc_mode_flag_buf = true; //need restore
        anc_store_timer_onoff(false, 0);
        BESUI_TRACE(0, "[UIANC][UITIMER]%s, wait restore, return !", __func__);
        return;
    }

    open_close_anc_buf = open_close_anc;
    store_last_anc_mode_flag_buf = store_last_anc_mode_flag;
    if(open_close_anc == false) //off
        anc_store_timer_onoff(true, 10);
    else //if(open_close_anc == true) //on speech end need delay and then open anc, otherwise crash
        anc_store_timer_onoff(true, 1200);
}

//tws dis, call ,  end call
void app_anc_open_close(bool open_close, bool store_last_flag)
{
    app_anc_mode_t curr_mode = APP_ANC_MODE_OFF;
    static app_anc_mode_t last_mode = APP_ANC_MODE_OFF;

    BESUI_TRACE(4,"[UIANC]%s, open_close=%d, store=%d, anc_mode=%d, last_mode=%d", __func__,open_close, store_last_flag, besui_get_curr_anc_mode(), last_mode);

    if(open_close)
    {
        if(last_mode != APP_ANC_MODE_OFF)
        {
            besui_set_curr_anc_mode(last_mode);
            app_anc_switch(last_mode);
            last_mode = APP_ANC_MODE_OFF;
        }
    }
    else
    {
        if((store_last_flag)&&(app_anc_get_curr_mode() != APP_ANC_MODE_OFF))
        {
            last_mode = app_anc_get_curr_mode();
        }
        else
        {
            last_mode = APP_ANC_MODE_OFF;
        }
        if(app_anc_get_curr_mode() != APP_ANC_MODE_OFF)
        {
            besui_set_curr_anc_mode(curr_mode);
            app_anc_switch(curr_mode);
        }
    }
}


void besui_anc_key_switch(void)
{
    uint8_t mode_buf = APP_ANC_MODE_OFF;
#if !defined(USER_NOISE_ADAPTIVE_ANC_EN)
    if(besui_get_curr_anc_mode() == APP_ANC_MODE2)
        besui_set_curr_anc_mode(APP_ANC_MODE1);
    else if(besui_get_curr_anc_mode() == APP_ANC_MODE1)
        besui_set_curr_anc_mode(APP_ANC_MODE_OFF);
    else if(besui_get_curr_anc_mode() == APP_ANC_MODE_OFF)
        besui_set_curr_anc_mode(APP_ANC_MODE2);
#endif

#ifdef ALGO_INFO_SYNC_EN
    mode_buf = besui_get_curr_anc_mode();
#if defined(USER_NOISE_ADAPTIVE_ANC_EN)
    if(besui_anc_adapt_get_onoff() || mode_buf == APP_ANC_MODE1 || mode_buf == APP_ANC_MODE2 || mode_buf == APP_ANC_MODE3)
        mode_buf = 4; //off
    else if(mode_buf == APP_ANC_MODE4) //Ambient
        mode_buf = nvrecord_uienv->anc_lmh_mode?nvrecord_uienv->anc_lmh_mode:7; 
    else if(mode_buf == APP_ANC_MODE_OFF)
        mode_buf = 2; //Ambient
    if(mode_buf == 1)
        algo_send_request(ALGO_ID_ANC, mode_buf, 3, 1, 0, 1);
    else
        algo_send_request(ALGO_ID_ANC, mode_buf, 0, 1, 0, 1);  
#else
    if(mode_buf == APP_ANC_MODE_OFF)
        mode_buf = 4;
    algo_send_request(ALGO_ID_ANC, mode_buf, 0, 0, 0, 1);
#endif
#endif
    BESUI_TRACE(0, "[UIANC]%s, anc_mode = %d", __func__, besui_get_curr_anc_mode());
}

void besui_anc_mode_nv_write(void)
{
    if(!app_bt_audio_count_connected_sco() && !app_poweroff_flag)
    {
        nv_record_env_get(&nvrecord_uienv);
        nvrecord_uienv->sync_anc_flag = true;
        nvrecord_uienv->sync_anc_mode = besui_get_curr_anc_mode();

#if defined(USER_NOISE_ADAPTIVE_ANC_EN)
        nvrecord_uienv->sync_anc_adapt = besui_anc_adapt_get_onoff();
#endif

        nv_record_env_set(nvrecord_uienv);
        nv_record_flash_flush();
        BESUI_TRACE(2,"[UIANC]%s anc_flag %d anc_mode %d", __func__, nvrecord_uienv->sync_anc_flag, nvrecord_uienv->sync_anc_mode);
    }
}

void besui_anc_mode_nv_read(void)
{
    nv_record_env_get(&nvrecord_uienv);
    BESUI_TRACE(2,"[UIANC]%s, anc_flag=%d,anc_mode=%d", __func__, nvrecord_uienv->sync_anc_flag, nvrecord_uienv->sync_anc_mode);
    if(nvrecord_uienv->sync_anc_flag)
    {
        besui_anc_set_mode(nvrecord_uienv->sync_anc_mode);
    }
#if defined(USER_NOISE_ADAPTIVE_ANC_EN)
    besui_anc_adapt_set_onoff(nvrecord_uienv->sync_anc_adapt);
#endif
}

void besui_anc_tws_sync(void)
{
    BESUI_TRACE(1,"[UIANC]%s, anc_mode = %d", __func__, besui_get_curr_anc_mode());
    app_tws_sync_anc_mode(besui_get_curr_anc_mode());    
}

void besui_anc_openbox_tws_sync(void)
{
    besui_anc_mode_nv_read();
    app_tws_sync_anc_mode(nvrecord_uienv->sync_anc_mode);
    BESUI_TRACE(1,"[UIANC]%s anc mode %d nv anc mode %d", __func__, app_anc_get_curr_mode(), nvrecord_uienv->sync_anc_mode);      
}
#endif

void app_open_box_fast_get_battery_level(void)
{
    if(uicom.poweron_bat_det_flag)
        return;

    uicom.box_open_bat_det_flag =true;
    app_battery_stop();
    app_battery_clear_index();
    app_battery_start();
}

#ifdef BESUI_NTC_EN
void app_ntc_detect_init(void)
{
	BESUI_TRACE(0, "[UINTC]%s ", __func__);
    ntc_capture_open();
    app_ntc_detect_volt_timer_onoff(true);
}

osTimerId app_ntc_open_process_timer = NULL;
void app_ntc_detect_timehandler(void const *param)
{
	BESUI_TRACE(0, "[UITIMER]%s ", __func__);

    ntc_capture_start();
    app_ntc_detect_volt_timer_onoff(true);
}

osTimerDef (APP_NTC_TIMER_NAME, (void (*)(void const *))app_ntc_detect_timehandler);

void app_ntc_detect_volt_timer_onoff(bool timer_en)
{
	BESUI_TRACE(1, "[UITIMER]%s timer_en %d ", __func__, timer_en);

	if(app_ntc_open_process_timer == NULL)
	{
    	app_ntc_open_process_timer = osTimerCreate(osTimer(APP_NTC_TIMER_NAME), osTimerOnce, NULL);
	}

	if(timer_en)
	{
		osTimerStart(app_ntc_open_process_timer, 10000);
	}
	else
	{
		osTimerStop(app_ntc_open_process_timer);
	}
}

/*
IH6
1----0度以下和45度以上禁止充电
2----00~10度：0.5C（20mA）充电
3----11~45度：  2C（80mA）充电
*/
#define NTC_TAB_MAX             82
#define NTC_TEMPERATURE_60      328
#define NTC_TEMPERATURE_45      475 //
#define NTC_TEMPERATURE_42      505 //
#define NTC_TEMPERATURE_11      910 //
#define NTC_TEMPERATURE_10      922 //
#define NTC_TEMPERATURE_03      1010
#define NTC_TEMPERATURE_01      1018 //
#define NTC_TEMPERATURE_00      1030 //

#define NTC_TEMPERATURE_F5      1069
#define NTC_TEMPERATURE_F10     1106
#define NTC_TEMPERATURE_F20     1160

const uint16_t temp_tab[NTC_TAB_MAX]={
    312,                                              //61
    328, 338, 348, 358, 368, 378, 388, 398, 408, 418, //60~51
    426, 435, 445, 455, 465, 475, 485, 495, 505, 516, //50~41
    520, 536, 552, 568, 584, 600, 616, 632, 648, 662, //40~31
    673, 685, 697, 709, 721, 733, 745, 757, 770, 783, //30~21
    800, 812, 825, 837, 849, 862, 874, 886, 898, 912, //20~11
    922, 932, 943, 953, 964, 975, 985, 995, 1007, 1018, //10~01
    1030,                                              //00
    1038, 1045, 1052, 1060, 1067, 1075, 1082, 1089, 1096, 1105, //-01 ~ -10
    1110, 1117, 1124, 1131, 1138, 1142, 1146, 1151, 1155, 1160, //-11 ~ -20
};

static uint8_t ntc_charger_sta = 0;
void app_ntc_detect_process(uint16_t ad_volt) //10s
{
    bud_box_state  local_box_state = app_ui_get_local_box_state();
    bud_box_state  peer_box_state = app_ui_get_peer_box_state();

    BESUI_TRACE(0, "[UINTC]%s, charge_sta = %d, ntc_volt = %d", __func__, besui_bat_charge_sta_get(), ad_volt);
    //BESUI_TRACE(0, "[UINTC]%s, local_box  = %d, peer box = %d", __func__, local_box_state, peer_box_state);
    
#if 1
    for(uint8_t i = 0;i < NTC_TAB_MAX;i ++)
    {
        if(ad_volt < temp_tab[i])
        {
             if(i <= 62)
                BESUI_TRACE(0, "[UINTC]%s, tempval = %d", __func__, 62-i);
             else
                BESUI_TRACE(0, "[UINTC]%s, tempval = -%d", __func__, i-62);
            break;
        }
        if(i == (NTC_TAB_MAX-1))
        {
            BESUI_TRACE(0, "[UINTC]%s, tempval < -20", __func__);
        }
    }
#endif

    #define NTC_SHAKE_CNT               6

    #define NTC_TYPE_CHARGER_STOP       1
    #define NTC_TYPE_CHARGER_20MA       2
    #define NTC_TYPE_CHARGER_80MA       3
    #define NTC_TYPE_POWEROFF           4

    uint8_t ntc_pre_type = 0;
    static uint8_t ntc_recharger_sta = 0;
    static uint8_t ntc_type = 0;
    static uint8_t shake_buf = 0;
    static bool ntc_start_process = false;

    if(ad_volt < NTC_TEMPERATURE_45 || ad_volt > NTC_TEMPERATURE_00)       //<0℃  >45℃ stop charger
        ntc_pre_type = NTC_TYPE_CHARGER_STOP;
    if((ad_volt >= NTC_TEMPERATURE_10) && (ad_volt <= NTC_TEMPERATURE_00)) //0~10℃ = 20MA
        ntc_pre_type = NTC_TYPE_CHARGER_20MA;
    if((ad_volt > NTC_TEMPERATURE_45) && (ad_volt <= NTC_TEMPERATURE_11)) //11~45℃ = 80MA
        ntc_pre_type = NTC_TYPE_CHARGER_80MA;
    if(ad_volt <= NTC_TEMPERATURE_60 || ad_volt >= NTC_TEMPERATURE_F20) //<-20℃  >=60℃  POWEROFF
        ntc_pre_type = NTC_TYPE_POWEROFF;

    if(ntc_type != ntc_pre_type)
    {
        shake_buf ++;
        if(shake_buf >= NTC_SHAKE_CNT)
        {
            shake_buf = 0;
            ntc_type = ntc_pre_type;
            ntc_start_process = true;
            BESUI_TRACE(0, "[UINTC]%s, ntc_type = %d", __func__, ntc_type);
        }
    }

    if(ntc_start_process == true)
    {
        ntc_start_process = false;
        
        if(ntc_type == NTC_TYPE_CHARGER_STOP)
        {
            ntc_charger_sta = 1;        //stop charger
            charger_param_set_onoff(false, uicom.charge_ma_val);
            BESUI_TRACE(0, "[UINTC]%s, charger stop", __func__);
        }
        else if(ntc_type == NTC_TYPE_CHARGER_20MA)
        {
            if(ntc_charger_sta == 1)
            {
                ntc_recharger_sta = 1;
                BESUI_TRACE(0, "[UINTC]%s, need recharger 20MA", __func__);
            }
            else
            {
                uicom.charge_ma_val = CHARGER_CHARGE_CONSTANT_CURRENT_20MA;
                charger_param_set_onoff(true, uicom.charge_ma_val);
                BESUI_TRACE(0, "[UINTC]%s, charger 20MA", __func__);
            }
        }
        else if(ntc_type == NTC_TYPE_CHARGER_80MA)
        {
            if(ntc_charger_sta == 1)
            {
                ntc_recharger_sta = 2;
                BESUI_TRACE(0, "[UINTC]%s, need recharger 80MA", __func__);
            }
            else
            {
                uicom.charge_ma_val = CHARGER_CHARGE_CONSTANT_CURRENT_80MA;
                charger_param_set_onoff(true, uicom.charge_ma_val);
                BESUI_TRACE(0, "[UINTC]%s, charger 80MA", __func__);
            }
        }
        else if(ntc_type == NTC_TYPE_POWEROFF)
        {
            if((local_box_state == IBRT_OUT_BOX)&&(peer_box_state == IBRT_OUT_BOX))
            {
                app_ibrt_if_event_entry(APP_UI_EV_DOCK);
            }
            if(ad_volt <= NTC_TEMPERATURE_60)
                BESUI_TRACE(0, "[UINTC]%s, temp too high, poweroff!!!", __func__);
            else
                BESUI_TRACE(0, "[UINTC]%s, temp too low, poweroff!!!", __func__);
            charger_param_set_onoff(false, uicom.charge_ma_val);
            uictl.shutdown_type = SHUTDOWN_NTC_TEMP;
            app_shutdown();
        }
    }

    if(ntc_recharger_sta == 1)
    {
        if((ad_volt >= NTC_TEMPERATURE_10) && (ad_volt <= NTC_TEMPERATURE_03)) //20MA
        {
            ntc_recharger_sta = 0;

            ntc_charger_sta = 0;
            uicom.charge_ma_val = CHARGER_CHARGE_CONSTANT_CURRENT_20MA;
            charger_param_set_onoff(true, uicom.charge_ma_val);
            BESUI_TRACE(0, "[UINTC]%s, recharger 20MA", __func__);
        }
    }
    else if(ntc_recharger_sta == 2)
    {
        if((ad_volt >= NTC_TEMPERATURE_42) && (ad_volt <= NTC_TEMPERATURE_11)) //80MA
        {
            ntc_recharger_sta = 0;

            ntc_charger_sta = 0;
            uicom.charge_ma_val = CHARGER_CHARGE_CONSTANT_CURRENT_80MA;
            charger_param_set_onoff(true, uicom.charge_ma_val);
            BESUI_TRACE(0, "[UINTC]%s, recharger 80MA", __func__);
        }
    }
}

bool ntc_charger_status(void)
{
    return ntc_charger_sta;
}
#endif //#ifdef BESUI_NTC_EN


void app_call_varible_init(void) //call tone handle
{
    BESUI_TRACE(0,"[UICOM]%s", __func__);

    uicom.call_end_prompt_type = DEFAULT_TYPE;
    uicom.call_end_type = DEFAULT_TYPE;
    uicom.call_incoming_flag = false;
    uicom.call_out_flag = false;
}

void app_callsetup_ind_handle_process(uint8_t device_id)
{
    struct BT_DEVICE_T* curr_device = app_bt_get_device(device_id);
#if (BT_DEVICE_NUM > 1)
    int another_device_id = app_bt_audio_get_another_hfp_device_for_user_action(device_id);
    struct BT_DEVICE_T* another_device = app_bt_get_device(another_device_id);
    BESUI_TRACE(8,"[UICOM]%s,call state= %d  %d  %d  %d, %d  %d  %d  %d", __func__,  
    curr_device->hfchan_callSetup, curr_device->hfchan_call,curr_device->hf_audio_state,curr_device->hf_callheld,
    another_device->hfchan_callSetup, another_device->hfchan_call, another_device->hf_audio_state, another_device->hf_callheld);
#else
    BESUI_TRACE(8,"[UICOM]%s,call state= %d  %d  %d  %d", __func__,  
    curr_device->hfchan_callSetup, curr_device->hfchan_call,curr_device->hf_audio_state,curr_device->hf_callheld);
#endif

    if((curr_device->hfchan_callSetup == BT_HFP_CALL_SETUP_IN)&&(curr_device->hfchan_call == BT_HFP_CALL_NONE)
    //&&(curr_device->hf_audio_state == BT_HFP_AUDIO_DISCON)
#if (BT_DEVICE_NUM > 1)
    //&&(another_device->hfchan_callSetup == BT_HF_CALLSETUP_NONE)
    &&(another_device->hfchan_callSetup == BT_HF_CALLSETUP_NONE)
    &&(another_device->hfchan_call == BT_HFP_CALL_NONE)&&(another_device->hf_audio_state == BT_HFP_AUDIO_DISCON)
#endif
    )
    {
        //incomingcall
        uicom.call_incoming_flag = true;
        app_status_indication_set(APP_STATUS_INDICATION_INCOMINGCALL);
    }
    else if(((curr_device->hfchan_callSetup == BT_HFP_CALL_SETUP_OUT)||(curr_device->hfchan_callSetup == BT_HFP_CALL_SETUP_ALERT))&&(curr_device->hfchan_call == BT_HFP_CALL_NONE)
    //&&(curr_device->hf_audio_state == BT_HFP_AUDIO_DISCON)
#if (BT_DEVICE_NUM > 1)
    //&&(another_device->hfchan_callSetup == BT_HF_CALLSETUP_NONE)
    &&(another_device->hfchan_callSetup == BT_HF_CALLSETUP_NONE)
    &&(another_device->hfchan_call == BT_HFP_CALL_NONE)&&(another_device->hf_audio_state == BT_HFP_AUDIO_DISCON)
#endif
    )
    {
        //outcall
        uicom.call_out_flag = true;
        app_status_indication_set(APP_STATUS_INDICATION_OUTCALL);
    }
    else if((curr_device->hfchan_call == BT_HFP_CALL_ACTIVE)
#if (BT_DEVICE_NUM > 1)
    ||(another_device->hfchan_call == BT_HFP_CALL_ACTIVE)
#endif
    )
    {
        //answer
        uicom.call_incoming_flag = false;
        uicom.call_out_flag = false;
        app_status_indication_set(APP_STATUS_INDICATION_ANSWERCALL);
    }
}

void app_tws_set_ledsta_call_slave_process(uint8_t led_type, uint8_t call_status)
{
    if(call_status == EARSTA_LINKLOSS_END)
    {
        app_status_indication_set((APP_STATUS_INDICATION_T)led_type);
        return;
    }

    if(led_type == APP_STATUS_INDICATION_PHONE_CONNECTED)
    {
        uicom.led_bt_conn_flag = true;
    }

    app_status_indication_set((APP_STATUS_INDICATION_T)led_type);

    if(led_type == APP_STATUS_INDICATION_PHONE_CONNECTED)
    {
        app_recover_led_timer_onoff(true, true);
    }

    if(call_status == EARSTA_INCOMINGCALL)
    {
        uicom.call_incoming_flag = true;
    }
    else if (call_status == EARSTA_OUTCALL_MODE)
    {
        uicom.call_out_flag = true;
    }
    else if(call_status == EARSTA_ACTIVECALL)
    {
        uicom.call_incoming_flag = false;
        uicom.call_out_flag = false;
    }
}

void app_call_connected_handle_process(uint8_t device_id)
{
    struct BT_DEVICE_T* curr_device = app_bt_get_device(device_id);
    int another_device_id = app_bt_audio_get_another_hfp_device_for_user_action(device_id);
    struct BT_DEVICE_T* another_device = app_bt_get_device(another_device_id);

    BESUI_TRACE(8,"[UICOM]%s,call state= %d  %d  %d  %d, %d  %d  %d  %d", __func__,  
    curr_device->hfchan_callSetup, curr_device->hfchan_call,curr_device->hf_audio_state,curr_device->hf_callheld,
    another_device->hfchan_callSetup, another_device->hfchan_call, another_device->hf_audio_state, another_device->hf_callheld);    

    if((uicom.call_incoming_flag == false)&&(uicom.call_out_flag == false))
    {
        if((curr_device->hfchan_call == BT_HFP_CALL_NONE)
#if (BT_DEVICE_NUM > 1)
        &&(another_device->hfchan_call == BT_HFP_CALL_NONE)
#endif        
        )
        {
            app_status_indication_set(APP_STATUS_INDICATION_ANSWERCALL);
            uicom.call_end_type = AUDIO_CONNECTED_NOCALL;
            return;
        }
    }

    uicom.call_end_type = DEFAULT_TYPE;
}

void app_call_discon_process(uint8_t device_id, uint8_t calldiscon_type)
{
    struct BT_DEVICE_T* curr_device = app_bt_get_device(device_id);
#if (BT_DEVICE_NUM > 1)
    int another_device_id = app_bt_audio_get_another_hfp_device_for_user_action(device_id);
    struct BT_DEVICE_T* another_device = app_bt_get_device(another_device_id);
    BESUI_TRACE(8,"[UICOM]%s,call state= %d  %d  %d  %d, %d  %d  %d  %d", __func__,  
    curr_device->hfchan_callSetup, curr_device->hfchan_call,curr_device->hf_audio_state,curr_device->hf_callheld,
    another_device->hfchan_callSetup, another_device->hfchan_call, another_device->hf_audio_state, another_device->hf_callheld);    
#else
    BESUI_TRACE(8,"[UICOM]%s,call state= %d  %d  %d  %d", __func__,  
    curr_device->hfchan_callSetup, curr_device->hfchan_call,curr_device->hf_audio_state,curr_device->hf_callheld);
#endif

    if(/*(curr_device->hfchan_callSetup == BT_HF_CALLSETUP_NONE)&&*/(curr_device->hf_callheld == BT_HFP_CALL_HELD_NONE))
    //&&(another_device->hfchan_callSetup == BT_HF_CALLSETUP_NONE)&&(another_device->hf_callheld == BT_HFP_CALL_HELD_NONE)
    //&&(another_device->hfchan_call == BT_HFP_CALL_NONE)&&(another_device->hf_audio_state == BT_HFP_AUDIO_DISCON))
    {
        BESUI_TRACE(2,"[UICOM]%s order_type %d voice %d role %d inc %d", __func__, uicom.call_end_type, uicom.call_end_prompt_type, app_ibrt_if_get_ui_role(), uicom.call_incoming_flag);
        if(uicom.call_end_type == DEFAULT_TYPE)
        {
            uicom.call_end_type = calldiscon_type;
        }
        else if(uicom.call_end_type == HANGUP_REJECT_TYPE)
        {
            if(uicom.call_end_prompt_type == END_CALL_TYPE)
            {
                if(TWS_UI_SLAVE != app_ibrt_if_get_ui_role())
                {
                    uicom.call_prompt_type = END_CALL_TYPE;
                    app_reject_call_voice_timer_onoff(true);
                }
            }
            else if(uicom.call_end_prompt_type == REJECT_CALL_TYPE)
            {
                if(TWS_UI_SLAVE != app_ibrt_if_get_ui_role())
                {
                    uicom.call_prompt_type = REJECT_CALL_TYPE;
                    app_reject_call_voice_timer_onoff(true);
                }
            }

            app_status_indication_set(APP_STATUS_INDICATION_CONNECTED);  //led timer
            app_call_varible_init();
            #if defined(ANC_APP)
            app_common_tws_open_close_anc(true, false);
            #endif
        }
        else if(uicom.call_end_type == AUDIO_CONNECTED_NOCALL)
        {
            if(uicom.call_end_prompt_type == REJECT_CALL_TYPE)
            {
                if(TWS_UI_SLAVE != app_ibrt_if_get_ui_role())
                {
                    uicom.call_prompt_type = REJECT_CALL_TYPE;
                    app_reject_call_voice_timer_onoff(true);
                }
            }
            else if(uicom.call_end_prompt_type == END_CALL_TYPE)
            {
                if(TWS_UI_SLAVE != app_ibrt_if_get_ui_role())
                {
                    uicom.call_prompt_type = END_CALL_TYPE;
                    app_reject_call_voice_timer_onoff(true);
                }
            }

            app_status_indication_set(APP_STATUS_INDICATION_CONNECTED);
            app_call_varible_init();
#if defined(ANC_APP)
            app_common_tws_open_close_anc(true, false);
#endif
        }
    }
}


void app_call_end_handle_process(uint8_t device_id, uint8_t endcall_type)
{
    struct BT_DEVICE_T* curr_device = app_bt_get_device(device_id);
#if (BT_DEVICE_NUM > 1)
    int another_device_id = app_bt_audio_get_another_hfp_device_for_user_action(device_id);
    struct BT_DEVICE_T* another_device = app_bt_get_device(another_device_id);
    BESUI_TRACE(8,"[UICOM]%s,call state= %d  %d  %d  %d, %d  %d  %d  %d", __func__,  
    curr_device->hfchan_callSetup, curr_device->hfchan_call,curr_device->hf_audio_state,curr_device->hf_callheld,
    another_device->hfchan_callSetup, another_device->hfchan_call, another_device->hf_audio_state, another_device->hf_callheld);   
#else
    BESUI_TRACE(8,"[UICOM]%s,call state= %d  %d  %d  %d", __func__,  
    curr_device->hfchan_callSetup, curr_device->hfchan_call,curr_device->hf_audio_state,curr_device->hf_callheld);
#endif


    if((curr_device->hfchan_callSetup == BT_HF_CALLSETUP_NONE)&&(curr_device->hf_callheld == BT_HFP_CALL_HELD_NONE))
    //&&(another_device->hfchan_callSetup == BT_HF_CALLSETUP_NONE)&&(another_device->hf_callheld == BT_HFP_CALL_HELD_NONE)
    //&&(another_device->hfchan_call == BT_HFP_CALL_NONE)&&(another_device->hf_audio_state == BT_HFP_AUDIO_DISCON))
    {
        BESUI_TRACE(2,"[UICOM]%s order_type %d voice %d role %d inc %d", __func__, uicom.call_end_type, uicom.call_end_prompt_type, app_ibrt_if_get_ui_role(), uicom.call_incoming_flag);

        if(uicom.call_end_type == DEFAULT_TYPE)
        {
            uicom.call_end_type = endcall_type;
            if(uicom.call_incoming_flag)
            {
                uicom.call_end_type = DEFAULT_TYPE;
                if(TWS_UI_SLAVE != app_ibrt_if_get_ui_role())
                {
                    uicom.call_prompt_type = REJECT_CALL_TYPE;
                    app_reject_call_voice_timer_onoff(true);
                }
            }
            else //hung up
            {
                if(uicom.call_end_prompt_type == END_CALL_TYPE)
                {
                    if(TWS_UI_SLAVE != app_ibrt_if_get_ui_role())
                    {
                        uicom.call_prompt_type = END_CALL_TYPE;
                        app_reject_call_voice_timer_onoff(true);
                    }
                }
                else if(uicom.call_end_prompt_type == REJECT_CALL_TYPE)
                {
                    if(TWS_UI_SLAVE != app_ibrt_if_get_ui_role())
                    {
                        uicom.call_prompt_type = REJECT_CALL_TYPE;
                        app_reject_call_voice_timer_onoff(true);
                    }
                }

                app_status_indication_set(APP_STATUS_INDICATION_CONNECTED);
                app_call_varible_init();
                #if defined(ANC_APP)
                app_common_tws_open_close_anc(true, false);
                #endif
            }
        }
        else if(uicom.call_end_type == DISCONNECTED_TYPE)
        {
            if(uicom.call_end_prompt_type == END_CALL_TYPE)
            {
                if(TWS_UI_SLAVE != app_ibrt_if_get_ui_role())
                {
                    uicom.call_prompt_type = END_CALL_TYPE;
                    app_reject_call_voice_timer_onoff(true);
                }
            }
            else if(uicom.call_end_prompt_type == REJECT_CALL_TYPE)
            {
                if(TWS_UI_SLAVE != app_ibrt_if_get_ui_role())
                {
                    uicom.call_prompt_type = REJECT_CALL_TYPE;
                    app_reject_call_voice_timer_onoff(true);
                }
            }
            
            app_status_indication_set(APP_STATUS_INDICATION_CONNECTED);
            app_call_varible_init();
#if defined(ANC_APP)
            app_common_tws_open_close_anc(true, false);
#endif
        }
    }
}

void app_tws_ledsta_to_slave_process(uint8_t connect_num, uint8_t led_type)
{
    uint8_t master_current_led = APP_STATUS_INDICATION_NUM;//app_status_indication_get();
    struct BT_DEVICE_T* curr_device = NULL;
    struct BT_DEVICE_T* another_device = NULL;
    uint8_t call_status = 0xff;
    BESUI_TRACE(1, "[UICOM]%s", __func__);
    if(led_type == APP_STATUS_INDICATION_NUM)
    {
        master_current_led = app_status_indication_get();
        if(connect_num == 1)
        {
            curr_device = app_bt_get_device(BT_DEVICE_ID_1);
            BESUI_TRACE(4,"[UICOM]%s, call state= %d  %d  %d  %d", __func__,
            curr_device->hfchan_callSetup, curr_device->hfchan_call,curr_device->hf_audio_state,curr_device->hf_callheld);

            if((curr_device->hfchan_call == BT_HFP_CALL_NONE)&&(curr_device->hfchan_callSetup == BT_HF_CALLSETUP_NONE))
            {
                if(curr_device->hf_audio_state == BT_HFP_AUDIO_DISCON)
                {
                    call_status = EARSTA_A2DP_MODE;
                }
                else
                {
                    call_status = EARSTA_AUDIO_CONNECTED;
                }
            }
            else
            {
                if(curr_device->hfchan_call == BT_HFP_CALL_ACTIVE)
                {
                    call_status = EARSTA_ACTIVECALL;
                }
                else
                {
                    if(curr_device->hfchan_callSetup == BT_HFP_CALL_SETUP_IN)
                    {
                        call_status = EARSTA_INCOMINGCALL;
                    }
                    else if((curr_device->hfchan_callSetup == BT_HFP_CALL_SETUP_OUT)||(curr_device->hfchan_callSetup == BT_HFP_CALL_SETUP_ALERT))
                    {
                        call_status = EARSTA_OUTCALL_MODE;
                    }
                }
            }
        }
        else if(connect_num == 2)
        {
            curr_device = app_bt_get_device(BT_DEVICE_ID_1);
            another_device = app_bt_get_device(BT_DEVICE_ID_2);
            BESUI_TRACE(8,"[UICOM]%s,call state= %d  %d  %d  %d, %d  %d  %d  %d", __func__,  
            curr_device->hfchan_callSetup, curr_device->hfchan_call,curr_device->hf_audio_state,curr_device->hf_callheld,
            another_device->hfchan_callSetup, another_device->hfchan_call, another_device->hf_audio_state, another_device->hf_callheld);

            if((curr_device->hfchan_call == BT_HFP_CALL_NONE)&&(curr_device->hfchan_callSetup == BT_HF_CALLSETUP_NONE)
            &&(another_device->hfchan_call == BT_HFP_CALL_NONE)&&(another_device->hfchan_callSetup == BT_HF_CALLSETUP_NONE))
            {
                if((curr_device->hf_audio_state == BT_HFP_AUDIO_DISCON)&&(another_device->hf_audio_state == BT_HFP_AUDIO_DISCON)
                )
                {
                    call_status = EARSTA_A2DP_MODE;
                }
                else
                {
                    call_status = EARSTA_AUDIO_CONNECTED;
                }
            }
            else
            {
                if((curr_device->hfchan_call == BT_HFP_CALL_ACTIVE)||(another_device->hfchan_call == BT_HFP_CALL_ACTIVE))
                {
                    call_status = EARSTA_ACTIVECALL;
                }
                else
                {
                    if((curr_device->hfchan_callSetup == BT_HFP_CALL_SETUP_IN)||(another_device->hfchan_callSetup == BT_HFP_CALL_SETUP_IN))
                    {
                        call_status = EARSTA_INCOMINGCALL;
                    }
                    else if((curr_device->hfchan_callSetup == BT_HFP_CALL_SETUP_OUT)||(curr_device->hfchan_callSetup == BT_HFP_CALL_SETUP_ALERT)
                    ||(another_device->hfchan_callSetup == BT_HFP_CALL_SETUP_OUT)||(another_device->hfchan_callSetup == BT_HFP_CALL_SETUP_ALERT))
                    {
                        call_status = EARSTA_OUTCALL_MODE;
                    }
                }
            }
        }
    }
    else
    {
        master_current_led = led_type;
    }

    app_ibrt_customif_cmd_sync_led_mode(master_current_led, call_status); //send to slave
}

void app_common_store_twsaddrto_nv_flash(uint8_t *btaddr) //store tws addr
{
    uint8_t i = 0;

    nv_record_env_get(&nvrecord_uienv);
    for(i = 0; i < 6; i++)
    {
        nvrecord_uienv->tws_con_addr[i] = btaddr[i];
    }
    nv_record_env_set(nvrecord_uienv);
    nv_record_env_get(&nvrecord_uienv);
    BESUI_TRACE(0,"[UICOM]%s store tws addr:", __func__);
    DUMP8("0x%02x ", nvrecord_uienv->tws_con_addr, 6);
}


bool app_charge_fast_exit_factory_mode(void) //fast exit factory mode
{
    BESUI_TRACE(0,"[UICOM]app_charge_fast_exit_factory_mode");
    if(app_factorymode_get())
    {
        uictl.poweroff_fast_flag = true;
        app_reset();
        return true;
    }
    return false;
}

void app_common_clear_pairlist_process(bool clear_tws_flag, bool clear_phone_flag, bool force_only_flag, bool uart_cmd_flag)
{
    return;
/*
    static bool first_clear_flag = false;
    // ibrt_ctrl_t *p_ibrt_ctrl = NULL;
    // p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();

    if((!clear_tws_flag)&&(!clear_phone_flag))
    {
        BESUI_TRACE(0,"[UICOM]do not need clear");
        return;
    }

    if((first_clear_flag)&&(force_only_flag))
    {
        BESUI_TRACE(0,"[UICOM]clear pairlist need only one");
        return;
    }

    first_clear_flag = force_only_flag;

    if(!uart_cmd_flag)
    {
        if(bts_tws_if_is_tws_link_connected())
        {
            if(TWS_UI_MASTER == app_ibrt_if_get_ui_role())
            {
                if(app_bt_get_curr_access_mode() == BTIF_BAM_GENERAL_ACCESSIBLE)
                {
                    //notify slave
                    app_ibrt_customif_cmd_sync_clear_pairlist(clear_tws_flag, clear_phone_flag);
                    //delay
                    osDelay(150);
                }
            }
        }
    }

    app_status_indication_set(APP_STATUS_INDICATION_CLEARPAIRLIST); //led display
    if((TWS_UI_SLAVE != app_ibrt_if_get_ui_role()&&(app_bt_get_curr_access_mode() == BTIF_BAM_GENERAL_ACCESSIBLE))||(TWS_UI_SLAVE == app_ibrt_if_get_ui_role()))
    {
        if(clear_tws_flag)//clear pairlist
            app_clear_tws_pairlist_process();

        if(clear_phone_flag)
            app_clear_phone_pairlist_process();

        app_clear_pairlist_poweroff_timer_onoff(true); //delay shutdown
        //app_shutdown();
    }
*/
}

extern bt_status_t LinkDisconnectDirectly(bool PowerOffFlag);
void app_remove_all_paired_list(void)
{
    bt_status_t            retStatus;
    btif_device_record_t   record;
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    int                    paired_dev_count = nv_record_get_paired_dev_count();

    BESUI_TRACE(0,"[UICOM]%s", __func__);
    BESUI_TRACE(0,"[UICOM]Master addr:");
    DUMP8("%02x ",p_ibrt_ctrl->local_addr.address, BTIF_BD_ADDR_SIZE);
    BESUI_TRACE(0,"[UICOM]Slave addr:");
    DUMP8("%02x ",p_ibrt_ctrl->peer_addr.address, BTIF_BD_ADDR_SIZE);

    ota_disconnect();
    bes_ble_gap_disconnect_all();

    LinkDisconnectDirectly(true);

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
    uicom.bt_pairlist_clear_flag = true;
    app_status_indication_set(APP_STATUS_INDICATION_CLEARPAIRLIST);
    app_ibrt_if_config_keeper_clear();
    memset(p_ibrt_ctrl->local_addr.address, 0, BTIF_BD_ADDR_SIZE);
    memset(p_ibrt_ctrl->peer_addr.address, 0, BTIF_BD_ADDR_SIZE);

    nv_record_env_get(&nvrecord_uienv);
    nvrecord_uienv->ibrt_mode.mode = IBRT_UNKNOW,
    memset(nvrecord_uienv->ibrt_mode.record.bdAddr.address, 0, BTIF_BD_ADDR_SIZE);
    memset(nvrecord_uienv->tws_con_addr, 0, BTIF_BD_ADDR_SIZE);
#ifdef BESUI_1WIRE_EN
    memset(nvrecord_uienv->another_addr, 0, BTIF_BD_ADDR_SIZE);
    nvrecord_uienv->need_twspair_flag = false;
#endif
#ifdef BESUI_APP_EN
    nvrecord_uienv->init_button = 0x00;
#endif

#ifdef BESUI_GAME_NV_EN
    nvrecord_uienv->remember_game_mode = 0;
#endif
#if defined(__VIRTUAL_SURROUND__) || defined(__VIRTUAL_SURROUND_STEREO__)
    nvrecord_uienv->space_audio_type = 0;  
#endif

#if defined(ANC_APP)
    nvrecord_uienv->sync_anc_flag = false;
    nvrecord_uienv->sync_anc_mode = 0;
    nvrecord_uienv->sync_anc_adapt = false;
#endif

    nv_record_env_set(nvrecord_uienv);
    // nv_record_flash_flush();

    nv_record_rebuild(NV_REBUILD_SDK_ONLY); //clear gfps dev name
}


void app_remove_all_phone_paired_list(void)
{
    bt_status_t            retStatus;
    btif_device_record_t   record;
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    int                    paired_dev_count = nv_record_get_paired_dev_count();
    uint8_t address_compare[6] = {0};

    BESUI_TRACE(0,"[UICOM]%s", __func__);
    BESUI_TRACE(0,"[UICOM]Master addr:");
    DUMP8("%02x ",p_ibrt_ctrl->local_addr.address, BTIF_BD_ADDR_SIZE);
    BESUI_TRACE(0,"[UICOM]Slave addr:");
    DUMP8("%02x ",p_ibrt_ctrl->peer_addr.address, BTIF_BD_ADDR_SIZE);

    for (int32_t index = paired_dev_count - 1; index >= 0; index--)
    {
        retStatus = nv_record_enum_dev_records(index, &record);
        if (BT_STS_SUCCESS == retStatus)
        {
            BESUI_TRACE(1,"[UICOM]The index %d of nv records:", index);
            DUMP8("%02x ", record.bdAddr.address, BTIF_BD_ADDR_SIZE);
            if (memcmp(record.bdAddr.address, p_ibrt_ctrl->local_addr.address, BTIF_BD_ADDR_SIZE) &&
                memcmp(record.bdAddr.address, p_ibrt_ctrl->peer_addr.address, BTIF_BD_ADDR_SIZE)&&
                memcmp(record.bdAddr.address, address_compare, BTIF_BD_ADDR_SIZE))
            {
            	nv_record_ddbrec_delete(&record.bdAddr);
            }
        }
    }

    nv_record_flash_flush();
}


void app_common_disconnected_device(bool all_device_flag, uint8_t device_id)
{
    BESUI_TRACE(0,"[UICOM]%s, %d, %d", __func__, all_device_flag, device_id);
    struct BT_DEVICE_T *curr_device = NULL;

    if(all_device_flag)
    {
        for (int i = 0; i < BT_DEVICE_NUM; ++i)
        {
            curr_device = app_bt_get_device(i);
            if (curr_device->acl_is_connected)
            {
                bts_bt_sink_conn_disconnect_connection(app_bt_get_remote_dev_by_handle(curr_device->acl_conn_hdl));
            }
        }
    }
    else
    {
        curr_device = app_bt_get_device(device_id);
        if (curr_device->acl_is_connected)
        {
            bts_bt_sink_conn_disconnect_connection(app_bt_get_remote_dev_by_handle(curr_device->acl_conn_hdl));
        }
    }
}

void app_discon_enter_pairmode(void)
{
    uint8_t conn_devices = besui_get_profile_conn_num();

    if(conn_devices > 0)
    {
        if(bts_tws_if_is_tws_link_connected())
        {
            if(TWS_UI_MASTER == app_ibrt_if_get_ui_role())
            {
                app_common_disconnected_device(true, BT_DEVICE_NUM);
            }
        }
        else
        {
            app_common_disconnected_device(true, BT_DEVICE_NUM);
        }
    }
    else
    {
        if(app_bt_get_curr_access_mode() != BTIF_BAM_GENERAL_ACCESSIBLE)
        {
            besui_enter_pairmode();
        }
    }
}


uint8_t app_get_software_version_high(void)
{
    uint16_t version = SOFTWARE_VERSION_INFO;
    uint8_t software_version_high = 0;

    software_version_high = (version/100);

    return software_version_high;
}

uint8_t app_get_software_version_middle(void)
{
    uint16_t version = SOFTWARE_VERSION_INFO;
    uint8_t software_version_middle = 0;
    
    software_version_middle = (version%100);
    software_version_middle = (software_version_middle/10);

    return software_version_middle;
}

uint8_t app_get_software_version_low(void)
{
    uint16_t version = SOFTWARE_VERSION_INFO;
    uint8_t software_version_low = 0;

    software_version_low = (version%10);
    
    return software_version_low;
}

bool mute_voice_flag = false;
void app_set_speaker_mute_voice(bool param)
{
    mute_voice_flag = param;
}
bool app_get_speaker_mute_voice_status(void)
{
    return mute_voice_flag;
}

bool mute_a2dp_flag = false;
void app_set_speaker_mute_a2dp(bool param)
{
    mute_a2dp_flag = param;
}
bool app_get_speaker_mute_a2dp_status(void)
{
    if(uictl.callend_mute_flag)
        return true;
    return mute_a2dp_flag;
}

bool mute_hfp_flag = false;
void app_set_speaker_mute_hfp(bool param)
{
    mute_hfp_flag = param;
}
bool app_get_speaker_mute_hfp_status(void)
{
    return mute_hfp_flag;
}

void app_set_speaker_mute_status(bool mute_flag)
{
    app_set_speaker_mute_voice(mute_flag);
    app_set_speaker_mute_a2dp(mute_flag);
    app_set_speaker_mute_hfp(mute_flag);
}


bool app_current_box_status_open_status(void)
{
#ifdef BESUI_1WIRE_EN
    BESUI_TRACE(0, "[UIBOX]%s, %02X", __func__, uicom.box_type);
    if(BOX_CMD_CASE_OPEN == uicom.box_type)
        return true;
#endif
    return false;
}

bool app_current_box_status_close_status(void)
{
#ifdef BESUI_1WIRE_EN
    BESUI_TRACE(0, "[UICOM]%s, %02X", __func__, uicom.box_type);
    if(BOX_CMD_CASE_CLOSE == uicom.box_type)
        return true;
#endif
    return false;
}


void app_uart_open_box_handle(void) //open box
{
#ifdef BESUI_1WIRE_EN
    BESUI_TRACE(2,"[UIBOX]%s, 0x%02X, %d", __func__, uicom.box_type, uicom.power_off2on_flag);
    if((BOX_CMD_CASE_OPEN != uicom.box_type)&&(uicom.power_off2on_flag))
    {
        uicom.box_type = BOX_CMD_CASE_OPEN;
        uictl.box_sta = 1;
        app_status_indication_set(APP_STATUS_INDICATION_POWERON);
        uicom.led_box_open_flag = true;
        app_recover_led_timer_onoff(true, false);

        besui_bat_timer_restart(); //case open battery detect retart

        app_get_history_phone_paired_num();

        app_enter_charge_flush_flash_timer_onoff(false);
        charger_param_set_onoff(false, uicom.charge_ma_val);
        app_phone_discon_pairmode_timer_onoff(false);

#if defined(ANC_APP)
        besui_anc_mode_nv_read();
#endif

        {
            app_open_box_fast_get_battery_level();
            if(app_ibrt_get_history_paired_device())
            {
                app_status_indication_set(APP_STATUS_INDICATION_PAGESCAN);
                app_ibrt_if_event_entry(APP_UI_EV_CASE_OPEN);
                osDelay(50);
                if(uicom.phone_pair_cnt > 1)
                {
                    app_bt_mobile_set_openreconnect_info(0, CONNECT_OPEN_RECON, true);
                }
                else if(uicom.phone_pair_cnt == 1)
                {
                    app_bt_mobile_set_openreconnect_info(0, CONNECT_OPEN_RECON, false);
                }

                #ifdef GFPS_ENABLED
                gfps_set_battery_datatype(SHOW_UI_INDICATION);
                #endif
            }
            else
            {
                app_poweron_twspairmode_timer_onoff(true);
            }
        }
        besui_tws_key_init();
        app_start_10_second_timer(APP_POWEROFF_TIMER_ID);
        app_poweroff_timer_onoff(true);
    }
    else if((BOX_CMD_CASE_OPEN != uicom.box_type)&&(!uicom.power_off2on_flag))
    {
        uicom.poweron_box_flag = true;
    }
#endif //#ifdef BESUI_1WIRE_EN
}

void app_uart_close_box_handle(void)
{
#ifdef BESUI_1WIRE_EN
    BESUI_TRACE(2,"[UIBOX]%s, 0x%02X, %d", __func__, uicom.box_type, uicom.power_off2on_flag);
    if((BOX_CMD_CASE_OPEN == uicom.box_type)&&(uicom.power_off2on_flag))
    {
        uicom.box_type = BOX_CMD_CASE_CLOSE;
        uictl.box_sta = 0;

#ifdef USER_APP_BLE_DIS_EN
        app_tota_switch_role_to_app();
#endif

        app_status_indication_set(APP_STATUS_INDICATION_INITIAL);

#ifdef GFPS_ENABLED
        gfps_set_battery_datatype(HIDE_UI_INDICATION);
#endif

        twsui_param_clear();

        app_siri_timer_onoff(false);
        app_poweron_twspairmode_timer_onoff(false);
        app_switch_role_poweroff_timer_onoff(false);
        app_phone_pairmode_timer_onoff(false);
        app_phone_discon_pairmode_timer_onoff(false);
        app_recover_led_timer_onoff(false, false);
        app_tws_linkloss_pairmode_timer_onoff(false);
        app_enter_charge_flush_flash_timer_onoff(true);

#ifdef BESUI_GAME_EN
#ifndef BESUI_GAME_NV_EN
        app_gamemode_off(false);
#endif
#endif
        
        app_ibrt_if_event_entry(APP_UI_EV_CASE_CLOSE);

        besui_tws_key_init();
        app_stop_10_second_timer(APP_PAIR_TIMER_ID);
        app_stop_10_second_timer(APP_POWEROFF_TIMER_ID);
        app_pairmode_timer_onoff(false);
        app_poweroff_timer_onoff(false);
#ifdef GFPS_ENABLED
        app_exit_fastpairing_mode();
#endif
    }
#endif //#ifdef BESUI_1WIRE_EN
}

void app_common_clear_other_earstatus(void)
{
    uicom.wear_peer_sta = OUTEAR_STATUS;
#ifdef BESUI_APP_EN
    uictl.ear_another_sta = 0;
#endif
}

osTimerId role_switch_delay_process_timer = NULL;
void role_switch_delay_process_timehandler(void const *param)
{
	BESUI_TRACE(1, "[UITIMER]%s", __func__);
    
    int current_device_id = app_bt_audio_get_hfp_device_for_user_action();
    struct BT_DEVICE_T* curr_device = app_bt_get_device(current_device_id);
    bt_hfp_call_setup_t   current_callSetup  = curr_device->hfchan_callSetup;
    bt_hfp_call_active_t  current_call       = curr_device->hfchan_call;
    bt_hfp_call_held_t    current_callheld   = curr_device->hf_callheld;
    bt_audio_state_t      current_audioState = curr_device->hf_audio_state;

    BESUI_TRACE(5,"[UICOM]%s, current state=%d, %d, %d, %d", __func__,current_callSetup,current_call,current_callheld,current_audioState);

    if((bts_tws_if_is_tws_link_connected())&&(TWS_UI_MASTER == app_ibrt_if_get_ui_role()))
    {
        if((uicom.wear_curr_sta == OUTEAR_STATUS) && (uicom.wear_peer_sta==INEAR_STATUS))
        {
            //if(app_bt_audio_count_connected_sco())
            {
                if(current_callSetup || current_call|| current_callheld|| current_audioState)//call
                    app_ui_user_role_switch(false);
            }
        }       
    }
}

osTimerDef (ROLE_SWITCH_DELAY_PROCESS_TIMER, (void (*)(void const *))role_switch_delay_process_timehandler);

void role_switch_delay_timer_onoff(bool timer_en)
{
	BESUI_TRACE(1, "[UITIMER]%s timer_en %d ", __func__, timer_en);

	if(role_switch_delay_process_timer == NULL)
	{
    	role_switch_delay_process_timer = osTimerCreate(osTimer(ROLE_SWITCH_DELAY_PROCESS_TIMER),osTimerOnce,NULL);
        if(!role_switch_delay_process_timer)
        {
             ASSERT(0, "[UITIMER][%s] osTimerCreate error", __func__);
        }
	}

	if(timer_en)
		osTimerStart(role_switch_delay_process_timer, 3000);
	else
		osTimerStop(role_switch_delay_process_timer);
}

void app_inoutear_common_handle(bool other_key_flag, uint8_t inoutearstatus)
{
    BESUI_TRACE(4,"[UIWEAR]%s key %d status %d cur %d other %d", __func__, other_key_flag, inoutearstatus, uicom.wear_curr_sta, uicom.wear_peer_sta);
    
    if(other_key_flag) //peer
    {
        if(bts_tws_if_is_local_left_side())
        {
            if(!nvrecord_uienv->inear_right_onoff)
            {
                BESUI_TRACE(0,"[UIWEAR]%s, inear_right_onoff", __func__);
                return;
            }
        }
        else
        {
            if(!nvrecord_uienv->inear_left_onoff && other_key_flag)
            {
                BESUI_TRACE(0,"[UIWEAR]%s, inear_left_onoff", __func__);
                return;
            }
        }
    }
    else
    {
        if(bts_tws_if_is_local_left_side())
        {
            if(!nvrecord_uienv->inear_left_onoff)
            {
                BESUI_TRACE(0,"[UIWEAR]%s, inear_left_onoff", __func__);
                return;
            }
        }
        else
        {
            if(!nvrecord_uienv->inear_right_onoff && other_key_flag)
            {
                BESUI_TRACE(0,"[UIWEAR]%s, inear_right_onoff", __func__);
                return;
            }
        }
    }

    if(other_key_flag) //slave
    {
        if(INEAR_STATUS == inoutearstatus)
        {
            uicom.wear_peer_sta = INEAR_STATUS;
            app_key_gui_inoutear_pp(true); //a2dp
#ifdef BESUI_CALL_WEAR_SWITCH
            if(uicom.wear_curr_sta == INEAR_STATUS || !bts_tws_if_is_tws_link_connected())         //sco
                app_key_gui_inoutear_call(true);
#endif
        }
        else if(OUTEAR_STATUS == inoutearstatus)
        {
            uicom.wear_peer_sta = OUTEAR_STATUS;
            app_key_gui_inoutear_pp(false);   //a2dp
#ifdef BESUI_CALL_WEAR_SWITCH
            if(uicom.wear_curr_sta == OUTEAR_STATUS)
                app_key_gui_inoutear_call(false);    //sco
#endif
        }
    }
    else
    {
        if(INEAR_STATUS == inoutearstatus)
        {
            uicom.wear_curr_sta = INEAR_STATUS;
            app_key_gui_inoutear_pp(true);   //a2dp
#ifdef BESUI_CALL_WEAR_SWITCH
            if(uicom.wear_peer_sta == INEAR_STATUS || !bts_tws_if_is_tws_link_connected())
                app_key_gui_inoutear_call(true);    //sco
#endif
        }
        else if(OUTEAR_STATUS == inoutearstatus)
        {
            uicom.wear_curr_sta = OUTEAR_STATUS;
            app_key_gui_inoutear_pp(false);   //a2dp
#ifdef BESUI_CALL_WEAR_SWITCH
            if(uicom.wear_peer_sta == OUTEAR_STATUS || !bts_tws_if_is_tws_link_connected())
                app_key_gui_inoutear_call(false);    //sco
#endif
        }
    }
#ifdef TWS_SPEECH_ROLE_SWITCH //master [off ear] switch role
    int current_device_id = app_bt_audio_get_hfp_device_for_user_action();
    struct BT_DEVICE_T* curr_device = app_bt_get_device(current_device_id);
    bt_hfp_call_setup_t   current_callSetup  = curr_device->hfchan_callSetup;
    bt_hfp_call_active_t  current_call       = curr_device->hfchan_call;
    bt_hfp_call_held_t    current_callheld   = curr_device->hf_callheld;
    bt_audio_state_t      current_audioState = curr_device->hf_audio_state;

    BESUI_TRACE(5,"[UICOM]%s, current state=%d, %d, %d, %d", __func__,current_callSetup,current_call,current_callheld,current_audioState);
    if(TWS_UI_MASTER == app_ibrt_if_get_ui_role())
    {
        if((uicom.wear_curr_sta == OUTEAR_STATUS) && (uicom.wear_peer_sta==INEAR_STATUS)) //switch role
        {
            if(current_callSetup || current_call|| current_callheld|| current_audioState)//call
                role_switch_delay_timer_onoff(true);
        }
    }
#endif
}

void peer_wear_sta_msg(uint8_t inoutear_status)
{
    BESUI_TRACE(0, "[UIWEAR]%s, inoutear_status = %d", __func__, inoutear_status);
    if(inoutear_status == INEAR_STATUS)
    {
        app_key_gui_post_msg(true, APP_KEY_CODE_FN1, APP_KEY_EVENT_ON_EAR);
    }
    else if(inoutear_status == OUTEAR_STATUS)
    {
        app_key_gui_post_msg(true, APP_KEY_CODE_FN1, APP_KEY_EVENT_OFF_EAR);
    }
}


#if defined(__VIRTUAL_SURROUND__) || defined(__VIRTUAL_SURROUND_STEREO__)
extern "C" int32_t stereo_surround_status;
extern "C" int32_t audio_process_stereo_surround_onoff(int32_t onoff);

void besspa_key_loop_switch(void)
{
    uint8_t BES_SPATIAL_STATUS = 0;
    if(!bts_tws_if_is_tws_link_connected())
        return;

    BES_SPATIAL_STATUS = !(besspa_audio_onoff_get());
#ifdef BESSPA_ONOFF_EN
    algo_send_request(ALGO_ID_BESSPA, 2-BES_SPATIAL_STATUS, 0, 0, 0, 0);
#endif
    BESUI_TRACE(0, "[UIALGO]%s, spa_sta = %d", __func__, BES_SPATIAL_STATUS);
}

void besspa_nv_init(void)
{
    nv_record_env_get(&nvrecord_uienv);

    if(nvrecord_uienv->space_audio_type == 1)
        stereo_surround_status = 1;
    else
        stereo_surround_status = 0;

#ifdef BESSPA_ONOFF_EN
    if(stereo_surround_status)
        algo_send_request(ALGO_ID_BESSPA, ALGO_ON, 0, 0, 0, 0);
    else 
        algo_send_request(ALGO_ID_BESSPA, ALGO_OFF, 0, 0, 0, 0);
#endif
    BESUI_TRACE(1,"[UICOM]%s space audio %d", __func__, nvrecord_uienv->space_audio_type);
}
#endif

osTimerId app_led2_level_detect_timer = NULL;
static uint8_t app_led2_low_cnt = 0;
static uint8_t app_led2_high_cnt = 0;
#define LED2_DETECT_CNT   6

void app_led2_putin_putout_detect_init(void)
{
#ifdef USER_BURN_ADDR_PAIR_EN
    return;
#endif
    uint8_t box_high_level_cnt = 0;
    uint8_t box_low_level_cnt = 0;
    uint8_t level = hal_gpio_pin_get_val((enum HAL_GPIO_PIN_T)HAL_IOMUX_PIN_LED2);

    while(1)
    {
        level = hal_gpio_pin_get_val((enum HAL_GPIO_PIN_T)HAL_IOMUX_PIN_LED2);

        BESUI_TRACE(1,"[UICOM]putinout level %d",level);

        if (level)
        {
            box_high_level_cnt++;
            box_low_level_cnt = 0;
        }
        else
        {
            box_low_level_cnt++;
            box_high_level_cnt = 0;
        }

        if((box_high_level_cnt >= 5)||(box_low_level_cnt >= 5))
        {
            break;
        }

        osDelay(10);
    }
    if(uictl.user_factory_flag == true)
    {
        uictl.user_factory_flag = false;
        uicom.poweron_box_flag = true;

#ifdef BESUI_BOX_PUTINOUT_EN
        uicom.box_put_sta = PUTINBOX_STATUS;
#endif
        uicom.key_func_onoff = true;
        app_set_speaker_mute_status(true);
#ifdef BESUI_1WIRE_EN
        communication_uart_function_enable(true);
#endif
    }
    else
    {
        if(box_high_level_cnt >= 5)
        {
#ifdef BESUI_BOX_PUTINOUT_EN
            uicom.box_put_sta = PUTINBOX_STATUS;
#endif
            uicom.key_func_onoff = true;
            app_set_speaker_mute_status(true);
#ifdef BESUI_1WIRE_EN
            communication_uart_function_enable(true);
#endif
        }
        else if(box_low_level_cnt >= 5)
        {
#ifdef BESUI_BOX_PUTINOUT_EN
            uicom.box_put_sta = PUTOUTBOX_STATUS;
#endif
            uicom.key_func_onoff = false;
            app_set_speaker_mute_status(false);
#ifdef BESUI_1WIRE_EN
            communication_uart_function_enable(false);
#endif

            uicom.poweron_box_flag = true;
            BESUI_TRACE(0, "[UIBOX]%s, uicom.poweron_box_flag = true", __func__);
        }
    }
#ifdef BESUI_BOX_PUTINOUT_EN
    BESUI_TRACE(1,"[UICOM]%s putinout_box_status %d", __func__, uicom.box_put_sta);
#endif
}

void app_led2_level_detect_process_timehandler(void const *param) //LED2 == UART_COM
{
    uint8_t level = hal_gpio_pin_get_val((enum HAL_GPIO_PIN_T)HAL_IOMUX_PIN_LED2);

    //BESUI_TRACE(1,"[UITIMER]%s level %d putinputout %d", __func__, level, uicom.box_put_sta);
#if 1 //debounce
    static uint8_t level_cnt = 0;
    static uint8_t level_buf = 0xff;
    if(level_buf != level)
    {
        if(level_cnt < 5) //50ms * 5
        {
            level_cnt ++;
            return;
        }
        BESUI_TRACE(0, "[UITIMER]%s, level = %d, putinout = %d", __func__, level, uicom.box_put_sta);
    }
    level_buf = level;
#endif

    if(uicom.box_put_sta == UNKNOW_STATUS)
    {
        if(level == 1)
        {
            app_led2_high_cnt++;
            app_led2_low_cnt = 0;
            if(app_led2_high_cnt >= LED2_DETECT_CNT)
            {
                app_led2_high_cnt = 0;
                uicom.box_put_sta = PUTINBOX_STATUS;
                app_set_speaker_mute_status(true);
            }
        }
        else
        {
            app_led2_low_cnt++;
            app_led2_high_cnt = 0;
            if(app_led2_low_cnt >= LED2_DETECT_CNT)
            {
                app_led2_low_cnt = 0;
                uicom.box_put_sta = PUTOUTBOX_STATUS;
                app_set_speaker_mute_status(false);
            }
        }
    }
    else if(uicom.box_put_sta == PUTOUTBOX_STATUS)
    {
        if(level == 1)
        {
            app_led2_high_cnt++;
            app_led2_low_cnt = 0;
            if(app_led2_high_cnt >= LED2_DETECT_CNT)
            {
                app_led2_high_cnt = 0;
                uicom.box_put_sta = PUTINBOX_STATUS;
                app_charge_putinout_ui_post_msg(PUTIN_BOX);
            }
        }
        else
        {
            app_led2_low_cnt = 0;
            app_led2_high_cnt = 0;
        }
    }
    else if(uicom.box_put_sta == PUTINBOX_STATUS)
    {
        if(level == 1)
        {
            app_led2_low_cnt = 0;
            app_led2_high_cnt = 0;
        }
        else
        {
            app_led2_low_cnt++;
            app_led2_high_cnt = 0;
            if(app_led2_low_cnt >= LED2_DETECT_CNT)
            {
                app_led2_low_cnt = 0;
                uicom.box_put_sta = PUTOUTBOX_STATUS;
                app_charge_putinout_ui_post_msg(PUTOUT_BOX);
            }
        }
    }
}

osTimerDef (APP_LED2_LEVEL_DETECT_PROCESS_TIMER, (void (*)(void const *))app_led2_level_detect_process_timehandler);
void app_led2_level_detect_timer_onoff(bool timer_en)
{
	BESUI_TRACE(1, "[UITIMER]%s timer_en %d ", __func__, timer_en);

	if(app_led2_level_detect_timer == NULL)
	{
    	app_led2_level_detect_timer = osTimerCreate(osTimer(APP_LED2_LEVEL_DETECT_PROCESS_TIMER),osTimerPeriodic,NULL);
	}

	if(timer_en)
	{
        osTimerStart(app_led2_level_detect_timer, 50);
	}
	else
	{
		osTimerStop(app_led2_level_detect_timer);
	}
}

osTimerId app_reject_call_delay_voice_process_timer = NULL;
void app_reject_call_delay_voice_process_timehandler(void const *param)
{
    BESUI_TRACE(1,"[UITIMER]%s voice_type %d", __func__, uicom.call_prompt_type);
    if(uicom.call_prompt_type == REJECT_CALL_TYPE)
    {
        media_PlayAudio(AUD_ID_BT_CALL_REFUSE, 0);
    }
    else if(uicom.call_prompt_type == END_CALL_TYPE)
    {
        media_PlayAudio(AUD_ID_BT_CALL_HUNG_UP, 0);
    }

    uicom.call_prompt_type = 0;

#if defined(ANC_APP)
    app_common_tws_open_close_anc(true, false);
#endif
    uictl.callend_mute_flag = false;
}

osTimerDef (APP_REJECT_CALL_DELAY_VOICE_PROCESS_TIMER, (void (*)(void const *))app_reject_call_delay_voice_process_timehandler);
void app_reject_call_voice_timer_onoff(bool timer_en)
{
	BESUI_TRACE(1, "[UITIMER]%s timer_en %d ", __func__, timer_en);

	if(app_reject_call_delay_voice_process_timer == NULL)
	{
    	app_reject_call_delay_voice_process_timer = osTimerCreate(osTimer(APP_REJECT_CALL_DELAY_VOICE_PROCESS_TIMER),osTimerOnce,NULL);
        if(!app_reject_call_delay_voice_process_timer)
        {
             ASSERT(0, "[UITIMER][%s] osTimerCreate error", __func__);
        }
    }

	if(timer_en)
	{
        osTimerStart(app_reject_call_delay_voice_process_timer, 300);
        uictl.callend_mute_flag = true;
	}
	else
	{
		osTimerStop(app_reject_call_delay_voice_process_timer);
	}
}

osTimerId app_clear_pairlist_delay_poweroff_process_timer = NULL;
void app_clear_pairlist_delay_poweroff_process_timehandler(void const *param)
{
    uictl.poweroff_fast_flag = true;
    app_common_poweroff_process(false, CLEARPAIRLIST_POWEROFF);

    osTimerDelete(app_clear_pairlist_delay_poweroff_process_timer);
    app_clear_pairlist_delay_poweroff_process_timer = NULL;
    BESUI_TRACE(1,"[UITIMER]%s, osTimerDelete", __func__);
}

osTimerDef (APP_CLEAR_PAIRLIST_DELAY_POWEROFF_PROCESS_TIMER, (void (*)(void const *))app_clear_pairlist_delay_poweroff_process_timehandler);

void app_clear_pairlist_poweroff_timer_onoff(bool timer_en)
{
	BESUI_TRACE(1, "[UITIMER]%s timer_en %d ", __func__, timer_en);

	if(app_clear_pairlist_delay_poweroff_process_timer == NULL)
	{
    	app_clear_pairlist_delay_poweroff_process_timer = osTimerCreate(osTimer(APP_CLEAR_PAIRLIST_DELAY_POWEROFF_PROCESS_TIMER),osTimerOnce,NULL);
        if(!app_clear_pairlist_delay_poweroff_process_timer)
        {
             ASSERT(0, "[UITIMER][%s] osTimerCreate error", __func__);
        }
    }

	if(timer_en)
	{
        osTimerStart(app_clear_pairlist_delay_poweroff_process_timer, 2200);
	}
	else
	{
		osTimerStop(app_clear_pairlist_delay_poweroff_process_timer);
	}
}


osTimerId app_uart_factory_poweroff_timer_id = NULL;
void app_uart_factory_poweroff_timehandler(void const *param)
{
#ifdef BESUI_1WIRE_EN
	BESUI_TRACE(1, "[UITIMER]%s, %d", __func__, uiuart.twspair_flag);
    if(uiuart.twspair_flag)
    {
        hal_sw_bootmode_clear(USER_BOOTMODE_FACTORY); //single wire mode
        hal_sw_bootmode_set(USER_BOOTMODE_FACTORY);
        app_reset();
    }
    else
    {
        uictl.shutdown_type = SHUTDOWN_FACTORY_RESET;
        app_shutdown();
    }
#endif
}

osTimerDef (APP_UART_FACTORY_POWEROFF_TIMER_NAME, (void (*)(void const *))app_uart_factory_poweroff_timehandler);

void app_uart_factory_poweroff_timer_onoff(bool timer_en)
{
	BESUI_TRACE(1, "[UITIMER]%s, %d ", __func__, timer_en);

	if(app_uart_factory_poweroff_timer_id == NULL)
	{
    	app_uart_factory_poweroff_timer_id = osTimerCreate(osTimer(APP_UART_FACTORY_POWEROFF_TIMER_NAME),osTimerOnce,NULL);
        if(!app_uart_factory_poweroff_timer_id)
             ASSERT(0, "[UITIMER][%s] osTimerCreate error", __func__);
    }

	if(timer_en)
        osTimerStart(app_uart_factory_poweroff_timer_id, 2200);
	else
		osTimerStop(app_uart_factory_poweroff_timer_id);
}

osTimerId app_recover_led_timerid = NULL;
void app_recover_led_timehandler(void const *param)
{
	BESUI_TRACE(0, "[UITIMER]%s uicom.led_recover_last_flag %d", __func__, uicom.led_recover_last_flag);
	
    if(uicom.led_recover_last_flag)
    {
        uicom.led_recover_last_flag = false;

        if(uicom.led_box_open_flag)
        {
            BESUI_TRACE(0,"[UITIMER]battery level %d", app_battery_current_level());
            if(app_battery_current_level() >= 2)
                app_status_indication_set(APP_STATUS_INDICATION_POWERON_BATTERY_DISPLAY);
            else
                app_status_indication_set(APP_STATUS_INDICATION_POWERON_LOWBATTERY_DISPLAY);
            app_recover_led_timer_onoff(true, false);
            uicom.led_box_open_flag = false;
        }
        else
        {
            if(uicom.led_bt_conn_flag)
            {
                uicom.led_bt_conn_flag = false;
                app_status_indication_set(APP_STATUS_INDICATION_PHONE_CONNECTED);
                app_recover_led_timer_onoff(true, true);
            }
            else
            {
                BESUI_TRACE(0,"[UITIMER]led last status %d", app_last_status_indication_get());
                if(app_last_status_indication_get() == APP_STATUS_INDICATION_NUM)
                    app_status_indication_set(APP_STATUS_INDICATION_INITIAL);
                else
                    app_status_indication_set(app_last_status_indication_get());
            }
        }
    }
}

osTimerDef (APP_RECOVER_LED_TIMER_NAME, (void (*)(void const *))app_recover_led_timehandler);

void app_recover_led_timer_onoff(bool timer_en, bool time_long)
{
	BESUI_TRACE(0, "[UITIMER]%s timer_en %d ", __func__, timer_en);

    if((uicom.led_recover_last_flag)&&(!besui_bat_charge_sta_get()))
    {
        BESUI_TRACE(0,"[UITIMER]open recover timer, do not open again");
        return;
    }

	if(app_recover_led_timerid == NULL)
	{
    	app_recover_led_timerid = osTimerCreate(osTimer(APP_RECOVER_LED_TIMER_NAME),osTimerOnce,NULL);
        if(!app_recover_led_timerid)
            ASSERT(0, "[UITIMER][%s] osTimerCreate error", __func__);
    }

	if(timer_en)
	{
        uicom.led_recover_last_flag = true;
        if(time_long)
        {
            osTimerStart(app_recover_led_timerid, 3400);
        }
        else
        {
            if(uicom.led_box_open_flag)
                osTimerStart(app_recover_led_timerid, 1700);
            else
		        osTimerStart(app_recover_led_timerid, 1200);
        }
	}
	else
	{
        uicom.led_recover_last_flag = false;
		osTimerStop(app_recover_led_timerid);
	}
}

osTimerId app_pairmode_timeout_timer_id = NULL;
uint8_t pairmode_poweroff_timeout = 0;
#define PAIRMODE_SHUTDOWN_COUNT     30//5 min
void app_pairmode_timeout_timehandler(void const *param)
{
    uint8_t conn_devices = besui_get_profile_conn_num();//app_bt_count_connected_device();

    pairmode_poweroff_timeout ++;
	BESUI_TRACE(0, "[UITIMER]%s, conn_devices = %d, timeout = %d", __func__, conn_devices, pairmode_poweroff_timeout);
    if(pairmode_poweroff_timeout >= PAIRMODE_SHUTDOWN_COUNT)
    {
        pairmode_poweroff_timeout = 0;
        besui_bt_msg_put(EXIT_PHONEMODE_EVENT, 0xff, BT_DEVICE_NUM);
    }
}

osTimerDef (APP_PAIRMODE_TIMEOUT_TIMER_NAME, (void (*)(void const *))app_pairmode_timeout_timehandler);

void app_pairmode_timer_onoff(bool timer_en)
{
#if !defined(PAIRMODE_AUTO_POWEROFF_EN)
    return;
#endif
// #ifdef TOTA_v2
//     if(tota_get_connect_status() == TOTA_SHAKE_HANDED && uictl.spp_conn_type != true) //connect "anc_tool" or "log_dump"
//     {
//         return;
//     }
// #endif
	BESUI_TRACE(0, "[UITIMER]%s timer_en %d ", __func__, timer_en);
    
	if(app_pairmode_timeout_timer_id == NULL)
	{
    	app_pairmode_timeout_timer_id = osTimerCreate(osTimer(APP_PAIRMODE_TIMEOUT_TIMER_NAME),osTimerPeriodic,NULL);
	}

    pairmode_poweroff_timeout = 0;
	if(timer_en)
        osTimerStart(app_pairmode_timeout_timer_id, 10000);
	else
		osTimerStop(app_pairmode_timeout_timer_id);
}

osTimerId app_poweroff_timerid = NULL;
uint8_t app_poweroff_cnt = 0;
#define POWEROFF_SHUTDOWN_COUNT     30//5 min
void app_poweroff_timehandler(void const *param)
{
    uint8_t conn_devices = besui_get_profile_conn_num();//app_bt_count_connected_device();

    app_poweroff_cnt ++;
	BESUI_TRACE(0, "[UITIMER]%s conn_devices %d timeout %d", __func__, conn_devices, app_poweroff_cnt);
    if(app_poweroff_cnt >= POWEROFF_SHUTDOWN_COUNT)
    {
        app_poweroff_cnt = 0;
        besui_bt_msg_put(EXIT_PHONEMODE_EVENT, 0xff, BT_DEVICE_NUM);
    }
}

osTimerDef (APP_POWEROFF_TIMER_NAME, (void (*)(void const *))app_poweroff_timehandler);

void app_poweroff_timer_onoff(bool timer_en)
{
	BESUI_TRACE(0, "[UITIMER]%s timer_en %d ", __func__, timer_en);

	if(app_poweroff_timerid == NULL)
    	app_poweroff_timerid = osTimerCreate(osTimer(APP_POWEROFF_TIMER_NAME),osTimerPeriodic,NULL);

    app_poweroff_cnt = 0;
	if(timer_en)
        osTimerStart(app_poweroff_timerid, 10000);
	else
		osTimerStop(app_poweroff_timerid);
}

//---------------------------------------------------------------------------------------
#ifdef USER_EXT_VOLTAGE_DET_EN
void ext_voltage_irqhandler(uint16_t irq_val, HAL_GPADC_MV_T volt)
{
    //uint32_t meanVolt = 0;
    //BESUI_TRACE(0, "[UIEXT]%s, irq_val=%d, volt=%d",__func__, irq_val, volt);

    if (volt == HAL_GPADC_BAD_VALUE)
        return;
    pmu_ntc_capture_disable();

    BESUI_TRACE(0, "[UIEXT]%s, ext_volt=%d",__func__, volt);
}

void besui_ext_voltage_adc_config(void)
{
    pmu_ntc_capture_enable();
    hal_gpadc_open(HAL_GPADC_CHAN_2, HAL_GPADC_ATP_ONESHOT, ext_voltage_irqhandler);
}
#endif
//---------------------------------------------------------------------------------------

void customer_ui_timer_delete(void)
{
	BESUI_TRACE(0, "%s", __func__);

    osTimerDelete(app_poweroff_timerid);
    app_poweroff_timerid = NULL;
    osTimerDelete(app_pairmode_timeout_timer_id);
    app_pairmode_timeout_timer_id = NULL;
    osTimerDelete(app_reject_call_delay_voice_process_timer);
    app_reject_call_delay_voice_process_timer = NULL;
    osTimerDelete(app_after_switch_role_poweroff_process_timer);
    app_after_switch_role_poweroff_process_timer = NULL;
    osTimerDelete(app_clear_pairlist_delay_poweroff_process_timer);
    app_clear_pairlist_delay_poweroff_process_timer = NULL;
}

void besui_common_timer_all_delete(void)
{
	BESUI_TRACE(0, "%s", __func__);

    osTimerDelete(app_after_switch_role_poweroff_process_timer);
    app_after_switch_role_poweroff_process_timer = NULL;

    osTimerDelete(app_reject_call_delay_voice_process_timer);
    app_reject_call_delay_voice_process_timer = NULL;
    osTimerDelete(app_clear_pairlist_delay_poweroff_process_timer);
    app_clear_pairlist_delay_poweroff_process_timer = NULL;
    osTimerDelete(app_uart_factory_poweroff_timer_id);
    app_uart_factory_poweroff_timer_id = NULL;

    osTimerDelete(app_recover_led_timerid);
    app_recover_led_timerid = NULL;
    osTimerDelete(app_pairmode_timeout_timer_id);
    app_pairmode_timeout_timer_id = NULL;
    osTimerDelete(app_poweroff_timerid);
    app_poweroff_timerid = NULL;

#ifdef BESUI_NTC_EN
    osTimerDelete(app_ntc_open_process_timer);
    app_ntc_open_process_timer = NULL;
#endif
}
#endif
