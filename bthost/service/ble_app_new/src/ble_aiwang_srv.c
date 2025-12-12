/*
 * ble_aiwang.c
 *
 *  Created on: 2025年12月11日
 *      Author: zhangyijun
 */

#include "gatt_service.h"
#include "ble_aiwang_srv.h"
#include "app_ble.h"
#include "bes_dp_api.h"
#include "bt_common_define.h"

#ifdef DEBUG_INFO
#undef DEBUG_INFO
#define DEBUG_INFO(num,...) TR_INFO(num,__VA_ARGS__)
#endif

#define BLE_SRV_PREFERRED_MTU      (512)
#define BLE_SRV_CONN_INTERVAL      (15) //ms
//CAAE4F76-6D20-4365-9DFA- DA 10 BA EF 4B CD
#define BLE_COMMUNICATE_PRIMARY_SERVICE       0xCD,0x4B,0xEF,0xBA,0x10,0xDA,0xFA,0x9D,0x65,0x43,0x20,0x6D,0x76,0x4F,0xAE,0xCA
#define BLE_COMMUNICATE_PRIMARY_SERVICE_TX    0xCD,0x4B,0xEF,0xBA,0x10,0xDA,0xFA,0x9D,0x65,0x43,0x21,0x6D,0x76,0x4F,0xAE,0xCA
#define BLE_COMMUNICATE_PRIMARY_SERVICE_RX    0xCD,0x4B,0xEF,0xBA,0x10,0xDA,0xFA,0x9D,0x65,0x43,0x22,0x6D,0x76,0x4F,0xAE,0xCA

GATT_DECL_128_LE_PRI_SERVICE(g_ble_primary_service,
		BLE_COMMUNICATE_PRIMARY_SERVICE);

GATT_DECL_128_LE_CHAR(g_ble_tx_character,
	BLE_COMMUNICATE_PRIMARY_SERVICE_TX,
    GATT_NTF_PROP,
    ATT_SEC_NONE);

GATT_DECL_CCCD_DESCRIPTOR(g_ble_tx_cccd,
    ATT_SEC_NONE);

GATT_DECL_CUDD_DESCRIPTOR(g_ble_tx_cudd,
    ATT_SEC_NONE);


GATT_DECL_128_LE_CHAR(g_ble_rx_character,
	BLE_COMMUNICATE_PRIMARY_SERVICE_RX,
    GATT_WR_REQ|GATT_WR_CMD,
    ATT_SEC_NONE);

GATT_DECL_CUDD_DESCRIPTOR(g_ble_rx_cudd,
    ATT_SEC_NONE);

static const gatt_attribute_t g_ble_attr_list[] = {
    /* Service */
    gatt_attribute(g_ble_primary_service),
    /* Characteristics */
    gatt_attribute(g_ble_tx_character),
	    gatt_attribute(g_ble_tx_cccd),
		gatt_attribute(g_ble_tx_cudd),
    /* Characteristics */
    gatt_attribute(g_ble_rx_character),
        gatt_attribute(g_ble_rx_cudd),
};

static struct ble_aiwang_server_env_tag ble_aiwang_server_env = {0};
static ble_aiwang_event_cb bw_event_callback = NULL;
static ble_aiwang_server_mtuexchanged_done_t  mtuexchanged_done_callback = NULL;
static const char tx_desc[] = "AiWang BLE_TX";
static const char rx_desc[] = "AiWang BLE_RX";

uint8_t ble_aiwang_srv_send_data_via_notification(uint8_t* data, uint32_t len)
{
    uint8_t conidx = ble_aiwang_server_env.connectionIndex;
    if (BLE_INVALID_CONNECTION_INDEX == conidx || \
            !(ble_aiwang_server_env.NtfIndCfg>>8))
    {
        return BT_STS_FAILED;
    }

    gatt_char_notify_t val_ntf=
    {
        .character = g_ble_tx_character,
        .service = g_ble_primary_service,
    };

    return gatts_send_value_notification(gap_conn_bf(gap_zero_based_conidx_to_ble_conidx(conidx)), &val_ntf, data, len);
}

uint8_t ble_aiwang_srv_send_data_via_indication(uint8_t* data, uint32_t len)
{
    uint8_t conidx = ble_aiwang_server_env.connectionIndex;
    if (BLE_INVALID_CONNECTION_INDEX == conidx || \
            !(ble_aiwang_server_env.NtfIndCfg&0xff))
    {
        return BT_STS_FAILED;
    }

    gatt_char_notify_t val_ind=
    {
        .character = g_ble_tx_character,
        .service = g_ble_primary_service,
    };

    return gatts_send_value_indication(gap_conn_bf(gap_zero_based_conidx_to_ble_conidx(conidx)), &val_ind, data, len);
}

