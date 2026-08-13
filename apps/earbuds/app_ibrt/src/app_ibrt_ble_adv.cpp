#ifdef __IAG_BLE_INCLUDE__

#include "bluetooth_bt_api.h"
#include "app_ibrt_ble_adv.h"
#include "hal_trace.h"
#include "me_api.h"
#include "cmsis_os.h"
#include "factory_section.h"
#include "bluetooth_ble_api.h"
#include "co_bt_defines.h"
#include "app_tws_ibrt.h"
#include "app_ibrt_debug.h"
#include "earbud_ux_api.h"
SLAVE_BLE_MODE_T slaveBleMode;
static app_ble_adv_para_data_t app_ble_adv_para_data_cfg;
#define APP_IBRT_BLE_ADV_DATA_MAX_LEN (31)
#define APP_IBRT_BLE_SCAN_RSP_DATA_MAX_LEN (31)
/* NTT color code from ble_sparrow_server.cpp */
extern "C" uint8_t ntt_color_code_nv_get(void);

/* Forward declarations */
void app_ibrt_ble_adv_para_data_init(void);
void app_ibrt_ble_set_adv_data_handler(app_ble_adv_para_data_t *adv_data_cfg);

const char *g_slave_ble_state_str[] =
{
    "BLE_STATE_IDLE",
    "BLE_ADVERTISING",
    "BLE_STARTING_ADV",
    "BLE_STOPPING_ADV",
};
    
const char *g_slave_ble_op_str[] =
{
    "BLE_OP_IDLE",
    "BLE_START_ADV",
    "BLE_STOP_ADV",
    "BLE_STOPPING_ADV",
};

#define SET_SLAVE_BLE_STATE(newState)                                                                                     \
    do                                                                                                              \
    {                                                                                                               \
        EARBUDS_TRACE(0,"[BLE][STATE]%s->%s at line %d", g_slave_ble_state_str[slaveBleMode.state], g_slave_ble_state_str[newState], __LINE__); \
        slaveBleMode.state = (newState);                                                                              \
    } while (0);

#define SET_SLAVE_BLE_OP(newOp)                                                                            \
    do                                                                                               \
    {                                                                                                \
        EARBUDS_TRACE(0,"[BLE][OP]%s->%s at line %d", g_slave_ble_op_str[slaveBleMode.op], g_slave_ble_op_str[newOp], __LINE__); \
        slaveBleMode.op = (newOp);                                                                     \
    } while (0);

static bool ntt_is_valid_bt_addr(const uint8_t *addr)
{
    static const uint8_t zero[6] = {0};
    static const uint8_t ff[6] = {0xff,0xff,0xff,0xff,0xff,0xff};

    return addr &&
           memcmp(addr, zero, 6) &&
           memcmp(addr, ff, 6);
}

static bool ntt_get_right_ear_bt_addr(uint8_t right_addr[6])
{
    uint8_t *local = app_ibrt_if_get_bt_local_address();
    uint8_t *peer  = app_ibrt_if_get_bt_peer_address();

    if (ntt_is_valid_bt_addr(local) && ((local[0] & 0x01) == 0)) {
        memcpy(right_addr, local, 6);
        return true;
    }

    if (ntt_is_valid_bt_addr(peer) && ((peer[0] & 0x01) == 0)) {
        memcpy(right_addr, peer, 6);
        return true;
    }

    return false;
}

