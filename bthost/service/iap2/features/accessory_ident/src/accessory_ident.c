#include "accessory_ident.h"
#include "iap2_link_adapter.h"
#include "apple_cert_adapter.h"
#include "nvrecord_bt.h"
#include "mfi_feat_common_define.h"
#include "ext_accessory.h"
#include "app_launch.h"
#include "bt_common_define.h"

accessory_ident_device_t ident_device[IAP2_IAP2_CONN_NUM];

ident_get_custom_config_t ident_get_custom_config_cb = NULL;

uint8_t bes_firmware_version[]               = {'0','.','1', '.', '0', 0 };

/* Following variables will be shared with iOS device during iAP2 Identification */
uint8_t iap2_ModelIdentifier[]               = { 'A', 'M', 'A', 'D', 'E', 'M', 'O', 0 };
uint8_t iap2_Manufacturer[]                  = { 'B', 'E', 'S', 0 };
uint8_t iap2_SerialNumber[]                  = { '0', '0', '0', '1', '0', '9', '7', '9', '5', '3', '8', '7', '2', '9', 0 };

uint8_t MessagesSentByAccessory[]                   = {0xEA, 0x02}; // RequestAppLaunch
uint8_t MessagesReceivedFromDevice[]                = {
    IAP2_HI_BYTE(StartExternalAccessoryProtocolSession), IAP2_LO_BYTE(StartExternalAccessoryProtocolSession),
    IAP2_HI_BYTE(StopExternalAccessoryProtocolSession), IAP2_LO_BYTE(StopExternalAccessoryProtocolSession)
};

#define IAP2_POWER_PROVIDING_CAPABILITY_NONE        0 // Shall be None if the accessory does not provide power to the device
uint8_t PowerProvidingCapability[]                  = { IAP2_POWER_PROVIDING_CAPABILITY_NONE };

#define IAP2_MAX_POWER_DRAWN_FROM_THE_DEVICE_NONE   0   // Shall be 0 if accessory does not draw power from the device
uint8_t MaximumCurrentDrawnFromDevice[]             = { IAP2_HI_BYTE(IAP2_MAX_POWER_DRAWN_FROM_THE_DEVICE_NONE), IAP2_LO_BYTE(IAP2_MAX_POWER_DRAWN_FROM_THE_DEVICE_NONE) };

uint8_t iap2_CurrentLanguage[]               = { 'e', 'n', 0 };
uint8_t iap2_SupportedLanguage[]             = { 'e', 'n', 0, 0 };

uint8_t iap2_AppMatchTeamId[]                = { '9', '4', 'K', 'V', '3', 'E', '6', '2', '6', 'L', 0 };

uint8_t iap2_HardwareVersion[]               = {'0','.','1', '.', '0', 0 };

#define IAP2_BLUETOOTH_TRANSPORT_COMPONENT_ID       1
uint8_t TransportComponentId[]                      = { IAP2_HI_BYTE(IAP2_BLUETOOTH_TRANSPORT_COMPONENT_ID), IAP2_LO_BYTE(IAP2_BLUETOOTH_TRANSPORT_COMPONENT_ID) };
uint8_t TransportComponentName[]                    = { 'B', 'l', 'u', 'e', 't', 'o', 'o', 't', 'h', 0 };

uint8_t iap2_ProductPlanUID[]                = { '3', '0', '2', '4', 'e', '0', 'b', '1', 'f', '5', '4', '9', '4', '5', '3', '7', 0 };

void bes_firmware_version_get(uint8_t* pVerStr)
{
    memcpy((void*)pVerStr,(void*)bes_firmware_version, strlen((const char*)bes_firmware_version));
}

static accessory_ident_device_t* search_accessory_ident_device_by_link_info( iap2_link_info_t* link_info)
{
    accessory_ident_device_t* device = NULL;

    for (int i = 0; i < IAP2_IAP2_CONN_NUM; i++)
    {
        device = &(ident_device[i]);
        
        if (device->link_info.type == link_info->type
            && memcmp(&(device->link_info.addr), &link_info->addr, sizeof(iap2_link_addr_t)) == 0)
        {
            return device;
        }
    }

    return NULL;
}

