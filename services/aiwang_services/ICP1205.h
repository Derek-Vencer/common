//***********************************************************************
//
// Copyright@2020, Shanghai Laiyuan Electronics Co,Ltd
// All rights reserved.
//
// File   Name     :    chip_register.h
// Design Name     :    Chip0000
//
// Author          :    by RegExpress Release Release 0.1
// Email           :    lx@icpow.com
// Data            :    2020-04-20
// Version         :    V1.0
//
// Abstract        :    Provides all register define in Chip0000
//                 :    Any hand edits will be lost when this file is regenerated!!// Called by       :
//
//
// Modification history
// ----------------------------------------------------------------------
//
// $Log$
//
//
//-----------------------------------------------------------------------

#ifndef __ICP1205_H__
#define __ICP1205_H__

typedef unsigned          char uint8_t;
typedef unsigned          char boolean_t;


///////////////////////////////////////////
//       ICP1205
///////////////////////////////////////////
#define      ICP1205_CHRG_CON1                    0x00
#define      ICP1205_CHRG_CON2                    0x01
#define      ICP1205_CHRG_CON3                    0x02
#define      ICP1205_CHRG_CON4                    0x03
#define      ICP1205_CHRG_STS1                    0x04
#define      ICP1205_CHRG_STS2                    0x05
#define      ICP1205_CHRG_STS3                    0x06
#define      ICP1205_CHRG_STS4                    0x07
#define      ICP1205_WDT_CON                      0x08
#define      ICP1205_SHIP_CON                     0x09
#define      ICP1205_CRCOM_CON1                   0x0a
#define      ICP1205_CRCOM_CON2                   0x0b

#define      ICP1205_CRCOM_CHR_DAT                0x0c
#define      ICP1205_CRCOM_WORD_DAT               0x000d
#define      ICP1205_CRCOM_INT_DATH               0x0d
#define      ICP1205_CRCOM_INT_DATL               0x0e

#define      ICP1205_CRCOM_TDAT                   0x0f

#define      ICP1205_COMM_CON                     0x10
#define      ICP1205_GPIO_CON                     0x11
#define      ICP1205_REV_ANA1                     0x12

#define      ICP1205_REV_DAT1                     0x14
#define      ICP1205_REV_DAT2                     0x15
#define      ICP1205_REV_DAT3                     0x16
#define      ICP1205_REV_DAT4                     0x17

#define      ICP1205_REG_1D                       0x1D


#define      ICP1205_INT_EN                       0x20
#define      ICP1205_INT_STAT1                    0x21
#define      ICP1205_INT_STAT2                    0x22
#define      ICP1205_INT_STAT3                    0x23

#define      ICP1205_INT_MASK1                    0x24
#define      ICP1205_INT_MASK2                    0x25
#define      ICP1205_INT_MASK3                    0x26


//////////////////////////////////////////////////////////////////

#define      ICP1205_DVCID                    0xC2


#define CHRGCON1_CHRG_EN        (0x1u)           /*!< 充电使能 */
#define CHRGCON1_RECHRG_EN      (0x2u)           /*!< 重新充电使能 */
#define CHRGCON1_NTC_EN         (0x4u)           /*!< NTC使能 */
#define CHRGCON1_ITERM_SEL      (0x8u)           /*!< 充电截止比例选择。0:1/10；   1:1/5 */



#define CHRGSTS1_CHRGSTS_BITS        (0x7u)         /*!< 充电状态：0空闲/2涓流模式/3快充模式/4慢充模式/5充电延时/6充满/7错误状态*/
#define CHRGSTS1_CHRG_IDLE_STS       (0x0u)         /*!< 充电状态：0空闲*/
#define CHRGSTS1_CHRG_TRCKL1_STS     (0x1u)         /*!< 充电状态：2涓流模式*/
#define CHRGSTS1_CHRG_TRCKL_STS      (0x2u)         /*!< 充电状态：2涓流模式*/
#define CHRGSTS1_CHRG_FAST_STS       (0x3u)         /*!< 充电状态：3快充模式态*/
#define CHRGSTS1_CHRG_NOR_STS        (0x4u)         /*!< 充电状态：4慢充模式*/
#define CHRGSTS1_CHRG_DLYTIME_STS    (0x5u)         /*!< 充电状态：5充电延时*/
#define CHRGSTS1_CHRG_FINSH_STS      (0x6u)         /*!< 充电状态：6充满/7错误状态*/
#define CHRGSTS1_CHRG_ERR_STS        (0x7u)         /*!< 7错误状态*/