/*****************************************************************************
 Prototype    : app_slave_ble_cmd_complete_callback
 Description  : stop ble adv
 Input        : const void *para
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History      :
 Date         : 2019/12/28
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/
void app_ibrt_slave_ble_cmd_complete_callback(uint16_t opcode, uint8_t *param, uint8_t param_len)
{
    switch (opcode)
    {
        case HCI_BLE_ADV_CMD_OPCODE:
            if(!param[0])
                app_ibrt_ble_switch_activities();
            else
            {
                EARBUDS_TRACE(0,"%s error param[0]: %d", __func__, param[0]);
                SET_SLAVE_BLE_STATE(BLE_STATE_IDLE);
                SET_SLAVE_BLE_OP(BLE_OP_IDLE);
            } 
        default:
        break;
    }
}

/*****************************************************************************
 Prototype    : app_ibrt_ble_adv_para_data_init
 Description  : initialize ble adv parameter and adv data
 Input        : None
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History      :
 Date         : 2019/12/28
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/
void app_ibrt_ble_adv_para_data_init(void)
{
    app_ble_adv_para_data_t *adv_para_cfg = &app_ble_adv_para_data_cfg;

    EARBUDS_TRACE(0, "%s", __func__);

    adv_para_cfg->adv_type = ADV_CONN_UNDIR;
    adv_para_cfg->advInterval_Ms = APP_IBRT_BLE_ADV_INTERVAL;
    adv_para_cfg->own_addr_type = BES_ADDR_PUBLIC;
    adv_para_cfg->peer_addr_type = BES_ADDR_PUBLIC;
    adv_para_cfg->adv_chanmap = ADV_ALL_CHNLS_EN;
    adv_para_cfg->adv_filter_policy = ADV_ALLOW_SCAN_ANY_CON_ANY;

    memset(adv_para_cfg->bd_addr.address, 0, BTIF_BD_ADDR_SIZE);

    /*
     * Always use the BLE name programmed in Factory Sector.
     */
    const char *ble_name_in_nv = (const char *)factory_section_get_ble_name();

    uint32_t nameLen = 0;

    if (ble_name_in_nv != NULL)
    {
        nameLen = strlen(ble_name_in_nv);
    }

    /*
     * Advertising Data:
     * Complete 128-bit Service UUID.
     */
    adv_para_cfg->adv_data_len = 0;

    const uint8_t aiWangPrimaryService[16] =
    {
        0xCD, 0x4B, 0xEF, 0xBA,
        0x10, 0xDA, 0xFA, 0x9D,
        0x65, 0x43, 0x20, 0x6D,
        0x76, 0x4F, 0xAE, 0xCA
    };

    adv_para_cfg->adv_data[adv_para_cfg->adv_data_len++] = 17;

    adv_para_cfg->adv_data[adv_para_cfg->adv_data_len++] = 0x07;

    memcpy(&adv_para_cfg->adv_data[adv_para_cfg->adv_data_len],aiWangPrimaryService,sizeof(aiWangPrimaryService));

    adv_para_cfg->adv_data_len += sizeof(aiWangPrimaryService);

    /*
     * Scan Response:
     *
     * Manufacturer Data:
     *   Length      : 1 byte
     *   AD Type     : 1 byte
     *   Company ID  : 2 bytes
     *   BT Address  : 6 bytes
     *   Product ID  : 6 bytes
     *   Color       : 1 byte
     *
     * Total = 17 bytes.
     */
    memset(adv_para_cfg->scan_rsp_data,0,sizeof(adv_para_cfg->scan_rsp_data));
    adv_para_cfg->scan_rsp_data_len = 16;
    adv_para_cfg->scan_rsp_data[0] = 0x10;
    adv_para_cfg->scan_rsp_data[1] = 0xFF;

    adv_para_cfg->scan_rsp_data[2] = 0x9B;
    adv_para_cfg->scan_rsp_data[3] = 0x0C;

    /*
     * Read original programmed BLE address first.
     */
    factory_section_original_bleaddr_get(&adv_para_cfg->scan_rsp_data[4]);

    /*
     * Keep the real BLE advertising address unchanged.
     */
    memcpy(adv_para_cfg->bd_addr.address,&adv_para_cfg->scan_rsp_data[4],BTIF_BD_ADDR_SIZE);

    /*
     * Manufacturer Data must carry the right-ear BT address.
     */
    uint8_t right_bt_addr[6] = {0};

    if (ntt_get_right_ear_bt_addr(right_bt_addr))
    {
        memcpy(
            &adv_para_cfg->scan_rsp_data[4],
            right_bt_addr,
            sizeof(right_bt_addr));

        EARBUDS_TRACE(
            0,
            "[NTT_ADV] manufacturer use right BT addr");

        DUMP8("%02x ",
              right_bt_addr,
              sizeof(right_bt_addr));
    }
    else
    {
        EARBUDS_TRACE(
            0,
            "[NTT_ADV] get right BT addr failed, keep original BLE addr");
    }

    memcpy(
        &adv_para_cfg->scan_rsp_data[10],
        "MBE003",
        6);

    adv_para_cfg->scan_rsp_data[16] =
        ntt_color_code_nv_get();

    /*
     * Manufacturer Data occupies bytes 0 ~ 16.
     */
    adv_para_cfg->scan_rsp_data_len = 17;

    EARBUDS_TRACE(
        1,
        "[COLOR_CODE][ADV] manufacturer color=0x%02X",
        adv_para_cfg->scan_rsp_data[16]);

    /*
     * Append the complete BLE local name from Factory Sector.
     *
     * For "LE nwm CLIPS":
     *
     * Manufacturer Data = 17 bytes
     * Name AD Header    =  2 bytes
     * Local Name        = 12 bytes
     * --------------------------------
     * Total             = 31 bytes
     */
    if ((ble_name_in_nv != NULL) &&
        (nameLen > 0))
    {
        uint32_t remain_len =
            sizeof(adv_para_cfg->scan_rsp_data) -
            adv_para_cfg->scan_rsp_data_len;

        if ((nameLen + 2) <= remain_len)
        {
            adv_para_cfg->scan_rsp_data[
                adv_para_cfg->scan_rsp_data_len++] =
                (uint8_t)(nameLen + 1);

            /*
             * 0x09 = Complete Local Name.
             */
            adv_para_cfg->scan_rsp_data[
                adv_para_cfg->scan_rsp_data_len++] =
                0x09;

            memcpy(
                &adv_para_cfg->scan_rsp_data[
                    adv_para_cfg->scan_rsp_data_len],
                ble_name_in_nv,
                nameLen);

            adv_para_cfg->scan_rsp_data_len +=
                nameLen;

            EARBUDS_TRACE(
                2,
                "[NTT_ADV] BLE name=%s len=%d",
                ble_name_in_nv,
                nameLen);
        }
        else
        {
            EARBUDS_TRACE(
                3,
                "[NTT_ADV] BLE name too long len=%d remain=%d",
                nameLen,
                remain_len);
        }
    }
    else
    {
        EARBUDS_TRACE(
            0,
            "[NTT_ADV] Factory BLE name invalid");
    }

    memset(&slaveBleMode,
           0,
           sizeof(slaveBleMode));
}

