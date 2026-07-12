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
//#ifdef BT_SPP_SUPPORT
#include <stdio.h>
#include "cmsis_os.h"
#include "hal_uart.h"
#include "hal_timer.h"
#include "audioflinger.h"
#include "lockcqueue.h"
#include "hal_trace.h"
#include "hal_cmu.h"
#include "hal_chipid.h"
#include "analog.h"
#include "app_audio.h"
#include "app_status_ind.h"
#include "bluetooth_bt_api.h"
#include "app_bt_stream.h"
#include "nvrecord_bt.h"
#include "nvrecord_env.h"
#include "cqueue.h"
#include "app_spp_tota.h"
#include "app_tota_cmd_code.h"
#include "app_tota.h"
#include "app_tota_cmd_handler.h"
#include "plat_types.h"
#include "spp_api.h"
#include "sdp_api.h"
#include "tota_stream_data_transfer.h"
#include "app_tota_common.h"
#include "heap_api.h"
#if defined(OTA_OVER_TOTA_ENABLED)
#include "ota_control.h"
#include "ota_basic.h"
#endif


typedef struct{
    bool isConnected;
    bt_spp_channel_t *pSppDevice;
    const tota_callback_func_t *callBack;
}tota_spp_ctl_t;

static tota_spp_ctl_t tota_spp_ctl = {
    .isConnected    = false,
    .pSppDevice     = NULL,
    .callBack       = NULL,
};

// static inline void _update_tx_buf(void);
// static inline uint8_t * _get_tx_buf_ptr(void);

#ifdef SPP_DEBUG_TOOL
extern bool app_spp_debug_cmd_check(uint8_t *cmd, uint16_t len);
extern uint8_t *app_spp_debug_cmd_process(uint8_t *cmd, uint16_t len, uint16_t *out_len);
#endif
extern "C" bool hal_cmd_debug_check(uint8_t *cmd, uint16_t len);
extern "C" uint8_t *hal_cmd_debug_process(uint8_t *cmd, uint16_t len, uint16_t *out_len);

extern "C" bool sparrow_spp_api_is_cmd(const uint8_t *data, uint16_t len);
extern "C" void sparrow_spp_api_rx_handler(const uint8_t *data, uint16_t len);
/* is tota busy, use to handle sniff */
bool spp_tota_in_progress(void)
{
    return is_stream_data_running();
}

/****************************************************************************
 * TOTA SPP SDP Entries
 ****************************************************************************/

/*---------------------------------------------------------------------------
 *
 * ServiceClassIDList
 */
static const U8 TotaSppClassId[] = {
#ifdef IS_TOTA_RFCOMM_UUID_CUSTOMIZED
    SDP_ATTRIB_HEADER_8BIT(17),
    SDP_UUID_128BIT(tota_rfcomm_custom_uuid),
#else
    SDP_ATTRIB_HEADER_8BIT(3),        /* Data Element Sequence, 6 bytes */
    SDP_UUID_16BIT(SC_SERIAL_PORT),     /* Hands-Free UUID in Big Endian */
#endif
};

static const U8 TotaSppProtoDescList[] = {
    SDP_ATTRIB_HEADER_8BIT(12),  /* Data element sequence, 12 bytes */

    /* Each element of the list is a Protocol descriptor which is a
     * data element sequence. The first element is L2CAP which only
     * has a UUID element.
     */
    SDP_ATTRIB_HEADER_8BIT(3),   /* Data element sequence for L2CAP, 3
                                  * bytes
                                  */

    SDP_UUID_16BIT(PROT_L2CAP),  /* Uuid16 L2CAP */

    /* Next protocol descriptor in the list is RFCOMM. It contains two
     * elements which are the UUID and the channel. Ultimately this
     * channel will need to filled in with value returned by RFCOMM.
     */

    /* Data element sequence for RFCOMM, 5 bytes */
    SDP_ATTRIB_HEADER_8BIT(5),

    SDP_UUID_16BIT(PROT_RFCOMM), /* Uuid16 RFCOMM */

    /* Uint8 RFCOMM channel number - value can vary */
    SDP_UINT_8BIT(RFCOMM_CHANNEL_TOTA)
};

/*
 * BluetoothProfileDescriptorList
 */
