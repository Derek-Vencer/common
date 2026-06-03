#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "iap2_link.h"
#include "iap2_buffer.h"
#include "iap2_packet.h"
#include "hal_timer.h"
#include "nvrecord_bt.h"
#include "apple_cert_adapter.h"
#include "bt_common_define.h"

#define IAP2_SEND_BUFFER_LEN   700 // magic number?
#define IAP2_SESSION_DATA_MAX_LEN   (1300)

// Once the transport connection is established, the accessory shall confirm the presence of a device
// supporting iAP2 by sending the following byte sequence at 1 Hz (once a second) until a response is
// received from the device
#define DAEMON_TIMER_DETETING_TIME   (1000)

#define IAP2_PACKET_HEADER_LEN              (9)
#define IAP2_SYN_PACKET_PAYLOAD_LEN         (16) // todo
#define IAP2_PACKET_PAYLOAD_CHK_LEN         (1)
#define IAP2_CONTROL_SESSION_HEADER_LEN     (6) // 2:sop 2:len 2:id
#define IAP2_EA_SSION_ID_LEN                (2)

#define true 1
#define false 0

iap2_link_global_t iap2_link_global;
iAP2Packet_t    *p_iap2_pck;
iAP2Packet_t iap2_pck;
iAP2PacketData_t iAP2PacketData;

typedef void (*process_handler)(iap2_link_t*, iap2_link_callback_param_t*);
void iap2_destroy_process_thread(void);

osThreadId iap2_link_process_pthread = NULL;

osMutexId iap2_send_mutex_id = NULL;
osMutexDef(iap2_send_mutex);

uint8_t iap2_sendData[IAP2_SEND_BUFFER_LEN];
uint8_t iap2_send_session_data[IAP2_SESSION_DATA_MAX_LEN];

void iap2_link_daemon_timer_timeout_handle(const void * param);
static bool iap2_link_send_detect(iap2_link_info_t* link);

#if IAP2_LINK_NUM > 0
osTimerDef (IAP2_LINK_DAEMON_TIMER0, iap2_link_daemon_timer_timeout_handle);
#endif // IAP2_LINK_NUM > 0

#if IAP2_LINK_NUM > 1
osTimerDef (IAP2_LINK_DAEMON_TIMER1, iap2_link_daemon_timer_timeout_handle);
#endif // IAP2_LINK_NUM > 1

#if IAP2_LINK_NUM > 2
osTimerDef (IAP2_LINK_DAEMON_TIMER2, iap2_link_daemon_timer_timeout_handle);
#endif // IAP2_LINK_NUM > 2

const uint8_t Iap2DetectData[]      = {0xFF, 0x55, 0x02, 0x00, 0xEE, 0x10};
const uint8_t Iap2Lackcompatibilitydata[] = {0xFF, 0x55, 0x0E, 0x00, 0x13, 0xFF, 0xFF, 0xFF, 0xFF,
                                             0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xEB};
const uint8_t Iap2DetectBadData0[]  = {0x55, 0x04, 0x00, 0x02, 0x04, 0xEE, 0x08};
const uint8_t Iap2DetectBadData1[]  = {0x55, 0x02, 0x00, 0x00, 0xFE};
const uint8_t Iap2DetectBadData2[]  = {0xFF, 0x55, 0x04, 0x00, 0x02, 0x04, 0xEE, 0x0};
const uint8_t Iap2DetectBadData3[]  = {0xFF, 0x55, 0x02, 0x00, 0x00, 0xFE};

iap2_detect_rsp_baddata_t detect_bad_data[] =
{
    {Iap2DetectBadData0, sizeof(Iap2DetectBadData0)},
    {Iap2DetectBadData1, sizeof(Iap2DetectBadData1)},
    {Iap2DetectBadData1, sizeof(Iap2DetectBadData1)},
    {Iap2DetectBadData3, sizeof(Iap2DetectBadData3)},
};

static void addr_little_endian_to_big_endian(uint8_t *addr)
{
    u8 mac_addr[6]={0};
    memcpy(mac_addr,addr,sizeof(mac_addr));
    for(int i=0;i<sizeof(mac_addr);i++){
        addr[i] = mac_addr[5-i];
    }
}

static POSSIBLY_UNUSED void get_big_endian_bt_addr(uint8_t *addr)
{
    char btaddr[6];
    if(nvrec_dev_get_btaddr(btaddr) == 0)
    {
        DEBUG_INFO(0, "get bt addr fail\r\n ");
        return;
    }
    memcpy(addr, btaddr, BTIF_BD_ADDR_SIZE);
    addr_little_endian_to_big_endian(addr);
    DEBUG_INFO(0, "big endian bt addr:");
    DUMP8("0x%02x ", addr, BT_ADDR_OUTPUT_PRINT_NUM);
}

void iap2_create_process_thread(void);

bool iap2_send_incompatible(iap2_link_info_t* link);
bool iap2_send_syn(iap2_link_t* link, bool retry);
bool iap2_send_ack(iap2_link_t* link, uint8_t control, uint8_t session_id);
void iap2_handle_EA_protocol(uint16_t length, uint8_t* data);
void iap2_stop_EA_protocol(uint16_t length, uint8_t* data);

iap2_link_global_t* iap2_link_get_global(void)
{
    return &iap2_link_global;
}

