#ifndef __BT_PAN_SERVICE_H__
#define __BT_PAN_SERVICE_H__
#include "bt_common_define.h"
#ifdef BT_PAN_SUPPORT
#ifdef __cplusplus
extern "C" {
#endif

#define PAN_HOST16_TO_BE(value,ptr) \
    *(((uint8_t *)(ptr))+0) = ((value)>>8)&0xFF; \
    *(((uint8_t *)(ptr))+1) = ((value))&0xFF

#define PAN_BE_TO_HOST16(ptr) \
    ((((uint16_t)*((uint8_t *)(ptr))) << 8) | *(((uint8_t *)(ptr))+1))

typedef struct {
    uint16_t network_protocol_type;
    uint16_t payload_length;
    uint8_t *payload_data;
    uint8_t *dest_address;
    uint8_t *source_address;
}bt_pan_ethernet_data_info;

typedef struct {
    void (*pan_opened)(uint8_t device_id, bool success);
    void (*pan_closed)(uint8_t device_id, uint8_t error_code);
    void (*pan_receive_ARP_protocol_data)(uint8_t device_id, const uint8_t *payload_data, uint16_t payload_length);
    void (*pan_receive_IPv4_protocol_data)(uint8_t device_id, const uint8_t *payload_data, uint16_t payload_length);
    void (*pan_receive_IPv6_protocol_data)(uint8_t device_id, const uint8_t *payload_data, uint16_t payload_length);
}bt_pan_register_t;

void bta_pan_register_callbacks(bt_pan_register_t *pan_register_callbacks);

int bta_pan_send_ARP_protocol_data(uint8_t device_id, const uint8_t *payload_data, uint16_t payload_length);

int bta_pan_send_IPv4_protocol_data(uint8_t device_id, const uint8_t *payload_data, uint16_t payload_length);

int bta_pan_send_IPv6_protocol_data(uint8_t device_id, const uint8_t *payload_data, uint16_t payload_length);

int bta_pan_send_address_specified_protocol_data(uint8_t device_id, const bt_pan_ethernet_data_info *data);

void bta_pan_init(void);

bool bta_pan_is_connected(uint8_t device_id);

int  bta_connect_pan_profile(bt_bdaddr_t *remote);

void bta_disconnect_pan_profile(uint8_t device_id);


#ifdef __cplusplus
}
#endif
#endif /* BT_PAN_SUPPORT */
#endif /* __BT_PAN_SERVICE_H__ */