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
#ifdef USB_HID_COMMAND_ENABLE

#include "string.h"
#include "usb_audio.h"
#include "usb_hid_app.h"


static USB_HID_DATA_RSP_T payload;
static uint16_t payload_len;
static USB_HID_CMD_RECV_CALLBACK usb_hid_cmd_callback[OP_USB_HID_CMD_HANDLER_NUM] = {NULL};

static bool usb_hid_app_cmd_get_report(struct EP0_TRANSFER *transfer)
{
    uint8_t __unused report_type;
    uint8_t __unused report_id;
    TRACE(1,"%s start", __func__);
    report_type = transfer->setup_pkt.wValue >> 8;
    report_id = transfer->setup_pkt.wValue & 0xFF;
    if (report_id == HID_REPORT_ID_RSP) {
        transfer->data = (uint8_t *)&payload;
        transfer->length = payload_len;
    }
    transfer->stage = DATA_IN_STAGE;
    return true;
}

static bool usb_hid_app_cmd_set_report(struct EP0_TRANSFER *transfer)
{
    uint8_t __unused report_type;
    uint8_t __unused report_id;
    TRACE(1,"%s start", __func__);
    report_type = transfer->setup_pkt.wValue >> 8;
    report_id = transfer->setup_pkt.wValue & 0xFF;
    if (report_id == HID_REPORT_ID_REQ) {
        transfer->length = HID_REPORT_RSP_PACKET_SIZE;
    }
    transfer->stage = DATA_OUT_STAGE;
    return true;
}

static bool usb_hid_app_cmd_received(struct EP0_TRANSFER *transfer)
{
    uint8_t __unused report_type;
    uint8_t __unused report_id;
    USB_HID_DATA_RSP_T* pPayload = (USB_HID_DATA_RSP_T *)transfer->data;
    USB_HID_CMD_PAYLOAD_T* pPayload_data = (USB_HID_CMD_PAYLOAD_T *)(pPayload->param);

    report_type = transfer->setup_pkt.wValue >> 8;
    report_id = transfer->setup_pkt.wValue & 0xFF;
    TRACE(0, "HID_REQ_SET_REPORT: report_id is %x",report_id);

    if (pPayload->report_id != HID_REPORT_ID_REQ) {
        TRACE(1,"%s: worry repord id!", __func__);
    } else {
        // execute the command handler
        if (usb_hid_cmd_callback[OP_USB_HID_CMD_HANDLER0_ID]) {
            if (pPayload_data->cmdCode < OP_USB_HID_COMMAND_COUNT ) {
                usb_hid_cmd_callback[OP_USB_HID_CMD_HANDLER0_ID]((USB_HID_APP_CMD_CODE_E)(pPayload_data->cmdCode), pPayload_data->param, pPayload_data->paramLen);
            } else {
                TRACE(0,"%s: worry command id!", __func__);
            }
        }
    }
    return true;
}

static void usb_hid_app_send_rsp(uint8_t * cmdParam, uint16_t cmdParam_len)
{
    return hid_epint_in_send_report(cmdParam, cmdParam_len);
}

//just for example
static void usb_hid_string_handler(uint8_t * ptrParam, uint16_t paramLen)
{
    TRACE(1,"%s start", __func__);
    uint8_t usb_hid_epint_in_test[3] = {0 ,1, 2};
    uint16_t dataLen = sizeof(usb_hid_epint_in_test);

    payload.report_id = HID_REPORT_ID_RSP;
    payload_len = dataLen + 7;

    USB_HID_CMD_RSP_T* pResponse_data = (USB_HID_CMD_RSP_T *)(payload.param);
    pResponse_data->cmdCodeToRsp = OP_USB_HID_STRING;
    pResponse_data->cmdRetStatus = STATUS_INVALID_PARAMETER;
    pResponse_data->rspDataLen   = dataLen;
    memcpy(pResponse_data->rspData, usb_hid_epint_in_test, dataLen);

    usb_hid_app_send_rsp((uint8_t *)&payload, payload_len);
}

//just for example
static void usb_hid_get_device_capability_handler(uint8_t * ptrParam, uint16_t paramLen)
{
    TRACE(1,"%s start", __func__);
    uint8_t usb_hid_epint_in_test[3] = {0 ,1, 2};
    uint16_t dataLen = sizeof(usb_hid_epint_in_test);

    payload.report_id = HID_REPORT_ID_RSP;
    payload_len = dataLen + 7;

    USB_HID_CMD_RSP_T* pResponse_data = (USB_HID_CMD_RSP_T *)(payload.param);
    pResponse_data->cmdCodeToRsp = OP_GetDeviceCapability;
    pResponse_data->cmdRetStatus = STATUS_OK;
    pResponse_data->rspDataLen   = dataLen;
    memcpy(pResponse_data->rspData, usb_hid_epint_in_test, dataLen);

    usb_hid_app_send_rsp((uint8_t *)&payload, payload_len);
}

static void usb_hid_cmd_handle(USB_HID_APP_CMD_CODE_E funcCode, uint8_t * ptrParam, uint16_t paramLen)
{
    TRACE(2,"%s enter: %x", __func__,funcCode);
    switch (funcCode)
    {
        case OP_USB_HID_STRING:
        //just for example. tigger command:ep0<- 21 09 24 03 03 00 40 00 data: 24 00 00
            usb_hid_string_handler(ptrParam, paramLen);
            break;
        case OP_GetDeviceCapability:
        //just for example
            usb_hid_get_device_capability_handler(ptrParam, paramLen);
            break;
        // TO ADD: new cmd
        default:
            break;
            ;
    }
}

void usb_hid_cmd_handler_callback_register(USB_HID_CMD_RECV_CALLBACK func)
{
    usb_hid_cmd_callback[OP_USB_HID_CMD_HANDLER0_ID] = func;
}

void usb_hid_app_init(void)
{
    usb_hid_enint_out_callback_register(usb_hid_app_cmd_received, usb_hid_app_cmd_get_report, usb_hid_app_cmd_set_report);
    usb_hid_cmd_handler_callback_register(usb_hid_cmd_handle);
}

#endif