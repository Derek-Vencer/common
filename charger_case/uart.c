#include "main.h"



#define DEBUG_USART_TX_GPIO_PORT0                GPIOB
#define DEBUG_USART_TX_GPIO_CLK_ENABLE0()        LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOB)
#define DEBUG_USART_TX_PIN0                      LL_GPIO_PIN_4
#define DEBUG_USART_TX_AF0                       LL_GPIO_AF_1



#define CrRxbuf_size 0x25
uint8_t *TxBuff = NULL;
__IO uint16_t TxCount = 0;

uint8_t *RxBuff = NULL;
//__IO uint16_t RxCount = 0;
uint8_t RxCount = 0;

uint8_t  u8RxHeadflg=0;
uint8_t  u8RxHeadDat=0;
uint8_t  RXDatalong=0;	
uint8_t  TXDatalong=0;


uint8_t  CrTxbuf [UART_Out_Size];
uint8_t  CrRxbuf [UART_In_Size];

uint8_t CH1CrRxbuf[7];
uint8_t CH2CrRxbuf[7];

uint8_t  gu8left_ear_bat,gu8right_ear_bat;
uint8_t  gLastLeftEarBat, gLastRightEarBat;
uint8_t  gLastBoxChargeBat;




void APP_ConfigUsart(USART_TypeDef *USARTx)
{

  LL_GPIO_InitTypeDef GPIO_InitStruct;
	LL_USART_Disable(USARTx);	
/*Enable clock, initialization pin, enable NVIC interrupt*/
  if (USARTx == USART1) 
  {

    LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);
    /*Enable USART1 clock*/
    LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_USART1);
    


    GPIO_InitStruct.Pin = LL_GPIO_PIN_7;//

    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;

    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;

    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;		

    GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
    /*Reuse as USART1 function*/
    GPIO_InitStruct.Alternate = LL_GPIO_AF1_USART1;
    /*GPIOA Init*/
    LL_GPIO_Init(GPIOA,&GPIO_InitStruct);

	  LL_USART_ConfigHalfDuplexMode(USARTx);		


    NVIC_SetPriority(USART1_IRQn,0);

    NVIC_EnableIRQ(USART1_IRQn);
	
  }
   
/*Enable UART module*/

	LL_USART_SetBaudRate(DEBUG_USART, SystemCoreClock, LL_USART_OVERSAMPLING_16, USART_BAUDRATE);
  LL_USART_SetDataWidth(DEBUG_USART, LL_USART_DATAWIDTH_8B);
  LL_USART_SetStopBitsLength(DEBUG_USART, LL_USART_STOPBITS_1);
  LL_USART_SetParity(DEBUG_USART, LL_USART_PARITY_NONE);
  LL_USART_SetHWFlowCtrl(DEBUG_USART, LL_USART_HWCONTROL_NONE);
  LL_USART_SetTransferDirection(DEBUG_USART, LL_USART_DIRECTION_TX_RX);
// 	CLEAR_BIT(USARTx->CR2, USART_CR2_CLKEN);
// 	SET_BIT(USARTx->CR3, USART_CR3_HDSEL);
  LL_USART_ClearFlag_TC(USARTx);
  LL_USART_Enable(USARTx);
}

