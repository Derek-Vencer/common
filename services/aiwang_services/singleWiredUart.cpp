/*
 * singleWiredUart.c
 *
 *  Created on: 2026年3月5日
 *      Author: liugenlin
 */
#include <stdint.h>
#include <stddef.h>
#include "tgt_hardware.h"
#include "hal_trace.h"
#include "string.h"
#include "cmsis_os.h"
#include "cmsis.h"
#include "cqueue.h"
#include "apps.h"
#include "string.h"
#include "bluetooth_bt_api.h"
#include "bluetooth_ble_api.h"
#include "app_bt_func.h"
#include "app_factory.h"
#include "factory_section.h"
#include "app_battery.h"
#include "apps.h"
#include "app_media_player.h"
#include "bt_common_define.h"
#include "audio_cfg.h"
#include "btapp.h"

#ifdef IBRT
#include "app_ibrt_internal.h"
#include "earbud_ux_api.h"
#include "app_tws_ibrt_cmd_handler.h"
#include "bts_core_if.h"
#endif

#if defined(IBRT)
#include "app_ibrt_internal.h"
#include "app_ibrt_customif_ui.h"
#include "app_ibrt_voice_report.h"
#include "bts_core_if.h"
#include "bts_tws_if.h"
#if defined(IBRT_UI)
#include "earbud_ux_api.h"
#include "app_tws_ibrt_ui_test.h"
#include "app_ibrt_tws_ext_cmd.h"
#include "app_ibrt_auto_test.h"
#endif
#endif

#include "nvrecord_bt.h"
#include "nvrecord_env.h"
#include "ICP1205.h"
#include "app_thread.h"
#include "communication_svr.h"
#include "earbud_ux_duplicate_api.h"
#include "bts_tws_if.h"
#include "app_ui_api.h"
#include "app_ui_param_config.h"
#include "app_bt.h"
#include "bts_bt_conn.h"
#include "ota_spp.h"
#include "hal_bootmode.h"
#include "pmu.h"

#ifdef IBRT
#include "app_ibrt_customif_cmd.h"
#endif

#include "hal_sleep.h"

// 串口空闲定时器
static osTimerId uart_idle_timer_id = NULL;
// 空闲超时时间（5秒）
#define UART_IDLE_TIMEOUT_MS 2000
// 空闲超时回调函数
static void uart_idle_timeout_callback(void const *argument);
void uart_idle_detection_init(void);
uint8_t getPeerBattery(void);
static uint8_t g_case_state = 1;   // 1=open, 0=close
extern void app_ibrt_customif_cmd_sync_battery_level(uint8_t current_level);
extern uint8_t app_ibrt_customif_get_tws_peer_battery_level(void);
extern uint8_t app_ibrt_customif_get_tws_peer_box_battery_level(void);
extern void earBudsCloseOff_PowerOff_StartTimer(void);
extern bool ntt_manual_pairing_mode;
// 定时器定义
osTimerDef(UART_IDLE_TIMER, uart_idle_timeout_callback);
extern "C" void app_ibrt_if_init_open_box_state_for_evb(void);
bool ntt_case_open_pending = false;
//#define DISABLE_GOC_UART_LOG
static void wired_uart_remove_all_phone_paired_list(void);
extern void handleSetEqIndex(uint8_t index);
extern "C" void ntt_audio_output_mute_refresh(void);
bool ntt_open_case_idle_reboot_needed = false;
//static void box_battery_nv_reset(void);
bool ntt_first_no_mobile_pair_mode = false;
extern void app_ibrt_start_power_on_tws_pairing(void);
extern void ntt_case_open_reconnect_mobile_start(void);
extern "C" void ntt_case_state_sync_local_update(bool in_case);
static bool aiWang_disconnect_second_phone_for_pairing(void);

#define NTT_BOX_BATTERY_CASE_INTERVAL_MS 5000

static uint32_t ntt_last_box_battery_case_tick = 0;

#ifdef DISABLE_GOC_UART_LOG

#undef printf
#define printf(...)

#undef DBGPRINT
#define DBGPRINT(...)

#else

#undef printf
#define printf(fmt, ...) \
    hal_trace_printf(0, "[goc-uart] " fmt, ##__VA_ARGS__)

#undef DBGPRINT
#define DBGPRINT(fmt, ...) \
    hal_trace_printf(2, "[goc-uart] " fmt, ##__VA_ARGS__)

#endif

//format
// 55 aa  cmd  chanl_index  00 data
#define 		FRAME_HEADER_1						0x55
#define 		FRAME_HEADER_2						0xAA

//Transparent command: CMID
////////////GET////////////////////////
#define 		CMD_CASE_STATE							0x00
#define			CMD_HANDSHAKE   	  		            0x01		  //Handshake instruction
#define			CMD_GET_STATE   						0x02		  //Get headphone status
#define			CMD_GET_MAC  	  		    		    0x03		  //Obtain the MAC address of the earphones
#define   	    CMD_GET_EAR_POWER						0x04		  //Get headphone battery level

////////////SET/////////////////////
#define 		CMD_NONE_CASE						    0x05
#define			CMD_POWER_OFF    						0x06		  //Headphones enter shipping
#define			CMD_OPEN_CASE   						0x07
#define			CMD_CLOSE_CASE   						0x08
#define 		CMD_EAR_RESET						    0x09
#define			CMD_SET_MAC  	  		    		    0x0A		  //Set headphone MAC address
#define			CMD_PEER_EAR						    0x0B
#define 		CMD_VOICE_INDICAT						0x0C		  //Switch voice prompts
#define         CMD_SET_EARBUD_SHIP_MOD                 0x0D
#define         CMD_SET_EARBUD_ENTER_PAIR               0x0E
#define         CMD_SEND_BOX_BATTERY_LEVEL              0x0F
#define         CMD_SEND_DUT_MODE                       0x10
#define         CMD_SEND_EAR_PUTIN                      0x11
#define         CMD_SEND_DTM_MODE                       0x12

typedef enum {
	PARSE_IDLE,
	PARSE_HEAD,
	PARSE_CMD,
	PARSE_PAYLOAD,
	PARSE_CRC
}PARSE_STATE;

//static enum PARSE_STATE parSeState = PARSE_IDLE;

typedef struct {
	uint8_t getBatteryOK;
	uint8_t leftEarBudsBattery;
	uint8_t rightEarBudsBattery;
	uint8_t boxChargerBattery;
	uint8_t boxLidsStates;
	uint8_t boxSoftVersion[16+1];
	uint8_t boxIsOpen;
    bool    needOpenEarbuds;
} BOX_STATUS;

extern void app_tws_ibrt_update_info(ibrt_role_e ibrtRole,bt_bdaddr_t *ibrtPeerAddr);

extern void aiWangSetBoxVersion(uint8_t *data, uint8_t len);

extern void earBudsCloseOff_PogonIn_StartTimer(void);
extern void earBudsCloseOff_PogonIn_StopTimer(void);

#define LEFT_BUDS  0
#define RIGHT_BUDS 1
#define MAX_RX_SIZE  64
static uint8_t  isRightEarbuds = 0xFF; //unknown odd  right , even left
static uint8_t  wiredUartInitFlag  = FALSE;
static uint8_t  wiredUartReceiveData[MAX_RX_SIZE];
static BOX_STATUS  boxChargerStatus  ;
static bool box_battery_cache_valid = false;
static bool box_battery_update_enable = true;

static uint8_t uart_rx_handle_data_count = 0;
/************************************************/
/************************************************/

// Pogo Pin 状态
typedef enum {
    POGO_PIN_STATE_UNKNOWN = 0,
    POGO_PIN_STATE_INSERTED,
    POGO_PIN_STATE_REMOVED
} pogo_pin_state_t;

// 全局状态变量
static pogo_pin_state_t current_pogo_state = POGO_PIN_STATE_UNKNOWN;
static osThreadId pogo_monitor_thread_id = NULL;
static bool pogo_monitor_running = false;

void set_er_inbox_status(uint8_t status);

/**
 * @brief 初始化 Pogo Pin 检测引脚
 * @return true - 初始化成功，false - 初始化失败
 */
bool pogo_pin_init(void)
{
    // 检查配置是否有效
    if (app_battery_ext_charger_detecter_cfg.pin == HAL_IOMUX_PIN_NUM) {
        DBGPRINT("[POGO] Pogo Pin detection not configured");
        return false;
    }
    
    // 初始化引脚功能
    hal_iomux_init((struct HAL_IOMUX_PIN_FUNCTION_MAP *)&app_battery_ext_charger_detecter_cfg, 1);
    
    // 设置为输入模式
    hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)app_battery_ext_charger_detecter_cfg.pin, HAL_GPIO_DIR_IN, 1);
    
    DBGPRINT("[POGO] Pogo Pin detection initialized successfully");
    return true;
}

