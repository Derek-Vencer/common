#ifndef __IAP2_LINK_ADAPTER_H__
#define __IAP2_LINK_ADAPTER_H__

#include "bt_common_define.h"
#include "adapter_common_type.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IAP2_IAP2_CONN_NUM      (2)

#define ACCESS_AUTHEN_SUBSCRIBER_NUM     (1)
#define ACCESS_IDENTIFI_SUBSCRIBER_NUM   (1)
#define APP_LAUNCH_SUBSCRIBER_NUM        (1)
#define EA_PROTOCOL_SUBSCRIBER_NUM       (1)

#define IAP2_CONN_SUBSCRIBER_NUM (ACCESS_AUTHEN_SUBSCRIBER_NUM + ACCESS_IDENTIFI_SUBSCRIBER_NUM + APP_LAUNCH_SUBSCRIBER_NUM + EA_PROTOCOL_SUBSCRIBER_NUM)

// start of used for convey from ipa2_link to subscriber(Accessory Authentication、Accessory Identification and App Launch)
typedef enum
{
    IAP2_CONN_EVENT_NONE    = 0x00000000,
    IAP2_CONN_EVENT_OPEN    = 0x00000001,// connection open
    IAP2_CONN_EVENT_CLOSE   = 0x00000002,// connection close
    IAP2_CONN_EVENT_TXDONE  = 0x00000004, // data tx done
    IAP2_CONN_EVENT_RECV_CTRL_DATA  = 0x00000008, //  recv controll session data
    IAP2_CONN_EVENT_RECV_FILE_TRANS_DATA = 0x00000100,   // recv File Transfer Session
    IAP2_CONN_EVENT_RECV_EA_DATA    = 0x00000200,   // recv External Accessory Session data
}IAP2_CONN_EVENT_T;

typedef struct
{
    uint16_t data_len;
    uint8_t* data;
}iap2_conn_recv_ctrl_data_callback_t;

typedef struct
{
    uint16_t data_len;
    uint8_t* data;
}iap2_conn_recv_ea_data_callback_t;

typedef union {
    iap2_conn_recv_ctrl_data_callback_t recv_ctrl_data;
    iap2_conn_recv_ea_data_callback_t recv_ea_data;
} iap2_subscriber_callback_param_t;

typedef void (*iap2_adapter_event_callback_t)(const iap2_link_info_t* link_info, IAP2_CONN_EVENT_T event, iap2_subscriber_callback_param_t* param);

typedef struct {
    iap2_adapter_event_callback_t iap2_event_cb;
    uint32_t event_mask;
} iap2_conn_subscriber_t;

// event_mask: The events that subscribers are interested in
void iap2_link_adapter_add_subscriber_callback(iap2_adapter_event_callback_t cb, uint32_t event_mask);
void iap2_link_adapter_report_event(const iap2_link_info_t* link_info, IAP2_CONN_EVENT_T event, iap2_subscriber_callback_param_t* param);
bool iap2_Link_adapter_send_control_session_data(iap2_link_info_t* link_info, uint16_t data_len, uint8_t* data);
bool iap2_Link_adapter_send_ea_session_data(iap2_link_info_t* link_info, uint16_t data_len, uint8_t* data);
// end of "used for convey from ipa2_link..."


#define IAP2_LINK_TYPE_LEN                  (1) // iap2_link_type_t
#define IAP2_LINK_ADDR_LEN                  (6) // iap2_link_addr_t
#define IAP2_LINK_EVENT_LEN                 (1) // iap2_trans_event_t
#define IAP2_LINK_RX_DATA_LEN_LEN           (2) // rx_data_len
#define IAP2_LINK_BUFFER_LEN                (IAP2_REC_MAX_DATA) // rx_data_ptr
#define IAP2_LINK_HEADER_LEN                (IAP2_LINK_TYPE_LEN + IAP2_LINK_ADDR_LEN + IAP2_LINK_EVENT_LEN)
#define IAP2_TRANS_TOTAL_LEN                (IAP2_LINK_HEADER_LEN + IAP2_LINK_RX_DATA_LEN_LEN + IAP2_LINK_BUFFER_LEN)

int iap2_link_adapter_push_data_to_rec_pool(uint8_t *buf, uint16_t len);
bool iap2_link_adapter_send_data_to_trans(iap2_link_info_t* trans_info, uint8_t* buf, uint16_t len);
uint32_t iap2_link_adapter_save_ctx(iap2_link_info_t* link_info, uint8_t *buf, uint32_t buf_len);
uint32_t iap2_link_adapter_restore_ctx(iap2_link_info_t* link_info, uint8_t *buf, uint32_t buf_len);

void iap2_link_adapter_init(void);

#ifdef __cplusplus
}
#endif
#endif // __IAP2_LINK_ADAPTER_H__