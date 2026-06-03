#include "app_iap2_spp_adapter.h"
#include "app_iap2_spp.h"
#include "bt_common_define.h"

iap2_transport_event_callback_t iap2_trans_cb = NULL;

void app_iap2_spp_adapter_register_transport_callback(iap2_transport_event_callback_t cb)
{
    DEBUG_INFO(0, " MFI_IAP2_SPP_ADA line %d iap2_trans_cb %p", __LINE__, cb);
    iap2_trans_cb = cb;
}

void app_iap2_spp_adapter_handle_spp_event(iap2_spp_adapter_callback_param_t spp_param)
{
    POSSIBLY_UNUSED bt_bdaddr_t addr = {0};
    iap2_transport_callback_param_t param = {0};
    IAP2_SPP_EVENT_T event = spp_param.event;

    memcpy(&param.link_info.addr.addr, &spp_param.addr, sizeof(bt_bdaddr_t));
    param.link_info.type = IAP2_LINK_TYPE_BT;

    //DEBUG_INFO(0, "MFI_IAP2_SPP_ADA line %d event 0x%x", __LINE__, event);

    switch (event)
    {
    case IAP2_SPP_EVENT_OPEN:
        param.event = IAP2_TRANS_EVENT_OPEN;
        break;
    case IAP2_SPP_EVENT_CLOSE:
        param.event = IAP2_TRANS_EVENT_CLOSE;
        break;
    case IAP2_SPP_EVENT_TXDONE:
        param.event = IAP2_TRANS_EVENT_TXDONE;
        break;
    case IAP2_SPP_EVENT_RECV_DATA:
        param.event = IAP2_TRANS_EVENT_RECV_DATA;
        param.rx_data_len = spp_param.rx_data_len;
        if (spp_param.rx_data_len > IAP2_TRANS_BUFFER_LEN)
        {
            param.rx_data_len = IAP2_TRANS_BUFFER_LEN;
            DEBUG_INFO(0, "MFI_IAP2_SPP_ADA line %d data_len too large %d\n", __LINE__, spp_param.rx_data_len);
        }
        memcpy(param.rx_data_ptr, spp_param.rx_data_ptr, spp_param.rx_data_len < IAP2_TRANS_BUFFER_LEN ? spp_param.rx_data_len : IAP2_TRANS_BUFFER_LEN);
    break;
    default:
        param.event = IAP2_TRANS_EVENT_NONE;
        DEBUG_INFO(0, "MFI_IAP2_SPP_ADA line %d unknown event %d\n", __LINE__, event);
    break;
    }

    if (iap2_trans_cb && param.event != IAP2_TRANS_EVENT_NONE)
    {
        iap2_trans_cb(param);
    }
    else
    {
        DEBUG_INFO(0, "MFI_IAP2_SPP_ADA line %d cb %p p_event %d\n", __LINE__, iap2_trans_cb, event);
    }

    return;
}

bool iap2_spp_adapter_send_data(bt_bdaddr_t *addr, uint8_t* buf, uint16_t len)
{
    uint16_t rfcomm_handle = 0;
    bt_status_t status;

    bool ret = false;

    ret = app_iap2_spp_get_rfcomm_handle(addr, &rfcomm_handle);
    if (ret == false)
    {
        return false;
    }

    status = bta_spp_write(rfcomm_handle, buf, len);
    if (status == BT_STS_FAILED)
    {
        DEBUG_INFO(0, "MFI_IAP2_SPP_ADA line %d send fail", __LINE__);
        return false;
    }

    return true;
}