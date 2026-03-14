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
static uint8_t mailbox_cnt = 0;

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
static void sparraw_tx_msg(uint8_t rsp_type, const uint8_t* data, uint16_t len);


extern "C" void system_get_info(uint8_t *fw_rev_0, uint8_t *fw_rev_1, uint8_t *fw_rev_2, uint8_t *fw_rev_3);

void handleGetBatteryLevel(const uint8_t *data, uint16_t len)
{

    uint8_t batflag; //left, right, box
    batflag = app_battery_current_level();
    TRACE(0,"%s batflag=%d", __func__, batflag);
    if(batflag > 9)
    {
        batflag = 9;
    }

	uint8_t batteryArray[3] = {0, 0, 0};
	batteryArray[0]  = batflag;
	if( 0xFF == getPeerBattery())
	{
		batteryArray[1]  = batflag;
	}
	else
	{
		batteryArray[1]  = getPeerBattery();
	    if(batteryArray[1] > 9)
	    {
	    	batteryArray[1] = 9;
	    }
	}
	batteryArray[2] = getBoxChargerBattery();
	sparraw_tx_msg(RSP_GET_BATTERY_LEVEL, batteryArray, 3);

}

void handleGetDeviceName(const uint8_t *data, uint16_t len)
{
	TRACE(0,"%s.", __func__);
	uint8_t* localname =  factory_section_get_bt_name();
    if(localname)
    {
    	sparraw_tx_msg(RSP_GET_DEVICE_NAME, (const uint8_t*)localname, strlen((const char *)localname)+1);
    }
}

void handleSetDeviceName(const uint8_t *data, uint16_t len)
{
	TRACE(0,"%s.", __func__);
	//if( factory_section_set_bt_name((const char *)&data[1], len -1))
	char nameBuffer[248+1] = {0};
	len = (len - 1) > 248?248:(len -1);
	if (len > 0)
	{
		memcpy(nameBuffer, &data[1], len);
		if( factory_section_set_bt_name(nameBuffer, len+1))
		{
			TRACE(0,"%s error", __func__);
		}
	}
	sparraw_tx_msg(RSP_SET_DEVICE_NAME, (const uint8_t*)"", 0);
}

void handleGetKeyMapping(const uint8_t *data, uint16_t len)
{
	TRACE(0,"%s.", __func__);
	const uint8_t keyMaps[2] = {0x00,0x14};
	sparraw_tx_msg(RSP_GET_KEY_MAPPING, (const uint8_t*)&keyMaps[0], (sizeof(keyMaps)/keyMaps[0]));
}

void handleSetKeyMapping(const uint8_t *data, uint16_t len)
{
	TRACE(0,"%s.", __func__);
	sparraw_tx_msg(RSP_SET_KEY_MAPPING, (const uint8_t*)"", 0);
}

void handleGetEqPresent(const uint8_t *data, uint16_t len)
{
	TRACE(0,"%s.", __func__);
	sparraw_tx_msg(RSP_GET_EQ_PRESET, 0, 1);
}

void handleSetEqPresent(const uint8_t *data, uint16_t len)
{
	TRACE(0,"%s.", __func__);
	sparraw_tx_msg(RSP_GET_EQ_PRESET, (const uint8_t*)"", 0);
}

//MM.NN.RR.AA
void handleGetFwVersion(const uint8_t *data, uint16_t len)
{
	TRACE(0,"%s.", __func__);
	//uint8_t version[12] = {'0','1','0','6'};
	//system_get_info(&version[0], &version[1], &version[2], &version[4]);
	const uint8_t *version = (const uint8_t *)"01.01.00.02";
	sparraw_tx_msg(RSP_GET_FW_VERSION, (const uint8_t*)version, 4);
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
    nvrecord_env->sn_len = len;
    for(int i = 0; i < len; i++)
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
		  if(app_sparraw_env.notifyEnable) { ble_aiwang_srv_send_data_via_notification(tempSn, strlen((const char*)tempSn)); }
	  } else if (WRTIE_SN == data[1]) {
		  memcpy(tempSn, &data[2], (len-2) > 12 ? 12 :(len-2));
		  TRACE(0, "WRTIE_SN:%s", tempSn);
		  aiWangSetSn(tempSn, (len-2) > 12 ? 12 :(len-2));
		  if(app_sparraw_env.notifyEnable) ble_aiwang_srv_send_data_via_notification(tempSn, len);
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
		  ble_aiwang_srv_send_data_via_notification(buff, 8);
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
		  ble_aiwang_srv_send_data_via_notification((uint8_t*)(ret?"OK":"NG"), 2);
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
    mailbox_cnt = 0;
    app_sparraw_env.notifyEnable = false;
}