static void ble_aiwang_srv_mtu_exchanged(uint8_t con_idx, uint16_t connhdl, uint16_t mtu)
{
    uint8_t conidx = gap_zero_based_conidx(con_idx);

    if (NULL != mtuexchanged_done_callback)
    {
        mtuexchanged_done_callback(conidx, mtu);
    }
    if (bw_event_callback)
    {
    	ble_aiwang_param_u param = {
            .event = BLE_AIWANG_SRV_MTU,
            .conidx = conidx,
            .data =    NULL,
            .len = mtu,
        };
        bw_event_callback(&param);
    }
}

static void ble_aiwang_send_desc_read_response(uint16_t connhdl, uint32_t token, bool is_tx_desc)
{
    uint8_t *buf = is_tx_desc ? (uint8_t *)tx_desc : (uint8_t *)rx_desc;
    uint16_t size = is_tx_desc ? sizeof(tx_desc) : sizeof(rx_desc);
    gatts_send_read_rsp(connhdl, token, 0, buf, size);
}

static void ble_aiwang_srv_connected(uint8_t conidx, bool notify_enabled, \
                                                bool indicate_enabled, const uint8_t *descriptor)
{
	DEBUG_INFO(0,"%s conidx=%d, notify_enabled=%d", __func__, conidx, notify_enabled);
	ble_aiwang_server_env.connectionIndex = conidx;
    if (g_ble_tx_cccd == descriptor)
    {
        if (notify_enabled) {
        	ble_aiwang_server_env.NtfIndCfg |= notify_enabled<<8;
        } else if (indicate_enabled) {
        	ble_aiwang_server_env.NtfIndCfg |= indicate_enabled;
        }
    }

    if (bw_event_callback)
    {
    	ble_aiwang_param_u param = {
            .event = BLE_AIWANG_SRV_CONN,
            .conidx = conidx,
            .data =    NULL,
            .len = notify_enabled,
        };
        bw_event_callback(&param);
    }
}

static void app_ble_aiwang_server_disconnected(uint8_t conidx, const uint8_t *descriptor)
{
    if (NULL == descriptor)
    {
        memset(&ble_aiwang_server_env, 0, sizeof(struct ble_aiwang_server_env_tag));
        ble_aiwang_server_env.connectionIndex = BLE_INVALID_CONNECTION_INDEX;
    }
    else if(g_ble_tx_cccd == descriptor)
    {
    	ble_aiwang_server_env.NtfIndCfg = 0;
    }

    if (bw_event_callback)
    {
    	ble_aiwang_param_u param = {
            .event = BLE_AIWANG_SRV_DISCONN,
            .conidx = conidx,
            .data =    NULL,
            .len = 0,
        };
        bw_event_callback(&param);
    }
}

static void app_ble_aiwang_server_tx_ccc_changed(uint8_t conidx, uint16_t config, const uint8_t *descriptor)
{
    if (config & GATT_CCCD_SET_NOTIFICATION)
    {
        ble_aiwang_srv_connected(conidx, true, false, descriptor);
    }
    else if (config & GATT_CCCD_SET_INDICATION)
    {
        ble_aiwang_srv_connected(conidx, false, true, descriptor);
    }
    else
    {
        app_ble_aiwang_server_disconnected(conidx, descriptor);
    }
}

static void ble_aiwang_srv_tx_data_sent(uint8_t conidx, const uint8_t *character)
{
    if (bw_event_callback)
    {
    	ble_aiwang_param_u param = {
            .event = BLE_AIWANG_SRV_SENDDONE,
            .conidx = conidx,
            .data =    NULL,
            .len = 0,
        };
        bw_event_callback(&param);
    }
}

static att_error_code_t ble_aiwang_srv_rx_data_received(uint8_t conidx, uint8_t *data, \
                                                                uint16_t len, const uint8_t *character)
{
    att_error_code_t status = ATT_ERROR_NO_ERROR;

    if (bw_event_callback)
    {
    	ble_aiwang_param_u param = {
            .event  = BLE_AIWANG_SRV_RX,
            .conidx = conidx,
            .data   = data,
            .len    = len,
        };
        bw_event_callback(&param);
    }

    return status;
}