void iap2_link_init(void)
{
    DEBUG_INFO(0, "MFI_IAP2_LINK init");

    iap2_link_global_t* g_iap2_link = iap2_link_get_global();

    if (g_iap2_link->initialize == true)
    {
        DEBUG_INFO(0, "MFI_IAP2_LINK already init");
        return;
    }

    g_iap2_link->initialize = true;

    if (g_iap2_link->datarecv_semID == NULL)
    {
#ifdef CMSIS_OS_RTX
        g_iap2_link->datarecv_SemaphoreDef.semaphore = g_iap2_link->datarecv_sem_data;
#endif
        g_iap2_link->datarecv_semID = osSemaphoreCreate(&(g_iap2_link->datarecv_SemaphoreDef), 0);
    }

    if(iap2_send_mutex_id == NULL)
    {
        iap2_send_mutex_id = osMutexCreate((osMutex(iap2_send_mutex)));
    }

    for (int i = 0; i < IAP2_LINK_NUM; i++)
    {
        iap2_link_t* iap2_link = &g_iap2_link->link[i];

        iap2_link->control_session_id = IAP2_CONTROL_SESSION_ID;
        iap2_link->file_transfer_session_id = IAP2_FILE_TRANSFER_SESSION_ID;
        iap2_link->ea_session_id = IAP2_EA_SESSION_ID;
        iap2_link->iap2_state = IAP2_LINK_STATE_NONE;
        iap2_link->info.type = IAP2_LINK_TYPE_NONE;
        DEBUG_INFO(0, "MFI_IAP2_LINK init i %d sess_id %d %d %d", i, iap2_link->control_session_id, iap2_link->file_transfer_session_id, iap2_link->ea_session_id);
        memset(&iap2_link->info.addr, 0, sizeof(iap2_link_addr_t));
        if (i == 0)
        {
            iap2_link->daemon_timer = osTimerCreate(osTimer(IAP2_LINK_DAEMON_TIMER0), osTimerOnce, iap2_link);
        }
#if IAP2_LINK_NUM > 1
        else if (i == 1)
        {
            iap2_link->daemon_timer = osTimerCreate(osTimer(IAP2_LINK_DAEMON_TIMER1), osTimerOnce, iap2_link);
        }
#endif
#if IAP2_LINK_NUM > 2
        else if (i == 2)
        {
            iap2_link->daemon_timer = osTimerCreate(osTimer(IAP2_LINK_DAEMON_TIMER2), osTimerOnce, iap2_link);
        }
#endif
    }

    p_iap2_pck = &iap2_pck;
    p_iap2_pck->pckData = &iAP2PacketData;
    iAP2PacketResetPacket(p_iap2_pck);

    iap2_receive_buffer_init();

#ifdef BOARD_HW_GPIO_I2C_MODULE
    apple_cert_adpt_i2c_init();
#endif

    iap2_create_process_thread();
}

void bt_iap2_packet_deinit(void)
{
    DEBUG_INFO(0, "%s",__func__);
    iap2_destroy_process_thread();
}

iap2_link_state_t get_iap2_link_state(iap2_link_info_t* info)
{
    DEBUG_INFO(0, "%s ", __func__);

    iap2_link_global_t* g_iap2_link = iap2_link_get_global();
    iap2_link_t* link = NULL;

    for (int i = 0; i < IAP2_LINK_NUM; i++)
    {
        link = &(g_iap2_link->link[i]);

        if (link->info.type == info->type
            && memcmp(&(link->info.addr), &info->addr, sizeof(iap2_link_addr_t)) == 0)
        {
            return link->iap2_state;
        }
    }

    return IAP2_LINK_STATE_NONE;
}

static void set_iap2_link_state(iap2_link_info_t* info, iap2_link_state_t state)
{
    DEBUG_INFO(0,  "MFI_IAP2_LINK state:%d", state);

    iap2_link_global_t* g_iap2_link = iap2_link_get_global();
    iap2_link_t* link = NULL;

    for (int i = 0; i < IAP2_LINK_NUM; i++)
    {
        link = &(g_iap2_link->link[i]);

        if (link->info.type == info->type
            && memcmp(&(link->info.addr), &info->addr, sizeof(iap2_link_addr_t)) == 0)
        {
            link->iap2_state = state;
            return;
        }
    }

    return;
}

static iap2_link_t* search_iap2_link_by_linkinfo(iap2_link_info_t* info)
{
    iap2_link_t* link = NULL;
    iap2_link_global_t* g_iap2_link = iap2_link_get_global();

    for (int i = 0; i < IAP2_LINK_NUM; i++)
    {
        link = &(g_iap2_link->link[i]);

        if (link->info.type == info->type
            && memcmp(&(link->info.addr), &info->addr, sizeof(iap2_link_addr_t)) == 0)
        {
            return link;
        }
    }

    return NULL;
}

static iap2_link_t* get_free_iap2_link(void)
{
    iap2_link_t* link = NULL;
    iap2_link_global_t* g_iap2_link = iap2_link_get_global();

    for (int i = 0; i < IAP2_LINK_NUM; i++)
    {
        link = &(g_iap2_link->link[i]);

        if (link->iap2_state == IAP2_LINK_STATE_NONE)
        {
            memset(&link->info.addr, 0, sizeof(iap2_link_addr_t));
            return link;
        }
    }

    return NULL;
}