static int32_t sparraw_event_mailbox_init(void)
{
    sparraw_event_mailbox_id = osMailCreate(osMailQ(sparraw_event_mailbox_id), NULL);
    if (sparraw_event_mailbox_id == NULL) {
        OTA_TRACE(0, "Failed to Create app_ota_event_mailbox");
        return -1;
    }
    return 0;
}

static osStatus sparraw_event_mailbox_free(SPARRAW_MESSAGE_BLOCK* rx_event)
{
    osStatus status;
    status = osMailFree(sparraw_event_mailbox_id, rx_event);
    ASSERT(osOK == status, "Free sparraw rx event mailbox failed!");
    if(mailbox_cnt > 0) mailbox_cnt--;
    return status;
}

int sparraw_event_mailbox_free_all(void)
{
	SPARRAW_MESSAGE_BLOCK *msg_p = NULL;
    int status = osOK;
    osEvent evt;
    for (uint8_t i = 0; i< mailbox_cnt; i++)
    {
        evt = osMailGet(sparraw_event_mailbox_id, 500);
        if (evt.status == osEventMail)
        {
            msg_p = (SPARRAW_MESSAGE_BLOCK *)evt.value.p;
            status = sparraw_event_mailbox_free(msg_p);
        }
        else
        {
            GFPS_TRACE(1, "%s get mailbox timeout!!!", __func__);
            continue;
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
        // OTA_TRACE(0, "flag %d len %d", (*rx_event)->flag,(*rx_event)->len);
        return 0;
    }
    return -1;
}

int sparraw_mailbox_put(uint8_t devId, uint8_t event, uint8_t *param, uint16_t len)
{
    osStatus status = osOK;
    SPARRAW_MESSAGE_BLOCK *msg_p = NULL;
    if (mailbox_cnt > SPARRAW_EVENT_MAX_MAILBOX) {
    	TRACE(0, "%s mail overflow mailbox_cnt=%d", __func__, mailbox_cnt);
    	return -1;
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
        GFPS_TRACE(2,"%s error status 0x%02x", __func__, status);
        return (int)status;
    }
    mailbox_cnt++;
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
}

static void sparraw_tx_msg(uint8_t rsp_type, const uint8_t* data, uint16_t len) {
	uint8_t  rsp_buffer[64];
	rsp_buffer[0] = rsp_type;
	if (NULL != data && len > 0)
	{
		memcpy(&rsp_buffer[1], data, len);
	}
	if(app_sparraw_env.notifyEnable) ble_aiwang_srv_send_data_via_notification(rsp_buffer, len + 1);
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
            return;
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
}

void sparraw_disconnected(void)
{
	TRACE(0,"%s.", __func__);
	sparraw_ble_init();
	sparraw_event_mailbox_free_all();
	sparraw_rx_cmd_init();
}

void sparraw_mtu(uint16_t mtu)
{
	TRACE(0,"%s.", __func__);
	app_sparraw_env.Mtu = mtu;
}

void sparraw_event_handle(ble_aiwang_param_u *param)
{
    ASSERT(param,"sparraw data is null.");
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

void sparraw_rx_thread_init(void)
{
    TRACE(0,"[%s] %d ",__func__, sizeof(aiWangCmdTypes)/sizeof(aiWangCmdTypes[0]));
    //InitCQueue(&sparraw_rx_cqueue, SPARRAW_EVENT_BUF_SIZE, ( CQItemType * )sparraw_tx_buf);
    sparraw_event_mailbox_init();
    app_sparraw_buf_lock = osMutexCreate(osMutex(app_sparraw_buf_lock));
    if (app_sparraw_buf_lock == NULL) {
        TRACE(1, "Failed to Create ota buf lock\n");
        return;
    }
    sparrow_thread_id = osThreadCreate(osThread(sparraw_event_handler_thread), NULL);

}

void sparraw_service_init(void)
{
	TRACE(0,"[%s]",__func__);
	// ble_aiwang_srv_init();
    sparraw_rx_thread_init();
	ble_aiwang_srv_register_event_cb(sparraw_event_handle);

    //start charger_manager_thread
    //previous in apps_init,prior settings ICP1205_ADS
    //charger_manager_start();
}