static bool ble_aiwang_srv_callback(gatt_svc_t *svc, gatt_server_event_t event, gatt_server_callback_param_t param)
{
	DEBUG_INFO(0, "%s :%04x", __func__, event);
    switch (event)
    {
        case GATT_SERV_EVENT_CONN_OPENED:
            break;
        case GATT_SERV_EVENT_CONN_CLOSED:
        {
            app_ble_aiwang_server_disconnected(gap_zero_based_conidx(svc->con_idx), NULL);
            break;
        }
        case GATT_SERV_EVENT_CONN_UPDATED:
            break;
        case GATT_SERV_EVENT_MTU_CHANGED:
        {
            gatt_server_mtu_changed_t *p = param.mtu_changed;
            ble_aiwang_srv_mtu_exchanged(gap_zero_based_conidx(svc->con_idx), svc->connhdl, p->mtu);
            gap_update_params_t update_param = {0};
            update_param.conn_interval_min_1_25ms = (BLE_SRV_CONN_INTERVAL * 100) / 125;
            update_param.conn_interval_max_1_25ms = (BLE_SRV_CONN_INTERVAL * 100) / 125;
            update_param.max_peripheral_latency = 0;
            DEBUG_INFO(0, "%s :GATT_SERV_EVENT_MTU_CHANGED", __func__);
            gap_update_le_conn_parameters(svc->connhdl, &update_param);
            break;
        }
        case GATT_SERV_EVENT_CHAR_WRITE:
        {
            gatt_server_char_write_t *p = param.char_write;
            if (p->value_offset != 0 || p->value_len == 0 || p->value == NULL)
            {
            	DEBUG_INFO(0, "%s :GATT_SERV_EVENT_CHAR_WRITE error", __func__);
                return false;
            }
            p->rsp_error_code = ble_aiwang_srv_rx_data_received(gap_zero_based_conidx(svc->con_idx), \
                                                                (uint8_t *)p->value, p->value_len, p->character);
            if (p->rsp_error_code == ATT_ERROR_NO_ERROR)
            {
                gatts_send_write_rsp(p->conn->connhdl, p->token, p->rsp_error_code);
            }
            return p->rsp_error_code;
        }
        case GATT_SERV_EVENT_DESC_WRITE:
        {
            gatt_server_desc_write_t *p = param.desc_write;
            uint16_t config = CO_COMBINE_UINT16_LE(p->value);
            app_ble_aiwang_server_tx_ccc_changed(gap_zero_based_conidx(svc->con_idx), config, (uint8_t *)p->desc_attr->attr_data);
            // Here for validate async call write rsp
            bt_thread_call_func_3(gatts_send_write_rsp, bt_fixed_param(svc->connhdl),                                                                bt_fixed_param(p->token),                                                                bt_fixed_param(0));
            return true;
        }
        case GATT_SERV_EVENT_NTF_TX_DONE:
        case GATT_SERV_EVENT_INDICATE_CFM:
        {
            gatt_server_indicate_cfm_t *p = param.confirm;
            ble_aiwang_srv_tx_data_sent(gap_zero_based_conidx(svc->con_idx), p->character);
            break;
        }
        case GATT_SERV_EVENT_DESC_READ:
        {
            gatt_server_desc_read_t *p = param.desc_read;
            //uint8_t conidx = gap_zero_based_conidx(svc->con_idx);
            // Check cccd notify enable bit
            if ((uint8_t *)p->desc_attr->attr_data == g_ble_tx_cccd)
            {
                uint16_t cccd_config = co_host_to_uint16_le(ble_aiwang_server_env.NtfIndCfg);
                gatts_send_read_rsp(p->conn->connhdl, p->token, 0, (uint8_t *)&cccd_config, sizeof(cccd_config));
                return true;
            }
            bool is_tx_desc = ((uint8_t *)p->desc_attr->attr_data == g_ble_tx_cudd);
            // Here for validate async call read rsp
            bt_thread_call_func_3(ble_aiwang_send_desc_read_response,
                                                        bt_fixed_param(svc->connhdl),
                                                        bt_fixed_param(p->token),
                                                        bt_fixed_param(is_tx_desc));
            return true;
            break;
        }
        default:
        {
            break;
        }
    }
    return 0;
}

void ble_aiwang_srv_register_event_cb(ble_aiwang_event_cb callback)
{
    bw_event_callback = callback;
}

void ble_aiwang_srv_init(void)
{
    memset(&ble_aiwang_server_env, 0, sizeof(struct ble_aiwang_server_env_tag));
    ble_aiwang_server_env.connectionIndex =  BLE_INVALID_CONNECTION_INDEX;

    gatts_cfg_t gatt_svc_cfg = {0};
    gatt_svc_cfg.preferred_mtu = BLE_SRV_PREFERRED_MTU;
    gatt_svc_cfg.dont_delay_report_conn_open = true;

    gatts_register_service(g_ble_attr_list, \
                            ARRAY_SIZE(g_ble_attr_list), \
                            ble_aiwang_srv_callback, &gatt_svc_cfg);
}




