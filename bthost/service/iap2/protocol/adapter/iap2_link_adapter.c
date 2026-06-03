#include "iap2_link_adapter.h"
#include "iap2_link.h"
#include "bt_common_define.h"

//#if defined (IAP2_LINK_USE_BT_SPP_TRANS)
#include "app_iap2_spp_adapter.h"
//#endif // IAP2_LINK_USE_BT_SPP_TRANS

static iap2_conn_subscriber_t g_iap2_subscriber[IAP2_CONN_SUBSCRIBER_NUM];

void iap2_link_adapter_add_subscriber_callback(iap2_adapter_event_callback_t cb, uint32_t event_mask)
{
    iap2_conn_subscriber_t *p = NULL;
    uint8_t subscriber_num = sizeof(g_iap2_subscriber)/sizeof(iap2_conn_subscriber_t);

    for (int i = 0; i < subscriber_num; i += 1)
    {
        p = g_iap2_subscriber + i;
        if (p->iap2_event_cb && p->iap2_event_cb == cb)
        {
            return;
        }
        if (p->iap2_event_cb == NULL)
        {
            p->iap2_event_cb = cb;
            p->event_mask = event_mask;
            return;
        }
    }

    DEBUG_INFO(0,  "MFI_LINK_ADA line %d register fail ca=%p", __LINE__, __builtin_return_address(0));
}

void iap2_link_adapter_report_event(const iap2_link_info_t* link_info, IAP2_CONN_EVENT_T event, iap2_subscriber_callback_param_t* param)
{
    iap2_conn_subscriber_t *p = NULL;
    uint8_t subscriber_num = sizeof(g_iap2_subscriber)/sizeof(iap2_conn_subscriber_t);
    DEBUG_INFO(0,  "MFI_LINK_ADA line %d event %d num %d", __LINE__, event, subscriber_num);

    for (int i = 0; i < subscriber_num; i += 1)
    {
        p = g_iap2_subscriber + i;
        if (p->iap2_event_cb && (p->event_mask & event))
        {
            p->iap2_event_cb(link_info, event, param);
        }
    }
}

bool iap2_Link_adapter_send_control_session_data(iap2_link_info_t* link_info, uint16_t data_len, uint8_t* data)
{
    return iap2_link_send_session_data(link_info, IAP2_SESSION_TYPE_CONTROL, data_len, data);
}

bool iap2_Link_adapter_send_ea_session_data(iap2_link_info_t* link_info, uint16_t data_len, uint8_t* data)
{
    return iap2_link_send_session_data(link_info, IAP2_SESSION_TYPE_EA, data_len, data);
}

void iap2_link_adpter_handle_transport_event(iap2_transport_callback_param_t param)
{
    iap2_trans_event_t event = param.event;
    iap2_link_callback_param_t link_param = {0};
    uint16_t link_param_size = 0;

    link_param.link_info = param.link_info;
    link_param.event = param.event;

    switch (event)
    {
        case IAP2_TRANS_EVENT_OPEN:
            link_param_size = IAP2_LINK_HEADER_LEN;
        break;
        case IAP2_TRANS_EVENT_CLOSE:
            link_param_size = IAP2_LINK_HEADER_LEN;
        break;
        case IAP2_TRANS_EVENT_TXDONE:
            link_param_size = IAP2_LINK_HEADER_LEN;
        break;
        case IAP2_TRANS_EVENT_RECV_DATA:
            link_param.rx_data_len = param.rx_data_len;
            link_param_size = IAP2_LINK_HEADER_LEN + IAP2_LINK_RX_DATA_LEN_LEN + param.rx_data_len;
            memcpy(&link_param.rx_data_ptr, param.rx_data_ptr, param.rx_data_len);
        break;
        default:
            DEBUG_INFO(0,  "MFI_LINK_ADA line %d warning event %d", __LINE__, event);
            link_param.event = IAP2_TRANS_EVENT_NONE;
        break;
    }

    if (link_param.event != IAP2_TRANS_EVENT_NONE)
    {
        iap2_link_adapter_push_data_to_rec_pool((uint8_t*)&link_param, link_param_size);
    }

    return;
}

static void iap2_link_adapter_register_transport_event_callback(void)
{
    DEBUG_INFO(0,  "MFI_LINK_ADA line %d register_trans_cb", __LINE__);
    app_iap2_spp_adapter_register_transport_callback(iap2_link_adpter_handle_transport_event);
}

void iap2_link_adapter_init(void)
{
    DEBUG_INFO(0,  "MFI_LINK_ADA line %d ada_init", __LINE__);
    iap2_link_init();
    iap2_link_adapter_register_transport_event_callback();
}

int iap2_link_adapter_push_data_to_rec_pool(uint8_t *buf, uint16_t len)
{
    return iap2_push_data_to_rec_pool(buf, len);
}

bool iap2_link_adapter_send_data_to_trans(iap2_link_info_t* link_info, uint8_t* buf, uint16_t len)
{
    if (link_info->type == IAP2_LINK_TYPE_BT)
    {
//#ifdef (IAP2_LINK_USE_BT_SPP_TRANS)
        return iap2_spp_adapter_send_data((bt_bdaddr_t *)&(link_info->addr), buf, len);
//#endif // IAP2_LINK_USE_BT_SPP_TRANS
    }
    else
    {
        DEBUG_INFO(0,  "MFI_LINK_ADA line %d link type error %d", __LINE__, link_info->type);
        return false;
    }
}

uint32_t iap2_link_adapter_save_ctx(iap2_link_info_t* link_info, uint8_t *buf, uint32_t buf_len)
{
    return iap2_link_save_ctx(link_info, buf, buf_len);
}

uint32_t iap2_link_adapter_restore_ctx(iap2_link_info_t* link_info, uint8_t *buf, uint32_t buf_len)
{
    return iap2_link_restore_ctx(link_info, buf, buf_len);
}