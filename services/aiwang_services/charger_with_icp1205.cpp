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
#include "communication_svr.h"


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

#define GOC_1205_DEBUG_ENABLE 1

#undef printf
#undef DBGPRINT

#if GOC_1205_DEBUG_ENABLE

#define printf(fmt, ...) \
    hal_trace_printf(0, "[goc-1205] " fmt, ##__VA_ARGS__)

#define DBGPRINT(fmt,...) \
    hal_trace_printf(2, "[goc-1205] " fmt, ##__VA_ARGS__)

#else

#define printf(...)
#define DBGPRINT(...)

#endif


static osThreadId charger_manager_thread_id = NULL;
static void charger_manager_handler_thread(const void *arg);
osThreadExDef(charger_manager_handler_thread, osPriorityNormal, 1, 1024*3, "charger_manager_thread", 1U);

extern "C" bool ntt_1wire_is_active(void);

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
        {HAL_IOMUX_PIN_P0_6, HAL_IOMUX_FUNC_MCU_I2C_M3_SCL,HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE},
        {HAL_IOMUX_PIN_P0_7, HAL_IOMUX_FUNC_MCU_I2C_M3_SDA,HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE},
        {ICP1205_GP0_IOMUX, HAL_IOMUX_FUNC_AS_GPIO,HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE},
        {ICP1205_INT_IOMUX, HAL_IOMUX_FUNC_AS_GPIO,HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE},
    };

    DBGPRINT("%s", __func__);

    /* Close I2C before reconfiguration */
    hal_i2c_close(HAL_I2C_ID_3);

    /* Configure IOMUX */
    hal_iomux_init(pinmux_iic, ARRAY_SIZE(pinmux_iic));

    /*
     * ICP1205 GPIO:
     * GP0 -> BES P0_4
     * INT -> BES P0_5
     *
     * Configure INT as input.
     */
    hal_gpio_pin_set_dir(
        ICP1205_INT_GPIO,
        HAL_GPIO_DIR_IN,
        0);

    /*
     * Debug GPIO levels before enabling IRQ.
     */
    DBGPRINT(
        "[NTT_1205_IOMUX] GP0_P04=%d INT_P05=%d",
        hal_gpio_pin_get_val(ICP1205_GP0_GPIO),
        hal_gpio_pin_get_val(ICP1205_INT_GPIO));

    /* Initialize I2C */
    struct HAL_I2C_CONFIG_T i2c_cfg = {};

    i2c_cfg.mode = HAL_I2C_API_MODE_TASK;
    i2c_cfg.use_dma = 0;
    i2c_cfg.use_sync = 1;
    i2c_cfg.speed = 100 * 1000;
    i2c_cfg.as_master = 1;
    i2c_cfg.addr_as_slave = 0;
    i2c_cfg.rising_time_ns = 0;

    uint32_t ret = hal_i2c_open(HAL_I2C_ID_3, &i2c_cfg);

    DBGPRINT(
        "[NTT_1205_I2C] OPEN RET=0x%x",
        (unsigned int)ret);
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
static volatile uint32_t ntt_1205_irq_count = 0;
static volatile uint32_t ntt_1205_irq_low_count = 0;
static volatile uint32_t ntt_1205_irq_high_count = 0;

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
    struct HAL_GPIO_IRQ_CFG_T gpio_cfg = {};

    if (IntReqHasEnable)
    {
        DBGPRINT("[NTT_1205_IRQ] Already enabled");
        return;
    }

    hal_gpio_pin_set_dir(pin, HAL_GPIO_DIR_IN, 0);

    DBGPRINT(
        "[NTT_1205_IRQ] BEFORE_SETUP GP0=%d INT=%d",
        hal_gpio_pin_get_val(ICP1205_GP0_GPIO),
        hal_gpio_pin_get_val(ICP1205_INT_GPIO));

    gpio_cfg.irq_enable = true;
    gpio_cfg.irq_debounce = false;
    gpio_cfg.irq_polarity = HAL_GPIO_IRQ_POLARITY_LOW_FALLING;
    gpio_cfg.irq_handler = ICP1205_GPIO_INT_IrqHandler;
    gpio_cfg.irq_type = HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE;

	uint32_t ret = hal_gpio_setup_irq(pin, &gpio_cfg);

	IntReqHasEnable = (ret == 0);

	DBGPRINT(
		"[NTT_1205_IRQ_SETUP] PIN=%d RET=%u INT_P05=%d ENABLE=%d",
		pin,
		(unsigned int)ret,
		hal_gpio_pin_get_val(ICP1205_INT_GPIO),
		IntReqHasEnable);

    DBGPRINT(
        "[NTT_1205_IRQ] AFTER_SETUP GP0=%d INT=%d ENABLE=%d",
        hal_gpio_pin_get_val(ICP1205_GP0_GPIO),
        hal_gpio_pin_get_val(ICP1205_INT_GPIO),
        IntReqHasEnable);
}