static accessory_ident_device_t* get_free_accessory_ident_device(void)
{
    accessory_ident_device_t* device = NULL;

    for (int i = 0; i < IAP2_IAP2_CONN_NUM; i++)
    {
        device = &(ident_device[i]);

        if (device->state == IDENT_STATE_NONE)
        {
            memset(device, 0, sizeof(accessory_ident_device_t));
            device->state = IDENT_STATE_NONE;
            return device;
        }
    }

    return NULL;
}

static void clear_accessory_ident_device(accessory_ident_device_t* device)
{
    if (device)
    {
        memset(device, 0, sizeof(accessory_ident_device_t));
        device->state = IDENT_STATE_NONE;
    }
}

uint16_t iap2_ctrl_sess_add_param_SupportedExternalAccessoryProtocol(iap2_link_info_t* link_info, iap2_ctrl_sess_param_t *p)
{
    uint8_t total_param_len = 0;
    iap2_ctrl_sess_param_t* group_start = p;
    iap2_ctrl_sess_param_t* sub_param = NULL;
    uint8_t len_of_single_param = 0;
    ea_protocol_group_t* ea_group = NULL;
    ea_protocol_group_t* ea_table = mfi_fea_ext_accessory_get_pro_group_table(link_info);
    uint8_t group_len = 0;

    if (ea_table == NULL)
    {
        DEBUG_INFO(0, "MFI_IDENT_ERROR line %d ea_table NULL", __LINE__);
    }

    for (uint8_t i = 0; i < SESSION_COUNT_MAX; i++)
    {
        ea_group = &ea_table[i];
        DEBUG_INFO(0, "MFI_IDENT line %d i %d ea_group->protocol_id_valid %d", __LINE__, i, ea_group->protocol_id_valid);
        if (ea_group->protocol_id_valid == false)
        {
            continue;
        }

        group_len = 0;

        group_start = (iap2_ctrl_sess_param_t*)((uint8_t*)group_start + total_param_len);
        DEBUG_INFO(0, "MFI_IDENT line %d i %d group_start %p", __LINE__, i, group_start);
        group_start->param_id_msb = IdentificationInformation_id_SupportedExternalAccessoryProtocol >> 8;
        group_start->param_id_lsb = IdentificationInformation_id_SupportedExternalAccessoryProtocol & 0xFF;
        group_len = IAP2_CONTROL_SESSION_PARAMETER_HEADER_SIZE;

        sub_param = (iap2_ctrl_sess_param_t*)((uint8_t*)group_start + group_len);
        DEBUG_INFO(0, "MFI_IDENT line %d i %d ea_group->protocol_id %d", __LINE__, i, ea_group->protocol_id);
        len_of_single_param = iap2_ctrl_sess_add_param(sub_param, ExternalAccessoryProtocolGroup_id_ExternalAccessoryProtocolIdentifier, (uint8_t*)&ea_group->protocol_id, 1);
        group_len += len_of_single_param;

        sub_param = (iap2_ctrl_sess_param_t*)((uint8_t*)group_start + group_len);
        DEBUG_INFO(0, "MFI_IDENT line %d i %d ea_group->ea_protocol_name %s", __LINE__, i, ea_group->ea_protocol_name);
        len_of_single_param = iap2_ctrl_sess_add_param(sub_param, ExternalAccessoryProtocolGroup_id_ExternalAccessoryProtocolName, (uint8_t*)ea_group->ea_protocol_name, ea_group->name_len);
        group_len += len_of_single_param;

        sub_param = (iap2_ctrl_sess_param_t*)((uint8_t*)group_start + group_len);
        DEBUG_INFO(0, "MFI_IDENT line %d i %d ea_group->match_action %d", __LINE__, i, ea_group->match_action);
        len_of_single_param = iap2_ctrl_sess_add_param(sub_param, ExternalAccessoryProtocolGroup_id_ExternalAccessoryMatchAction, (uint8_t*)&ea_group->match_action, 1);
        group_len += len_of_single_param;
        total_param_len += group_len;

        group_start->param_len_msb = group_len >> 8;
        group_start->param_len_lsb = group_len & 0xFF;
        DEBUG_INFO(0, "MFI_IDENT line %d i %d total_param_len %d", __LINE__, i, total_param_len);
    }

    return total_param_len;
}

