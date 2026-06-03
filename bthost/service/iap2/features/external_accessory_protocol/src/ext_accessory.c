#include "ext_accessory.h"
#include "mfi_feat_common_define.h"
#include "bt_common_define.h"

static void mfi_fea_ext_accessory_device_clear_session(ea_device_t *ea_device);

ea_device_t ea_device[IAP2_IAP2_CONN_NUM];

#define IAP2_AMA_IDENTIFIER 0x01
const uint8_t EA_ProtocolIdentifier_AMA      = IAP2_AMA_IDENTIFIER;
const uint8_t EA_ProtocolName_AMA[] = { 'c', 'o', 'm', '.', 'a', 'm', 'a', 'z', 'o', 'n', '.', 'b', 't',  'a', 'l', 'e', 'x', 'a', 0 };
const uint8_t EA_ProtocolMatchaction_AMA = IAP2_MATCH_ACTION_DEVICE_MAY_PROMPT_AND_FIND_APP_BUTTON;

const uint8_t EA_ProtocolIdentifier_BUNDLE       = 0x03; // magic number
const uint8_t EA_ProtocolName_BUNDLE[] = {'c','o','m','.','a','m','a','z','o','n','.','e','c','h','o',0};
const uint8_t EA_ProtocolMatchaction_BUNDLE = IAP2_MATCH_ACTION_DEVICE_MAY_PROMPT_AND_FIND_APP_BUTTON;

ea_protocol_group_t ea_group_table[] =
{
    {EA_DEVICE_STATE_CLOSE, false, EA_ProtocolIdentifier_AMA,    0,  EA_ProtocolName_AMA,    sizeof(EA_ProtocolName_AMA),    EA_ProtocolMatchaction_AMA},
    {EA_DEVICE_STATE_CLOSE, false, EA_ProtocolIdentifier_BUNDLE, 0,  EA_ProtocolName_BUNDLE, sizeof(EA_ProtocolName_BUNDLE), EA_ProtocolMatchaction_BUNDLE},
};

ext_accessory_cb_t ext_access_cb = {NULL};

static ea_device_t* search_ea_device_by_link_info(iap2_link_info_t* link_info)
{
    ea_device_t* device = NULL;

    for (int i = 0; i < IAP2_IAP2_CONN_NUM; i++)
    {
        device = &(ea_device[i]);
        
        if (device->link_info.type == link_info->type
            && memcmp(&(device->link_info.addr), &link_info->addr, sizeof(iap2_link_addr_t)) == 0)
        {
            return device;
        }
    }

    return NULL;
}

static void clear_ea_device(ea_device_t* device)
{
    device->dev_state = EA_DEVICE_STATE_CLOSE;
    memset(&(device->link_info), 0, sizeof(iap2_link_info_t));
    mfi_fea_ext_accessory_device_clear_session(device);
}

static ea_device_t* get_free_ea_device(void)
{
    ea_device_t* device = NULL;

    for (int i = 0; i < IAP2_IAP2_CONN_NUM; i++)
    {
        device = &(ea_device[i]);

        if (device->dev_state == EA_DEVICE_STATE_CLOSE)
        {
            clear_ea_device(device);
            return device;
        }
    }

    return NULL;
}

static void dump_all_ea_device(void)
{
    POSSIBLY_UNUSED ea_device_t* device = NULL;
    DEBUG_INFO(0,  "MFI_EA dump_all_ea_device");
    for (int i = 0; i < IAP2_IAP2_CONN_NUM; i++)
    {
        device = &(ea_device[i]);
        DEBUG_INFO(0, "i %d state %d type %d", i, device->dev_state, device->link_info.type);
        DEBUG_INFO(0, "MFI_EA_DUMP type %d 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x",
                                    device->link_info.type,
                                     device->link_info.addr.addr.address[0],
                                     device->link_info.addr.addr.address[1],
                                     device->link_info.addr.addr.address[2],
                                     device->link_info.addr.addr.address[3],
                                     device->link_info.addr.addr.address[4],
                                     device->link_info.addr.addr.address[5]);

        for (int j = 0; j < SESSION_COUNT_MAX; j++)
        {
            DEBUG_INFO(0, "MFI_EA_DUMP j %d state %d pro_id 0x%x sess_id 0x%x", j,
                    device->pro_group[j].state,
                    device->pro_group[j].protocol_id,
                    device->pro_group[j].protocol_sess_id);
        }
    }

    return;
}