static void clear_iap2_link(iap2_link_t* t_link)
{
    iap2_link_t* link = NULL;
    iap2_link_global_t* g_iap2_link = iap2_link_get_global();

    for (int i = 0; i < IAP2_LINK_NUM; i++)
    {
        link = &(g_iap2_link->link[i]);

        if (link->info.type == t_link->info.type
            && memcmp(&(link->info.addr), &t_link->info.addr, sizeof(iap2_link_addr_t)) == 0)
        {
            link->info.type = IAP2_LINK_TYPE_NONE;
            memset(&link->info.addr, 0, sizeof(iap2_link_addr_t));
            link->iap2_state = IAP2_LINK_STATE_NONE;
        }
    }
}

void wake_up_recv_process(void)
{
    //DEBUG_INFO(0, "MFI_IAP2_LINK wake proc");
    iap2_link_global_t* g_iap2_link = iap2_link_get_global();

    if(g_iap2_link && g_iap2_link->datarecv_semID)
    {
        osSemaphoreRelease(g_iap2_link->datarecv_semID);
    }
}

void iap2_link_daemon_timer_timeout_handle(const void* param)
{
    return;
}

void iap2_link_stop_daemon_timer(iap2_link_t* iap2_link)
{
    if (iap2_link->daemon_timer && osTimerIsRunning(iap2_link->daemon_timer))
    {
        osTimerStop(iap2_link->daemon_timer);
        iap2_link->daemon_context = DAEMON_CONTEXT_NONE;
        return;
    }

    DEBUG_INFO(0,  "MFI_IAP2_LINK daemon_timer NULL");
    return;
}

void iap2_link_start_daemon_timer(iap2_link_t* iap2_link, uint32_t millisec, daemon_timer_context_t context)
{
    if (iap2_link && iap2_link->daemon_timer)
    {
        iap2_link_stop_daemon_timer(iap2_link);
        osTimerStart(iap2_link->daemon_timer, millisec);
        iap2_link->daemon_context = context;
        return;
    }

    DEBUG_INFO(0,  "MFI_IAP2_LINK daemon_timer NULL");
    return;
}

static void iap2_link_none_state_handler(iap2_link_t* iap2_link, iap2_link_callback_param_t* data)
{
    iap2_trans_event_t event = data->event;
    bool ret = false;

    DEBUG_INFO(0,  "MFI_IAP2_LINK_NONE event %d", event);

    switch (event)
    {
    case IAP2_TRANS_EVENT_OPEN:
        set_iap2_link_state(&iap2_link->info, IAP2_LINK_STATE_TRANSPORT_OPEN);
        ret = iap2_link_send_detect(&iap2_link->info);
        if (ret)
        {
            iap2_link_start_daemon_timer(iap2_link, DAEMON_TIMER_DETETING_TIME, DAEMON_CONTEXT_DETECTING);
            set_iap2_link_state(&iap2_link->info, IAP2_LINK_STATE_DETECTING);
        }
        else
        {
            clear_iap2_link(iap2_link);
        }
        break;
    case IAP2_TRANS_EVENT_TXDONE:
        break;
    default:
        DEBUG_INFO(0,  "MFI_IAP2_LINK_NONE, unexpect event");
        break;
    }

    return;
}

static void iap2_link_transport_open_state_handler(iap2_link_t* iap2_link, iap2_link_callback_param_t* data)
{
    DEBUG_INFO(0,  "MFI_IAP2_LINK_TRANS_OPEN %d", data->event);
    return;
}

static void iap2_link_detecting_state_handler(iap2_link_t* iap2_link, iap2_link_callback_param_t* data)
{
    iap2_trans_event_t event = data->event;
    DEBUG_INFO(0,  "MFI_IAP2_LINK_DETECTING event %d", event);

    switch (event)
    {
    case IAP2_TRANS_EVENT_OPEN:
        break;
    case IAP2_TRANS_EVENT_CLOSE:
        iap2_link_stop_daemon_timer(iap2_link);
        clear_iap2_link(iap2_link);
        break;
    case IAP2_TRANS_EVENT_RECV_DATA:
    {
        uint16_t rx_data_len = data->rx_data_len;
        uint8_t* detecting_rsp_data = data->rx_data_ptr;
        bool ret = false;

        if (rx_data_len == sizeof(Iap2DetectData)
            && memcmp(Iap2DetectData, detecting_rsp_data, sizeof(Iap2DetectData)) == 0)
        {
            set_iap2_link_state(&iap2_link->info, IAP2_LINK_STATE_INITIATED);
            // reset receivced_remote_psn and remote_last_received_insequence_psn
            iap2_link->receivced_remote_psn =0;
            iap2_link->remote_last_received_insequence_psn =0;
            ret = iap2_send_syn(iap2_link, FALSE);
            if (ret)
            {
                iap2_link_stop_daemon_timer(iap2_link);
                iap2_link_start_daemon_timer(iap2_link, DAEMON_TIMER_DETETING_TIME, DAEMON_CONTEXT_SEND_SYN);
                set_iap2_link_state(&iap2_link->info, IAP2_LINK_STATE_SEND_SYN);
            }
            else
            {
                clear_iap2_link(iap2_link);
            }
            return;
        }

        uint8_t baddata_array_size = sizeof(detect_bad_data)/sizeof(iap2_detect_rsp_baddata_t);
        iap2_detect_rsp_baddata_t bad_data;
        for (uint8_t i = 0; i < baddata_array_size; i ++)
        {
            bad_data = detect_bad_data[i];
            if (rx_data_len == bad_data.data_len
                && memcmp(bad_data.data, detecting_rsp_data, rx_data_len) == 0)
            {
                ret = iap2_send_incompatible(&iap2_link->info);
                clear_iap2_link(iap2_link);
                return;
            }
        }

        DEBUG_INFO(0,  "unknown data:");
        DUMP8("%02x ", detecting_rsp_data, rx_data_len);
        return;
    }
        break;
    case IAP2_TRANS_EVENT_TXDONE:
        break;
    default:
        DEBUG_INFO(0,  "MFI_IAP2_LINK_DETECTING unknown event %d", event);
    break;
    }

    return;
}