/*
 * Add Bluetotoh Transport component group
 */
uint8_t iap2_ctrl_sess_add_bluetooth_transport_component_group(iap2_ctrl_sess_param_t *p)
{
    iap2_ctrl_sess_param_t* group_start = p;
    uint8_t BluetoothTransportMACAddress[6];
    int ret = 0;
    uint8_t total_param_len = 0;
    uint8_t len_of_single_param = 0;
    char flsh_dongle_addr[6];
    iap2_ctrl_sess_param_t* sub_param = NULL;
    uint8_t* p_param = NULL;
    uint16_t param_len = 0;

    ret = nvrec_dev_get_btaddr(flsh_dongle_addr);
    if(ret == 0){
        DEBUG_INFO(0, "get bt addr fail\r\n ");
        return total_param_len;
    }
    for (uint8_t i = 0; i < 6; i++) {
        BluetoothTransportMACAddress[i] = flsh_dongle_addr[5-i];
        DEBUG_INFO(0, " 0x%x ",BluetoothTransportMACAddress[i]);
    }

    group_start->param_id_msb = IdentificationInformation_id_BluetoothTransportComponent >> 8;
    group_start->param_id_lsb = IdentificationInformation_id_BluetoothTransportComponent & 0xFF;
    total_param_len += IAP2_CONTROL_SESSION_PARAMETER_HEADER_SIZE;

    sub_param = (iap2_ctrl_sess_param_t*)((uint8_t*)group_start + total_param_len);
    if (ident_get_custom_config_cb && ident_get_custom_config_cb(IdentificationInformation_id_BluetoothTransportComponent, BluetoothTransportComponent_id_TransportComponentIdentifier, &p_param, &param_len))
    {
        len_of_single_param = iap2_ctrl_sess_add_param(sub_param, BluetoothTransportComponent_id_TransportComponentIdentifier, p_param, param_len);
    }
    else
    {
        len_of_single_param = iap2_ctrl_sess_add_param(sub_param, BluetoothTransportComponent_id_TransportComponentIdentifier, TransportComponentId, sizeof(TransportComponentId));
    }
    total_param_len += len_of_single_param;

    sub_param = (iap2_ctrl_sess_param_t*)((uint8_t*)group_start + total_param_len);
    if (ident_get_custom_config_cb && ident_get_custom_config_cb(IdentificationInformation_id_BluetoothTransportComponent, BluetoothTransportComponent_id_TransportComponentName, &p_param, &param_len))
    {
        len_of_single_param = iap2_ctrl_sess_add_param(sub_param, BluetoothTransportComponent_id_TransportComponentName, p_param, param_len);
    }
    else
    {
        len_of_single_param = iap2_ctrl_sess_add_param(sub_param, BluetoothTransportComponent_id_TransportComponentName, TransportComponentName, sizeof(TransportComponentName));
    }
    total_param_len += len_of_single_param;

    sub_param = (iap2_ctrl_sess_param_t*)((uint8_t*)group_start + total_param_len);
    if (ident_get_custom_config_cb && ident_get_custom_config_cb(IdentificationInformation_id_BluetoothTransportComponent, BluetoothTransportComponent_id_TransportSupportsiAP2Connection, &p_param, &param_len))
    {
        len_of_single_param = iap2_ctrl_sess_add_param(sub_param, BluetoothTransportComponent_id_TransportSupportsiAP2Connection, p_param, param_len);
    }
    else
    {
        len_of_single_param = iap2_ctrl_sess_add_param(sub_param, BluetoothTransportComponent_id_TransportSupportsiAP2Connection, NULL, 0);
    }
    total_param_len += len_of_single_param;

    sub_param = (iap2_ctrl_sess_param_t*)((uint8_t*)group_start + total_param_len);
    if (ident_get_custom_config_cb && ident_get_custom_config_cb(IdentificationInformation_id_BluetoothTransportComponent, BluetoothTransportComponent_id_BluetoothTransportMediaAccessControlAddress, &p_param, &param_len))
    {
        len_of_single_param = iap2_ctrl_sess_add_param(sub_param, BluetoothTransportComponent_id_BluetoothTransportMediaAccessControlAddress, p_param, param_len);
    }
    else
    {
        len_of_single_param = iap2_ctrl_sess_add_param(sub_param, BluetoothTransportComponent_id_BluetoothTransportMediaAccessControlAddress, BluetoothTransportMACAddress, sizeof(BluetoothTransportMACAddress));
    }
    total_param_len += len_of_single_param;

    group_start->param_len_msb = total_param_len >> 8;
    group_start->param_len_lsb = total_param_len & 0xFF;

    return total_param_len;
}