void mfi_fea_ea_device_handle_ctrl_session_data(ea_device_t* device, uint16_t data_len, uint8_t* data)
{
    iap2_ctrl_sess_payload_header_t *ctrl_data = (iap2_ctrl_sess_payload_header_t*)data;

    POSSIBLY_UNUSED uint16_t message_len = ((ctrl_data->message_len_msb << 8) | ctrl_data->message_len_lsb) - IAP2_CONTROL_SESSION_HEADER_SIZE;
    uint16_t message_id = (ctrl_data->message_id_msb << 8) | ctrl_data->message_id_lsb;
    uint8_t protocol_id = 0;
    uint16_t session_id = 0;
    iap2_ctrl_sess_param_t*  param = NULL;
    uint16_t param_len = 0;

    DEBUG_INFO(0,  "MFI_EA_RECV_CTRL message_id 0x%x message_len %d", message_id, message_len);

    switch (message_id)
    {
        case StartExternalAccessoryProtocolSession:
            param = (iap2_ctrl_sess_param_t*) ((uint8_t*)data + IAP2_CONTROL_SESSION_HEADER_SIZE);
            while (message_len > IAP2_CONTROL_SESSION_PARAMETER_HEADER_SIZE)
            {
                param_len = param->param_id_msb << 8 | param->param_len_lsb;
                protocol_id = param->data[0];
                message_len -= param_len;
                DEBUG_INFO(0,  "MFI_EA_START_EA, protocol_id 0x%x param_len %d left_mess_len %d", protocol_id, param_len, message_len);

                param = (iap2_ctrl_sess_param_t*)((uint8_t*)param + param_len);
                param_len = param->param_id_msb << 8 | param->param_len_lsb;
                session_id = param->data[0] << 8 | param->data[1];
                message_len -= param_len;
                DEBUG_INFO(0,  "MFI_EA_START_EA session_id 0x%x param_len %d left_mess_len %d", session_id, param_len, message_len);

                for (int j =0; j < SESSION_COUNT_MAX; j++)
                {
                    if (device->pro_group[j].protocol_id_valid && device->pro_group[j].protocol_id == protocol_id)
                    {
                        device->pro_group[j].state = EA_SESSION_STATE_OPEN;
                        device->pro_group[j].protocol_sess_id = session_id;
                        if (ext_access_cb.start_ea_cb)
                        {
                            ext_access_cb.start_ea_cb(&device->link_info, protocol_id, session_id);
                            DEBUG_INFO(0, "MFI_EA_START_EA j %d cus 0x%x 0x%x 0x%x", j, device->pro_group[j].protocol_id, device->pro_group[j].protocol_sess_id, device->pro_group[j].state);
                        }
                        mfi_fea_ext_accessory_send_status_ea_session(&device->link_info, session_id, SessionStatusOK);
                    }
                }

                param = (iap2_ctrl_sess_param_t*)((uint8_t*)param + param_len);
            }
        break;
        case StopExternalAccessoryProtocolSession:
            param = (iap2_ctrl_sess_param_t*) ((uint8_t*)data + IAP2_CONTROL_SESSION_HEADER_SIZE);
            while (message_len > IAP2_CONTROL_SESSION_PARAMETER_HEADER_SIZE)
            {
                param = (iap2_ctrl_sess_param_t*)((uint8_t*)param + param_len);
                param_len = param->param_id_msb << 8 | param->param_len_lsb;
                session_id = param->data[0] << 8 | param->data[1];
                message_len -= param_len;
                DEBUG_INFO(0,  "MFI_EA_START_EA session_id 0x%x param_len %d left_mess_len %d", session_id, param_len, message_len);

                for (int j =0; j < SESSION_COUNT_MAX; j++)
                {
                    if (device->pro_group[j].protocol_id_valid && device->pro_group[j].protocol_sess_id == session_id)
                    {
                        device->pro_group[j].state = EA_SESSION_STATE_CLOSE;
                        device->pro_group[j].protocol_sess_id = 0;
                        if (ext_access_cb.stop_ea_cb)
                        {
                            ext_access_cb.stop_ea_cb(&device->link_info, device->pro_group[j].protocol_id);
                        }
                        mfi_fea_ext_accessory_send_status_ea_session(&device->link_info, session_id, SessionClose);
                    }
                }

                param = (iap2_ctrl_sess_param_t*)((uint8_t*)param + param_len);
            }
        break;

        default:
        break;
    }

    return;
}

