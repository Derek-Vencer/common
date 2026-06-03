#include <string.h>
#include "accessory_authen.h"
#include "iap2_link_adapter.h"
#include "apple_cert_adapter.h"
#include "mfi_feat_common_define.h"
#include "nvrecord_bt.h"
#include "bt_common_define.h"

accessory_authen_device_t authen_device[IAP2_IAP2_CONN_NUM];

static accessory_authen_device_t* search_authen_device_by_link_info(iap2_link_info_t* link_info)
{
    accessory_authen_device_t* device = NULL;

    for (int i = 0; i < IAP2_IAP2_CONN_NUM; i++)
    {
        device = &(authen_device[i]);
        
        if (device->link_info.type == link_info->type
            && memcmp(&(device->link_info.addr), &link_info->addr, sizeof(iap2_link_addr_t)) == 0)
        {
            return device;
        }
    }

    return NULL;
}

static accessory_authen_device_t* get_free_access_authen_device(void)
{
    accessory_authen_device_t* device = NULL;

    for (int i = 0; i < IAP2_IAP2_CONN_NUM; i++)
    {
        device = &(authen_device[i]);

        if (device->state == AUTHEN_STATE_NONE)
        {
            memset(device, 0, sizeof(accessory_authen_device_t));
            device->state = AUTHEN_STATE_NONE;
            return authen_device;
        }
    }

    return NULL;
}

static void clear_access_authen_device(accessory_authen_device_t* device)
{
    if (device)
    {
        memset(device, 0, sizeof(accessory_authen_device_t));
        device->state = AUTHEN_STATE_NONE;
    }
}

