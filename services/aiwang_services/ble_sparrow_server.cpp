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

#ifndef TRACE
#define TRACE(attr, str, ...)   TR_DEBUG(attr, str, ##__VA_ARGS__)
#endif

#define MAX_PACKET_SIZE             (512)
#define SPARRAW_EVENT_MAX_MAILBOX   (10)
#define SPARRAW_EVENT_BUF_SIZE      (MAX_PACKET_SIZE*SPARRAW_EVENT_MAX_MAILBOX)
#define SPARRAW_BUFF_SIZE           (4096)

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

extern "C" void system_get_info(uint8_t *fw_rev_0, uint8_t *fw_rev_1, uint8_t *fw_rev_2, uint8_t *fw_rev_3);

void handleSetKeyMapActionAndFunc(uint8_t index,uint8_t action,uint8_t func);
void handleSetKeyMapNumber(uint8_t index);
static void keymap_init_default(void);


// #define  DISPLAY_EARBUDS_VERSION "01.01.00.03"
#define  DISPLAY_EARBUDS_VERSION   "V0.1.3" //"01.01.00.04"

typedef struct{
	uint8_t set_name_status;
}bleCmdSetStatus;
#define need_send_data_by_notify 0

bleCmdSetStatus bleCmdSet_status;

// 空闲定时器
static osTimerId double_hold_idle_timer_id = NULL;
// 空闲超时时间（5秒）
#define DOUBLE_HOLD_IDLE_TIMEOUT_MS 200
uint8_t button_hold_type = 0xff;

static void double_hold_idle_timeout_callback(void const *argument);

// 定时器定义
osTimerDef(DOUBLE_HOLD_IDLE_TIMER, double_hold_idle_timeout_callback);

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

uint8_t key_event_is_left = 0;
/***********************************************/


uint8_t er_inbox = 0;
void handleGetBatteryLevel(const uint8_t *data, uint16_t len)
{

    uint8_t batflag; //left, right, box
    batflag = app_battery_current_level();
    TRACE(0,"%s batflag=%d", __func__, batflag);
#if 0
    if(batflag > 9)
    {
        batflag = 9;
    }
#endif
	uint8_t batteryArray[3] = {0, 0, 0};
	batteryArray[0]  = batflag;
	if( 0xFF == getPeerBattery())
	{
		batteryArray[1]  = batflag;
	}
	else
	{
		batteryArray[1]  = getPeerBattery();
		#if 0
	    if(batteryArray[1] > 9)
	    {
	    	batteryArray[1] = 9;
	    }
		#endif
	}
	batteryArray[2] = getBoxChargerBattery();
//#if need_send_data_by_notify
	//sparraw_tx_msg(RSP_GET_BATTERY_LEVEL, batteryArray, 3);
//#endif
	batteryArray[2] = getBoxChargerBattery();
	uint8_t read_send_data[20] = {0};
	read_send_data[1] = 3;
	memcpy(&read_send_data[2],batteryArray,2);
	//sparraw_read_rsp_msg(RSP_GET_BATTERY_LEVEL,param->aw_connhdl,param->aw_token, batteryArray, 3);
	sparraw_tx_msg(0x31, read_send_data, 3+2);


}

void handleGetDeviceName(const uint8_t *data, uint16_t len)
{
	TRACE(0,"%s.", __func__);
	uint8_t* localname =  factory_section_get_bt_name();
    if(localname)
    {
#if need_send_data_by_notify
    	sparraw_tx_msg(RSP_GET_DEVICE_NAME, (const uint8_t*)localname, strlen((const char *)localname)+1);
#endif
    }
}