#define CHRGSTS2_VIN_STS         (0x1u)           /*!< 0：Vin无电；1：Vin>1.5V有电 ，有100mS防抖*/
#define CHRGSTS2_SHORT_STS       (0x1u<<5)        /*!< 输出短路状态 0:正常; 1:短路 */
#define CHRGSTS2_VIN_OVP_STS     (0x1u<<6)        /*!< 输入过压（大于6.0V） 0:正常; 1:短路 */
#define CHRGSTS2_VIN_UVLO_STS    (0x1u<<7)        /*!< 输入欠压（小于3.3V） 0:正常; 1:短路 */

#define CHRGSTS3_ITERM_STS       (0x1u)           /*!< 截止电流状态 0:未截止;1:截止 */
#define CHRGSTS3_VIN2BAT_0P15CMP (0x1u<<1)        /*!< 充电压差0p15伏比较器 0:小于；1:大于*/
#define CHRGSTS3_VIN2BAT_0P35CMP (0x1u<<2)        /*!< 充电压差0p35伏比较器 0:小于；1:大于 */
#define CHRGSTS3_CV_STS          (0x1u<<3)        /*!< 充电恒压状态 0:其他充电状态；1:恒压充电状态（可快充）*/
#define CHRGSTS3_TRICKLE_STS     (0x1u<<4)        /*!< 充电涓流状态 0:其他充电状态；1:涓流充电状态 */
#define CHRGSTS3_NTC_STS         (0x1u<<5)        /*!< NTC状态 */
#define CHRGSTS3_NTC_L_STS       (0x1u<<6)        /*!< 低温NTC状态 */
#define CHRGSTS3_NTC_H_STS       (0x1u<<7)        /*!< 高温NTC状态 */


#define CRCOMCON2_CHR_DATA_UPDT_F   (0x1u)        /*!< 载波接收char数据寄存器更新标志，0:没有更新；1:更新*/
#define CRCOMCON2_INT_DATA_UPDT_F   (0x1u<<1)     /*!< 载波接收整型数据寄存器更新标志，0:没有更新；1:更新*/


#define COMMCON_BUSY_STS       	(0x1u<<4)       /*!< 透传模式标志 0:非透传；1:透传*/

#define GPIOCON_GP0_STS       	(0x1u<<0)       /*!< GPIO0状态 可通过I2C读写，也可以通过透传指令控制*/
#define GPIOCON_KEY_STS       	(0x1u<<1)       /*!< 按键状态 仅可读*/

#define INT1_PLGIN_FLG       	(0x1u<<0)      
#define INT1_PLGOUT_FLG         (0x1u<<1)      
#define INT1_CHRGERR_FLG        (0x1u<<2)       
#define INT1_WDTOVTIME_FLG      (0x1u<<4)      
#define INT1_BATLOW_FLG         (0x1u<<5)      
#define INT1_GP0CH_FLG          (0x1u<<6)      
#define INT1_CHRGFIN_FLG        (0x1u<<7)      

#define INT2_CHRGSTS_CH_FLG     (0x1u<<0)      
#define INT2_CCCV_TIMEOUT_FLG   (0x1u<<1)      
#define INT2_TRCKL_TIMEOUT_FLG  (0x1u<<2)      
#define INT2_NTC_FLG            (0x1u<<3)      
#define INT2_CVCHRG_FLG         (0x1u<<4)      
#define INT2_0P15CMP_FLG        (0x1u<<5)      
#define INT2_0P35CMP_FLG        (0x1u<<6)      
#define INT2_ITERM_FLG          (0x1u<<7)      

#define INT3_KEYDN_FLG       	(0x1u<<0)       
#define INT3_KEYUP_FLG       	(0x1u<<1)       
#define INT3_CRRXCMD_FLG       	(0x1u<<2)       
#define INT3_CRRXERR_FLG       	(0x1u<<3)       
#define INT3_TRCOM_FLG      	(0x1u<<4)
#define C0MMON_BUSY_FLAG     	(0x1u<<4)


