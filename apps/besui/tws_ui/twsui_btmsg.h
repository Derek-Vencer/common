#ifndef __TWSUI_BTMSG_H__
#define __TWSUI_BTMSG_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef BESUI_TWS_EN
#ifdef BESUI_BTMSG_EN

#include "stdint.h"
#include "bt_common_define.h"

#define CONNECT_DEFAULT       0xff
#define CONNECT_OPEN_RECON    0x00
#define CONNECT_RECONNECT     0x01
#define CONNECT_CONNECTED     0x03


typedef struct
{
    bt_bdaddr_t current_id_addr;
    uint8_t open_reconnected_type;
    uint8_t open_reconnected_count;
}ibrt_mobile_open_reconnect_info;


typedef struct
{
    bt_bdaddr_t current_id_addr;
    uint8_t reconnected_type;
    uint8_t reconnected_count;
}ibrt_mobile_reconnect_info;


typedef struct
{
    bt_bdaddr_t connect_mobile_addr;
    bool mobile_connected;
}ibrt_mobile_connect_info;

void    besui_enter_pairmode(void);
void    besui_bt_msg_modual_init(void);
void    besui_bt_msg_put(uint8_t id, uint8_t type, uint8_t device_id);
void    app_bt_mobile_connect_info_init(void);
void    app_bt_mobile_reconnect_info_init(void);
void    app_bt_mobile_open_reconnect_info_init(void);
void    app_bt_clear_mobile_connect_info(uint8_t device_id);
uint8_t app_bt_compare_connect_addr(uint8_t *addr);
uint8_t app_bt_compare_reconnect_addr(uint8_t *addr);
uint8_t app_bt_compare_openreconnect_addr(uint8_t *addr);
void    app_bt_mobile_clear_reconnect_info(uint8_t device_id);
void    app_bt_mobile_set_reconnect_info(uint8_t device_id, uint8_t *mobile_addr);
void    app_bt_set_mobile_reconnect_addr(uint8_t device_id, uint8_t *mobile_addr);
bool    app_bt_get_mobile_connected_status(uint8_t device_id);
uint8_t app_bt_get_reconnected_type(void);
void    app_bt_mobile_set_openreconnect_info(uint8_t device_id, uint8_t recon_type, bool set_all_flag);
void    app_bt_mobile_clear_openreconnect_info(uint8_t device_id, bool clear_all_flag);
void    app_bt_set_mobile_connected_info(uint8_t device_id, uint8_t *bt_addr);
void    app_bt_set_mobile_connected_status(uint8_t device_id, bool connected_flag);
void    user_tws_tx_nbytes(uint8_t cmd, uint8_t byte1, uint8_t byte2);

//timer
#ifdef BESUI_PROMPT_ISSUE_EN
void prompt_set_flag(bool param);
bool prompt_get_flag(void);
void prompt_issue_timer_onoff(bool timer_en, uint16_t param);
#endif
void app_phone_pairmode_timer_onoff(bool timer_en);
void app_phone_discon_pairmode_timer_onoff(bool timer_en);
void app_tws_linkloss_pairmode_timer_onoff(bool timer_en);
#endif

#endif

#ifdef __cplusplus
}
#endif

#endif