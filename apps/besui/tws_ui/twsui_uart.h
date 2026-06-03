#ifndef __TWSUI_UART_H__
#define __TWSUI_UART_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

#ifdef BESUI_1WIRE_EN
#define UART_HEAD_BOX                   0x03
#define UART_HEAD_LEFT                  0x04
#define UART_HEAD_RIGHT                 0x05

#define BOX_CMD_ENTER_DUT               0x01
#define BOX_CMD_GET_SW_VERSION          0x04
#define BOX_CMD_GET_BT_NAME             0x05
#define BOX_CMD_GET_MAC_ADDR            0x08
#define BOX_CMD_WRITE_SN                0x0A
#define BOX_CMD_READ_SN                 0x10
#define BOX_CMD_ENTER_BTMODE            0x39
#define BOX_CMD_GET_MASTER_ADDR         0x53
#define BOX_CMD_OTA_UPGRADE             0x58
#define BOX_CMD_WRITE_COLOR             0x59
#define BOX_CMD_READ_COLOR              0x5A
#define BOX_CMD_GET_WEAR_STA            0x5B
#define BOX_CMD_PRE_CASE_CLOSE          0x5C
#define BOX_CMD_CAP_TEST                0x5D
#define BOX_CMD_CAP_TEST_RETURN         0x5E
#define BOX_CMD_CAP_ONOFF               0x5F

//new add cmd, 0x80~0xDF
#define BOX_CMD_GET_IMU_ID              0x80

#define BOX_CMD_HEARTBEAT               0xE0
#define BOX_CMD_CASE_OPEN               0xE1
#define BOX_CMD_CASE_CLOSE              0xE2
#define BOX_CMD_ENTER_SHIPMODE          0xE3
#define BOX_CMD_SW_RESET                0xE4
#define BOX_CMD_BT_PAIRMODE             0xE5
#define BOX_CMD_CLEAR_BTPAIRLIST        0xE6
#define BOX_CMD_CLEAR_BTTWSPAIRLIST     0xE7


#define BOX_CMD_GET_CURR_ADDR           0xF1
#define BOX_CMD_GET_PEER_ADDR           0xF2
#define BOX_CMD_ADDR_SWAP_OK            0xF5

#define BOX_CMD_1WIRE_UPGRADE           0xF3
#define BOX_CMD_GET_EAR_BATLEVEL        0xF7
#define BOX_CMD_POWEROFF                0xF8
#define BOX_CMD_TX_PEER_ADDDR           0xF9

#define UART_EVENT_OTA_UPGRADE          0x01
#define UART_EVENT_ENTER_DUT            0x02
#define UART_EVENT_1WIRE_UPGRADE        0x03
#define UART_EVENT_ENTER_SHIPMODE       0x04
#define UART_EVENT_ENTER_BT_MODE        0x05
#define UART_EVENT_CLEAR_BTPAIRLIST     0x06
#define UART_EVENT_CLEAR_BTTWSPAIRLIST  0x07
#define UART_EVENT_SW_RESET             0x08
#define UART_EVENT_BT_PAIRMODE          0x09

#if 0
#define OPEN_CLOSE_CHARGE_BOX           0x00
#define GET_RIGHT_ADDR                  0x01
#define SEND_ADDR_TO_LEFT               0x02
#define SEMD_LEFT_ADDR_TO_RIGHT         0x03
#define TWS_NEED_CONNECTED              0x04
#define ENTER_OTA_MODE                  0x05
#define ENTER_DUT_MODE                  0x06
#define CLEAR_PAIRLIST                  0x07
#define GET_SOFTWARE_VERSION            0x08
#define NEED_SHUTDOWN                   0x09
#endif

#define DATA_MAX_LEN                    40
typedef struct {
    bool twspair_flag;
    bool init_flag;
    bool rx_onoff;

    uint8_t event_type;
    uint8_t rx_dat[40];

} type_uiuart_t;
extern type_uiuart_t uiuart;

void app_uart_set_need_twspair_init(void);
void app_uart_communication_modual_init(void);
void app_uart_clear_need_twspair_process(void);
void app_uart_enter_twspair_config_set(bool isRightMasterFlag);
void app_uart_communication_post_msg(uint8_t *uart_data, uint8_t len);

void app_uart_event_timer_onoff(bool timer_en);

void app_uart_post_msg(uint32_t message_id, uint32_t param0);
void app_uart_threadhandle_init(void);

#endif //#ifdef BESUI_1WIRE_EN

#ifdef __cplusplus
}
#endif

#endif