void mfi_fea_accessory_ident_device_prepare_identinfo(accessory_ident_device_t* device, iap2_ctrl_sess_payload_t* payload, uint16_t* payload_len)
{
    uint16_t len_of_single_param = 0;
    iap2_ctrl_sess_param_t* param_start = NULL;
    u8* pt = NULL;
    uint8_t version[10]={0};
    payload->header.message_id_msb = IdentificationInformation >> 8;
    payload->header.message_id_lsb = IdentificationInformation & 0xFF;
    uint16_t total_payload_len = IAP2_CONTROL_SESSION_HEADER_SIZE;
    uint8_t* p_param = NULL;
    uint16_t param_len = 0;

    bes_firmware_version_get(version);
    // start add param
    param_start = (iap2_ctrl_sess_param_t*)((uint8_t*)payload + total_payload_len);
    if (ident_get_custom_config_cb && ident_get_custom_config_cb(IdentificationInformation_id_Name, 0, &p_param, &param_len))
    {
        len_of_single_param = iap2_ctrl_sess_add_param(param_start, IdentificationInformation_id_Name, p_param, param_len);
    }
    else
    {
        len_of_single_param = iap2_ctrl_sess_add_param(param_start, IdentificationInformation_id_Name, (uint8_t *)nvrec_dev_get_bt_name(), strlen((char *)nvrec_dev_get_bt_name()) + 1);
    }
    total_payload_len += len_of_single_param;

    param_start = (iap2_ctrl_sess_param_t*)((uint8_t*)payload + total_payload_len);
    if (ident_get_custom_config_cb && ident_get_custom_config_cb(IdentificationInformation_id_ModelIdentifier, 0, &p_param, &param_len))
    {
        len_of_single_param = iap2_ctrl_sess_add_param(param_start, IdentificationInformation_id_ModelIdentifier, p_param, param_len);
    }
    else
    {
        len_of_single_param = iap2_ctrl_sess_add_param(param_start, IdentificationInformation_id_ModelIdentifier, iap2_ModelIdentifier, strlen((char *)iap2_ModelIdentifier) + 1);
    }
    total_payload_len += len_of_single_param;

    param_start = (iap2_ctrl_sess_param_t*)((uint8_t*)payload + total_payload_len);
    if (ident_get_custom_config_cb && ident_get_custom_config_cb(IdentificationInformation_id_Manufacturer, 0, &p_param, &param_len))
    {
        len_of_single_param = iap2_ctrl_sess_add_param(param_start, IdentificationInformation_id_Manufacturer, p_param, param_len);
    }
    else
    {
        len_of_single_param = iap2_ctrl_sess_add_param(param_start, IdentificationInformation_id_Manufacturer, iap2_Manufacturer, strlen((char *)iap2_Manufacturer) + 1);
    }
    total_payload_len += len_of_single_param;

    param_start = (iap2_ctrl_sess_param_t*)((uint8_t*)payload + total_payload_len);
    if (ident_get_custom_config_cb && ident_get_custom_config_cb(IdentificationInformation_id_SerialNumber, 0, &p_param, &param_len))
    {
        len_of_single_param = iap2_ctrl_sess_add_param(param_start, IdentificationInformation_id_SerialNumber, p_param, param_len);
    }
    else
    {
        len_of_single_param = iap2_ctrl_sess_add_param(param_start, IdentificationInformation_id_SerialNumber, iap2_SerialNumber, strlen((char *)iap2_SerialNumber) + 1);
    }
    total_payload_len += len_of_single_param;

    param_start = (iap2_ctrl_sess_param_t*)((uint8_t*)payload + total_payload_len);
    if (ident_get_custom_config_cb && ident_get_custom_config_cb(IdentificationInformation_id_FirmwareVersion, 0, &p_param, &param_len))
    {
        len_of_single_param = iap2_ctrl_sess_add_param(param_start, IdentificationInformation_id_FirmwareVersion, p_param, param_len);
    }
    else
    {
        len_of_single_param = iap2_ctrl_sess_add_param(param_start, IdentificationInformation_id_FirmwareVersion, version, strlen((char *)version) + 1);
    }
    total_payload_len += len_of_single_param;

    param_start = (iap2_ctrl_sess_param_t*)((uint8_t*)payload + total_payload_len);
    if (ident_get_custom_config_cb && ident_get_custom_config_cb(IdentificationInformation_id_HardwareVersion, 0, &p_param, &param_len))
    {
        len_of_single_param = iap2_ctrl_sess_add_param(param_start, IdentificationInformation_id_HardwareVersion, p_param, param_len);
    }
    else
    {
        len_of_single_param = iap2_ctrl_sess_add_param(param_start, IdentificationInformation_id_HardwareVersion, iap2_HardwareVersion, strlen((char *)iap2_HardwareVersion) + 1);
    }
    total_payload_len += len_of_single_param;

    param_start = (iap2_ctrl_sess_param_t*)((uint8_t*)payload + total_payload_len);
    if (ident_get_custom_config_cb && ident_get_custom_config_cb(IdentificationInformation_id_MessagesSentByAccessory, 0, &p_param, &param_len))
    {
        len_of_single_param = iap2_ctrl_sess_add_param(param_start, IdentificationInformation_id_MessagesSentByAccessory, p_param, param_len);
    }
    else
    {
        len_of_single_param = iap2_ctrl_sess_add_param(param_start, IdentificationInformation_id_MessagesSentByAccessory, MessagesSentByAccessory, sizeof(MessagesSentByAccessory));
    }
    total_payload_len += len_of_single_param;

    param_start = (iap2_ctrl_sess_param_t*)((uint8_t*)payload + total_payload_len);
    if (ident_get_custom_config_cb && ident_get_custom_config_cb(IdentificationInformation_id_MessagesReceivedFromDevice, 0, &p_param, &param_len))
    {
        len_of_single_param = iap2_ctrl_sess_add_param(param_start, IdentificationInformation_id_MessagesReceivedFromDevice, p_param, param_len);
    }
    else
    {
        len_of_single_param = iap2_ctrl_sess_add_param(param_start, IdentificationInformation_id_MessagesReceivedFromDevice, MessagesReceivedFromDevice, sizeof(MessagesReceivedFromDevice));
    }
    total_payload_len += len_of_single_param;

    param_start = (iap2_ctrl_sess_param_t*)((uint8_t*)payload + total_payload_len);
    if (ident_get_custom_config_cb && ident_get_custom_config_cb(IdentificationInformation_id_PowerProvidingCapability, 0, &p_param, &param_len))
    {
        len_of_single_param = iap2_ctrl_sess_add_param(param_start, IdentificationInformation_id_PowerProvidingCapability, p_param, param_len);
    }
    else
    {
        len_of_single_param = iap2_ctrl_sess_add_param(param_start, IdentificationInformation_id_PowerProvidingCapability, PowerProvidingCapability, sizeof(PowerProvidingCapability));
    }
    total_payload_len += len_of_single_param;

    param_start = (iap2_ctrl_sess_param_t*)((uint8_t*)payload + total_payload_len);
    if (ident_get_custom_config_cb && ident_get_custom_config_cb(IdentificationInformation_id_MaximumCurrentDrawnFromDevice, 0, &p_param, &param_len))
    {
        len_of_single_param = iap2_ctrl_sess_add_param(param_start, IdentificationInformation_id_MaximumCurrentDrawnFromDevice, p_param, param_len);
    }
    else
    {
        len_of_single_param = iap2_ctrl_sess_add_param(param_start, IdentificationInformation_id_MaximumCurrentDrawnFromDevice, MaximumCurrentDrawnFromDevice, sizeof(MaximumCurrentDrawnFromDevice));
    }
    total_payload_len += len_of_single_param;

    param_start = (iap2_ctrl_sess_param_t*)((uint8_t*)payload + total_payload_len);
    len_of_single_param = iap2_ctrl_sess_add_param_SupportedExternalAccessoryProtocol(&device->link_info, param_start);
    total_payload_len += len_of_single_param;

    param_start = (iap2_ctrl_sess_param_t*)((uint8_t*)payload + total_payload_len);
    if (ident_get_custom_config_cb && ident_get_custom_config_cb(IdentificationInformation_id_AppMatchTeamID, 0, &p_param, &param_len))
    {
        len_of_single_param = iap2_ctrl_sess_add_param(param_start, IdentificationInformation_id_AppMatchTeamID, p_param, param_len);
    }
    else
    {
        len_of_single_param = iap2_ctrl_sess_add_param(param_start, IdentificationInformation_id_AppMatchTeamID, iap2_AppMatchTeamId, strlen((char *)iap2_AppMatchTeamId) + 1);
    }
    total_payload_len += len_of_single_param;

    param_start = (iap2_ctrl_sess_param_t*)((uint8_t*)payload + total_payload_len);
    if (ident_get_custom_config_cb && ident_get_custom_config_cb(IdentificationInformation_id_CurrentLanguage, 0, &p_param, &param_len))
    {
        len_of_single_param = iap2_ctrl_sess_add_param(param_start, IdentificationInformation_id_CurrentLanguage, p_param, param_len);
    }
    else
    {
        len_of_single_param = iap2_ctrl_sess_add_param(param_start, IdentificationInformation_id_CurrentLanguage, iap2_CurrentLanguage, strlen((char *)iap2_CurrentLanguage) + 1);
    }
    total_payload_len += len_of_single_param;

    if (ident_get_custom_config_cb && ident_get_custom_config_cb(IdentificationInformation_id_SupportedLanguage, 0, &p_param, &param_len))
    {
        for (; *p_param != 0; p_param += 3)
        {
            param_start = (iap2_ctrl_sess_param_t*)((uint8_t*)payload + total_payload_len);
            len_of_single_param = iap2_ctrl_sess_add_param(param_start, IdentificationInformation_id_SupportedLanguage, p_param, 3);
            total_payload_len += len_of_single_param;
        }
    }
    else
    {
        for (pt = iap2_SupportedLanguage; *pt != 0; pt += 3)
        {
            param_start = (iap2_ctrl_sess_param_t*)((uint8_t*)payload + total_payload_len);
            len_of_single_param = iap2_ctrl_sess_add_param(param_start, IdentificationInformation_id_SupportedLanguage, pt, 3);
            total_payload_len += len_of_single_param;
        }
    }

    param_start = (iap2_ctrl_sess_param_t*)((uint8_t*)payload + total_payload_len);
    len_of_single_param = iap2_ctrl_sess_add_bluetooth_transport_component_group(param_start);
    total_payload_len += len_of_single_param;

    param_start = (iap2_ctrl_sess_param_t*)((uint8_t*)payload + total_payload_len);
    if (ident_get_custom_config_cb && ident_get_custom_config_cb(IdentificationInformation_id_ProductPlanUID, 0, &p_param, &param_len))
    {
        len_of_single_param = iap2_ctrl_sess_add_param(param_start, IdentificationInformation_id_ProductPlanUID, p_param, param_len);
    }
    else
    {
        len_of_single_param = iap2_ctrl_sess_add_param(param_start, IdentificationInformation_id_ProductPlanUID, iap2_ProductPlanUID, strlen((char *)iap2_ProductPlanUID) + 1);
    }
    total_payload_len += len_of_single_param;

    payload->header.message_len_msb = total_payload_len >> 8;
    payload->header.message_len_lsb = total_payload_len & 0xFF;
    *payload_len = total_payload_len;
}

