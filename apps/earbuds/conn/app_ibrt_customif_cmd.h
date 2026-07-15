/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
#ifndef __APP_IBRT_IF_CUSTOM_CMD__
#define __APP_IBRT_IF_CUSTOM_CMD__

#include "app_ibrt_custom_cmd.h"

#define APP_TWS_CMD_PRIO_0       (0x0)
#define APP_TWS_CMD_PRIO_1       (0x1)
#define APP_TWS_CMD_PRIO_2       (0x2)
#define APP_TWS_CMD_PRIO_3       (0x3)
#define APP_TWS_CMD_PRIO_4       (0x4)
#define APP_TWS_CMD_PRIO_5       (0x5)
#define APP_TWS_CMD_PRIO_6       (0x6)
#define APP_TWS_CMD_PRIO_7       (0x7)

#define app_ibrt_custom_cmd_rsp_timeout_handler_null   (0)
#define app_ibrt_custom_cmd_rsp_handler_null           (0)
#define app_ibrt_custom_cmd_rx_handler_null            (0)
#define app_ibrt_custom_cmd_tx_done_handler_null       (0)

typedef enum
{
    APP_IBRT_CUSTOM_CMD_TEST1 = APP_IBRT_CMD_BASE|APP_IBRT_CUSTOM_CMD_PREFIX|0x01,
    APP_IBRT_CUSTOM_CMD_TEST2 = APP_IBRT_CMD_BASE|APP_IBRT_CUSTOM_CMD_PREFIX|0x02,

//-------------------------------------------------------------------------------------------------------
#ifdef BESUI_TWS_EN
    APP_TWS_CMD_CAPSENSOR_WEAR = APP_IBRT_CMD_BASE | APP_IBRT_CUSTOM_CMD_PREFIX | 0x03,
    APP_TWS_CMD_SEND_TOTA_BATTERY_LEVEL_CMD = APP_IBRT_CMD_BASE | APP_IBRT_CUSTOM_CMD_PREFIX | 0x04,
    APP_TWS_CMD_SYNC_TOTA_BUTTON_EVENT = APP_IBRT_CMD_BASE | APP_IBRT_CUSTOM_CMD_PREFIX | 0x05,
    APP_TWS_CMD_SEND_TOTA_BATTERY_LEVEL_RSP = APP_IBRT_CMD_BASE | APP_IBRT_CUSTOM_CMD_PREFIX | 0x06,
    APP_TWS_CMD_SYNC_EQ  = APP_IBRT_CMD_BASE | APP_IBRT_CUSTOM_CMD_PREFIX | 0x07,
    APP_TWS_CMD_SYNC_BESSPA  = APP_IBRT_CMD_BASE | APP_IBRT_CUSTOM_CMD_PREFIX | 0x08,
    APP_TWS_CMD_SYNC_DOLBY  = APP_IBRT_CMD_BASE | APP_IBRT_CUSTOM_CMD_PREFIX | 0x09,
    APP_TWS_CMD_PHONE_CONNECT_SYNC = APP_IBRT_CMD_BASE | APP_IBRT_CUSTOM_CMD_PREFIX | 0x0A,
    APP_TWS_CMD_POWEROFF_SHUTDOWN_SYNC = APP_IBRT_CMD_BASE | APP_IBRT_CUSTOM_CMD_PREFIX | 0x0B,
    APP_TWS_CMD_BATTERY_LEVEL_SYNC = APP_IBRT_CMD_BASE | APP_IBRT_CUSTOM_CMD_PREFIX | 0x0C,
#ifdef BESUI_GAME_EN
    APP_TWS_CMD_SWITCH_GAME_MODE_SYNC = APP_IBRT_CMD_BASE | APP_IBRT_CUSTOM_CMD_PREFIX | 0x0D,
#endif
    APP_TWS_CMD_SWITCH_LED_MODE_SYNC = APP_IBRT_CMD_BASE | APP_IBRT_CUSTOM_CMD_PREFIX | 0x0E,
    APP_TWS_CMD_BTADDR_SYNC = APP_IBRT_CMD_BASE | APP_IBRT_CUSTOM_CMD_PREFIX | 0x0F,
    APP_TWS_CMD_CLEAR_PIARLIST_SYNC = APP_IBRT_CMD_BASE | APP_IBRT_CUSTOM_CMD_PREFIX | 0x10,
    APP_TWS_CMD_SPACE_AUDIO_SYNC_STATUS = APP_IBRT_CMD_BASE | APP_IBRT_CUSTOM_CMD_PREFIX | 0x11,
    APP_TWS_CMD_SET_OPENRECONNET_STATUS = APP_IBRT_CMD_BASE | APP_IBRT_CUSTOM_CMD_PREFIX | 0x12,
#if defined(OTA_BOOT_SYNC_EN)
    APP_TWS_CMD_SYNC_OTABOOT  = APP_IBRT_CMD_BASE | APP_IBRT_CUSTOM_CMD_PREFIX | 0x13,
#endif
#ifdef BESUI_COMM_EN
    APP_TWS_CMD_SYNC_SOMETHING  = APP_IBRT_CMD_BASE | APP_IBRT_CUSTOM_CMD_PREFIX | 0x14,
#endif
#endif
//-------------------------------------------------------------------------------------------------------

    //new customer cmd add here
} app_ibrt_custom_cmd_code_e;

typedef struct
{
    uint8_t rsv;
    uint8_t buff[6];
} __attribute__((packed))ibrt_custom_cmd_test_t;

void app_ibrt_customif_cmd_test(ibrt_custom_cmd_test_t *cmd_test);

#if defined(BESUI_TWS_EN) || defined(BESUI_STEREO_EN)
enum{
    TWS_SYNC_WEAR_M2S       = 0x01,
    TWS_SYNC_WEAR_S2M       = 0x02,
    TWS_SYNC_WEAR_CTRL      = 0x03,
    TWS_SYNC_WEAR_PROMPT    = 0x04,

    TWS_SYNC_WEAR_MAX       = 0xFF,
};
#endif

#ifdef BESUI_TWS_EN
void app_ibrt_customif_cmd_sync_battery_level(uint8_t current_level);
void app_ibrt_customif_cmd_sync_poweroff_shutdown(bool poweroff_flag);
#ifdef BESUI_GAME_EN
void app_ibrt_customif_cmd_sync_game_mode(bool switch_game_flag, uint8_t need_set_ame_mode);
#endif
void app_ibrt_customif_cmd_sync_led_mode(uint8_t led_mode, uint8_t call_status);
void app_ibrt_customif_cmd_sync_btaddr(uint8_t *cur_btaddr);
void app_ibrt_customif_cmd_sync_clear_pairlist(bool clear_tws, bool clear_phone);
void app_ibrt_customif_cmd_sync_space_audio_status(bool switch_flag, uint8_t current_status);
void app_ibrt_customif_cmd_sync_set_reconnect_status(uint8_t device_id, uint8_t recon_status);
void twsui_wear_pp_tx(bool role, uint8_t sta);
#endif

#endif