void BSP_USART_Config0(void)
{
  DEBUG_USART_CLK_ENABLE();

  /* USART Init */
  LL_USART_SetBaudRate(DEBUG_USART, SystemCoreClock, LL_USART_OVERSAMPLING_16, DEBUG_USART_BAUDRATE);
  LL_USART_SetDataWidth(DEBUG_USART, LL_USART_DATAWIDTH_8B);
  LL_USART_SetStopBitsLength(DEBUG_USART, LL_USART_STOPBITS_1);
  LL_USART_SetParity(DEBUG_USART, LL_USART_PARITY_NONE);
  LL_USART_SetHWFlowCtrl(DEBUG_USART, LL_USART_HWCONTROL_NONE);
  LL_USART_SetTransferDirection(DEBUG_USART, LL_USART_DIRECTION_TX_RX);
  LL_USART_Enable(DEBUG_USART);
  LL_USART_ClearFlag_TC(DEBUG_USART);

  /**USART GPIO Configuration
    PA3    ------> USART1_TX
    PB5     ------> USART1_RX
    */
  DEBUG_USART_RX_GPIO_CLK_ENABLE();
  DEBUG_USART_TX_GPIO_CLK_ENABLE0();

  LL_GPIO_SetPinMode(DEBUG_USART_TX_GPIO_PORT0, DEBUG_USART_TX_PIN0, LL_GPIO_MODE_ALTERNATE);
  LL_GPIO_SetPinSpeed(DEBUG_USART_TX_GPIO_PORT0, DEBUG_USART_TX_PIN0, LL_GPIO_SPEED_FREQ_VERY_HIGH);
  LL_GPIO_SetPinPull(DEBUG_USART_TX_GPIO_PORT0, DEBUG_USART_TX_PIN0, LL_GPIO_PULL_UP);
  LL_GPIO_SetAFPin_0_7(DEBUG_USART_TX_GPIO_PORT0, DEBUG_USART_TX_PIN0, DEBUG_USART_TX_AF0);

  LL_GPIO_SetPinMode(DEBUG_USART_RX_GPIO_PORT, DEBUG_USART_RX_PIN, LL_GPIO_MODE_ALTERNATE);
  LL_GPIO_SetPinSpeed(DEBUG_USART_RX_GPIO_PORT, DEBUG_USART_RX_PIN, LL_GPIO_SPEED_FREQ_VERY_HIGH);
  LL_GPIO_SetPinPull(DEBUG_USART_RX_GPIO_PORT, DEBUG_USART_RX_PIN, LL_GPIO_PULL_UP);
  LL_GPIO_SetAFPin_0_7(DEBUG_USART_RX_GPIO_PORT, DEBUG_USART_RX_PIN, DEBUG_USART_RX_AF);
}


/**
  * @brief  USART sending function
  * @param  USARTx: USART module, which can be USART1 USART2
  * @param  pData： Send buffer
  * @param  Size： Send buffer size
  * @retval nothing
  */
static void APP_UsartTransmit(USART_TypeDef *USARTx, uint8_t *pData, uint16_t Size)
{
  TxBuff = pData;
  TxCount = Size;
  
  /* Send data */ 
  while (TxCount > 0)
  {
		
    /* Wait for the TXE flag to be set */
    while(LL_USART_IsActiveFlag_TXE(USARTx) != 1)
		{
#if 0
			printf("UART_DIE\n");
#endif
		}
    /* Send data */
    LL_USART_TransmitData8(USARTx, *TxBuff);
    TxBuff++;
    TxCount--;
  }
  
  /* Wait for the TC flag to be set */ 
  while(LL_USART_IsActiveFlag_TC(USARTx) != 1);
}

/**
  * @brief  USART receiving function
  * @param  USARTx: USART module, which can be USART1 USART2
  * @param  pData： Receive buffer
  * @param  Size： Receive buffer size
  * @retval nothing
  */