/**
 * @brief 获取 Pogo Pin 状态（带去抖）
 * @param debounce_ms 去抖时间（毫秒）
 * @return POGO_PIN_STATE_INSERTED - Pogo Pin 插入
 *         POGO_PIN_STATE_REMOVED - Pogo Pin 未插入
 *         POGO_PIN_STATE_UNKNOWN - 状态未知
 */
pogo_pin_state_t get_pogo_pin_state(uint32_t debounce_ms)
{
    if (app_battery_ext_charger_detecter_cfg.pin == HAL_IOMUX_PIN_NUM) {
        return POGO_PIN_STATE_UNKNOWN;
    }
    
    // 第一次读取
    uint8_t first_state = hal_gpio_pin_get_val((enum HAL_GPIO_PIN_T)app_battery_ext_charger_detecter_cfg.pin);
    
    // 延时去抖
    osDelay(debounce_ms);
    
    // 第二次读取
    uint8_t second_state = hal_gpio_pin_get_val((enum HAL_GPIO_PIN_T)app_battery_ext_charger_detecter_cfg.pin);
    
    // 两次状态一致才确认
    if (first_state == second_state) {
        // 根据硬件设计调整逻辑，这里假设低电平表示插入
        return (first_state == 0) ? POGO_PIN_STATE_INSERTED : POGO_PIN_STATE_REMOVED;
    }
    
    return POGO_PIN_STATE_UNKNOWN;
}

/**
 * @brief Pogo Pin 状态监控线程
 * @param argument 线程参数（未使用）
 */
static void pogo_pin_monitor_thread(void const *argument)
{
    DBGPRINT("[POGO] Pogo Pin monitor thread started");
    pogo_monitor_running = true;
    
    while (pogo_monitor_running) {
        // 每500毫秒检测一次
        osDelay(500);
#if 0        
        // 获取 Pogo Pin 状态（10ms 去抖）
        pogo_pin_state_t new_state = get_pogo_pin_state(10);
        
        // 状态变化时输出日志
        if (new_state != current_pogo_state) {
            current_pogo_state = new_state;
            
            switch (current_pogo_state) {
                case POGO_PIN_STATE_INSERTED:
                    DBGPRINT("[POGO] Pogo Pin inserted - Earbud in case");
                    // 执行插入时的操作
                    // 例如：启动充电、进入低功耗模式等
                    break;
                    
                case POGO_PIN_STATE_REMOVED:
                    DBGPRINT("[POGO] Pogo Pin removed - Earbud out of case");
                    // 执行移除时的操作
                    // 例如：停止充电、开始蓝牙广播等
                    break;
                    
                case POGO_PIN_STATE_UNKNOWN:
                    DBGPRINT("[POGO] Pogo Pin state unknown");
                    break;
            }
        }

#else
		if(POGO_PIN_STATE_INSERTED != current_pogo_state) {
            current_pogo_state = POGO_PIN_STATE_INSERTED;
		}
		uart_rx_handle_data_count ++;
		if(uart_rx_handle_data_count >= 10)
		{
			DBGPRINT("@@@@@@@@@pogo out stop");
			
			communication_stop();
			hal_sleep_start_stats(10000, 10000);
			uart_rx_handle_data_count = 0;
			break;
			
		}
#endif
    }
	osDelay(200);
	//app_bt_ME_ControlSleepMode(1);
#if 1
	// 重置蓝牙连接参数，确保进入低功耗模式
        struct BT_DEVICE_T *device = app_bt_get_device(BT_DEVICE_ID_1);
        if (device && device->acl_is_connected) {
            // 先禁用所有策略
            app_bt_Me_SetLinkPolicy(device->acl_conn_hdl, BTIF_BLP_DISABLE_ALL);
            osDelay(50);
            // 再启用Sniff模式
            app_bt_Me_SetLinkPolicy(device->acl_conn_hdl, BTIF_BLP_SNIFF_MODE);
        }

#else
	app_bt_ME_ControlSleepMode(0);
	osDelay(200);
	app_bt_ME_ControlSleepMode(1);

#endif
	
    DBGPRINT("[POGO] Pogo Pin monitor thread exited");
}

// 线程定义
osThreadDef(pogo_pin_monitor_thread, osPriorityNormal, 1, 1024, "pogo_monitor");

/**
 * @brief 启动 Pogo Pin 监控
 * @return true - 启动成功，false - 启动失败
 */
bool start_pogo_pin_monitor(void)
{
    // 初始化 Pogo Pin 检测
    if(0){
	    if (!pogo_pin_init()) {
	        return false;
	    }
	}
    
    // 创建监控线程
    pogo_monitor_thread_id = osThreadCreate(osThread(pogo_pin_monitor_thread), NULL);
    if (pogo_monitor_thread_id == NULL) {
        DBGPRINT("[POGO] Failed to create Pogo Pin monitor thread");
        return false;
    }
    
    DBGPRINT("[POGO] Pogo Pin monitor started");
    return true;
}

/**
 * @brief 停止 Pogo Pin 监控
 */
void stop_pogo_pin_monitor(void)
{
    if (pogo_monitor_running) {
        pogo_monitor_running = false;
        
        if (pogo_monitor_thread_id != NULL) {
            osThreadTerminate(pogo_monitor_thread_id);
            pogo_monitor_thread_id = NULL;
        }
        
        DBGPRINT("[POGO] Pogo Pin monitor stopped");
    }
}

/************************************************/


static const uint8_t crc8_table[256] =
{
	0x00,0x07,0x0E,0x09,0x1C,0x1B,0x12,0x15,
	0x38,0x3F,0x36,0x31,0x24,0x23,0x2A,0x2D,
	0x70,0x77,0x7E,0x79,0x6C,0x6B,0x62,0x65,
	0x48,0x4F,0x46,0x41,0x54,0x53,0x5A,0x5D,
	0xE0,0xE7,0xEE,0xE9,0xFC,0xFB,0xF2,0xF5,
	0xD8,0xDF,0xD6,0xD1,0xC4,0xC3,0xCA,0xCD,
	0x90,0x97,0x9E,0x99,0x8C,0x8B,0x82,0x85,
	0xA8,0xAF,0xA6,0xA1,0xB4,0xB3,0xBA,0xBD,
	0xC7,0xC0,0xC9,0xCE,0xDB,0xDC,0xD5,0xD2,
	0xFF,0xF8,0xF1,0xF6,0xE3,0xE4,0xED,0xEA,
	0xB7,0xB0,0xB9,0xBE,0xAB,0xAC,0xA5,0xA2,
	0x8F,0x88,0x81,0x86,0x93,0x94,0x9D,0x9A,
	0x27,0x20,0x29,0x2E,0x3B,0x3C,0x35,0x32,
	0x1F,0x18,0x11,0x16,0x03,0x04,0x0D,0x0A,
	0x57,0x50,0x59,0x5E,0x4B,0x4C,0x45,0x42,
	0x6F,0x68,0x61,0x66,0x73,0x74,0x7D,0x7A,
	0x89,0x8E,0x87,0x80,0x95,0x92,0x9B,0x9C,
	0xB1,0xB6,0xBF,0xB8,0xAD,0xAA,0xA3,0xA4,
	0xF9,0xFE,0xF7,0xF0,0xE5,0xE2,0xEB,0xEC,
	0xC1,0xC6,0xCF,0xC8,0xDD,0xDA,0xD3,0xD4,
	0x69,0x6E,0x67,0x60,0x75,0x72,0x7B,0x7C,
	0x51,0x56,0x5F,0x58,0x4D,0x4A,0x43,0x44,
	0x19,0x1E,0x17,0x10,0x05,0x02,0x0B,0x0C,
	0x21,0x26,0x2F,0x28,0x3D,0x3A,0x33,0x34,
	0x4E,0x49,0x40,0x47,0x52,0x55,0x5C,0x5B,
	0x76,0x71,0x78,0x7F,0x6A,0x6D,0x64,0x63,
	0x3E,0x39,0x30,0x37,0x22,0x25,0x2C,0x2B,
	0x06,0x01,0x08,0x0F,0x1A,0x1D,0x14,0x13,
	0xAE,0xA9,0xA0,0xA7,0xB2,0xB5,0xBC,0xBB,
	0x96,0x91,0x98,0x9F,0x8A,0x8D,0x84,0x83,
	0xDE,0xD9,0xD0,0xD7,0xC2,0xC5,0xCC,0xCB,
	0xE6,0xE1,0xE8,0xEF,0xFA,0xFD,0xF4,0xF3
};


