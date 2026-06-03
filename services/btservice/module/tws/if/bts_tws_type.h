/***************************************************************************
 *
 * Copyright 2015-2025 BES.
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
#ifndef __BTS_IBRT_TYPE_H__
#define __BTS_IBRT_TYPE_H__

#define TWS_SYNC_BUF_SIZE           APP_TWS_CTRL_BUFFER_MAX_LEN

/******************************macro defination*****************************/
#define  IBRT_UI_LONG_POLL_INTERVAL                         (0xD0)
#define  IBRT_UI_DEFAULT_POLL_INTERVAL                      (0x68)

#ifdef DUAL_MIC_RECORDING
#define  IBRT_UI_SHORT_POLL_INTERVAL                        (0x18)
#else
#define  IBRT_UI_SHORT_POLL_INTERVAL                        (0x34)
#endif
#define  IBRT_UI_EXTREMELY_SHORT_POLL_INTERVAL              (0x1A)

#define  IBRT_UI_DEFAULT_POLL_INTERVAL_IN_SCO               (0x9c)
#ifdef DUAL_MIC_RECORDING
#define  IBRT_UI_SHORT_POLL_INTERVAL_IN_SCO                 (0x27)
#else
#define  IBRT_UI_SHORT_POLL_INTERVAL_IN_SCO                 (0x4E)
#endif

#define  IBRT_TWS_BT_TPOLL_DEFAULT                          (80)

typedef enum
{
    TWS_SYNC_CONTINUE_FILLING   = 0,
    TWS_SYNC_BUF_FULL           = 1,
    TWS_SYNC_BUF_SEGMENTED      = 2,
    TWS_SYNC_NO_DATA_FILLED     = 3
} TWS_SYNC_FILL_RET_E;

typedef enum {
    TWS_SYNC_USER_BLE_INFO      = 0,
    TWS_SYNC_USER_OTA           = 1,
    TWS_SYNC_USER_AI_CONNECTION = 2,
    TWS_SYNC_USER_GFPS_INFO     = 3,
    TWS_SYNC_USER_AI_INFO       = 4,
    TWS_SYNC_USER_AI_MANAGER    = 5,
    TWS_SYNC_USER_DIP           = 6,
    TWS_SYNC_USER_LE_AUDIO      = 7,

    TWS_SYNC_USER_NUM,
} TWS_SYNC_USER_E;

typedef void (*TWS_SYNC_INFO_PREPARE_FUNC_T)(uint8_t *buf, uint16_t *totalLen, uint16_t *len, uint16_t expectLen);
typedef void (*TWS_INFO_SYNC_FUNC_T)(uint8_t *buf, uint16_t len, bool isContinueInfo);

typedef struct {
    TWS_SYNC_INFO_PREPARE_FUNC_T sync_info_prepare_handler;
    TWS_INFO_SYNC_FUNC_T sync_info_received_handler;
    TWS_SYNC_INFO_PREPARE_FUNC_T sync_info_prepare_rsp_handler;
    TWS_INFO_SYNC_FUNC_T sync_info_rsp_received_handler;
    TWS_INFO_SYNC_FUNC_T sync_info_rsp_timeout_handler;
} TWS_SYNC_USER_T;

typedef struct {
    TWS_SYNC_USER_E userId;
    bool continueInfo;
    uint16_t infoLen;     // length of the valid data in info[TWS_SYNC_BUF_SIZE-1]
    uint8_t info[TWS_SYNC_BUF_SIZE];
}TWS_SYNC_ENTRY_T;

#define TWS_SYNC_MAX_XFER_SIZE      sizeof(TWS_SYNC_ENTRY_T)

typedef struct {
    uint16_t totalLen;  // length of the valid data in content[TWS_SYNC_MAX_XFER_SIZE]
    uint8_t content[TWS_SYNC_MAX_XFER_SIZE];
}TWS_SYNC_DATA_T;

typedef struct {
    TWS_SYNC_USER_T syncUser[TWS_SYNC_USER_NUM];
    TWS_SYNC_DATA_T sync_data;
} TWS_ENV_T;

#endif /*__BTS_IBRT_TYPE_H__*/