void mfi_fea_accessory_authen_device_handle_ctrl_session_data(accessory_authen_device_t* device, uint16_t data_len, uint8_t* data)
{
    iap2_ctrl_sess_payload_header_t *ctrl_data = (iap2_ctrl_sess_payload_header_t*)data;
    iap2_ctrl_sess_payload_t payload = {.header.som_msb = 0x40, .header.som_lsb = 0x40};
    uint16_t total_payload_len = 0;
    uint16_t len_of_single_param = 0;
    iap2_ctrl_sess_param_t* param_start = NULL;

    uint16_t message_len = ((ctrl_data->message_len_msb << 8) | ctrl_data->message_len_lsb) - IAP2_CONTROL_SESSION_HEADER_SIZE;
    uint16_t message_id = (ctrl_data->message_id_msb << 8) | ctrl_data->message_id_lsb;

    uint16_t param_len = 0;
    uint16_t param_id = 0;

    switch (message_id)
    {
    case RequestAuthenticationCertificate:
    {
        if (message_len >= 4)
        {
            iap2_ctrl_sess_param_t* param_header = (iap2_ctrl_sess_param_t*)(data + IAP2_CONTROL_SESSION_HEADER_SIZE);
            param_len = (param_header->param_len_msb << 8) | param_header->param_len_lsb;
            param_id = (param_header->param_id_msb << 8) | param_header->param_id_lsb;

            if (param_id == RequestAuthenticationCertificate_id_SerialNumber)
            {
                int len = 0;
                // control session payload header
                payload.header.message_id_msb = AccessoryAuthenticationSerialNumber >> 8;
                payload.header.message_id_lsb = AccessoryAuthenticationSerialNumber & 0xFF;
                total_payload_len = IAP2_CONTROL_SESSION_HEADER_SIZE;

                // start add param
                param_start = (iap2_ctrl_sess_param_t*)payload.param;
                len_of_single_param = 2 + 2; // 2: Parameter Length 2: Parameter
                param_start->param_id_msb = AccessoryAuthenticationSerialNumber_id_AuthenticationSerialNumber >> 8;
                param_start->param_id_lsb = AccessoryAuthenticationSerialNumber_id_AuthenticationSerialNumber & 0xFF;
                len = apple_cert_adpt_read_accessory_certificate_serial_number(param_start->data, 32); // TODO: check 32
                len_of_single_param += len;
                param_start->param_len_msb = len_of_single_param >> 8;
                param_start->param_len_lsb = (uint8_t) (len_of_single_param & 0xFF);

                total_payload_len += len_of_single_param;
                payload.header.message_len_msb = total_payload_len >> 8;
                payload.header.message_len_lsb = (uint8_t) (total_payload_len & 0xFF);

                iap2_Link_adapter_send_control_session_data(&device->link_info, total_payload_len, (uint8_t*)&payload);
            }
        }
        else if (message_len == 0)
        {
                int len = 0;
                // control session payload header
                payload.header.message_id_msb = AuthenticationCertificate >> 8;
                payload.header.message_id_lsb = AuthenticationCertificate & 0xFF;
                total_payload_len = IAP2_CONTROL_SESSION_HEADER_SIZE;

                // start add param
                param_start = (iap2_ctrl_sess_param_t*)payload.param;
                param_start->param_id_msb = AuthenticationCertificate_id_AuthenticationCertificate >> 8;
                param_start->param_id_lsb = AuthenticationCertificate_id_AuthenticationCertificate & 0xFF;
                len_of_single_param = 2 + 2; // 2: Parameter Length 2: Parameter id
                len = apple_cert_adpt_read_accessory_certificate_data(param_start->data, 1280);
                len_of_single_param += len;
                param_start->param_len_msb = len_of_single_param >> 8;
                param_start->param_len_lsb = (uint8_t) (len_of_single_param & 0xFF);

                total_payload_len += len_of_single_param;
                payload.header.message_len_msb = total_payload_len >> 8;
                payload.header.message_len_lsb = (uint8_t) (total_payload_len & 0xFF);

                iap2_Link_adapter_send_control_session_data(&device->link_info, total_payload_len, (uint8_t*)&payload);
        }
    }
        break;
    case RequestAuthenticationChallengeResponse:
    {
        if (message_len >= 4)
        {
            int len = 0;
            uint8_t* recv_challenge_data = NULL;
            uint16_t recv_challenge_data_len = 0;
            iap2_ctrl_sess_param_t* param_header = (iap2_ctrl_sess_param_t*)(data + IAP2_CONTROL_SESSION_HEADER_SIZE);

            param_len = (param_header->param_len_msb << 8) | param_header->param_len_lsb;
            param_id = (param_header->param_id_msb << 8) | param_header->param_id_lsb;

            if (param_id != RequestAuthenticationChallengeResponse_id_AuthenticationChallenge
                ||  param_len != data_len - IAP2_CONTROL_SESSION_HEADER_SIZE)
            {
                DEBUG_INFO(0, "MFI_AUTH_ERROR line %d type = %d len = %d \r\n", __LINE__, param_id, param_len);
                break;
            }

            // prepare challenge data
            recv_challenge_data_len = param_len - IAP2_CONTROL_SESSION_PARAMETER_HEADER_SIZE;
            recv_challenge_data = param_header->data;

            DEBUG_INFO(0, "MFI_AUTH line %d chall_data %d %d ... %d \r\n", __LINE__, recv_challenge_data[0], recv_challenge_data[1], recv_challenge_data[recv_challenge_data_len-1]);

            // control session payload header
            payload.header.message_id_msb = AuthenticationResponse >> 8;
            payload.header.message_id_lsb = AuthenticationResponse & 0xFF;
            total_payload_len = IAP2_CONTROL_SESSION_HEADER_SIZE;

            // start add param
            param_start = (iap2_ctrl_sess_param_t*)payload.param;
            param_start->param_id_msb = AuthenticationResponse_id_AuthenticationResponse >> 8;
            param_start->param_id_lsb = AuthenticationResponse_id_AuthenticationResponse & 0xFF;
            len_of_single_param = 2 + 2; // 2: Parameter Length 2: Parameter id
            len = apple_cert_adpt_read_challenge_response_data(recv_challenge_data, recv_challenge_data_len, param_start->data, 128);   // Todo: 128 magic number
            len_of_single_param += len;
            DEBUG_INFO(0, "MFI_AUTH line %d type = %d len = %d \r\n", __LINE__, param_id, len_of_single_param);
            param_start->param_len_msb = len_of_single_param >> 8;
            param_start->param_len_lsb = len_of_single_param & 0xFF;
            total_payload_len += len_of_single_param;

            payload.header.message_len_msb = total_payload_len >> 8;
            payload.header.message_len_lsb = (uint8_t) (total_payload_len & 0xFF);

            iap2_Link_adapter_send_control_session_data(&device->link_info, total_payload_len, (uint8_t*)&payload);
        }
    }
    break;
    case AuthenticationSucceeded:
    {
        DEBUG_INFO(0, "MFI_AUTH line %d AuthenticationSucceeded", __LINE__);
        device->state = AUTHEN_STATE_AUTHENED_SUCCESS;
    }
    break;
    case AuthenticationFailed:
    {
        DEBUG_INFO(0, "MFI_AUTH_ERROR line %d AuthenticationFailed", __LINE__);
        clear_access_authen_device(device);
    }
    break;
    default:
        break;
    }
}