static const U8 TotaSppProfileDescList[] = {
#ifdef IS_TOTA_RFCOMM_UUID_CUSTOMIZED
    SDP_ATTRIB_HEADER_8BIT(22), /* Data element sequence, 22 bytes */

    /* Data element sequence for ProfileDescriptor, 20 bytes */
    SDP_ATTRIB_HEADER_8BIT(20),

    SDP_UUID_128BIT(tota_rfcomm_custom_uuid), /* Uuid16 SPP */
    SDP_UINT_16BIT(0x0102)          /* As per errata 2239 */
#else
    SDP_ATTRIB_HEADER_8BIT(8),        /* Data element sequence, 8 bytes */

    /* Data element sequence for ProfileDescriptor, 6 bytes */
    SDP_ATTRIB_HEADER_8BIT(6),

    SDP_UUID_16BIT(SC_SERIAL_PORT),   /* Uuid16 SPP */
    SDP_UINT_16BIT(0x0102)            /* As per errata 2239 */
#endif
};

/*
 * * OPTIONAL *  ServiceName
 */
static const U8 TotaSppServiceName1[] = {
    SDP_TEXT_8BIT(5),          /* Null terminated text string */
    'T', 'O', 'T', 'A', '\0'
};

// static const U8 TotaSppServiceName2[] = {
//     SDP_TEXT_8BIT(5),          /* Null terminated text string */
//     'S', 'p', 'p', '2', '\0'
// };

/* SPP attributes.
 *
 * This is a ROM template for the RAM structure used to register the
 * SPP SDP record.
 */
//static const SdpAttribute TotaSppSdpAttributes1[] = {
static bt_sdp_record_attr_t TotaSppSdpAttributes1[] = { // list attr id in ascending order

    SDP_ATTRIBUTE(AID_SERVICE_CLASS_ID_LIST, TotaSppClassId),

    SDP_ATTRIBUTE(AID_PROTOCOL_DESC_LIST, TotaSppProtoDescList),

    SDP_ATTRIBUTE(AID_BT_PROFILE_DESC_LIST, TotaSppProfileDescList),

    /* SPP service name*/
    SDP_ATTRIBUTE((AID_SERVICE_NAME + 0x0100), TotaSppServiceName1),
};

/*
static bt_sdp_record_attr_t TotaSppSdpAttributes2[] = { // list attr id in ascending order

    SDP_ATTRIBUTE(AID_SERVICE_CLASS_ID_LIST, TotaSppClassId),

    SDP_ATTRIBUTE(AID_PROTOCOL_DESC_LIST, TotaSppProtoDescList),

    SDP_ATTRIBUTE(AID_BT_PROFILE_DESC_LIST, TotaSppProfileDescList),


    SDP_ATTRIBUTE((AID_SERVICE_NAME + 0x0100), TotaSppServiceName2),
};
*/

/* extern declaration */
extern bool sparrow_spp_api_is_cmd(const uint8_t *data, uint16_t len);
extern void sparrow_spp_api_rx_handler(const uint8_t *data, uint16_t len);

static int tota_spp_handle_data_event_func(const bt_bdaddr_t *remote,bt_spp_event_t event,bt_spp_callback_param_t *param)
{
    uint8_t *pData;
    uint16_t dataLen;

    (void)remote;
    (void)event;

    /*
     * RX_DATA event 必須具有有效的：
     * 1. callback param
     * 2. SPP channel
     * 3. RX data pointer
     * 4. RX data length
     */
    if ((param == NULL) || (param->spp_chan == NULL) || (param->rx_data_ptr == NULL) || (param->rx_data_len == 0))
    {
        TOTA_V2_TRACE(
            0,
            "[SPP_RX] invalid param=%p chan=%p data=%p len=%u",
            (void *)param,
            (param != NULL) ?
                (void *)param->spp_chan : NULL,
            (param != NULL) ?
                (void *)param->rx_data_ptr : NULL,
            (unsigned int)((param != NULL) ?
                param->rx_data_len : 0));

        return -1;
    }

    pData = (uint8_t *)param->rx_data_ptr;
    dataLen = param->rx_data_len;
    TOTA_V2_TRACE(1,"spp tota v2 rx:%u",(unsigned int)dataLen);
    DUMP8("%02X ",pData,dataLen);

#ifdef SPP_DEBUG_TOOL
    /*
     * HAL CMD / EQ Tuning
     *
     * Header:
     * 7B 00 00 00 7B 00 00 00 ...
     */
    if (app_spp_debug_cmd_check(pData,dataLen))
    {
        uint8_t *ret_buf;

        ret_buf = app_spp_debug_cmd_process(pData,dataLen,&dataLen);

        if ((ret_buf != NULL) && (dataLen > 0))
        {
            bt_status_t ret;
            ret = bta_spp_write(param->spp_chan->rfcomm_handle,ret_buf,dataLen);
            TOTA_V2_TRACE(1,"[SPP_DEBUG] write ret=%d len=%u",(int)ret,(unsigned int)dataLen);
        }
        else
        {
            TOTA_V2_TRACE(0,"[SPP_DEBUG] invalid response buf=%p len=%u",(void *)ret_buf,(unsigned int)dataLen);
        }

        TOTA_V2_TRACE(0,"[%s] HAL CMD handled.",__func__);
        return 0;
    }
#endif

    /*
     * Sparrow API
     *
     * 0x30 Get Battery
     * 0x34 Get Device Name
     * 0x3C Get Key Mapping
     * 0x40 Set Key Mapping
     * 0x44 Get EQ
     * 0x46 Set EQ
     * 0x4C Get FW Version
     * 0x50 Color Code
     * ...
     */
    if (sparrow_spp_api_is_cmd(pData,dataLen))
    {
        TOTA_V2_TRACE(1,"[SPARROW_API] cmd=0x%02X len=%u",(unsigned int)pData[0],(unsigned int)dataLen);
        sparrow_spp_api_rx_handler(pData,dataLen);
        return 0;
    }

    /*
     * Original TOTA v2 data path.
     */
    if ((tota_spp_ctl.callBack != NULL) && (tota_spp_ctl.callBack->rx_cb != NULL))
    {
        tota_spp_ctl.callBack->rx_cb(pData,dataLen);
    }
    else
    {
        TOTA_V2_TRACE(0,"[SPP_RX] original TOTA rx callback is NULL");
    }
    return 0;
}