void mfi_fea_accessory_ident_device_handle_ctrl_session_data(accessory_ident_device_t* device, uint16_t data_len, uint8_t* data)
{
    uint16_t total_payload_len = 0;
    iap2_ctrl_sess_payload_header_t *ctrl_data = (iap2_ctrl_sess_payload_header_t*)data;
    iap2_ctrl_sess_payload_t payload = {.header.som_msb = 0x40, .header.som_lsb = 0x40};

    POSSIBLY_UNUSED uint16_t message_len = ((ctrl_data->message_len_msb << 8) | ctrl_data->message_len_lsb) - IAP2_CONTROL_SESSION_HEADER_SIZE;
    POSSIBLY_UNUSED uint16_t message_id = (ctrl_data->message_id_msb << 8) | ctrl_data->message_id_lsb;

    POSSIBLY_UNUSED uint16_t param_len = 0;
    POSSIBLY_UNUSED uint16_t param_id = 0;
    DEBUG_INFO(0, "MFI_IDENT_recv_ctrl, message_id 0x%x", message_id);
    switch (message_id)
    {
    case StartIdentification:
    {
        mfi_fea_accessory_ident_device_prepare_identinfo(device, &payload, &total_payload_len);
        if (total_payload_len > 0)
        {
            iap2_Link_adapter_send_control_session_data(&device->link_info, total_payload_len, (uint8_t*)&payload);
        }
    }
        break;
    case IdentificationAccepted:
    {
        DEBUG_INFO(0, "MFI_IDENT, IdentificationAccepted");
        device->state = IDENT_STATE_IDENT_ACCEPTED;
         // for test app_launch. test ok
         // uint8_t weixin[] = "com.tencent.xin";
         // mfi_fea_app_launch(&device->link_info, weixin, strlen((char *)weixin)+1, AppLaunchMethod_id_without_user_alert);
    }
        break;
    case IdentificationRejected:
    {
        DEBUG_INFO(0, "MFI_IDENT_ERROR, rejected, plz check");
    }
        break;
    default:

        break;
    }
}