void mfi_fea_accessory_authen_device_handle_event(const iap2_link_info_t* link_info, IAP2_CONN_EVENT_T event, iap2_subscriber_callback_param_t* param)
{
    DEBUG_INFO(0,  "%s event 0x%x", __func__, event);
    POSSIBLY_UNUSED accessory_authen_device_t* device = search_authen_device_by_link_info((iap2_link_info_t*)link_info);

    switch (event)
    {
    case IAP2_CONN_EVENT_OPEN:
        {
            if (device)
            {
                DEBUG_INFO(0,  "MFI_AUTH_ERROR, already has device, plz check");
                return;
            }

            device = get_free_access_authen_device();
            if (device)
            {
                device->state = AUTHEN_STATE_NONE;
                device->link_info.type = link_info->type;
                memcpy(&device->link_info.addr, &link_info->addr, sizeof(iap2_link_addr_t));
                return;
            }
            else
            {
                DEBUG_INFO(0, "MFI_AUTH_ERROR line %d device NULL", __LINE__);
                return;
            }
        }
        break;
    case IAP2_CONN_EVENT_CLOSE:
        {
            if (device)
            {
                DEBUG_INFO(0, "MFI_AUTH_ERROR line %d device NULL", __LINE__);
                return;
            }
            
            clear_access_authen_device(device);
        }
        break;
    case IAP2_CONN_EVENT_TXDONE:
        break;
    case IAP2_CONN_EVENT_RECV_CTRL_DATA:
        {
            if (device == NULL)
            {
                DEBUG_INFO(0, "MFI_AUTH_ERROR line %d device NULL", __LINE__);
                return;
            }

            mfi_fea_accessory_authen_device_handle_ctrl_session_data(device, param->recv_ctrl_data.data_len, param->recv_ctrl_data.data);
        }
        break;
    default:
        DEBUG_INFO(0, "MFI_AUTH_ERROR line %d uninterested event", __LINE__);
        break;
    }
}

uint32_t mfi_fea_accessory_authen_devcie_save_ctx(iap2_link_info_t* link_info, uint8_t *buf, uint32_t buf_len)
{
    uint32_t offset = 0;
    accessory_authen_device_t* device = search_authen_device_by_link_info(link_info);

    if (device == NULL)
    {
        buf[offset++] = false;
        DEBUG_INFO(0, "MFI_AUTH_SAVE line %d save ctx", __LINE__);
        return offset;
    }

    buf[offset++] = true;
    buf[offset++] = device->state;
    DEBUG_INFO(0, "MFI_AUTH_SAVE line %d state %d", __LINE__, device->state);
    return offset;
}

uint32_t mfi_fea_accessory_authen_devcie_restore_ctx(iap2_link_info_t* link_info, uint8_t *buf, uint32_t buf_len)
{
    uint32_t offset = 0;
    accessory_authen_device_t* device = search_authen_device_by_link_info(link_info);
    if (buf[offset++] == false)
    {
        DEBUG_INFO(0, "MFI_AUTH_RESTORE line %d no ctx", __LINE__);
        return offset;
    }

    if (device == NULL)
    {
        device = get_free_access_authen_device();
        if (device == NULL)
        {
            DEBUG_INFO(0, "MFI_AUTH_RESTORE error, no free iap2 link");
            return offset;
        }
        device->link_info.type = link_info->type;
        memcpy(&(device->link_info.addr), &(link_info->addr), sizeof(iap2_link_addr_t));
    }

    device->state = buf[offset++];
    DEBUG_INFO(0, "MFI_AUTH_RESTORE line %d state %d", __LINE__, device->state);

    return offset;
}

accessory_authen_state_t mfi_fea_accessory_authen_device_state(iap2_link_info_t* link_info)
{
   accessory_authen_device_t* device = search_authen_device_by_link_info(link_info);

    if (device == NULL)
    {
        return AUTHEN_STATE_NONE;
    }
    else
    {
        return device->state;
    }
}

void mfi_fea_accessory_authen_init(void)
{
    DEBUG_INFO(0, "MFI_AUTH_INIT line %d", __LINE__);
    for (uint8_t i = 0; i < IAP2_IAP2_CONN_NUM; i++)
    {
        memset(&authen_device[i], 0, sizeof(accessory_authen_device_t));
        authen_device[i].state = AUTHEN_STATE_NONE;
    }

    iap2_link_adapter_add_subscriber_callback(mfi_fea_accessory_authen_device_handle_event,
                                              (IAP2_CONN_EVENT_OPEN
                                              | IAP2_CONN_EVENT_CLOSE
                                              | IAP2_CONN_EVENT_TXDONE
                                              | IAP2_CONN_EVENT_RECV_CTRL_DATA));
}
