/***************************************************************************
 *
 * Copyright 2020-2025 BES.
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
#ifndef __BLE_AIWANG_SRV_H__
#define __BLE_AIWANG_SRV_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include "bes_dp_api.h"

#ifndef BLE_AIWANG_SRV_ENABLED
#define BLE_AIWANG_SRV_ENABLED
#endif


typedef enum cmds {
	A0_SETS     = 0xA0,
	B0_SETS     = 0xB0,
	C0_SETS     = 0xC0,
	OTA_SETS    = 0xF0
} AI_WANG_SETS_CMDS;


typedef enum sub_cmds {
	ENTER_FACTORY_MODE = 0x01,
	EXIT_AND_REBOOT    = 0x02,
	ENTER_SHIP_MODE    = 0x03,
	FACTORY_RESET      = 0x04,
	AUDIO_LOOPBACK     = 0x01,
	PLAY_TEST_TONE     = 0x02,
	LED_CONTROL        = 0x03,
	BUTTON_EVENT       = 0x04,
	GET_FW_VERSION     = 0x01,
	GET_BATTERY_INFO   = 0x02,
	READ_SN            = 0x03,
	WRTIE_SN           = 0x04,
	OTA_CMD            = 0x01
} AI_WANG_SETS_SUB_CMDS;

typedef enum {
	REQUEST_CMD   = 0,
	INDICATOR_CMD = 1,
	RESPONSE_WITHOUT_ERROR = 2,
	RESPONSE_WITH_ERROR    = 3,
}COMMAND_CODE_TYPE_T;

typedef struct {
	uint8_t cmd_type;
	uint8_t remain_count_packests;
	uint8_t params_len;
	uint8_t payload[0];
} REQUEST_DATA_STRUCT;

typedef struct {
	uint8_t rsp_cmd_type;
	uint8_t remain_count_packests;
	uint8_t params_len;
	uint8_t payload[0];
} RESPONSE_DATA_NO_ERROR_STRUCT;

typedef struct {
	uint8_t rsp_cmd_type;
	uint8_t remain_count_packests;
	uint8_t params_len;
	uint8_t error_code;
	uint8_t payload[0];
} RESPONSE_DATA_ERROR_STRUCT;



typedef struct {
	uint8_t indicator_cmd_type;
	uint8_t remain_count_packests;
	uint8_t params_len;
	uint8_t payload[0];
} INDICATE_DATA_STRUCT;



#ifdef BLE_AIWANG_SRV_ENABLED
#define BLE_INVALID_CONNECTION_INDEX    0xFF

struct ble_aiwang_server_env_tag
{
    uint8_t connectionIndex;
    uint16_t NtfIndCfg;
};

typedef struct {
    uint8_t  event;
    uint8_t  conidx;
    uint8_t* data;
    uint16_t len;
} __attribute__((packed)) ble_aiwang_param_u;

enum BLE_AIWANG_EVENT_TYPE_E
{
    BLE_AIWANG_SRV_CONN = 1,
    BLE_AIWANG_SRV_DISCONN,
    BLE_AIWANG_SRV_MTU,
    BLE_AIWANG_SRV_RX,
    BLE_AIWANG_SRV_SENDDONE,
    BLE_AIWANG_SRV_EVENT_NUM,
};

typedef void(*ble_aiwang_event_cb)(ble_aiwang_param_u *para_p);
typedef void(*ble_aiwang_server_mtuexchanged_done_t)(uint8_t conidx, uint16_t mtu);

void ble_aiwang_srv_init(void);
void ble_aiwang_srv_register_event_cb(ble_aiwang_event_cb callback);
uint8_t ble_aiwang_srv_send_data_via_notification(uint8_t* data, uint32_t len);
uint8_t ble_aiwang_srv_send_data_via_indication(uint8_t* data, uint32_t len);


#endif

#ifdef __cplusplus
}
#endif

#endif /* __BLE_WIFI_SRV_H__ */