static void iap2_link_initiated_state_handler(iap2_link_t* iap2_link, iap2_link_callback_param_t* data)
{
    DEBUG_INFO(0,  "MFI_IAP2_LINK_INIIATED, should not receive event %d", data->event);
    return;
}

static void iap2_link_send_syn_state_handler(iap2_link_t* iap2_link, iap2_link_callback_param_t* data)
{
    iap2_trans_event_t event = data->event;
    iap2_subscriber_callback_param_t sub_param = {0};
    DEBUG_INFO(0,  "MFI_IAP2_LINK_SEND_SYN event %d", event);

    switch (event)
    {
    case IAP2_TRANS_EVENT_OPEN:
        break;
    case IAP2_TRANS_EVENT_CLOSE:
        iap2_link_stop_daemon_timer(iap2_link);
        iap2_link_adapter_report_event(&iap2_link->info, IAP2_CONN_EVENT_CLOSE, &sub_param);
        clear_iap2_link(iap2_link);
        break;
    case IAP2_TRANS_EVENT_RECV_DATA:
    {
        POSSIBLY_UNUSED uint16_t rx_data_len = data->rx_data_len;
        uint8_t* syn_rsp_data = data->rx_data_ptr;
        iAP2PacketData_t* data = (iAP2PacketData_t*)syn_rsp_data;
        if (data->sess == kIAP2PacketReservedSessionID
            && data->ctl & kIAP2PacketControlMaskSYN
            && data->ctl & kIAP2PacketControlMaskACK)
        {
            //Todo: check Checksum and param
            /* todo: If the device and accessory have proposed identical values for all negotiable connection parameters,
            the accessory shall send a final ACK packet of the most recent SYN+ACK from the device, and the
            connection is considered to be established. Otherwise, SYN+ACK packets will continue to be exchanged
            up to 10 times until all negotiable connection parameters are agreed upon. If 10 exchanges occur
            without agreement on negotiable connection parameters the device will stop responding to any further
            packets sent by the accessory. */
            bool is_identical_values = true;
            bool ret = false;
            iap2_link_stop_daemon_timer(iap2_link);

            if (is_identical_values)
            {
                // store last recevice
                iap2_link->receivced_remote_psn = data->seq;
                iap2_link->remote_last_received_insequence_psn = data->seq;
                ret = iap2_send_ack(iap2_link, kIAP2PacketControlMaskACK, 0);
                if (ret)
                {
                    set_iap2_link_state(&iap2_link->info, IAP2_LINK_STATE_CONNECT_OPEN);
                    iap2_link_adapter_report_event(&iap2_link->info, IAP2_CONN_EVENT_OPEN, &sub_param);
                }
                else
                {
                    clear_iap2_link(iap2_link);
                }
            }
            else
            {
                /* Otherwise, SYN+ACK packets will continue to be exchanged
                up to 10 times until all negotiable connection parameters are agreed upon. If 10 exchanges occur
                without agreement on negotiable connection parameters the device will stop responding to any further
                packets sent by the accessory. */
            }
        }
        else
        {
            // todo
        }
    }
        break;
    case IAP2_TRANS_EVENT_TXDONE:
        break;
    default:
        DEBUG_INFO(0,  "MFI_IAP2_LINK_SEND_SYN unknown event %d", event);
    break;
    }

    return;
}

static void iap2_link_process_control_session_data(iap2_link_t* iap2_link, uint8_t* rx_data, uint16_t data_len)
{
    uint16_t massage_len = 0;
    iap2_subscriber_callback_param_t sub_param = {0};

    if (data_len < IAP2_CONTROL_SESSION_HEADER_LEN)
    {
        DEBUG_INFO(0,  "MFI_IAP2_LINK_PROC_CTRL line %d %d", __LINE__, data_len);
        // todo 
        return;
    }

    if (rx_data[0] != 0x40 || rx_data[1] != 0x40)
    {
        DEBUG_INFO(0,  "MFI_IAP2_LINK_PROC_CTRL line %d %d %d", __LINE__, rx_data[0], rx_data[1]);
        return;
    }

    massage_len = (rx_data[2] << 8) | rx_data[3];

    if (massage_len + IAP2_CONTROL_SESSION_HEADER_LEN < data_len)
    {
        DEBUG_INFO(0,  "MFI_IAP2_LINK_PROC_CTRL line %d len error %d", __LINE__,massage_len);
        // todo 
        return;
    }

    sub_param.recv_ctrl_data.data = rx_data; // include: Start of Message field  and Message Length field
    sub_param.recv_ctrl_data.data_len = massage_len; // include: Start of Message field  and Message Length field
    iap2_link_adapter_report_event(&iap2_link->info, IAP2_CONN_EVENT_RECV_CTRL_DATA, &sub_param);
    return;
}

static void iap2_link_process_file_transfer_session_data(iap2_link_t* iap2_link, uint8_t* rx_data, uint16_t data_len)
{
    return;
}

