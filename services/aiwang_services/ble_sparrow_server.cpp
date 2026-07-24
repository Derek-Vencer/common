/*
 * ble_sparrow_server.cpp
 *
 *  Created on: 2025年12月11日
 *      Author: zhangyijun
 */
#include "hal_trace.h"
#include "string.h"
#include "cmsis_os.h"
#include "cmsis.h"
#include "cqueue.h"
#include "bluetooth_bt_api.h"
#include "bluetooth_ble_api.h"
#include "app_bt_func.h"
#include "ble_aiwang_srv.h"

#ifdef IBRT
#include "app_ibrt_internal.h"
#include "earbud_ux_api.h"
#include "app_tws_ibrt_cmd_handler.h"
#include "bts_core_if.h"
#endif

#include "app_factory.h"
#include "factory_section.h"
#include "app_battery.h"
#include "apps.h"
#include "app_media_player.h"
#include "bt_common_define.h"

#if defined(USER_TOTA_SPP_SYNC_KEY_EN) || defined(BESUI_COMM_EN)
#include "besui_common.h"
#endif

#include "charger_with_icp1205.h"
#include "ICP1205.h"
#include "nvrecord_bt.h"
#include "nvrecord_env.h"
#include "nvrecord_extension.h"
#include "app_factory_audio.h"
#include "charger_ntc.h"

#include "app_ibrt_customif_cmd.h"
#include "app_tws_ibrt.h"
#include "bts_tws_if.h"
#include "app_ibrt_customif_cmd.h"
#include "audio_cfg.h"

#define GOC_APP_DEBUG_ENABLE 1

#undef printf
#undef TRACE

#if GOC_APP_DEBUG_ENABLE