//static void APP_UsartReceive(USART_TypeDef *USARTx, uint8_t *pData, uint16_t Size)
//{
//  RxBuff = pData;
//  RxCount = Size;
//  
//  /*接收数据*/
//  while (RxCount > 0)
//  {
//    /* 等待RXNE标志位置位 */
//    while(LL_USART_IsActiveFlag_RXNE(USARTx) != 1);
//    /* 接收数据 */
//    *RxBuff = LL_USART_ReceiveData8(USARTx);
//    RxBuff++;
//    RxCount--;
//  }
//}
static void APP_UsartReceive_IT(USART_TypeDef *USARTx, uint8_t *pData, uint16_t Size)
{
	static uint8_t i;
	RxBuff = pData;
	RxCount = Size;
	u8RxHeadflg =0;
	u8RxHeadDat =0;
	for(i = 0;i<Size-1;i++)
	{
		*RxBuff = 0;
		RxBuff++;
	}
	RxBuff = pData;
///* Enable receiving parity check error interrupts*/
//  LL_USART_EnableIT_PE(USARTx);

///* Enable receiving error interrupts*/
//  LL_USART_EnableIT_ERROR(USARTx);

/*Enable non empty interrupt of receiving data register*/
//	if((LL_USART_IsActiveFlag_TC(USARTx))&&(LL_USART_IsActiveFlag_TXE(USARTx)))
//  {
	     LL_USART_EnableIT_RXNE(USARTx);
//	}

}
void APP_UsartIRQCallback(USART_TypeDef *USARTx)
{
	uint8_t temp;
 /*The receiving data register is not empty*/

    if ((LL_USART_IsActiveFlag_RXNE(USARTx) != RESET) && (LL_USART_IsEnabledIT_RXNE(USARTx) != RESET))
    {

			if(RxCount <= CrRxbuf_size)
			{

				temp = LL_USART_ReceiveData8(USARTx);
#if 0				
				if(u8RxHeadDat == 0)
				{
					if(temp == FRAME_HEADER_1)
					{
						u8RxHeadDat = 1;
					}
					else
					{
						return;
					}
				}
#endif
					CrRxbuf[CrRxbuf_size-RxCount] =	temp;
					if(RxCount)RxCount--;	
					RXDatalong = CrRxbuf_size-RxCount;
		
			}
      else
      {
				LL_USART_ClearFlag_RXNE(USARTx);
        LL_USART_DisableIT_RXNE(USARTx);
        LL_USART_DisableIT_PE(USARTx);
        LL_USART_DisableIT_ERROR(USARTx);    
        //UartReady = SET;
      }
      return;
    }
		else
		{
			LL_USART_ClearFlag_RXNE(USARTx);
      LL_USART_DisableIT_RXNE(USARTx);
		}
}

///**
// *******************************************************************************
// ** \brief CRCMODBUS
// **
//* * \ param [in] data address, CRC result returns address, calculates array length
// **
//* * \ retal void returns nothing
// ******************************************************************************/
#if 0
uint8_t ModBusCRC8(uint8_t *prt,uint8_t len)
{
    uint8_t i; 
	  uint8_t crc=0x00; /*  Calculate the initial CRC value*/
	//    pnt_UART_Out		=	CrTxbuf;// Record the RAM address of the first data

    while(len--)
    {
      crc ^= *prt++;  /*XOR with the data that needs to be calculated each time,
						 and point to the next data after calculation*/
			i=8;
			do     
			{ 
							if (crc & 0x01)
									crc = (crc >>1) ^ 0x8C;
							else
									crc = (crc >> 1);
			} while(--i);         //  dzsn    count1;


    }
	return crc;
}
#else
#include <stdint.h>
#include <stddef.h>

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

uint8_t ModBusCRC8(const uint8_t *data, uint32_t length)
{
    uint8_t crc = 0x00;

    while (length--)
    {
        crc = crc8_table[crc ^ *data];
        data++;
    }
    return crc;
}

#endif

static const uint8_t boxVersion[12] = "V0.1.0"; //"01.00.00.03";