static int spp_tota_callback(const bt_bdaddr_t *remote,bt_spp_event_t event,bt_spp_callback_param_t *param)
{
    switch (event)
    {
        case BT_SPP_EVENT_OPENED:
        {
            TOTA_V2_TRACE(0,"spp_tota_callback v2 :: BTIF_SPP_EVENT_REMDEV_CONNECTED");
            if ((param == NULL) || (param->spp_chan == NULL))
            {
                /*
                 * OPENED event 資料異常。
                 * 不可對外公布 connected 狀態。
                 */
                tota_spp_ctl.isConnected = false;

                tota_spp_ctl.pSppDevice = NULL;

                TOTA_V2_TRACE(0,"[SPP_OPEN] invalid param=%p chan=%p",(void *)param,(param != NULL) ?(void *)param->spp_chan : NULL);
                break;
            }

            /*
             * 必須先設定實際 channel，
             * 最後才設定 isConnected。
             *
             * 避免其他 thread 看到 isConnected=true 時，
             * pSppDevice 還是舊值或 NULL。
             */
            tota_spp_ctl.pSppDevice = param->spp_chan;

            tota_spp_ctl.isConnected = true;

            TOTA_V2_TRACE(0,"[SPP_OPEN] connected chan=%p handle=0x%08X",(void *)tota_spp_ctl.pSppDevice,(unsigned int)tota_spp_ctl.pSppDevice->rfcomm_handle);
#if defined(OTA_OVER_TOTA_ENABLED)
            {
                bes_ota_event_param_t otaParam;
                memset(&otaParam,0,sizeof(otaParam));
                otaParam.pathType = DATA_PATH_SPP;
                memcpy(otaParam.param.address,(uint8_t *)&param->spp_chan->remote,sizeof(otaParam.param.address));
                otaParam.event = BES_OTA_CONN;
                app_ota_push_rx_data(SPP_RX_DATA_SELF_OTA_OVER_TOTA,&otaParam);
            }
#endif

            if ((tota_spp_ctl.callBack != NULL) && (tota_spp_ctl.callBack->connected_cb != NULL))
            {
                tota_spp_ctl.callBack->connected_cb();
            }
            else
            {
                TOTA_V2_TRACE(0,"[SPP_OPEN] connected callback is NULL");
            }
            break;
        }

        case BT_SPP_EVENT_CLOSED:
        {
            TOTA_V2_TRACE(0,"spp_tota_callback v2 :: BTIF_SPP_EVENT_REMDEV_DISCONNECTED");
            /*
             * 先禁止 TX，再清除 channel。
             */
            tota_spp_ctl.isConnected = false;
            tota_spp_ctl.pSppDevice = NULL;
#if defined(OTA_OVER_TOTA_ENABLED)
            {
                bes_ota_event_param_t otaParam;
                memset(&otaParam,0,sizeof(otaParam));
                otaParam.pathType = DATA_PATH_SPP;
                otaParam.event = BES_OTA_DISCONN;
                app_ota_push_rx_data(SPP_RX_DATA_SELF_OTA_OVER_TOTA,&otaParam);
            }
#endif

            if ((tota_spp_ctl.callBack != NULL) && (tota_spp_ctl.callBack->disconnected_cb != NULL))
            {
                tota_spp_ctl.callBack->disconnected_cb();
            }
            break;
        }

        case BT_SPP_EVENT_TX_DONE:
        {
            TOTA_V2_TRACE(0,"spp_tota_callback v2 :: BTIF_SPP_EVENT_DATA_SENT");
            if ((tota_spp_ctl.callBack != NULL) && (tota_spp_ctl.callBack->tx_done_cb != NULL))
            {
                tota_spp_ctl.callBack->tx_done_cb();
            }

            break;
        }

        case BT_SPP_EVENT_RX_DATA:
        {
            if (param == NULL)
            {
                TOTA_V2_TRACE(0,"[SPP_RX] callback param is NULL");
                break;
            }

            (void)tota_spp_handle_data_event_func(remote,event,param);
            break;
        }

        default:
        {
            TOTA_V2_TRACE(1,"[SPP] unknown event=%d",(int)event);
            break;
        }
    }

    return 0;
}

