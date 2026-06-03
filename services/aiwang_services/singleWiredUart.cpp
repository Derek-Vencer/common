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
#define UART_IDLE_TIMEOUT_MS 5000
// 空闲超时回调函数
static void uart_idle_timeout_callback(void const *argument);
void uart_idle_detection_init(void);

// 定时器定义
osTimerDef(UART_IDLE_TIMER, uart_idle_timeout_callback);


#undef printf
#define printf(fmt, ...) \
    hal_trace_printf(0, "[goc-uart] " fmt, ##__VA_ARGS__)
#undef DBGPRINT
#define DBGPRINT(fmt,...)  \
	hal_trace_printf(2, "[goc-uart] " fmt, ##__VA_ARGS__)

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

#define LEFT_BUDS  0
#define RIGHT_BUDS 1
#define MAX_RX_SIZE  64
static uint8_t  isRightEarbuds = 0xFF; //unknown odd  right , even left
static uint8_t  wiredUartInitFlag  = FALSE;
static uint8_t  wiredUartReceiveData[MAX_RX_SIZE];
static BOX_STATUS  boxChargerStatus  ;

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

static uint8_t enter_pair = 0;
static uint8_t enter_pair_count = 0;

static uint8_t pair_status = 0;
static void wired_uart_get_battery_level(void)
{
#if 0
    uint8_t buff[4] = {0};
    buff[0] = 0x55;
    buff[1] = 0xAA;
    buff[2] = app_battery_current_level()%9;
	if(app_battery_current_level()>=9)
	{
		buff[2] = 9;
	}
    buff[3] = crc8(buff,3);
    communication_send_buf(buff, 4);
#else
	uint8_t buff[5] = {0};
    buff[0] = 0x55;
    buff[1] = 0xAA;
    buff[2] = app_battery_current_level()%9;
	if(app_battery_current_level()>=9)
	{
		buff[2] = 9;
	}
	if(1)//(enter_pair == 1)
	{
		pair_status = get_pair_status();
		buff[3] = pair_status;
		//pair_status = 0;
		if(enter_pair_count > 1)
		{
			enter_pair = 0;
			set_pair_status(0);
			enter_pair_count = 0;
		}
		else
			enter_pair_count ++;
		//set_pair_status(0);
	}
	else{
		buff[3] = 0;
		enter_pair_count = 0;
	}
	//set_pair_status(0);
	//buff[3] = 0;
    buff[4] = crc8(buff,4);
    communication_send_buf(buff, 5);
#endif
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
    bt_status_t            retStatus;
    btif_device_record_t   record;
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    int                    paired_dev_count = nv_record_get_paired_dev_count();
    DBGPRINT("%s.", __func__);
    //uint8_t address_compare[6] = {0};
    DBGPRINT("Master addr:");
    DUMP8("%02x ",p_ibrt_ctrl->local_addr.address, BTIF_BD_ADDR_SIZE);
    DBGPRINT("Slave addr:");
    DUMP8("%02x ",p_ibrt_ctrl->peer_addr.address, BTIF_BD_ADDR_SIZE);

    for (int32_t index = paired_dev_count - 1; index >= 0; index--)
    {
        retStatus = nv_record_enum_dev_records(index, &record);
        if (BT_STS_SUCCESS == retStatus)
        {
        	DBGPRINT("Remove The index %d of nv records:", index);
            DUMP8("%02x ", record.bdAddr.address, BTIF_BD_ADDR_SIZE);
            //if (memcmp(record.bdAddr.address, p_ibrt_ctrl->local_addr.address, BTIF_BD_ADDR_SIZE) &&
            //    memcmp(record.bdAddr.address, p_ibrt_ctrl->peer_addr.address, BTIF_BD_ADDR_SIZE)&&
            //    memcmp(record.bdAddr.address, address_compare, BTIF_BD_ADDR_SIZE))
            {
            	nv_record_ddbrec_delete(&record.bdAddr);
            }
        }
    }
    nv_record_flash_flush();
}

uint8_t getBoxChargerBattery(void)
{
	if(!boxChargerStatus.getBatteryOK) return 0xff;
	return boxChargerStatus.boxChargerBattery ;
}

bool  aiWangIsNeedOpenEarBuds(void)
{
    return boxChargerStatus.needOpenEarbuds;
}

uint8_t getPeerBattery(void)
{
	if(!boxChargerStatus.getBatteryOK) return 0xff;
	return boxChargerStatus.leftEarBudsBattery;
}

static void wired_uart_get_box_battery(uint8_t *data ,uint8_t len)
{
	DBGPRINT("%s boxChargerBattery=0x%02x leftEarBudsBattery=0x%02x rightEarBudsBattery=0x%02x",
			__func__,
			data[0],
			data[1],
			data[2]
			);
	boxChargerStatus.getBatteryOK        = true;
	boxChargerStatus.boxChargerBattery   = data[0];
	boxChargerStatus.leftEarBudsBattery  = data[1];
	boxChargerStatus.rightEarBudsBattery = data[2];
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
		osTimerStop(uart_idle_timer_id);
		osTimerStart(uart_idle_timer_id, UART_IDLE_TIMEOUT_MS);
		//printf("UART idle timer reset\n");
	}
	else{
		uart_idle_detection_init();
	}
		

    DBGPRINT("wiredUart cmd_event=0x%02X", cmd_event);
	uart_rx_handle_data_count = 0;
	
    boxChargerStatus.boxIsOpen = true;
    switch(cmd_event)
    {
    case CMD_CASE_STATE:
         {
        	 break;
         }
    case CMD_HANDSHAKE :		  //Handshake
         {
        	 break;
         }
    case CMD_GET_STATE : 		  //Get headphone status
         {
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
    case CMD_GET_EAR_POWER:		  //Get headphone battery level
         {
        	 if (operateLeftOrRight == isRightEarbuds)
        	 {
        		 memset(&boxChargerStatus.boxSoftVersion[0], 0, 16+1);
        		 memcpy(&boxChargerStatus.boxSoftVersion[0],&uart_cmd_dat[4],11);
        		 DBGPRINT("CMD_GET_EAR_POWER include boxVersion:%s!!!", boxChargerStatus.boxSoftVersion);
                 aiWangSetBoxVersion(&boxChargerStatus.boxSoftVersion[0], 11);
        		 wired_uart_get_battery_level();
        	 }
       	     break;
         }
    case CMD_NONE_CASE:
        {
    	    break;
        }
    case CMD_POWER_OFF:	         //Headphones enter shipping
        {
        	boxChargerStatus.boxIsOpen = false;
            boxChargerStatus.needOpenEarbuds = false;
            app_shutdown();
        }
    	break;
    case CMD_OPEN_CASE:
        {
        	printf("CMD_OPEN_CASE!!!");
            boxChargerStatus.boxIsOpen = true;
            boxChargerStatus.needOpenEarbuds = true;
        	wired_uart_get_battery_level();
        	osDelay(10);
            //hal_sw_bootmode_clear(HAL_SW_BOOTMODE_REBOOT);
            //hal_sw_bootmode_set(HAL_SW_BOOTMODE_SINGLE_LINE_DOWNLOAD);
            //pmu_reboot();
        	//app_reset();
        }
    	break;
    case CMD_CLOSE_CASE:
        {
          printf("CMD_CLOSE_CASE poweroff!!!");
          boxChargerStatus.boxIsOpen = false;
          wired_uart_get_battery_level();
          if( app_is_stack_ready())
          {
#ifndef BLE_ONLY_ENABLED
			int activeCons = 0, active_phone_cons;
			int activeSourceCons = 0;
			//fixed only count the phone count,except tws
			activeCons = app_bt_get_active_cons();
			active_phone_cons = app_bt_count_mobile_link();
			activeSourceCons = btif_me_get_source_activeCons();
			DBGPRINT("CMD_CLOSE_CASE activeCons==%d activeSourceCons=%d active_phone_cons=%d\n", activeCons, activeSourceCons, active_phone_cons);
#ifdef IBRT
			if (bts_tws_if_is_tws_link_connected())
			{
				//app_ibrt_customif_cmd_sync_poweroff_shutdown(true);
				uint8_t cmd_sync_poweroff_shutdown[1];
				cmd_sync_poweroff_shutdown[0] = 1;
				DBGPRINT("%s poweroff_flag true",__func__);
				tws_ctrl_send_cmd(APP_TWS_CMD_POWEROFF_SHUTDOWN_SYNC, cmd_sync_poweroff_shutdown, 1);
				osDelay(60);
		   }
#endif //IBRT
#endif //BLE_ONLY_ENABLED
          }
          osDelay(30);
          
          boxChargerStatus.needOpenEarbuds = false;
          earBudsCloseOff_PogonIn_StartTimer();
          //app_shutdown();
        }
    	break;
    case CMD_EAR_RESET: //fatory
        {
        	//aiWang_remove_all_paired_list();  // Removed: deletes ALL pairings including TWS
        	printf("CMD_EAR_RESET factory resett!!!");
            LinkDisconnectDirectly(true);
        	wired_uart_remove_all_phone_paired_list();  // Keep: only deletes phone pairings, preserves TWS
        	if (bts_tws_if_is_tws_link_connected())
        	{
        		uint8_t cmd_sync_clear_pairlist[2];
        	    cmd_sync_clear_pairlist[0] = 0;
        	    cmd_sync_clear_pairlist[1] = 1;
        		tws_ctrl_send_cmd(APP_TWS_CMD_POWEROFF_SHUTDOWN_SYNC, cmd_sync_clear_pairlist, 2);
        	}
            bes_ble_gap_disconnect_all();
            bes_ble_gap_clear_white_list_for_mobile();
            osDelay(10);
        	app_reset();
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
    case  CMD_SET_EARBUD_ENTER_PAIR:
      {
    	  if (operateLeftOrRight == isRightEarbuds)
    	  {
    		   DBGPRINT("CMD_SET_EARBUD_ENTER_PAIR isRightEarbuds=%d!!!", isRightEarbuds);
#if 1
			   enter_pair = 1;
				enter_pair_count = 0;
				set_pair_status(0);
				set_er_discover_connectable_status(1);
				app_bt_set_access_mode(BTIF_BAM_GENERAL_ACCESSIBLE);
				app_bt_reset_delay_power_off();
    		   aiWang_disconnet_phone_enter_pairmode();
#else
    		   wired_uart_enter_pairmode();
               if( 1 == isRightEarbuds) //Enter PairMode
               {
            	   app_ibrt_if_init_open_box_state_for_evb();
                   app_ibrt_if_enter_pairing_after_tws_connected();
               }
               else
               {
	                app_ibrt_if_init_open_box_state_for_evb();
	                app_ibrt_internal_enter_freeman_pairing();
               }
#endif

    	  }
    	  break;
      }
    case CMD_SEND_BOX_BATTERY_LEVEL:
       {
    	   if (operateLeftOrRight == isRightEarbuds)
    	   {
    		   wired_uart_get_box_battery(&uart_cmd_dat[4], 6);
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
		  DBGPRINT("CMD_SEND_EAR_PUTIN");
          boxChargerStatus.needOpenEarbuds = true;
		  wired_uart_send_cmd_ack_ok();
		  //disconnected_device(true, BT_DEVICE_ID_1);
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
    DBGPRINT("wired_uart_communication_rx: ");
    DUMP8("%02x ", wiredUartReceiveData, data_len);
    wired_uart_communication_cmd_handle_process(wiredUartReceiveData, data_len);
    return 0;
}


/**
 * @brief 串口空闲超时回调函数
 * @param argument 回调参数（未使用）
 */
static void uart_idle_timeout_callback(void const *argument)
{
    DBGPRINT("UART idle timeout detected! No data received for %dms\n", UART_IDLE_TIMEOUT_MS);
    set_er_inbox_status(0);
	
	osTimerDelete(uart_idle_timer_id);
	uart_idle_timer_id = NULL;
    // 调用其他函数
    //call_other_function();
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