#define INT1_PLGIN_MSK       	(0x1u<<0)      
#define INT1_PLGOUT_MSK         (0x1u<<1)      
#define INT1_CHRGERR_MSK        (0x1u<<2)      
#define INT1_ANARDY_MSK         (0x1u<<3)      
#define INT1_WDTOVTIME_MSK      (0x1u<<4)      
#define INT1_BATLOW_MSK         (0x1u<<5)      
#define INT1_GP0CH_MSK          (0x1u<<6)      
#define INT1_CHRGFIN_MSK        (0x1u<<7)      

#define INT2_CHRGSTS_CH_MSK     (0x1u<<0)       
#define INT2_CCCV_TIMEOUT_MSK   (0x1u<<1)       
#define INT2_TRCKL_TIMEOUT_MSK  (0x1u<<2)       
#define INT2_NTC_MSK            (0x1u<<3)       
#define INT2_CVCHRG_MSK         (0x1u<<4)       
#define INT2_0P15CMP_MSK        (0x1u<<5)       
#define INT2_0P35CMP_MSK        (0x1u<<6)       
#define INT2_ITERM_MSK          (0x1u<<7)       

#define INT3_KEYDN_MSK       	(0x1u<<0)       /*!< 按键按下标志*/
#define INT3_KEYUP_MSK       	(0x1u<<1)       /*!<  按键抬起标志*/
#define INT3_CRRXCMD_MSK       	(0x1u<<2)       /*!< 载波接收成功中断标志*/
#define INT3_CRRXERR_MSK       	(0x1u<<3)       /*!< 载波接收错误中断标志*/
#define INT3_TRCOM_MSK      	(0x1u<<4)       /*!< 透传模式中断*/


//载波命令
#define CR_REC_CMD_QUE_CHRG_STS   (0x08<<4)      /*!< 载波接收到查询充电状态命令，无需回复*/
#define CR_REC_CMD_READ_DATA      (0x09<<4)      /*!< 载波接收到查询数据寄存器数据，可以更新新的数据*/
#define CR_REC_CMD_FRC_CHRG_FNSH  (0xAu<<4)      /*!< 载波强制充电完成*/
#define CR_REC_CMD_SHIP_MOD       (0xBu<<4)      /*!< 载波强制运输模式*/
#define CR_REC_CMD_CHAR_DATA      (0xCu<<4)      /*!< 载波发送1字节数据*/
#define CR_REC_CMD_WORD_DATA      (0xDu<<4)      /*!< 载波发送2字节数据*/
#define CR_REC_CMD_CLR_GP0        (0xEu<<4)      /*!< 载波强制GP0=0*/
#define CR_REC_CMD_SET_GP0        (0xFu<<4)      /*!< 载波强制GP0=1*/






/******************************************************************************
 * Global type definitions
 ******************************************************************************/
typedef enum en_icp1205_chrgfun   
{
    ChrgDisable  = 0x0u,               ///< 
    ChrgEnable   = 0X1u,               ///< 
}en_icp1205_chrgfun_t;

typedef enum en_icp1205_rechrgfun   
{
    RechrgDisable  = 0x0u,               ///< 
    RechrgEnable   = 0X2u,               ///< 
}en_icp1205_rechrgfun_t;

typedef enum en_icp1205_ntcfun   
{
    NtcDisable  = 0x0u,               ///< 
    NtcEnable   = 0X4u,               ///< 
}en_icp1205_ntcfun_t;

typedef enum en_icp1205_iterm_div   
{
    Iterm1Div10  = 0x0u,               ///< 
    Iterm1Div5   = 0X8u,               ///< 
}en_icp1205_iterm_div_t;

/*********************
**充电恒压值(4100~4400)
*********************/
typedef enum en_icp1205_chrg_cv_sel   
{
    Cv4100mV  = 0u<<4,               ///< 
    Cv4150mV  = 1u<<4,               ///< 
    Cv4200mV  = 2u<<4,               ///< 
    Cv4250mV  = 3u<<4,               ///< 
    Cv4300mV  = 4u<<4,               ///< 
    Cv4350mV  = 5u<<4,               ///< 
    Cv4400mV  = 6u<<4,               ///< 
    //Cv4400mV  = 7u<<4,               ///< 
}en_icp1205_chrg_cv_sel_t;



