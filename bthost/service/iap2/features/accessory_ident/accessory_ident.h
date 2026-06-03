#ifndef __ACCESSORY_IDENT_H__
#define __ACCESSORY_IDENT_H__
#include <stdint.h>
#include "adapter_common_type.h"

#ifdef __cplusplus
extern "C" {
#endif

//
// Control session messages (identification)
//
#define StartIdentification                                                 0x1D00 /* from device */

#define IdentificationInformation                                           0x1D01 /* from accessory */
#define IdentificationInformation_id_Name                                           0
#define IdentificationInformation_id_ModelIdentifier                                1
#define IdentificationInformation_id_Manufacturer                                   2
#define IdentificationInformation_id_SerialNumber                                   3
#define IdentificationInformation_id_FirmwareVersion                                4
#define IdentificationInformation_id_HardwareVersion                                5
#define IdentificationInformation_id_MessagesSentByAccessory                        6
#define IdentificationInformation_id_MessagesReceivedFromDevice                     7
#define IdentificationInformation_id_PowerProvidingCapability                       8
#define IdentificationInformation_id_MaximumCurrentDrawnFromDevice                  9
#define IdentificationInformation_id_SupportedExternalAccessoryProtocol             10

#define ExternalAccessoryProtocolGroup_id_ExternalAccessoryProtocolIdentifier       0
#define ExternalAccessoryProtocolGroup_id_ExternalAccessoryProtocolName             1
#define ExternalAccessoryProtocolGroup_id_ExternalAccessoryMatchAction              2
#define ExternalAccessoryProtocolGroup_id_NativeTransportComponentIdentifier        3

#define IdentificationInformation_id_AppMatchTeamID                                 11
#define IdentificationInformation_id_CurrentLanguage                                12
#define IdentificationInformation_id_SupportedLanguage                              13
#define IdentificationInformation_id_BluetoothTransportComponent                    17
#define IdentificationInformation_id_ProductPlanUID                                 34

#define BluetoothTransportComponent_id_TransportComponentIdentifier                 0
#define BluetoothTransportComponent_id_TransportComponentName                       1
#define BluetoothTransportComponent_id_TransportSupportsiAP2Connection              2
#define BluetoothTransportComponent_id_BluetoothTransportMediaAccessControlAddress  3

#define IdentificationInformation_id_LocationInformationComponent                   22
#define IdentificationInformation_id_BluetoothHIDComponent                          22

#define IdentificationAccepted                                              0x1D02 /* from device */

// a subset of IdentificationInformation parameters are used as is */
#define IdentificationRejected                                              0x1D03 /* from device */

#define CancelIdentification                                                0x1D05 /* from accessory */

// a subset of IdentificationInformation parameters are used as is */
#define IdentificationInformationUpdate                                     0x1D06 /* from accessory */

typedef enum
{
    IDENT_STATE_NONE,
    IDENT_STATE_IDENT_ACCEPTED,
    IDENT_STATE_IDENT_FAILED,
    IDENT_STATE_MAX,
}accessory_ident_state_t;

typedef struct
{
    iap2_link_info_t link_info;
    accessory_ident_state_t state;
}accessory_ident_device_t;

/**
 * @brief
 *
 * @param[in] param_id: such as IdentificationInformation_id_Name
 * @param[in] sub_param_id:
 * @param[in] buff: buff that save the param_id's value
 * @param[in] len: the length of param_id, if param_Id is utf8(String) type, len will include Null terminated
 *
 * @return bool: true when customer redefine this param_id, Otherwise, it should be false
 */
typedef bool (*ident_get_custom_config_t)(uint16_t param_id, uint8_t sub_param_id, uint8_t** buff, uint16_t* len);

void mfi_fea_accessory_ident_device_register_config_callback(ident_get_custom_config_t cb);
uint32_t mfi_fea_accessory_ident_devcie_save_ctx(iap2_link_info_t* link_info, uint8_t *buf, uint32_t buf_len);
uint32_t mfi_fea_accessory_ident_devcie_restore_ctx(iap2_link_info_t* link_info, uint8_t *buf, uint32_t buf_len);
accessory_ident_state_t mfi_fea_accessory_ident_device_state(iap2_link_info_t* link_info);
void mfi_fea_accessory_ident_init(void);

#ifdef __cplusplus
}
#endif
#endif //__ACCESSORY_IDENT_H__