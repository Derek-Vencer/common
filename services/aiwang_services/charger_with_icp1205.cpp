/*
 * ble_sparrow_server.cpp
 *
 *  Created on: 2025年12月11日
 *      Author: zhangyijun
 */

#include <stdio.h>
#include <string.h>
#include "cmsis_os.h"
#include "cmsis.h"
#include "cqueue.h"
#include "pmu.h"
#include "hal_iomux_best1306p.h"
#include "plat_addr_map_best1306p.h"
#include "hal_gpio.h"
#include "hal_i2c.h"
#include "hal_trace.h"
#include "ICP1205.h"
#include "charger_ntc.h"

#include "charger_with_icp1205.h"



//#define EARBUDS_ICP1205_I2C_ADDRESS 0xC2
#define EARBUDS_ICP1205_I2C_ADDRESS  (0xC2 >> 1)
#define ICP1205_I2C_ADDRESS_WRITE    (0xC2)
#define ICP1205_I2C_ADDRESS_READ     (0xC3)

#define ICP1205_GP0_GPIO             (HAL_GPIO_PIN_P0_4)  //outPut
#define ICP1205_INT_GPIO             (HAL_GPIO_PIN_P0_5)  //outPut
#define ICP1205_GP0_IOMUX            (HAL_IOMUX_PIN_P0_4) //(HAL_IOMUX_PIN_LED_NUM) //(HAL_IOMUX_PIN_P0_4) //outPut
#define ICP1205_INT_IOMUX            (HAL_IOMUX_PIN_P0_5) //(HAL_IOMUX_PIN_LED_NUM) //(HAL_IOMUX_PIN_P0_5) //outPut

#ifndef HAL_I2C_ID_3
#define HAL_I2C_ID_3                 ((HAL_I2C_ID_T)3)
#endif

#undef printf
#undef DBGPRINT