static void ICP1205_GPIO_INT_IrqHandler(enum HAL_GPIO_PIN_T pin)
{
    ntt_1205_irq_count++;

    uint8_t level = hal_gpio_pin_get_val(pin);

    ICP1205_GPIO_INT_IRQ_Disable(pin);

    if (level == 0)
    {
        ntt_1205_irq_low_count++;

        communication_batter1205();
    }
    else
    {
        ntt_1205_irq_high_count++;

        // IRQ 已關閉，但沒有送出處理訊息，
        // 必須重新 Enable，避免中斷永久停用。
        ICP1205_GPIO_INT_IRQ_Enable(pin);
    }
}

void ntt_1205_irq_debug_dump(void)
{
    DBGPRINT(
        "[NTT_1205_DEBUG] IRQ=%u LOW=%u HIGH=%u GPIO=%d ENABLE=%d",
        (unsigned int)ntt_1205_irq_count,
        (unsigned int)ntt_1205_irq_low_count,
        (unsigned int)ntt_1205_irq_high_count,
        hal_gpio_pin_get_val(ICP1205_INT_GPIO),
        IntReqHasEnable);
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
	  DBGPRINT("Icp1205SetChrgFunDisable");
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
   DBGPRINT("Icp1205SetChrgFunEnable");
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
   DBGPRINT("Icp1205Cmp0P15IntEnable");
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
   DBGPRINT("Icp1205Cmp0P15IntDisable");	
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
	DBGPRINT("Icp1205ClearIntFlag");	
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
    uint8_t u8dat = 0x01;
    uint8_t readback = 0xFF;

    uint32_t wr_ret;
    uint32_t rd_ret;

    wr_ret = writeDataTo_ICP1205(ICP1205_INT_EN,&u8dat,1);

    rd_ret = readDataFrom_ICP1205(ICP1205_INT_EN,&readback,1);

    DBGPRINT("[NTT_1205_INT_EN] WR=%u RD=%u VALUE=0x%02X",(unsigned int)wr_ret,(unsigned int)rd_ret,readback);
}

