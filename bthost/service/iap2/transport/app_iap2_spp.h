#ifndef __APP_IAP2_SPP_H__
#define __APP_IAP2_SPP_H__

#include "bluetooth_bt_api.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IAP2_SPP_RECV_BUFFER_SIZE   (BT_SPP_MAX_TX_MTU*6)
#define IAP2_DEV_NUM    BT_DEVICE_NUM

#define IAP2_RFCOMM_CHANNEL RFCOMM_CHANNEL_IAP2

typedef struct {
    bt_spp_channel_t* iap2_spp_dev[IAP2_DEV_NUM];
} iap2_spp_global_t;

void app_iap2_spp_init(void);
bool app_iap2_spp_get_rfcomm_handle(bt_bdaddr_t *remote, uint16_t* rfcomm_handle);

#ifdef __cplusplus
}
#endif
#endif // __APP_IAP2_SPP_H__