uint8_t crc8(const uint8_t *data, uint32_t length)
{
    uint8_t crc = 0x00;
    while (length--)
    {
        crc = crc8_table[crc ^ *data];
        data++;
    }
    return crc;
}

static uint8_t g_tws_peer_battery_percent = 0xFF;

void set_tws_peer_battery_percent(uint8_t battery)
{
    if (battery <= 100)
    {
        g_tws_peer_battery_percent = battery;
    }
}

uint8_t get_tws_peer_battery_percent(void)
{
    return g_tws_peer_battery_percent;
}

static uint8_t enter_pair = 0;
static uint8_t enter_pair_count = 0;

static uint8_t pair_status = 0;
static void wired_uart_get_battery_level(void)
{
    uint8_t buff[5] = {0};
    int8_t raw_level = 0;
    uint8_t report_level = 0;

    raw_level = app_battery_current_level();

#if defined(IBRT)
    static int8_t s_last_tws_connected = -1;

    bool tws_connected = bts_tws_if_is_tws_link_connected();

    if (s_last_tws_connected != tws_connected)
    {

        s_last_tws_connected = tws_connected;
    }


    app_ibrt_customif_cmd_sync_battery_level(raw_level);
#endif

    if (raw_level < 0)
    {
        report_level = 0;
    }
    else if (raw_level >= 90)
    {
        report_level = 9;
    }
    else
    {
        report_level = raw_level / 10;
    }

    buff[0] = 0x55;
    buff[1] = 0xAA;
    buff[2] = report_level;

    if (1)
    {
        pair_status = get_pair_status();
        buff[3] = pair_status;
        DBGPRINT("[PHONE_CONNECTED][BOX_BAT] wired_uart_get_battery_level pair_status =%d",pair_status);
        if (enter_pair_count > 1)
        {
            enter_pair = 0;
            set_pair_status(0);
            enter_pair_count = 0;
        }
        else
        {
            enter_pair_count++;
        }
    }
    else
    {
        buff[3] = 0;
        enter_pair_count = 0;
    }

    buff[4] = crc8(buff, 4);

    DBGPRINT("[EAR_POWER][UART_TX][%s] raw=%d report=%u pair=%u crc=0x%02X",
            isRightEarbuds ? "RIGHT" : "LEFT",
            raw_level,
            report_level,
            pair_status,
            buff[4]);

    DUMP8("[EAR_POWER][UART_TX_RAW] ", buff, sizeof(buff));

    communication_send_buf(buff, 5);
}

extern "C" void wired_uart_mobile_connected_get_box_battery(void)
{
    set_pair_status(1);
    //if (operateLeftOrRight != isRightEarbuds)
    //{
    //    DBGPRINT("[PHONE_CONNECTED][BOX_BAT] skip, not right earbuds");
    //    return;
    //}

    DBGPRINT("[PHONE_CONNECTED][BOX_BAT] read case battery");

    wired_uart_get_battery_level();
}

static void wired_uart_get_ear_addr_handle(void)
{
   DBGPRINT("%s.", __func__);
   uint8_t *bt_local_addr = NULL;
   uint8_t buff[9] = {0};
   buff[0] = 0x55;
   buff[1] = 0xAA;
   bt_local_addr = (uint8_t *)bt_get_local_address();
   isRightEarbuds = bt_local_addr[0]&0x01?RIGHT_BUDS:LEFT_BUDS;
   memcpy(&buff[2], bt_local_addr, 6);
   REL_TRACE_NOCRLF(0, "GetLocalBtAddress: ");
   DUMP8("%02X ", bt_local_addr, 6);
   buff[8] = crc8(buff,8);
   communication_send_buf(buff, 9);
}

static void wired_uart_send_cmd_ack_ok(void)
{
   DBGPRINT("%s.", __func__);
   uint8_t buff[5] = {0};
   buff[0] = 0x55;
   buff[1] = 0xAA;
   buff[2] = 'O';
   buff[3] = 'K';
   buff[4] = crc8(buff,4);
   communication_send_buf(buff, 5);
}

static void wired_uart_set_peer_address(uint8_t *data ,uint8_t len)
{
	DBGPRINT("%s.", __func__);
	uint8_t *bt_local_addr = NULL;
	bt_bdaddr_t peerAddress;
	bt_local_addr = (uint8_t *)bt_get_local_address();
	if ( len >= 6 ) {
		  REL_TRACE_NOCRLF(0, "localAddress: ");
		  DUMP8("%02X ", bt_local_addr, 6);
		  REL_TRACE_NOCRLF(0, "setPeerAddress: ");
		  DUMP8("%02X ", &data[0], 6);
		  if ( memcmp(&data[0],bt_local_addr,6))
		  if(bt_local_addr[5] == data[5] && bt_local_addr[4] == data[4] && bt_local_addr[3] == data[3])
		  {
			  DBGPRINT("%s ok", __func__);
			  memcpy(&peerAddress.address[0], &data[0], 6);
			  //app_tws_ibrt_update_info(IBRT_MASTER, &peerAddress);
			  nv_record_update_ibrt_info(data[0]&0x01?IBRT_SLAVE:IBRT_MASTER, &peerAddress);
			  wired_uart_send_cmd_ack_ok();
			  osDelay(30);
			  communication_stop();
			  osDelay(30);
			  (void)app_reset();
		  }
	}
}

static void wired_uart_remove_all_phone_paired_list(void)
{
    bt_status_t retStatus;
    btif_device_record_t record;
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    int paired_dev_count = nv_record_get_paired_dev_count();
    uint8_t empty_addr[BTIF_BD_ADDR_SIZE] = {0};

    DBGPRINT("%s.", __func__);

    if (p_ibrt_ctrl == NULL)
    {
        DBGPRINT("[PAIR_CLR] ibrt ctrl is NULL");
        return;
    }

    DBGPRINT("Master addr:");
    DUMP8("%02x ", p_ibrt_ctrl->local_addr.address, BTIF_BD_ADDR_SIZE);

    DBGPRINT("Slave addr:");
    DUMP8("%02x ", p_ibrt_ctrl->peer_addr.address, BTIF_BD_ADDR_SIZE);

    for (int32_t index = paired_dev_count - 1; index >= 0; index--)
    {
        retStatus = nv_record_enum_dev_records(index, &record);
        if (BT_STS_SUCCESS != retStatus)
        {
            DBGPRINT("[PAIR_CLR] enum index=%d failed ret=%d", index, retStatus);
            continue;
        }

        DBGPRINT("[PAIR_CLR] check index=%d:", index);
        DUMP8("%02x ", record.bdAddr.address, BTIF_BD_ADDR_SIZE);

        if (!memcmp(record.bdAddr.address, p_ibrt_ctrl->local_addr.address, BTIF_BD_ADDR_SIZE) ||
            !memcmp(record.bdAddr.address, p_ibrt_ctrl->peer_addr.address, BTIF_BD_ADDR_SIZE) ||
            !memcmp(record.bdAddr.address, empty_addr, BTIF_BD_ADDR_SIZE))
        {
            DBGPRINT("[PAIR_CLR] skip TWS/local/empty record");
            continue;
        }

        DBGPRINT("[PAIR_CLR] delete phone record index=%d", index);
        nv_record_ddbrec_delete(&record.bdAddr);
    }

    nv_record_flash_flush();
}

