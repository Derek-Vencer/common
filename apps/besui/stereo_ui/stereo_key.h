#ifndef __STEREO_KEY_H__
#define __STEREO_KEY_H__

#include "app_key.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CHECK_SW_POWER_KEY                                      HAL_KEY_CODE_NONE
#define POWER_KEY                                               HAL_KEY_CODE_PWR
#define VOLADD_KEY                                              HAL_KEY_CODE_FN1
#define VOLSUB_KEY                                              HAL_KEY_CODE_FN2
#define ANC_KEY                                                 HAL_KEY_CODE_FN3
#define PLAYPAUSE_KEY                                           HAL_KEY_CODE_FN4


#define AVRCP_KEY_PLAY                                          BT_AVRCP_KEY_PLAY
#define AVRCP_KEY_STOP                                          BT_AVRCP_KEY_STOP
#define AVRCP_KEY_PAUSE                                         BT_AVRCP_KEY_PAUSE
#define AVRCP_KEY_FORWARD                                       BT_AVRCP_KEY_FORWARD
#define AVRCP_KEY_BACKWARD                                      BT_AVRCP_KEY_BACKWARD
#define AVRCP_KEY_VOLUME_UP                                     BT_AVRCP_KEY_VOLUME_UP
#define AVRCP_KEY_VOLUME_DOWN                                   BT_AVRCP_KEY_VOLUME_DOWN

#define HFP_KEY_ANSWER_CALL                                     8
#define HFP_KEY_HANGUP_CALL                                     9
#define HFP_KEY_REDIAL_LAST_CALL                                10
#define HFP_KEY_CHANGE_TO_PHONE                                 11
#define HFP_KEY_ADD_TO_EARPHONE                                 12
#define HFP_KEY_MUTE                                            13
#define HFP_KEY_CLEAR_MUTE                                      14
//3way calls oper
#define HFP_KEY_THREEWAY_HOLD_AND_ANSWER                        15
#define HFP_KEY_THREEWAY_HANGUP_AND_ANSWER                      16
#define HFP_KEY_THREEWAY_HOLD_REL_INCOMING                      17
#define HFP_KEY_THREEWAY_HOLD_ADD_HELD_CALL                     18

#define HFP_KEY_DUAL_HF_HANGUP_ANOTHER                          19
#define HFP_KEY_DUAL_HF_HANGUP_CURR_ANSWER_ANOTHER              20
#define HFP_KEY_DUAL_HF_HOLD_CURR_ANSWER_ANOTHER                21
#define HFP_KEY_DUAL_HF_CHANGETOPHONE_ANSWER_ANOTHER            22
#define HFP_KEY_DUAL_HF_CHANGETOPHONE_ANOTHER_ADDTOEARPHONE     23
#define HFP_KEY_DUAL_HF_HANGUP_ANOTHER_ADDTOEARPHONE            24
#define HFP_KEY_DUAL_HF_CHANGETOPHONE_ANSWER_CURR               25

//hsp
#define HSP_KEY_CKPD_CONTROL     	                            21
#define HSP_KEY_ADD_TO_EARPHONE 	                            22
#define HSP_KEY_CHANGE_TO_PHONE	                                23


void app_ui_key_poweron_init(void);
void app_ui_bt_key_handle_down_key(enum APP_KEY_EVENT_T event);
void app_ui_bt_key_handle_up_key(enum APP_KEY_EVENT_T event);
void bt_key_handle_clear_device_key(enum APP_KEY_EVENT_T event);
void app_ui_key_handle(void);
void stereo_tota_button_event_handler(APP_KEY_STATUS *status, bool is_right);
void stereo_tota_button_init(void);
void stereo_tota_button_info_set(uint8_t* ptrParam, uint16_t paramLen);

void stereo_bes_spatial_onoff(void);
#if defined(OTA_BOOT_COPY_EN) || defined(BESUI_STEREO_EN)
void stereo_ota_boot_or_single_update(bool ota_flag, bool single_flag);
#endif
void stereo_game_mode_onoff(bool onoff, bool prompt);

#ifdef __cplusplus
}
#endif

#endif