void mfi_fea_ea_device_handle_ea_session_data(ea_device_t* device, uint16_t data_len, uint8_t* data)
{
    iap2_ea_sess_payload_t* payload = (iap2_ea_sess_payload_t*)data;
    uint16 session_id = payload->sess_id_B0 << 8 |  payload->sess_id_B1;
    DEBUG_INFO(0,  "MFI_EA, recv ea session 0x%x 0x%x 0x%x",  payload->sess_id_B0,  payload->sess_id_B1, session_id);
    for (int j =0; j < SESSION_COUNT_MAX; j++)
    {
        DEBUG_INFO(0,  "MFI_EA, recv ea session j %d vaild %d sess_id %d", j, device->pro_group[j].protocol_id_valid, device->pro_group[j].protocol_sess_id);
        if (device->pro_group[j].protocol_id_valid && device->pro_group[j].protocol_sess_id == session_id)
        {
            if (ext_access_cb.ea_data_recv_cb)
            {
                ext_access_cb.ea_data_recv_cb(&device->link_info, device->pro_group[j].protocol_id, data_len - IAP2_EA_SESSION_ID_LEN, data + IAP2_EA_SESSION_ID_LEN);
            }
        }
    }
    return;
}

void mfi_fea_ea_device_handle_event(const iap2_link_info_t* link_info, IAP2_CONN_EVENT_T event, iap2_subscriber_callback_param_t* param)
{
    DEBUG_INFO(0,  "%s event 0x%d", __func__, event);
    POSSIBLY_UNUSED ea_device_t* device = search_ea_device_by_link_info((iap2_link_info_t*)link_info);

    switch (event)
    {
    case IAP2_CONN_EVENT_OPEN:
        {
            if (device)
            {
                DEBUG_INFO(0,  "MFI_EA_ERROR, already has device, plz check");
                return;
            }

            device = get_free_ea_device();
            if (device)
            {
                device->dev_state = EA_DEVICE_STATE_OPEN;
                device->link_info.type = link_info->type;
                memcpy(&device->link_info.addr, &link_info->addr, sizeof(iap2_link_addr_t));
                return;
            }
            else
            {
                DEBUG_INFO(0, "MFI_EA_ERROR line %d device NULL", __LINE__);
                return;
            }
        }
        break;
    case IAP2_CONN_EVENT_CLOSE:
        {
            if (device == NULL)
            {
                DEBUG_INFO(0, "MFI_EA_ERROR line %d device NULL", __LINE__);
                return;
            }

            for (int j =0; j < SESSION_COUNT_MAX; j++)
            {
                DEBUG_INFO(0,  "MFI_EA, recv conn close j %d vaild %d state %d", j, device->pro_group[j].protocol_id_valid, device->pro_group[j].state);
                if (device->pro_group[j].protocol_id_valid && device->pro_group[j].state == EA_SESSION_STATE_OPEN)
                {
                    if (ext_access_cb.stop_ea_cb)
                    {
                        ext_access_cb.stop_ea_cb(&device->link_info, device->pro_group[j].protocol_id);
                    }
                }
            }
            clear_ea_device(device);
        }
        break;
    case IAP2_CONN_EVENT_RECV_CTRL_DATA:
        {
            if (device == NULL)
            {
                DEBUG_INFO(0, "MFI_EA_ERROR line %d device NULL", __LINE__);
                return;
            }

            mfi_fea_ea_device_handle_ctrl_session_data(device, param->recv_ctrl_data.data_len, param->recv_ctrl_data.data);
        }
        break;
    case IAP2_CONN_EVENT_RECV_EA_DATA:
        {
            if (device == NULL)
            {
                DEBUG_INFO(0, "MFI_EA_ERROR line %d device NULL", __LINE__);
                return;
            }
            mfi_fea_ea_device_handle_ea_session_data(device, param->recv_ea_data.data_len, param->recv_ea_data.data);
        }
        break;
    default:
        break;
    }
}

void mfi_fea_ext_accessory_device_register_callback(ext_accessory_cb_t ea_cb)
{
    DEBUG_INFO(0, "MFI_EXT_ACCESSORY line %d register cb", __LINE__);
    ext_access_cb.group_cb = ea_cb.group_cb;
    ext_access_cb.start_ea_cb = ea_cb.start_ea_cb;
    ext_access_cb.stop_ea_cb = ea_cb.stop_ea_cb;
    ext_access_cb.ea_data_recv_cb = ea_cb.ea_data_recv_cb;
}