static void wired_uart_factory_reset_app_nv(void)
{
    struct nvrecord_env_t *nvrecord_env = NULL;

    static const uint8_t default_action[] =
    {
        0x10, 0x11, 0x12, 0x13, 0x14,
        0x20, 0x21, 0x22, 0x23, 0x24,
        0x50, 0x51, 0x52, 0x53, 0x54,
        0x60, 0x61, 0x62, 0x63, 0x64,
    };

    static const uint8_t default_func[] =
    {
        0x03, 0x04, 0x05, 0x07, 0x08,
        0x03, 0x04, 0x05, 0x06, 0x08,
        0x01, 0x02, 0x00, 0x07, 0x00,
        0x01, 0x02, 0x00, 0x06, 0x00,
    };

    nv_record_env_get(&nvrecord_env);

    if (nvrecord_env == NULL)
    {
        DBGPRINT("[FACTORY_RESET] app nv env null");
        return;
    }

    nvrecord_env->eq_index_data = 0;
    nvrecord_env->key_map_number = sizeof(default_action);

    for (uint8_t i = 0; i < sizeof(default_action); i++)
    {
        nvrecord_env->key_map_action[i] = default_action[i];
        nvrecord_env->key_map_func[i] = default_func[i];
    }

    nv_record_env_set(nvrecord_env);

    DBGPRINT("[FACTORY_RESET] EQ index reset to 0");
    DBGPRINT("[FACTORY_RESET] key mapping reset count=%d", nvrecord_env->key_map_number);
}

#define BOX_BATTERY_INVALID 0xFF

static uint8_t box_battery_nv_cache = BOX_BATTERY_INVALID;
static bool box_battery_nv_loaded = false;

static void box_battery_nv_load(void)
{
    struct nvrecord_env_t *nvrecord_env = NULL;

    if (box_battery_nv_loaded)
    {
        return;
    }

    nv_record_env_get(&nvrecord_env);

    if (nvrecord_env == NULL)
    {
        box_battery_nv_cache = BOX_BATTERY_INVALID;
        box_battery_nv_loaded = true;
        return;
    }

    box_battery_nv_cache = nvrecord_env->chargerBoxBattery;

    boxChargerStatus.boxChargerBattery = box_battery_nv_cache;
    box_battery_cache_valid = (box_battery_nv_cache <= 100);

    box_battery_nv_loaded = true;
}

static void box_battery_nv_save(uint8_t box_battery)
{
    struct nvrecord_env_t *nvrecord_env = NULL;

    if (box_battery > 100)
    {
        return;
    }

    box_battery_nv_load();

    if (box_battery_nv_cache == box_battery)
    {
        return;
    }

    nv_record_env_get(&nvrecord_env);

    if (nvrecord_env == NULL)
    {
        DBGPRINT("[BOX_BAT][NV_ERR] env null");
        return;
    }

    nvrecord_env->chargerBoxBattery = box_battery;
    nv_record_env_set(nvrecord_env);
    nv_record_flash_flush();

    box_battery_nv_cache = box_battery;
    box_battery_cache_valid = true;
}

static uint8_t box_battery_nv_get(void)
{
    box_battery_nv_load();

    return box_battery_nv_cache;
}
/*
static void box_battery_nv_reset(void)
{
    struct nvrecord_env_t *nvrecord_env = NULL;

    nv_record_env_get(&nvrecord_env);

    if (nvrecord_env == NULL)
    {
        DBGPRINT("[BOX_BAT][RESET] nv env null");
        return;
    }

    nvrecord_env->chargerBoxBattery = BOX_BATTERY_INVALID;
    nv_record_env_set(nvrecord_env);

    box_battery_nv_cache = BOX_BATTERY_INVALID;
    box_battery_nv_loaded = true;
    box_battery_cache_valid = false;

    boxChargerStatus.boxChargerBattery = BOX_BATTERY_INVALID;
    boxChargerStatus.leftEarBudsBattery = BOX_BATTERY_INVALID;
    boxChargerStatus.rightEarBudsBattery = BOX_BATTERY_INVALID;

    DBGPRINT("[BOX_BAT][RESET] box battery reset to 0xFF");
}
*/
uint8_t getBoxChargerBattery(void)
{
    uint8_t tws_box_battery = 0xFF;
    uint8_t nv_box_battery = 0xFF;

    if (box_battery_cache_valid &&
        boxChargerStatus.boxChargerBattery <= 100)
    {
        return boxChargerStatus.boxChargerBattery;
    }

    nv_box_battery = box_battery_nv_get();
    if (nv_box_battery <= 100)
    {
        return nv_box_battery;
    }

    tws_box_battery = app_ibrt_customif_get_tws_peer_box_battery_level();
    if (tws_box_battery <= 100)
    {
        return tws_box_battery;
    }

    return 0xFF;
}

bool  aiWangIsNeedOpenEarBuds(void)
{
    return boxChargerStatus.needOpenEarbuds;
}

uint8_t getPeerBattery(void)
{
    uint8_t twsPeerBattery = app_ibrt_customif_get_tws_peer_battery_level();

    if (twsPeerBattery <= 100)
    {
        DBGPRINT("[BAT] getPeerBattery from TWS sync peer=%d",
                 twsPeerBattery);

        return twsPeerBattery;
    }

    DBGPRINT("[BAT] getPeerBattery TWS invalid=%d", twsPeerBattery);

    return 0xFF;
}

void aiwang_box_battery_update_enable(bool enable)
{
    box_battery_update_enable = enable;
}

static uint8_t box_ear_level_to_percent(uint8_t level)
{
    /*
     * Charging case earbud battery protocol:
     *
     * 0 = 10%
     * 1 = 20%
     * 2 = 30%
     * ...
     * 8 = 90%
     * 9 = 100%
     *
     * 如果充電盒定義 0 代表 0%，請改成另一種換算方式。
     */
    if (level > 9)
    {
        return 0xFF;
    }

    return (uint8_t)((level + 1) * 10);
}

