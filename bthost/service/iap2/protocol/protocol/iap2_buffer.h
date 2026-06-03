#ifndef __IAP2_BUFFER_H__
#define __IAP2_BUFFER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "plat_types.h"
#include "iap2_def.h"
#include "iap2_link_adapter.h"

// MAX packet size that can be sent over iAP2
#define IAP2_REC_MAX_DATA  IAP2_TRANS_BUFFER_LEN

typedef struct {
    u16 pckLength;
    u8  pckValid;
    u8   padding;
    u8  pckData[IAP2_REC_MAX_DATA];
} IAP2_REC_PCK;

int iap2_receive_buffer_init();
int iap2_push_data_to_rec_pool(uint8_t *buf, uint16_t len); 
IAP2_REC_PCK * iap2_pull_data_from_rec_pool();
void iap2_set_all_receive_packet_invalid();
void iap2_free_packet(IAP2_REC_PCK *iap2_rec_pck);

#ifdef __cplusplus
}
#endif

#endif /* __IAP2_BUFFER_H__ */