/*********************
**默认充电恒流值设定

==芯片有快速充电与默认充电模式，在快速充电过程中，
==如果遇到芯片过温等异常情况时，会自动切换到默认充电的档位
*********************/
typedef enum en_icp1205_normal_cc_sel   
{
    NorCc25mA  = 0u,               ///< 
    NorCc30mA  = 1u,               ///< 
    NorCc35mA  = 2u,               ///< 
    NorCc40mA  = 3u,               ///< 
    NorCc45mA  = 4u,               ///< 
    NorCc50mA  = 5u,               ///< 
    NorCc55mA  = 6u,               ///< 
    NorCc60mA  = 7u,               ///< 
    NorCc75mA  = 8u,               ///< 
    NorCc90mA  = 9u,               ///< 
    NorCc105mA  = 10u,               ///< 
    NorCc130mA  = 11u,               ///< 
    NorCc145mA  = 12u,               ///< 
    NorCc150mA  = 13u,               ///< 
    NorCc155mA  = 14u,               ///< 
    NorCc160mA  = 15u,               ///< 
}en_icp1205_normal_cc_sel_t;


/*********************
**默认充电恒流值设定

==芯片有快速充电与默认充电模式，在快速充电过程中，
==在CC充电、或者遇到芯片过温等异常情况时，会自动切换到默认充电的档位
*********************/
typedef enum en_icp1205_fast_cc_sel   
{
    FastCc25mA  = (0u<<4),               ///< 
    FastCc30mA  = (1u<<4),               ///< 
    FastCc35mA  = (2u<<4),               ///< 
    FastCc40mA  = (3u<<4),               ///< 
    FastCc45mA  = (4u<<4),               ///< 
    FastCc50mA  = (5u<<4),               ///< 
    FastCc55mA  = (6u<<4),               ///< 
    FastCc60mA  = (7u<<4),               ///< 
    FastCc75mA  = (8u<<4),               ///< 
    FastCc90mA  = (9u<<4),               ///< 
    FastCc105mA  =(10u<<4),               ///< 
    FastCc120mA  =(11u<<4),               ///< 
    FastCc135mA  =(12u<<4),               ///< 
    FastCc150mA  =(13u<<4),               ///< 
    FastCc165mA  =(14u<<4),               ///< 
    FastCc180mA  =(15u<<4),               ///< 
}en_icp1205_fast_cc_sel_t;






/*********************
**充电延时设定

==触发充电截止电流后，继续充电的时间
*********************/
typedef enum en_icp1205_chrg_dlytime_sel   
{
    Delay4min   = 0u,               ///< 
    Delay8min   = 1u,               ///< 
    Delay16min  = 2u,               ///< 
    Delay0min   = 3u,               ///< 
}en_icp1205_chrg_dly_sel_t;


/*********************
**充电超时设定

==充电超时自己关闭设置
*********************/
typedef enum en_icp1205_chrg_overtime_sel   
{
    ChrgOverTime2hour   =(0u<<2),               ///< 
    ChrgOverTime3hour   =(1u<<2),               ///< 
    ChrgOverTime4hour   =(2u<<2),               ///< 
    ChrgOverTimeUnlimt  =(3u<<2),               ///< 无限制
}en_icp1205_chrg_overtime_sel_t;



/*********************
**充电功能配置
*注：Iterm的占空比值未在结构体中体现，如需调整，可以在配置函数中去修改默认值
*********************/
typedef struct stc_icp1205_chrg_cfg
{
    en_icp1205_chrgfun_t       			ChrgFun;     			///<  充电功能
    en_icp1205_rechrgfun_t     			RechrgFun;   			///<  重新充电功能
    en_icp1205_ntcfun_t        			NtcFun;      			///< 充电NTC保护功能
    en_icp1205_iterm_div_t    			ItermDiv;    			///< 充电截止电流比例设置
    en_icp1205_chrg_cv_sel_t   			CvSel;       			///< 恒压充电电压设置
	en_icp1205_normal_cc_sel_t 			NorCcSel;    			///< 正常恒流电流设置
	en_icp1205_fast_cc_sel_t   			FastCcSel;   			///< 快充恒流电流设置
	en_icp1205_chrg_dly_sel_t  			ChrgDlySel;  			///< 充电延时截止设置
	en_icp1205_chrg_overtime_sel_t  ChrtOverTimeSel;  ///<充电超时设置
	
} stc_icp1205_chrg_cfg_t;


