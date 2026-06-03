#ifndef __TWSUI_COMM_H__
#define __TWSUI_COMM_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef BESUI_TWS_EN
#include "stdint.h"



#define EARSTA_A2DP_MODE                    0x00
#define EARSTA_INCOMINGCALL                 0x01
#define EARSTA_OUTCALL_MODE                 0x02
#define EARSTA_ACTIVECALL                   0x03
#define EARSTA_AUDIO_CONNECTED              0x04//siri or di san fang yuyin software
#define EARSTA_LINKLOSS_END                 0x05

#define DEFAULT_MODE                        0xff
#define VOICE_PLAY_START                    0x00
#define VOICE_PLAY_STOP                     0x01
#define BAT_LOW_PROMPT_CNT                  30
#define LED_DISPLAY_SPACE                   3

#ifdef BESUI_GAME_EN
#define LATENCY_OFF                         0x00
#define LATENCY_ON                          0x01
#endif

#define ONLY_OPEN_MASTER_MIC                0x00
#define ONLY_OPEN_SLAVE_MIC                 0x01
#define OPEN_MASTER_SLAVE_MIC               0x02
#define CLOSE_MASTER_SLAVE_MIC              0x03

#if defined(ANC_APP)
#define ANC_TEST_FF                         0x01
#define ANC_TEST_FB                         0x02
#define ANC_TEST_TALK                       0x03
#endif

#define SPEECH_TEST_MIC0                    0x01
#define SPEECH_TEST_MIC1                    0x02
#define SPEECH_TEST_MIC2                    0x03

// #if !defined(CHIP_BEST1306) && !defined(CHIP_BEST1503)
// enum{
//     CHARGER_CHARGE_CONSTANT_CURRENT_20MA  = 0,
//     CHARGER_CHARGE_CONSTANT_CURRENT_40MA  = 1,
//     CHARGER_CHARGE_CONSTANT_CURRENT_60MA  = 2, // default
//     CHARGER_CHARGE_CONSTANT_CURRENT_80MA  = 3,
//     CHARGER_CHARGE_CONSTANT_CURRENT_100MA = 4,
//     CHARGER_CHARGE_CONSTANT_CURRENT_120MA = 5,
//     CHARGER_CHARGE_CONSTANT_CURRENT_140MA = 6,
//     CHARGER_CHARGE_CONSTANT_CURRENT_160MA = 7,
//     CHARGER_CHARGE_CONSTANT_CURRENT_180MA = 8,
//     CHARGER_CHARGE_CONSTANT_CURRENT_200MA = 9,
//     CHARGER_CHARGE_CONSTANT_CURRENT_220MA = 10,
//     CHARGER_CHARGE_CONSTANT_CURRENT_240MA = 11,
//     CHARGER_CHARGE_CONSTANT_CURRENT_260MA = 12,
//     CHARGER_CHARGE_CONSTANT_CURRENT_280MA = 13,
//     CHARGER_CHARGE_CONSTANT_CURRENT_300MA = 14,
//     CHARGER_CHARGE_CONSTANT_CURRENT_320MA = 15,

//     CHARGER_CHARGE_CONSTANT_CURRENT_QTY,
// };
// #endif

//------------------------------------------------------------------------------------------------------------
typedef struct {
    uint8_t bat_curr_level;
    uint8_t bat_peer_level;

    uint8_t bat_box_percent;
    bool    bat_low_prompt_flag;
    uint8_t bat_low_prompt_sta;
    uint8_t bat_low_prompt_cnt;
    uint8_t bat_low_led_cnt;

    bool    poweron_bat_det_flag;
    bool    poweron_box_flag;
    bool    power_off2on_flag;

    uint8_t box_type;
    bool    box_open_bat_det_flag;
    uint8_t box_put_sta;
    // uint8_t box_rx_open;

    uint8_t wear_curr_sta;
    uint8_t wear_peer_sta;

    bool    key_func_onoff;
    bool    bt_pairlist_clear_flag;

    uint8_t charge_ma_val;

    uint8_t phone_pair_cnt;

    bool    led_box_open_flag;
    bool    led_recover_last_flag;
    bool    led_bt_conn_flag;

    uint8_t call_prompt_type;
    uint8_t call_end_prompt_type;
    uint8_t call_end_type;
    uint8_t call_incoming_flag;
    uint8_t call_out_flag;

    uint8_t ear_lr_side;

#ifdef BESUI_GAME_EN
    uint8_t latency_mode;

    uint8_t latency_delay_aac_mtu;
    uint8_t latency_delay_aac_us;
    uint8_t latency_max_waterline_aac_mtu;
    uint8_t latency_min_waterline_aac_mtu;

    uint8_t latency_delay_sbc_mtu;
    uint8_t latency_delay_sbc_us;
    uint8_t latency_delay_sbc_frame_mtu;
    uint8_t latency_max_waterline_sbc_mtu;
    uint8_t latency_min_waterline_sbc_mtu;
#endif
    bool ship_mode_flag;

    bool poweron_flag;
} type_uicom_t;
extern type_uicom_t uicom;

//------------------------------------------------------------------------------------------------------------
#if defined(ANC_APP)
void anc_ff_fb_set_onoff(uint8_t sta);
uint8_t anc_ff_fb_get_onoff(void);
#endif

void speech_mic_set_onoff(uint8_t sta);
uint8_t speech_mic_get_onoff(void);

#ifdef BESUI_LR_IODET_EN
void besui_fixed_lr_det_init(void);
#endif
uint8_t besui_get_lr_sta(void);