static void wired_uart_get_box_battery(uint8_t *data, uint8_t len)
{
    uint8_t box_battery;
    uint8_t left_level;
    uint8_t right_level;
    uint8_t left_percent;
    uint8_t right_percent;

    if ((data == NULL) || (len < 3))
    {
        DBGPRINT(
            "[BOX_BAT][ERROR] invalid data=%p len=%u",
            data,
            len);

        return;
    }

    /*
     * UART payload:
     *
     * data[0] = Charging case battery, range 0~100%
     * data[1] = Left ear battery level, range 0~9
     * data[2] = Right ear battery level, range 0~9
     */
    box_battery = data[0];
    left_level  = data[1];
    right_level = data[2];

    DBGPRINT(
        "[BOX_BAT][UART_RAW] CASE=%u L_LEVEL=%u R_LEVEL=%u",
        box_battery,
        left_level,
        right_level);

    /*
     * Validate protocol ranges.
     */
    if ((box_battery > 100) ||
        (left_level > 9) ||
        (right_level > 9))
    {
        DBGPRINT(
            "[BOX_BAT][INVALID] CASE=%u L_LEVEL=%u R_LEVEL=%u",
            box_battery,
            left_level,
            right_level);

        return;
    }

    left_percent  = box_ear_level_to_percent(left_level);
    right_percent = box_ear_level_to_percent(right_level);

    /*
     * CMD_SEND_BOX_BATTERY_LEVEL is fresh data from the charging case.
     * Always update the RAM cache after validation.
     *
     * Do not block this update using:
     *     !box_battery_update_enable && box_battery_cache_valid
     */
    boxChargerStatus.getBatteryOK        = true;
    boxChargerStatus.boxChargerBattery   = box_battery;
    boxChargerStatus.leftEarBudsBattery  = left_percent;
    boxChargerStatus.rightEarBudsBattery = right_percent;

    box_battery_cache_valid = true;

    /*
     * Only CASE battery is currently stored in NV.
     */
    box_battery_nv_save(box_battery);

    DBGPRINT(
        "[BOX_BAT][UPDATED][%s] "
        "CASE=%u%% L=%u%% R=%u%% "
        "(raw L=%u R=%u)",
        isRightEarbuds == RIGHT_BUDS ?
            "RIGHT_EAR_FW" : "LEFT_EAR_FW",
        boxChargerStatus.boxChargerBattery,
        boxChargerStatus.leftEarBudsBattery,
        boxChargerStatus.rightEarBudsBattery,
        left_level,
        right_level);

#if defined(IBRT)
    {
        bool tws_connected =
            bts_tws_if_is_tws_link_connected();

        DBGPRINT(
            "[BOX_BAT][TWS] connected=%u",
            tws_connected);
    }
#endif
}

uint8_t aiWang_get_profile_conn_num(void)
{
    uint8_t conn_cnt = 0;
    if(app_bt_get_device(BT_DEVICE_ID_1)->profile_mgr.profile_connected)
        conn_cnt ++;
    if(app_bt_get_device(BT_DEVICE_ID_2)->profile_mgr.profile_connected)
        conn_cnt ++;
    DBGPRINT("%s, conn_cnt = %d", __func__, conn_cnt);

    return conn_cnt;
}

void wired_uart_enter_pairmode(void)
{
	DBGPRINT("%s tws connect %d", __func__, bts_tws_if_is_tws_link_connected());
    if(bts_tws_if_is_tws_link_connected())
    {
        if(TWS_UI_MASTER == app_ibrt_if_get_ui_role())
        {
            app_ibrt_if_enter_pairing_after_tws_connected();
        }
    }
    else
    {
        app_ui_enter_pairing_mode(IBRT_UI_DISABLE_BT_SCAN_TIMEOUT, false);
    }
}

void disconnected_device(bool all_device_flag, uint8_t device_id)
{
	DBGPRINT("%s, %d, %d", __func__, all_device_flag, device_id);
    struct BT_DEVICE_T *curr_device = NULL;
    if(all_device_flag)
    {
        for (int i = 0; i < BT_DEVICE_NUM; ++i)
        {
            curr_device = app_bt_get_device(i);
            if (curr_device->acl_is_connected)
            {
                bts_bt_sink_conn_disconnect_connection(app_bt_get_remote_dev_by_handle(curr_device->acl_conn_hdl));
            }
        }
    }
    else
    {
        curr_device = app_bt_get_device(device_id);
        if (curr_device->acl_is_connected)
        {
            bts_bt_sink_conn_disconnect_connection(app_bt_get_remote_dev_by_handle(curr_device->acl_conn_hdl));
        }
    }
}

/**
 * @brief 進入新手機配對前，若目前已有兩支手機連線，
 *        保留第一支手機，只斷開第二支手機。
 *
 * @return true  已觸發第二支手機斷線
 * @return false 未滿兩支手機，沒有執行斷線
 */
bool aiWang_disconnect_second_phone_for_pairing(void)
{
    struct BT_DEVICE_T *device0 = app_bt_get_device(0);
    struct BT_DEVICE_T *device1 = app_bt_get_device(1);

    bool device0_connected =
        (device0 != NULL) && device0->acl_is_connected;

    bool device1_connected =
        (device1 != NULL) && device1->acl_is_connected;

    DBGPRINT("[PAIR] phone0=%d phone1=%d",
             device0_connected,
             device1_connected);

    /*
     * 只有兩個手機槽都存在 ACL 連線時，
     * 才斷開第二支手機 device_id = 1。
     */
    if (device0_connected && device1_connected)
    {
        /*
         * TWS 已連線時，只由 MASTER 執行手機斷線。
         * 避免左右耳同時對手機連線進行操作。
         */
        if (bts_tws_if_is_tws_link_connected())
        {
            if (TWS_UI_MASTER != app_ibrt_if_get_ui_role())
            {
                DBGPRINT("[PAIR] skip disconnect: this earbud is not TWS master");
                return false;
            }
        }

        DBGPRINT("[PAIR] disconnect second phone, device_id=1");

        app_bt_disconnect_link_by_id(1);

        return true;
    }

    DBGPRINT("[PAIR] less than two phones connected, no disconnect");

    return false;
}

void aiWang_disconnet_phone_enter_pairmode(void)
{
    uint8_t conn_devices = aiWang_get_profile_conn_num();
    DBGPRINT("%s, %d", __func__, conn_devices);
    if(conn_devices > 0)
    {
        if(bts_tws_if_is_tws_link_connected())
        {
            if(TWS_UI_MASTER == app_ibrt_if_get_ui_role())
            {
            	disconnected_device(true, BT_DEVICE_NUM);
            }
        }
        else
        {
        	disconnected_device(true, BT_DEVICE_NUM);
        }
    }
    else
    {
        if(app_bt_get_curr_access_mode() != BTIF_BAM_GENERAL_ACCESSIBLE)
        {
        	wired_uart_enter_pairmode();
        }
    }
}


extern bt_status_t LinkDisconnectDirectly(bool PowerOffFlag);
void aiWang_remove_all_paired_list(void)
{
    bt_status_t            retStatus;
    btif_device_record_t   record;
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    int                    paired_dev_count = nv_record_get_paired_dev_count();

    DBGPRINT("%s", __func__);
    DBGPRINT("Master addr:");
    DUMP8("%02x ",p_ibrt_ctrl->local_addr.address, BTIF_BD_ADDR_SIZE);
    DBGPRINT("Slave addr:");
    DUMP8("%02x ",p_ibrt_ctrl->peer_addr.address, BTIF_BD_ADDR_SIZE);

    ota_disconnect();
    bes_ble_gap_disconnect_all();

    LinkDisconnectDirectly(true);

    for (int32_t index = paired_dev_count - 1; index >= 0; index--)
    {
        retStatus = nv_record_enum_dev_records(index, &record);
        if (BT_STS_SUCCESS == retStatus)
        {
        	DBGPRINT("The index %d of nv records:", index);
            DUMP8("%02x ", record.bdAddr.address, BTIF_BD_ADDR_SIZE);
            //if (memcmp(record.bdAddr.address, p_ibrt_ctrl->local_addr.address, BTIF_BD_ADDR_SIZE) &&
            //    memcmp(record.bdAddr.address, p_ibrt_ctrl->peer_addr.address, BTIF_BD_ADDR_SIZE)&&
            //    memcmp(record.bdAddr.address, address_compare, BTIF_BD_ADDR_SIZE))
            {
            	nv_record_ddbrec_delete(&record.bdAddr);
            }
        }
    }
    app_ibrt_if_config_keeper_clear();
    memset(p_ibrt_ctrl->local_addr.address, 0, BTIF_BD_ADDR_SIZE);
    memset(p_ibrt_ctrl->peer_addr.address, 0, BTIF_BD_ADDR_SIZE);
    nv_record_flash_flush();
}

bool aiWangBoxIsUsed(void)
{
	return boxChargerStatus.boxIsOpen;
}



