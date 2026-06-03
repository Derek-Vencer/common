#ifndef __IAP2_LINK_H__
#define __IAP2_LINK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "iap2_def.h"
#include "iap2_packet.h"
#include "iap2_buffer.h"
#include "iap2_link_adapter.h"

#define UINT8_TO_BE_STREAM(p, u8)   {*(p)++ = (uint8_t)(u8);}
#define UINT16_TO_BE_STREAM(p, u16) {*(p)++ = (uint8_t)((u16) >> 8); *(p)++ = (uint8_t)(u16);}

#define IAP2_LINK_NUM   IAP2_IAP2_CONN_NUM

typedef enum
{
    DAEMON_CONTEXT_NONE,
    DAEMON_CONTEXT_DETECTING,
    DAEMON_CONTEXT_SEND_SYN,
} daemon_timer_context_t;

typedef enum {
    IAP2_LINK_STATE_NONE = 0,           // iap2 link is 
    IAP2_LINK_STATE_TRANSPORT_OPEN,     // TANSPORT OPEN, such as iap2 spp connnect
    IAP2_LINK_STATE_DETECTING,          // send detect, but haven't received a rsp yet.
    IAP2_LINK_STATE_INITIATED,          // recv the same byte sequence
    IAP2_LINK_STATE_SEND_SYN,           // send SYN, but haven't received a rsp yet.
    IAP2_LINK_STATE_CONNECT_OPEN,       //
    IAP2_LINK_STATE_MAX,
} iap2_link_state_t;

typedef struct {
    iap2_link_info_t info;
    osTimerId daemon_timer;
    daemon_timer_context_t daemon_context;
    osTimerId send_ack_timer;
    uint8_t control_session_id;
    uint8_t file_transfer_session_id;
    uint8_t ea_session_id;
    uint8_t initial_send_psn;
    uint8_t remote_last_received_insequence_psn;
    uint8_t receivced_remote_psn;
    // uint8_t receivedoutofsequencepsns[n]; // Todo

    iap2_link_state_t iap2_state;
} iap2_link_t;

typedef struct
{
    iap2_link_info_t link_info;
    iap2_trans_event_t event;
    uint16_t rx_data_len;
    uint8_t rx_data_ptr[IAP2_REC_MAX_DATA];
}__attribute__ ((packed)) iap2_link_callback_param_t;

typedef struct
{
    bool initialize;

    osSemaphoreId datarecv_semID;
    osSemaphoreDef_t datarecv_SemaphoreDef;
#ifdef CMSIS_OS_RTX
    uint32_t datarecv_sem_data[2];
#endif

    iap2_link_t link[IAP2_LINK_NUM];
}iap2_link_global_t;

typedef struct
{
    const uint8_t* data;
    uint8_t data_len;
}iap2_detect_rsp_baddata_t;

void iap2_link_init(void);
extern uint8_t get_iap2_link_state();
void wake_up_recv_process();
u16 get_ea_protocol_session_id(int proto_id);
bool iap2_link_send_session_data(iap2_link_info_t* link, iap2_session_type_t sess_type, uint16_t payload_len, uint8_t* payload);
uint32_t iap2_link_save_ctx(iap2_link_info_t* link_info, uint8_t *buf, uint32_t buf_len);
uint32_t iap2_link_restore_ctx(iap2_link_info_t* link_info, uint8_t *buf, uint32_t buf_len);

#ifdef __cplusplus
}
#endif

#endif /* __IAP2_LINK_H__ */
