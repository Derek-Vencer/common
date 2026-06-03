#ifndef __TWSUI_CHARGE_H__
#define __TWSUI_CHARGE_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef BESUI_TWS_EN

#ifdef BESUI_CHARGE_EN
#include "stdint.h"
#define POWERON_SEND            0x80


#if defined(CHIP_BEST1501P) || defined(CHIP_BEST1502X) || defined(CHIP_BEST1501)
enum CHARGER_CHARGE_STATUE_E {
    CHARGER_CHARGE_STATUS_IDLE    = 0,
    CHARGER_CHARGE_STATUS_TRICKLE = 1,
    CHARGER_CHARGE_STATUS_FAST    = 2,
    CHARGER_CHARGE_STATUS_PRE     = 3,
    CHARGER_CHARGE_STATUS_FAULT   = 5,
    CHARGER_CHARGE_STATUS_OFF     = 6,
    CHARGER_CHARGE_STATUS_DONE    = 7,

    CHARGER_CHARGE_STATUS_QTY,
};
enum{
    CHARGER_CHARGE_CONSTANT_CURRENT_20MA  = 0,
    CHARGER_CHARGE_CONSTANT_CURRENT_40MA  = 1,
    CHARGER_CHARGE_CONSTANT_CURRENT_60MA  = 2, // default
    CHARGER_CHARGE_CONSTANT_CURRENT_80MA  = 3,
    CHARGER_CHARGE_CONSTANT_CURRENT_100MA = 4,
    CHARGER_CHARGE_CONSTANT_CURRENT_120MA = 5,
    CHARGER_CHARGE_CONSTANT_CURRENT_140MA = 6,
    CHARGER_CHARGE_CONSTANT_CURRENT_160MA = 7,
    CHARGER_CHARGE_CONSTANT_CURRENT_180MA = 8,
    CHARGER_CHARGE_CONSTANT_CURRENT_200MA = 9,
    CHARGER_CHARGE_CONSTANT_CURRENT_220MA = 10,
    CHARGER_CHARGE_CONSTANT_CURRENT_240MA = 11,
    CHARGER_CHARGE_CONSTANT_CURRENT_260MA = 12,
    CHARGER_CHARGE_CONSTANT_CURRENT_280MA = 13,
    CHARGER_CHARGE_CONSTANT_CURRENT_300MA = 14,
    CHARGER_CHARGE_CONSTANT_CURRENT_320MA = 15,

    CHARGER_CHARGE_CONSTANT_CURRENT_QTY,
};
enum CHARGER_CHARGE_STATUE_E charger_charge_status_get(void);
#endif

#ifdef BESUI_BOX_PUTINOUT_EN
void app_putin_putout_box_init(void);
void app_putin_putout_box_irq_set(bool irq_enable, uint8_t current_box_status);
void app_putinout_box_event_process(uint8_t putinputout_event, bool phone_con_flag);
void app_putinout_scan_timer_onoff(bool timer_en);
#endif

void app_charge_putinout_ui_modual_init(void);

void besui_bat_full_sta_set(bool param);
bool besui_bat_full_sta_get(void);
bool besui_bat_charge_sta_get(void);
void besui_bat_charge_sta_set(bool param);
void app_poweron_enter_charge_process(uint8_t poweron_type);
void app_charge_putinout_ui_post_msg(uint32_t charge_putinout_wear_event);
void app_poweron_twspairmode_timer_onoff(bool timer_en);

// enum CHARGER_CHARGE_STATUE_E charger_charge_status_get(void);
void charger_param_set_onoff(bool open_close_flag, uint8_t battery_current);

#endif //#ifdef BESUI_CHARGE_EN

void besui_delay_putinout_timer_onoff(bool timer_en);
void app_poweron_init_send_msg_timer_onoff(bool timer_en);
void app_enter_charge_flush_flash_timer_onoff(bool timer_en);
#if defined(USER_CHARGE_EXT_EN)
bool twsui_box_charge_full_shutdown(void);
#endif
#endif //#ifdef BESUI_TWS_EN

#ifdef __cplusplus
}
#endif

#endif