#define printf(fmt, ...) \
    hal_trace_printf(0, "[goc-app] " fmt, ##__VA_ARGS__)

#define TRACE(attr, fmt, ...) \
    hal_trace_printf(attr, "[goc-app] " fmt, ##__VA_ARGS__)

#else

#define printf(fmt, ...) do {} while (0)
#define TRACE(attr, fmt, ...) do {} while (0)

#endif
extern void ntt_ble_adv_refresh_data(void);
extern void app_ibrt_customif_cmd_sync_battery_level(uint8_t current_level);
extern "C" uint8_t ntt_color_code_nv_get(void);
extern "C" void ntt_color_code_nv_set(uint8_t color);
extern void app_ibrt_customif_cmd_sync_color_code(uint8_t color_code);
extern bool app_spp_tota_send_data(uint8_t* ptrData, uint16_t length);

#define MAX_PACKET_SIZE             (512)
#define SPARRAW_EVENT_MAX_MAILBOX   (10)
#define SPARRAW_EVENT_BUF_SIZE      (MAX_PACKET_SIZE*SPARRAW_EVENT_MAX_MAILBOX)
#define SPARRAW_BUFF_SIZE           (4096)

#define NTT_COLOR_CODE_BLACK         0x4B
#define NTT_COLOR_CODE_SILVER_WHITE  0x53
#define NTT_COLOR_CODE_GOLD          0x4E
#define NTT_COLOR_CODE_DEFAULT       NTT_COLOR_CODE_BLACK

uint8_t g_ntt_color_code = NTT_COLOR_CODE_DEFAULT;

static bool ntt_color_code_is_valid(uint8_t color)
{
    return (color == NTT_COLOR_CODE_BLACK) ||
           (color == NTT_COLOR_CODE_SILVER_WHITE) ||
           (color == NTT_COLOR_CODE_GOLD);
}
typedef struct {
    uint8_t     devId;
    uint8_t     event;
    uint16_t    len;
    uint8_t     data[256];
} SPARRAW_MESSAGE_BLOCK;

typedef struct
{
    bool                sendDone;
    uint8_t             conidx;
    bool                notifyEnable;
    uint16_t            Mtu;
//    uint8_t             macAddress[OTA_BD_ADDR_LEN];
} SPARRAW_ENV_T;


typedef struct
{
    uint8_t     event;
    uint16_t    len;
    uint8_t     data[0];
} SPARRAW_EVENT_T;

static SPARRAW_ENV_T app_sparraw_env;
//static uint8_t sparraw_tx_buf[SPARRAW_EVENT_BUF_SIZE] = {0};
//static CQueue  sparraw_rx_cqueue;
static uint8_t ble_sparrow_task_init = false;

typedef enum {
	RX_IDLE     = 0,
	RX_CMD_TYPE,
	RX_CMD_REMAIN_COUNT_PACKETS,
	RX_CMD_PARAM_lEN,
	RX_CMD_PAYLOAD,
	RX_NEXT_SPLIT_PACKET,
}SPARRAW_RX_STATE;

static REQUEST_DATA_STRUCT   rxDataStruct;
static SPARRAW_RX_STATE sparraw_rx_state = RX_IDLE;
static uint8_t   payload_buffer[1024];
static uint16_t  rx_param_len = 0;
static uint8_t   enterKeyClickTestMode = FALSE;

static osThreadId sparrow_thread_id = NULL;
static void sparraw_event_handler_thread(const void *arg);
osThreadExDef(sparraw_event_handler_thread, osPriorityAboveNormal, 1, 1024*3, "sparraw_ble_thread", 1U);

osMutexId app_sparraw_buf_lock;
osMutexDef(app_sparraw_buf_lock);
static osMailQId sparraw_event_mailbox_id = NULL;
osMailQDef(sparraw_event_mailbox_id, SPARRAW_EVENT_MAX_MAILBOX, SPARRAW_MESSAGE_BLOCK);

int app_reset(void);
uint8_t getBoxChargerBattery(void);
uint8_t getPeerBattery(void);

static void sparraw_tx_cmd_data_rsp_ack(uint8_t cmd_type, uint8_t sub_cmd);
//#if need_send_data_by_notify
static void sparraw_tx_msg(uint8_t rsp_type, const uint8_t* data, uint16_t len);
//#endif

typedef enum
{
    SPARROW_API_TRANSPORT_BLE = 0,
    SPARROW_API_TRANSPORT_SPP = 1,
} SPARROW_API_TRANSPORT_E;

static SPARROW_API_TRANSPORT_E g_sparrow_api_transport = SPARROW_API_TRANSPORT_BLE;

/* app_spp_tota.cpp */

static void sparrow_api_set_transport(SPARROW_API_TRANSPORT_E transport)
{
    g_sparrow_api_transport = transport;
}

extern "C" void system_get_info(uint8_t *fw_rev_0, uint8_t *fw_rev_1, uint8_t *fw_rev_2, uint8_t *fw_rev_3);

void handleSetKeyMapActionAndFunc(uint8_t index,uint8_t action,uint8_t func);
void handleSetKeyMapNumber(uint8_t index);
static void keymap_init_default(void);


// #define  DISPLAY_EARBUDS_VERSION "01.01.00.03"
#define  DISPLAY_EARBUDS_VERSION   "V0.9.3.5" //"01.01.00.04"

typedef struct{
	uint8_t set_name_status;
}bleCmdSetStatus;
#define need_send_data_by_notify 1

bleCmdSetStatus bleCmdSet_status;

// 空闲定时器
static osTimerId double_hold_idle_timer_id = NULL;
// 空闲超时时间（5秒）
#define DOUBLE_HOLD_IDLE_TIMEOUT_MS 200
uint8_t button_hold_type = 0xff;

static void double_hold_idle_timeout_callback(void const *argument);

// 定时器定义
osTimerDef(DOUBLE_HOLD_IDLE_TIMER, double_hold_idle_timeout_callback);

extern bool btapp_hfp_is_call_active(void);
extern bool btapp_hfp_is_sco_active(void);
extern uint8_t btapp_hfp_get_call_active(void);

//button function
/***********************************************/
/***********************************************/
// 操作类型（高4位）
typedef enum {
    ACTION_IDLE_MUSIC = 0x00,   // 空闲/音乐播放
    ACTION_CALL       = 0x01,   // 通话
    ACTION_LEFT       = 0x20,   // 左
    ACTION_RIGHT      = 0x30    // 右
} action_type_t;

// 点击类型（低4位）
typedef enum {
    CLICK_SINGLE       = 0x00,  // 单击
    CLICK_DOUBLE       = 0x01,  // 双击
    CLICK_TRIPLE       = 0x02,  // 三击
    CLICK_DOUBLE_HOLD  = 0x03,  // 双击并按住
    CLICK_HOLD_2S      = 0x04   // 按住2秒
} click_type_t;

// 功能码
typedef enum {
    FUNC_NOT_ASSIGNED   = 0x00,
    FUNC_ACCEPT_CALL    = 0x01,
    FUNC_REJECT_CALL    = 0x02,
    FUNC_PLAY_PAUSE     = 0x03,
    FUNC_NEXT_SONG      = 0x04,
    FUNC_PREV_SONG      = 0x05,
    FUNC_VOLUME_UP      = 0x06,
    FUNC_VOLUME_DOWN    = 0x07,
    FUNC_VOICE_ASSIST   = 0x08
} function_t;

// 单个映射条目
typedef struct {
    uint8_t actions;    // 高4位操作类型 + 低4位点击类型
    uint8_t function;   // 对应的功能
} key_map_entry_t;
// 整个映射块头部（偏移1~3）
typedef struct {
    uint16_t length;    // 总长度 = 2 * key_count + 1
    uint8_t  key_count; // 映射条目数量
    // 后面紧跟 key_count 个 key_map_entry_t
} key_map_header_t;

/* 功能函数类型：无参数、无返回值（可根据需要扩展） */
typedef void (*function_handler_t)(void);

/*==============================================================================
 * 2. 保存映射值的全局表（可修改，支持运行时保存/更新）
 *----------------------------------------------------------------------------*/
#define MAX_KEY_MAP_ENTRIES  21
static key_map_entry_t s_key_map[MAX_KEY_MAP_ENTRIES];
static uint8_t s_key_map_count = 0;

// 1003 1104 1205 1307 1408 2003 2104 2205 2306 2408 
// 5001 5102 5200 5300 5400 6001 6102 6200 6300 6400

const key_map_entry_t s_key_default_map[21]{
	{0x10,0x03},{0x11,0x04},{0x12,0x05},{0x13,0x07},{0x14,0x08},{0x20,0x03},{0x21,0x04},{0x22,0x05},{0x23,0x06},{0x24,0x08},
	{0x50,0x01},{0x51,0x02},{0x52,0x00},{0x53,0x07},{0x54,0x00},{0x60,0x01},{0x61,0x02},{0x62,0x00},{0x63,0x06},{0x64,0x00},
};

uint8_t key_event_is_left = 0;
/***********************************************/

uint8_t er_inbox = 0;

typedef enum {
    API_ERR_INVALID_PARAM = 0x01,
    API_ERR_NOT_SUPPORTED = 0x02,
    API_ERR_BUSY          = 0x03,
    API_ERR_UNAUTHORIZED  = 0x04,
    API_ERR_BATTERY_LOW   = 0x05,
    API_ERR_STORAGE_ERROR = 0x06,
    API_ERR_TIMEOUT       = 0x07,
    API_ERR_TOO_LONG      = 0x08,
} NTT_API_ERROR_CODE_T;

#define ERR_GET_BATTERY_LEVEL  0x33
#define ERR_GET_DEVICE_NAME    0x37
#define ERR_SET_DEVICE_NAME    0x3B
#define ERR_GET_KEY_MAPPING    0x3F
#define ERR_SET_KEY_MAPPING    0x43
#define ERR_GET_EQ_PRESET      0x47
#define ERR_SET_EQ_PRESET      0x4B
#define ERR_GET_FW_VERSION     0x4F

static const char *ntt_api_error_string(uint8_t err)
{
    switch (err) {
        case API_ERR_INVALID_PARAM: return "Invalid param";
        case API_ERR_NOT_SUPPORTED: return "Not supported";
        case API_ERR_BUSY:          return "Busy";
        case API_ERR_UNAUTHORIZED:  return "Unauthorized";
        case API_ERR_BATTERY_LOW:   return "Battery low";
        case API_ERR_STORAGE_ERROR: return "Storage error";
        case API_ERR_TIMEOUT:       return "Timeout";
        case API_ERR_TOO_LONG:      return "Too long param";
        default:                    return "Unknown error";
    }
}

static void ntt_api_send_error_notify(uint8_t rsp_cmd, uint8_t err)
{
    uint8_t buf[64] = {0};
    const char *detail = ntt_api_error_string(err);
    uint16_t detail_len = strlen(detail);
    uint16_t value_len = detail_len + 1;

    buf[0] = (value_len >> 8) & 0xFF;
    buf[1] = value_len & 0xFF;
    buf[2] = err;
    memcpy(&buf[3], detail, detail_len);

    TRACE(0, "[API_ERR][NOTIFY] rsp=0x%02X err=0x%02X detail=%s", rsp_cmd, err, detail);
    sparraw_tx_msg(rsp_cmd, buf, detail_len + 3);
}

void handleGetBatteryLevel(const uint8_t *data, uint16_t len)
{
    uint8_t localBattery;
    uint8_t peerBattery;
    uint8_t boxBattery;
    bool peerValid = false;
    bool twsConnected = false;

    uint8_t leftBattery  = 0;
    uint8_t rightBattery = 0;
    uint8_t batteryArray[3] = {0};

    if ((data == NULL) || (len == 0))
    {
        TRACE(0, "[BAT][REQ] invalid");
        ntt_api_send_error_notify(0x33, API_ERR_INVALID_PARAM);
        return;
    }

    localBattery = app_battery_current_level();
    peerBattery  = app_ibrt_customif_get_tws_peer_battery_level();
    boxBattery   = getBoxChargerBattery();

    /* 電池尚未更新完成 */
    if ((localBattery > 100) || (boxBattery > 100))
    {
        ntt_api_send_error_notify(0x33, API_ERR_BUSY);
        return;
    }

    twsConnected = bts_tws_if_is_tws_link_connected();

    if ((peerBattery != 0xFF) && (peerBattery <= 100))
    {
        peerValid = true;
    }
    else
    {
        peerBattery = 0;
        peerValid = false;
    }

#ifdef IBRT
    app_ibrt_customif_cmd_sync_battery_level(localBattery);

    if (app_ibrt_if_is_right_side())
    {
        rightBattery = localBattery;
        leftBattery  = peerValid ? peerBattery : 0;
    }
    else
    {
        leftBattery  = localBattery;
        rightBattery = peerValid ? peerBattery : 0;
    }
#else
    leftBattery  = localBattery;
    rightBattery = peerValid ? peerBattery : 0;
#endif
	TRACE(0,
      "[BAT][PEER] tws=%d peer=%d valid=%d",
      twsConnected,
      peerBattery,
      peerValid);
      
    batteryArray[0] = leftBattery;
    batteryArray[1] = rightBattery;
    batteryArray[2] = boxBattery;

    sparraw_tx_msg(0x31, batteryArray, sizeof(batteryArray));
}

#define NTT_BT_NAME_MAX_LEN             45
#define NTT_BT_NAME_DELAY_WRITE_MS          10000
#define NTT_BT_NAME_RETRY_WHEN_BUSY_MS      3000
#define NTT_BT_NAME_MAX_RETRY_COUNT         20

static uint8_t ntt_bt_name_sync_buf[NTT_BT_NAME_MAX_LEN + 1] = {0};
static uint16_t ntt_bt_name_sync_len = 0;
static bool ntt_bt_name_pending = false;
static osTimerId ntt_bt_name_write_timer = NULL;

static void ntt_bt_name_write_timer_handler(void const *param);

osTimerDef(NTT_BT_NAME_WRITE_TIMER, ntt_bt_name_write_timer_handler);

static uint8_t ntt_bt_name_retry_count = 0;

void handleGetDeviceName(const uint8_t *data, uint16_t len)
{
    TRACE(0, "%s.", __func__);

    const uint8_t *localname = NULL;
    uint16_t name_len = 0;

    if ((data == NULL) || (len == 0))
    {
        ntt_api_send_error_notify(0x37, API_ERR_INVALID_PARAM);
        return;
    }

    /*
     * If device name was just set but not written to flash yet,
     * return the RAM pending name first.
     */
    if (ntt_bt_name_pending &&
        (ntt_bt_name_sync_len > 1) &&
        (ntt_bt_name_sync_len <= sizeof(ntt_bt_name_sync_buf)) &&
        (ntt_bt_name_sync_buf[0] != 0))
    {
        localname = ntt_bt_name_sync_buf;
        name_len = ntt_bt_name_sync_len;

        TRACE(1, "[GET_NAME] return pending RAM name: %s", localname);
    }
    else
    {
        localname = factory_section_get_bt_name();

        if ((localname == NULL) || (strlen((char *)localname) == 0))
        {
            ntt_api_send_error_notify(0x37, API_ERR_STORAGE_ERROR);
            return;
        }

        name_len = strlen((char *)localname) + 1;

        TRACE(1, "[GET_NAME] return flash name: %s", localname);
    }

#if need_send_data_by_notify
    sparraw_tx_msg(RSP_GET_DEVICE_NAME,
                   localname,
                   name_len);
#endif
}

static bool ntt_bt_name_is_audio_busy(void)
{
    uint8_t device_id = 0;

    for (device_id = 0; device_id < BT_DEVICE_NUM; device_id++)
    {
        struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);

        if (curr_device == NULL)
        {
            continue;
        }

        if (curr_device->a2dp_streamming)
        {
            TRACE(1, "[SET_NAME] busy: a2dp streaming device_id=%d", device_id);
            return true;
        }
    }

    if (btapp_hfp_is_call_active())
    {
        TRACE(0, "[SET_NAME] busy: hfp call active");
        return true;
    }

    if (btapp_hfp_is_sco_active())
    {
        TRACE(0, "[SET_NAME] busy: hfp sco active");
        return true;
    }

    if (btapp_hfp_get_call_active())
    {
        TRACE(0, "[SET_NAME] busy: hfp call setup/alert");
        return true;
    }

    return false;
}

static void ntt_bt_name_delay_write_start(void)
{
    if (ntt_bt_name_write_timer == NULL)
    {
        ntt_bt_name_write_timer =
            osTimerCreate(osTimer(NTT_BT_NAME_WRITE_TIMER),
                          osTimerOnce,
                          NULL);
    }

    if (ntt_bt_name_write_timer)
    {
        osTimerStop(ntt_bt_name_write_timer);
        osTimerStart(ntt_bt_name_write_timer, NTT_BT_NAME_DELAY_WRITE_MS);
    }
}

static void ntt_bt_name_write_timer_handler(void const *param)
{
    if (!ntt_bt_name_pending)
    {
        return;
    }

    if ((ntt_bt_name_sync_len == 0) ||
        (ntt_bt_name_sync_len > sizeof(ntt_bt_name_sync_buf)))
    {
        TRACE(1, "[SET_NAME] invalid pending len=%d", ntt_bt_name_sync_len);
        ntt_bt_name_pending = false;
        return;
    }

    if (ntt_bt_name_is_audio_busy())
    {
        TRACE(0, "[SET_NAME] audio busy, delay factory write");

        osTimerStop(ntt_bt_name_write_timer);
        osTimerStart(ntt_bt_name_write_timer, 3000);
        return;
    }

    TRACE(2,
          "[SET_NAME] delayed factory write name=%s len=%d",
          ntt_bt_name_sync_buf,
          ntt_bt_name_sync_len);

    if (factory_section_set_bt_name((char *)ntt_bt_name_sync_buf,
                                    ntt_bt_name_sync_len))
    {
        TRACE(0, "[SET_NAME] delayed factory_section_set_bt_name failed");

        osTimerStop(ntt_bt_name_write_timer);
        osTimerStart(ntt_bt_name_write_timer, NTT_BT_NAME_DELAY_WRITE_MS);
        return;
    }

    TRACE(0, "[SET_NAME] delayed factory write done");

    app_ibrt_customif_cmd_sync_bt_name(ntt_bt_name_sync_buf,
                                       ntt_bt_name_sync_len);

    ntt_bt_name_pending = false;
}

void handleSetDeviceName(const uint8_t *data, uint16_t len)
{
    TRACE(0, "%s.", __func__);

    char nameBuffer[NTT_BT_NAME_MAX_LEN + 1] = {0};

    if ((data == NULL) || (len < 4))
    {
        bleCmdSet_status.set_name_status = 0x3B;
        ntt_api_send_error_notify(0x3B, API_ERR_INVALID_PARAM);
        return;
    }

    uint16_t name_len = ((uint16_t)data[1] << 8) | data[2];

    if (name_len == 0)
    {
        bleCmdSet_status.set_name_status = 0x3B;
        ntt_api_send_error_notify(0x3B, API_ERR_INVALID_PARAM);
        return;
    }

    if (name_len > NTT_BT_NAME_MAX_LEN)
    {
        bleCmdSet_status.set_name_status = 0x3B;
        ntt_api_send_error_notify(0x3B, API_ERR_TOO_LONG);
        return;
    }

    if ((name_len + 3) != len)
    {
        bleCmdSet_status.set_name_status = 0x3B;
        ntt_api_send_error_notify(0x3B, API_ERR_INVALID_PARAM);
        return;
    }

    memcpy(nameBuffer, &data[3], name_len);
    nameBuffer[name_len] = 0;

    TRACE(1, "[SET_NAME] New Name = %s", nameBuffer);

    /*
     * Do NOT call factory_section_set_bt_name() here.
     * BLE command handler / music playback path may reboot.
     */

    memset(ntt_bt_name_sync_buf, 0, sizeof(ntt_bt_name_sync_buf));
    memcpy(ntt_bt_name_sync_buf, nameBuffer, name_len + 1);

    ntt_bt_name_sync_len = name_len + 1;
    ntt_bt_name_pending = true;
    ntt_bt_name_retry_count = 0;
    ntt_bt_name_delay_write_start();

    bleCmdSet_status.set_name_status = 0x3A;

#if need_send_data_by_notify
    sparraw_tx_msg(RSP_SET_DEVICE_NAME, (const uint8_t *)"", 0);
#endif
}

extern void handleGetKeyMapNumber(uint8_t *index);
extern void handleGetKeyMapActionAndFunc(uint8_t index, uint8_t *action, uint8_t *func);
void handleGetKeyMapping(const uint8_t *data, uint16_t len)
{
    TRACE(0, "%s.", __func__);

    if ((data == NULL) || (len == 0))
    {
        ntt_api_send_error_notify(0x3F, API_ERR_INVALID_PARAM);
        return;
    }

    struct nvrecord_env_t *nvrecord_env = NULL;
    nv_record_env_get(&nvrecord_env);

    if (nvrecord_env == NULL)
    {
        ntt_api_send_error_notify(0x3F, API_ERR_STORAGE_ERROR);
        return;
    }

    uint8_t key_number = nvrecord_env->key_map_number;
    uint8_t read_key_map_data[40] = {0};   // 20 keys * 2 bytes
    bool use_default_map = false;
    bool all_empty = true;

    TRACE(0, "[KEYMAP] nv key_number=%d", key_number);

    if ((key_number == 0) || (key_number > 20))
    {
        TRACE(0, "[KEYMAP] invalid nv key_number=%d, use default", key_number);
        key_number = 20;
        use_default_map = true;
    }
    else
    {
        for (uint8_t i = 0; i < key_number; i++)
        {
            if ((nvrecord_env->key_map_action[i] != 0) ||
                (nvrecord_env->key_map_func[i] != 0))
            {
                all_empty = false;
                break;
            }
        }

        if (all_empty)
        {
            TRACE(0, "[KEYMAP] nv keymap empty, use default");
            key_number = 20;
            use_default_map = true;
        }
    }

    for (uint8_t i = 0; i < key_number; i++)
    {
        uint8_t action = 0;
        uint8_t func = 0;

        if (use_default_map)
        {
            action = s_key_default_map[i].actions;
            func   = s_key_default_map[i].function;
        }
        else
        {
            action = nvrecord_env->key_map_action[i];
            func   = nvrecord_env->key_map_func[i];
        }

        TRACE(0, "[KEYMAP] i=%d action=0x%02X func=0x%02X",
              i, action, func);

        read_key_map_data[i * 2]     = action;
        read_key_map_data[i * 2 + 1] = func;
    }

    TRACE(0, "[KEYMAP] number=%d payload_len=%d source=%s",
          key_number,
          key_number * 2,
          use_default_map ? "default" : "nv");

    DUMP8("%02X ", read_key_map_data, key_number * 2);

    sparraw_tx_msg(RSP_GET_KEY_MAPPING,
                   read_key_map_data,
                   key_number * 2);
}

void handleSetKeyMapping(const uint8_t *data, uint16_t len)
{
    TRACE(0, "%s.", __func__);

    /*
     * Packet format:
     *
     * data[0] : command
     * data[1] : data length high
     * data[2] : data length low
     * data[3] : changed key count
     * data[4] : action 0
     * data[5] : function 0
     * data[6] : action 1
     * data[7] : function 1
     * ...
     *
     * data_len = 1 + key_count * 2
     */

    if ((data == NULL) || (len < 6))
    {
        TRACE(0,
              "[KEYMAP][SET] invalid packet data=%p len=%d",
              data,
              len);

        ntt_api_send_error_notify(0x43, API_ERR_INVALID_PARAM);
        return;
    }

    const uint8_t *data_buf = data + 1;

    uint16_t data_len =
        ((uint16_t)data_buf[0] << 8) |
        ((uint16_t)data_buf[1]);

    uint8_t update_key_count = data_buf[2];

    /*
     * At least one key must be included.
     */
    if ((update_key_count == 0) ||
        (update_key_count > 20))
    {
        TRACE(0,
              "[KEYMAP][SET] invalid update count=%d",
              update_key_count);

        ntt_api_send_error_notify(0x43, API_ERR_INVALID_PARAM);
        return;
    }

    /*
     * data_len contains:
     *   1 byte key_count
     *   2 bytes for each mapping
     */
    uint16_t expected_data_len =
        (uint16_t)(1U + ((uint16_t)update_key_count * 2U));

    if (data_len != expected_data_len)
    {
        TRACE(0,
              "[KEYMAP][SET] data_len mismatch rx=%d expected=%d",
              data_len,
              expected_data_len);

        ntt_api_send_error_notify(0x43, API_ERR_INVALID_PARAM);
        return;
    }

    /*
     * Total packet:
     * command(1) + length(2) + key_count(1)
     * + mapping(update_key_count * 2)
     */
    uint16_t expected_packet_len =
        (uint16_t)(4U + ((uint16_t)update_key_count * 2U));

    if (len != expected_packet_len)
    {
        TRACE(0,
              "[KEYMAP][SET] packet len mismatch rx=%d expected=%d",
              len,
              expected_packet_len);

        ntt_api_send_error_notify(0x43, API_ERR_INVALID_PARAM);
        return;
    }

    /*
     * Ensure RAM/NV mapping has been initialized.
     *
     * If NV is empty, this function writes the default 20-key map.
     * If NV already has valid data, it loads the existing map into RAM.
     */
    keymap_init_default();

    struct nvrecord_env_t *nvrecord_env = NULL;
    nv_record_env_get(&nvrecord_env);

    if (nvrecord_env == NULL)
    {
        TRACE(0, "[KEYMAP][SET] nvrecord_env is NULL");

        ntt_api_send_error_notify(0x43, API_ERR_STORAGE_ERROR);
        return;
    }

    uint8_t current_key_count = nvrecord_env->key_map_number;

    if ((current_key_count == 0) ||
        (current_key_count > 20))
    {
        TRACE(0,
              "[KEYMAP][SET] invalid current count=%d",
              current_key_count);

        ntt_api_send_error_notify(0x43, API_ERR_STORAGE_ERROR);
        return;
    }

    const uint8_t *key_map = &data_buf[3];

    /*
     * First pass:
     * Find every incoming action in the existing mapping table.
     *
     * We validate everything first so an invalid packet does not cause
     * only part of the key mapping table to be modified.
     */
    uint8_t update_index[20];

    memset(update_index, 0xFF, sizeof(update_index));

    for (uint8_t i = 0; i < update_key_count; i++)
    {
        uint8_t incoming_action = key_map[i * 2];
        uint8_t incoming_func   = key_map[i * 2 + 1];
        bool found = false;

        /*
         * Reject duplicate action entries in the same APP packet.
         */
        for (uint8_t check = 0; check < i; check++)
        {
            if (key_map[check * 2] == incoming_action)
            {
                TRACE(0,
                      "[KEYMAP][SET] duplicate incoming action=0x%02X",
                      incoming_action);

                ntt_api_send_error_notify(0x43,
                                          API_ERR_INVALID_PARAM);
                return;
            }
        }

        /*
         * action is the key identifier.
         * Find the matching action in the current complete mapping table.
         */
        for (uint8_t j = 0; j < current_key_count; j++)
        {
            if (nvrecord_env->key_map_action[j] ==
                incoming_action)
            {
                update_index[i] = j;
                found = true;

                TRACE(0,
                      "[KEYMAP][SET] match input=%d nv_index=%d "
                      "action=0x%02X old_func=0x%02X new_func=0x%02X",
                      i,
                      j,
                      incoming_action,
                      nvrecord_env->key_map_func[j],
                      incoming_func);

                break;
            }
        }

        if (!found)
        {
            TRACE(0,
                  "[KEYMAP][SET] action not found=0x%02X",
                  incoming_action);

            /*
             * Do not append an unknown action because the current protocol
             * is intended to update an existing key definition.
             */
            ntt_api_send_error_notify(0x43,
                                      API_ERR_INVALID_PARAM);
            return;
        }
    }

    /*
     * Second pass:
     * All actions are valid. Update only the matching function fields.
     */
    bool mapping_changed = false;

    for (uint8_t i = 0; i < update_key_count; i++)
    {
        uint8_t nv_index = update_index[i];
        uint8_t incoming_action = key_map[i * 2];
        uint8_t incoming_func   = key_map[i * 2 + 1];

        if (nv_index >= current_key_count)
        {
            TRACE(0,
                  "[KEYMAP][SET] invalid resolved index=%d",
                  nv_index);

            ntt_api_send_error_notify(0x43,
                                      API_ERR_STORAGE_ERROR);
            return;
        }

        /*
         * action remains unchanged because it is used as the Key ID.
         * Only overwrite the function selected by the APP.
         */
        if (nvrecord_env->key_map_func[nv_index] != incoming_func)
        {
            TRACE(0,
                  "[KEYMAP][SET] update index=%d "
                  "action=0x%02X func:0x%02X->0x%02X",
                  nv_index,
                  incoming_action,
                  nvrecord_env->key_map_func[nv_index],
                  incoming_func);

            nvrecord_env->key_map_func[nv_index] = incoming_func;
            mapping_changed = true;
        }
        else
        {
            TRACE(0,
                  "[KEYMAP][SET] unchanged index=%d "
                  "action=0x%02X func=0x%02X",
                  nv_index,
                  incoming_action,
                  incoming_func);
        }

        /*
         * Keep the runtime RAM table synchronized immediately.
         */
        s_key_map[nv_index].actions =
            nvrecord_env->key_map_action[nv_index];

        s_key_map[nv_index].function =
            nvrecord_env->key_map_func[nv_index];
    }

    /*
     * Do not modify key_map_number.
     *
     * APP update_key_count means "number of changed keys",
     * not "total number of keys in the mapping table".
     */
    s_key_map_count = current_key_count;

    /*
     * Write NV only once after all entries have been updated.
     */
    if (mapping_changed)
    {
        nv_record_env_set(nvrecord_env);

        TRACE(0,
              "[KEYMAP][SET] NV save done total=%d updated=%d",
              current_key_count,
              update_key_count);
    }
    else
    {
        TRACE(0,
              "[KEYMAP][SET] no NV write, mapping unchanged");
    }

    /*
     * Synchronize the complete mapping table to the peer earbud.
     *
     * Although APP only changes one or more keys, sending the complete
     * table prevents the left/right earbuds from having different maps.
     */
    uint8_t cmd_sync_button_map[50] = {0};

    cmd_sync_button_map[0] = current_key_count;

    for (uint8_t i = 0; i < current_key_count; i++)
    {
        cmd_sync_button_map[i * 2 + 1] =
            nvrecord_env->key_map_action[i];

        cmd_sync_button_map[i * 2 + 2] =
            nvrecord_env->key_map_func[i];

        TRACE(0,
              "[KEYMAP][SYNC] index=%d action=0x%02X func=0x%02X",
              i,
              cmd_sync_button_map[i * 2 + 1],
              cmd_sync_button_map[i * 2 + 2]);
    }

#ifdef IBRT
    app_ibrt_customif_cmd_sync_button_map(
        cmd_sync_button_map,
        (uint16_t)(current_key_count * 2U + 1U));
#endif

#if need_send_data_by_notify
    sparraw_tx_msg(RSP_SET_KEY_MAPPING,
                   (const uint8_t *)"",
                   0);
#endif
}

void handleSetEqIndex(uint8_t index)
{
    struct nvrecord_env_t *nvrecord_env;
    nv_record_env_get(&nvrecord_env);

	nvrecord_env->eq_index_data = index;

	nv_record_env_set(nvrecord_env);

}

void handleGetKeyMapNumber(uint8_t *index)
{
    struct nvrecord_env_t *nvrecord_env;
    nv_record_env_get(&nvrecord_env);
	*index = nvrecord_env->key_map_number;
}

void handleSetKeyMapNumber(uint8_t index)
{
    struct nvrecord_env_t *nvrecord_env;
    nv_record_env_get(&nvrecord_env);

	nvrecord_env->key_map_number = index;

	nv_record_env_set(nvrecord_env);

}
void handleGetKeyMapActionAndFunc(uint8_t index,uint8_t *action,uint8_t *func)
{
    struct nvrecord_env_t *nvrecord_env;
    nv_record_env_get(&nvrecord_env);
	*action = nvrecord_env->key_map_action[index];
	*func = nvrecord_env->key_map_func[index];
}

void handleSetKeyMapActionAndFunc(uint8_t index,uint8_t action,uint8_t func)
{
    struct nvrecord_env_t *nvrecord_env;
    nv_record_env_get(&nvrecord_env);

	nvrecord_env->key_map_action[index] = action;
	nvrecord_env->key_map_func[index] = func;

	nv_record_env_set(nvrecord_env);

}

void handleGetEqIndex(uint8_t *index)
{
    struct nvrecord_env_t *nvrecord_env;
    nv_record_env_get(&nvrecord_env);
	*index = nvrecord_env->eq_index_data;
}

void handleGetEqPresent(const uint8_t *data, uint16_t len)
{
    TRACE(0, "%s.", __func__);

    if ((data == NULL) || (len == 0))
    {
        ntt_api_send_error_notify(0x47, API_ERR_INVALID_PARAM);
        return;
    }

    uint8_t index = 0xFF;

    handleGetEqIndex(&index);

    if (index >= 6)
    {
        ntt_api_send_error_notify(0x47, API_ERR_STORAGE_ERROR);
        return;
    }

    TRACE(0, "[EQ] preset=%d", index);

    sparraw_tx_msg(RSP_GET_EQ_PRESET,
                   &index,
                   sizeof(index));
}

#include "hw_codec_iir_process.h"
#include "audio_process.h"
//extern int audio_eq_hw_dac_iir_callback(uint8_t *buf, uint32_t  len);
extern const IIR_CFG_T * const POSSIBLY_UNUSED audio_eq_cfg_vol_list[VOL_CTRL_EQ_LIST_NUM];
//#include "hw_codec_iir_process.h"
//IIR_CFG_T hw_dac_iir_cfg;
#if 0
const IIR_CFG_T audio_eq_iir_cfg = {
    .gain0 = 0.0,
    .gain1 = 0.0,
    .num = 6,
    .param = {
        {IIR_TYPE_PEAK, 0, 500, 0.7},
        {IIR_TYPE_PEAK, 0, 1000, 0.7},
        {IIR_TYPE_PEAK, 0, 2000, 0.7},
        {IIR_TYPE_PEAK, -23.5, 4983, 0.7},
        {IIR_TYPE_PEAK, 12.3, 4806, 0.7},
        {IIR_TYPE_PEAK, -20.3, 10837, 0.7},
    }
};
#endif
void handleSetEqPresent(const uint8_t *data, uint16_t len)
{
    TRACE(0, "%s.", __func__);

    uint8_t presetId = 0;

    if ((data == NULL) || (len < 4))
    {
        ntt_api_send_error_notify(0x4B, API_ERR_INVALID_PARAM);
        return;
    }

    uint16_t data_len = ((uint16_t)data[1] << 8) | data[2];

    if (data_len != 1)
    {
        ntt_api_send_error_notify(0x4B, API_ERR_INVALID_PARAM);
        return;
    }

    presetId = data[3];

    if (presetId >= 6)
    {
        TRACE(0, "%s invalid presetId=%d", __func__, presetId);
        ntt_api_send_error_notify(0x4B, API_ERR_INVALID_PARAM);
        return;
    }

    TRACE(0, "%s[%d].", __func__, presetId);

    audio_eq_set_cfg(NULL,
                     audio_eq_cfg_vol_list[presetId],
                     AUDIO_EQ_TYPE_HW_DAC_IIR);

    app_ibrt_customif_cmd_sync_music_eq(presetId);

#ifdef __AUDIO_DYNAMIC_BOOST__
#ifdef DYNAMIC_BOOST_USE_HW_EQ
    audio_dynamic_boost_set_new_customer_iir_eq(&audio_process.hw_dac_iir_cfg,
                                                AUDIO_EQ_TYPE_HW_DAC_IIR);
#endif
#endif

    handleSetEqIndex(presetId);

#if need_send_data_by_notify
    sparraw_tx_msg(RSP_SET_EQ_PRESET, (const uint8_t *)"", 0);
#endif
}

void handleSetDtmEnable(const uint8_t *data, uint16_t len)
{
    TRACE(0, "%s.", __func__);

    /*
     * APP packet:
     *
     * Enable:
     *   90 00 01 01
     *
     * Disable:
     *   90 00 01 00
     *
     * data[0] = command
     * data[1] = payload length high
     * data[2] = payload length low
     * data[3] = enable
     */

    if ((data == NULL) || (len != 4))
    {
        TRACE(0,
              "[DTM][SET] invalid packet data=%p len=%d",
              data,
              len);

        ntt_api_send_error_notify(RSP_SET_DTM_ENABLE,
                                  API_ERR_INVALID_PARAM);
        return;
    }

    uint16_t data_len =
        ((uint16_t)data[1] << 8) |
        ((uint16_t)data[2]);

    if (data_len != 1)
    {
        TRACE(0,
              "[DTM][SET] invalid data_len=%d",
              data_len);

        ntt_api_send_error_notify(RSP_SET_DTM_ENABLE,
                                  API_ERR_INVALID_PARAM);
        return;
    }

    uint8_t enable = data[3];

    if (enable > 1)
    {
        TRACE(0,
              "[DTM][SET] invalid enable=%d",
              enable);

        ntt_api_send_error_notify(RSP_SET_DTM_ENABLE,
                                  API_ERR_INVALID_PARAM);
        return;
    }

    if (enable)
    {
        TRACE(0, "[DUT] Enter DTM mode");

        /*
         * Disable 1-Mic Noise Suppression.
         */
        ntt_dut_speech_tx_1mic_ns_bypass_set(1);

        TRACE(0, "[DUT] TX 1Mic NS -> BYPASS");
    }
    else
    {
        TRACE(0, "[DUT] Exit DTM mode");

        /*
         * Restore 1-Mic Noise Suppression.
         */
        ntt_dut_speech_tx_1mic_ns_bypass_set(0);

        TRACE(0, "[DUT] TX 1Mic NS -> NORMAL");
    }

#if need_send_data_by_notify
    /*
     * Response:
     *   92 00 01 01
     * or
     *   92 00 01 00
     */
    sparraw_tx_msg(RSP_SET_DTM_ENABLE,
                   &enable,
                   sizeof(enable));
#endif
}

void aiWangSetBoxVersion(uint8_t *data, uint8_t len)
{
    struct nvrecord_env_t *nvrecord_env;
    nv_record_env_get(&nvrecord_env);
    len  = len >16?16:len;
    if(memcmp(nvrecord_env->chargerBoxVersion, data, len))
    {
		memset(&nvrecord_env->chargerBoxVersion[0], 0, 15+1); //16+1
	 	memcpy(&nvrecord_env->chargerBoxVersion[0], data, len);
		TRACE(0, "set box version:%s", nvrecord_env->chargerBoxVersion);
		nv_record_env_set(nvrecord_env);
    }
}

void aiWangGetChargerBoxVersion(uint8_t *data)
{
    struct nvrecord_env_t *nvrecord_env;
    nv_record_env_get(&nvrecord_env);
    //memcpy(data, &nvrecord_env->chargerBoxVersion[0], 15); //16
	memcpy(data, &nvrecord_env->chargerBoxVersion[0], 11);
}

//MM.NN.RR.AA Earbuds version format, MM: major version, NN: minor version, RR: revision version, AA: additional info
//MM.NN.RR.AA     Box version format, MM: major version, NN: minor version, RR: revision version, AA: additional info

void handleGetFwVersion(const uint8_t *data, uint16_t len)
{
    TRACE(0, "%s.", __func__);

    if ((data == NULL) || (len < 3))
    {
        ntt_api_send_error_notify(0x4F, API_ERR_INVALID_PARAM);
        return;
    }

    const char *earbud_ver = DISPLAY_EARBUDS_VERSION;
    uint16_t fw_len = 0;

    if ((earbud_ver == NULL) || (strlen(earbud_ver) == 0))
    {
        ntt_api_send_error_notify(0x4F, API_ERR_STORAGE_ERROR);
        return;
    }

    fw_len = strlen(earbud_ver);

    TRACE(0, "GET_FW_VERSION");
    TRACE(0, "DISPLAY_EARBUDS_VERSION=%s", earbud_ver);
    TRACE(0, "FW Version Len=%d", fw_len);
    TRACE(0, "FW Version Send=%s", earbud_ver);

#if need_send_data_by_notify
    sparraw_tx_msg(RSP_GET_FW_VERSION,
                   (const uint8_t *)earbud_ver,
                   fw_len);
#endif
}

void handleFactoryCmdSys(const uint8_t *data, uint16_t len)
{
	  TRACE(0,"%s.", __func__);
	  sparraw_tx_cmd_data_rsp_ack(data[0], data[1]);
	  if (ENTER_FACTORY_MODE == data[1]) {
		  TRACE(0, "ENTER_FACTORY_MODE");
		  app_factorymode_enter();
	  } else if (EXIT_AND_REBOOT == data[1]) {
		  TRACE(0, "EXIT_AND_REBOOT");
		  (void)app_reset();
	  } else if (ENTER_SHIP_MODE == data[1]) {
		  TRACE(0, "ENTER_SHIP_MODE");
		  Icp1205ShipEnable();
	  } else if (FACTORY_RESET == data[1]) {
		  TRACE(0, "FACTORY_RESET exit keyClickTestMode");
		  enterKeyClickTestMode = FALSE;
		  //nv_record_ddbrec_clear();
		  //app_ibrt_customif_cmd_sync_clear_pairlist(false, false);
	  }
}


extern "C" uint8_t aiWangGetEarBudsColor(void)
{
    return ntt_color_code_nv_get();
}

void aiWangSetEarBudsColor(uint8_t color)
{
    ntt_color_code_nv_set(color);
}

void handleFactoryCmdAudio(const uint8_t *data, uint16_t len)
{
	  TRACE(0,"%s.", __func__);
	  sparraw_tx_cmd_data_rsp_ack(data[0], data[1]);
	  if (AUDIO_LOOPBACK == payload_buffer[1]) {
		  TRACE(0, "AUDIO_LOOPBACK");
		  app_factorymode_audioloop(true, APP_SYSFREQ_104M);
	  } else if (PLAY_TEST_TONE == payload_buffer[1]) {
		  TRACE(0, "PLAY_TEST_TONE");
		  int stop = payload_buffer[2];
		  if('1' == stop)
		  {
		      media_PlayAudio_continuous_end(AUD_ID_TONE_1K, 0);
		  }
		  else
		  {
		      media_PlayAudio_continuous_start(AUD_ID_TONE_1K, 0);
		  }
	  } else if (LED_CONTROL == payload_buffer[1]) {
		  TRACE(0, "LED_CONTROL");
	  }  else if (BUTTON_EVENT == payload_buffer[1]) {
		  TRACE(0, "BUTTON_EVENT");
		  enterKeyClickTestMode = TRUE;
	  }
}


void aiWangSetSn(uint8_t *data, uint8_t len){

    struct nvrecord_env_t *nvrecord_env;
    nv_record_env_get(&nvrecord_env);
    nvrecord_env->sn_len = (len > 12?12:len);
    for(int i = 0; i < nvrecord_env->sn_len ; i++)
    {
        nvrecord_env->sn_data[i] = data[i];
    }
    nv_record_env_set(nvrecord_env);
}

void aiWangGetSn( uint8_t *data, uint8_t len) {
    struct nvrecord_env_t *nvrecord_env;
    nv_record_env_get(&nvrecord_env);
    len  = nvrecord_env->sn_len >12?12:nvrecord_env->sn_len;
    memcpy(data, &nvrecord_env->sn_data[0], 12);
}


void handleFactoryCmdInfo(const uint8_t *data, uint16_t len)
{
	  TRACE(0,"%s.", __func__);
	  uint8_t  tempSn[12+1] = {0};
	  if (READ_SN == data[1]) {
		  aiWangGetSn(tempSn, 12);
		  TRACE(0, "READ_SN:%s", tempSn);
#if need_send_data_by_notify
		  if(app_sparraw_env.notifyEnable) { ble_aiwang_srv_send_data_via_notification(tempSn, strlen((const char*)tempSn)); }
#endif
	  } else if (WRTIE_SN == data[1]) {
		  memcpy(tempSn, &data[2], (len-2) > 12 ? 12 :(len-2));
		  TRACE(0, "WRTIE_SN:%s", tempSn);
		  aiWangSetSn(tempSn, (len-2) > 12 ? 12 :(len-2));
#if need_send_data_by_notify		  
		  if(app_sparraw_env.notifyEnable) ble_aiwang_srv_send_data_via_notification(tempSn, len);
#endif		  
	  } else if (SET_BUDS_COLOR == payload_buffer[1]) {
		  aiWangSetEarBudsColor(payload_buffer[2]);
		  sparraw_tx_cmd_data_rsp_ack(data[0], data[1]);
	  } else  {
		  TRACE(0, "unknown ...");
	  }
}

void handleGetLocalBtAddress(const uint8_t *data, uint16_t len)
{
	  TRACE(0,"%s.", __func__);
	  uint8_t *bt_local_addr = NULL;
	  uint8_t buff[8] = {0};
	  buff[0] = data[0];
	  buff[1] = 6;
	  bt_local_addr = (uint8_t *)bt_get_local_address();
	  memcpy(&buff[2], bt_local_addr, 6);
	  if(app_sparraw_env.notifyEnable)
	  {
#if need_send_data_by_notify
		  ble_aiwang_srv_send_data_via_notification(buff, 8);
#endif
	  }
	  REL_TRACE_NOCRLF(0, "GetLocalBtAddress: ");
	  DUMP8("%02X ", bt_local_addr, 6);
}

extern void app_tws_ibrt_update_info(ibrt_role_e ibrtRole,bt_bdaddr_t *ibrtPeerAddr);
void handleSetPeerBtAddress(const uint8_t *data, uint16_t len)
{
	  TRACE(0,"%s.", __func__);
	  uint8_t *bt_local_addr = NULL;
	  bt_bdaddr_t peerAddress;
	  bool ret = false;
	  bt_local_addr = (uint8_t *)bt_get_local_address();
	  if ( len >= 6 ) {
		  REL_TRACE_NOCRLF(0, "localAddress: ");
		  DUMP8("%02X ", bt_local_addr, 6);
		  REL_TRACE_NOCRLF(0, "setPeerAddress: ");
		  DUMP8("%02X ", &data[1], 6);
		  if ( memcmp(&data[1],bt_local_addr,6))
		  //if(bt_local_addr[5] == data[5] && bt_local_addr[4] == data[4] && bt_local_addr[3] == data[3])
		  {
			  memcpy(&peerAddress.address[0], &data[1], 6);
			  //app_tws_ibrt_update_info(IBRT_MASTER, &peerAddress);
		      nv_record_update_ibrt_info(data[1]&0x01?IBRT_SLAVE:IBRT_MASTER, &peerAddress);
			  ret = true;
			  osDelay(200);
			  (void)app_reset();
		  }

	  }
	  if(app_sparraw_env.notifyEnable)
	  {
#if need_send_data_by_notify
		  ble_aiwang_srv_send_data_via_notification((uint8_t*)(ret?"OK":"NG"), 2);
#else
	if(ret){}
#endif
	  }

}

typedef struct {
	uint8_t cmd;
	void (*handleFunc)(const uint8_t *data, uint16_t len);
} CMD_HANDLE_TABLE;

static const CMD_HANDLE_TABLE aiWangCmdTypes[] = {
	    {GET_BATTERY_LEVEL,   handleGetBatteryLevel},
		{GET_DEVICE_NAME,     handleGetDeviceName},
		{SET_DEVICE_NAME,     handleSetDeviceName},
		{GET_KEY_MAPPING,     handleGetKeyMapping},
		{SET_KEY_MAPPING,	  handleSetKeyMapping},
		{GET_EQ_PRESET,		  handleGetEqPresent},
		{SET_EQ_PRESET,       handleSetEqPresent},
		{GET_FW_VERSION,      handleGetFwVersion},
        {SET_DTM_ENABLE,      handleSetDtmEnable},
		{FACTORY_COMMAND_SYS, handleFactoryCmdSys},
		{FACTORY_COMMAND_AUDIO_IO,handleFactoryCmdAudio},
		{FACTORY_COMMAND_INFO,    handleFactoryCmdInfo},
		{GET_LOCAL_BT_ADDR,       handleGetLocalBtAddress},
		{SET_PEER_BT_ADDR,        handleSetPeerBtAddress},
};

static const uint8_t aiWangCmdTypesCount =
    sizeof(aiWangCmdTypes) / sizeof(aiWangCmdTypes[0]);

static void sparraw_ble_init(void)
{
    memset((uint8_t *)&app_sparraw_env, 0, sizeof(app_sparraw_env));
    app_sparraw_env.conidx = 0xff;
    app_sparraw_env.notifyEnable = false;
}

static int32_t sparraw_event_mailbox_init(void)
{
    sparraw_event_mailbox_id = osMailCreate(osMailQ(sparraw_event_mailbox_id), NULL);
    if (sparraw_event_mailbox_id == NULL) {
        OTA_TRACE(0, "Failed to Create sparraw event mailbox");
        return -1;
    }
	else
		{
		OTA_TRACE(0, "Success to Create sparraw event mailbox");
	}
    return 0;
}

static osStatus sparraw_event_mailbox_free(SPARRAW_MESSAGE_BLOCK* rx_event)
{
    osStatus status;
    if (sparraw_event_mailbox_id == NULL || rx_event == NULL) {
        return osErrorParameter;
    }
    status = osMailFree(sparraw_event_mailbox_id, rx_event);
    ASSERT(osOK == status, "Free sparraw rx event mailbox failed!");
    return status;
}

int sparraw_event_mailbox_free_all(void)
{
    SPARRAW_MESSAGE_BLOCK *msg_p = NULL;
    int status = osOK;
    osEvent evt;

    if (sparraw_event_mailbox_id == NULL) {
        return osErrorParameter;
    }

    while (osMailGetCount(sparraw_event_mailbox_id) > 0) {
        evt = osMailGet(sparraw_event_mailbox_id, 500);
        if (evt.status == osEventMail) {
            msg_p = (SPARRAW_MESSAGE_BLOCK *)evt.value.p;
            status = sparraw_event_mailbox_free(msg_p);
        } else {
            GFPS_TRACE(1, "%s get mailbox timeout!!!", __func__);
            break;
        }
    }
    return status;
}

static int32_t sparraw_event_mailbox_get(SPARRAW_MESSAGE_BLOCK** rx_event)
{
    osEvent evt;
    evt = osMailGet(sparraw_event_mailbox_id, osWaitForever);
    if (evt.status == osEventMail) {
        *rx_event = (SPARRAW_MESSAGE_BLOCK *)evt.value.p;
        TRACE(0, "len %d", (*rx_event)->len);
        return 0;
    }
    return -1;
}
static void sparraw_rx_cmd_parse_v2(const uint8_t *data, uint16_t len);


#if 1
static uint8_t neet_notify_send_flash = 0;
#endif
static uint8_t aiwan_read_data = 0;
#if 1
static void need_notify_send(uint8_t *param, uint16_t len)
{
	aiwan_read_data = param[0];
	switch(param[0])
	{
		case GET_BATTERY_LEVEL:		
		{
			neet_notify_send_flash = 1;
		}break;
		
		default:neet_notify_send_flash = 1;break;
	}
}
#endif
int sparraw_mailbox_put(uint8_t devId, uint8_t event, uint8_t *param, uint16_t len)
{
#if 0
    osStatus status = osOK;
    SPARRAW_MESSAGE_BLOCK *msg_p = NULL;

    if (sparraw_event_mailbox_id == NULL) {
        return -1;
    }

	need_notify_send(param,len);
	if(neet_notify_send_flash == 0)
	{
		return 0;
	}

    msg_p = (SPARRAW_MESSAGE_BLOCK*)osMailAlloc(sparraw_event_mailbox_id, 0);
    if (msg_p == NULL)
    {
        sparraw_event_mailbox_free_all();
        msg_p = (SPARRAW_MESSAGE_BLOCK*)osMailAlloc(sparraw_event_mailbox_id, 0);
        ASSERT(msg_p, "sparraw_mailbox_put osMailAlloc error\r\n");
    }
	
    msg_p->devId = devId;
    msg_p->event = event;
    msg_p->len   = (len >256?256:len);
    if (param && len > 0)
    {
        memcpy((uint8_t *)&msg_p->data[0], param, msg_p->len);
    }

    status = osMailPut(sparraw_event_mailbox_id, msg_p);
    if (osOK != status)
    {
        TRACE(0,"%s error status 0x%02x", __func__, status);
        osMailFree(sparraw_event_mailbox_id, msg_p);
        return (int)status;
    }
#else
	
	osStatus status = osOK;
	#if 0
	if (sparraw_event_mailbox_id == NULL) {
			return -1;
		}
	#endif
	need_notify_send(param,len);
	sparraw_rx_cmd_parse_v2(param, len);
#endif
    return (int)status;
}

static void sparraw_tx_cmd_data_rsp_ack(uint8_t cmd_type, uint8_t sub_cmd){
	uint8_t  rsp_buffer[64];
	rsp_buffer[0] = RESPONSE_WITHOUT_ERROR;
	rsp_buffer[1] = 0;
	rsp_buffer[2] = 2;
	rsp_buffer[3] = cmd_type;
	rsp_buffer[4] = sub_cmd;
	if(app_sparraw_env.notifyEnable) ble_aiwang_srv_send_data_via_notification(rsp_buffer, 5);
}

static void sparraw_tx_cmd_data_rsp_neg(uint8_t error_code, uint8_t cmd_type, uint8_t sub_cmd){
	uint8_t  rsp_buffer[64];
	rsp_buffer[0] = RESPONSE_WITH_ERROR;
	rsp_buffer[1] = 0;
	rsp_buffer[2] = 3;
	rsp_buffer[3] = cmd_type;
	rsp_buffer[4] = sub_cmd;
	rsp_buffer[5] = error_code;
	if(app_sparraw_env.notifyEnable) ble_aiwang_srv_send_data_via_notification(rsp_buffer, 6);
}

//extern void handleGetKeyMapActionAndFunc(uint8_t index,uint8_t *action,uint8_t *func);

#if 0
//button function
/***********************************************/
/***********************************************/
// 操作类型（高4位）
typedef enum {
    ACTION_IDLE_MUSIC = 0x00,   // 空闲/音乐播放
    ACTION_CALL       = 0x01,   // 通话
    ACTION_LEFT       = 0x20,   // 左
    ACTION_RIGHT      = 0x30    // 右
} action_type_t;

// 点击类型（低4位）
typedef enum {
    CLICK_SINGLE       = 0x00,  // 单击
    CLICK_DOUBLE       = 0x01,  // 双击
    CLICK_TRIPLE       = 0x02,  // 三击
    CLICK_DOUBLE_HOLD  = 0x03,  // 双击并按住
    CLICK_HOLD_2S      = 0x04   // 按住2秒
} click_type_t;

// 功能码
typedef enum {
    FUNC_NOT_ASSIGNED   = 0x00,
    FUNC_ACCEPT_CALL    = 0x01,
    FUNC_REJECT_CALL    = 0x02,
    FUNC_PLAY_PAUSE     = 0x03,
    FUNC_NEXT_SONG      = 0x04,
    FUNC_PREV_SONG      = 0x05,
    FUNC_VOLUME_UP      = 0x06,
    FUNC_VOLUME_DOWN    = 0x07,
    FUNC_VOICE_ASSIST   = 0x08
} function_t;

// 单个映射条目
typedef struct {
    uint8_t actions;    // 高4位操作类型 + 低4位点击类型
    uint8_t function;   // 对应的功能
} key_map_entry_t;
// 整个映射块头部（偏移1~3）
typedef struct {
    uint16_t length;    // 总长度 = 2 * key_count + 1
    uint8_t  key_count; // 映射条目数量
    // 后面紧跟 key_count 个 key_map_entry_t
} key_map_header_t;

/* 功能函数类型：无参数、无返回值（可根据需要扩展） */
typedef void (*function_handler_t)(void);

/*==============================================================================
 * 2. 保存映射值的全局表（可修改，支持运行时保存/更新）
 *----------------------------------------------------------------------------*/
#define MAX_KEY_MAP_ENTRIES  21
static key_map_entry_t s_key_map[MAX_KEY_MAP_ENTRIES];
static uint8_t s_key_map_count = 0;

// 1003 1104 1205 1307 1408 2003 2104 2205 2306 2408 
// 5001 5102 5200 5300 5400 6001 6102 6200 6300 6400

const key_map_entry_t s_key_default_map[21]{
	{0x10,0x03},{0x11,0x04},{0x12,0x05},{0x13,0x07},{0x14,0x08},{0x20,0x03},{0x21,0x04},{0x22,0x05},{0x23,0x06},{0x24,0x08},
	{0x50,0x01},{0x51,0x02},{0x52,0x00},{0x53,0x00},{0x54,0x00},{0x60,0x01},{0x61,0x02},{0x62,0x00},{0x63,0x00},{0x64,0x00},
};
#endif
// 初始化默认映射
static void keymap_init_default(void)
{
    uint8_t key_number = 0;

    handleGetKeyMapNumber(&key_number);

    if ((key_number == 0) || (key_number > 20))
    {
        TRACE(0, "[KEYMAP] NV empty, write default to NV");

        s_key_map_count = 20;

        for (uint8_t i = 0; i < 20; i++)
        {
            uint8_t key_action = s_key_default_map[i].actions;
            uint8_t key_func   = s_key_default_map[i].function;

            s_key_map[i].actions  = key_action;
            s_key_map[i].function = key_func;

            handleSetKeyMapActionAndFunc(i, key_action, key_func);
        }

        handleSetKeyMapNumber(20);
    }
    else
    {
        s_key_map_count = key_number;

        for (uint8_t i = 0; i < key_number; i++)
        {
            uint8_t key_action = 0;
            uint8_t key_func   = 0;

            handleGetKeyMapActionAndFunc(i, &key_action, &key_func);

            s_key_map[i].actions  = key_action;
            s_key_map[i].function = key_func;
        }
    }

    TRACE(0, "[KEYMAP] init count=%d", s_key_map_count);
}

// 保存整个映射表（例如写入 Flash / EEPROM）
void keymap_save_config(void)
{
    // 实际项目中可调用存储驱动，将 s_key_map 和 s_key_map_count 保存到非易失介质
    //printf("[KeyMap] Saved %d entries\n", s_key_map_count);
}

// 加载之前保存的映射表
void keymap_load_config(void)
{
    // 实际项目：从非易失介质读取并恢复 s_key_map 和 s_key_map_count
    // 若无保存数据，则调用默认初始化
    TRACE(0,"@@@@@@@%s", __func__);
    keymap_init_default();
    //printf("[KeyMap] Loaded %d entries\n", s_key_map_count);
}

// 动态修改/保存单个映射条目（覆盖已有的 actions，或新增）
void keymap_set_entry(action_type_t action, click_type_t click, function_t func)
{
    uint8_t target_actions = action | click;
    for (int i = 0; i < s_key_map_count; i++) {
        if (s_key_map[i].actions == target_actions) {
            s_key_map[i].function = func;
            printf("[KeyMap] Updated entry 0x%02X -> func 0x%02X\n", target_actions, func);
            return;
        }
    }
    // 未找到则新增
    if (s_key_map_count < MAX_KEY_MAP_ENTRIES) {
        s_key_map[s_key_map_count++] = (key_map_entry_t){ target_actions, func };
        //printf("[KeyMap] Added entry 0x%02X -> func 0x%02X\n", target_actions, func);
    } else {
        //printf("[KeyMap] Error: table full\n");
    }
}

/*==============================================================================
 * 3. 按键查找函数（基于保存的映射表）
 *----------------------------------------------------------------------------*/
function_t keymap_lookup(uint8_t action, click_type_t click ,uint8_t isREarbuds)
{
    uint8_t target = (action << 4) | click;
	//printf("@@target:%d",target);
    for (int i = 0; i < s_key_map_count; i++) {
		//printf("@@s_key_map[].actions:%d",s_key_map[i].actions);
        if (s_key_map[i].actions == target) {
            return (function_t)s_key_map[i].function;
        }
    }
    return FUNC_NOT_ASSIGNED;
}

/*==============================================================================
 * 4. 功能执行函数（具体动作实现）
 *----------------------------------------------------------------------------*/
 #if 0
static uint8_t check_call_status(void)
{
    if (bes_bt_hfp_has_call_active()) {
        return BT_HFP_CALL_ACTIVE;
    } else if (bes_bt_hfp_has_call_setup() == BT_HFP_CALL_SETUP_IN) {
        return BT_HFP_CALL_SETUP_IN;
    } else if (bes_bt_hfp_has_call_setup() == BT_HFP_CALL_SETUP_OUT) {
        return BT_HFP_CALL_SETUP_OUT;
    }else {
        return BT_HFP_CALL_NONE;
    }
}
#endif
static void on_accept_call(void)    {
	//printf(">>> Accept call\n"); 
#if 0
	uint8_t call_status = check_call_status();
	if((call_status == BT_HFP_CALL_SETUP_IN) || (call_status == BT_HFP_CALL_SETUP_OUT))
		bes_bt_hfp_call_action(BT_DEVICE_ID_1, BT_HFP_ANSWER_CALL);
#else
	CALL_STATE_E call_state = app_bt_get_call_state();
    if ((call_state == CALL_STATE_INCOMING) || (call_state == CALL_STATE_THREE_WAY_INCOMING))
    {
       //bt_key_handle_call(call_state);
       app_audio_control_call_answer();
    }
    
#endif
	
}
static void on_reject_call(void)    {
	//printf(">>> Reject/end call\n"); 
#if 0
	bes_bt_hfp_call_action(BT_DEVICE_ID_1, BT_HFP_HANGUP_CALL);
#else
	CALL_STATE_E call_state = app_bt_get_call_state();
    if ((call_state == CALL_STATE_OUTGOING) || (call_state == CALL_STATE_ACTIVE) || (call_state == CALL_STATE_TRREE_WAY_HOLD_CALLING))
    {
       bt_key_handle_call(call_state);
    }
	else if((call_state == CALL_STATE_INCOMING) || (call_state == CALL_STATE_THREE_WAY_INCOMING))
	{
		app_audio_control_call_terminate();
	}
#endif	
}
static void on_play_pause(void)     {
	//printf(">>> Play/Pause\n"); 
#if 1
	uint8_t play_status = app_bt_get_music_playback_status();
	if(play_status == PLAYING)
	{
		app_audio_control_media_pause();
	}
	else
	{
		app_audio_control_media_play();
	}
#else
	uint8_t play_status = app_bt_get_music_playback_status();
	if(play_status == PLAYING)
	{
		app_audio_control_media_pause();
		TRACE(0,"%s PAUSE music", __func__);
	}
	else
	{
		app_audio_control_media_play();
		TRACE(0,"%s PLAYING music", __func__);
	}
#endif
}
static void on_next_song(void)      {
	//printf(">>> Next song\n"); 
	app_audio_control_media_forward();
}
static void on_prev_song(void)      {
	//printf(">>> Previous song\n"); 
	app_audio_control_media_backward();
}
static void on_volume_up(void)      {
	//printf(">>> Volume up\n"); 
	app_audio_control_streaming_volume_up();
}
static void on_volume_down(void)    {
	//printf(">>> Volume down\n"); 
	app_audio_control_streaming_volume_down();
}
static void on_voice_assist(void)   {
	//printf(">>> Voice assistant\n"); 
	app_audio_control_open_voice_assistant();
}

void key_function_execute(function_t func)
{
    switch (func)
    {
        case FUNC_ACCEPT_CALL:
            TRACE(0, "[ACTION] ACCEPT_CALL");
            on_accept_call();
            break;

        case FUNC_REJECT_CALL:
            TRACE(0, "[ACTION] REJECT_CALL");
            on_reject_call();
            break;

        case FUNC_PLAY_PAUSE:
            TRACE(0, "[ACTION] PLAY_PAUSE");
            on_play_pause();
            break;

        case FUNC_NEXT_SONG:
            TRACE(0, "[ACTION] NEXT_SONG");
            on_next_song();
            break;

        case FUNC_PREV_SONG:
            TRACE(0, "[ACTION] PREV_SONG");
            on_prev_song();
            break;

        case FUNC_VOLUME_UP:
            TRACE(0, "[ACTION] VOLUME_UP");
            on_volume_up();
            break;

        case FUNC_VOLUME_DOWN:
            TRACE(0, "[ACTION] VOLUME_DOWN");
            on_volume_down();
            break;

        case FUNC_VOICE_ASSIST:
            TRACE(0, "[ACTION] VOICE_ASSIST");
            on_voice_assist();
            break;

        default:
            TRACE(0, "[ACTION] NONE func=0x%02X", func);
            break;
    }
}

extern "C" uint8_t ntt_color_code_nv_get(void)
{
    struct nvrecord_env_t *nvrecord_env = NULL;

    nv_record_env_get(&nvrecord_env);

    if (nvrecord_env)
    {
        TRACE(0, "[COLOR_CODE][GET] color_data=0x%02X",
              nvrecord_env->color_data);

        switch (nvrecord_env->color_data)
        {
            case NTT_COLOR_CODE_BLACK:
            case NTT_COLOR_CODE_SILVER_WHITE:
            case NTT_COLOR_CODE_GOLD:
                return nvrecord_env->color_data;
        }
    }

    return NTT_COLOR_CODE_BLACK;
}

extern "C" void ntt_color_code_nv_set(uint8_t color)
{
    struct nvrecord_env_t *nvrecord_env = NULL;

    if (!ntt_color_code_is_valid(color))
    {
        TRACE(0, "[COLOR_CODE][SET] invalid=0x%02X", color);
        return;
    }

    nv_record_env_get(&nvrecord_env);

    if (nvrecord_env)
    {
        TRACE(0, "[COLOR_CODE][SET] old=0x%02X new=0x%02X",
              nvrecord_env->color_data,
              color);

        if (nvrecord_env->color_data != color)
        {
            nvrecord_env->color_data = color;
            nv_record_env_set(nvrecord_env);
        }

        TRACE(0, "[COLOR_CODE][SET] readback=0x%02X",
              nvrecord_env->color_data);
    }
}

/*==============================================================================
 * 5. 按键事件处理：判断并调用相关函数
 *----------------------------------------------------------------------------*/
 
#define NTT_KEY_SIDE_RIGHT    0x01
#define NTT_KEY_SIDE_LEFT     0x02

static const char *ntt_key_side_str(uint8_t side)
{
    return (side == NTT_KEY_SIDE_LEFT) ? "LEFT" : "RIGHT";
}

static uint8_t ntt_get_key_side_snapshot(void)
{
    uint8_t *bt_local_addr = NULL;
    uint8_t local_side = NTT_KEY_SIDE_RIGHT;
    uint8_t key_side = NTT_KEY_SIDE_RIGHT;

    bt_local_addr = (uint8_t *)bt_get_local_address();

    /*
     * Original project rule:
     * bt_local_addr[0] odd  -> LEFT  = 0x02
     * bt_local_addr[0] even -> RIGHT = 0x01
     */
    local_side = (bt_local_addr[0] & 0x01) ? NTT_KEY_SIDE_LEFT : NTT_KEY_SIDE_RIGHT;
    key_side = local_side;

    /*
     * key_event_is_left means key event comes from peer side.
     * Convert local side to peer side.
     */
    if (key_event_is_left == 1)
    {
        key_side = (local_side == NTT_KEY_SIDE_LEFT) ?
                    NTT_KEY_SIDE_RIGHT :
                    NTT_KEY_SIDE_LEFT;
    }

    TRACE(0,
          "[KEYSIDE] local_addr_lsb=0x%02X local=%s key_event_is_left=%d resolved=%s",
          bt_local_addr[0],
          ntt_key_side_str(local_side),
          key_event_is_left,
          ntt_key_side_str(key_side));

    return key_side;
}

#ifdef SUPPORT_SIRI

static bool ntt_voice_assist_is_active(void)
{
    for (uint8_t i = 0; i < BT_DEVICE_NUM; i++)
    {
        struct BT_DEVICE_T *curr_device = app_bt_get_device(i);

        if ((curr_device != NULL) &&
            curr_device->hf_conn_flag &&
            btif_hf_is_voice_rec_active(curr_device->hf_channel))
        {
            return true;
        }
    }

    return false;
}

static void ntt_voice_assist_close(void)
{
    for (uint8_t i = 0; i < BT_DEVICE_NUM; i++)
    {
        struct BT_DEVICE_T *curr_device = app_bt_get_device(i);

        if ((curr_device != NULL) &&
            curr_device->hf_conn_flag &&
            btif_hf_is_voice_rec_active(curr_device->hf_channel))
        {
            TRACE(0,
                  "[ACTION] VOICE_ASSIST_CLOSE dev=%d",
                  i);

            btif_hf_enable_voice_recognition(
                curr_device->hf_channel,
                false);
        }
    }
}

#endif

 void handle_key_event(click_type_t click, uint8_t key_side)
{
    uint8_t action = 0;
    uint8_t action_er = 0;
    function_t func;
    CALL_STATE_E call_state = app_bt_get_call_state();

    if (call_state == CALL_STATE_IDLE)
    {
        action = 0x00;
    }
    else
    {
        action = 0x01;
    }

    action_er = (action << 2) | key_side;

    TRACE(0,
          "[KEYMAP] call_state=%d state=%s side=%s click=%d action_er=0x%02X",
          call_state,
          (action == 0) ? "IDLE/MUSIC" : "CALL",
          ntt_key_side_str(key_side),
          click,
          action_er);

    func = keymap_lookup(action_er, click, key_side);

    TRACE(0, "[KEYMAP] lookup func=0x%02X", func);

    key_function_execute(func);
}

/*
void handle_key_event(click_type_t click)
{
    uint8_t action = 0;
    CALL_STATE_E call_state = app_bt_get_call_state();

    if (call_state == CALL_STATE_IDLE)
    {
        action = 0x00;
    }
    else
    {
        action = 0x01;
    }

    uint8_t *bt_local_addr = (uint8_t *)bt_get_local_address();

    uint8_t local_side = (bt_local_addr[0] & 0x01) ? 0x02 : 0x01;
    uint8_t isLeftEarbuds = local_side;

    if (key_event_is_left == 1)
    {
        isLeftEarbuds = (local_side == 0x02) ? 0x01 : 0x02;
    }

    uint8_t action_er = (action << 2) | isLeftEarbuds;

    TRACE(0,
        "[KEYMAP] call_state=%d state=%s local_side=%s key_event_is_left=%d resolved_side=%s click=%d action_er=0x%02X",
        call_state,
        (action == 0) ? "IDLE/MUSIC" : "CALL",
        (local_side == 0x02) ? "LEFT" : "RIGHT",
        key_event_is_left,
        (isLeftEarbuds == 0x02) ? "LEFT" : "RIGHT",
        click,
        action_er);

    function_t func = keymap_lookup(action_er, click, isLeftEarbuds);

    TRACE(0, "[KEYMAP] lookup func=0x%02X", func);

    key_function_execute(func);
}
*/

/***********************************************/
void aparraw_set_key_event_left(uint8 status)
{
	key_event_is_left = status;
}

/*************************************************/
static uint8_t g_button_hold_side = NTT_KEY_SIDE_RIGHT;

static void ntt_cancel_double_hold_state(const char *reason)
{
    TRACE(0,
          "[KEYMAP] cancel hold state reason=%s type=0x%02X timer=%p side=%s",
          reason ? reason : "unknown",
          button_hold_type,
          double_hold_idle_timer_id,
          ntt_key_side_str(g_button_hold_side));

    /*
     * 必須先清除狀態，再停止 timer。
     *
     * 即使 timer callback 已經進入排程，
     * callback 看到 button_hold_type == 0xFF
     * 也不會再執行 DOUBLE_HOLD 功能。
     */
    button_hold_type = 0xFF;
    g_button_hold_side = NTT_KEY_SIDE_RIGHT;

    if (double_hold_idle_timer_id != NULL)
    {
        osTimerStop(double_hold_idle_timer_id);
    }
}

static void double_hold_idle_timeout_callback(void const *argument)
{
    uint8_t hold_type = button_hold_type;
    uint8_t hold_side = g_button_hold_side;

    /*
     * 已被 KEY_UP 或 DOUBLE_CLICK 取消。
     */
    if (hold_type == 0xFF)
    {
        TRACE(0, "[KEYMAP] TIMER ignored, hold state canceled");
        return;
    }

    if (hold_type == CLICK_DOUBLE_HOLD)
    {
        TRACE(0,
              "[KEYMAP] TIMER CLICK_DOUBLE_HOLD side=%s",
              ntt_key_side_str(hold_side));

        handle_key_event(CLICK_DOUBLE_HOLD, hold_side);

        /*
         * handle_key_event() 執行期間可能收到取消事件。
         * 必須重新確認狀態，不能無條件重啟 timer。
         */
        if ((button_hold_type == CLICK_DOUBLE_HOLD) &&
            (double_hold_idle_timer_id != NULL))
        {
            osTimerStart(double_hold_idle_timer_id,
                         DOUBLE_HOLD_IDLE_TIMEOUT_MS);
        }
        else
        {
            TRACE(0,
                  "[KEYMAP] TIMER not restarted, current type=0x%02X",
                  button_hold_type);
        }

        return;
    }

    if (hold_type == CLICK_HOLD_2S)
    {
        TRACE(0, "[KEYMAP] TIMER ignore CLICK_HOLD_2S");
        ntt_cancel_double_hold_state("timer saw hold 2s");
        return;
    }

    TRACE(0,
          "[KEYMAP] TIMER unknown hold type=0x%02X",
          hold_type);

    ntt_cancel_double_hold_state("unknown timer state");
}

void double_hold_idle_detection_init(void)
{
    if (double_hold_idle_timer_id == NULL)
    {
        double_hold_idle_timer_id =
            osTimerCreate(osTimer(DOUBLE_HOLD_IDLE_TIMER),
                          osTimerOnce,
                          NULL);

        if (double_hold_idle_timer_id == NULL)
        {
            TRACE(0, "[KEYMAP] create DOUBLE_HOLD timer failed");
            button_hold_type = 0xFF;
            return;
        }
    }

    osTimerStop(double_hold_idle_timer_id);

    TRACE(0,
          "[KEYMAP] start DOUBLE_HOLD timer type=0x%02X side=%s timeout=%d",
          button_hold_type,
          ntt_key_side_str(g_button_hold_side),
          DOUBLE_HOLD_IDLE_TIMEOUT_MS);

    osTimerStart(double_hold_idle_timer_id,
                 DOUBLE_HOLD_IDLE_TIMEOUT_MS);
}

/************************************************/

void sparraw_tx_key_click_notify_msg(uint8_t kick_type)
{
    uint8_t keyEventNotify[3];
    uint8_t key_side = NTT_KEY_SIDE_RIGHT;

    keyEventNotify[0] = 0xB0;
    keyEventNotify[1] = 0x04;
    keyEventNotify[2] = kick_type;

    key_side = ntt_get_key_side_snapshot();

    TRACE(0,
          "[KEY] kick=%d test=%d left_flag=%d inbox=%d side=%s",
          kick_type,
          enterKeyClickTestMode,
          key_event_is_left,
          er_inbox,
          ntt_key_side_str(key_side));

    if (enterKeyClickTestMode && app_sparraw_env.notifyEnable)
    {
        ble_aiwang_srv_send_data_via_notification(keyEventNotify, 3);
    }

    if ((er_inbox == 1) && (key_event_is_left == 0))
    {
        TRACE(0, "[KEY] ignore key event, earbud in charging case");
        return;
    }

    switch (kick_type)
    {
        case KEY_CLICK:
        {
            ntt_cancel_double_hold_state("single click");

            TRACE(0, "[KEYMAP] CLICK_SINGLE");
            handle_key_event(CLICK_SINGLE, key_side);
        }
        break;

        case KEY_DOUBLE_CLICK:
        {
    #ifdef SUPPORT_SIRI
            if (ntt_voice_assist_is_active())
            {
                TRACE(0,
                    "[KEY] DOUBLE_CLICK -> CLOSE_VOICE_ASSIST");

                ntt_cancel_double_hold_state("double click close voice assist");

                ntt_voice_assist_close();
                break;
            }
    #endif

            ntt_cancel_double_hold_state("normal double click");

            TRACE(0, "[KEYMAP] CLICK_DOUBLE");
            handle_key_event(CLICK_DOUBLE, key_side);
        }
        break;

        case KEY_TRIPLE_CLICK:
        {
            ntt_cancel_double_hold_state("triple click");

            TRACE(0, "[KEYMAP] CLICK_TRIPLE");
            handle_key_event(CLICK_TRIPLE, key_side);
        }
        break;

        case KEY_DOUBLE_HOLD_CLICK:
        {
            TRACE(0, "[KEYMAP] CLICK_DOUBLE_HOLD");

           ntt_cancel_double_hold_state ("start new double hold");

            g_button_hold_side = key_side;
            button_hold_type = CLICK_DOUBLE_HOLD;

            handle_key_event(CLICK_DOUBLE_HOLD, key_side);
            double_hold_idle_detection_init();
        }
        break;

        case KEY_HOLD_CLICK:
        {
            TRACE(0, "[KEYMAP] CLICK_HOLD_2S");

            ntt_cancel_double_hold_state("hold 2s event");

            handle_key_event(CLICK_HOLD_2S, key_side);
        }
        break;

        case KEY_UP:
        {
            TRACE(0, "[KEYMAP] KEY_UP");

            ntt_cancel_double_hold_state("key up");
            aparraw_set_key_event_left(0);
        }
        break;

        default:
        {
            TRACE(0,
                "[KEYMAP] UNKNOWN kick=%d",
                kick_type);
        }
        break;
    }
}

//#if need_send_data_by_notify
extern bool app_spp_tota_send_data(uint8_t *ptrData, uint16_t length);

static void sparraw_tx_msg(uint8_t rsp_type,
                           const uint8_t *data,
                           uint16_t len)
{
    uint8_t rsp_buffer[256] = {0};
    const uint16_t rsp_len = (uint16_t)(len + 3U);

    if ((size_t)rsp_len > sizeof(rsp_buffer))
    {
        TRACE(0, "[SPARROW_TX] overflow len=%u", (unsigned)rsp_len);
        return;
    }

    rsp_buffer[0] = rsp_type;
    rsp_buffer[1] = (uint8_t)((len >> 8) & 0xFF);
    rsp_buffer[2] = (uint8_t)(len & 0xFF);

    if ((data != NULL) && (len > 0))
    {
        memcpy(&rsp_buffer[3], data, len);
    }

    if (g_sparrow_api_transport == SPARROW_API_TRANSPORT_SPP)
    {
        bool ret = false;

        TRACE(0, "[SPARROW_TX][SPP] rsp=0x%02X payload_len=%u total=%u",
              rsp_type,
              (unsigned)len,
              (unsigned)rsp_len);

        DUMP8("%02X ", rsp_buffer, rsp_len);

        ret = app_spp_tota_send_data(rsp_buffer, rsp_len);

        TRACE(0, "[SPARROW_TX][SPP] send ret=%d", ret ? 1 : 0);
        return;
    }

    if (app_sparraw_env.notifyEnable)
    {
        TRACE(0, "[SPARROW_TX][BLE] rsp=0x%02X payload_len=%u total=%u",
              rsp_type,
              (unsigned)len,
              (unsigned)rsp_len);

        ble_aiwang_srv_send_data_via_notification(rsp_buffer, rsp_len);
    }
    else
    {
        TRACE(0, "[SPARROW_TX][BLE] notify disabled rsp=0x%02X len=%u",
              rsp_type,
              (unsigned)rsp_len);
    }
}
//#endif
static void sparraw_read_rsp_msg(uint8_t rsp_type,uint16_t aw_connhdl,uint32_t aw_token, const uint8_t* data, uint16_t len) {
	uint8_t  rsp_buffer[64];
	rsp_buffer[0] = rsp_type;
	if (NULL != data && len > 0)
	{
		memcpy(&rsp_buffer[1], data, len);
	}
	gatts_send_read_rsp(aw_connhdl, aw_token, 0, rsp_buffer, len + 1);
}

static void sparraw_read_error_rsp_msg(uint8_t rsp_type,
                                       uint16_t aw_connhdl,
                                       uint32_t aw_token,
                                       uint8_t error_code)
{
    uint8_t rsp_data[32] = {0};
    const char *detail = ntt_api_error_string(error_code);
    uint16_t detail_len = strlen(detail);
    uint16_t value_len = detail_len + 1;

    rsp_data[0] = (value_len >> 8) & 0xFF;
    rsp_data[1] = value_len & 0xFF;
    rsp_data[2] = error_code;

    memcpy(&rsp_data[3], detail, detail_len);

    TRACE(0,
          "[API_ERR][READ_RSP] rsp=0x%02X err=0x%02X detail=%s",
          rsp_type,
          error_code,
          detail);

    sparraw_read_rsp_msg(rsp_type,
                         aw_connhdl,
                         aw_token,
                         rsp_data,
                         value_len + 2);
}

static void sparraw_rx_cmd_init(void){
	sparraw_rx_state = RX_IDLE;
	rx_param_len = 0;
	memset(&rxDataStruct, 0, sizeof(REQUEST_DATA_STRUCT));
}

POSSIBLY_UNUSED static void sparraw_rx_cmd_handler(void){
	  TRACE(0, "%s cmd_type=0x%02x sub_cmd=0x%02x", __func__, rxDataStruct.cmd_type, payload_buffer[1]);
      sparraw_tx_cmd_data_rsp_ack(rxDataStruct.cmd_type, payload_buffer[1]);
      switch(rxDataStruct.cmd_type) {
      case A0_SETS: {
    	  if (ENTER_FACTORY_MODE == payload_buffer[1]) {
    		  TRACE(0, "ENTER_FACTORY_MODE");
    	  } else if (EXIT_AND_REBOOT == payload_buffer[1]) {
    		  TRACE(0, "EXIT_AND_REBOOT");
    	  } else if (ENTER_SHIP_MODE == payload_buffer[1]) {
    		  TRACE(0, "ENTER_SHIP_MODE");
    	  } else if (FACTORY_RESET == payload_buffer[1]) {
    		  TRACE(0, "FACTORY_RESET");
    	  }
    	  break;
      }
      case B0_SETS:{
    	  if (AUDIO_LOOPBACK == payload_buffer[1]) {
    		  TRACE(0, "AUDIO_LOOPBACK");
    	  } else if (PLAY_TEST_TONE == payload_buffer[1]) {
    		  TRACE(0, "PLAY_TEST_TONE");
    	  } else if (LED_CONTROL == payload_buffer[1]) {
    		  TRACE(0, "LED_CONTROL");
    	  } else if (BUTTON_EVENT == payload_buffer[1]) {
    		  TRACE(0, "BUTTON_EVENT");
    	  }
    	  break;
      }
      case C0_SETS: {
    	  if (GET_FW_VERSION == payload_buffer[1]) {
    		  TRACE(0, "GET_FW_VERSION");
    	  } else if (GET_BATTERY_INFO == payload_buffer[1]) {
    		  TRACE(0, "GET_BATTERY_INFO");
    	  } else if (READ_SN == payload_buffer[1]) {
    		  TRACE(0, "READ_SN");
    	  } else if (WRTIE_SN == payload_buffer[1]) {
    		  TRACE(0, "WRTIE_SN");
    	  }
    	  break;
      }
      case OTA_SETS:
    	  break;
      default:{
    	  sparraw_tx_cmd_data_rsp_neg(1,rxDataStruct.cmd_type, payload_buffer[1]);
    	  break;
      }
    }
}

POSSIBLY_UNUSED static void sparraw_rx_cmd_parse(const uint8_t *data, uint16_t len) {
  uint16_t i = 0;
  for (; i < len; i++) {
	  //A0 00 01 01
	  if ((RX_IDLE == sparraw_rx_state) &&
			  (REQUEST_CMD == data[i] \
			  || A0_SETS == data[i] \
			  || B0_SETS == data[i] \
			  || C0_SETS == data[i] \
			  || OTA_SETS == data[i])) {
		  //TRACE(0,"sparraw cmd_type=0x%02x", data[i]);
		  rxDataStruct.cmd_type = data[i];
		  sparraw_rx_state  = RX_CMD_TYPE;
		  rx_param_len      = 0;
		  payload_buffer[0] = 0;
		  rxDataStruct.params_len = 0;
		  continue;
	  }
	  if ((RX_CMD_TYPE == sparraw_rx_state)) {
		  rxDataStruct.remain_count_packests = data[i];
		  sparraw_rx_state = RX_CMD_PARAM_lEN;
		  //TRACE(0,"sparraw remain_count_packests=0x%02x", data[i]);
		  continue;
	  }
	  if ((RX_CMD_PARAM_lEN == sparraw_rx_state)) {
		  payload_buffer[0] += data[i]; //retain the payload total len
		  sparraw_rx_state = RX_CMD_PAYLOAD;
		  rxDataStruct.params_len = data[i];
		  //TRACE(0,"sparraw RX_CMD_PARAM_LEN=0x%02x", data[i]);
		  if (data[i] > 0) {
		     continue;
		  }
	  }
	  if (RX_CMD_PAYLOAD == sparraw_rx_state) {
		  if (rxDataStruct.params_len > 0) {
			  --rxDataStruct.params_len;
			  if (rx_param_len < sizeof(payload_buffer) -1) payload_buffer[1 + rx_param_len++] = data[i];
		  }
		  TRACE(0,"sparraw RX_CMD_PAYLOAD=0x%02x %d %d", data[i],
				  rxDataStruct.params_len,
				  rxDataStruct.remain_count_packests);
		  if( 0 == rxDataStruct.params_len) { //current packet commands over
              if (rxDataStruct.remain_count_packests > 0) {
            	  --rxDataStruct.remain_count_packests;
              }
              if ( 0 == rxDataStruct.remain_count_packests) {
            	  sparraw_rx_cmd_handler();
            	  sparraw_rx_state = RX_IDLE;
            	  rx_param_len     = 0;
              } else {
            	  sparraw_rx_state = RX_NEXT_SPLIT_PACKET;
              }
		  }
    	  continue;
	  }
	  if ((RX_NEXT_SPLIT_PACKET == sparraw_rx_state) && (data[i] == rxDataStruct.cmd_type)){
		  sparraw_rx_state = RX_CMD_PARAM_lEN;
	  } else {
		  sparraw_rx_state = RX_IDLE;
	  }
   }
}

static void sparraw_handle_set_color_code_cmd(const uint8_t *data, uint16_t len)
{
    uint8_t color_code;

    if ((data == NULL) || (len < 1))
    {
        TRACE(0, "[COLOR_CODE][RX] invalid");
        return;
    }

    /*
     * APP packet:
     * 50 00 08 73 70 61 72 72 6F 77 XX
     *
     * Last byte is color code:
     * Black        0x4B
     * Silver White 0x53
     * Gold         0x4E
     */
    color_code = data[len - 1];

    TRACE(0, "[COLOR_CODE][RX] color=0x%02X", color_code);

    if (!ntt_color_code_is_valid(color_code))
    {
        TRACE(0, "[COLOR_CODE][RX] invalid color=0x%02X", color_code);
        return;
    }

    ntt_color_code_nv_set(color_code);
    ntt_ble_adv_refresh_data();
#ifdef IBRT
	app_ibrt_customif_cmd_sync_color_code(color_code);
#endif

    TRACE(0, "[COLOR_CODE][RX] save done color=0x%02X", color_code);
}

static void sparraw_rx_cmd_parse_v2(const uint8_t *data, uint16_t len)
{
    uint16_t i;

    if ((data == NULL) || (len == 0))
    {
        TRACE(0,
              "[SPARROW_RX] invalid data=%p len=%d",
              data,
              len);
        return;
    }

    TRACE(0,
          "[SPARROW_RX] len=%d cmd=0x%02X",
          len,
          data[0]);

    TRACE(0, "[SPARROW_RX] payload:");
    DUMP8("%02X ", data, len);

	if (data[0] == SET_COLOR_CODE)
    {
        sparraw_handle_set_color_code_cmd(data, len);
        return;
    }

    for (i = 0; i < aiWangCmdTypesCount; i++)
    {
        TRACE(0,
              "[SPARROW_RX] search cmd=0x%02X table=0x%02X idx=%d",
              data[0],
              aiWangCmdTypes[i].cmd,
              i);

        if (data[0] == aiWangCmdTypes[i].cmd)
        {
            TRACE(0,
                  "[SPARROW_RX] MATCH cmd=0x%02X idx=%d handler=%p",
                  data[0],
                  i,
                  aiWangCmdTypes[i].handleFunc);

            if (aiWangCmdTypes[i].handleFunc)
            {
                aiWangCmdTypes[i].handleFunc(data, len);

                TRACE(0,
                      "[SPARROW_RX] DONE cmd=0x%02X",
                      data[0]);
            }
            else
            {
                TRACE(0,
                      "[SPARROW_RX] NULL handler cmd=0x%02X",
                      data[0]);
            }

            break;
        }
    }

    if (i >= aiWangCmdTypesCount)
    {
        TRACE(0,
              "[SPARROW_RX] UNKNOWN cmd=0x%02X len=%d",
              data[0],
              len);

        TRACE(0, "[SPARROW_RX] unknown payload:");
        DUMP8("%02X ", data, len);
    }
}


extern "C" bool sparrow_spp_api_is_cmd(const uint8_t *data, uint16_t len)
{
    uint16_t i;

    if ((data == NULL) || (len == 0))
    {
        return false;
    }

    /* HAL_CMD/SPP EQ tuning packet starts with 7B 00 00 00 7B 00 00 00.
     * Keep it in the original TOTA/HAL path, not Sparrow GATT API bridge.
     */
    if ((len >= 8) &&
        (data[0] == 0x7B) && (data[1] == 0x00) &&
        (data[2] == 0x00) && (data[3] == 0x00) &&
        (data[4] == 0x7B) && (data[5] == 0x00) &&
        (data[6] == 0x00) && (data[7] == 0x00))
    {
        return false;
    }

    if (data[0] == SET_COLOR_CODE)
    {
        return true;
    }

    for (i = 0; i < aiWangCmdTypesCount; i++)
    {
        if (data[0] == aiWangCmdTypes[i].cmd)
        {
            return true;
        }
    }

    return false;
}

extern "C" void sparrow_spp_api_rx_handler(const uint8_t *data, uint16_t len)
{
    if (!sparrow_spp_api_is_cmd(data, len))
    {
        TRACE(0, "[SPARROW_RX][SPP] not api packet len=%d", len);
        return;
    }

    TRACE(0, "[SPARROW_RX][SPP] len=%d cmd=0x%02X", len, data[0]);
    sparrow_api_set_transport(SPARROW_API_TRANSPORT_SPP);
    sparraw_rx_cmd_parse_v2(data, len);
}

static void sparraw_event_handler_thread(void const *argument)
{
	int len = 0;
	SPARRAW_MESSAGE_BLOCK* rx_event = NULL;
	uint8_t     event;
    while (true)
    {
        if (sparraw_event_mailbox_get(&rx_event))
        {
            continue;
        }
        //TRACE(2, "%s ", __func__);
        osMutexWait(app_sparraw_buf_lock, osWaitForever);
		REL_TRACE_NOCRLF(0, "NTII_SRV_RX[%d %d]: ", rx_event->event, rx_event->len);
		DUMP8("%02X ", &rx_event->data[0], rx_event->len);
		event = rx_event->event;
		len = rx_event->len > sizeof(payload_buffer)?sizeof(payload_buffer):rx_event->len;
        memcpy(&payload_buffer[0], &rx_event->data[0],len);
        sparraw_event_mailbox_free(rx_event);
        osMutexRelease(app_sparraw_buf_lock);
#if 0
        if (BLE_AIWANG_SRV_RX == event)
        {
            sparraw_rx_cmd_parse(&rx_event->data[0], rx_event->len);
        }
#else
        if (BLE_AIWANG_SRV_RX == event)
        {
           sparrow_api_set_transport(SPARROW_API_TRANSPORT_BLE);
           sparraw_rx_cmd_parse_v2(&payload_buffer[0], len);
        }
#endif

    }
}


void sparraw_connected(uint8_t conidx, bool enableNotify)
{
    TRACE(0,"%s.", __func__);
    app_sparraw_env.conidx = conidx;
    app_sparraw_env.notifyEnable = enableNotify;
	aw_ntc_detect_volt_timer_onoff(false);
}

void sparraw_disconnected(void)
{
	TRACE(0,"%s.", __func__);
	sparraw_ble_init();
	sparraw_event_mailbox_free_all();
	sparraw_rx_cmd_init();
	aw_ntc_detect_volt_timer_onoff(true);
}

void sparraw_mtu(uint16_t mtu)
{
	TRACE(0,"%s.", __func__);
	app_sparraw_env.Mtu = mtu;
}

void sparraw_event_handle(ble_aiwang_param_u *param)
{
    ASSERT(param,"sparraw data is null. ble_sparrow_task_init=%d", ble_sparrow_task_init);
    TRACE(0,"[%s] ble_sparrow_task_init:%d",__func__, ble_sparrow_task_init);
	DEBUG_WARNING(0,
        "[SPARROW_EVT] enter event=0x%02X conidx=%d len=%d init=%d",
        param->event,
        param->conidx,
        param->len,
        ble_sparrow_task_init);
    switch(param->event)
    {
		case BLE_AIWANG_SRV_CONN:{
			TRACE(0,"[%s] event BLE_AIWANG_SRV_CONN  conidx:%d",   __func__, param->conidx);
			sparraw_connected(param->conidx, param->len >0 ? true :false);
			break;
		}
		case BLE_AIWANG_SRV_DISCONN:{
			TRACE(0,"[%s] event BLE_AIWANG_SRV_DISCONN conidx:%d", __func__, param->conidx);
			sparraw_disconnected();
			break;
		}
		case BLE_AIWANG_SRV_MTU:{
			TRACE(0,"[%s] event BLE_AIWANG_SRV_DISCONN conidx:%d, mtu=%d", __func__, param->conidx,param->len);
			sparraw_mtu(param->len);
			break;
		}
		case BLE_AIWANG_SRV_RX:{
			REL_TRACE_NOCRLF(0, "%s", "BLE_AIWANG_SRV_RX:");
			DUMP8("%02X ", param->data, param->len);
			sparraw_mailbox_put(param->conidx, BLE_AIWANG_SRV_RX, param->data, param->len);
			break;
		}
		case BLE_AIWANG_SRV_SENDDONE:{
			TRACE(0,"[%s] BLE_AIWANG_SRV_SENDDONE %02x", __func__, param->event);
			break;
		}
		default:
			TRACE(0,"[%s]unknown event %02x", __func__, param->event);
			break;
    }
}

void sparraw_event_read_handle(ble_aiwang_read_param_u *param)
{
	uint8_t read_send_data[20] = {0};
	memset(read_send_data,0,sizeof(read_send_data));
    switch(aiwan_read_data)
    {
		case GET_BATTERY_LEVEL:
		{
				uint8_t localBattery;
				uint8_t peerBattery;
				uint8_t boxBattery;
				bool peerValid = false;
				bool twsConnected = false;

				uint8_t leftBattery  = 0;
				uint8_t rightBattery = 0;
				uint8_t batteryArray[3] = {0, 0, 0};

				localBattery = app_battery_current_level();
				peerBattery  = app_ibrt_customif_get_tws_peer_battery_level();
				boxBattery   = getBoxChargerBattery();
				twsConnected = bts_tws_if_is_tws_link_connected();
                peerValid = twsConnected && (peerBattery != 0xFF) && (peerBattery <= 100);
                if (!peerValid)
                {
                    peerBattery = 0xFF;
                }

                leftBattery  = 0xFF;
                rightBattery = 0xFF;

				TRACE(0, "[BAT32][READ_REQ] GET_BATTERY_LEVEL");
				TRACE(0,
					"[BAT32][SRC] local=%d peer=%d peerValid=%d tws=%d box=%d",
					localBattery,
					peerBattery,
					peerValid,
					twsConnected,
					boxBattery);

				if (app_ibrt_if_is_right_side())
				{
                        rightBattery = localBattery;
                        leftBattery  = peerBattery;

						TRACE(0,
							"[BAT32][ROLE] RIGHT local=%d tws_peer=%d valid=%d",
							localBattery,
							peerBattery,
							peerValid);
				}
				else
				{
                            leftBattery  = localBattery;
                            rightBattery = peerBattery;

						TRACE(0,
							"[BAT32][ROLE] LEFT local=%d tws_peer=%d valid=%d",
							localBattery,
							peerBattery,
							peerValid);
				}


				batteryArray[0] = leftBattery;
				batteryArray[1] = rightBattery;
				batteryArray[2] = boxBattery;

				TRACE(0,
					"[BAT32][READ_RSP] L=%d R=%d C=%d",
					batteryArray[0],
					batteryArray[1],
					batteryArray[2]);

				read_send_data[0] = 0x00;
				read_send_data[1] = 3;
				memcpy(&read_send_data[2], batteryArray, 3);

				TRACE(0, "[BAT32][READ_PAYLOAD] rsp_cmd=0x32 len=%d:", 5);
				DUMP8("%02X ", read_send_data, 5);

				sparraw_read_rsp_msg(RSP_GET_BATTERY_LEVEL,
									param->aw_connhdl,
									param->aw_token,
									read_send_data,
									3 + 2);
				break;
		}
		case GET_DEVICE_NAME:{
			uint8_t* localname =  factory_section_get_bt_name();
		    if(localname)
		    {
#if 1
		    	char nameBuffer[60+1] = {0};
				nameBuffer[1] = strlen((const char *)localname);
				uint16_t name_len = nameBuffer[1] > 60?60:nameBuffer[1];
				memcpy(&nameBuffer[2],localname,name_len);
				sparraw_read_rsp_msg(RSP_GET_DEVICE_NAME, param->aw_connhdl,param->aw_token,(const uint8_t*)nameBuffer, name_len+2);
#else
		    	read_send_data[1] = strlen((const char *)localname);
				memcpy(&read_send_data[2],localname,strlen((const char *)localname));
		    	//sparraw_read_rsp_msg(RSP_GET_DEVICE_NAME, param->aw_connhdl,param->aw_token,(const uint8_t*)localname, strlen((const char *)localname)+1);
		    	sparraw_read_rsp_msg(RSP_GET_DEVICE_NAME, param->aw_connhdl,param->aw_token,(const uint8_t*)read_send_data, strlen((const char *)localname)+1+2);
#endif
		    }
		
			break;
		}
		case SET_DEVICE_NAME:{
			read_send_data[1] = 0;
			sparraw_read_rsp_msg(RSP_SET_DEVICE_NAME,param->aw_connhdl,param->aw_token, read_send_data, 0+2);
			break;
		}
		case GET_KEY_MAPPING:
		{
			uint8_t read_key_map_data[50] = {0};
			uint8_t key_number = 0;

			handleGetKeyMapNumber(&key_number);

			if (key_number > 20)
			{
				sparraw_read_error_rsp_msg(0x3F,
										param->aw_connhdl,
										param->aw_token,
										API_ERR_STORAGE_ERROR);
				break;
			}

			if (key_number == 0)
			{
				key_number = 20;

				read_key_map_data[0] = 0x00;
				read_key_map_data[1] = key_number * 2;

				for (int i = 0; i < key_number; i++)
				{
					read_key_map_data[i * 2 + 2] = s_key_default_map[i].actions;
					read_key_map_data[i * 2 + 3] = s_key_default_map[i].function;
				}
			}
			else
			{
				read_key_map_data[0] = 0x00;
				read_key_map_data[1] = key_number * 2;

				for (int i = 0; i < key_number; i++)
				{
					uint8_t action = 0;
					uint8_t func = 0;

					handleGetKeyMapActionAndFunc(i, &action, &func);

					read_key_map_data[i * 2 + 2] = action;
					read_key_map_data[i * 2 + 3] = func;
				}
			}

			uint8_t data_len = key_number * 2 + 2;

			sparraw_read_rsp_msg(RSP_GET_KEY_MAPPING,
								param->aw_connhdl,
								param->aw_token,
								read_key_map_data,
								data_len);
			break;
		}
		case SET_KEY_MAPPING:{
			//const uint8_t keyMaps[2] = {0x00,0x14};
			
			sparraw_read_rsp_msg(RSP_SET_KEY_MAPPING,param->aw_connhdl,param->aw_token, read_send_data, 0+2);
			
			break;
		}
		case GET_EQ_PRESET:{
			uint8_t index = 0;
			handleGetEqIndex(&index);
			read_send_data[1] = 1;
			read_send_data[2] = index;
			sparraw_read_rsp_msg(RSP_GET_EQ_PRESET,param->aw_connhdl,param->aw_token, read_send_data, 1+2);
			
			break;
		}
		case SET_EQ_PRESET:{
			
			sparraw_read_rsp_msg(RSP_SET_EQ_PRESET, param->aw_connhdl,param->aw_token,read_send_data, 0+2);			
			break;
		}
		case GET_FW_VERSION:{
			uint8_t version[11+11+1] = {0};
			memcpy(&version[0], DISPLAY_EARBUDS_VERSION, strlen(DISPLAY_EARBUDS_VERSION));
			//aiWangGetChargerBoxVersion(&version[strlen(DISPLAY_EARBUDS_VERSION)]);
			TRACE(0, "GET_FW_VERSION");
    		TRACE(0, "DISPLAY_EARBUDS_VERSION=%s", version);
			read_send_data[1] = strlen((char*)version);
			memcpy(&read_send_data[2],version,strlen((char*)version));
			TRACE(0, "FW Version Len=%d", read_send_data[1]);
    		TRACE(0, "FW Version Send=%s", &read_send_data[2]);
			sparraw_read_rsp_msg(RSP_GET_FW_VERSION, param->aw_connhdl,param->aw_token,read_send_data, strlen((char*)version)+2);		
			break;
		}
		case SET_COLOR_CODE:
		{
			uint8_t color_code = NTT_COLOR_CODE_DEFAULT;

			/*
			* Current API command:
			* 50 00 08 73 70 61 72 72 6F 77 04
			*
			* Since sparraw_event_read_handle() currently has no RX buffer,
			* temporarily map old APP color index:
			*   0x04 -> Black 0x4B
			*
			* Later APP v3.7 should send direct color code:
			*   Black        0x4B
			*   Silver White 0x53
			*   Gold         0x4E
			*/
			color_code = NTT_COLOR_CODE_BLACK;

			if (!ntt_color_code_is_valid(color_code))
			{
				TRACE(0, "[COLOR_CODE][SET] invalid color=0x%02X", color_code);
				color_code = NTT_COLOR_CODE_DEFAULT;
			}

			ntt_color_code_nv_set(color_code);
			ntt_ble_adv_refresh_data();

			TRACE(0, "[COLOR_CODE][SET] save color=0x%02X", color_code);

			read_send_data[0] = 0x00;
			read_send_data[1] = 1;
			read_send_data[2] = color_code;

			sparraw_read_rsp_msg(SET_COLOR_CODE,
								param->aw_connhdl,
								param->aw_token,
								read_send_data,
								3);
			break;
		}
		case FACTORY_COMMAND_SYS:{
			sparraw_read_rsp_msg(0x00, param->aw_connhdl,param->aw_token,(const uint8_t*)"", 0);			
			break;
		}
		case FACTORY_COMMAND_AUDIO_IO:{
			sparraw_read_rsp_msg(0x00, param->aw_connhdl,param->aw_token,(const uint8_t*)"", 0);			
			break;
		}
		case FACTORY_COMMAND_INFO:{
			uint8_t  tempSn[12+1] = {0};
			aiWangGetSn(tempSn, 12);
			sparraw_read_rsp_msg(0x00, param->aw_connhdl,param->aw_token,tempSn, strlen((const char*)tempSn));			
			break;
		}
		default:
			break;
    }
}

void set_er_inbox_status(uint8_t status)
{
	er_inbox = status;
}

extern "C" uint8_t get_er_inbox_status(void)
{
    return er_inbox;
}

void sparraw_rx_thread_init(void)
{
    TRACE(0,"[%s] %d ",__func__, sizeof(aiWangCmdTypes)/sizeof(aiWangCmdTypes[0]));
    //InitCQueue(&sparraw_rx_cqueue, SPARRAW_EVENT_BUF_SIZE, ( CQItemType * )sparraw_tx_buf);
    if (sparraw_event_mailbox_init() != 0) {
        TRACE(1, "Failed to initialize sparraw event mailbox\n");
        return;
    }
    app_sparraw_buf_lock = osMutexCreate(osMutex(app_sparraw_buf_lock));
    if (app_sparraw_buf_lock == NULL) {
        TRACE(1, "Failed to Create app_sparraw_buf_lock\n");
        return;
    }
    sparrow_thread_id = osThreadCreate(osThread(sparraw_event_handler_thread), NULL);
}

void sparraw_service_init(void)
{
	TRACE(0,"[%s]",__func__);
	// ble_aiwang_srv_init();
	if (ble_sparrow_task_init) {
		TRACE(0,"[%s] has init done!!\n",__func__);
		return;
	}
    //sparraw_rx_thread_init();
	ble_aiwang_srv_register_event_cb(sparraw_event_handle);
	ble_aiwang_srv_set_read_data_cb(sparraw_event_read_handle);
	ble_sparrow_task_init = true;
	keymap_load_config();
	app_ibrt_customifsetbuttonmap_cb(keymap_load_config);
    //start charger_manager_thread
    //previous in apps_init,prior settings ICP1205_ADS
    //charger_manager_start();
}