void handleSetDeviceName(const uint8_t *data, uint16_t len)
{
	TRACE(0,"%s.", __func__);
	//if( factory_section_set_bt_name((const char *)&data[1], len -1))
	char nameBuffer[248+1] = {0};
	if((len - 3) > 248)
	{
		bleCmdSet_status.set_name_status = 0x3B;
		return;
	}

	char name_len_buf[3] = {0};
	name_len_buf[0] = data[1];
	name_len_buf[1] = data[2];
	name_len_buf[2] = 0;

	uint16_t name_len = 0;
	if((name_len_buf[0] == 0x00) && (name_len_buf[1] < 248))
	{
		name_len = name_len_buf[1];
	}
	else{
		bleCmdSet_status.set_name_status = 0x3B;
		return;
	}
	if((name_len + 3) < len)
	{
		bleCmdSet_status.set_name_status = 0x3B;
		return;
	}
#if 1
	//name_len = name_len > 248?248:name_len;
	name_len = name_len > 45?45:name_len;
	if (name_len > 0)
	{
		memcpy(nameBuffer, &data[3], name_len);
		if( factory_section_set_bt_name(nameBuffer, name_len+1))
		{
			TRACE(0,"%s set bt name error", __func__);
			bleCmdSet_status.set_name_status = 0x3B;
		}
		else
		{
			bleCmdSet_status.set_name_status = 0x3A;
		}

		app_ibrt_customif_cmd_sync_bt_name((uint8_t*)nameBuffer,name_len+1);
		// if( factory_section_set_ble_name((const char*)nameBuffer,len+1))
		// {
		// 	TRACE(0,"%s set ble name error", __func__);
		// }
	}
#else
	len = (len - 1) > 248?248:(len -1);
	if (len > 0)
	{
		memcpy(nameBuffer, &data[1], len);
		if( factory_section_set_bt_name(nameBuffer, len+1))
		{
			TRACE(0,"%s set bt name error", __func__);
			bleCmdSet_status.set_name_status = 0x3B;
		}
		else
		{
			bleCmdSet_status.set_name_status = 0x3A;
		}
		// if( factory_section_set_ble_name((const char*)nameBuffer,len+1))
		// {
		// 	TRACE(0,"%s set ble name error", __func__);
		// }
	}
#endif
#if need_send_data_by_notify
	sparraw_tx_msg(RSP_SET_DEVICE_NAME, (const uint8_t*)"", 0);
#endif
}

void handleGetKeyMapping(const uint8_t *data, uint16_t len)
{
	TRACE(0,"%s.", __func__);
#if need_send_data_by_notify
	const uint8_t keyMaps[2] = {0x00,0x14};

	sparraw_tx_msg(RSP_GET_KEY_MAPPING, (const uint8_t*)&keyMaps[0], (sizeof(keyMaps)/keyMaps[0]));
#endif
}