uint8_t comm_build_msg_header(uint8_t chx_sel, uint8_t cmid) 
{
	uint8_t    Datalong = 8;

  CrTxbuf[0] = 0x55;
  CrTxbuf[1] = 0xAA;
  CrTxbuf[2] = cmid;
  CrTxbuf[3] = chx_sel|0x40;

	//---------------------------Code to be sent----------------------------------//
	if(cmid == CMD_SET_MAC)
	{
		if(chx_sel == OnlyCh1)
		{
			for(uint8_t i = 0; i < 6; i++)
			{
				CrTxbuf[5 + i] = CH2CrRxbuf[i];
			}
		}
		else if(chx_sel == OnlyCh2)
		{
			for(uint8_t i = 0; i < 6; i++)
			{
				CrTxbuf[5 + i] = CH1CrRxbuf[i]; //5 6 7 8 9 10
			}
		}
		Datalong = 12;
	}
	else if(cmid == CMD_SEND_BOX_BATTERY_LEVEL)
	{
		 CrTxbuf[4]  = gSbatsts.gu8batlv;
	   if(chx_sel == OnlyCh1)
		 {
			 CrTxbuf[5]  = gu8left_ear_bat;
		 }
		 else if(chx_sel == OnlyCh2)
		 {
			 CrTxbuf[6]  = gu8right_ear_bat;
		 }
	}
	else if (CMD_GET_EAR_POWER == cmid)
	{
	   //memcpy(&CrTxbuf[4], &boxVersion[0], 12);
		// Datalong = 17;
		memcpy(&CrTxbuf[4], &boxVersion[0], 7);
		Datalong = 12;
	}
	/*else
	{
		Datalong = 8;
	}*/

	CrTxbuf[Datalong -1] = ModBusCRC8(CrTxbuf,Datalong-1);
	
	return Datalong;

}

//const uint8_t testData[10] = "123456";
//static int enable_send_uart_data = 1;
uint8_t CrSendComm(uint8_t chx_sel, uint8_t cmid)
{
#if 0
	if(cmid == 0xf1)
	{
		enable_send_uart_data = 0;
		return 0;
	}
	else if(cmid == 0xf2)
	{
		enable_send_uart_data = 1;
		return 0;
	}
	if(enable_send_uart_data == 0)
	{
		return 0;
	}
#endif	
	TXDatalong = comm_build_msg_header(chx_sel, cmid);
	//---------------------------6108 configuration sending----------------------------------//
	
/*	
	LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_7, LL_GPIO_MODE_OUTPUT);
	LL_GPIO_ResetOutputPin(GPIOA,LL_GPIO_PIN_7); //Lower by 10ms
	Icp6108ChxFuncfg(chx_sel,ChnxTrComSndEnable); //0x04 Switch to transparent transmission mode
	LL_mDelay(200); //15ms
	Icp6108ChxFuncfg(chx_sel,0x01); //0x04 Switch to transparent transmission mode
	LL_mDelay(500); //15ms
*/
	
	I2C_MasterWriteByte(ICP6108_CHN_CON,0x01); //0x03 -- 3.3V //0x01 //Switching the coding level to 1.8V
	
#if USING_ICP1205 //If combined with ICP1205, waveform pulling is required to make 1205 enter transparent transmission
	LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_7, LL_GPIO_MODE_OUTPUT);
	LL_GPIO_ResetOutputPin(GPIOA,LL_GPIO_PIN_7); //Lower by 10ms
	Icp6108ChxFuncfg(chx_sel,ChnxTrComSndEnable); //0x04 Switch to transparent transmission mode
	LL_mDelay(15); //26 //200ms //15ms
	APP_ConfigUsart(USART1);
	LL_mDelay(10); //15 //5ms
#else
  APP_ConfigUsart(USART1);
  Icp6108ChxFuncfg(chx_sel,ChnxTrComSndEnable);	//Switch to transparent transmission mode
#endif	

	APP_UsartTransmit(USART1,(uint8_t*)CrTxbuf,TXDatalong);
	//APP_UsartTransmit(USART1,(uint8_t*)testData,6);

	Icp6108ChxFuncfg(chx_sel,ChnxTrComRecEnable);	//0x0C Switch to transparent transmission receiving mode

	//LL_mDelay(30);		 // Receive and wait for 30ms

	APP_UsartReceive_IT(USART1, (uint8_t*)CrRxbuf, CrRxbuf_size);

	LL_mDelay(30);       // Receive and wait for 30ms
		
	//Received data processing------------------------------//
	Icp6108ChxFuncfg(chx_sel,ChnxIdle6OutV5P0Sub2vgs); //Processing data after receiving data and switching states
	I2C_MasterWriteByte(ICP6108_CHN_CON,0x00); //Switching the coding level to 5V
	

