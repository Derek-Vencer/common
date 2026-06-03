#include "mfi_feat_common_define.h"
#include "nvrecord_bt.h"

uint16_t iap2_ctrl_sess_add_param(iap2_ctrl_sess_param_t *p, uint16_t param_id, uint8_t *p_param, uint16_t param_len)
{
    uint8_t total_param_len = IAP2_CONTROL_SESSION_PARAMETER_HEADER_SIZE + param_len;

    p->param_len_msb = total_param_len >> 8;
    p->param_len_lsb = total_param_len & 0xFF;

    p->param_id_msb = param_id >> 8;
    p->param_id_lsb = param_id & 0xFF;

    if (p_param && param_len)
    {
        memcpy(p->data, p_param, param_len);
    }

    return total_param_len;
}

