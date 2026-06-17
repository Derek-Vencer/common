#ifndef _UART_H_
#define _UART_H_

#include <stdint.h>
#include "py32f002bx5.h"
//------------------------------------------------------------------------//
#define 		FRAME_HEADER_1						0x55
#define 		FRAME_HEADER_2						0xAA

//Transparent command: CMID
////////////GET////////////////////////
#define 		CMD_CASE_STATE							0x00
#define			CMD_HANDSHAKE   	  		    0x01		  //Handshake instruction
#define			CMD_GET_STATE   						0x02		  //Get headphone status
#define			CMD_GET_MAC  	  		    		0x03			//Obtain the MAC address of the earphones
#define   	CMD_GET_EAR_POWER						0x04			//Get headphone battery level

////////////SET/////////////////////
#define 		CMD_NONE_CASE								0x05
#define			CMD_POWER_OFF    						0x06		  //Headphones enter shipping
#define			CMD_OPEN_CASE   						0x07
#define			CMD_CLOSE_CASE   						0x08
#define 		CMD_EAR_RESET								0x09
#define			CMD_SET_MAC  	  		    		0x0A			//Set headphone MAC address
#define			CMD_PEER_EAR								0x0B
#define 		CMD_VOICE_INDICAT						0x0C		  //Switch voice prompts
#define     CMD_SET_EARBUD_SHIP_MOD     0x0D
#define     CMD_SET_EARBUD_ENTER_PAIR   0x0E
#define     CMD_SEND_BOX_BATTERY_LEVEL  0x0F
#define     CMD_SEND_DUT_MODE           0x10
#define     CMD_SEND_EAR_PUTIN          0x11



#define USART_BAUDRATE  38400
#define UART_Out_Size   0x25
extern uint8_t    CrTxbuf [UART_Out_Size];
#define UART_In_Size 0x25
extern uint8_t    CrRxbuf [UART_In_Size];







void APP_UsartIRQCallback(USART_TypeDef *USARTx);
void APP_ConfigUsart(USART_TypeDef *USARTx);
uint8_t CrSendComm(uint8_t chx_sel, uint8_t cmid);
void 	ICP6108A_ComOutPut5vToEarbuds(void);
#endif