//#define printf(fmt,...)     hal_trace_printf(2, fmt, ##__VA_ARGS__)
//#define DBGPRINT(fmt,...)   hal_trace_printf(0, fmt, ##__VA_ARGS__)
#undef printf
#define printf(fmt, ...) \
    hal_trace_printf(0, "[goc-1205] " fmt, ##__VA_ARGS__)
#undef DBGPRINT
#define DBGPRINT(fmt,...)  \
	hal_trace_printf(2, "[goc-1205] " fmt, ##__VA_ARGS__)


static osThreadId charger_manager_thread_id = NULL;
static void charger_manager_handler_thread(const void *arg);
osThreadExDef(charger_manager_handler_thread, osPriorityAboveNormal, 1, 1024*3, "charger_manager_thread", 1U);


static void ICP1205_GPIO_INT_IrqHandler(enum HAL_GPIO_PIN_T pin);
/*********************************************************************************
 ** \brief ICP1205_BES_I2c_Init
 **
 ** \param [in]  无
 **
 ** \retval void            返回无
 ******************************************************************************/
void ICP1205_BES_I2c_Init(void)
{
	struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux_iic[] = {
        {HAL_IOMUX_PIN_P0_6, HAL_IOMUX_FUNC_MCU_I2C_M3_SCL, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE},
        {HAL_IOMUX_PIN_P0_7, HAL_IOMUX_FUNC_MCU_I2C_M3_SDA, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE},
        {ICP1205_GP0_IOMUX, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE},
        {ICP1205_INT_IOMUX, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE},
    };

    DBGPRINT("%s", __func__);

	hal_i2c_close(HAL_I2C_ID_3);
    hal_iomux_init(pinmux_iic, ARRAY_SIZE(pinmux_iic));

	struct HAL_I2C_CONFIG_T i2c_cfg;
	i2c_cfg.mode = HAL_I2C_API_MODE_TASK;
    i2c_cfg.use_dma  = 0;
    i2c_cfg.use_sync = 1;
	i2c_cfg.speed = 100*1000; //100k
    i2c_cfg.as_master = 1;
    i2c_cfg.addr_as_slave  = 0;
    i2c_cfg.rising_time_ns = 0;
    uint32_t ret = hal_i2c_open(HAL_I2C_ID_3, &i2c_cfg);
    printf("%s open I2C Failed: 0x%x", __func__, ret);
}

/*********************************************************************************
 ** \brief writeDataTo_ICP1205
 **
 **  reg ::I2CP1205_ADS register , data ,length
 **
 ** \return write status 0 success ,others failed
 ******************************************************************************/

uint32_t writeDataTo_ICP1205(unsigned char reg, unsigned char *data, unsigned char length)
{
    uint8_t buf[256];
    buf[0] = reg;
    if (length > sizeof(buf)) {
    	printf("%s length over flow", __func__);
    	return 1;
    }
    memcpy(&buf[1], data, length);
    return hal_i2c_task_msend(HAL_I2C_ID_3, EARBUDS_ICP1205_I2C_ADDRESS, buf, length+1, 1, 0, NULL);
}

/*********************************************************************************
 ** \brief readDataFrom_ICP1205
 **
 **  reg ::I2CP1205_ADS register , data , length
 **
 ** \return read status 0 success ,others failed
 ******************************************************************************/
uint32_t readDataFrom_ICP1205(unsigned char reg, unsigned char *data, unsigned char length)
{
	uint32_t ret = hal_i2c_task_mrecv(HAL_I2C_ID_3, EARBUDS_ICP1205_I2C_ADDRESS, &reg, 1, data, length, HAL_I2C_RESTART_AFTER_WRITE, 0, 0 );
	return ret;
}

/*********************************************************************************
 ** \brief ICP1205_GPIO_INT_IRQ_Disable
 **
 **  reg ::I2CP1205_ADS INT
 **
 ** \return read status 0 success ,others failed
 ******************************************************************************/
static bool IntReqHasEnable = false;

static void ICP1205_GPIO_INT_IRQ_Disable(enum HAL_GPIO_PIN_T pin)
{
	struct HAL_GPIO_IRQ_CFG_T gpio_cfg;
	if (!IntReqHasEnable) return;
	IntReqHasEnable     = false;
	gpio_cfg.irq_enable = false;
	gpio_cfg.irq_debounce = false;
	gpio_cfg.irq_polarity = HAL_GPIO_IRQ_POLARITY_LOW_FALLING;
	gpio_cfg.irq_handler = NULL;
	gpio_cfg.irq_type = HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE;//HAL_GPIO_IRQ_TYPE_LEVEL_SENSITIVE;//HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE;
    hal_gpio_setup_irq(pin, &gpio_cfg);
}

static void ICP1205_GPIO_INT_IRQ_Enable(enum HAL_GPIO_PIN_T pin)
{
	struct HAL_GPIO_IRQ_CFG_T gpio_cfg;
	if (IntReqHasEnable) return;

    hal_gpio_pin_set_dir(pin, HAL_GPIO_DIR_IN, 0);
    IntReqHasEnable = true;
    gpio_cfg.irq_enable = true;
    gpio_cfg.irq_debounce = true;
    gpio_cfg.irq_polarity = HAL_GPIO_IRQ_POLARITY_LOW_FALLING;
    gpio_cfg.irq_handler = ICP1205_GPIO_INT_IrqHandler;
    gpio_cfg.irq_type = HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE;//HAL_GPIO_IRQ_TYPE_LEVEL_SENSITIVE;//HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE;
    hal_gpio_setup_irq(pin, &gpio_cfg);
}

static void ICP1205_GPIO_INT_IrqHandler(enum HAL_GPIO_PIN_T pin)
{
	ICP1205_GPIO_INT_IRQ_Disable(pin);
	//DBGPRINT("hal_gpio_pin_get_val(pin) %d",hal_gpio_pin_get_val(pin));
	if(hal_gpio_pin_get_val(pin) == 0) {
		osSignalSet(charger_manager_thread_id, 0x02);
	}
	//ICP1205_GPIO_INT_IRQ_Enable(pin);
}

/**
 *******************************************************************************
 ** \brief 关闭CHRG功能
 **
 ** \param [in]  无
 **
 ** \retval void            返回无
 ******************************************************************************/
void Icp1205SetChrgFunDisable(void)
{
	  uint8_t dat;
	  readDataFrom_ICP1205(ICP1205_CHRG_CON1, &dat, 1);
	  dat &=0XFC;//(~CHRGCON1_CHRG_EN);
	  writeDataTo_ICP1205(ICP1205_CHRG_CON1, &dat, 1);
}

/**
 *******************************************************************************
 ** \brief 打开CHRG功能
 **
 ** \param [in]  无
 **
 ** \retval void            返回无
 ******************************************************************************/
void Icp1205SetChrgFunEnable(void)
{
   uint8_t dat;
   readDataFrom_ICP1205(ICP1205_CHRG_CON1,&dat,1);
   dat |=0x23;
   writeDataTo_ICP1205(ICP1205_CHRG_CON1,&dat,1);
}

/**
 *******************************************************************************
 ** \brief CMP0.15中断使能
 **
 ** \param [in]  无
 **
 ** \retval void            返回无
 ******************************************************************************/
void Icp1205Cmp0P15IntEnable(void)
{
   uint8_t dat;
   readDataFrom_ICP1205(ICP1205_INT_MASK2,&dat,1);
   dat &=(~INT2_0P15CMP_MSK);
   writeDataTo_ICP1205(ICP1205_INT_MASK2,&dat,1);
}
/**
 *******************************************************************************
 ** \brief CMP0.15中断关闭
 **
 ** \param [in]  无
 **
 ** \retval void            返回无
 ******************************************************************************/
void Icp1205Cmp0P15IntDisable(void)
{
   uint8_t dat;
   readDataFrom_ICP1205(ICP1205_INT_MASK2,&dat,1);
   dat |=INT2_0P15CMP_MSK;
   writeDataTo_ICP1205(ICP1205_INT_MASK2,&dat,1);
}

/**
 *******************************************************************************
 ** \brief ICP1205清除所有中断状态
 **
 ** \param [in]  功能参数
 **
 ** \retval void            返回无
 ** ICP1205_INT_STAT1 reg 0x21,0x22,0x23
 ******************************************************************************/
void Icp1205ClearIntFlag(void)
{
	uint8_t u8dat[3]={0x00,0x00,0x00};
	writeDataTo_ICP1205(ICP1205_INT_STAT1,u8dat,sizeof(u8dat));
}

/**
 *******************************************************************************
 ** \brief ICP1205使能中断
 **
 ** \param [in]  功能参数
 **
 ** \retval void            返回无
 ******************************************************************************/
void Icp1205IntEnable(void)
{
  uint8_t u8dat;
  //readDataFrom_ICP1205(ICP1205_INT_EN,&u8dat,1);
  u8dat =0X01;
  writeDataTo_ICP1205(ICP1205_INT_EN,&u8dat,1);

}

/**
 *******************************************************************************
 ** \brief ICP1205关闭中断
 **
 ** \param [in]  功能参数
 **
 ** \retval void            返回无
 ******************************************************************************/
void Icp1205IntDisable(void)
{
	uint8_t u8dat;
    u8dat =0;
    writeDataTo_ICP1205(ICP1205_INT_EN,&u8dat,1);
}


/**
 *******************************************************************************
 ** \brief 进入Ship模式,控制位需要先清零，再写1才能进入
 **
 ** \param [in]  无
 **
 ** \retval void            返回无
 **
 ** 方法一是 BT主控通过 I2C设置 1205的 Reg09寄存器进入。
 ** 方法二是 ICP1106/1108通过载波指令控制 1205进入
 **
 ******************************************************************************/
void Icp1205ShipReset(void)
{
	uint8_t dat;
	//enable low 4Bit writeable
	DBGPRINT("%s" ,__func__);
	dat=0xF0;
	writeDataTo_ICP1205(ICP1205_SHIP_CON,&dat,sizeof(dat));
	//reset
	dat=0x10;
	writeDataTo_ICP1205(ICP1205_SHIP_CON,&dat,sizeof(dat));

}

/**
 *******************************************************************************
 ** \brief 进入Ship模式,控制位需要先复位，再写1才能进入
 **
 ** \param [in]  无
 **
 ** \retval void            返回无
 ******************************************************************************/
void Icp1205ShipEnable(void)
{
	 uint8_t dat;
     dat=	0x11;
     DBGPRINT("%s" ,__func__);
	 writeDataTo_ICP1205(ICP1205_SHIP_CON,&dat,1);
}

/**
 *******************************************************************************
 ** \brief Reg 0x0A载波使能
 **
 ** \param [in]  无
 **
 ** \retval void            返回无
 ******************************************************************************/
void Icp1205LoadComEnable(bool tx_enable, bool rx_enable)
{
	 uint8_t data = 0;
     if(tx_enable)  data |=(1<<1);
     if(rx_enable)  data |=(1<<0);
     DBGPRINT("%s data=0x%02x" , __func__,  data);
	 writeDataTo_ICP1205(ICP1205_CRCOM_CON1, &data, 1);
}

/**
 *******************************************************************************
 ** \brief ICP1205默认初始化输出配置,注意充电的寄存器没有初始化
 **
 ** \param [in]  功能参数
 **
 ** \retval void            返回无
 ******************************************************************************/
void ICP1205_Init(void)
{
	uint8_t buffer[64]   = {0};
	uint32_t ret = 0;
	int32_t syncRetryCount = 0;

	DBGPRINT("%s",__func__);
	ICP1205_BES_I2c_Init();
//	return;

    while (syncRetryCount < 100) {
	   ret = readDataFrom_ICP1205(ICP1205_CHRG_CON1,&buffer[0],1);
	   if (ret != 0){
		  osDelay(100);
	   } else {
		   DBGPRINT("%s buffer[0]=0x%02x", __func__, buffer[0]);
           if (0x00 == buffer[0] || 0xFF == buffer[0]){
        	  osDelay(100);
           } else {
        	   printf("ICP1205 communicate succeed!!");
        	   break;
           }
	   }
	   syncRetryCount++;
    }

    buffer[0] = 0X03;//保留透传、载波错误与成功中断
	DBGPRINT("Write Icp1205_Reg10 0x%02x",buffer[0]);
	writeDataTo_ICP1205(ICP1205_COMM_CON,&buffer[0],1);
	readDataFrom_ICP1205(ICP1205_COMM_CON,&buffer[0],1);
	DBGPRINT("Read Icp1205_Reg10 0x%02x",buffer[0]);

#if 1
	//deafult_value reg
	uint8_t initSettingsRegsValue[16] = {0X23,0x99,0x00,0x99,0x00,0x00,0x00,0x00,0xf0,0xf0,0x03,0x07,0x00,0x00,0x00,0x00};
	//ICP1205_CHRG_CON1  0x00  4.2v re/charge_en 0X23
	//ICP1205_CHRG_CON2  0x01  90ma fast; slow 90ma
	//ICP1205_CHRG_CON3  0x02  120min
	//ICP1205_CHRG_STS1  0x04
	//ICP1205_CHRG_STS2  0x05
	//ICP1205_CHRG_STS3  0x06
	//ICP1205_WDT_CON    0x08  disable watchdog enable
	//ICP1205_SHIP_CON   0x09
	//ICP1205_CRCOM_CON1 0x0a  0x03 tx_en ,rx_en
	//ICP1205_CRCOM_CON2 0x0b  0x07 new_data
	//ICP1205_CRCOM_CHR_DAT 0x0c~0x0e data_in
	//ICP1205_CRCOM_TDAT    0x0f      tx_Data

	writeDataTo_ICP1205(0x00,initSettingsRegsValue,16);	//charge enable

	//check I2C write
	initSettingsRegsValue[0] = 0x11;
	readDataFrom_ICP1205(0X00,initSettingsRegsValue,16);
	REL_TRACE_NOCRLF(0, "initSettingsRegsValue: ");
	DUMP8("%02x ",&initSettingsRegsValue[0], 16);


	//read charge status
	DBGPRINT("charge status: %02X", initSettingsRegsValue[ICP1205_CHRG_STS2]);
#endif

#if 1
	//Reg 1D
	//buffer[0] = 0x11;
	//buffer[1] = 0x10;
	//buffer[2] = 0x00;
	//writeDataTo_ICP1205( ICP1205_REG_1D, buffer, 3);

	//Reg 14 supervise ICP1205_ADS abnormal reset
	buffer[0] = {0xA5};
	writeDataTo_ICP1205( ICP1205_REV_DAT1, buffer, 1);
	//Reg 15 ICP1205_ADS abnormal reset
	buffer[0] = {0xA5};
	writeDataTo_ICP1205( ICP1205_REV_DAT2, buffer, 1);

	//reset SHIP_MODE
	Icp1205ShipReset();
	//Enable SHIP_MODE
	//Icp1205ShipEnable();

	//Enable ChargeEnable
	Icp1205SetChrgFunEnable();
	Icp1205ClearIntFlag();
	Icp1205IntDisable();

//	u8tmp=INT1_GP0CH_MSK | INT1_ANARDY_MSK |INT1_WDTOVTIME_MSK |INT1_BATLOW_MSK;
//	writeDataTo_ICP1205(ICP1205_INT_MASK1,&u8tmp,1);
//	u8tmp=INT2_0P35CMP_MSK |INT2_CHRGSTS_CH_MSK | INT2_ITERM_MSK |INT2_CVCHRG_MSK |INT2_TRCKL_TIMEOUT_MSK;
//	writeDataTo_ICP1205(ICP1205_INT_MASK2,&u8tmp,1);
//	u8tmp=INT1_GP0CH_MSK | INT1_ANARDY_MSK |INT1_WDTOVTIME_MSK |INT1_BATLOW_MSK;
//	writeDataTo_ICP1205(ICP1205_INT_MASK1,&u8tmp,1);
//	u8tmp=INT2_0P35CMP_MSK |INT2_CHRGSTS_CH_MSK | INT2_ITERM_MSK |INT2_CVCHRG_MSK |INT2_TRCKL_TIMEOUT_MSK;
//	writeDataTo_ICP1205(ICP1205_INT_MASK2,&u8tmp,1);
	//测试载波程序

	//u8tmp=0Xfc;//保留插入拔出中断
	buffer[0] = 0X7C;
	writeDataTo_ICP1205(ICP1205_INT_MASK1, &buffer[0], 1);

	buffer[0] = 0XDF;//保留中断完成、0.15中断
	writeDataTo_ICP1205(ICP1205_INT_MASK2, &buffer[0], 1);

	buffer[0] = 0X03;//保留透传、载波错误与成功中断
	writeDataTo_ICP1205(ICP1205_INT_MASK3, &buffer[0], 1);
#endif
	//Reg10 ICP1205_ADS EnterTransparentMode
	//buffer[0] = 0x03;
	//writeDataTo_ICP1205( ICP1205_COMM_CON, &buffer[0], 1);

}

//1， Reg0Ch/Reg0Dh/Reg0Eh 寄存器存放 1106 通过载波发送过来的数据， 主控可以通过
//I2C 读取寄存器里的值， 但是不能通过 I2C 写这 3 个寄存器的值。
//2， Reg0Ch/Reg0Dh/Reg0Eh 寄存器的数据需要 VIN 端维持大于 3.8V 的电压来保存， 一旦
//VIN 电压降低到 3.6V 以下， Reg0Ch/Reg0Dh/Reg0Eh 的值会复位为 0X00。
void hds_check_cmd_at_boot(void)
{
	uint8_t buffer[3];
	readDataFrom_ICP1205(ICP1205_CRCOM_CHR_DAT,  buffer,     1);
	readDataFrom_ICP1205(ICP1205_CRCOM_INT_DATH, &buffer[1], 1);
	readDataFrom_ICP1205(ICP1205_CRCOM_INT_DATL, &buffer[2], 1);
	DBGPRINT("cmd at boot: %x %x %x",buffer[0], buffer[1], buffer[2]);

	if ((buffer[1]&0x0f) == 0x6)
	{
		//hds_status_set_cang_onoff(true);
		writeDataTo_ICP1205(ICP1205_CRCOM_TDAT,&buffer[1],1);
	}else if ((buffer[1]&0x0f) == 0x5)
	{
		//hds_status_set_cang_onoff(false);
		writeDataTo_ICP1205(ICP1205_CRCOM_TDAT,&buffer[1],1);
	}
}


static void Icp1205UpdataIntSts(void)
{
	uint8_t u8RegTable[40],u8dat1,u8tmp;
	uint32_t ret ;
	//Read REG0x14 Reg0x15
	ret = readDataFrom_ICP1205(ICP1205_REV_DAT1,&u8RegTable[0],2);
	DBGPRINT("Icp1205UpdataIntSts REG0x14=0x%02x Reg0x15=0x%02x ret=%d",u8RegTable[0],u8RegTable[1], ret);
	if ( 0xA5 != u8RegTable[0] || 0xA5 != u8RegTable[1]) {
		ICP1205_Init();
		ICP1205_GPIO_INT_IRQ_Enable(ICP1205_INT_GPIO);
		Icp1205IntEnable();
		return;
	}

	//Reg10 ICP1205_ADS EnterTransparentMode
	readDataFrom_ICP1205( ICP1205_COMM_CON, &u8tmp, 1);
	DBGPRINT("Read Icp1205_Reg10 %02x",u8tmp);
	if(u8tmp & C0MMON_BUSY_FLAG)
	{
		u8tmp = 0X03;//保留透传、载波错误与成功中断
		DBGPRINT("Write Icp1205_Reg10 %02x",u8tmp);
		writeDataTo_ICP1205(ICP1205_COMM_CON,&u8tmp,1);
	}

	do {
		//ICP1205_INT_STAT1 0x21 ICP1205_INT_STAT2 0x22 ICP1205_INT_STAT3 0x23
	   if(readDataFrom_ICP1205(ICP1205_INT_STAT1,u8RegTable,6))
	   {
	        //renable interrupt
			u8tmp=0XFF;//保留插入拔出中断
			writeDataTo_ICP1205(ICP1205_INT_STAT1,&u8tmp,1);
			u8tmp=0XFF;//保留?.15中断
			writeDataTo_ICP1205(ICP1205_INT_STAT2,&u8tmp,1);
			u8tmp=0XFF;//保留透传、载波错误与成功中断
			writeDataTo_ICP1205(ICP1205_INT_STAT3,&u8tmp,1);
		    break;
	   }

	   DBGPRINT("Icp1205UpdataIntSts %d %d %d",u8RegTable[0],u8RegTable[1],u8RegTable[2]);
	   //关闭CHARGE
	   if ((u8RegTable[1] & INT2_0P15CMP_FLG) && (u8RegTable[4] & INT2_0P15CMP_MSK) == 0x00){
			u8dat1 = INT2_0P15CMP_FLG;
			writeDataTo_ICP1205(ICP1205_INT_STAT2, &u8dat1, 1);
			readDataFrom_ICP1205(ICP1205_CHRG_STS3, &u8tmp, 1);
			if ((u8tmp & CHRGSTS3_VIN2BAT_0P15CMP) == 0) {
				printf("Charger error.Icp1205SetChrgFunDisable");
				Icp1205SetChrgFunDisable();
			}
		}

	    //Reg 0x21 ICP1205_INT_STAT1
        //PlugIn
		if (u8RegTable[0] & INT1_PLGIN_FLG) {
			Icp1205SetChrgFunEnable();		//test
			u8dat1 = INT1_PLGIN_FLG;
			printf("Charger In Icp1205SetChrgFunEnable");
			writeDataTo_ICP1205(ICP1205_INT_STAT1, &u8dat1, 1);
			//hds_status_set_chargemode(true);
			Icp1205SetChrgFunEnable();
			//hds_thread_msg_send(THREAD_MSG_INPUT_EVENT_INBOX);
		}
		//PlugOut
		if (u8RegTable[0] & INT1_PLGOUT_FLG) {
			Icp1205SetChrgFunEnable();
			u8dat1 = INT1_PLGOUT_FLG;
			printf("Charger In INT1_PLGOUT_FLG");
			writeDataTo_ICP1205(ICP1205_INT_STAT1, &u8dat1, 1);
			//hds_status_set_chargemode(false);
			Icp1205SetChrgFunEnable();
			//hds_thread_msg_send(THREAD_MSG_INPUT_EVENT_OUTBOX);
		}
		//chrg_err
		if (u8RegTable[0] & INT1_CHRGERR_FLG) {
			DBGPRINT("Charger error.");
			u8dat1 = INT1_CHRGERR_FLG;
			writeDataTo_ICP1205(ICP1205_INT_STAT1, &u8dat1, 1);
			//return;//DEBUG
		}
        //wdt_timeout
		if (u8RegTable[0] & INT1_WDTOVTIME_FLG) {
			DBGPRINT("WDT over time.");
			osDelay(100);
			u8dat1 = INT1_WDTOVTIME_FLG;
			writeDataTo_ICP1205(ICP1205_INT_STAT1, &u8dat1, 1);
		}
        //battery low
		if (u8RegTable[0] & INT1_BATLOW_FLG) {
			DBGPRINT("Batter low");
			//osDelay(100);
			u8dat1 = INT1_BATLOW_FLG;
			writeDataTo_ICP1205(ICP1205_INT_STAT1, &u8dat1, 1);
		}
        //GPO change
		if (u8RegTable[ICP1205_INT_STAT1] & INT1_GP0CH_FLG) {
			if (u8RegTable[ICP1205_GPIO_CON] & GPIOCON_GP0_STS)
				DBGPRINT("GPIO0 set 'H'");
			else
				DBGPRINT("GPIO0 Clr 'L'");
			//osDelay(100);
			u8dat1 = INT1_GP0CH_FLG;
			writeDataTo_ICP1205(ICP1205_INT_STAT1, &u8dat1, 1);
		}
        //Charger Finished
		if (u8RegTable[0] & INT1_CHRGFIN_FLG) {
			DBGPRINT("Charger Finished");
			u8dat1 = INT1_CHRGFIN_FLG;
			writeDataTo_ICP1205(ICP1205_INT_STAT1, &u8dat1, 1);
		}
        //End Reg 0x21 ICP1205_INT_STAT1

	    //Reg 0x22 ICP1205_INT_STAT2
		//Bit0 :charge_state_change
		if ((u8RegTable[1] & INT2_CHRGSTS_CH_FLG) && 0x00 ==(u8RegTable[4] & INT2_CHRGSTS_CH_MSK))
		{
			readDataFrom_ICP1205(ICP1205_CHRG_STS1, &u8tmp, 1);
			u8tmp = u8tmp & 0x07;
			switch (u8tmp) {
			case CHRGSTS1_CHRG_IDLE_STS:
				DBGPRINT("Charge Idle.");
				break;
			case CHRGSTS1_CHRG_TRCKL_STS:
				DBGPRINT("Charge Trickle.");
				break;
			case CHRGSTS1_CHRG_FAST_STS:
				DBGPRINT("Charge Fast STS.");
				break;
			case CHRGSTS1_CHRG_NOR_STS:
				DBGPRINT("Charge normal STS.");
				break;
			case CHRGSTS1_CHRG_DLYTIME_STS:
				DBGPRINT("Charge extend.");
				break;
			case CHRGSTS1_CHRG_FINSH_STS:
				DBGPRINT("Charge finish.");
				break;
			case CHRGSTS1_CHRG_ERR_STS:
				DBGPRINT("Charge error.");
				if (u8RegTable[ICP1205_CHRG_STS2] & CHRGSTS2_VIN_OVP_STS) {
					DBGPRINT("Charge error OVP!");
				}
				if (u8RegTable[ICP1205_CHRG_STS2] & CHRGSTS2_VIN_UVLO_STS) {
					DBGPRINT("Charge error UVLO!");
				}
				break;
			default:
				break;

			}
			//osDelay(100);
			u8dat1 = INT2_CHRGSTS_CH_FLG;
			writeDataTo_ICP1205(ICP1205_INT_STAT2, &u8dat1, 1);
		}
		//Bit1 :chrg_timeout
		if(u8RegTable[1]&INT2_CCCV_TIMEOUT_FLG)
		{
			DBGPRINT("Charge over time!");
			//osDelay(100);
			u8dat1 = INT2_CCCV_TIMEOUT_FLG;
			writeDataTo_ICP1205(ICP1205_INT_STAT2,&u8dat1,1);
		}
		//Bit2 :chrg_trickle_timeout
		if(u8RegTable[1]&INT2_TRCKL_TIMEOUT_FLG)
		{
			DBGPRINT("chrg_trickle_timeout!");
			//osDelay(100);
			u8dat1 = INT2_TRCKL_TIMEOUT_FLG;
			writeDataTo_ICP1205(ICP1205_INT_STAT2,&u8dat1,1);
			//return;//DEBUG
		}
		//Bit3 :chrg_ntc
		readDataFrom_ICP1205(ICP1205_CHRG_STS1, &u8RegTable[ICP1205_CHRG_STS3], 1);
		if(u8RegTable[1]&INT2_NTC_FLG)
		{
			osDelay(100);
			DBGPRINT("always INT2_NTC_FLG!");
		    u8dat1 = INT2_NTC_FLG;
		    writeDataTo_ICP1205(ICP1205_INT_STAT2,&u8dat1,1);
		}
		//Bit4 :cv_stat
		if(u8RegTable[1]&INT2_CVCHRG_FLG)
		{
		    if(u8RegTable[ICP1205_CHRG_STS3]&CHRGSTS3_CV_STS) {
			  DBGPRINT("always voltage!");
		    } else {
			  DBGPRINT("other voltages!");
		    }
		    //osDelay(100);
		    u8dat1 = INT2_CVCHRG_FLG;
		    writeDataTo_ICP1205(ICP1205_INT_STAT2,&u8dat1,1);
		}
		//Bit5 :chrg_ntc
		if(u8RegTable[1]&INT2_0P15CMP_FLG)
		{
		    if(u8RegTable[ICP1205_CHRG_STS3]&CHRGSTS3_VIN2BAT_0P15CMP) {
			    DBGPRINT(" gap '0.15v'!");
		    } else {
		    	DBGPRINT(" other gap '0.15v'!");
		    }
		    //osDelay(100);
		    u8dat1 = INT2_0P15CMP_FLG;
		    writeDataTo_ICP1205(ICP1205_INT_STAT2,&u8dat1,1);
		}
		//Bit6 :chrg_ntc
		if(u8RegTable[1]&INT2_0P35CMP_FLG)
		{
		    if(u8RegTable[ICP1205_CHRG_STS3]&CHRGSTS3_VIN2BAT_0P35CMP) {
		    	 DBGPRINT(" gap '0.35v'!");
		    } else {
		    	DBGPRINT(" other gap '0.35v'!");
		    }
		    //osDelay(100);
		    u8dat1 = INT2_0P35CMP_FLG;
		    writeDataTo_ICP1205(ICP1205_INT_STAT2,&u8dat1,1);
		}
		//Bit7 :chrg_ntc
		if(u8RegTable[1]&INT2_ITERM_FLG)
		{
		    if(u8RegTable[ICP1205_CHRG_STS3]&CHRGSTS3_ITERM_STS) {
			  DBGPRINT("iterm_cmp '1'!");
		    } else {
			  DBGPRINT("iterm_cmp '0'!");
		    }
		    //osDelay(100);
		    u8dat1 = INT2_ITERM_FLG;
		    writeDataTo_ICP1205(ICP1205_INT_STAT2,&u8dat1,1);
		}
		//End Reg 0x22 ICP1205_INT_STAT2


	    //Reg 0x23 ICP1205_INT_STAT3
		//Bit2 :rx_cmd  载波
		if(u8RegTable[2]&INT3_CRRXCMD_FLG)
		{
			u8dat1=INT3_CRRXCMD_FLG;
			writeDataTo_ICP1205(ICP1205_INT_STAT3,&u8dat1,1);

			readDataFrom_ICP1205(ICP1205_CRCOM_CON1,&u8tmp,1);

			Icp1205SetChrgFunEnable();

      		switch(u8tmp&0xf0)
			{
				case CR_REC_CMD_QUE_CHRG_STS:
					break;
				case CR_REC_CMD_READ_DATA:
					//OLED_ShowString(40,2,(uint8_t*)":Read 0x",16);
				  	readDataFrom_ICP1205(ICP1205_CRCOM_TDAT,&u8dat1,1);	//设置载波回传数据
					break;
				case CR_REC_CMD_FRC_CHRG_FNSH:
					//OLED_ShowString(40,2,(uint8_t*)":Chrg Fnsh",16);
				  	Icp1205SetChrgFunDisable();//test
					break;
				case CR_REC_CMD_SHIP_MOD:
					//OLED_ShowString(40,2,(uint8_t*)":Ship mode",16);
					break;
				case CR_REC_CMD_CHAR_DATA:
					break;
				case CR_REC_CMD_WORD_DATA:
					uint8_t data_tmp[2];
					readDataFrom_ICP1205(ICP1205_CRCOM_INT_DATH/*ICP1205_CRCOM_WORD_DAT*/,data_tmp,2);
					DBGPRINT("data_tmp %x %x",data_tmp[0],data_tmp[1]);
					//OLED_ShowString(40,2,(uint8_t*)":Rec 0x",16);

					if((data_tmp[1]&0x0f)==0x1)//升级模式
					{
						DBGPRINT("upgrade cmd: battery %x", data_tmp[0]);
				    	writeDataTo_ICP1205(ICP1205_CRCOM_TDAT,&data_tmp[1],1);
						//hds_thread_msg_send(THREAD_MSG_UART_CMD_UPGRADE);
					}

					if((data_tmp[1]&0x0f)==0x2)//配对模式
					{
						DBGPRINT("bt pair cmd: battery %x", data_tmp[0]);
				    	writeDataTo_ICP1205(ICP1205_CRCOM_TDAT,&data_tmp[1],1);
						Icp1205Cmp0P15IntEnable();//test 0713
						//hds_thread_msg_send(THREAD_MSG_UART_CMD_BTPAIR);
					}
					/*
					if((data_tmp[1]&0x0f)==0x4)
					{
						DBGPRINT("reset cmd: battery %x", data_tmp[0]);
				    	writeDataTo_ICP1205(ICP1205_CRCOM_TDAT,&data_tmp[1],1);
						hds_thread_msg_send(THREAD_MSG_UART_CMD_RESET);
					}
					*/
					//if(u8tmp==0x5a)
					if((data_tmp[1]&0x0f)==0x5)//关盖
					{
						DBGPRINT("close cang cmd: battery %x", data_tmp[0]);
				    	writeDataTo_ICP1205(ICP1205_CRCOM_TDAT,&data_tmp[1],1);
						//hds_status_set_chargemode(true);
						//hds_thread_msg_send(THREAD_MSG_UART_CMD_CLOSECANG);
						//Icp1205Cmp0P15DisEnable();//test 0713
					}

				  	//if(u8tmp==0x69)
				  	if((data_tmp[1]&0x0f)==0x6)//开盖
					{
				  		DBGPRINT("open cang cmd: battery %x", data_tmp[0]);
				    	writeDataTo_ICP1205(ICP1205_CRCOM_TDAT,&data_tmp[1],1);
						Icp1205Cmp0P15IntEnable();//test 0713
						//hds_thread_msg_send(THREAD_MSG_UART_CMD_OPENCANG);
					}

					if((data_tmp[1]&0x0f)==0x7)//入仓
					{
						DBGPRINT("into cang cmd: battery %x", data_tmp[0]);
				    	writeDataTo_ICP1205(ICP1205_CRCOM_TDAT,&data_tmp[1],1);
						Icp1205Cmp0P15IntEnable();//test 0713
						//hds_thread_msg_send(THREAD_MSG_INPUT_EVENT_INBOX);
					}

					if((data_tmp[1]&0x0f)==0x8)//双耳配对
					{
						DBGPRINT("recovery cmd: battery %x", data_tmp[0]);
				    	writeDataTo_ICP1205(ICP1205_CRCOM_TDAT,&data_tmp[1],1);
						//hds_thread_msg_send(THREAD_MSG_UART_CMD_RESET);
						Icp1205SetChrgFunDisable();
						hal_gpio_pin_clr(HAL_GPIO_PIN_P0_4);
					}

					if((data_tmp[1]&0x0f)==0x9)//充电完成
					{
						DBGPRINT("charge full: battery %x", data_tmp[0]);
				    	writeDataTo_ICP1205(ICP1205_CRCOM_TDAT,&data_tmp[1],1);
						//Icp1205Cmp0P15IntEnable();//test 0713
						//hds_status_set_chargemode(false);
						//hds_thread_msg_send(THREAD_MSG_BATTRY_EVENT_FULLCHARGE);
					}

					if((data_tmp[1]&0x0f)==0xA)//单耳测试模式
					{
						DBGPRINT("test cmd: battery %x", data_tmp[0]);
				    	writeDataTo_ICP1205(ICP1205_CRCOM_TDAT,&data_tmp[1],1);
						Icp1205Cmp0P15IntEnable();//test 0713
						//hds_thread_msg_send(THREAD_MSG_UART_CMD_TEST);
					}
					break;
				case CR_REC_CMD_CLR_GP0:
					DBGPRINT("GP0='0'");
					break;
				case CR_REC_CMD_SET_GP0:
					DBGPRINT("GP0='10'");
					break;
				default:
					break;
			}
		}
        //Bit3 :rx_failed
		if(u8RegTable[2]&INT3_CRRXERR_FLG)
		{
			u8dat1   =  INT3_CRRXERR_FLG;
			writeDataTo_ICP1205(ICP1205_INT_STAT3,&u8dat1,1);
			printf("Crcom Err!");
		}
		//Bit4 :comm_mode 透传模式
		if(u8RegTable[2]&INT3_TRCOM_FLG)
		{
			u8dat1=INT3_TRCOM_FLG;
			writeDataTo_ICP1205(ICP1205_INT_STAT3,&u8dat1,1);
			//Icp1205SetChrgFunEnable();
			//Icp1205TrcomTest();
		}


        //renable interrupt
		u8tmp=0XFF;//保留插入拔出中断
		writeDataTo_ICP1205(ICP1205_INT_STAT1,&u8tmp,1);
		u8tmp=0XFF;//保留?.15中断
		writeDataTo_ICP1205(ICP1205_INT_STAT2,&u8tmp,1);
		u8tmp=0XFF;//保留透传、载波错误与成功中断
		writeDataTo_ICP1205(ICP1205_INT_STAT3,&u8tmp,1);

		readDataFrom_ICP1205(ICP1205_INT_STAT1,&u8tmp,1);
		//DBGPRINT("ICP1205_INT_STAT1= %x",u8tmp);
		readDataFrom_ICP1205(ICP1205_INT_STAT2,&u8tmp,1);
		//DBGPRINT("ICP1205_INT_STAT2= %x",u8tmp);
		readDataFrom_ICP1205(ICP1205_INT_STAT3,&u8tmp,1);
		//DBGPRINT("ICP1205_INT_STAT3= %x",u8tmp);

		} while (0);
 }

void ICP1205_Init_again(void){
	ICP1205_GPIO_INT_IRQ_Enable(ICP1205_INT_GPIO);
	//feedup in or out cang state for fastest outbox
	uint8_t u8tmp;
	readDataFrom_ICP1205(ICP1205_CHRG_STS2,&u8tmp,1);
	DBGPRINT("charge status: %02x",u8tmp);
	if ((u8tmp&CHRGSTS2_VIN_STS) == 0)
	{
		//hds_status_set_cang_onoff(true);C
		//hds_status_set_cang_inout(true);
	}
	readDataFrom_ICP1205(ICP1205_CHRG_STS3,&u8tmp,1);
	DBGPRINT("charge status: %02x",u8tmp);
	if ((u8tmp&CHRGSTS3_VIN2BAT_0P15CMP) != 0)
	{
		u8tmp=INT2_0P15CMP_FLG;
		writeDataTo_ICP1205(ICP1205_INT_STAT2,&u8tmp,1);
	}
}

static void charger_manager_handler_thread(const void *arg)
{
	ICP1205_Init();
	ICP1205_GPIO_INT_IRQ_Enable(ICP1205_INT_GPIO);
	Icp1205IntEnable();
	while(true)
	{
		//osSignalWait(0x02,1600);
	    osSignalWait(0x02,osWaitForever);
		Icp1205UpdataIntSts();
		ICP1205_GPIO_INT_IRQ_Enable(ICP1205_INT_GPIO);
	}
}

void charger_manager_start(void)
{
	DBGPRINT("%s", __func__);
	aw_ntc_detect_init();
	charger_manager_thread_id = osThreadCreate(osThread(charger_manager_handler_thread), NULL);
}