static void wired_uart_communication_cmd_handle_process(uint8_t *uart_cmd_dat, uint8_t uart_dat_len)
{
    uint8_t cmd_event = 0xff;
    uint16_t crc_dat = 0;
    uint8_t  operateLeftOrRight = 0xFF;
    if(uart_dat_len >= 4 && (0x55 == uart_cmd_dat[0]) && (0xAA == uart_cmd_dat[1]))
    {
        cmd_event = uart_cmd_dat[2];
        crc_dat = crc_dat | uart_cmd_dat[uart_dat_len-1];

        uint8_t *bt_local_addr = NULL;
        bt_local_addr = (uint8_t *)bt_get_local_address();
        if( 0xFF == isRightEarbuds) {
            isRightEarbuds = bt_local_addr[0]&0x01? RIGHT_BUDS:LEFT_BUDS;
        }
        if(0x41 == uart_cmd_dat[3]) operateLeftOrRight = LEFT_BUDS;
        if(0x42 == uart_cmd_dat[3]) operateLeftOrRight = RIGHT_BUDS;
        //DBGPRINT("%s crc dat 0x%04x %s", __func__, crc_dat, isRightEarbuds?"Right":"Left");
        if(crc_dat == crc8(uart_cmd_dat, uart_dat_len-1))
        {
        	//DBGPRINT("%s crc8 OK %s", __func__, isRightEarbuds?"Right":"Left");
        }
        else
        {
        	printf("%s crc8  fail", __func__);
            return;
        }
    }
    else
    {
    	printf("%s data two short", __func__);
        return;
    }

	// 重置空闲定时器
	if (uart_idle_timer_id != NULL) {
		set_er_inbox_status(1);
        ntt_audio_output_mute_refresh();
		osTimerStop(uart_idle_timer_id);
		osTimerStart(uart_idle_timer_id, UART_IDLE_TIMEOUT_MS);
		//DBGPRINT("UART idle timer reset\n");
	}
	else{
		uart_idle_detection_init();
	}
		

    static uint8_t s_last_cmd_event = 0xFF;

    if (s_last_cmd_event != cmd_event)
    {
        DBGPRINT("wiredUart cmd_event=0x%02X", cmd_event);
        s_last_cmd_event = cmd_event;
    }

	uart_rx_handle_data_count = 0;
	
    switch(cmd_event)
    {
    case CMD_HANDSHAKE :		  //Handshake
         {
        	 break;
         }
    case CMD_CASE_STATE:
    case CMD_GET_STATE:
        {
            /*
            * Current charging box firmware does not support real case-state query.
            * Ignore this command for now.
            */
            DBGPRINT("[CASE_STATE] ignored, box query not supported");
            break;
        }
    case CMD_GET_MAC   :  		  //Obtain the MAC address of the earphones
         {
             if (operateLeftOrRight == isRightEarbuds)
             {
            	 DBGPRINT("CMD_GET_MAC!!!");
            	 wired_uart_get_ear_addr_handle();
             }
        	 break;
         }
    case CMD_GET_EAR_POWER:               // Get headphone battery level
    {
        int8_t local_battery_raw = app_battery_current_level();
        uint8_t local_battery = 0xFF;
        uint8_t peer_battery = getPeerBattery();

        /*
        * app_battery_current_level() 預期回傳 0~100。
        * 小於 0 視為無效，大於 100 則限制為 100。
        */
        if (local_battery_raw < 0)
        {
            local_battery = 0xFF;
        }
        else if (local_battery_raw > 100)
        {
            local_battery = 100;
        }
        else
        {
            local_battery = (uint8_t)local_battery_raw;
        }

        /*
        * 將本機電量與 TWS Peer 電量寫入正確的左右耳欄位。
        *
        * 右耳執行時：
        *   LOCAL -> rightEarBudsBattery
        *   PEER  -> leftEarBudsBattery
        *
        * 左耳執行時：
        *   LOCAL -> leftEarBudsBattery
        *   PEER  -> rightEarBudsBattery
        */
        if (isRightEarbuds == RIGHT_BUDS)
        {
            if (local_battery <= 100)
            {
                boxChargerStatus.rightEarBudsBattery = local_battery;
            }

            if (peer_battery <= 100)
            {
                boxChargerStatus.leftEarBudsBattery = peer_battery;
            }
        }
        else
        {
            if (local_battery <= 100)
            {
                boxChargerStatus.leftEarBudsBattery = local_battery;
            }

            if (peer_battery <= 100)
            {
                boxChargerStatus.rightEarBudsBattery = peer_battery;
            }
        }

        /*
        * 顯示本機與 Peer 電量。
        * 無效值使用 INVALID，避免顯示成 255%。
        */
        if (local_battery <= 100 && peer_battery <= 100)
        {
            DBGPRINT("[EAR_POWER][%s] LOCAL=%u%% PEER=%u%%",
                    (isRightEarbuds == RIGHT_BUDS) ? "RIGHT" : "LEFT",
                    local_battery,
                    peer_battery);
        }
        else if (local_battery <= 100)
        {
            DBGPRINT("[EAR_POWER][%s] LOCAL=%u%% PEER=INVALID",
                    (isRightEarbuds == RIGHT_BUDS) ? "RIGHT" : "LEFT",
                    local_battery);
        }
        else if (peer_battery <= 100)
        {
            DBGPRINT("[EAR_POWER][%s] LOCAL=INVALID PEER=%u%%",
                    (isRightEarbuds == RIGHT_BUDS) ? "RIGHT" : "LEFT",
                    peer_battery);
        }
        else
        {
            DBGPRINT("[EAR_POWER][%s] LOCAL=INVALID PEER=INVALID",
                    (isRightEarbuds == RIGHT_BUDS) ? "RIGHT" : "LEFT");
        }

        /*
        * 顯示目前保存的左耳、右耳與充電盒電量。
        */
        if (boxChargerStatus.leftEarBudsBattery <= 100 &&
            boxChargerStatus.rightEarBudsBattery <= 100 &&
            boxChargerStatus.boxChargerBattery <= 100)
        {
            DBGPRINT("[EAR_POWER][%s] L=%u%% R=%u%% CASE=%u%%",
                    (isRightEarbuds == RIGHT_BUDS) ? "RIGHT" : "LEFT",
                    boxChargerStatus.leftEarBudsBattery,
                    boxChargerStatus.rightEarBudsBattery,
                    boxChargerStatus.boxChargerBattery);
        }
        else
        {
            DBGPRINT("[EAR_POWER][%s] L=%s R=%s CASE=%s",
                    (isRightEarbuds == RIGHT_BUDS) ? "RIGHT" : "LEFT",

                    (boxChargerStatus.leftEarBudsBattery <= 100) ?
                        "VALID" : "INVALID",

                    (boxChargerStatus.rightEarBudsBattery <= 100) ?
                        "VALID" : "INVALID",

                    (boxChargerStatus.boxChargerBattery <= 100) ?
                        "VALID" : "INVALID");

            DBGPRINT("[EAR_POWER][%s][RAW] L=%u R=%u CASE=%u",
                    (isRightEarbuds == RIGHT_BUDS) ? "RIGHT" : "LEFT",
                    boxChargerStatus.leftEarBudsBattery,
                    boxChargerStatus.rightEarBudsBattery,
                    boxChargerStatus.boxChargerBattery);
        }

        /*
        * 目前只有右耳保存充電盒版本。
        *
        * Frame 格式：
        * [0]      0x55
        * [1]      0xAA
        * [2]      CMD
        * [3]      Target
        * [4..N-2] Payload
        * [N-1]    CRC
        *
        * 因此實際 payload 長度為：
        * uart_dat_len - 5
        */
        if (isRightEarbuds == RIGHT_BUDS)
        {
            uint8_t version_payload_len = 0;

            memset(boxChargerStatus.boxSoftVersion,
                0,
                sizeof(boxChargerStatus.boxSoftVersion));

            if (uart_dat_len > 5)
            {
                version_payload_len = uart_dat_len - 5;

                /*
                * boxSoftVersion 最多保存 16 bytes，
                * 最後一個 byte 保留給 '\0'。
                */
                if (version_payload_len >
                    (sizeof(boxChargerStatus.boxSoftVersion) - 1))
                {
                    version_payload_len =
                        sizeof(boxChargerStatus.boxSoftVersion) - 1;
                }

                memcpy(boxChargerStatus.boxSoftVersion,
                    &uart_cmd_dat[4],
                    version_payload_len);

                boxChargerStatus.boxSoftVersion[version_payload_len] = '\0';

                DBGPRINT("[EAR_POWER][BOX] version=\"%s\" payload_len=%u frame_len=%u",
                        boxChargerStatus.boxSoftVersion,
                        version_payload_len,
                        uart_dat_len);

                aiWangSetBoxVersion(boxChargerStatus.boxSoftVersion,
                                    version_payload_len);
            }
            else
            {
                DBGPRINT("[EAR_POWER][BOX] no version payload frame_len=%u",
                        uart_dat_len);
            }
        }

        /*
        * 回報本機耳機電量給充電盒。
        * 此函式是 UART TX，不是讀取充電盒三組電量。
        */
        DBGPRINT("[EAR_POWER][CALL] wired_uart_get_battery_level()");

        wired_uart_get_battery_level();

        break;
    }
    case CMD_NONE_CASE:
        {
    	    break;
        }
    case CMD_POWER_OFF:          // Headphones enter shipping / power off
        {
            DBGPRINT("[POWER_OFF] received, wait 1.6s and check charger state");

            /*
            * Do not directly set needOpenEarbuds = false.
            * Do not shutdown immediately.
            * Final decision is based on charger contact status after 1.6s.
            */
            earBudsCloseOff_PowerOff_StartTimer();

            break;
        }
    case CMD_OPEN_CASE:
        {
            g_case_state = 1;
            boxChargerStatus.boxIsOpen = true;
            boxChargerStatus.needOpenEarbuds = true;
            ntt_case_open_pending = true;

            earBudsCloseOff_PogonIn_StopTimer();


        }
        break;

    case CMD_CLOSE_CASE:
        {
            DBGPRINT("[CASE] CLOSE received");

            /*
            * Do not shutdown immediately.
            * Mark close state and start delayed check.
            * If CMD_OPEN_CASE comes before timer expires,
            * CMD_OPEN_CASE will stop the timer.
            */
            g_case_state = 0;
            boxChargerStatus.boxIsOpen = false;
            boxChargerStatus.needOpenEarbuds = false;

            wired_uart_get_battery_level();

            enum APP_BATTERY_CHARGER_T charger_status =
                (enum APP_BATTERY_CHARGER_T)app_battery_charger_indication_open();

            DBGPRINT("[CASE_CLOSE] charger=%s",
                    (charger_status == APP_BATTERY_CHARGER_PLUGIN) ?
                    "PLUGIN" : "PLUGOUT");

            /*
            * Only allow shutdown when charger is really detected.
            */
            if (charger_status != APP_BATTERY_CHARGER_PLUGIN)
            {
                DBGPRINT("[CASE_CLOSE] skip shutdown, charger plugout");
                break;
            }

            if (app_is_stack_ready())
            {
        #ifndef BLE_ONLY_ENABLED
                int activeCons = 0;
                int active_phone_cons = 0;
                int activeSourceCons = 0;

                activeCons = app_bt_get_active_cons();
                active_phone_cons = app_bt_count_mobile_link();
                activeSourceCons = btif_me_get_source_activeCons();

                DBGPRINT("CMD_CLOSE_CASE activeCons=%d activeSourceCons=%d active_phone_cons=%d",
                        activeCons,
                        activeSourceCons,
                        active_phone_cons);

        #ifdef IBRT
                if (bts_tws_if_is_tws_link_connected())
                {
                    uint8_t cmd_sync_poweroff_shutdown[1];

                    cmd_sync_poweroff_shutdown[0] = 1;

                    DBGPRINT("%s send APP_TWS_CMD_POWEROFF_SHUTDOWN_SYNC",
                            __func__);

                    tws_ctrl_send_cmd(APP_TWS_CMD_POWEROFF_SHUTDOWN_SYNC,
                                    cmd_sync_poweroff_shutdown,
                                    1);
                }
        #endif
        #endif
            }

            DBGPRINT("[CASE_CLOSE] start delayed shutdown timer");

            earBudsCloseOff_PogonIn_StartTimer();
        }
    break;

    case CMD_EAR_RESET:
    {
      
        if (0)
        {
            printf("CMD_EAR_RESET factory reset!!! return ");
            return;
        }
        else {

            printf("CMD_EAR_RESET factory reset!!!");

            LinkDisconnectDirectly(true);

            /*
            * Delete phone paired records only.
            * Do not delete TWS pairing records.
            */
            wired_uart_remove_all_phone_paired_list();

            /*
            * Clear SDK NV records.
            * Note: Do not call besui_app_clear_all_nvrecord() here,
            * because it cannot be linked from this module.
            */
            //nv_record_rebuild(NV_REBUILD_SDK_ONLY);

            /*
            * Restore EQ preset 0.
            */
            handleSetEqIndex(0);
            app_ibrt_customif_cmd_sync_music_eq(0);

            /*
            * Restore default key mapping.
            */
            wired_uart_factory_reset_app_nv();
            /*
            * Restore default device name.
            */
            factory_section_set_bt_name("nwm CLIPS", strlen("nwm CLIPS") + 1);

            /*
            * Reset saved box battery to invalid value.
            */
            //box_battery_nv_reset();

            /*
            * Clear BLE state.
            */
            bes_ble_gap_disconnect_all();
            bes_ble_gap_clear_white_list_for_mobile();

            osDelay(500);

            app_reset();
        }
    }
    break;

    case CMD_SET_MAC:
        {
        	if (operateLeftOrRight == isRightEarbuds)
        	{
        		wired_uart_set_peer_address(&uart_cmd_dat[5], 6);
        	}
        }
    	break;
    case CMD_PEER_EAR:
        {
			 if (operateLeftOrRight == isRightEarbuds)
			 {
				 wired_uart_get_battery_level();
			 }
        }
    	break;
    case CMD_VOICE_INDICAT:
    	break;
    case CMD_SET_EARBUD_SHIP_MOD:
       {
    	  DBGPRINT("CMD_SET_EARBUD_SHIP_MOD!!!");
    	  Icp1205ShipEnable();
    	  app_shutdown();
    	  break;
       }

    case CMD_SET_EARBUD_ENTER_PAIR:
    {
        if (operateLeftOrRight == isRightEarbuds)
        {
            DBGPRINT("CMD_SET_EARBUD_ENTER_PAIR isRightEarbuds=%d!!!",
                    isRightEarbuds);

            /*
            * 如果目前已有兩支手機連線：
            * 保留 device_id 0，斷開 device_id 1。
            *
            * 如果只有一支或沒有手機連線：
            * 不執行任何斷線。
            */
            aiWang_disconnect_second_phone_for_pairing();

            enter_pair = 1;
            enter_pair_count = 0;

            ntt_first_no_mobile_pair_mode = true;
            ntt_manual_pairing_mode = true;

            set_pair_status(0);
            set_er_discover_connectable_status(1);

            app_ibrt_if_init_open_box_state_for_evb();

            app_bt_set_access_mode(BTIF_BAM_GENERAL_ACCESSIBLE);
            app_bt_reset_delay_power_off();
        }
        break;
    }
    case CMD_SEND_BOX_BATTERY_LEVEL:
    {
        uint32_t now = hal_sys_timer_get();

        DBGPRINT(
            "[BOX_BAT][RX][%s] len=%u target=0x%02X",
            isRightEarbuds == RIGHT_BUDS ? "RIGHT" : "LEFT",
            uart_dat_len,
            uart_cmd_dat[3]);

        DUMP8("[BOX_BAT][RX_RAW] ",
            uart_cmd_dat,
            uart_dat_len);

        if (ntt_last_box_battery_case_tick != 0)
        {
            uint32_t diff_ms =
                TICKS_TO_MS(now - ntt_last_box_battery_case_tick);

            if (diff_ms < NTT_BOX_BATTERY_CASE_INTERVAL_MS)
            {
                DBGPRINT(
                    "[BOX_BAT][SKIP][%s] interval=%u ms",
                    isRightEarbuds == RIGHT_BUDS ?
                        "RIGHT" : "LEFT",
                    diff_ms);

                break;
            }
        }

        ntt_case_state_sync_local_update(true);

        ntt_last_box_battery_case_tick = now;

        DBGPRINT(
            "[BOX_BAT][PROCESS][%s] crc=0x%04X",
            isRightEarbuds == RIGHT_BUDS ?
                "RIGHT" : "LEFT",
            crc_dat);

        wired_uart_get_box_battery(&uart_cmd_dat[4], 6);

        DBGPRINT(
            "[BOX_BAT][RESULT][%s] CASE=%u L=%u R=%u",
            isRightEarbuds == RIGHT_BUDS ?
                "RIGHT" : "LEFT",
            boxChargerStatus.boxChargerBattery,
            boxChargerStatus.leftEarBudsBattery,
            boxChargerStatus.rightEarBudsBattery);

        if (isRightEarbuds == RIGHT_BUDS)
        {
            DBGPRINT(
                "[NTT_TWS] connected=%d",
                bts_tws_if_is_tws_link_connected());

            if (!bts_tws_if_is_tws_link_connected())
            {
                DBGPRINT(
                    "[NTT_TWS] not connected, start TWS pairing");

                app_ibrt_start_power_on_tws_pairing();
            }
            else
            {
                DBGPRINT(
                    "[NTT_TWS] RIGHT received box battery, resend case state");

                ntt_case_state_sync_resend();
            }
        }
        else
        {
            if (bts_tws_if_is_tws_link_connected())
            {
                DBGPRINT(
                    "[NTT_TWS] LEFT received box battery, resend case state");

                ntt_case_state_sync_resend();
            }
        }

        break;
    }
    case CMD_SEND_DUT_MODE:
        {
            if (operateLeftOrRight == isRightEarbuds)
            {
                DBGPRINT("CMD_SEND_DUT_MODE");
                wired_uart_send_cmd_ack_ok();
                osDelay(20);
                communication_stop();
                osDelay(10);
                hal_sw_bootmode_clear(HAL_SW_BOOTMODE_REBOOT);
                app_factorymode_enter();
            }
        }
        break;
    case CMD_SEND_EAR_PUTIN:
    {
        g_case_state = 1;
        boxChargerStatus.boxIsOpen = true;
        boxChargerStatus.needOpenEarbuds = true;        
        earBudsCloseOff_PogonIn_StopTimer();

        DBGPRINT("[CASE] EAR_PUTIN -> keep power on");

        wired_uart_send_cmd_ack_ok();
    }
    break;
    case CMD_SEND_DTM_MODE:
    {
        DBGPRINT("[DUT] Enter DTM mode");

        /* Disable 1-Mic Noise Suppression */
        ntt_dut_speech_tx_1mic_ns_bypass_set(1);
        DBGPRINT("[DUT] TX 1Mic NS -> BYPASS");
    }
    break;
    default:
       break;
    }
}

