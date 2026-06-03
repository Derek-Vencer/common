#include "cmsis_os.h"
#include "app_iap2_spp.h"
#include "app_iap2_spp_adapter.h"
#include "sdp_service.h"
#include "bt_common_define.h"

iap2_spp_global_t g_iap2_spp_global = {0};


/****************************************************************************
 * CUSTOM SPP SDP Entries
 ****************************************************************************/

static const U8 IOS_IAP2_SPP_128UuidClassId[] = {
    SDP_ATTRIB_HEADER_8BIT(17),        /* Data Element Sequence, 17 bytes */
    DETD_UUID + DESD_16BYTES,
    0x00, 0x00, 0x00, 0x00, 0xDE, 0xCA, 0xFA, 0xDE, 0xDE,0xCA, 0xDE, 0xAF, 0xDE, 0xCA, 0xCA, 0xFF
};

static const U8 IOS_IAP2_SPP_128UuidProtoDescList[] = {
    SDP_ATTRIB_HEADER_8BIT(12),  /* Data element sequence, 12 bytes */

    /* Each element of the list is a Protocol descriptor which is a
     * data element sequence. The first element is L2CAP which only
     * has a UUID element.
     */
    SDP_ATTRIB_HEADER_8BIT(3),   /* Data element sequence for L2CAP, 3
                                  * bytes
                                  */

    SDP_UUID_16BIT(PROT_L2CAP),  /* Uuid16 L2CAP */

    /* Next protocol descriptor in the list is RFCOMM. It contains two
     * elements which are the UUID and the channel. Ultimately this
     * channel will need to filled in with value returned by RFCOMM.
     */

    /* Data element sequence for RFCOMM, 5 bytes */
    SDP_ATTRIB_HEADER_8BIT(5),

    SDP_UUID_16BIT(PROT_RFCOMM), /* Uuid16 RFCOMM */

    /* Uint8 RFCOMM channel number - value can vary */
    SDP_UINT_8BIT(IAP2_RFCOMM_CHANNEL)
};

/*
 * * OPTIONAL * Language BaseId List (goes with the service name).
 */
static const U8 IOS_IAP2_SPP_128UuidLangBaseIdList[] = {
    SDP_ATTRIB_HEADER_8BIT(9),  /* Data Element Sequence, 9 bytes */
    SDP_UINT_16BIT(0x656E),     /* English Language */
    SDP_UINT_16BIT(0x006A),     /* UTF-8 encoding */
    SDP_UINT_16BIT(0x0100)      /* Primary language base Id */
};

/*
 * * OPTIONAL *  ServiceName
 */
static const U8 IOS_IAP2_SPP_128UuidServiceName[] = {
    SDP_TEXT_8BIT(7),          /* Null terminated text string */
    'I', 'O', 'S', 'S', 'P', 'P', '\0'
};

static bt_sdp_record_attr_t IOS_IAP2_SPP_128UuidSdpAttributes[] = { // list attr id in ascending order

    SDP_ATTRIBUTE(AID_SERVICE_CLASS_ID_LIST, IOS_IAP2_SPP_128UuidClassId),

    SDP_ATTRIBUTE(AID_PROTOCOL_DESC_LIST, IOS_IAP2_SPP_128UuidProtoDescList),

    /* Language base id (Optional: Used with service name) */
    SDP_ATTRIBUTE(AID_LANG_BASE_ID_LIST, IOS_IAP2_SPP_128UuidLangBaseIdList),

    /* SPP service name*/
    SDP_ATTRIBUTE((AID_SERVICE_NAME + 0x0100), IOS_IAP2_SPP_128UuidServiceName),
};

static iap2_spp_global_t* get_iap2_spp_global(void)
{
    return &g_iap2_spp_global;
}

bool app_iap2_spp_get_rfcomm_handle(bt_bdaddr_t *remote, uint16_t* rfcomm_handle)
{
    iap2_spp_global_t* iap2_spp_global = get_iap2_spp_global();
    bt_spp_channel_t* chnl = NULL;
    bool ret = false;

    for (int i = 0; i < IAP2_DEV_NUM; i++)
    {
        chnl = iap2_spp_global->iap2_spp_dev[i];
        if (chnl == NULL)
        {
            continue;
        }

        if (memcmp(remote, &chnl->remote, sizeof(bt_bdaddr_t)) == 0
            && chnl->spp_state == BT_SOCKET_STATE_OPENED)
        {
            *rfcomm_handle = chnl->rfcomm_handle;
            return true;
        }
    }

    return ret;
}