void mfi_fea_accessory_ident_device_handle_event(const iap2_link_info_t* link_info, IAP2_CONN_EVENT_T event, iap2_subscriber_callback_param_t* param)
{
    DEBUG_INFO(0, "%s event %d", __func__, event);
    POSSIBLY_UNUSED accessory_ident_device_t* device = search_accessory_ident_device_by_link_info((iap2_link_info_t*)link_info);

    switch (event)
    {
    case IAP2_CONN_EVENT_OPEN:
    {
        if (device)
        {
            DEBUG_INFO(0, "MFI_IDENT_ERROR, already has device, plz check");
            return;
        }

        device = get_free_accessory_ident_device();
        if (device)
        {
            device->state = IDENT_STATE_NONE;
            device->link_info.type = link_info->type;
            memcpy(&device->link_info.addr, &link_info->addr, sizeof(iap2_link_addr_t));
            return;
        }
        else
        {
            DEBUG_INFO(0, "MFI_IDENT_ERROR line %d device NULL", __LINE__);
            return;
        }
    }
        break;
    case IAP2_CONN_EVENT_CLOSE:
    {
        if (device == NULL)
        {
            DEBUG_INFO(0, "MFI_IDENT_ERROR line %d device NULL", __LINE__);
            return;
        }
        
        clear_accessory_ident_device(device);
    }
        break;
    case IAP2_CONN_EVENT_TXDONE:
        break;
    case IAP2_CONN_EVENT_RECV_CTRL_DATA:
    {
        if (device == NULL)
        {
            DEBUG_INFO(0, "MFI_IDENT_ERROR line %d device NULL", __LINE__);
            return;
        }

        mfi_fea_accessory_ident_device_handle_ctrl_session_data(device, param->recv_ctrl_data.data_len, param->recv_ctrl_data.data);
    }
        break;
    default:
        DEBUG_INFO(0, "MFI_IDENT_ERROR line %d uninterested event", __LINE__);
        break;
    }
}