static void wired_uart_communication_post_msg(uint8_t *uart_data, uint8_t len)
{
    APP_MESSAGE_BLOCK msg;
    //DBGPRINT("%s", __func__);
    msg.mod_id = APP_MODUAL_AIWANG_WIRED_UART;
    msg.msg_body.message_Param2 = len;
    app_mailbox_put(&msg);
	//handle_key_event(ACTION_IDLE_MUSIC,CLICK_SINGLE);
}

static void wired_uart_communication_rx_data_pre(uint8_t *data_buf, uint8_t data_len)
{
	if( data_buf[0] != 0x55 || data_len < 3) return;
	memset(wiredUartReceiveData, 0x00, MAX_RX_SIZE);
	memcpy(wiredUartReceiveData, data_buf, (data_len>MAX_RX_SIZE?MAX_RX_SIZE:data_len));
	wired_uart_communication_post_msg(data_buf, (data_len>MAX_RX_SIZE?MAX_RX_SIZE:data_len));
}

static int wired_uart_communication_msg_handle_process(APP_MESSAGE_BODY *msg_body)
{
    uint8_t data_len = 0;
    //DBGPRINT("%s", __func__);
    data_len = (uint8_t)msg_body->message_Param2;
    //DBGPRINT("wired_uart_communication_rx: ");
    //DUMP8("%02x ", wiredUartReceiveData, data_len);
    wired_uart_communication_cmd_handle_process(wiredUartReceiveData, data_len);
    return 0;
}

