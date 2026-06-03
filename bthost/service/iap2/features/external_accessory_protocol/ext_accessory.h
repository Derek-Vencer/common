#ifndef __EXT_ACCESSORY_H__
#define __EXT_ACCESSORY_H__
#include <stdint.h>
#include <stdbool.h>
#include "adapter_common_type.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IAP2_MATCH_ACTION_NO_PROMPT                                 0
#define IAP2_MATCH_ACTION_DEVICE_MAY_PROMPT_AND_FIND_APP_BUTTON     1
#define IAP2_MATCH_ACTION_DEVICE_NO_PROMPT_AND_FIND_APP_BUTTON      2

//
// Control session messages (EA protocol)
//
#define StartExternalAccessoryProtocolSession                               0xEA00 /* from device */
#define StartExternalAccessoryProtocolSession_id_ExternalAccessoryProtocolIdentifier        0
#define StartExternalAccessoryProtocolSession_id_ExternalAccessoryProtocolSessionIdentifier 1

#define StopExternalAccessoryProtocolSession                                0xEA01 /* from device */
#define StopExternalAccessoryProtocolSession_id_ExternalAccessoryProtocolSessionIdentifier  0

#define StatusExternalAccessoryProtocolSession                                0xEA03
#define StatusExternalAccessoryProtocolSession_id_ExternalAccessoryProtocolSessionIdentifier  0
#define StatusExternalAccessoryProtocolSession_id_ExternalAccessoryProtocolSessionStatus  1
#define SessionStatusOK 0
#define SessionClose    1

typedef enum
{
    EA_SESSION_STATE_OPEN,
    EA_SESSION_STATE_CLOSE,
    EA_SESSION_STATE_MAX,
}ea_session_state_t;

typedef enum
{
    EA_DEVICE_STATE_OPEN,
    EA_DEVICE_STATE_CLOSE,
    EA_DEVICE_STATE_MAX,
}ea_device_state_t;

typedef struct
{
    ea_session_state_t state;
    bool protocol_id_valid;
    uint8_t protocol_id;
    uint16_t protocol_sess_id;
    const uint8_t* ea_protocol_name;
    uint8_t name_len;   // include '\0'
    uint8_t match_action;
}ea_protocol_group_t;

#define SESSION_COUNT_MAX (2)

typedef struct
{
    iap2_link_info_t link_info;
    ea_device_state_t dev_state;
    ea_protocol_group_t pro_group[SESSION_COUNT_MAX];
}ea_device_t;

typedef bool (*ext_accessory_protocol_group_cb)(ea_protocol_group_t** group, uint8_t* count);
typedef void (*ext_accessory_notify_start_ea_session_cb)(iap2_link_info_t* link_info, uint8_t protocol_id, uint16_t protocol_sess_id);
typedef void (*ext_accessory_notify_stop_ea_session_cb)(iap2_link_info_t* link_info, uint8_t protocol_id);
typedef void (*ext_accessory_notify_data_recv_cb)(iap2_link_info_t* link_info, uint8_t protocol_id, uint16_t data_len, uint8_t* buf);

typedef struct ext_accessory_cb
{
    ext_accessory_protocol_group_cb group_cb;
    ext_accessory_notify_start_ea_session_cb start_ea_cb;
    ext_accessory_notify_stop_ea_session_cb stop_ea_cb;
    ext_accessory_notify_data_recv_cb ea_data_recv_cb;
}ext_accessory_cb_t;

void mfi_fea_ext_accessory_device_register_callback(ext_accessory_cb_t ea_cb);
ea_protocol_group_t* mfi_fea_ext_accessory_get_pro_group_table(iap2_link_info_t* link_info);
bool mfi_fea_ext_accessory_send_data(iap2_link_info_t* link_info, uint16_t protocol_id, uint16_t data_len, uint8_t* buf);
uint32_t mfi_fea_ext_accessory_devcie_save_ctx(iap2_link_info_t* link_info, uint8_t *buf, uint32_t buf_len);
uint32_t mfi_fea_ext_accessory_devcie_restore_ctx(iap2_link_info_t* link_info, uint8_t *buf, uint32_t buf_len);
ea_session_state_t mfi_fea_ext_accessory_device_get_protocol_state(iap2_link_info_t* link_info, uint8_t protocol_id, uint16_t* protocol_sess_id);
bool mfi_fea_ext_accessory_send_status_ea_session(iap2_link_info_t* link_info, uint16_t session_id, uint8_t status);
void mfi_fea_ext_accessory_init(void);

#ifdef __cplusplus
}
#endif
#endif //__EXT_ACCESSORY_H__