static void iap2_link_process_ea_session_data(iap2_link_t* iap2_link, uint8_t* rx_data, uint16_t data_len)
{
    POSSIBLY_UNUSED uint16_t session_id = 0;
    iap2_subscriber_callback_param_t sub_param = {0};

    if (data_len < IAP2_EA_SSION_ID_LEN)
    {
        DEBUG_INFO(0, "MFI_IAP2_LINK_PROC_EA line %d %d", __LINE__, data_len);
        // todo
        return;
    }
    session_id =  rx_data[0] << 8 | rx_data[1];

    DEBUG_INFO(0, "MFI_IAP2_LINK_PROC_EA line %d session 0x%x len %d", __LINE__, session_id, data_len);

    sub_param.recv_ea_data.data = rx_data;      // include: protocol_session_id
    sub_param.recv_ea_data.data_len = data_len; // include: protocol_session_id
    iap2_link_adapter_report_event(&iap2_link->info, IAP2_CONN_EVENT_RECV_EA_DATA, &sub_param);
    return;
}

static void iap2_link_conn_open_state_handle_data(iap2_link_t* iap2_link, uint8_t* rx_data, uint16_t data_len)
{
    DEBUG_INFO(0,  "MFI_IAP2_LINK_CONN_OPEN data_len %d", data_len);
    iAP2PacketData_t* data = NULL;
    uint16_t packet_len = 0;
    uint16_t payload_len = 0;
    uint16_t left_data_len = data_len;

    if (left_data_len < IAP2_PACKET_HEADER_LEN)
    {
        return;
    }

    data = (iAP2PacketData_t*)rx_data;
    if (data->sess == kIAP2PacketReservedSessionID
        && IAP2_PACKET_HEADER_LEN == data_len
        && data->ctl == kIAP2PacketControlMaskRST)
    {
        DEBUG_INFO(0,  "MFI_IAP2_LINK_CONN_OPEN dont care 0"); // Todo:
        return;
    }

    // maybe one iap2 link packet has multiple control session payload
    while (1)
    {
        packet_len = (data->len_msb << 8 | data->len_lsb);
        if (packet_len > left_data_len)
        {
            DEBUG_INFO(0, "MFI_IAP2_LINK_CONN_OPEN len error %d %d", packet_len, left_data_len);
            break;
        }

        if (packet_len < IAP2_PACKET_HEADER_LEN)
        {
            DEBUG_INFO(0, "MFI_IAP2_LINK_CONN_OPEN packet_len %d", packet_len);
            break;
        }

        if (packet_len == IAP2_PACKET_HEADER_LEN)
        {
            payload_len = 0;
        }
        else
        {
            payload_len = packet_len - IAP2_PACKET_HEADER_LEN - IAP2_PACKET_PAYLOAD_CHK_LEN;
        }

        // store last recevice
        iap2_link->receivced_remote_psn = data->seq;
        if (iap2_link->receivced_remote_psn != (iap2_link->remote_last_received_insequence_psn + 1))
        {
            DEBUG_INFO(0, "MFI_IAP2_LINK_CONN_OPEN psn error %d %d", iap2_link->receivced_remote_psn, iap2_link->remote_last_received_insequence_psn);
        }
        else
        {
            iap2_link->remote_last_received_insequence_psn = data->seq;
        }

        DEBUG_INFO(0, "MFI_IAP2_LINK_CONN_OPEN sess_id 0x%x len %d 0x%x 0x%x 0x%x", data->sess, payload_len, iap2_link->control_session_id, iap2_link->file_transfer_session_id, iap2_link->ea_session_id);
        if (data->sess == iap2_link->control_session_id)
        {
            iap2_link_process_control_session_data(iap2_link, data->data, payload_len);
        }
        else if (data->sess == iap2_link->file_transfer_session_id)
        {
            iap2_link_process_file_transfer_session_data(iap2_link, data->data, payload_len);
        }
        else if (data->sess == iap2_link->ea_session_id)
        {
            iap2_link_process_ea_session_data(iap2_link, data->data, payload_len);
        }
        else
        {
            DEBUG_INFO(0, "MFI_IAP2_LINK_CONN_OPEN unknown session id %d", data->sess);
            break;
        }

        left_data_len = left_data_len - packet_len;
        if (left_data_len < IAP2_PACKET_HEADER_LEN)
        {
            DEBUG_INFO(0, "MFI_IAP2_LINK_CONN_OPEN data_process_complete %d", left_data_len);
            break;
        }

        data = (iAP2PacketData_t*)((uint8_t*)data + packet_len);
    }

    return;
}

static void iap2_link_conn_open_state_handler(iap2_link_t* iap2_link, iap2_link_callback_param_t* data)
{
    iap2_trans_event_t event = data->event;
    iap2_subscriber_callback_param_t sub_param = {0};
    DEBUG_INFO(0,  "MFI_IAP2_LINK_CONN_OPEN event %d", event);

    switch (event)
    {
    case IAP2_TRANS_EVENT_OPEN:
        break;
    case IAP2_TRANS_EVENT_CLOSE:
        iap2_link_stop_daemon_timer(iap2_link);
        iap2_link_adapter_report_event(&iap2_link->info, IAP2_CONN_EVENT_CLOSE, &sub_param);
        clear_iap2_link(iap2_link);
        break;
    case IAP2_TRANS_EVENT_RECV_DATA:
        iap2_link_conn_open_state_handle_data(iap2_link, data->rx_data_ptr, data->rx_data_len);
        break;
    case IAP2_TRANS_EVENT_TXDONE:
        break;
    default:
        DEBUG_INFO(0,  "MFI_IAP2_LINK_CONN_OPEN unknown event %d", event);
    break;
    }

    return;
}

