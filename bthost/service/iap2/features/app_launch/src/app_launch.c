#include "app_launch.h"
#include "accessory_authen.h"
#include "iap2_link_adapter.h"
#include "mfi_feat_common_define.h"
#include "bt_common_define.h"

bool mfi_fea_app_launch(iap2_link_info_t* link_info, uint8_t* app_bundle_id, uint8_t app_bundle_id_len, uint8_t AppLaunchMethod)
{
    bool ret = false;
    uint16_t total_payload_len = 0;
    uint8_t len_of_single_param = 0;
    iap2_ctrl_sess_param_t* param_start = NULL;
    DEBUG_INFO(0, "%s",__func__);

    iap2_ctrl_sess_payload_t payload = {.header.som_msb = 0x40, .header.som_lsb = 0x40};

    payload.header.message_id_msb = RequestAppLaunch >> 8;
    payload.header.message_id_lsb = RequestAppLaunch & 0xFF;
    total_payload_len = IAP2_CONTROL_SESSION_HEADER_SIZE;

    param_start = (iap2_ctrl_sess_param_t*)((uint8_t*)&payload + total_payload_len);
    len_of_single_param = iap2_ctrl_sess_add_param(param_start, RequestAppLaunch_id_AppBundleID, app_bundle_id, app_bundle_id_len);
    total_payload_len += len_of_single_param;

    param_start = (iap2_ctrl_sess_param_t*)((uint8_t*)&payload + total_payload_len);
    len_of_single_param = iap2_ctrl_sess_add_param(param_start, RequestAppLaunch_id_AppLaunchMethod, &AppLaunchMethod, 1);
    total_payload_len += len_of_single_param;

    payload.header.message_len_msb = total_payload_len >> 8;
    payload.header.message_len_lsb = total_payload_len & 0xFF;

    if (total_payload_len > 0)
    {
        ret = iap2_Link_adapter_send_control_session_data(link_info, total_payload_len, (uint8_t*)&payload);
        if (ret == false)
        {
            DEBUG_INFO(0, "MFI_APPLAUNCH error send fail \r\n ");
        }
    }
    else
    {
        DEBUG_INFO(0, "MFI_APPLAUNCH error\r\n ");
    }

    return ret;
}