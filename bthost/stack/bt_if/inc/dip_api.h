#ifndef __DIP_API__H__
#define __DIP_API__H__

#include "bluetooth.h"
#include "sdp_api.h"
#include "dip_i.h"
#include "dip_common_define.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*DipApiCallBack)(bt_bdaddr_t *_addr, struct dip_callback_param cb_para);

void btif_dip_init(DipApiCallBack callback);
void btif_dip_clear_ctl(const bt_bdaddr_t *remote);
void btif_dip_set_state(const bt_bdaddr_t *remote, enum dip_ctrl_state state);
bool btif_dip_check_is_ios_device(const bt_bdaddr_t *remote);
bt_dip_pnp_info_t* btif_dip_get_device_info(const bt_bdaddr_t *remote);
void btif_dip_query_remote_info(bt_bdaddr_t *remote);
bool btif_dip_check_is_ios_by_vend_id(uint16_t vend_id, uint16_t vend_id_source);

#ifdef __cplusplus
}
#endif

#endif