#if 0
	for(uint8_t i = 0; i < 8; i++)
	{
		printf("CrRxbuf = 0x%x\n",CrRxbuf[i]);  
		//CrTxbuf[5+i] = CrRxbuf[i];
	}
	//APP_UsartTransmit(USART1,(uint8_t*)CrRxbuf,8+5);
	// printf("RXDatalong = 0x%x\n",RXDatalong); 	
#endif

	if(CrRxbuf[0] != FRAME_HEADER_1 || CrRxbuf[1] != FRAME_HEADER_2) return 0;
	if(CrRxbuf[RXDatalong-1] != ModBusCRC8(CrRxbuf,RXDatalong-1))
	{
#if 0
	  printf("RXDatalong = 0x%x\n",RXDatalong); 	
#endif
		return 0;
	}

	switch(cmid)
	{
		case CMD_OPEN_CASE:
				 if(chx_sel == OnlyCh1)
				 {
						gu8left_ear_bat = CrRxbuf[2];
						boxnewsts.BoxChn1Exs_f = InBox;
						//CrSendComm(OnlyCh1, 0xd5);
				 }
				 else
				 {
						gu8right_ear_bat = CrRxbuf[2];
						boxnewsts.BoxChn2Exs_f = InBox;
						//CrSendComm(OnlyCh1, 0xd6);
				 }
			break;
		case CMD_CLOSE_CASE:
				 if(chx_sel == OnlyCh1)
				 {
						gu8left_ear_bat = CrRxbuf[2];
						boxnewsts.BoxChn1Exs_f=InBox;
						//CrSendComm(OnlyCh1, 0xd7);
				 }
				 else
				 {
						gu8right_ear_bat = CrRxbuf[2];
						boxnewsts.BoxChn2Exs_f=InBox;
						//CrSendComm(OnlyCh1, 0xd8);
				 }	
			break;		
		case CMD_GET_EAR_POWER:
				 if(chx_sel == OnlyCh1)
				 {
						gu8left_ear_bat = CrRxbuf[2];
						boxnewsts.BoxChn1Exs_f=InBox;
						//CrSendComm(OnlyCh1, 0xd9);
				 }
				 else
				 {
						gu8right_ear_bat = CrRxbuf[2];
						boxnewsts.BoxChn2Exs_f=InBox;	
						//CrSendComm(OnlyCh1, 0xda);
				 }			
		  break;
		case	 CMD_PEER_EAR:
	      break;
		case CMD_GET_MAC:
				 if(chx_sel == OnlyCh1)
				 {
								for(uint8_t i= 0;i<=6;i++)
								{
									CH1CrRxbuf[i] = CrRxbuf[i+2];
								}		
				 }
				 else
				 {
								for(uint8_t i= 0;i<=6;i++)
								{
									CH2CrRxbuf[i] = 	CrRxbuf[i+2];
								}
				 }				
		     break;
		case CMD_SET_MAC:	
		case CMD_SEND_DUT_MODE:
		case CMD_SET_EARBUD_SHIP_MOD:
    case CMD_SEND_EAR_PUTIN:
			   if(chx_sel == OnlyCh1)
				 {
					  if(('O' == CrRxbuf[2]) && ('K' == CrRxbuf[3])) {
							  boxnewsts.BoxSetCh1PeerAddrOk = 1;
						}
				 }
				 else
				 {
					  if(('O' == CrRxbuf[2]) && ('K' == CrRxbuf[3])) {
							  boxnewsts.BoxSetCh2PeerAddrOk = 1;
						}
				 }	
					break;
		case CMD_POWER_OFF:
					break;		
		
	}
	return 1;

}
/*
** Brief :
**
*/
void 	ICP6108A_ComOutPut5vToEarbuds(void)
{
   I2C_MasterWriteByte(ICP6108_CHN_CON,0x00); //Switching the coding level to 5V
   LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_7, LL_GPIO_MODE_OUTPUT);
}


