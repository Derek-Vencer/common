#ifndef __APP_IAP2_SPP_ADAPTER__H__
#define __APP_IAP2_SPP_ADAPTER__H__
#include "bt_common_define.h"
#include "adapter_common_type.h"
typedef enum
{
    IAP2_SPP_EVENT_NONE,
    IAP2_SPP_EVENT_OPEN,
    IAP2_SPP_EVENT_CLOSE,
    IAP2_SPP_EVENT_TXDONE,
    IAP2_SPP_EVENT_RECV_DATA,
    IAP2_SPP_EVENT_MAX,
}IAP2_SPP_EVENT_T;

typedef struct
{
    bt_bdaddr_t addr;
    IAP2_SPP_EVENT_T event;
    uint16_t rx_data_len;
    uint8_t *rx_data_ptr;
}iap2_spp_adapter_callback_param_t;

void app_iap2_spp_adapter_register_transport_callback(iap2_transport_event_callback_t cb);
void app_iap2_spp_adapter_handle_spp_event(iap2_spp_adapter_callback_param_t spp_param);
bool iap2_spp_adapter_send_data(bt_bdaddr_t *addr, uint8_t* buf, uint16_t len);

#endif // __APP_IAP2_SPP_ADAPTER__H__