/***************************************************************************
 *
 * Copyright 2015-2023 BES.
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
#ifndef __USB_HID_APP_H__
#define __USB_HID_APP_H__

#include "hal_trace.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HID_REPORT_ID_REQ                   0x24
#define HID_REPORT_ID_RSP                   0x27
#define HID_REPORT_RSP_PACKET_SIZE          0x3D

typedef enum
{
    //just for example
    OP_USB_HID_STRING,
    // TO ADD: new cmd
    OP_GetDeviceCapability = 2,
    /* commond count */
    OP_USB_HID_COMMAND_COUNT,
} USB_HID_APP_CMD_CODE_E;

typedef enum
{
    STATUS_OK = 0,
    STATUS_INVALID_PARAMETER = 2,
    STATUS_FAIL = 3,
} USB_HID_CMD_RET_STATUS_E;

enum USB_HID_CMD_HANDLER_ID_T {
    OP_USB_HID_CMD_HANDLER0_ID,
    // TO ADD: new handler id
    OP_USB_HID_CMD_HANDLER_NUM,
};

typedef struct
{
    uint16_t    cmdCode;        /**< command code, from USB_HID_APP_CMD_CODE_E */
    uint16_t    paramLen;       /**< length of the following parameter */
    uint8_t     param[HID_REPORT_RSP_PACKET_SIZE-5];
} USB_HID_CMD_PAYLOAD_T;

typedef struct
{
    uint16_t    cmdCodeToRsp;    /**< tell which command code to response */
    uint16_t    cmdRetStatus;    /**< handling result of the command, from USB_HID_CMD_RET_STATUS_E */
    uint16_t    rspDataLen;      /**< length of the response data */
    uint8_t     rspData[HID_REPORT_RSP_PACKET_SIZE-7];
} USB_HID_CMD_RSP_T;

typedef struct
{
    uint8_t     report_id;
    uint8_t     param[HID_REPORT_RSP_PACKET_SIZE-1];
} USB_HID_DATA_RSP_T;

typedef void (*USB_HID_CMD_RECV_CALLBACK)(USB_HID_APP_CMD_CODE_E funcCode, uint8_t * ptrParam, uint16_t paramLen);

void usb_hid_app_init(void);

#ifdef __cplusplus
}
#endif

#endif