void mfi_fea_ext_accessory_device_clear_session(ea_device_t *ea_device)
{
    if (ea_device == NULL)
    {
        DEBUG_INFO(0, "MFI_EXT_ACCESSORY_INIT line %d device NULL", __LINE__);
        return;
    }

    for (int j = 0; j < SESSION_COUNT_MAX; j++)
    {
        // only clear state and protocol_sess_id
        ea_device->pro_group[j].state = EA_SESSION_STATE_CLOSE;
        ea_device->pro_group[j].protocol_sess_id = 0;
    }

    return;
}

bool mfi_fea_ext_accessory_send_data(iap2_link_info_t* link_info, uint16_t protocol_id, uint16_t len, uint8_t* buf)
{
    uint16_t protocol_sess_id = 0;
    ea_device_t* device = search_ea_device_by_link_info(link_info);
    iap2_ea_sess_payload_t ea_paylod;
    DEBUG_INFO(0, "MFI_EA_SEND_DATA protocol_id 0x%x len %d", protocol_id, len);
    DEBUG_INFO(0, "MFI_EA_SEND_DATA type %d 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x",
                                        link_info->type,
                                        link_info->addr.addr.address[0],
                                        link_info->addr.addr.address[1],
                                        link_info->addr.addr.address[2],
                                        link_info->addr.addr.address[3],
                                        link_info->addr.addr.address[4],
                                        link_info->addr.addr.address[5]);
    // search device
    if (device == NULL || device->dev_state != EA_DEVICE_STATE_OPEN)
    {
        DEBUG_INFO(0, "MFI_EA_SEND_DATA ERROR device %p", device);
        dump_all_ea_device();
        return false;
    }

    // search protocol session id
    for (int j =0; j < SESSION_COUNT_MAX; j++)
    {
        if (device->pro_group[j].protocol_id_valid == true \
            && device->pro_group[j].protocol_id == protocol_id \
            && device->pro_group[j].state == EA_SESSION_STATE_OPEN)
        {
            protocol_sess_id = device->pro_group[j].protocol_sess_id;
            goto send_ea_data;
        }
    }
    DEBUG_INFO(0, "MFI_EA_SEND_DATA line %d find no session", __LINE__);
    dump_all_ea_device();
    return false;

send_ea_data:
    memset(&ea_paylod, 0, sizeof(iap2_ea_sess_payload_t));
    ea_paylod.sess_id_B0 = protocol_sess_id >> 8;
    ea_paylod.sess_id_B1 = protocol_sess_id & 0xFF;
    memcpy(ea_paylod.data, buf, len);

    return iap2_Link_adapter_send_ea_session_data(link_info, len+2, (uint8_t*)&ea_paylod);
}

ea_protocol_group_t* mfi_fea_ext_accessory_get_pro_group_table(iap2_link_info_t* link_info)
{
    ea_device_t* ea_device = search_ea_device_by_link_info(link_info);

    if (ea_device == NULL)
    {
        DEBUG_INFO(0, "MFI_EA_ERROR line %d device NULL", __LINE__);
        return NULL;
    }

    return ea_device->pro_group;

}

uint32_t mfi_fea_ext_accessory_devcie_save_ctx(iap2_link_info_t* link_info, uint8_t *buf, uint32_t buf_len)
{
    uint32_t offset = 0;
    ea_device_t* ea_device = search_ea_device_by_link_info(link_info);

    if (ea_device == NULL)
    {
        buf[offset++] = false;
        DEBUG_INFO(0, "MFI_EA_SAVE line %d device NULL", __LINE__);
        return offset;
    }

    buf[offset++] = true;
    buf[offset++] = ea_device->dev_state;

    for (int i = 0; i < SESSION_COUNT_MAX; i++)
    {
        buf[offset++] = ea_device->pro_group[i].state;
        buf[offset++] = ea_device->pro_group[i].protocol_id_valid;
        buf[offset++] = ea_device->pro_group[i].protocol_id;
        buf[offset++] = ea_device->pro_group[i].protocol_sess_id >> 8;
        buf[offset++] = ea_device->pro_group[i].protocol_sess_id & 0xFF;
        DEBUG_INFO(0, "MFI_EA_SAVE line %d state %d pro_id 0x%x sess_id 0x%x", __LINE__,
                ea_device->pro_group[i].state,
                ea_device->pro_group[i].protocol_id,
                ea_device->pro_group[i].protocol_sess_id);
    }

    return offset;
}

