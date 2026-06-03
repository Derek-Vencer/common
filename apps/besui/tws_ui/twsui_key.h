#ifndef __TWSUI_KEY_H__
#define __TWSUI_KEY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

#ifdef BESUI_TWS_EN

#ifdef BESUI_KEY_EN

void besui_tws_key_init(void);

void app_key_gui_modual_init(void);
void app_key_gui_to_dut_test(void);
void app_key_gui_to_otaboot_or_single(bool ota_flag, bool single_flag);
void app_key_gui_click_process(bool slave_key_flag);
void app_key_gui_longpress_process(bool slave_key_flag);
void app_key_gui_ultraclick_process(bool slave_key_flag);
void app_key_gui_doubleclick_process(bool slave_key_flag);
void app_key_gui_tripleclick_process(bool slave_key_flag);
void app_key_gui_rampageclick_process(bool slave_key_flag);
void app_key_gui_longlongpress_process(bool slave_key_flag);
void app_key_gui_inoutear_pp(bool play_flag);
void app_key_gui_inoutear_call(bool answer_flag);
void app_pwr_key_gui_handler(bool slave_key_flag, uint8_t key_event);
void app_fn1_key_gui_handler(bool other_key_flag, uint8_t key_event);
void app_key_gui_post_msg(bool slave_key_flag, uint32_t key_code, uint8_t key_event);

void app_left_key_gui_click(void);
void app_left_key_gui_doubleclick(void);

void app_siri_timer_onoff(bool timer_en);

#if defined(CAPSENSOR_SLIDE)
void app_left_key_gui_upslide(void);
void app_right_key_gui_upslide(void);
void app_key_gui_upslide_process(bool slave_key_flag);
void app_left_key_gui_downslide(void);
void app_right_key_gui_downslide(void);
void app_key_gui_downslide_process(bool slave_key_flag);
#endif

#endif //#ifdef BESUI_KEY_EN
#endif //#ifdef BESUI_TWS_EN

#ifdef __cplusplus
}
#endif

#endif //__TWSUI_KEY_H__