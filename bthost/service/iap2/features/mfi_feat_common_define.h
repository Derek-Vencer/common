#ifndef __MFI_FEAT_COMMON_DEFINE_H__
#define __MFI_FEAT_COMMON_DEFINE_H__

#include <string.h>
#include <stdint.h>
#include "iap2_link_adapter.h"

// 6: Start of Message field, Message Length field and Message ID field
#define IAP2_CONTROL_SESSION_HEADER_SIZE            6
#define IAP2_CONTROL_SESSION_PARAMETER_HEADER_SIZE  4
#define IAP2_CONTROL_SESSION_DATA_MAX_LEN           (1300)
#define IAP2_EA_SESSION_DATA_MAX_LEN                (1300)
#define IAP2_EA_SESSION_ID_LEN                      (2)

#define IAP2_HI_BYTE(X) (((X) >> 8) & 0xFF)
#define IAP2_LO_BYTE(X) ((X) & 0xFF)

typedef struct
{
    uint8_t som_msb;    // 0x40
    uint8_t som_lsb;    // 0x40
    uint8_t message_len_msb;
    uint8_t message_len_lsb;
    uint8_t message_id_msb;
    uint8_t message_id_lsb;
} __attribute__ ((packed)) iap2_ctrl_sess_payload_header_t;

typedef struct
{
    iap2_ctrl_sess_payload_header_t header;
    uint8_t param[IAP2_CONTROL_SESSION_DATA_MAX_LEN];
} __attribute__ ((packed)) iap2_ctrl_sess_payload_t;

typedef struct
{
    uint8_t sess_id_B0;
    uint8_t sess_id_B1;
    uint8_t data[IAP2_EA_SESSION_DATA_MAX_LEN];
} __attribute__ ((packed)) iap2_ea_sess_payload_t;

typedef struct
{
    uint8_t param_len_msb;
    uint8_t param_len_lsb;
    uint8_t param_id_msb;
    uint8_t param_id_lsb;
    uint8_t data[0];
}__attribute__ ((packed)) iap2_ctrl_sess_param_t;

uint16_t iap2_ctrl_sess_add_param(iap2_ctrl_sess_param_t *p, uint16_t param_id, uint8_t *p_param, uint16_t param_len);

#endif // __MFI_FEAT_COMMON_DEFINE_H__