uint32_t mfi_fea_ext_accessory_devcie_restore_ctx(iap2_link_info_t* link_info, uint8_t *buf, uint32_t buf_len)
{
    uint32_t offset = 0;
    ea_device_t* ea_device = NULL;

    if (buf[offset++] == false)
    {
        DEBUG_INFO(0, "MFI_EA_RESTORE line %d device NULL", __LINE__);
        return offset;
    }

    ea_device = search_ea_device_by_link_info(link_info);
    if (ea_device == NULL)
    {
        ea_device = get_free_ea_device();
        if (ea_device == NULL)
        {
            DEBUG_INFO(0, "MFI_EA_RESTORE line %d no free device", __LINE__);
            // todo:dump all ea_device
            return offset;
        }
        ea_device->link_info.type = link_info->type;
        memcpy(&(ea_device->link_info.addr), &(link_info->addr), sizeof(iap2_link_addr_t));
    }

    ea_device->dev_state = buf[offset++];
    for (int i = 0; i < SESSION_COUNT_MAX; i++)
    {
        ea_device->pro_group[i].state = buf[offset++];
        ea_device->pro_group[i].protocol_id_valid = buf[offset++];
        ea_device->pro_group[i].protocol_id = buf[offset++];
        uint8_t sess_id_b0 = buf[offset++];
        uint8_t sess_id_b1 = buf[offset++];
        ea_device->pro_group[i].protocol_sess_id = sess_id_b0 << 8 | sess_id_b1;
        DEBUG_INFO(0, "MFI_EA_RESTORE line %d state %d pro_id 0x%x sess_id 0x%x", __LINE__,
        ea_device->pro_group[i].state,
        ea_device->pro_group[i].protocol_id,
        ea_device->pro_group[i].protocol_sess_id);
        if (ea_device->pro_group[i].state == EA_SESSION_STATE_OPEN && ext_access_cb.start_ea_cb)
        {
            ext_access_cb.start_ea_cb(link_info, ea_device->pro_group[i].protocol_id, ea_device->pro_group[i].protocol_sess_id);
        }
    }

    return offset;
}

ea_session_state_t mfi_fea_ext_accessory_device_get_protocol_state(iap2_link_info_t* link_info, uint8_t protocol_id, uint16_t* protocol_sess_id)
{
    ea_device_t* ea_device = search_ea_device_by_link_info(link_info);

    if (ea_device == NULL)
    {
        return EA_SESSION_STATE_CLOSE;
    }

    for (int i = 0; i < SESSION_COUNT_MAX; i++)
    {
        if (ea_device->pro_group[i].protocol_id_valid == true
            && ea_device->pro_group[i].protocol_id == protocol_id)
            {
                *protocol_sess_id = ea_device->pro_group[i].protocol_sess_id;
                return ea_device->pro_group[i].state;
            }
    }

    return EA_SESSION_STATE_CLOSE;
}

bool mfi_fea_ext_accessory_send_status_ea_session(iap2_link_info_t* link_info, uint16_t session_id, uint8_t status)
{
    DEBUG_INFO(0, "MFI_EA_SEND_EA_STATUS protocol_id 0x%x", session_id);
    DEBUG_INFO(0, "MFI_EA_SEND_EA_STATUS type %d 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x",
                                        link_info->type,
                                        link_info->addr.addr.address[0],
                                        link_info->addr.addr.address[1],
                                        link_info->addr.addr.address[2],
                                        link_info->addr.addr.address[3],
                                        link_info->addr.addr.address[4],
                                        link_info->addr.addr.address[5]);
    iap2_ctrl_sess_payload_t payload = {.header.som_msb = 0x40, .header.som_lsb = 0x40};
    uint16_t total_ctrl_len = 0;
    uint16_t param_len = 0;
    uint8_t temp_session_id[] = {session_id >> 8,  session_id & 0xFF};
    uint8_t temp_status = status;
    bool ret = false;
    iap2_ctrl_sess_param_t* param_start = NULL;

    ea_device_t* ea_device = search_ea_device_by_link_info(link_info);

    if (ea_device == NULL)
    {
        DEBUG_INFO(0, "MFI_EA_SEND_EA_STATUS ea_device NULL");
        dump_all_ea_device();
        return false;
    }

    payload.header.message_id_msb = StatusExternalAccessoryProtocolSession >> 8;
    payload.header.message_id_lsb = StatusExternalAccessoryProtocolSession & 0xFF;
    total_ctrl_len = IAP2_CONTROL_SESSION_HEADER_SIZE;

    param_start = (iap2_ctrl_sess_param_t*)((uint8_t*)&payload + total_ctrl_len);
    param_len = iap2_ctrl_sess_add_param(param_start, StatusExternalAccessoryProtocolSession_id_ExternalAccessoryProtocolSessionIdentifier, temp_session_id, 2);
    total_ctrl_len += param_len;

    param_start = (iap2_ctrl_sess_param_t*)((uint8_t*)&payload + total_ctrl_len);
    param_len = iap2_ctrl_sess_add_param(param_start, StatusExternalAccessoryProtocolSession_id_ExternalAccessoryProtocolSessionStatus, &temp_status, 1);
    total_ctrl_len += param_len;

    payload.header.message_len_msb = total_ctrl_len >> 8;
    payload.header.message_len_lsb =  total_ctrl_len & 0xFF;

    if (total_ctrl_len > 0)
    {
        ret = iap2_Link_adapter_send_control_session_data(&ea_device->link_info, total_ctrl_len, (uint8_t*)&payload);
    }

    return ret;
}

