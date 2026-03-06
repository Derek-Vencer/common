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

#include "nvrecord_bt.h"
#include "nvrecord_env.h"
#include "ICP1205.h"
#include "app_thread.h"
#include "communication_svr.h"
#include "earbud_ux_duplicate_api.h"


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


typedef enum {
	PARSE_IDLE,
	PARSE_HEAD,
	PARSE_CMD,
	PARSE_PAYLOAD,
	PARSE_CRC
}PARSE_STATE;

//static enum PARSE_STATE parSeState = PARSE_IDLE;

typedef struct {
	uint8_t leftEarBudsBattery;
	uint8_t rightEarBudsBattery;
	uint8_t boxChargerBattery;
	uint8_t boxLidsStates;
} BOX_STATUS;

extern void app_tws_ibrt_update_info(ibrt_role_e ibrtRole,bt_bdaddr_t *ibrtPeerAddr);


#define LEFT_BUDS  0
#define RIGHT_BUDS 1
#define MAX_RX_SIZE  64
static uint8_t  isRightEarbuds = 0xFF; //unknown odd  right , even left
static uint8_t  wiredUartInitFlag  = FALSE;
static uint8_t  wiredUartReceiveData[MAX_RX_SIZE];
static BOX_STATUS  boxChargerStatus  ;

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

static void wired_uart_get_battery_level(void)
{
    uint8_t buff[4] = {0};
    buff[0] = 0x55;
    buff[1] = 0xAA;
    buff[2] = app_battery_current_level()%9;
    buff[3] = crc8(buff,3);
    communication_send_buf(buff, 4);
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

static void wired_uart_send_set_peer_addr_ok(void)
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
			  wired_uart_send_set_peer_addr_ok();
			  osDelay(100);
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
    uint8_t address_compare[6] = {0};
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
            if (memcmp(record.bdAddr.address, p_ibrt_ctrl->local_addr.address, BTIF_BD_ADDR_SIZE) &&
                memcmp(record.bdAddr.address, p_ibrt_ctrl->peer_addr.address, BTIF_BD_ADDR_SIZE)&&
                memcmp(record.bdAddr.address, address_compare, BTIF_BD_ADDR_SIZE))
            {
            	nv_record_ddbrec_delete(&record.bdAddr);
            }
        }
    }
    nv_record_flash_flush();
}

static void wired_uart_get_box_battery(uint8_t *data ,uint8_t len)
{
	DBGPRINT("%s boxChargerBattery=0x%02x boxChargerBattery=0x%02x rightEarBudsBattery=0x%02x",
			__func__,
			data[0],
			data[1],
			data[2]
			);
	boxChargerStatus.boxChargerBattery   = data[0];
	boxChargerStatus.leftEarBudsBattery  = data[1];
	boxChargerStatus.rightEarBudsBattery = data[2];
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
        	DBGPRINT("%s crc8 OK %s", __func__, isRightEarbuds?"Right":"Left");
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

    DBGPRINT("%s, cmd_event=0x%02X", __func__, cmd_event);

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
            	 wired_uart_get_ear_addr_handle();
             }
        	 break;
         }
    case CMD_GET_EAR_POWER:		  //Get headphone battery level
         {
        	 if (operateLeftOrRight == isRightEarbuds)
        	 {
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
            app_shutdown();
        }
    	break;
    case CMD_OPEN_CASE:
        {
        	wired_uart_get_battery_level();
        }
    	break;
    case CMD_CLOSE_CASE:
        {
            printf("CMD_CLOSE_CASE poweroff!!!");
            wired_uart_get_battery_level();
            osDelay(30);
            app_shutdown();
        }
    	break;
    case CMD_EAR_RESET:
        {
        	wired_uart_remove_all_phone_paired_list();
        }
    	break;
    case CMD_SET_MAC:
        {
        	if (operateLeftOrRight == isRightEarbuds) {
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
    	  break;
       }
    case  CMD_SET_EARBUD_ENTER_PAIR:
      {
    	  if (operateLeftOrRight == isRightEarbuds)
    	  {
    		   DBGPRINT("CMD_SET_EARBUD_ENTER_PAIR isRightEarbuds=%d!!!", isRightEarbuds);
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
			   wired_uart_send_set_peer_addr_ok();
			   osDelay(100);
			   app_factorymode_enter();
		   }
      }
      break;

    default:
       break;
    }
}

static void wired_uart_communication_post_msg(uint8_t *uart_data, uint8_t len)
{
    APP_MESSAGE_BLOCK msg;
    DBGPRINT("%s", __func__);
    msg.mod_id = APP_MODUAL_AIWANG_WIRED_UART;
    msg.msg_body.message_Param2 = len;
    app_mailbox_put(&msg);
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
    DBGPRINT("%s", __func__);
    data_len = (uint8_t)msg_body->message_Param2;
    DUMP8("%02x ", wiredUartReceiveData, data_len);
    wired_uart_communication_cmd_handle_process(wiredUartReceiveData, data_len);
    return 0;
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
    }
}