void app_call_varible_init(void);
void twsui_param_clear(void);
void twsui_param_init(void);
uint8_t app_battery_level_compare(void);
void app_battery_low_voice_play_process(void);
bool app_ibrt_get_history_paired_device(void);
void app_open_box_fast_get_battery_level(void);
bool app_get_history_phone_paired_device(void);
void app_poweroff_shutdown_ui_modual_init(void);
#ifdef BATTERY_SWITCH_ROLE_EN
void besui_battery_role_switch(void);
#endif

bool    app_get_mic_hfp_enc_onoff(void);
uint8_t app_get_software_version_high(void);
uint8_t app_get_software_version_middle(void);
uint8_t app_get_software_version_low(void);

bool    app_get_speaker_mute_hfp_status(void);
bool    app_get_speaker_mute_a2dp_status(void);
bool    app_get_speaker_mute_voice_status(void);
void    app_set_speaker_mute_status(bool mute_flag);
uint8_t app_get_mic_hfp_enc_test_type(void);

void    app_get_history_phone_paired_num(void);


void app_callsetup_ind_handle_process(uint8_t device_id);
void app_call_connected_handle_process(uint8_t device_id);
void app_battery_low_voice_enable_set(uint8_t battery_level);
uint8_t app_battery_level_tran_process(uint16_t battery_volt);
void app_battery_set_other_battery_level(uint8_t battery_level);
void app_tws_battery_update(bool tws_connect_flag);
void app_call_end_handle_process(uint8_t device_id, uint8_t endcall_type);
void app_tws_ledsta_to_slave_process(uint8_t connect_num, uint8_t led_type);
void app_common_poweroff_process(bool slave_key_flag, uint8_t poweroff_type);
void app_common_mic_hfp_enc_control(bool enc_open_flag, uint8_t open_mic_type);
void app_poweroff_shutdown_ui_post_msg(bool slave_key_flag, uint8_t poweroff_type);
void app_call_discon_process(uint8_t device_id, uint8_t calldiscon_type);
void app_tws_set_ledsta_call_slave_process(uint8_t led_type, uint8_t call_status);
void app_led2_putin_putout_detect_init(void);
bool app_current_box_status_open_status(void);
bool app_current_box_status_close_status(void);
void app_uart_open_box_handle(void);
void app_uart_close_box_handle(void);
void app_common_clear_other_earstatus(void);
void app_inoutear_common_handle(bool other_key_flag, uint8_t inoutearstatus);
void peer_wear_sta_msg(uint8_t inoutear_status);
void app_remove_all_paired_list(void);
void app_remove_all_phone_paired_list(void);
void app_discon_enter_pairmode(void);

#ifdef BESUI_GAME_EN
#ifdef BESUI_GAME_NV_EN
void app_gamemode_nv_write(uint8_t param);
#endif
void app_gamemode_off(bool param);
void app_gamemode_set_param(uint8_t gamemode_type);
void app_gamemode_key_switch(bool switch_flag, uint8_t game_mode);
void besui_gamemode_key_switch(bool switch_flag, uint8_t current_set_gamemode, bool voice_flag);
void app_gamemode_voice_timer_onoff(bool timer_en);
uint8_t besui_latency_mode_get(void);
void gamemode_timer_onoff(bool timer_en);
#endif

#ifdef GFPS_ENABLED
uint8_t app_gfps_renew_battery_level(uint8_t charge_status, uint8_t *batteryLevel);
#endif

#ifdef BESUI_NTC_EN
void app_ntc_detect_init(void);
void app_ntc_detect_volt_timer_onoff(bool timer_en);
void app_ntc_detect_process(uint16_t ad_volt);
bool ntc_charger_status(void);
#endif

void app_common_store_twsaddrto_nv_flash(uint8_t *btaddr);
bool app_charge_fast_exit_factory_mode(void);
void app_common_clear_pairlist_process(bool clear_tws_flag, bool clear_phone_flag, bool force_only_flag, bool uart_cmd_flag);

#if defined(ANC_APP)
void user_anc_set_judge(bool sta);
bool user_anc_get_judge(void);
int32_t besui_anc_set_mode(uint8_t anc_mode);
void besui_set_curr_anc_mode(uint8_t anc_mode);
uint8_t besui_get_curr_anc_mode(void);
void app_tws_sync_anc_mode(uint8_t anc_mode);
void app_anc_open_close(bool open_close, bool store_last_flag);

void besui_anc_key_switch(void);
void besui_anc_tws_sync(void);
void besui_anc_openbox_tws_sync(void);
void besui_anc_mode_nv_read(void);
void besui_anc_mode_nv_write(void);
void app_common_tws_open_close_anc(bool open_close_anc, bool store_last_anc_mode_flag);
#endif

#if defined(__VIRTUAL_SURROUND__) || defined(__VIRTUAL_SURROUND_STEREO__)
void besspa_nv_init(void);
void besspa_key_loop_switch(void);
#endif

void app_led2_level_detect_timer_onoff(bool timer_en);
void app_reject_call_voice_timer_onoff(bool timer_en);
void app_switch_role_poweroff_timer_onoff(bool timer_en);
void app_clear_pairlist_poweroff_timer_onoff(bool timer_en);
void app_uart_factory_poweroff_timer_onoff(bool timer_en);
void app_recover_led_timer_onoff(bool timer_en, bool time_long);
void app_pairmode_timer_onoff(bool timer_en);
void app_poweroff_timer_onoff(bool timer_en);

uint8_t twsui_get_bat_level(void);

#ifdef USER_EXT_VOLTAGE_DET_EN
void besui_ext_voltage_adc_config(void);
#endif

void customer_ui_timer_delete(void);
void besui_common_timer_all_delete(void);
#endif

#ifdef __cplusplus
}
#endif

#endif //__TWSUI_COMM_H__