void mfi_fea_ext_accessory_init(void)
{
    DEBUG_INFO(0, "MFI_EXT_ACCESSORY_INIT line %d ", __LINE__);
    uint8_t group_count = 0;
    ea_protocol_group_t* group = NULL;

    for (uint8_t i = 0; i < IAP2_IAP2_CONN_NUM; i++)
    {
        memset(&ea_device[i], 0, sizeof(ea_device_t));
        ea_device[i].dev_state = EA_DEVICE_STATE_CLOSE;

        mfi_fea_ext_accessory_device_clear_session(&ea_device[i]);
        DEBUG_INFO(0, "MFI_EXT_ACCESSORY_INIT line %d get_custom_group_cb %p", __LINE__, ext_access_cb.group_cb);
        if (ext_access_cb.group_cb == NULL)
        {
            // use default value of sdk
            group_count = sizeof(ea_group_table) / sizeof(ea_protocol_group_t);
            DEBUG_INFO(0, "MFI_EXT_ACCESSORY_INIT line %d group_count %d", __LINE__, group_count);
            for (int j =0; j < group_count; j++)
            {
                group = &(ea_group_table[j]);
                ea_device[i].pro_group[j].state = EA_SESSION_STATE_CLOSE;
                ea_device[i].pro_group[j].protocol_id_valid = true;
                ea_device[i].pro_group[j].protocol_id = group->protocol_id;
                ea_device[i].pro_group[j].protocol_sess_id = 0;
                ea_device[i].pro_group[j].ea_protocol_name = group->ea_protocol_name;
                ea_device[i].pro_group[j].name_len = group->name_len;
                ea_device[i].pro_group[j].match_action = group->match_action;
            }
        }
        else
        {
            ext_access_cb.group_cb(&group, &group_count);

            if (group_count > SESSION_COUNT_MAX || group_count == 0)
            {
                DEBUG_INFO(0, "MFI_EXT_ACCESSORYT line %d group len error %d", __LINE__, group_count);
                if (group_count > SESSION_COUNT_MAX)
                {
                    group_count = SESSION_COUNT_MAX;
                }
            }

            for (int j = 0; j < group_count; j++)
            {
                ea_device[i].pro_group[j].state = EA_SESSION_STATE_CLOSE;
                ea_device[i].pro_group[j].protocol_id_valid = true;
                ea_device[i].pro_group[j].protocol_id = group[j].protocol_id;
                ea_device[i].pro_group[j].protocol_sess_id = 0;
                ea_device[i].pro_group[j].ea_protocol_name = group[j].ea_protocol_name;
                ea_device[i].pro_group[j].name_len = group[j].name_len;
                ea_device[i].pro_group[j].match_action = group[j].match_action;
            }

        }
    }

    iap2_link_adapter_add_subscriber_callback(mfi_fea_ea_device_handle_event,
                                              (IAP2_CONN_EVENT_OPEN
                                              | IAP2_CONN_EVENT_CLOSE
                                              | IAP2_CONN_EVENT_TXDONE
                                              | IAP2_CONN_EVENT_RECV_CTRL_DATA
                                              | IAP2_CONN_EVENT_RECV_EA_DATA));
}