#ifndef __ACCESSORY_AUTHEN_H__
#define __ACCESSORY_AUTHEN_H__
#include <stdint.h>
#include "adapter_common_type.h"

#ifdef __cplusplus
extern "C" {
#endif

//
// Control session messages (authentication)
//

#define RequestAuthenticationCertificate                                    0xAA00  /* from device */
#define RequestAuthenticationCertificate_id_SerialNumber                    0

#define AuthenticationCertificate                                           (0xAA01)  /* from accessory */
#define AuthenticationCertificate_id_AuthenticationCertificate              0

#define RequestAuthenticationChallengeResponse                              0xAA02  /* from device */
#define RequestAuthenticationChallengeResponse_id_AuthenticationChallenge   0

#define AuthenticationResponse                                              0xAA03  /* from accessory */
#define AuthenticationResponse_id_AuthenticationResponse                    0

#define AuthenticationFailed                                                0xAA04  /* from device */
#define AuthenticationSucceeded                                             0xAA05  /* from device */

#define AccessoryAuthenticationSerialNumber                                 0xAA06  /* */
#define AccessoryAuthenticationSerialNumber_id_AuthenticationSerialNumber   0x0000  /* */

typedef enum
{
    AUTHEN_STATE_NONE,
    AUTHEN_STATE_AUTHENED_SUCCESS,
    AUTHEN_STATE_AUTHENED_FAILED,
    AUTHEN_STATE_MAX,
}accessory_authen_state_t;

typedef struct
{
    iap2_link_info_t link_info;
    accessory_authen_state_t state;
}accessory_authen_device_t;

uint32_t mfi_fea_accessory_authen_devcie_save_ctx(iap2_link_info_t* link_info, uint8_t *buf, uint32_t buf_len);
uint32_t mfi_fea_accessory_authen_devcie_restore_ctx(iap2_link_info_t* link_info, uint8_t *buf, uint32_t buf_len);
accessory_authen_state_t mfi_fea_accessory_authen_device_state(iap2_link_info_t* link_info);
void mfi_fea_accessory_authen_init(void);

#ifdef __cplusplus
}
#endif
#endif //__ACCESSORY_AUTHEN_H__