void mfi_fea_accessory_ident_device_register_config_callback(ident_get_custom_config_t cb)
{
    DEBUG_INFO(0, "MFI_IDENT line %d register cb", __LINE__);
    ident_get_custom_config_cb = cb;
}

uint32_t mfi_fea_accessory_ident_devcie_save_ctx(iap2_link_info_t* link_info, uint8_t *buf, uint32_t buf_len)
{
    uint32_t offset = 0;
    accessory_ident_device_t* device = search_accessory_ident_device_by_link_info(link_info);
    if (device == NULL)
    {
        buf[offset++] = false;
        return offset;
    }

    buf[offset++] = true;
    buf[offset++] = device->state;
    DEBUG_INFO(0, "MFI_IDENT_SAVE line %d state %d", __LINE__, device->state);

    return offset;
}

uint32_t mfi_fea_accessory_ident_devcie_restore_ctx(iap2_link_info_t* link_info, uint8_t *buf, uint32_t buf_len)
{
    uint32_t offset = 0;
    accessory_ident_device_t* device = NULL;

    if (buf[offset++] == false)
    {
        DEBUG_INFO(0, "MFI_IDENT_RESTORE line %d no ctx", __LINE__);
        return offset;
    }

    device = search_accessory_ident_device_by_link_info(link_info);

    if (device == NULL)
    {
        device = get_free_accessory_ident_device();
        if (device == NULL)
        {
            DEBUG_INFO(0, "MFI_IDENT_RESTORE line %d no free device", __LINE__);
            // todo: dump all device
            return offset;
        }

        device->link_info.type = link_info->type;
        memcpy(&(device->link_info.addr), &(link_info->addr), sizeof(iap2_link_addr_t));
    }

    device->state = buf[offset++];
    DEBUG_INFO(0, "MFI_IDENT_RESTORE line %d state %d", __LINE__, device->state);

    return offset;
}

accessory_ident_state_t mfi_fea_accessory_ident_device_state(iap2_link_info_t* link_info)
{
    accessory_ident_device_t* device = search_accessory_ident_device_by_link_info(link_info);

    if (device == NULL)
    {
        return IDENT_STATE_NONE;
    }
    else
    {
        return device->state;
    }
}

void mfi_fea_accessory_ident_init(void)
{
    DEBUG_INFO(0, "MFI_IDENT_INIT line %d ", __LINE__);

    for (uint8_t i = 0; i < IAP2_IAP2_CONN_NUM; i++)
    {
        memset(&ident_device[i], 0, sizeof(accessory_ident_device_t));
        ident_device[i].state = IDENT_STATE_NONE;
    }

    iap2_link_adapter_add_subscriber_callback(mfi_fea_accessory_ident_device_handle_event,
                                              (IAP2_CONN_EVENT_OPEN
                                              | IAP2_CONN_EVENT_CLOSE
                                              | IAP2_CONN_EVENT_TXDONE
                                              | IAP2_CONN_EVENT_RECV_CTRL_DATA));
}