static void ntt_1205_dump_irq_registers(const char *stage)
{
    uint8_t int_stat[3] = {0};
    uint8_t int_mask[3] = {0};
    uint8_t int_en = 0;

    int ret_stat;
    int ret_mask;
    int ret_en;

    /*
     * ICP1205 Interrupt Status
     * 0x21 / 0x22 / 0x23
     */
    ret_stat = readDataFrom_ICP1205(
        ICP1205_INT_STAT1,
        int_stat,
        sizeof(int_stat));

    /*
     * ICP1205 Interrupt Mask
     * 0x24 / 0x25 / 0x26
     */
    ret_mask = readDataFrom_ICP1205(
        ICP1205_INT_MASK1,
        int_mask,
        sizeof(int_mask));

    /*
     * ICP1205 Interrupt Enable
     * 0x20
     */
    ret_en = readDataFrom_ICP1205(
        ICP1205_INT_EN,
        &int_en,
        1);

    DBGPRINT(
        "[NTT_1205_IRQ_REG] %s "
        "STAT=%02X %02X %02X "
        "MASK=%02X %02X %02X "
        "EN=%02X GPIO=%d IRQ_ENABLE=%d "
        "RET=%d/%d/%d",
        stage,
        int_stat[0],
        int_stat[1],
        int_stat[2],
        int_mask[0],
        int_mask[1],
        int_mask[2],
        int_en,
        hal_gpio_pin_get_val(ICP1205_INT_GPIO),
        IntReqHasEnable,
        ret_stat,
        ret_mask,
        ret_en);

    /*
     * Carrier RX Command interrupt flag
     * INT_STAT3 bit 2
     */
    DBGPRINT(
        "[NTT_1205_CARRIER_IRQ] %s "
        "RX_CMD=%d RX_ERR=%d TRANSPARENT=%d "
        "MASK3=0x%02X",
        stage,
        (int_stat[2] & 0x04) ? 1 : 0,
        (int_stat[2] & 0x08) ? 1 : 0,
        (int_stat[2] & 0x10) ? 1 : 0,
        int_mask[2]);
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
	DBGPRINT("Icp1205IntDisable");	
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
	Icp1205ShipReset();
	 uint8_t dat;
     dat=	0x33;
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
	 DBGPRINT("Icp1205LoadComEnable");	
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
uint8_t icp1205_get_chrg_sts2(void)
{
    uint8_t sts2 = 0xFF;
    if (readDataFrom_ICP1205(ICP1205_CHRG_STS2,&sts2,1) != 0)
    {
        DBGPRINT("[NTT_1205] read CHRG_STS2 failed");
        return 0xFF;
    }
    DBGPRINT("[NTT_1205] CHRG_STS2=0x%02X",sts2);

    return sts2;
}

static uint8_t g_icp1205_reg10 = 0xFF;

uint8_t Icp1205_Reg10_get(void)
{
    return g_icp1205_reg10;
}

void Icp1205_Reg10_set(uint8_t status)
{
    g_icp1205_reg10 = status;

    DBGPRINT("[ICP1205] Reg10 cache set = 0x%02X",g_icp1205_reg10);
}

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

    /*
     * NTT DEBUG:
     * Read ICP1205 carrier registers before initialization.
     *
     * 0x0B : CRCOM_CON2
     * 0x0C : CRCOM_CHR_DAT
     * 0x0D : CRCOM_INT_DATH
     * 0x0E : Carrier data
     *
     * These registers may be reset when VIN drops.
     */
    {
        uint8_t crcom_con2 = 0xFF;
        uint8_t carrier_data[3] = {0xFF, 0xFF, 0xFF};
        uint8_t int_status[3] = {0xFF, 0xFF, 0xFF};

        uint32_t ret1 = readDataFrom_ICP1205(ICP1205_CRCOM_CON2,&crcom_con2,1);

        uint32_t ret2 = readDataFrom_ICP1205(ICP1205_CRCOM_CHR_DAT,carrier_data,3);

        uint32_t ret3 = readDataFrom_ICP1205(ICP1205_INT_STAT1,int_status,3);

        DBGPRINT("[NTT_1205_BOOT_CARRIER] ret=%u/%u/%u CON2=0x%02X DATA=%02X %02X %02X INT=%02X %02X %02X",
            (unsigned int)ret1,
            (unsigned int)ret2,
            (unsigned int)ret3,
            crcom_con2,
            carrier_data[0],
            carrier_data[1],
            carrier_data[2],
            int_status[0],
            int_status[1],
            int_status[2]);

        if (ret1 == 0 && ret2 == 0)
        {
            uint8_t carrier_cmd = carrier_data[1] & 0x0F;

            DBGPRINT("[NTT_1205_BOOT_CMD] carrier_cmd=0x%02X",carrier_cmd);

            if (carrier_cmd == 0x06)
            {
                DBGPRINT("[NTT_1205_BOOT_CMD] OPEN_CASE candidate");
            }
            else if (carrier_cmd == 0x05)
            {
                DBGPRINT("[NTT_1205_BOOT_CMD] CLOSE_CASE candidate");
            }
        }
    }

    buffer[0] = 0X03;//保留透传、载波错误与成功中断
	DBGPRINT("Write Icp1205_Reg10 0x%02x",buffer[0]);
	writeDataTo_ICP1205(ICP1205_COMM_CON,&buffer[0],1);
	readDataFrom_ICP1205(ICP1205_COMM_CON,&buffer[0],1);
	DBGPRINT("Read Icp1205_Reg10 0x%02x",buffer[0]);
	Icp1205_Reg10_set(buffer[0]);

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

	// Read ICP1205 charge / communication status
	{
		uint8_t sts2  = initSettingsRegsValue[ICP1205_CHRG_STS2];
		uint8_t reg10 = 0xFF;

		uint32_t reg10_ret =
			readDataFrom_ICP1205(
				ICP1205_COMM_CON,
				&reg10,
				1);

		bool vin     = !!(sts2 & CHRGSTS2_VIN_STS);
		bool uvlo    = !!(sts2 & CHRGSTS2_VIN_UVLO_STS);
		bool ovp     = !!(sts2 & CHRGSTS2_VIN_OVP_STS);
		bool shorted = !!(sts2 & CHRGSTS2_SHORT_STS);
		bool bit1    = !!(sts2 & (1u << 1));

		bool transparent =
			(reg10_ret == 0) &&
			!!(reg10 & COMMCON_BUSY_STS);

		const char *vin_state;

		if (ovp)
		{
			vin_state = "VIN_OVP";
		}
		else if (uvlo && vin)
		{
			vin_state = "VIN_PRESENT_UVLO";
		}
		else if (uvlo && !vin)
		{
			vin_state = "VIN_LOW_OR_OFF";
		}
		else if (vin)
		{
			vin_state = "VIN_NORMAL";
		}
		else
		{
			vin_state = "VIN_NOT_PRESENT";
		}

		DBGPRINT(
			"[ICP1205_STATUS] "
			"STS2=0x%02X VIN=%d UVLO=%d OVP=%d SHORT=%d BIT1=%d state=%s "
			"REG10=0x%02X TRANSPARENT=%d 1WIRE=%d",
			sts2,
			vin,
			uvlo,
			ovp,
			shorted,
			bit1,
			vin_state,
			reg10,
			transparent,
			ntt_1wire_is_active());
	}
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
	DBGPRINT("[NTT_ICP1205_IRQ] ENTER");
    uint8_t u8RegTable[40] = {0};
    uint8_t u8dat1;
    uint8_t u8tmp;

    // NTT debug: always read latest charger status
    uint8_t chrg_sts1 = 0;
    uint8_t chrg_sts2 = 0;
    uint8_t chrg_sts3 = 0;

    uint32_t ret;
	DBGPRINT("[NTT_ICP1205_IRQ] BEFORE I2C");
    // Read REG0x14 Reg0x15
    ret = readDataFrom_ICP1205(ICP1205_REV_DAT1,&u8RegTable[0],2);

    DBGPRINT("Icp1205UpdataIntSts REG0x14=0x%02x Reg0x15=0x%02x ret=%d",u8RegTable[0],u8RegTable[1],ret);

    (void)ret;

    if (0xA5 != u8RegTable[0] || 0xA5 != u8RegTable[1])
    {
        ICP1205_Init();
        ICP1205_GPIO_INT_IRQ_Enable(ICP1205_INT_GPIO);
        Icp1205IntEnable();
        return;
    }

    // ---------------------------------------------------------
    // NTT: read real-time charger status
    // ---------------------------------------------------------
    readDataFrom_ICP1205(ICP1205_CHRG_STS1,&chrg_sts1,1);
    readDataFrom_ICP1205(ICP1205_CHRG_STS2,&chrg_sts2,1);
    readDataFrom_ICP1205(ICP1205_CHRG_STS3,&chrg_sts3,1);

    DBGPRINT("[NTT_1205_STS] STS1=%02X STS2=%02X STS3=%02X VIN=%d UVLO=%d OVP=%d SHORT=%d",
        chrg_sts1,
        chrg_sts2,
        chrg_sts3,
        !!(chrg_sts2 & CHRGSTS2_VIN_STS),
        !!(chrg_sts2 & CHRGSTS2_VIN_UVLO_STS),
        !!(chrg_sts2 & CHRGSTS2_VIN_OVP_STS),
        !!(chrg_sts2 & CHRGSTS2_SHORT_STS));

    // Reg10 ICP1205_ADS EnterTransparentMode
    readDataFrom_ICP1205(ICP1205_COMM_CON,&u8tmp,1);
    DBGPRINT("Read Icp1205_Reg10 %02x", u8tmp);

    if (u8tmp & C0MMON_BUSY_FLAG)
    {
        u8tmp = 0X03;
        DBGPRINT("Write Icp1205_Reg10 %02x", u8tmp);
        writeDataTo_ICP1205(ICP1205_COMM_CON,&u8tmp,1);
    }

    do
    	{
		//ICP1205_INT_STAT1 0x21 ICP1205_INT_STAT2 0x22 ICP1205_INT_STAT3 0x23
	   if(readDataFrom_ICP1205(ICP1205_INT_STAT1,u8RegTable,6))
	   {
			DBGPRINT("[NTT_1205_INT] ""INT1=0x%02X INT2=0x%02X INT3=0x%02X",u8RegTable[0],u8RegTable[1],u8RegTable[2]);
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
		// PlugIn
		if (u8RegTable[0] & INT1_PLGIN_FLG)
		{
			u8dat1 = INT1_PLGIN_FLG;

			DBGPRINT("[ICP1205_CHG] PlugIn -> Charge Enable");

			writeDataTo_ICP1205(ICP1205_INT_STAT1,&u8dat1,1);

			Icp1205SetChrgFunEnable();
		}

		// PlugOut
		if (u8RegTable[0] & INT1_PLGOUT_FLG)
		{
			u8dat1 = INT1_PLGOUT_FLG;

			DBGPRINT("[ICP1205_CHG] PlugOut -> Charge Disable");

			writeDataTo_ICP1205(ICP1205_INT_STAT1,&u8dat1,1);

			Icp1205SetChrgFunDisable();
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

				if (chrg_sts2 & CHRGSTS2_VIN_OVP_STS)
				{
					DBGPRINT("Charge error OVP!");
					DBGPRINT("[NTT_1205_CASE] VIN PRESENT, STS2=%02X",chrg_sts2);
				}

				if (chrg_sts2 & CHRGSTS2_VIN_UVLO_STS)
				{
					DBGPRINT("Charge error UVLO!");
					DBGPRINT("[NTT_1205_CASE] VIN ABSENT, STS2=%02X UVLO=%d",chrg_sts2,!!(chrg_sts2 & CHRGSTS2_VIN_UVLO_STS));
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
		readDataFrom_ICP1205(ICP1205_CHRG_STS3,&u8RegTable[ICP1205_CHRG_STS3],1);
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

void handle_batter_1205(void)
{
	DBGPRINT("[NTT_1205_HANDLE] entered tick=%u",(unsigned int)osKernelSysTick());
	Icp1205UpdataIntSts();
	ICP1205_GPIO_INT_IRQ_Enable(ICP1205_INT_GPIO);
}

/*
 * Diagnostic only.
 * Enable after confirming ICP1205 INT_STAT1/2/3 are W1C.
 */
#define NTT_1205_TEST_CLEAR_PENDING_IRQ  1

void charger_manager_start(void)
{
    DBGPRINT("%s", __func__);

    aw_ntc_detect_init();

#if 0
    charger_manager_thread_id =
        osThreadCreate(
            osThread(charger_manager_handler_thread),
            NULL);
#else

    if (charger_manager_thread_id)
        ;

    /*
     * Step 1: Initialize ICP1205.
     */
    ICP1205_Init();
	DBGPRINT("[NTT_1205_TEST] BEFORE_IRQ_ENABLE INT_P05=%d",hal_gpio_pin_get_val(ICP1205_INT_GPIO));

    /*
     * Step 2: Read interrupt registers before clearing.
     */
    ntt_1205_dump_irq_registers("BEFORE_CLEAR");

#if NTT_1205_TEST_CLEAR_PENDING_IRQ

    /*
     * Step 3: TEST ONLY
     *
     * Clear ICP1205 pending interrupt flags.
     * Registers:
     * 0x21 = INT_STAT1
     * 0x22 = INT_STAT2
     * 0x23 = INT_STAT3
     *
     * WARNING:
     * Only enable if these registers are confirmed W1C.
     */
    uint8_t clear_flags[3] = {0xFF, 0xFF, 0xFF};

    int clear_ret = writeDataTo_ICP1205(
        ICP1205_INT_STAT1,
        clear_flags,
        sizeof(clear_flags));

    DBGPRINT(
        "[NTT_1205_CLEAR] ret=%d GPIO=%d",
        clear_ret,
        hal_gpio_pin_get_val(ICP1205_INT_GPIO));

    /*
     * Step 4: Read interrupt registers after clearing.
     */
    ntt_1205_dump_irq_registers("AFTER_CLEAR");

#endif

    /*
     * Step 5: Enable BES1306P GPIO IRQ.
     */
    ICP1205_GPIO_INT_IRQ_Enable(ICP1205_INT_GPIO);
	DBGPRINT("[NTT_1205_TEST] AFTER_IRQ_ENABLE INT_P05=%d",hal_gpio_pin_get_val(ICP1205_INT_GPIO));

    /*
     * Step 6: Enable ICP1205 interrupt output.
     */
    Icp1205IntEnable();
	DBGPRINT("[NTT_1205_TEST] AFTER_CHIP_INT_ENABLE INT_P05=%d",hal_gpio_pin_get_val(ICP1205_INT_GPIO));

    /*
     * Step 7: Read interrupt registers after IRQ enable.
     */
    ntt_1205_dump_irq_registers("AFTER_ENABLE");

#endif

    DBGPRINT(
        "[NTT_1205_INIT] FINAL GPIO=%d IRQ_ENABLE=%d",
        hal_gpio_pin_get_val(ICP1205_INT_GPIO),
        IntReqHasEnable);
}