void app_spp_tota_init(const tota_callback_func_t *tota_callback_func)
{
    //tota_spp_ctl.txSem = osSemaphoreCreate(osSemaphore(tota_spp_tx_sem), 1);
    //tota_spp_ctl.txMutex = osMutexCreate(osMutex(tota_spp_tx_mutex));
    tota_spp_ctl.callBack = tota_callback_func;

    bta_spp_create_port(RFCOMM_CHANNEL_TOTA, TotaSppSdpAttributes1, ARRAY_SIZE(TotaSppSdpAttributes1));

    bta_spp_set_callback(RFCOMM_CHANNEL_TOTA, TOTA_RX_BUF_SIZE, spp_tota_callback, NULL);

    bta_spp_listen(RFCOMM_CHANNEL_TOTA, false, NULL);

    tota_spp_ctl.pSppDevice = bta_spp_create_channel(BT_DEVICE_ID_1, RFCOMM_CHANNEL_TOTA);
}

/* this func is safe in thread */
/* this func is safe in thread */
bool app_spp_tota_send_data(uint8_t *ptrData,uint16_t length)
{
    bt_status_t ret;
    bt_spp_channel_t *sppDevice;
    uint32_t rfcommHandle;

    if ((ptrData == NULL) || (length == 0))
    {
        TOTA_V2_TRACE(0,"[SPP_TX] invalid data=%p len=%u",(void *)ptrData,(unsigned int)length);
        return false;
    }

    if (!tota_spp_ctl.isConnected)
    {
        TOTA_V2_TRACE(0,"[SPP_TX] not connected len=%u",(unsigned int)length);
        return false;
    }

    /*
     * 先將 global channel 指標保存為區域變數。
     */
    sppDevice = tota_spp_ctl.pSppDevice;

    if (sppDevice == NULL)
    {
        TOTA_V2_TRACE( 0,"[SPP_TX] device is NULL len=%u",(unsigned int)length);
        return false;
    }

    /*
     * rfcomm_handle 是 uint32_t，不是 pointer。
     * 因此必須使用 %X，不能使用 %p。
     */
    rfcommHandle = sppDevice->rfcomm_handle;

    if (rfcommHandle == 0)
    {
        TOTA_V2_TRACE(0,"[SPP_TX] invalid rfcomm handle len=%u",(unsigned int)length);
        return false;
    }

    TOTA_V2_TRACE(1,"[SPP_TX] len=%u handle=0x%08X data=%p",(unsigned int)length,(unsigned int)rfcommHandle,(void *)ptrData);
    ret = bta_spp_write(rfcommHandle,ptrData,length);

    if (ret != BT_STS_SUCCESS)
    {
        TOTA_V2_TRACE( 1,"[SPP_TX] write failed ret=%d len=%u handle=0x%08X",(int)ret,(unsigned int)length,(unsigned int)rfcommHandle);
        return false;
    }

    TOTA_V2_TRACE(1,"[SPP_TX] write accepted len=%u handle=0x%08X",(unsigned int)length,(unsigned int)rfcommHandle);
    return true;
}

// static inline void _update_tx_buf(void)
// {
//     tota_spp_ctl.txIndex = (tota_spp_ctl.txIndex + 1) % MAX_SPP_PACKET_NUM;
// }

// static inline uint8_t * _get_tx_buf_ptr(void)
// {
//     return (tota_spp_ctl.txBuff + tota_spp_ctl.txIndex*MAX_SPP_PACKET_SIZE);
// }

//#endif /* BT_SPP_SUPPORT */