process_handler link_handler[IAP2_LINK_STATE_MAX] = 
{
    iap2_link_none_state_handler,
    iap2_link_transport_open_state_handler,
    iap2_link_detecting_state_handler,
    iap2_link_initiated_state_handler,
    iap2_link_send_syn_state_handler,
    iap2_link_conn_open_state_handler,
};

static void iap2_link_pthread(void const *argument)
{
    if (p_iap2_pck == NULL)
    {
        DEBUG_INFO(0, "MFI_IAP2_LINK warning p_iap2_pck NULL");
        return;
    }

    DEBUG_INFO(0, "MFI_IAP2_LINK enter");

    iap2_link_global_t* g_iap2_link = iap2_link_get_global();

    if (g_iap2_link == NULL)
    {
        DEBUG_INFO(0, "MFI_IAP2_LINK warning g_iap2_link NULL");
        return;
    }

    while(TRUE)
    {
         osSemaphoreWait(g_iap2_link->datarecv_semID, osWaitForever);

        iap2_link_callback_param_t *param = {0};
        IAP2_REC_PCK *rec_pck = iap2_pull_data_from_rec_pool();
        iap2_link_t* iap2_link = NULL;
        process_handler handler = NULL;

        if (rec_pck == NULL)
        {
            DEBUG_INFO(0,  "MFI_IAP2_LINK Warning, rec_pck NULL");
            continue;
        }

        param = (iap2_link_callback_param_t *)rec_pck->pckData;

        iap2_link = search_iap2_link_by_linkinfo(&param->link_info);

        if (iap2_link == NULL)
        {
            iap2_link = get_free_iap2_link();
            if (iap2_link)
            {
                iap2_link->iap2_state = IAP2_LINK_STATE_NONE;
                memcpy(&iap2_link->info.addr, &param->link_info.addr, sizeof(iap2_link_addr_t));
                iap2_link->info.type = param->link_info.type;
            }
            else
            {
                DEBUG_INFO(0,  "MFI_IAP2_LINK, cant find free iap2_link");
                continue;
            }
        }

        if (iap2_link->iap2_state >= IAP2_LINK_STATE_MAX)
        {
            DEBUG_INFO(0,  "MFI_IAP2_LINK,iap2_state error %d", iap2_link->iap2_state);
            // TODO: free this iap2_link
            continue;
        }

        handler = link_handler[iap2_link->iap2_state];
        if (handler)
        {
            handler(iap2_link, param);
        }

        iap2_free_packet(rec_pck);
    }
    return ;
}


osThreadDef(iap2_link_pthread, osPriorityAboveNormal, 1, 4096, "iap2_link_pthread");

void iap2_create_process_thread(void)
{
    DEBUG_INFO(0, "MFI_IAP2_LINK create");

    if(iap2_link_process_pthread == NULL)
    {
        iap2_link_process_pthread = osThreadCreate(osThread(iap2_link_pthread), NULL);
    }
}

void iap2_destroy_process_thread(void)
{
    DEBUG_INFO(0, "MFI_IAP2_LINK destroy");

    if(iap2_link_process_pthread != NULL)
    {
        osThreadTerminate(iap2_link_process_pthread);
        iap2_link_process_pthread = NULL;
    }
}

bool iap2_send_data(iap2_link_info_t* link, void* pBuf, uint16_t len)
{
    return iap2_link_adapter_send_data_to_trans(link, pBuf, len);
}

static bool iap2_link_send_detect(iap2_link_info_t* link)
{
    DEBUG_INFO(0, "MFI_IAP2_LINK send detect");
    bool ret = false;

    ret = iap2_send_data(link, (void*)Iap2DetectData, (uint16_t)sizeof(Iap2DetectData));

    if(ret == false)
    {
        DEBUG_INFO(0, "MFI_IAP2_LINK send detect fail \r\n");
    }

    return ret;
}

/*
 * Format and send SYN Packet
 */
