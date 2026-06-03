#ifndef __IAP2_LINK_ADAPTER_COMMON_TYPE_H__
#define __IAP2_LINK_ADAPTER_COMMON_TYPE_H__

#include "bt_common_define.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IAP2_TRANS_BUFFER_LEN  (512) // magic number
typedef enum
{
    IAP2_SESSION_TYPE_CONTROL,
    IAP2_SESSION_TYPE_FILE_TRANS,
    IAP2_SESSION_TYPE_EA,
}__attribute__ ((packed)) iap2_session_type_t;


typedef enum
{
    IAP2_LINK_TYPE_NONE,
    IAP2_LINK_TYPE_BT,
    IAP2_LINK_TYPE_BLE,
}__attribute__ ((packed)) iap2_link_type_t;

typedef union
{
    bt_bdaddr_t addr;
}__attribute__ ((packed)) iap2_link_addr_t;

typedef struct
{
    iap2_link_type_t type;
    iap2_link_addr_t addr;
}__attribute__ ((packed)) iap2_link_info_t;


typedef enum
{
    IAP2_TRANS_EVENT_NONE,
    IAP2_TRANS_EVENT_OPEN,
    IAP2_TRANS_EVENT_CLOSE,
    IAP2_TRANS_EVENT_TXDONE,
    IAP2_TRANS_EVENT_RECV_DATA,
}iap2_trans_event_t;

typedef struct
{
    iap2_link_info_t link_info;
    iap2_trans_event_t event;
    uint16_t rx_data_len;
    uint8_t rx_data_ptr[IAP2_TRANS_BUFFER_LEN];
}__attribute__ ((packed)) iap2_transport_callback_param_t;

typedef void (*iap2_transport_event_callback_t)(iap2_transport_callback_param_t param);

#ifdef __cplusplus
}
#endif
#endif // __IAP2_LINK_ADAPTER_COMMON_TYPE_H__