static int app_iap2_spp_callback(const bt_bdaddr_t *remote, bt_spp_event_t event, bt_spp_callback_param_t *param)
{
    iap2_spp_adapter_callback_param_t cb_event = {0};
    iap2_spp_global_t* iap2_spp_global = get_iap2_spp_global();

    DEBUG_INFO(0,  "app_iap2_spp_callback event 0x%x", event);

    if (/*app_bt_check_is_ios_device(remote) ==*/ false) // todo
    {
        DEBUG_INFO(0,  "remote is not ios device, plz chk");
        return 0;
    }

    if (BT_SPP_EVENT_OPENED == event)
    {
        // update spp channel
        iap2_spp_global->iap2_spp_dev[param->device_id] = param->spp_chan;
#if defined(IBT_SVC_MODULE_IBRT_ENABLEDBRT)
        if (app_ibrt_conn_is_ibrt_slave(remote) == false)
#endif
        {
            DEBUG_INFO(0,  "not slave, notify to iap2 layer");
            memcpy(&cb_event.addr, remote, sizeof(bt_bdaddr_t));
            cb_event.event = IAP2_SPP_EVENT_OPEN;
        }
        memcpy(&cb_event.addr, remote, sizeof(bt_bdaddr_t));
        cb_event.event = IAP2_SPP_EVENT_OPEN;
    }
    else if (BT_SPP_EVENT_CLOSED == event)
    {
        memcpy(&cb_event.addr, remote, sizeof(bt_bdaddr_t));
        cb_event.event = IAP2_SPP_EVENT_CLOSE;
    }
    else if (BT_SPP_EVENT_TX_DONE == event)
    {
        memcpy(&cb_event.addr, remote, sizeof(bt_bdaddr_t));
        cb_event.event = IAP2_SPP_EVENT_TXDONE;
    }
    else if (BT_SPP_EVENT_RX_DATA == event)
    {
        memcpy(&cb_event.addr, remote, sizeof(bt_bdaddr_t));
        cb_event.event = IAP2_SPP_EVENT_RECV_DATA;
        cb_event.rx_data_len =  param->rx_data_len;
        cb_event.rx_data_ptr = (uint8_t *)param->rx_data_ptr;
    }
    else
    {
        cb_event.event = IAP2_SPP_EVENT_NONE;
        DEBUG_INFO(0, "::unknown event %d\n", event);
    }

    if (cb_event.event != IAP2_SPP_EVENT_NONE)
    {
        app_iap2_spp_adapter_handle_spp_event(cb_event);
    }
    return 0;
}

static void app_iap2_get_spp_uuid_sdp_services(uint8_t *service_id, bt_sdp_record_param_t *param)
{
    bt_sdp_record_attr_t *SPP_128UuidSdpAttributes = NULL;
    uint8_t sdp_record_num = 0;

    SPP_128UuidSdpAttributes = IOS_IAP2_SPP_128UuidSdpAttributes;
    sdp_record_num = ARRAY_SIZE(IOS_IAP2_SPP_128UuidSdpAttributes);

    /* Register 128bit UUID SDP Attributes */
    *service_id = IAP2_RFCOMM_CHANNEL;

    param->attrs = SPP_128UuidSdpAttributes;
    param->attr_count = sdp_record_num;
    param->COD = BTIF_COD_MAJOR_PERIPHERAL;

    DEBUG_INFO(0, "%s serviceId %d", __func__, *service_id);
}

void app_iap2_spp_init(void)
{
    DEBUG_INFO(0, "%s",__func__);
    bt_sdp_record_param_t param;
    uint8_t service_id = 0;

    iap2_spp_global_t* iap2_spp_global = get_iap2_spp_global();

    app_iap2_get_spp_uuid_sdp_services(&service_id, &param);

    bta_spp_create_port(service_id, param.attrs, param.attr_count);

    bta_spp_set_callback(service_id, IAP2_SPP_RECV_BUFFER_SIZE, app_iap2_spp_callback, NULL);

    bta_spp_listen(service_id, true, NULL);
    
    for (int i = 0; i < IAP2_DEV_NUM; i++)
    {
        iap2_spp_global->iap2_spp_dev[i] = bta_spp_create_channel(i, service_id);
        if (iap2_spp_global->iap2_spp_dev[i])
        {
            iap2_spp_global->iap2_spp_dev[i]->device_id = i;
            DEBUG_INFO(0, "%s iap2_spp_dev %p i %d", __func__,  iap2_spp_global->iap2_spp_dev[i], i);
        }
        else
        {
             DEBUG_INFO(0, "%s iap2_spp_dev NULL i %d", __func__, i);
        }
    }
}