/**
 * @brief 串口空闲超时回调函数
 * @param argument 回调参数（未使用）
 */
static void uart_idle_timeout_callback(void const *argument)
{
    int8_t charging = app_battery_is_charging();

    DBGPRINT(
        "UART idle timeout detected! No data received for %dms, charging=%d",
        UART_IDLE_TIMEOUT_MS,
        charging);

    aiwang_box_battery_update_enable(false);

    /*
     * UART idle does not mean out of case.
     * When still charging, keep the current state and do not trigger
     * IBRT/UI state callbacks during early charging boot.
     */
    if (charging)
    {
        DBGPRINT(
            "[NTT_INBOX] UART idle but still charging, "
            "ignore OUT_CASE");

        set_er_inbox_status(1);        

                /*
         * Update IBRT/UI state.
         * Without this, UI may remain at IN_BOX_OPEN and CASE_CLOSE /
         * power-off flow will never be triggered.
         */
        app_ui_set_local_box_state(IBRT_IN_BOX_CLOSED);
        app_ui_sync_box_state(IBRT_IN_BOX_CLOSED);

        goto exit;
    }

    set_er_inbox_status(0);
    ntt_audio_output_mute_refresh();
    ntt_case_state_sync_local_update(false);

    DBGPRINT(
        "[NTT_OUTBOX] UART idle + charging=0 "
        "-> local OUT_CASE and sync peer");

    /*
     * NTT:
     * UART idle means earbud is out of pogo / out of box.
     * Update UI box state first, otherwise slave may stay in IN_BOX_OPEN.
     */
    app_ui_set_local_box_state(IBRT_OUT_BOX);
    app_ui_sync_box_state(IBRT_OUT_BOX);
    /*
     * First pair mode:
     * No mobile record case.
     * Out case for 2 seconds:
     * Do not shutdown. Only leave pairing / discoverable mode.
     */
    if (ntt_first_no_mobile_pair_mode &&
        bts_tws_if_is_tws_link_connected() &&
        !app_bt_ibrt_has_mobile_link_connected())
    {
        DBGPRINT("[NTT_PAIR] uart idle + no mobile record + tws connected + out case -> exit pairing mode");

        app_bt_set_access_mode(BTIF_BAM_NOT_ACCESSIBLE);

        ntt_first_no_mobile_pair_mode = false;

        goto exit;
    }

exit:
    osTimerDelete(uart_idle_timer_id);
    uart_idle_timer_id = NULL;
}

/**
 * @brief 初始化串口空闲检测
 * @param uart_id 串口ID
 */
void uart_idle_detection_init(void)
{
    // 创建空闲定时器
    if (uart_idle_timer_id == NULL) {
        uart_idle_timer_id = osTimerCreate(osTimer(UART_IDLE_TIMER), osTimerOnce, NULL);
        if (uart_idle_timer_id == NULL) {
            printf("Failed to create UART idle timer\n");
            return;
        }
    }
    
    // 启动定时器
    osTimerStart(uart_idle_timer_id, UART_IDLE_TIMEOUT_MS);

}

void wired_uart_communication_modual_init(void)
{
    if(!wiredUartInitFlag)
    {
        DBGPRINT("%s", __func__);
        app_set_threadhandle(APP_MODUAL_AIWANG_WIRED_UART, wired_uart_communication_msg_handle_process);
        communication_init();
        communication_receive_register_callback(wired_uart_communication_rx_data_pre);
        wiredUartInitFlag = true;
		//osDelay(200);
		//communication_stop();
		
		// 启动 Pogo Pin 监控
    	//start_pogo_pin_monitor();

		uart_idle_detection_init();
    }
}