void handleSetKeyMapping(const uint8_t *data, uint16_t len)
{
	TRACE(0,"%s.", __func__);
	const uint8_t *data_buf = data + 1;
	
if(len > 2){
		uint16_t data_len = data_buf[0] << 8 | (data_buf[1]);
		uint16_t key_count = ((data_len - 1) >> 1);
		if(key_count != data_buf[2] || (key_count > 20))
		{
			//error data
		}
		else{
			uint16_t key_map_len = key_count * 2;
			if((key_map_len + 4) != len){
				//error data
			}
			else{
				const uint8_t *key_map = &data_buf[3];
				uint8_t local_er_count = 0;
				//uint8_t *bt_local_addr = NULL;
				//bt_local_addr = (uint8_t *)bt_get_local_address();
				//uint8_t LocalLeftEarbuds = bt_local_addr[0]&0x01?0:1;
				for(int i = 0; i < key_count; i++){
					//uint8_t  isLeftEarbuds = key_map[0+i*2]&0x01;
					uint8_t key_actions = key_map[0+i*2];
					uint8_t key_func = key_map[1+i*2];
					handleSetKeyMapActionAndFunc(local_er_count,key_actions,key_func);
					local_er_count ++;					
					
				}
				handleSetKeyMapNumber(local_er_count);
				keymap_init_default();
				uint8_t cmd_sync_button_map[50] = {0};
				cmd_sync_button_map[0] = local_er_count;

				for(int i = 0;i < local_er_count;i ++){
					cmd_sync_button_map[i*2+1] = (uint8_t)s_key_map[i].actions;
					cmd_sync_button_map[i*2+2] = (uint8_t)s_key_map[i].function;
				}
				app_ibrt_customif_cmd_sync_button_map(cmd_sync_button_map,local_er_count*2+1);
			}
		}
	}
	else
	{
		
	}
	
#if need_send_data_by_notify
	sparraw_tx_msg(RSP_SET_KEY_MAPPING, (const uint8_t*)"", 0);
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
	TRACE(0,"%s.", __func__);
	//sparraw_tx_msg(RSP_GET_EQ_PRESET, 0, 1);
	uint8_t index = 0;
	handleGetEqIndex(&index);
#if need_send_data_by_notify
	sparraw_tx_msg(RSP_GET_EQ_PRESET, &index, 1);
#endif
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
	TRACE(0,"%s.", __func__);
	uint8_t presetId = 0;
	if(len >= 3)
	{
		presetId = data[3];
		if(presetId >= 6)
		{
			TRACE(0,"%s.", "ERROR_ID");
		}
		else{
			TRACE(0,"%s[%d].", __func__,presetId);
			//audio_eq_hw_dac_iir_callback((uint8_t*)"1",1);
			//audio_eq_set_cfg(NULL, &audio_eq_iir_cfg, AUDIO_EQ_TYPE_HW_DAC_IIR); 
			
			audio_eq_set_cfg(NULL, audio_eq_cfg_vol_list[presetId], AUDIO_EQ_TYPE_HW_DAC_IIR); //AUDIO_EQ_TYPE_SW_IIR
			app_ibrt_customif_cmd_sync_music_eq(presetId);

			#ifdef __AUDIO_DYNAMIC_BOOST__
#ifdef DYNAMIC_BOOST_USE_HW_EQ
        audio_dynamic_boost_set_new_customer_iir_eq(&audio_process.hw_dac_iir_cfg, AUDIO_EQ_TYPE_HW_DAC_IIR);
#endif
#endif

			handleSetEqIndex(presetId);
#if 0
				HW_CODEC_IIR_CFG_T *hw_iir_cfg_dac = NULL;
				enum AUD_SAMPRATE_T sample_rate_hw_dac_iir;

				memset(&hw_dac_iir_cfg, 0, sizeof(IIR_CFG_T));
				
						hw_iir_cfg_dac = hw_codec_iir_get_cfg(sample_rate_hw_dac_iir,&hw_dac_iir_cfg);
			        ASSERT(hw_iir_cfg_dac != NULL, "[%s] %d codec IIR parameter error!", __func__, (uint32_t)hw_iir_cfg_dac);

			        // hal_codec_iir_dump(hw_iir_cfg_dac);

			        hw_codec_iir_set_cfg(hw_iir_cfg_dac, sample_rate_hw_dac_iir, HW_CODEC_IIR_DAC);

#ifdef __AUDIO_DYNAMIC_BOOST__
#ifdef DYNAMIC_BOOST_USE_HW_EQ
			        audio_dynamic_boost_set_new_customer_iir_eq(&audio_process.hw_dac_iir_cfg, AUDIO_EQ_TYPE_HW_DAC_IIR);
#endif
#endif
#endif
		}
	}
#if need_send_data_by_notify
	sparraw_tx_msg(RSP_SET_EQ_PRESET, (const uint8_t*)"", 0);
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
	TRACE(0,"%s.", __func__);
	uint8_t version[11+11+1] = {0};
	//system_get_info(&version[0], &version[1], &version[2], &version[4]);
	//const uint8_t *version = (const uint8_t *)"01.01.00.03";
	memcpy(&version[0], DISPLAY_EARBUDS_VERSION, strlen(DISPLAY_EARBUDS_VERSION));
	aiWangGetChargerBoxVersion(&version[11]);
#if 0
	sparraw_tx_msg(RSP_GET_FW_VERSION, (const uint8_t*)version, 23);
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


extern "C" uint8_t aiWangGetEarBudsColor(void) {
    struct nvrecord_env_t *nvrecord_env;
    nv_record_env_get(&nvrecord_env);
    return nvrecord_env->color_data;
}

void aiWangSetEarBudsColor(uint8_t color){
    struct nvrecord_env_t *nvrecord_env;
    nv_record_env_get(&nvrecord_env);
    if (nvrecord_env->color_data != color)
    {
		nvrecord_env->color_data = color;
		nv_record_env_set(nvrecord_env);
    }
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


//extern void handleGetKeyMapNumber(uint8_t *index);
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
#if 0
    s_key_map_count = 0;
    // 音乐模式
    s_key_map[s_key_map_count++] = (key_map_entry_t){ .actions = (ACTION_IDLE_MUSIC | CLICK_SINGLE),      .function = FUNC_PLAY_PAUSE };
    s_key_map[s_key_map_count++] = (key_map_entry_t){ .actions = (ACTION_IDLE_MUSIC | CLICK_DOUBLE),      .function = FUNC_NEXT_SONG };
    s_key_map[s_key_map_count++] = (key_map_entry_t){ .actions = (ACTION_IDLE_MUSIC | CLICK_TRIPLE),      .function = FUNC_PREV_SONG };
    s_key_map[s_key_map_count++] = (key_map_entry_t){ .actions = (ACTION_IDLE_MUSIC | CLICK_HOLD_2S),     .function = FUNC_VOICE_ASSIST };
    // 通话模式
    s_key_map[s_key_map_count++] = (key_map_entry_t){ .actions = (ACTION_CALL | CLICK_SINGLE),            .function = FUNC_ACCEPT_CALL };
    s_key_map[s_key_map_count++] = (key_map_entry_t){ .actions = (ACTION_CALL | CLICK_DOUBLE),            .function = FUNC_REJECT_CALL };
    s_key_map[s_key_map_count++] = (key_map_entry_t){ .actions = (ACTION_CALL | CLICK_HOLD_2S),           .function = FUNC_VOICE_ASSIST };
    // 左右方向
    s_key_map[s_key_map_count++] = (key_map_entry_t){ .actions = (ACTION_LEFT | CLICK_SINGLE),            .function = FUNC_VOLUME_DOWN };
    s_key_map[s_key_map_count++] = (key_map_entry_t){ .actions = (ACTION_LEFT | CLICK_HOLD_2S),           .function = FUNC_PREV_SONG };
    s_key_map[s_key_map_count++] = (key_map_entry_t){ .actions = (ACTION_RIGHT | CLICK_SINGLE),           .function = FUNC_VOLUME_UP };
    s_key_map[s_key_map_count++] = (key_map_entry_t){ .actions = (ACTION_RIGHT | CLICK_HOLD_2S),          .function = FUNC_NEXT_SONG };
#endif
	uint8_t key_number = 0;
	handleGetKeyMapNumber(&key_number);
	s_key_map_count = key_number;
	if(key_number == 0)
	{
		for(int i = 0;i<20;i++)
		{
			uint8_t key_action = s_key_default_map[i].actions;
			uint8_t key_func = s_key_default_map[i].function;
			
			s_key_map[i] = (key_map_entry_t){ .actions = key_action,      .function = key_func };
		}
		s_key_map_count = 20;
		//printf("@@key_number usr defaule\n");
	}
	else{
		for(int i = 0;i<key_number;i++)
		{
			uint8_t key_action = 0;
			uint8_t key_func = 0;
			handleGetKeyMapActionAndFunc(i,&key_action,&key_func);
			
			s_key_map[i] = (key_map_entry_t){ .actions = key_action,      .function = key_func };
		}
	}
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
       bt_key_handle_call(call_state);
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
	switch (func) {
		case FUNC_ACCEPT_CALL:	on_accept_call(); break;
		case FUNC_REJECT_CALL:	on_reject_call(); break;
		case FUNC_PLAY_PAUSE:	on_play_pause(); break;
		case FUNC_NEXT_SONG:	on_next_song(); break;
		case FUNC_PREV_SONG:	on_prev_song(); break;
		case FUNC_VOLUME_UP:	on_volume_up(); break;
		case FUNC_VOLUME_DOWN:	on_volume_down(); break;
		case FUNC_VOICE_ASSIST: on_voice_assist(); break;
		default: printf(">>> No function assigned\n"); break;
	}
}
/*==============================================================================
 * 5. 按键事件处理：判断并调用相关函数
 *----------------------------------------------------------------------------*/
void handle_key_event(click_type_t click)
{
	uint8_t action = 0;
	CALL_STATE_E call_state = app_bt_get_call_state();
	//TRACE(0, "%s call_state=%d",  __func__, call_state);
	
    if (call_state == CALL_STATE_IDLE)
    {
       //bt_key_handle_music_playback();
       action = 0x00;
    }
	else
	{
		action = 0x01;
	}
	#if 0
	uint8_t *bt_local_addr = NULL;
	bt_local_addr = (uint8_t *)bt_get_local_address();
    uint8_t isLeftEarbuds = bt_local_addr[0]&0x01?0x02:0x01;
	#else
	uint8_t *bt_local_addr = NULL;
	bt_local_addr = (uint8_t *)bt_get_local_address();
    uint8_t isLeftEarbuds = bt_local_addr[0]&0x01?0x02:0x01;
	if(key_event_is_left == 1)
	{
		if(isLeftEarbuds == 0x02)
			isLeftEarbuds = 0x01;
		else
			isLeftEarbuds = 0x02;
		//key_event_is_left = 0;
	}

	#endif
   uint8_t action_er = (action * 4)  | isLeftEarbuds;
   //action_er = action_er << 2;
    //printf("Key event: action=0x%02X, click=0x%02X\n", action_er, click);
    function_t func = keymap_lookup(action_er, click,isLeftEarbuds);
    //printf("  -> function code: 0x%02X\n", func);
    key_function_execute(func);
}


/***********************************************/
void aparraw_set_key_event_left(uint8 status)
{
	key_event_is_left = status;
}

/*************************************************/
static void double_hold_idle_timeout_callback(void const *argument)
{
    //DBGPRINT("UART idle timeout detected! No data received for %dms\n", DOUBLE_HOLD_IDLE_TIMEOUT_MS);
	if(button_hold_type != 0xff){
		if(button_hold_type == CLICK_DOUBLE_HOLD)
			handle_key_event(CLICK_DOUBLE_HOLD);
		else if(button_hold_type == CLICK_HOLD_2S)
			handle_key_event(CLICK_HOLD_2S);
		if(double_hold_idle_timer_id)
		{
			osTimerStart(double_hold_idle_timer_id, DOUBLE_HOLD_IDLE_TIMEOUT_MS);
		}
	}
    // 调用其他函数
    //call_other_function();
}
void double_hold_idle_detection_init(void)
{
    // 创建空闲定时器
    if (double_hold_idle_timer_id == NULL) {
        double_hold_idle_timer_id = osTimerCreate(osTimer(DOUBLE_HOLD_IDLE_TIMER), osTimerOnce, NULL);
        if (double_hold_idle_timer_id == NULL) {
            printf("Failed to create UART idle timer\n");
            return;
        }
    }
    
    // 启动定时器
    osTimerStart(double_hold_idle_timer_id, 500);

}
static void delete_double_hold_time(void)
{
	if(double_hold_idle_timer_id != NULL)
	{
		osTimerDelete(double_hold_idle_timer_id);
		double_hold_idle_timer_id = NULL;
	}
}
/************************************************/
void sparraw_tx_key_click_notify_msg(uint8_t kick_type)
{
	uint8_t  keyEventNotify[3];
	keyEventNotify[0] = 0xB0;
	keyEventNotify[1] = 0x04;
	keyEventNotify[2] = kick_type;
	TRACE(0, "%s kick_type=%d enterKeyClickTestMode=%d",  __func__, kick_type, enterKeyClickTestMode);
	if( enterKeyClickTestMode && app_sparraw_env.notifyEnable)
	{
		ble_aiwang_srv_send_data_via_notification(keyEventNotify, 3);
	}
	if((er_inbox == 1) && (key_event_is_left == 0))
	{
		return;
	}
	switch(kick_type)
	{
		case KEY_CLICK:
		{
			handle_key_event(CLICK_SINGLE);
		}break;
		case KEY_DOUBLE_CLICK:
		{
			handle_key_event(CLICK_DOUBLE);
		}break;
		case KEY_TRIPLE_CLICK:
		{
			handle_key_event(CLICK_TRIPLE);
		}break;
		case KEY_DOUBLE_HOLD_CLICK:
		{
			handle_key_event(CLICK_DOUBLE_HOLD);
			double_hold_idle_detection_init();
			button_hold_type = CLICK_DOUBLE_HOLD;
		}break;
		case KEY_HOLD_CLICK:
		{
			handle_key_event(CLICK_HOLD_2S);
			double_hold_idle_detection_init();
			button_hold_type = CLICK_HOLD_2S;
		}break;
		case KEY_UP:
		{
			delete_double_hold_time();
			aparraw_set_key_event_left(0);
			button_hold_type = 0xff;
		}break;
		default:break;
	}
}

//#if need_send_data_by_notify
static void sparraw_tx_msg(uint8_t rsp_type, const uint8_t* data, uint16_t len) {
	uint8_t  rsp_buffer[64];
	rsp_buffer[0] = rsp_type;
	if (NULL != data && len > 0)
	{
		memcpy(&rsp_buffer[1], data, len);
	}
	if(app_sparraw_env.notifyEnable) ble_aiwang_srv_send_data_via_notification(rsp_buffer, len + 1);
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

static void sparraw_rx_cmd_parse_v2(const uint8_t *data, uint16_t len) {
   uint16_t i ;
   for (i = 0; i < aiWangCmdTypesCount/*sizeof(aiWangCmdTypes)/sizeof(aiWangCmdTypes[0])*/; i++)
   {
	   TRACE(0,"%s data[0] = %02x %02x", __func__, data[0], aiWangCmdTypes[i].cmd);
	   if ( data[0] == aiWangCmdTypes[i].cmd ) {
		   if(aiWangCmdTypes[i].handleFunc) aiWangCmdTypes[i].handleFunc( data,  len);
		   break;
	   }
   }
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
		case GET_BATTERY_LEVEL:{
			uint8_t batflag; //left, right, box
		    batflag = app_battery_current_level();
		    TRACE(0,"%s batflag=%d", __func__, batflag);
#if 0
		    if(batflag > 9)
		    {
		        batflag = 9;
		    }
#endif
			uint8_t batteryArray[3] = {0, 0, 0};
			batteryArray[0]  = batflag;
			if( 0xFF == getPeerBattery())
			{
				batteryArray[1]  = batflag;
			}
			else
			{
				batteryArray[1]  = getPeerBattery();
				#if 0
			    if(batteryArray[1] > 9)
			    {
			    	batteryArray[1] = 9;
			    }
				#endif
			}
			batteryArray[2] = getBoxChargerBattery();
			read_send_data[1] = 3;
			memcpy(&read_send_data[2],batteryArray,2);
			//sparraw_read_rsp_msg(RSP_GET_BATTERY_LEVEL,param->aw_connhdl,param->aw_token, batteryArray, 3);
			sparraw_read_rsp_msg(RSP_GET_BATTERY_LEVEL,param->aw_connhdl,param->aw_token, read_send_data, 3+2);
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
		case GET_KEY_MAPPING:{
#if 0
			const uint8_t keyMaps[2] = {0x00,0x14};
			read_send_data[1] = sizeof(keyMaps)/keyMaps[0];
			memcpy(&read_send_data[2],keyMaps,sizeof(keyMaps)/keyMaps[0]);
			//sparraw_tx_msg(RSP_GET_KEY_MAPPING, (const uint8_t*)&keyMaps[0], (sizeof(keyMaps)/keyMaps[0]));
#else
			uint8_t read_key_map_data[50] = {0};
			uint8_t key_number = 0;
			handleGetKeyMapNumber(&key_number);
			read_key_map_data[1] = key_number*2;
			if(key_number == 0){
				key_number = 20;
				for(int i = 0;i<20; i ++)
				{
					uint8_t action = s_key_default_map[i].actions;
					uint8_t func = s_key_default_map[i].function;
					read_key_map_data[i*2+2] = action;
					read_key_map_data[i*2+1+2] = func;
				}
			}
			else{
				for(int i = 0;i<key_number; i ++)
				{
					uint8_t action = 0;
					uint8_t func = 0;
					handleGetKeyMapActionAndFunc(i,&action,&func);
					read_key_map_data[i*2+2] = action;
					read_key_map_data[i*2+1+2] = func;
				}
			}
			uint8_t data_len = key_number*2 + 2;
			sparraw_read_rsp_msg(RSP_GET_KEY_MAPPING,param->aw_connhdl,param->aw_token, read_key_map_data, data_len);
#endif
			//sparraw_read_rsp_msg(RSP_GET_KEY_MAPPING,param->aw_connhdl,param->aw_token, read_send_data, (sizeof(keyMaps)/keyMaps[0])+2);
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
			read_send_data[1] = strlen((char*)version);
			memcpy(&read_send_data[2],version,strlen((char*)version));
			sparraw_read_rsp_msg(RSP_GET_FW_VERSION, param->aw_connhdl,param->aw_token,read_send_data, strlen((char*)version)+2);		
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