/*********************
**WDT 使能
*********************/
typedef enum en_icp1205_wdt_fun_en   
{
   WdtDisable  = 0x10,               ///< 
   WdtEnable  =  0x11,               ///< 
}en_icp1205_wdt_fun_en_t;

/*********************
**WDT 复位时间
*********************/
typedef enum en_icp1205_wdt_tim_sel   
{
   WdtTim4S  =  0xc0,               ///< 
   WdtTim8S  =  0xc4,               ///< 
   WdtTim16S  = 0xc8,               ///< 
   WdtTim32S  = 0xcc,               ///< 
}en_icp1205_wdt_tim_sel_t;


/*********************
**SHIP模式屏蔽时间
*********************/
typedef enum en_icp1205_ship_msktim_sel   
{
   ShipMskTim4S  =  0xc0,               ///< 
   ShipMskTim8S  =  0xc4,               ///< 
   ShipMskTim16S  = 0xc8,               ///< 
   ShipMskTim32S  = 0xcc,               ///< 
}en_icp1205_ship_msktim_sel_t;


/*********************
**载波模式设置
*********************/
typedef enum en_icp1205_cr_com_rx_fun   
{
    CrComRxDisable  = 0x0u,               ///< 
    CrComRxEnable   = 0X1u,               ///< 载波接收使能
}en_icp1205_cr_com_rx_fun_t;


typedef enum en_icp1205_cr_com_tx_fun   
{
    CrComTxDisable  = 0x0u,               ///< 
	  CrComTxEnable   = 0X2u,               ///< 载波发送使能
}en_icp1205_cr_com_tx_fun_t;

/*********************
**载波接收到数据回复ACK的延时时间
--说明：ICP1106发送完数据或，需要配置成载波接收模式才能正常接收，
        因此ICP1205回复应答信号时，需要延时
*********************/
typedef enum en_icp1205_cr_com_ask_delay_sel   
{
    CrComAckDly20mS   = (0x0u<<2),       ///< 
    CrComAckDly30mS   = (0x1u<<2),       ///< 
    CrComAckDly40mS   = (0x2u<<2),       ///< 
    CrComAckDly50mS   = (0x3u<<2),       ///< 
}en_icp1205_cr_com_ack_delay_sel_t;

typedef struct stc_icp1205_cr_com_cfg
{
    en_icp1205_cr_com_rx_fun_t      	CrComRxFun;        ///< 载波接收功能使能
	  en_icp1205_cr_com_tx_fun_t     		CrComTxFun;   		 ///< 载波发送功能使能
    en_icp1205_cr_com_ack_delay_sel_t CrComAckDlySel;    ///< 载波应答延时
} stc_icp1205_cr_com_cfg_t;

/*********************
**透传通讯模式设置
*********************/
typedef enum en_icp1205_tr_com_fun   
{
    TrComDisable  = 0x0u,               ///< 
    TrComEnable   = 0X1u,               ///< 透传功能使能
}en_icp1205_tr_com_fun_t;


typedef enum en_icp1205_trcom_mod_sel   
{
	  TrComGpioCtrMode    = 0x0u,            ///< 透传方向由IO口控制，RTMOD低时接收，高时发送
	  TrComRegCtrRxMode   = 0X2u,            ///< 寄存器控制方向时，透传接收模式
	  TrComRegCtrTxMode   = 0x6u,            ///< 寄存器控制方向时，透传发送模式
}en_icp1205_trcom_mod_sel_t;




/******************************************************************************
 * Global variable declarations ('extern', definition in C source)
 *****************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

void Icp1205SetChrgFunEnable(void);
void Icp1205SetChrgFunDisable(void);
void Icp1205WdtFun(en_icp1205_wdt_fun_en_t Icp1205_Wdt_fun);
void Icp1205WdtWeek(void);
void Icp1205ShipEnable(void);
void Icp1205ShipMaskTimSel(en_icp1205_ship_msktim_sel_t icp1106_ship_tim_sel);
void Icp1205DefInit(void);
void Icp1205ClearIntFlag(void);
void Icp1205IntEnable(void);
void Icp1205IntDisable(void);

#ifdef __cplusplus
}
#endif
/******************************************************************************
 * Global function prototypes (definition in C source)
 *****************************************************************************/
#endif