bool iap2_send_syn(iap2_link_t* link, bool retry)
{
    u8* p = iap2_sendData;
    u8* p_payload = NULL;
    u8  checkSum = 0;
    bool ret = false;
    uint8_t syn_packet_len = IAP2_PACKET_HEADER_LEN + IAP2_SYN_PACKET_PAYLOAD_LEN + IAP2_PACKET_PAYLOAD_CHK_LEN;

    // initialize send sequence number
    memset(iap2_sendData, 0, IAP2_SEND_BUFFER_LEN);
    link->initial_send_psn = (uint8_t)rand();
    DEBUG_INFO(0, "MFI_IAP2_LINK iap2_init_seq_number = 0x%02X %d %d", link->initial_send_psn, link->control_session_id, link->ea_session_id);

    UINT8_TO_BE_STREAM(p, kIAP2PacketSoPM);
    UINT8_TO_BE_STREAM(p, kIAP2PacketSoPL);
    UINT16_TO_BE_STREAM(p, syn_packet_len);
    UINT8_TO_BE_STREAM(p, kIAP2PacketControlMaskSYN); // SYN bit set 1
    UINT8_TO_BE_STREAM(p, link->initial_send_psn);// psn
    // Packet Acknowledgement Number, If ACK is not set, the Packet Acknowledgement Number shall be set to 0 by the sender
    UINT8_TO_BE_STREAM(p, 0);
    UINT8_TO_BE_STREAM(p, kIAP2PacketReservedSessionID);
    checkSum = iAP2PacketCalcChecksum(iap2_sendData, 8);
    UINT8_TO_BE_STREAM(p, checkSum);

    p_payload = p;

    UINT8_TO_BE_STREAM(p, IAP2_VERSION);                    // version 1
    UINT8_TO_BE_STREAM(p, IAP2_MAX_OUTSTANDING);
    UINT16_TO_BE_STREAM(p, IAP2_REC_MAX_DATA);           // max packet
    if (retry)
    {
        UINT16_TO_BE_STREAM(p, IAP2_RE_TX_TIMEOUT);
        UINT16_TO_BE_STREAM(p, IAP2_CUM_ACK_TIMEOUT);
        UINT8_TO_BE_STREAM(p, IAP2_MAX_RETRANSMISSIONS);
        UINT8_TO_BE_STREAM(p, IAP2_MAX_CUM_ACKS);
    }
    else // negotiate zero-acknowledgement/zero-retransmit link configuration
    {
        UINT16_TO_BE_STREAM(p, IAP2_ZERO_ACK_RE_TX_TIMEOUT);
        UINT16_TO_BE_STREAM(p, IAP2_ZERO_ACK_CUM_ACK_TIMEOUT);
        UINT8_TO_BE_STREAM(p, IAP2_ZERO_ACK_MAX_RETRANSMISSIONS);
        UINT8_TO_BE_STREAM(p, IAP2_ZERO_ACK_MAX_CUM_ACKS);
    }

    // Following is per session info.  We support 2 sessions, control and data.
    UINT8_TO_BE_STREAM(p, link->control_session_id);
    UINT8_TO_BE_STREAM(p, kIAP2PacketServiceTypeControl);
    UINT8_TO_BE_STREAM(p, IAP2_VERSION);                    // Version 1
    // Todo:Whether EA is needed shall be decided by the upper management.
    UINT8_TO_BE_STREAM(p, link->ea_session_id);
    UINT8_TO_BE_STREAM(p, kIAP2PacketServiceTypeEA);
    UINT8_TO_BE_STREAM(p, IAP2_VERSION);                    // Version 1

    checkSum = iAP2PacketCalcChecksum(p_payload, (p - p_payload));
    UINT8_TO_BE_STREAM(p, checkSum);

    ret= iap2_send_data(&link->info, (void *)iap2_sendData, syn_packet_len);

    if(ret == false)
    {
        DEBUG_INFO(0, "MFI_IAP2_LINK send fail once\r\n");
    }

    return ret;
}

/*
 * Format and send iAP2 incompatible packet
 */
bool iap2_send_incompatible(iap2_link_info_t* link)
{
    uint8_t  iap2_packet_incompatible[] = {
        0xFF, 0x55, 0x0E, 0x00, 0x13, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xEB};
    bool ret = false;

    DEBUG_INFO(0, "MFI_IAP2_LINK send incompatible");

    ret = iap2_send_data(link, (void *)iap2_packet_incompatible, sizeof(iap2_packet_incompatible));

    if(ret == false)
    {
        DEBUG_INFO(0, "MFI_IAP2_LINK send_incompatible fail \r\n");
    }

    return ret;
}

bool iap2_send_ack(iap2_link_t* link, uint8_t control, uint8_t session_id)
{
    u8* p = iap2_sendData;
    //UINT8* p_payload = NULL;
    u8  checkSum = 0;
    //int i  = 0;
    bool ret = false;

    DEBUG_INFO(0, "MFI_IAP2_LINK send ack control = 0x%02X init_psn 0x%x\r\n", control, link->initial_send_psn);

    memset(iap2_sendData,0,IAP2_SEND_BUFFER_LEN);
    UINT8_TO_BE_STREAM(p, kIAP2PacketSoPM);
    UINT8_TO_BE_STREAM(p, kIAP2PacketSoPL);
    UINT16_TO_BE_STREAM(p, kIAP2PacketHeaderLen);
    UINT8_TO_BE_STREAM(p, control);
    UINT8_TO_BE_STREAM(p, link->initial_send_psn);
    UINT8_TO_BE_STREAM(p, link->remote_last_received_insequence_psn);
    UINT8_TO_BE_STREAM(p, session_id);
    checkSum = iAP2PacketCalcChecksum(iap2_sendData, 8);
    UINT8_TO_BE_STREAM(p, checkSum);

    ret = iap2_send_data(&link->info, (void *)iap2_sendData, kIAP2PacketHeaderLen);

    if(ret == false)
    {
        DEBUG_INFO(0, "MFI_IAP2_LINK send ack fail \r\n");
    }

    return ret;
}