void ntt_ble_adv_refresh_data(void)
{
    app_ibrt_ble_adv_para_data_init();

    EARBUDS_TRACE(0,
        "[NTT_ADV] refresh color=0x%02X",
        app_ble_adv_para_data_cfg.scan_rsp_data[16]);

    if (slaveBleMode.state == BLE_ADVERTISING)
    {
        btif_me_ble_set_adv_en(false);
        app_ibrt_ble_set_adv_data_handler(&app_ble_adv_para_data_cfg);
        btif_me_ble_set_adv_en(true);
    }
    else
    {
        app_ibrt_ble_set_adv_data_handler(&app_ble_adv_para_data_cfg);
    }
}

/*****************************************************************************
 Prototype    : app_ibrt_ble_set_adv_para_handler
 Description  : config ble adv parameter
 Input        : app_ble_adv_para_data_t *adv_para_cfg
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History      :
 Date         : 2019/12/28
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/
void app_ibrt_ble_set_adv_para_handler(app_ble_adv_para_data_t *adv_para_cfg)
{
    btif_adv_para_struct_t adv_para;
    EARBUDS_TRACE(0,"%s", __func__);
    adv_para.adv_type = adv_para_cfg->adv_type;
    adv_para.interval_max = adv_para_cfg->advInterval_Ms * 8 / 5;
    adv_para.interval_min = adv_para_cfg->advInterval_Ms * 8 / 5;
    adv_para.own_addr_type = adv_para_cfg->own_addr_type;
    adv_para.peer_addr_type = adv_para_cfg->peer_addr_type;
    adv_para.adv_chanmap = adv_para_cfg->adv_chanmap;
    adv_para.adv_filter_policy = adv_para_cfg->adv_filter_policy;
    memcpy(adv_para.bd_addr.address, adv_para_cfg->bd_addr.address, BTIF_BD_ADDR_SIZE);
    
    btif_me_ble_set_adv_parameters(&adv_para);
}
/*****************************************************************************
 Prototype    : app_ibrt_ble_set_adv_data_handler
 Description  : config ble adv data and scan response data
 Input        : app_ble_adv_para_data_t *adv_data_cfg
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History      :
 Date         : 2019/12/28
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/
void app_ibrt_ble_set_adv_data_handler(app_ble_adv_para_data_t *adv_data_cfg)
{
	EARBUDS_TRACE(0,"%s", __func__);
    btif_me_ble_set_adv_data(adv_data_cfg->adv_data_len, adv_data_cfg->adv_data);
    btif_me_ble_set_scan_rsp_data(adv_data_cfg->scan_rsp_data_len, adv_data_cfg->scan_rsp_data);
}
/*****************************************************************************
 Prototype    : app_ibrt_ble_adv_start
 Description  : enable ble adv
 Input        : uint8_t adv_type, uint16_t advInterval
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History      :
 Date         : 2019/12/28
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/
void app_ibrt_ble_adv_start(uint8_t adv_type, uint16_t advInterval)
{
    EARBUDS_TRACE(0,"ble adv start with adv_type %d advIntervalms %dms", adv_type, advInterval);
    app_ibrt_ble_adv_para_data_init();
    app_ble_adv_para_data_cfg.adv_type = adv_type;
    app_ble_adv_para_data_cfg.advInterval_Ms = advInterval;

    switch(slaveBleMode.state)
    {
        case BLE_ADVERTISING:
            SET_SLAVE_BLE_STATE(BLE_STOPPING_ADV);
            SET_SLAVE_BLE_OP(BLE_START_ADV);
            btif_me_ble_set_adv_en(false);
            break;
        case BLE_STARTING_ADV:
        case BLE_STOPPING_ADV:
            SET_SLAVE_BLE_OP(BLE_START_ADV);
            break;
        case BLE_STATE_IDLE:
            SET_SLAVE_BLE_STATE(BLE_STARTING_ADV);
            app_ibrt_ble_set_adv_para_handler(&app_ble_adv_para_data_cfg);
            app_ibrt_ble_set_adv_data_handler(&app_ble_adv_para_data_cfg);
            btif_me_ble_set_adv_en(true); 
            break;
        default:
        break;
    }

}
/*****************************************************************************
 Prototype    : app_ibrt_ble_adv_stop
 Description  : disable ble adv
 Input        : None
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History      :
 Date         : 2019/12/28
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/
void app_ibrt_ble_adv_stop(void)
{
    EARBUDS_TRACE(0,"%s",__func__);
    SET_SLAVE_BLE_STATE(BLE_STOPPING_ADV);
    SET_SLAVE_BLE_OP(BLE_STOP_ADV);
    btif_me_ble_set_adv_en(false);
}
/*****************************************************************************
 Prototype    : app_ibrt_ble_switch_activities
 Description  : ble adv state machine switch
 Input        : None
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History      :
 Date         : 2019/12/28
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/
void app_ibrt_ble_switch_activities(void)
{
    switch(slaveBleMode.state)
    {
        case BLE_STARTING_ADV:
            SET_SLAVE_BLE_STATE(BLE_ADVERTISING);
            if(slaveBleMode.op == BLE_START_ADV)
            {
                SET_SLAVE_BLE_OP(BLE_OP_IDLE);
            }
            break;
       case BLE_STOPPING_ADV:
            SET_SLAVE_BLE_STATE(BLE_STATE_IDLE);
            if(slaveBleMode.op == BLE_STOP_ADV)
            {
                SET_SLAVE_BLE_OP(BLE_OP_IDLE);
            }
            break;            
       default:
        break; 
    }
    switch(slaveBleMode.op)
    {
        case BLE_START_ADV:
             app_ibrt_ble_adv_start(ADV_CONN_UNDIR, app_ble_adv_para_data_cfg.advInterval_Ms);
             break;
        default:
             break;
    }
}

/*****************************************************************************
 Prototype    : app_ibrt_ble_adv_data_config
 Description  : config ble adv data and scan response data by customer
 Input        : uint8_t *advData, uint8_t advDataLen
              : uint8_t *scanRspData, uint8_t scanRspDataLen
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History      :
 Date         : 2019/12/28
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/
void app_ibrt_ble_adv_data_config(uint8_t *advData, uint8_t advDataLen,
                                            uint8_t *scanRspData, uint8_t scanRspDataLen)
{
	 EARBUDS_TRACE(0,"%s", __func__);
    ASSERT(APP_IBRT_BLE_ADV_DATA_MAX_LEN >= advDataLen, "ble adv data len exceed");
    ASSERT(APP_IBRT_BLE_SCAN_RSP_DATA_MAX_LEN >= scanRspDataLen, "scan response data len exceed");
    memcpy(app_ble_adv_para_data_cfg.adv_data, advData, advDataLen);
    memcpy(app_ble_adv_para_data_cfg.scan_rsp_data, scanRspData, scanRspDataLen);
    app_ble_adv_para_data_cfg.adv_data_len = advDataLen;
    app_ble_adv_para_data_cfg.scan_rsp_data_len = scanRspDataLen;
}
#endif


