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

#if defined(USER_TOTA_SPP_SYNC_KEY_EN) || defined(BESUI_COMM_EN)
#include "besui_common.h"
#endif

#ifndef TRACE
#define TRACE(attr, str, ...)   TR_DEBUG(attr, str, ##__VA_ARGS__)
#endif

#define MAX_PACKET_SIZE            (512)
#define SPARRAW_EVENT_MAX_MAILBOX   (5)
#define SPARRAW_EVENT_BUF_SIZE      (MAX_PACKET_SIZE*SPARRAW_EVENT_MAX_MAILBOX)
#define SPARRAW_BUFF_SIZE           (4096)

typedef struct {
    uint8_t     devId;
    uint8_t     event;
    uint16_t    len;
    uint8_t     data[512];
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
static uint8_t sparraw_tx_buf[SPARRAW_EVENT_BUF_SIZE] = {0};
static CQueue  sparraw_rx_cqueue;
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
static uint8_t   payload_buffer[2048];
static uint16_t  rx_param_len = 0;

static osThreadId sparrow_thread_id = NULL;
static void sparraw_event_handler_thread(const void *arg);
osThreadExDef(sparraw_event_handler_thread, osPriorityAboveNormal, 1, 1024*3, "sparraw_ble_thread", 1U);

osMutexId app_sparraw_buf_lock;
osMutexDef(app_sparraw_buf_lock);
static osMailQId sparraw_event_mailbox_id = NULL;
osMailQDef(sparraw_event_mailbox_id, SPARRAW_EVENT_MAX_MAILBOX, SPARRAW_MESSAGE_BLOCK);


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

    for (uint8_t i=0; i< mailbox_cnt; i++)
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
    }

    msg_p = (SPARRAW_MESSAGE_BLOCK*)osMailAlloc(sparraw_event_mailbox_id, 0);
    if (msg_p == NULL)
    {
    	sparraw_event_mailbox_free_all();
        msg_p = (SPARRAW_MESSAGE_BLOCK*)osMailAlloc(sparraw_event_mailbox_id, 0);
        ASSERT(msg_p, "osMailAlloc error");
    }

    msg_p->devId = devId;
    msg_p->event = event;
    msg_p->len = (len >512?512:len);
    if (param && len)
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

static void sparraw_rx_cmd_init(void){
	sparraw_rx_state = RX_IDLE;
	rx_param_len = 0;
	memset(&rxDataStruct, 0, sizeof(REQUEST_DATA_STRUCT));
}

static void sparraw_rx_cmd_handler(void){
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

static void sparraw_rx_cmd_parse(const uint8_t *data, uint16_t len) {
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

static void sparraw_event_handler_thread(void const *argument)
{
    while (true)
    {
    	SPARRAW_MESSAGE_BLOCK* rx_event = NULL;
        if (sparraw_event_mailbox_get(&rx_event))
            return;
        //TRACE(2, "%s ", __func__);
        osMutexWait(app_sparraw_buf_lock, osWaitForever);
		REL_TRACE_NOCRLF(0, "SPARRAW_SRV_RX: %d ", rx_event->event);
		DUMP8("%02X ", &rx_event->data[0], rx_event->len);
        if (BLE_AIWANG_SRV_RX == rx_event->event) {
            sparraw_rx_cmd_parse(&rx_event->data[0], rx_event->len);
        }
        sparraw_event_mailbox_free(rx_event);
        osMutexRelease(app_sparraw_buf_lock);
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
    TRACE(0,"[%s]",__func__);

    InitCQueue(&sparraw_rx_cqueue, SPARRAW_EVENT_BUF_SIZE, ( CQItemType * )sparraw_tx_buf);
    sparraw_event_mailbox_init();
    sparrow_thread_id = osThreadCreate(osThread(sparraw_event_handler_thread), NULL);
    app_sparraw_buf_lock = osMutexCreate(osMutex(app_sparraw_buf_lock));
    if (app_sparraw_buf_lock == NULL) {
        TRACE(1, "Failed to Create ota buf lock\n");
        return;
    }
}

void sparraw_service_init(void)
{
	TRACE(0,"[%s]",__func__);
    sparraw_rx_thread_init();
   // ble_aiwang_srv_init();
    ble_aiwang_srv_register_event_cb(sparraw_event_handle);
}