// payload dont include Payload Checksum
bool iap2_link_send_session_data(iap2_link_info_t* link_info, iap2_session_type_t sess_type, uint16_t payload_len, uint8_t* payload)
{
    bool ret = false;
    u8 i=0;
    uint16_t total_len = 0;
    uint8_t header_checksum = 0;
    uint8_t payload_checksum = 0;
    uint8_t* payload_start;
    iap2_link_t* iap2_link = search_iap2_link_by_linkinfo(link_info);

    DEBUG_INFO(0, "MFI_IAP2_LINK send sess data");
    if(iap2_send_mutex_id)
    {
        osMutexWait(iap2_send_mutex_id, osWaitForever);
    }
    memset(iap2_send_session_data, 0,IAP2_SEND_BUFFER_LEN);

    iap2_send_session_data[i++] = 0xFF;
    iap2_send_session_data[i++] = 0x5A;
    if (payload_len == 0)
    {
        total_len = 9;  // from Start of Packet to Header Checksum
    }
    else
    {
        total_len = 9 + payload_len + 1; // 9: from Start of Packet to Header Checksum 1: Payload Checksum
    }
    iap2_send_session_data[i++] = total_len >> 8; // len
    iap2_send_session_data[i++] = (uint8_t)(total_len & 0xff);
    iap2_send_session_data[i++] = 0x40; // control byte kIAP2PacketControlMaskACK
    iap2_send_session_data[i++] = ++iap2_link->initial_send_psn;
    iap2_send_session_data[i++] = iap2_link->remote_last_received_insequence_psn;
    if (IAP2_SESSION_TYPE_EA == sess_type)
    {
        iap2_send_session_data[i++] = iap2_link->ea_session_id;
    }
    else if (IAP2_SESSION_TYPE_CONTROL == sess_type)
    {
        iap2_send_session_data[i++] = iap2_link->control_session_id;
    }
    else
    {
        DEBUG_INFO(0, "MFI_IAP2_LINK send error type %d", sess_type);
        if(iap2_send_mutex_id)
        {
            osMutexRelease(iap2_send_mutex_id);
        }
        return ret;
    }
    header_checksum = iAP2PacketCalcChecksum(iap2_send_session_data, 8);
    iap2_send_session_data[i] = header_checksum;

    if (payload_len != 0)
    {
        payload_start = &(iap2_send_session_data[i+1]);
        memcpy(payload_start, payload, payload_len);
        payload_checksum = iAP2PacketCalcChecksum(payload_start, payload_len);
        iap2_send_session_data[total_len-1] = payload_checksum;
    }

    ret = iap2_send_data(link_info, iap2_send_session_data, total_len);
    if (ret == false)
    {
        iap2_link->initial_send_psn--;
        DEBUG_INFO(0, "MFI_IAP2_LINK send ctrl data fail");
    }

    if(iap2_send_mutex_id)
    {
        osMutexRelease(iap2_send_mutex_id);
    }
    return ret;
}

uint32_t iap2_link_save_ctx(iap2_link_info_t* link_info, uint8_t *buf, uint32_t buf_len)
{
    uint32_t offset = 0;
    iap2_link_t* iap2_link = NULL;

    iap2_link = search_iap2_link_by_linkinfo(link_info);
    if (iap2_link == NULL)
    {
        DEBUG_INFO(0, "MFI_IAP2_LINK save ctx, link not conn");
        buf[offset++] = false;  // false: no iap2 link exist
        return offset;
    }

    buf[offset++] = true; // true: link ctx exist
    buf[offset++] = iap2_link->control_session_id;
    buf[offset++] = iap2_link->file_transfer_session_id;
    buf[offset++] = iap2_link->ea_session_id;
    buf[offset++] = iap2_link->initial_send_psn;
    buf[offset++] = iap2_link->remote_last_received_insequence_psn;
    buf[offset++] = iap2_link->receivced_remote_psn;
    buf[offset++] = iap2_link->iap2_state;

    DEBUG_INFO(0, "MFI_IAP2_LINK save ctx, sess_id 0x%x 0x%x 0x%x psn 0x%x 0x%x 0x%x state %d",
            iap2_link->control_session_id,
            iap2_link->file_transfer_session_id,
            iap2_link->ea_session_id,
            iap2_link->initial_send_psn,
            iap2_link->remote_last_received_insequence_psn,
            iap2_link->receivced_remote_psn,
            iap2_link->iap2_state);
    return offset;
}

uint32_t iap2_link_restore_ctx(iap2_link_info_t* link_info, uint8_t *buf, uint32_t buf_len)
{
    uint32_t offset = 0;
    POSSIBLY_UNUSED iap2_link_t* iap2_link = NULL;

    if (buf[offset++] == false)
    {
        DEBUG_INFO(0, "MFI_IAP2_LINK restore, no ctx");
        return offset;
    }

    iap2_link = search_iap2_link_by_linkinfo(link_info);
    if (iap2_link == NULL)
    {
        iap2_link = get_free_iap2_link();
        if (iap2_link == NULL)
        {
            DEBUG_INFO(0, "MFI_IAP2_LINK restore error, no free iap2 link");
            return offset;
        }

        iap2_link->info.type = link_info->type;
        memcpy(&(iap2_link->info.addr), &(link_info->addr), sizeof(iap2_link_addr_t));
    }

    iap2_link->control_session_id = buf[offset++];
    iap2_link->file_transfer_session_id = buf[offset++];
    iap2_link->ea_session_id = buf[offset++];
    iap2_link->initial_send_psn = buf[offset++];
    iap2_link->remote_last_received_insequence_psn = buf[offset++];
    iap2_link->receivced_remote_psn = buf[offset++];
    iap2_link->iap2_state = buf[offset++];

    DEBUG_INFO(0, "MFI_IAP2_LINK restore ctx, sess_id 0x%x 0x%x 0x%x psn 0x%x 0x%x 0x%x state %d",
            iap2_link->control_session_id,
            iap2_link->file_transfer_session_id,
            iap2_link->ea_session_id,
            iap2_link->initial_send_psn,
            iap2_link->remote_last_received_insequence_psn,
            iap2_link->receivced_remote_psn,
            iap2_link->iap2_state);

    return offset;
}