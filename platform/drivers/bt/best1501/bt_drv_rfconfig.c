/***************************************************************************
 *
 * Copyright 2015-2019 BES.
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a database or retrieval system, without prior written
 * permission of BES.
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary information of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/
#include <string.h>
#include "plat_types.h"
#include "hal_i2c.h"
#include "hal_uart.h"
#include "bt_drv.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "cmsis.h"
#include "hal_cmu.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "tgt_hardware.h"
#include "bt_drv_internal.h"
#include "bt_drv_1501_internal.h"
#include "hal_chipid.h"
#include "bt_drv_interface.h"
#include "bt_drv_reg_op.h"
#include "pmu.h"
#include "iqcorrect.h"
#include "bt_drv_iq_common.h"
#ifdef __RF_INIT_XTAL_CAP_FROM_NV__
#include "nvrecord_dev.h"
#endif
#define AUTO_CAL                                0

#ifndef BT_RF_MAX_XTAL_TUNE_PPB
// Default 10 ppm/bit or 10000 ppb/bit
#define BT_RF_MAX_XTAL_TUNE_PPB                 10000
#endif

#ifndef BT_RF_XTAL_TUNE_FACTOR
// Default 0.2 ppm/bit or 200 ppb/bit
#define BT_RF_XTAL_TUNE_FACTOR                  200
#endif


#define XTAL_FCAP_NORMAL_SHIFT                  0
#define XTAL_FCAP_NORMAL_MASK                   (0x1FF << XTAL_FCAP_NORMAL_SHIFT)
#define XTAL_FCAP_NORMAL(n)                     BITFIELD_VAL(XTAL_FCAP_NORMAL, n)

#define RF_REG_XTAL_FCAP                        0x95
#define RF_REG_XTAL_CMOM_DR                     0x95

#define SPI_TRIG_RX_NEG_TIMEOUT                 MS_TO_TICKS(3)

#define SPI_TRIG_NEG_TIMEOUT                    SPI_TRIG_RX_NEG_TIMEOUT

#define BTRF_DELAY_CAL_TIME                          16

enum BIT_OFFSET_CMD_TYPE_T
{
    BIT_OFFSET_CMD_STOP = 0,
    BIT_OFFSET_CMD_START,
    BIT_OFFSET_CMD_ACK,
};

static uint16_t xtal_fcap = DEFAULT_XTAL_FCAP;
static uint16_t init_xtal_fcap = DEFAULT_XTAL_FCAP;

struct bt_drv_tx_table_t
{
    uint16_t tbl[16][3];
};


struct RF_SYS_INIT_ITEM
{
    uint16_t reg;
    uint16_t set;
    uint16_t mask;
    uint16_t delay;
};

static const struct RF_SYS_INIT_ITEM rf_sys_init_tbl[] =
{
    {0x00E9,0x0000,0x000F,1},
};

#define REG_EB_VAL 0x083f
#define REG_181_VAL (0x00bf)
#define REG_EC_VAL 0x081f
#define REG_182_VAL 0x00bf
#define REG_ED_VAL 0x091f
#define REG_183_VAL 0x00bf
#define REG_EE_VAL 0x0b1f
#define REG_184_VAL 0x00bf
#define REG_EF_VAL 0x00c7
#define REG_185_VAL 0x00bf
#define REG_F0_VAL 0x0147
#define REG_186_VAL 0x00bf
#define REG_F1_VAL 0x0347
#define REG_187_VAL 0x00af
#define REG_F2_VAL 0x0347
#define REG_188_VAL 0x008d

const uint16_t rf_init_tbl_1[][3] =
{
    {0x0081,0x0104,1},
    {0x0082,0x0a10,1},
    {0x0083,0x0a0a,1},
    {0x0084,0x1320,1},//delay pa on time to cover locking ripple by walker mail 20201020
    {0x008b,0x0806,1},
    {0x008f,0x0008,1},//close dpa
    {0x009e,0x0072,1},//reg_bt_rfpll_cnt_time_tx[7:5] by walker mail 20201016
    {0x009f,0x8000,1},

    {0x00a0,0x2040,1},
    {0x00a1,0x0052,1},//reg_bt_rfpll_cnt_time_rx[7:5] by walker mail 20201016
    {0x00a5,0x8400,1},//reg_bt_rfpll_dither_bypass_tx by xulicheng 20201106
    {0x00AA,0x0158,1},//Increase rxvco current to prevent rxpll losing lock By Xulicheng 20210507
    {0x00b7,0x20bf,1},//tx_gain_sel_en

    {0x01E4,0x0009,1},

    {0x0283,0x0910,1},
    {0x0297,0x3F10,1},//Increase pll bandwidth and speed up locking by Xulilcheng mail 20210104
    {0x0299,0x1C14,1},
    {0x029e,0x3848,1},
    {0x029f,0x0040,1},// improve ACP by walker mail 20201208
    {0x02a0,0x7230,1},// tmx bsp ictrl
#ifdef HIGH_EFFICIENCY_TX_PWR_CTRL
    {0x02a2,0x2901,1},//decrease pa current for better MC by luobin mail 20210319
#else
    {0x02a2,0x2800,1},//decrease pa current for better MC by walker mail 20201020
#endif
    {0x02b6,0x48A8,1},
    {0x02b9,0x1950,1},
    {0x02bb,0x3441,1},//optimize ADC mismatch by Fangfei
    {0x02bd,0x0407,1},

    {0x0389,0x1ac0,1},///padrvgain=0

    {0x03b0,0x1796,1},//strengthen the dac clock drive capability by xulicheng 20201106

    {0x00E8,0x0000,1},//release lna dr
    {0x00bf,0x0000,1},//release filter dr
    {0x00c0,0x0006,1},//
    {0x03ad,0x0000,1},//lna_ic=110

    {0x01E4,0x0000,1},
    {0x01E7,0x2494,1},
    {0x01E8,0x0049,1},
    {0x01Ed,0x9248,1},
    {0x01E4,0x0024,1},

    //rx gain
    {0x00cd,0x683f,1},//gain idx 0
    {0x00d5,0x01ce,1},

    {0x00ce,0x481c,1},//gain idx 1
    {0x00d6,0x01ce,1},

    {0x00cf,0x00c7,1},//gain idx 2
    {0x00d7,0x01ce,1},

    {0x00d0,0x00c7,1},//gain idx 3
    {0x00d8,0x01be,1},

    {0x00d1,0x01c7,1},//gain idx 4
    {0x00d9,0x01be,1},

    {0x00d2,0x0147,1},//gain idx 5
    {0x00da,0x01be,1},

    {0x00d3,0x0347,1},//gain idx 6
    {0x00db,0x01ae,1},

    {0x00d4,0x0347,1},//gain idx 7
    {0x00dc,0x018d,1},

    {0x00b6,0x4c80,1},  //disable hwagc en

    {0x2C1,0x117F,1},   //increase sar adc voltage by Fangfei
    //[vco pnoise]
    //{0x2C2,0x6768,1},
    //{0x2C3,0x23C0,1},
    //{0x2CB,0x8174,1},

    //[lowpower pll] by xwy
    {0x3ca,0x3900,1},
    {0x2c2,0x4728,1},//slightly increase the current of rfpll IF divider and rxvco by xulicheng 20201109
    {0x2c4,0x0582,1},//improve driving ability at low temperature by xulicheng mail 20201221
    {0x2c7,0x2944,1},//slightly increase the current of the rfpll IF divider to improve reliability by xulicheng 20201118
    {0x2c8,0x1971,1},
    {0x2c9,0x8294,1},//for stable trx pll
    {0x2cb,0x2124,1},

    {0x2a5,0x0101,1},///iqcal pu =0
    {0x1e7,0x2492,1},////set by luobin
    {0x1ed,0x9249,1},
    {0xbc,0x4e10,1},

#ifdef BT_I2V_VAL_RD
////fangfei add for block det
    {0x008c,0x1000,1},
    {0x00b6,0x4c80,1},//enable hwagc en
    {0x00b7,0x20a1,1},//tx_gain_sel_en
    {0x0181,0x4436,1},//smp_sel=010
    {0x0182,0x803e,1},
    {0x0183,0xc001,1},//stop_en_1=111; restart_disable=0000
    {0x0189,0x0310,1},//burst_mode_en=1
    {0x018a,0x1010,1},//burst_mode_en=0
    {0x018b,0x1f1f,1},//burst_mode_en=0
    {0x018e,0x0305,1},//lna_threshold
    {0x018f,0x0306,1},//i2v threshold
    {0x0195,0x04bb,1},//i2v threshold
    {0x01a1,0x101c,1},//lna_gain_value
    {0x01a2,0x2f38,1},//lna_rssi_lim
    {0x01a3,0x5f65,1},//i2v_rssi_lim
    {0x01a4,0x0075,1},//i2v_max gain rssi lim
    {0x01a6,0x0006,1},//rssi_lim_rxpwr
    {0x01a8,0x0003,1},//max_gain_rssi_lim_en
    {0x02be,0x1138,1},//abb_pdt_vcm
    {0x03ab,0x0280,1},//abb_pdt_op_gain
#endif

#ifdef BT_IF_750K
    {0x00b3,0x0300,1},//bt_rfpll_lo_freq
    //{0x00bb,0x4500,1},//filter cap * 1.25; flt_bw=001
    {0x00bd,0x0311,1},
    //{0x02b9,0x1958,1},//filter cplx res=1.25M
#elif defined(BT_IF_1M)
    {0x00b3,0x0400,1},
    {0x00bd,0x0311,1},//filter lowpass
#elif defined(BT_IF_1P05M)
    {0x00b3,0x0433,1},
    {0x00bd,0x0311,1},//filter lowpass
#endif
};

const uint16_t rf_init_tbl_2[][3] =
{
    {0x2FD,0x91,1},
    {0x2FE,0x01,1},
    {0x2ED,0x02,1},
    {0x2EE,0x0a,1},
};

#ifdef __HW_AGC__
const uint16_t rf_init_tbl_1_hw_agc[][3] = //hw agc table
{
   //{0x00b6,0x4c82,1},//enable hwagc en
    //rxgain
    {0x00cd,0x03c8,1},
    {0x00ce,0x00c8,1},
    {0x00cf,0x481c,1},

    {0x01e7,0x0121,1},///should check by luobin andn fangfei
//    {0x00E8,0x0000,1},//release lna dr
//    {0x00E9,0x1100,1},//release sw itxb dr
//    {0x03ad,0x0000,1},//release lna_ic
//    {0x00bf,0x0000,1},//release filter dr
    {0x00c0,0x0000,1},//release adc sel gain
    {0x01E4,0x0020,1},//release i2v rin


    {0x0181,0x4436,1},//smp_sel=010
    {0x0182,0x803f,1},
    {0x0183,0xe001,1},//stop_en_1=111; restart_disable=0000
    {0x0189,0x0010,1},//burst_mode_en=0
    {0x018e,0x0305,1},//lna_threshold
    {0x018f,0x0306,1},//i2v threshold
    {0x0195,0x04bb,1},//i2v threshold
    {0x01a1,0x101c,1},//lna_gain_value
    {0x01a2,0x2f38,1},//lna_rssi_lim
    {0x01a3,0x5f65,1},//i2v_rssi_lim
    {0x01a4,0x0075,1},//i2v_max gain rssi lim
    {0x01a6,0x0006,1},//rssi_lim_rxpwr
    {0x01a8,0x0003,1},//max_gain_rssi_lim_en
    {0x02be,0x1138,1},//abb_pdt_vcm
    {0x03ab,0x0280,1},//abb_pdt_op_gain
};
#endif //__HW_AGC__

struct btdrv_customer_rf_config_t btdrv_rf_customer_config;
void bt_drv_rf_sdk_init(void)
{
    DRIVERS_TRACE(0,"SDK default rf config");
    btdrv_rf_customer_config.config_xtal_en = false;//get value from NV
    btdrv_rf_customer_config.xtal_cap_val = 0;//get value from NV

    //config TX power in dbm
    btdrv_rf_customer_config.config_tx_pwr_en = true;

    btdrv_rf_customer_config.bt_tx_page_pwr = TX_PWR_8DBM;//in dbm
    btdrv_rf_customer_config.bt_tx_max_pwr  = TX_PWR_15DBM;//in dbm
    btdrv_rf_customer_config.bt_tx_idx4_pwr = TX_PWR_12DBM;//in dbm
    btdrv_rf_customer_config.bt_tx_idx3_pwr = TX_PWR_8DBM;//in dbm
    btdrv_rf_customer_config.bt_tx_idx2_pwr = TX_PWR_4DBM;//in dbm
    btdrv_rf_customer_config.bt_tx_idx1_pwr = TX_PWR_0DBM;//in dbm
    btdrv_rf_customer_config.bt_tx_idx0_pwr = TX_PWR_N3DBM;//in dbm

    btdrv_rf_customer_config.le_tx_max_pwr  = TX_PWR_15DBM;//in dbm
    btdrv_rf_customer_config.le_tx_idx4_pwr = TX_PWR_12DBM;//in dbm
    btdrv_rf_customer_config.le_tx_idx3_pwr = TX_PWR_8DBM;//in dbm
    btdrv_rf_customer_config.le_tx_idx2_pwr = TX_PWR_4DBM;//in dbm
    btdrv_rf_customer_config.le_tx_idx1_pwr = TX_PWR_0DBM;//in dbm
    btdrv_rf_customer_config.le_tx_idx0_pwr = TX_PWR_N3DBM;//in dbm

    //init BLE convert table
    btdrv_txpwr_conv_tbl[0] = btdrv_rf_customer_config.le_tx_idx0_pwr;
    btdrv_txpwr_conv_tbl[1] = btdrv_rf_customer_config.le_tx_idx1_pwr;
    btdrv_txpwr_conv_tbl[2] = btdrv_rf_customer_config.le_tx_idx2_pwr;
    btdrv_txpwr_conv_tbl[3] = btdrv_rf_customer_config.le_tx_idx3_pwr;
    btdrv_txpwr_conv_tbl[4] = btdrv_rf_customer_config.le_tx_idx4_pwr;
    btdrv_txpwr_conv_tbl[5] = btdrv_rf_customer_config.le_tx_max_pwr;
    btdrv_txpwr_conv_tbl[6] = btdrv_rf_customer_config.le_tx_max_pwr;
    btdrv_txpwr_conv_tbl[7] = btdrv_rf_customer_config.le_tx_max_pwr;
}

void bt_drv_rf_set_customer_config(struct btdrv_customer_rf_config_t* config)
{
    memset(&btdrv_rf_customer_config, 0, sizeof(struct btdrv_customer_rf_config_t));
    if(config !=NULL)
    {
        btdrv_rf_customer_config = *config;
    }
    else
    {
        bt_drv_rf_sdk_init();
    }
}

void bt_drv_rf_set_bt_hw_agc_enable(bool enable)
{
    uint16_t val_e2 = 0;
    btdrv_read_rf_reg(0xb6,&val_e2);
    if(enable)
    {
        //open rf bt hw agc mode
        val_e2 |= (1<<1);
    }
    else
    {
        //close rf bt hw agc mode
        val_e2 &= ~(1<<1);
    }
    btdrv_write_rf_reg(0xb6,val_e2);
}

void bt_drv_rf_set_ble_hw_agc_enable(bool enable)
{
    uint16_t val_e2 = 0;
    btdrv_read_rf_reg(0xb7,&val_e2);
    if(enable)
    {
        //open rf ble hw agc mode
        val_e2 |= (1<<11);
    }
    else
    {
        //close rf ble hw agc mode
        val_e2 &= ~(1<<11);
    }
    btdrv_write_rf_reg(0xb7,val_e2);
}

void bt_drv_rf_set_afh_monitor_gain(void)
{
    uint16_t val_e2 = 0;
    btdrv_read_rf_reg(0x1e4,&val_e2);//reg_bt_i2v_rin_afh[6:4]
    //i2v_rin_afh = 2 i2v_stb_afh=1
    val_e2 &= ~(7<<4);
    val_e2 |= (2<<4);
    val_e2 &= ~(7<<12); //reg_bt_i2v_stb_afh[14:12]
    val_e2 |= (1<<12); //reg_bt_i2v_stb_afh[14:12]
    btdrv_write_rf_reg(0x1e4,val_e2);


    btdrv_read_rf_reg(0x3cb,&val_e2); //lna_ic = 0 , i2v_gain = 7  lna_ldo_ic = 0xb
    val_e2 &= ~(0xf<<0);//reg_bt_i2v_gain_afh[3:0]
    val_e2 |= (7<<0);

    val_e2 &= ~(3<<4);//reg_bt_adc_vin_sel_afh[5:4]
    val_e2 |= (1<<4);

    val_e2 &= ~(0xF<<8);//reg_bt_lna_ldo_ic_afh[11:8]
    val_e2 |= (0x5<<8);

    val_e2 &= ~(7<<12);// reg_bt_lna_ic_afh[14:12]

    val_e2 &= ~(3<<6);//reg_bt_adc_vin_sel_normal[7:6]
    val_e2 |= (1<<6);

    btdrv_write_rf_reg(0x3cb,val_e2);


    btdrv_read_rf_reg(0x3cc,&val_e2); //flt = 7, hg_en=0, hg=0, rpass=11, rin=00,rfflt=0

    val_e2 = 0x1E0C;

    btdrv_write_rf_reg(0x3cc,val_e2);
}

uint32_t btdrv_rf_get_max_xtal_tune_ppb(void)
{
    return BT_RF_MAX_XTAL_TUNE_PPB;
}

uint32_t btdrv_rf_get_xtal_tune_factor(void)
{
    return BT_RF_XTAL_TUNE_FACTOR;
}

void btdrv_rf_init_xtal_fcap(uint32_t fcap)
{
    xtal_fcap = SET_BITFIELD(xtal_fcap, XTAL_FCAP_NORMAL, fcap);
    btdrv_write_rf_reg(RF_REG_XTAL_FCAP, xtal_fcap);
    init_xtal_fcap = xtal_fcap;
}

uint32_t btdrv_rf_get_init_xtal_fcap(void)
{
    return GET_BITFIELD(init_xtal_fcap, XTAL_FCAP_NORMAL);
}

uint32_t btdrv_rf_get_xtal_fcap(void)
{
    return GET_BITFIELD(xtal_fcap, XTAL_FCAP_NORMAL);
}

void btdrv_rf_set_xtal_fcap(uint32_t fcap, uint8_t is_direct)
{
    xtal_fcap = SET_BITFIELD(xtal_fcap, XTAL_FCAP_NORMAL, fcap);
    btdrv_write_rf_reg(RF_REG_XTAL_FCAP, xtal_fcap);
}

int btdrv_rf_xtal_fcap_busy(uint8_t is_direct)
{
    return 0;
}

uint32_t btdrv_rf_bit_offset_get(void)
{
    return 0;
}

uint16_t btdrv_rf_bitoffset_get(uint8_t conidx)
{
    return bt_drv_reg_op_bitoff_getf(conidx);
}

void btdrv_rf_log_delay_cal(void)
{
    unsigned short read_value;
    unsigned short write_value;

    DRIVERS_TRACE(1,"%s", __func__);
    BTDIGITAL_REG(0xd0340020) = 0x010e01c0;
    DRIVERS_TRACE(1,"0xd0340020 =%x\n",BTDIGITAL_REG(0xd0340020) );

    btdrv_write_rf_reg(0xd4, 0x000f);
    btdrv_write_rf_reg(0xd5, 0x4000);
    btdrv_write_rf_reg(0xd2, 0x1003);
    btdrv_write_rf_reg(0xa7, 0x004e);
    btdrv_write_rf_reg(0xd4, 0x0000);
    btdrv_write_rf_reg(0xd5, 0x4002);

    BTDIGITAL_REG(0xd0340020) = 0x030e01c1;
    DRIVERS_TRACE(1,"0xd0340020 =%x\n",BTDIGITAL_REG(0xd0340020) );

    btdrv_delay(1);

    btdrv_write_rf_reg(0xd2, 0x5003);

    btdrv_delay(1);

    btdrv_read_rf_reg(0x1e2, &read_value);
    DRIVERS_TRACE(1,"0x1e2 read_value:%x\n",read_value);
    if(read_value == 0xff80)
    {
        btdrv_write_rf_reg(0xd3, 0xffff);
    }
    else
    {
        write_value = ((read_value>>7)&0x0001) | ((read_value & 0x007f)<<1) | ((read_value&0x8000)>>7) | ((read_value&0x7f00)<<1);
        DRIVERS_TRACE(1,"d3 write_value:%x\n",write_value);
        btdrv_write_rf_reg(0xd3, write_value);
    }
    btdrv_delay(1);

    BTDIGITAL_REG(0xd0340020) = 0x010e01c0;
    DRIVERS_TRACE(1,"0xd0340020 =%x\n",BTDIGITAL_REG(0xd0340020) );


    btdrv_write_rf_reg(0xd4, 0x000f);
    btdrv_write_rf_reg(0xd2, 0x1003);
    btdrv_write_rf_reg(0xd5, 0x4000);

}

void btdrv_rf_log_delay_cal_init(void)
{
    unsigned short read_value;
    uint16_t rf_f5_13_7, rf_f5_6_0 = 0;

    DRIVERS_TRACE(1,"%s", __func__);
    //open RX ON for calabration
    BTDIGITAL_REG(0xd0220c00) = 0x000A002A;
    //delay to preventing BT/MCU RF SPI conflict
    btdrv_delay(10);

    //init
    btdrv_write_rf_reg(0x29c, 0xffb3);
    btdrv_write_rf_reg(0x29d, 0x001f);

    //cal flow
    btdrv_write_rf_reg(0xc2, 0x0);
    //bit9 : reg_rstn_log_cal
    btdrv_read_rf_reg(0xbe, &read_value);
    read_value |= (1 << 9);
    btdrv_write_rf_reg(0xbe, read_value);
    //reg_bt_log_cal_en
    btdrv_read_rf_reg(0x3af, &read_value);
    read_value |= (1 << 0);
    btdrv_write_rf_reg(0x3af, read_value);

    //log_cal_value[14:0]
    btdrv_read_rf_reg(0xf5, &read_value);
    DRIVERS_TRACE(0,"0xf5=0x%x",read_value);
    rf_f5_13_7 = (read_value & 0x3f80) >> 7;
    rf_f5_6_0  = (read_value & 0x7f);

    //0xc2 reg_fdata1[7:1] | 0xc2 reg_fdata2[15:9] | bit0:reg_fdata1_dr | bit8:reg_fdata2_dr
    btdrv_read_rf_reg(0xc2, &read_value);
    read_value = 0x0;
    read_value |= ((rf_f5_6_0 << 1) | (rf_f5_13_7 << 9) | (1<<0) | (1<<8));
    btdrv_write_rf_reg(0xc2, read_value);
    DRIVERS_TRACE(0,"0xc2=0x%x",read_value);

    //cal done
    btdrv_read_rf_reg(0xbe, &read_value);
    read_value &= ~(1 << 9);
    btdrv_write_rf_reg(0xbe, read_value);

    btdrv_read_rf_reg(0x3af, &read_value);
    read_value &= ~(1 << 0);
    btdrv_write_rf_reg(0x3af, read_value);

    //delay to preventing BT/MCU RF SPI conflict
    btdrv_delay(10);
    BTDIGITAL_REG(0xd0220c00) = 0x0;
}

void btdrv_tx_iq_manual_cal(void)
{
    uint16_t read_val;
    uint8_t EFUSE_F_BT_IMAGE, RF_C2_7_1, RF_C2_15_9 = 0;

    pmu_get_efuse(PMU_EFUSE_PAGE_RESERVED_15, &read_val);
    DRIVERS_TRACE(2, "%s efuse=0x%x",__func__, read_val);
    EFUSE_F_BT_IMAGE = read_val & 0xf;          //get[3:0]

    //Expansion bit5, 6
    EFUSE_F_BT_IMAGE |= ((getbit(read_val, 5) << 4) | (getbit(read_val, 6) << 5));
    DRIVERS_TRACE(1, "EFUSE_F_BT_IMAGE:0x%x",EFUSE_F_BT_IMAGE);

    BTRF_REG_GET_FIELD(0xC2, 0x7f, 1, RF_C2_7_1);
    BTRF_REG_GET_FIELD(0xC2, 0x7f, 9, RF_C2_15_9);
    DRIVERS_TRACE(1, "original RF_C2_7_1=0x%x , RF_C2_15_9=0x%x",RF_C2_7_1,RF_C2_15_9);

    if (getbit(read_val,4)) {
        if (EFUSE_F_BT_IMAGE >= RF_C2_7_1) {
            DRIVERS_TRACE(1, "%s To prevent data overflow, set fdata to 0.",__func__);
            RF_C2_7_1 = RF_C2_15_9 = 0;
        } else {
            RF_C2_7_1  -= EFUSE_F_BT_IMAGE;
            RF_C2_15_9 -= EFUSE_F_BT_IMAGE;
        }
    } else {
        RF_C2_7_1  += EFUSE_F_BT_IMAGE;
        RF_C2_15_9 += EFUSE_F_BT_IMAGE;
        if (RF_C2_7_1 > 127) {
            RF_C2_7_1 = 127;
            DRIVERS_TRACE(1, "%s To prevent data overflow, set fdata to 127.",__func__);
        }
        if (RF_C2_15_9 > 127) {
            RF_C2_15_9 = 127;
            DRIVERS_TRACE(1, "%s To prevent data overflow, set fdata to 12/.",__func__);
        }
    }

    BTRF_REG_SET_FIELD(0xC2, 0x7f, 1, RF_C2_7_1);
    BTRF_REG_SET_FIELD(0xC2, 0x7f, 9, RF_C2_15_9);

    //BTRF_REG_DUMP(0xC2);
}

void btdrv_rf_rx_gain_adjust_req(uint32_t user, bool lowgain)
{
    return;
}

//rf Image calib
void btdtv_rf_image_calib(void)
{
    uint16_t read_val = 0;
    //read calibrated val from efuse 0x05 register
    pmu_get_efuse(PMU_EFUSE_PAGE_SW_CFG, &read_val);
    //check if bit 11 has been set
    uint8_t calb_done_flag = ((read_val &0x800)>>11);
    if(calb_done_flag)
    {
        DRIVERS_TRACE(1,"EFUSE REG[5]=%x",read_val);
    }
    else
    {
        DRIVERS_TRACE(0,"EFUSE REG[5] rf image has not been calibrated!");
        return;
    }
    //[bit 12] calib flag
    uint8_t calib_val = ((read_val &0x1000)>>12);
    btdrv_read_rf_reg(0x9b,&read_val);
    read_val&=0xfcff;

    if(calib_val==0)
    {
        read_val|= 1<<8;
    }
    else if(calib_val== 1)
    {
        read_val|= 1<<9;
    }

    DRIVERS_TRACE(1,"write rf image calib val=%x in REG[0x9b]", read_val);
    btdrv_write_rf_reg(0x9b,read_val);
}

#ifdef RX_IQ_CAL

int bt_iqimb_test_ex (int mismatch_type);
extern int iq_gain;
extern int iq_phy;

void dataSort(int *arry,int len)
{
    for (int i = 0; i < len - 1; i++){
        for (int j = 0; j < len - 1 - i; j++){
            if (arry[j] > arry[j + 1]){
                int temp = arry[j + 1];
                arry[j + 1] = arry[j];
                arry[j] = temp;
            }
        }
    }
}

const uint16_t rx_cal_rfreg_set[][2] =
{
    {0xE8, 0x36B1},
    {0xE9, 0x1175},
    {0x3AD, 0x0038},
    {0xBF, 0x0D00},
    {0xC0,0x1E17},
    {0x1E4, 0x0025},
};

const uint16_t rx_cal_rfreg_store[][1] =
{
    {0xE8},
    {0xE9},
    {0x3AD},
    {0xBF},
    {0xC0},
    {0x1E4},
};

void btdrv_rx_iq_cal(void)
{
    uint16_t val;
    int i;
    uint16_t value = 0;
    uint32_t tx_cal_digreg_store[6];

    const uint16_t (*rx_cal_rfreg_set_p)[2];
    const uint16_t (*rx_cal_rfreg_store_p)[1];
    uint32_t rx_reg_set_tbl_size = 0;

    tx_cal_digreg_store[0] = BTDIGITAL_REG(0xD0350214);
    tx_cal_digreg_store[1] = BTDIGITAL_REG(0xd0350218);
    tx_cal_digreg_store[2] = BTDIGITAL_REG(0xd0350240);
    tx_cal_digreg_store[3] = BTDIGITAL_REG(0xD0350300);
    tx_cal_digreg_store[4] = BTDIGITAL_REG(0xD035031c);
    tx_cal_digreg_store[5] = BTDIGITAL_REG(0xD0350320);

    rx_cal_rfreg_store_p = &rx_cal_rfreg_store[0];
    uint32_t rx_reg_store_tbl_size = ARRAY_SIZE(rx_cal_rfreg_store);
    uint16_t rx_rf_local[rx_reg_store_tbl_size];
    DRIVERS_TRACE(0,"rx reg_store:\n");
    for(i=0; i< rx_reg_store_tbl_size; i++)
    {
        btdrv_read_rf_reg(rx_cal_rfreg_store_p[i][0],&value);
        rx_rf_local[i] = value;
        DRIVERS_TRACE(2,"rx reg=%x,v=%x",rx_cal_rfreg_store_p[i][0],value);
    }


    DRIVERS_TRACE(1,"0xd0350214:%x\n",tx_cal_digreg_store[0]);
    DRIVERS_TRACE(1,"0xD0350218:%x\n",tx_cal_digreg_store[1]);
    DRIVERS_TRACE(1,"0xD0350240:%x\n",tx_cal_digreg_store[2]);
    DRIVERS_TRACE(1,"0xD0350300:%x\n",tx_cal_digreg_store[3]);
    DRIVERS_TRACE(1,"0xD035031c:%x\n",tx_cal_digreg_store[4]);
    DRIVERS_TRACE(1,"0xD0350320:%x\n",tx_cal_digreg_store[5]);

    BTDIGITAL_REG_WR(0xd0350218, 0x00000583);
    BTDIGITAL_REG_WR(0xD0350214, 0x0);
    BTDIGITAL_REG_WR(0xD0350320, 0x0001000F);
    BTDIGITAL_REG_SET_FIELD(0xD0350300, 7, 0, 3);
    BTDIGITAL_REG_SET_FIELD(0xD035031c, 0xf, 0, 1);
#ifdef BT_IF_750K
    BTDIGITAL_REG_SET_FIELD(0xD0350320, 0x7f, 0, 0xc);
#elif defined(BT_IF_1P05M)
    BTDIGITAL_REG_SET_FIELD(0xD0350320, 0x7f, 0, 0xf);
#endif
    BTDIGITAL_REG_SET_FIELD(0xd0350240, 0x1, 12, 1);

#if 1
    uint16_t val_b8,val_c0, val_389,val_8c;
    btdrv_read_rf_reg(0xb8,&val_b8);
    btdrv_read_rf_reg(0xc0,&val_c0);
    btdrv_read_rf_reg(0x389,&val_389);
    btdrv_read_rf_reg(0x8c,&val_8c);
    //int32_t gain_v[9],phi_v[9];
    int32_t gain_v,phi_v;
    const int iq_count = 1;


    val = (val_8c & (~(0x3c0)))|(0x7<<6);

    btdrv_write_rf_reg(0x8c, val);

    for(i=0;i<iq_count;i++)
    {
        btdrv_write_rf_reg(0xb8, (1|(5<<1)));
        btdrv_read_rf_reg(0xb8,&val);
        DRIVERS_TRACE(1,"0xb8:%x\n",val);

        rx_cal_rfreg_set_p = &rx_cal_rfreg_set[0];
        rx_reg_set_tbl_size = ARRAY_SIZE(rx_cal_rfreg_set);
        DRIVERS_TRACE(0,"rx reg_set:\n");
        for(i=0; i< rx_reg_set_tbl_size; i++)
        {
            btdrv_write_rf_reg(rx_cal_rfreg_set_p[i][0],rx_cal_rfreg_set_p[i][1]);
            BTRF_REG_DUMP(rx_cal_rfreg_set_p[i][0]);
        }

        //bit4:reg_bt_txflt_gain_dr | reg_bt_txflt_gain[8:5]
        btdrv_read_rf_reg(0xc0,&val);
        val &= 0xFE0F;      //clear [8:4]
        val |= (0x1 << 4);
        btdrv_write_rf_reg(0xc0, val);
        btdrv_read_rf_reg(0xc0,&val);
        DRIVERS_TRACE(1,"0xc0:%x\n",val);

        //reg_bt_padrv_gain[7:6] | bit8:reg_bt_padrv_gain_dr
        btdrv_read_rf_reg(0x389,&val);
        val &= 0xFE3F;      //clear [8:6]
        val |= (0x4 << 6);
        btdrv_write_rf_reg(0x389, val);
        btdrv_read_rf_reg(0x389,&val);
        DRIVERS_TRACE(1,"0x389:%x\n",val);

        BTDIGITAL_REG_WR(0xd0220c00, 0x0);
        btdrv_delay(1);
        BTDIGITAL_REG_WR(0xd0220c00, 0x800A0000);
        btdrv_delay(1);
        bt_iqimb_test_ex(1);
        gain_v=iq_gain;
        phi_v=iq_phy;
    }
    //dataSort(gain_v,iq_count);
    //dataSort(phi_v,iq_count);
    iq_gain =gain_v;
    iq_phy = phi_v;
    //iq_gain = gain_v/iq_count;
    //iq_phy = phi_v/iq_count;
    DRIVERS_TRACE(2,"val gain:%x phi:%x\n",iq_gain,iq_phy);

    DRIVERS_TRACE(0,"reg_reset:\n");
    btdrv_write_rf_reg(0xb8, val_b8);
    btdrv_write_rf_reg(0x8c, val_8c);
    btdrv_write_rf_reg(0xc0, val_c0);
    btdrv_write_rf_reg(0x389,val_389);
#else
    //... ...
#endif
    BTDIGITAL_REG_WR(0xd0220c00, 0x0);
    BTDIGITAL_REG_SET_FIELD(0xD035025c, 0x3, 2, 0);
    DRIVERS_TRACE(0,"25c:%x\n",BTDIGITAL_REG(0xd035025c));

    DRIVERS_TRACE(0,"reg_reset:\n");

    for(i=0; i< rx_reg_store_tbl_size; i++)
    {
        btdrv_write_rf_reg(rx_cal_rfreg_store_p[i][0],rx_rf_local[i]);
        BTRF_REG_DUMP(rx_cal_rfreg_store_p[i][0]);
    }

    BTDIGITAL_REG_WR(0xD0350214, tx_cal_digreg_store[0]);
    BTDIGITAL_REG_WR(0xd0350218, tx_cal_digreg_store[1]);
    BTDIGITAL_REG_WR(0xd0350240, tx_cal_digreg_store[2]);
    BTDIGITAL_REG_WR(0xD0350300, tx_cal_digreg_store[3]);
    BTDIGITAL_REG_WR(0xD035031c, tx_cal_digreg_store[4]);
    BTDIGITAL_REG_WR(0xD0350320, tx_cal_digreg_store[5]);

    DRIVERS_TRACE(1,"0xD0350214:%x\n",BTDIGITAL_REG(0xD0350214));
    DRIVERS_TRACE(1,"0xd0350218:%x\n",BTDIGITAL_REG(0xd0350218));
    DRIVERS_TRACE(1,"0xD0350240:%x\n",BTDIGITAL_REG(0xD0350240));
    DRIVERS_TRACE(1,"0xD0350300:%x\n",BTDIGITAL_REG(0xD0350300));
    DRIVERS_TRACE(1,"0xD035031c:%x\n",BTDIGITAL_REG(0xD035031c));
    DRIVERS_TRACE(1,"0xD0350320:%x\n",BTDIGITAL_REG(0xD0350320));
    //DRIVERS_TRACE(1,"0xd0350240:%x\n",BTDIGITAL_REG(0xd0350240));
}
#endif

void btdrv_rf_init_ext(void)
{
#ifdef __RF_INIT_XTAL_CAP_FROM_NV__
    unsigned int temp_xtal_fcap;
    if (!nvrec_dev_get_xtal_fcap(&temp_xtal_fcap))
    {
        btdrv_rf_init_xtal_fcap(temp_xtal_fcap);
        btdrv_delay(1);
        DRIVERS_TRACE(2,"%s 0x95=0x%x", __func__, temp_xtal_fcap);
    }
    else
#endif
    {
        btdrv_rf_init_xtal_fcap(DEFAULT_XTAL_FCAP);
        DRIVERS_TRACE(1,"%s failed", __func__);
    }
}

void bt_drv_rf_init_xtal_fcap(void)
{
    unsigned int xtal_fcap_temp = DEFAULT_XTAL_FCAP;
    uint16_t xtal_val = 0;

    if(btdrv_rf_customer_config.config_xtal_en == true)
    {
        xtal_fcap_temp = btdrv_rf_customer_config.xtal_cap_val;
        DRIVERS_TRACE(1,"btdrv customer set xtal fcap=0x%x", xtal_fcap_temp);
    }
    else
    {
#ifdef __RF_INIT_XTAL_CAP_FROM_NV__
        if (!nvrec_dev_get_xtal_fcap(&xtal_fcap_temp))
        {
            DRIVERS_TRACE(2,"%s xtal cap=0x%x", __func__, xtal_fcap_temp);
        }
        else
#endif
        {
            xtal_fcap_temp = DEFAULT_XTAL_FCAP;
            DRIVERS_TRACE(1,"%s failed", __func__);
        }
    }

    for (uint32_t i = 0; i<9; i++){
        if ((xtal_fcap_temp) >> (i) & 1) {
            xtal_val |= 1 << i;
            btdrv_rf_init_xtal_fcap(xtal_val);
        }
        btdrv_delay(1);
    }
}

uint8_t rf_reg_txpwr_val_get(int8_t txpwr_dbm)
{
    uint8_t txpwr_val = 0;
    switch(txpwr_dbm)
    {
        case 0:
            txpwr_val = TX_PWR_0dbm_VAL;
            break;
        case 1:
            txpwr_val = TX_PWR_1dbm_VAL;
            break;
        case 2:
            txpwr_val = TX_PWR_2dbm_VAL;
            break;
        case 3:
            txpwr_val = TX_PWR_3dbm_VAL;
            break;
        case 4:
            txpwr_val = TX_PWR_4dbm_VAL;
            break;
        case 5:
            txpwr_val = TX_PWR_5dbm_VAL;
            break;
        case 6:
            txpwr_val = TX_PWR_6dbm_VAL;
            break;
        case 7:
            txpwr_val = TX_PWR_7dbm_VAL;
            break;
        case 8:
            txpwr_val = TX_PWR_8dbm_VAL;
            break;
        case 9:
            txpwr_val = TX_PWR_9dbm_VAL;
            break;
        case 10:
            txpwr_val = TX_PWR_10dbm_VAL;
            break;
        case 11:
            txpwr_val = TX_PWR_11dbm_VAL;
            break;
        case 12:
            txpwr_val = TX_PWR_12dbm_VAL;
            break;
        case 13:
            txpwr_val = TX_PWR_13dbm_VAL;
            break;
        case 14:
            txpwr_val = TX_PWR_14dbm_VAL;
            break;
        case 15:
            txpwr_val = TX_PWR_15dbm_VAL;
            break;
        case 16:
            txpwr_val = TX_PWR_16dbm_VAL;
            break;
        case -1:
            txpwr_val = TX_PWR_N1dbm_VAL;
            break;
        case -2:
            txpwr_val = TX_PWR_N2dbm_VAL;
            break;
        case -3:
            txpwr_val = TX_PWR_N3dbm_VAL;
            break;
        default:
            if(txpwr_dbm < -3)
            {
                txpwr_val = TX_PWR_N3dbm_VAL;
            }
            else
            {
                txpwr_val = TX_PWR_16dbm_VAL;
            }
            break;
    }

    return txpwr_val;
}

static uint8_t rf_txpwr_reg_get(uint8_t idx)
{
    uint8_t rf_reg = 0;
    switch(idx)
    {
        case TX_PWR_IDX_0:
            rf_reg = 0xdd;
            break;
        case TX_PWR_IDX_1:
            rf_reg = 0xde;
            break;
        case TX_PWR_IDX_2:
            rf_reg = 0xdf;
            break;
        case TX_PWR_IDX_3:
            rf_reg = 0xe0;
            break;
        case TX_PWR_IDX_4:
            rf_reg = 0xe1;
            break;
        case TX_PWR_IDX_MAX:
            rf_reg = 0xe2;
            break;
        case TX_PWR_IDX_PAGE:
            rf_reg = 0xe3;
            break;
        default:
            ASSERT(0, "BT_DRV:ERROR tx pwr idx=%d", idx);
            break;
    }

    return rf_reg;
}

static void bt_drv_rf_txpwr_set(uint8_t idx, uint8_t bt_val, uint8_t le_val)
{
    uint8_t rf_reg = rf_txpwr_reg_get(idx);

    TX_PWR_REG_T reg_txpwr;

    reg_txpwr.u.reg_bit.bt_pwr = rf_reg_txpwr_val_get(bt_val);
    reg_txpwr.u.reg_bit.ble_pwr = rf_reg_txpwr_val_get(le_val);

    btdrv_write_rf_reg(rf_reg, reg_txpwr.u.reg_val);

    DRIVERS_TRACE(0, "BT DRV: set TX pwr reg=0x%x,EDR=0x%02x,GFSK=0x%02x,val=0x%04x",rf_reg,
        reg_txpwr.u.reg_bit.bt_pwr, reg_txpwr.u.reg_bit.ble_pwr,reg_txpwr.u.reg_val);
}

void bt_drv_tx_pwr_init(void)
{
    struct btdrv_customer_rf_config_t cfg = btdrv_rf_customer_config;

    if(cfg.config_tx_pwr_en == true)
    {
        bt_drv_rf_txpwr_set(TX_PWR_IDX_0, cfg.bt_tx_idx0_pwr, cfg.le_tx_idx0_pwr);
        bt_drv_rf_txpwr_set(TX_PWR_IDX_1, cfg.bt_tx_idx1_pwr, cfg.le_tx_idx1_pwr);
        bt_drv_rf_txpwr_set(TX_PWR_IDX_2, cfg.bt_tx_idx2_pwr, cfg.le_tx_idx2_pwr);
        bt_drv_rf_txpwr_set(TX_PWR_IDX_3, cfg.bt_tx_idx3_pwr, cfg.le_tx_idx3_pwr);
        bt_drv_rf_txpwr_set(TX_PWR_IDX_4, cfg.bt_tx_idx4_pwr, cfg.le_tx_idx4_pwr);
        bt_drv_rf_txpwr_set(TX_PWR_IDX_MAX, cfg.bt_tx_max_pwr, cfg.le_tx_max_pwr);
        bt_drv_rf_txpwr_set(TX_PWR_IDX_PAGE, cfg.bt_tx_page_pwr, cfg.le_tx_idx3_pwr);

        DRIVERS_TRACE(1,"btdrv customer set TX power");
    }
    else
    {
        ASSERT(0, "BT_DRV:pls check TX pwr config");
    }
}

void bt_drv_ble_rf_reg_txpwr_set(uint8_t rf_reg, uint8_t val)
{
    TX_PWR_REG_T reg_txpwr;

    btdrv_read_rf_reg(rf_reg, &reg_txpwr.u.reg_val);

    reg_txpwr.u.reg_bit.ble_pwr = val;

    btdrv_write_rf_reg(rf_reg, reg_txpwr.u.reg_val);
}

void bt_drv_ble_adv_txpwr_via_advhdl(uint8_t adv_hdl, uint8_t idx, int8_t txpwr_dbm)
{
    uint8_t rf_reg;
    uint8_t txpwr_val;

    DRIVERS_TRACE(2,"set adv_txpwr_via_advhdl hdl 0x%02x idx %d dbm %d", adv_hdl, idx, txpwr_dbm);

    if(idx > 5)
    {
        return;
    }

    if(txpwr_dbm <-3)
    {
        txpwr_dbm += 18;
        bt_drv_reg_op_low_txpwr(true, FACTOR_ATTENUATION_18DBM, BLE_ADV_LOW_TXPWR_MODE, adv_hdl);
    }
    else
    {
        bt_drv_reg_op_low_txpwr(false, FACTOR_ATTENUATION_18DBM, BLE_ADV_LOW_TXPWR_MODE, adv_hdl);
    }

    rf_reg = rf_txpwr_reg_get(idx);
    txpwr_val = rf_reg_txpwr_val_get(txpwr_dbm);

    bt_drv_reg_op_le_rf_reg_txpwr_set(rf_reg, txpwr_val);
}

extern int pmu_ext_get_efuse(enum PMU_EXT_EFUSE_PAGE_T page, unsigned short *efuse);

static void btdrv_2500p_cap_delay_cal_tx_calib(uint16_t calib_condition, int *tx_calib_flag)
{
    uint16_t flag = 0;
    uint16_t reg_val_299;
    uint16_t reg_val_29A;
    uint16_t low_volt = 0;


    BTRF_REG_SET_FIELD(0x298, 0x1, 14, 0x1);
    BTRF_REG_SET_FIELD(0x2C8, 0x1, 13, 0x1);
    BTRF_REG_GET_FIELD(0x299, 0x7, 0, reg_val_299);
    BTRF_REG_SET_FIELD(0x299, 0x7, 0, calib_condition);
    // 2402M
    BTDIGITAL_REG(0xd0220c00) = 0x000A0000;
    hal_sys_timer_delay(US_TO_TICKS(100));

    for (uint32_t i = 0; i < BTRF_DELAY_CAL_TIME; i++) {
        BTRF_REG_SET_FIELD(0x295, 0xF, 6, i);
        hal_sys_timer_delay(US_TO_TICKS(60));
        BTRF_REG_GET_FIELD(0xEB, 0x1, 0, flag);
        tx_calib_flag[i] &= flag;
    }
    DRIVERS_TRACE(0,"freq: 2402, condition: %d  tx calib flag:", calib_condition);
    DRIVERS_DUMP32("%d ", tx_calib_flag, 16);

    // 2480M
    BTDIGITAL_REG(0xd0220c00) = 0x000A004E;
    hal_sys_timer_delay(US_TO_TICKS(100));
    for (uint32_t i = 0; i < BTRF_DELAY_CAL_TIME; i++) {
        BTRF_REG_SET_FIELD(0x295, 0xF, 6, i);
        hal_sys_timer_delay(US_TO_TICKS(60));
        BTRF_REG_GET_FIELD(0xEB, 0x1, 0, flag);
        tx_calib_flag[i] &= flag;
    }
    DRIVERS_TRACE(0,"freq: 2480, condition: %d  tx calib flag:", calib_condition);
    DRIVERS_DUMP32("%d ", tx_calib_flag, 16);

    // 2440M
    BTDIGITAL_REG(0xd0220c00) = 0x000A0026;
    hal_sys_timer_delay(US_TO_TICKS(100));
    BTRF_REG_GET_FIELD(0x29A, 0xF, 12, reg_val_29A);
    BTRF_REG_SET_FIELD(0x29A, 0xF, 12, 0x3);
    for (uint32_t i = 0; i < BTRF_DELAY_CAL_TIME; i++) {
        BTRF_REG_SET_FIELD(0x295, 0xF, 6, i);
        hal_sys_timer_delay(US_TO_TICKS(60));
        BTRF_REG_GET_FIELD(0xEB, 0x1, 0, flag);
        tx_calib_flag[i] &= flag;
    }
    DRIVERS_TRACE(0,"freq: 2440 high volt, condition: %d  tx calib flag:", calib_condition);
    DRIVERS_DUMP32("%d ", tx_calib_flag, 16);

    // 2440M
    BTDIGITAL_REG(0xd0220c00) = 0x000A0026;
    hal_sys_timer_delay(US_TO_TICKS(100));
    BTRF_REG_SET_FIELD(0x29A, 0xF, 12, 0x2);
    for (uint32_t i = 0; i < BTRF_DELAY_CAL_TIME; i++) {
        BTRF_REG_SET_FIELD(0x295, 0xF, 6, i);
        hal_sys_timer_delay(US_TO_TICKS(60));
        BTRF_REG_GET_FIELD(0xEB, 0x1, 0, flag);
        if (tx_calib_flag[(i + 8) % 16] == 0){
            low_volt ++;
        }
    }
    if (low_volt < 16){
        for (uint32_t i = 0; i < BTRF_DELAY_CAL_TIME; i++) {
            BTRF_REG_SET_FIELD(0x295, 0xF, 6, i);
            hal_sys_timer_delay(US_TO_TICKS(60));
            BTRF_REG_GET_FIELD(0xEB, 0x1, 0, flag);
            tx_calib_flag[(i + 8) % 16] &= flag;
        }

    }
    DRIVERS_TRACE(0,"freq: 2440 high volt, condition: %d  tx calib flag:", calib_condition);
    DRIVERS_DUMP32("%d ", tx_calib_flag, 16);

    DRIVERS_TRACE(0,"final tx flag, condition: %d  tx calib flag:", calib_condition);
    DRIVERS_DUMP32("%d ", tx_calib_flag, 16);

    BTRF_REG_SET_FIELD(0x29A, 0xF, 12, reg_val_29A);
    BTRF_REG_SET_FIELD(0x299, 0xF, 12, reg_val_299);
}

static void btdrv_2500p_cap_delay_cal_rx_calib(uint16_t calib_condition, int *rx_calib_flag)
{
    uint16_t flag = 0;
    uint16_t reg_val_2C9;

    BTRF_REG_SET_FIELD(0x298, 0x1, 14, 0x1);
    BTRF_REG_SET_FIELD(0x2C8, 0x1, 13, 0x1);

    BTRF_REG_GET_FIELD(0x2C9, 0x7, 5, reg_val_2C9);
    BTRF_REG_SET_FIELD(0x2C9, 0x7, 5, (calib_condition + 1));
    // 2530M
    BTDIGITAL_REG(0xd0220c00) = 0x000A0080;
    hal_sys_timer_delay(US_TO_TICKS(100));
    for (uint32_t i = 0; i < BTRF_DELAY_CAL_TIME; i++) {
        BTRF_REG_SET_FIELD(0x295, 0xF, 6, i);
        hal_sys_timer_delay(US_TO_TICKS(60));
        BTRF_REG_GET_FIELD(0xEC, 0x1, 0, flag);
        rx_calib_flag[i] &= flag;
    }
    DRIVERS_TRACE(0,"freq: 2530, condition: %d  rx calib flag:", calib_condition);
    DRIVERS_DUMP32("%d ", rx_calib_flag, 16);

    // 2608M
    BTDIGITAL_REG(0xd0220c00) = 0x000A00CE;
    hal_sys_timer_delay(US_TO_TICKS(100));
    for (uint32_t i = 0; i < BTRF_DELAY_CAL_TIME; i++) {
        BTRF_REG_SET_FIELD(0x295, 0xF, 6, i);
        hal_sys_timer_delay(US_TO_TICKS(60));
        BTRF_REG_GET_FIELD(0xEC, 0x1, 0, flag);
        rx_calib_flag[i] &= flag;
    }
    DRIVERS_TRACE(0,"freq: 2608, condition: %d  rx calib flag:", calib_condition);
    DRIVERS_DUMP32("%d ", rx_calib_flag, 16);

    DRIVERS_TRACE(0,"final rx flag, condition: %d  rx calib flag:", calib_condition);
    DRIVERS_DUMP32("%d ", rx_calib_flag, 16);

    BTRF_REG_SET_FIELD(0x2C9, 0x7, 5, reg_val_2C9);
}

static void btdrv_2500p_cap_delay_cal_get_best_val(int *calib_flag, uint16_t *zero_number, uint8_t *best_calib_value)
{
    uint16_t max_len = 0;

    for (int j = 0; j < BTRF_DELAY_CAL_TIME; j++){
        if (calib_flag[j] != 0){
            for (int k = 0; k < BTRF_DELAY_CAL_TIME; k++){
                if ( (calib_flag[(j + k)%16] == 0) || (k == 15)){
                    if (k > max_len){
                        max_len = k;
                        *best_calib_value = ((k / 2) + j) % 16;
                    }
                    break;
                }
            }
        }
    }

    *zero_number = 0;
    for (int i = 0; i < BTRF_DELAY_CAL_TIME; i++){
        if(calib_flag[i] == 0){
            *zero_number += 1;
        }
    }
    if ( *zero_number > 8){
        *best_calib_value = 0x8;    //default value
        DRIVERS_TRACE(0," too many flag = 0, dont use this value, default value :%d", *best_calib_value);
    }
}

static void btdrv_2500p_cap_delay_cal_soft_proc_tx(uint8_t *tx_best_val)
{
    uint16_t calib_condition = 0;
    uint16_t zero_number = 0;

    for (calib_condition = 1; calib_condition < 8; calib_condition++){
        int tx_calib_flag[BTRF_DELAY_CAL_TIME] =
                {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
        btdrv_2500p_cap_delay_cal_tx_calib(calib_condition, tx_calib_flag);

        btdrv_2500p_cap_delay_cal_get_best_val(tx_calib_flag, &zero_number, tx_best_val);

        if ( zero_number < 8){
            DRIVERS_TRACE(0,"final  best calib tx val:  %d", *tx_best_val);
            break;
        }
    }
}

static void btdrv_2500p_cap_delay_cal_soft_proc_rx(uint8_t *rx_best_val)
{
    uint16_t calib_condition = 0;
    uint16_t zero_number = 0;

    for (calib_condition = 1; calib_condition < 8; calib_condition++){
        int rx_calib_flag[BTRF_DELAY_CAL_TIME] =
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
        btdrv_2500p_cap_delay_cal_rx_calib(calib_condition, rx_calib_flag);

        btdrv_2500p_cap_delay_cal_get_best_val(rx_calib_flag, &zero_number, rx_best_val);

        if ( zero_number < 8){
            DRIVERS_TRACE(0,"final  best calib rx val:  %d", *rx_best_val);
            break;
        }
    }
}

void btdrv_2500p_cap_delay_cal(void)
{
    uint16_t value;
    uint8_t tx_capdly, rx_capdly = 0;
    enum PMU_CHIP_TYPE_T  pmu_type;

    pmu_type = pmu_get_chip_type();

    if ((pmu_type == PMU_CHIP_TYPE_1802) || (pmu_type == PMU_CHIP_TYPE_1807)) {
        pmu_ext_get_efuse(PMU_EXT_EFUSE_PAGE_RESERVED_9, &value);
        value = (value & 0xFF00) >> 8;
    } else if (pmu_type == PMU_CHIP_TYPE_1805) {
        pmu_get_efuse(PMU_EFUSE_PAGE_DCCALIB2_L_LP, &value);
    } else {
        pmu_ext_get_efuse(PMU_EXT_EFUSE_PAGE_RESERVED_10, &value);
    }
    DRIVERS_TRACE(3, "%s pmu_type=%d efuse_val=0x%04x", __func__, pmu_type, value);

    tx_capdly = value & 0xF;
    rx_capdly = ((value & 0xF0) >> 4);

    if ((0 == tx_capdly) || (0 == rx_capdly)) {
        DRIVERS_TRACE(0, "invalid efuse ,so use soft proc");
        btdrv_2500p_cap_delay_cal_soft_proc_tx(&tx_capdly);
        btdrv_2500p_cap_delay_cal_soft_proc_rx(&rx_capdly);
    }

    BTRF_REG_SET_FIELD(0x295, 0xF, 6, tx_capdly);
    BTRF_REG_SET_FIELD(0x2C5, 0xF, 6, rx_capdly);
}

uint8_t btdrv_rf_init(void)
{
    uint16_t value = 0;
    const uint16_t (*rf_init_tbl_p)[3] = NULL;
    uint32_t tbl_size = 0;
    //uint8_t ret;
    uint8_t i;

    for (i = 0; i < ARRAY_SIZE(rf_sys_init_tbl); i++)
    {
        btdrv_read_rf_reg(rf_sys_init_tbl[i].reg, &value);
        value = (value & ~rf_sys_init_tbl[i].mask) | (rf_sys_init_tbl[i].set & rf_sys_init_tbl[i].mask);
        if (rf_sys_init_tbl[i].delay)
        {
            btdrv_delay(rf_sys_init_tbl[i].delay);
        }
        btdrv_write_rf_reg(rf_sys_init_tbl[i].reg, value);
    }

    rf_init_tbl_p = &rf_init_tbl_1[0];
    tbl_size = ARRAY_SIZE(rf_init_tbl_1);

    for(i=0; i< tbl_size; i++)
    {
        btdrv_write_rf_reg(rf_init_tbl_p[i][0],rf_init_tbl_p[i][1]);
        if(rf_init_tbl_p[i][2] !=0)
            btdrv_delay(rf_init_tbl_p[i][2]);//delay
        btdrv_read_rf_reg(rf_init_tbl_p[i][0],&value);
        DRIVERS_TRACE(2,"reg=%x,v=%x",rf_init_tbl_p[i][0],value);
    }

    if (hal_get_chip_metal_id() >= HAL_CHIP_METAL_ID_4) {
        rf_init_tbl_p = &rf_init_tbl_2[0];
        tbl_size = ARRAY_SIZE(rf_init_tbl_2);

        for(i=0; i< tbl_size; i++) {
            btdrv_write_rf_reg(rf_init_tbl_p[i][0],rf_init_tbl_p[i][1]);
            if(rf_init_tbl_p[i][2] !=0)
                btdrv_delay(rf_init_tbl_p[i][2]);
            btdrv_read_rf_reg(rf_init_tbl_p[i][0],&value);
            DRIVERS_TRACE(2,"reg=%x,v=%x",rf_init_tbl_p[i][0],value);
        }
    }
    //customer option
    bt_drv_rf_set_customer_config(NULL);

    bt_drv_tx_pwr_init();

#ifdef __HW_AGC__
    bt_drv_rf_set_bt_hw_agc_enable(1);
    bt_drv_rf_set_ble_hw_agc_enable(1);
    for(i=0; i< ARRAY_SIZE(rf_init_tbl_1_hw_agc); i++)
    {
        btdrv_write_rf_reg(rf_init_tbl_1_hw_agc[i][0],rf_init_tbl_1_hw_agc[i][1]);
        if(rf_init_tbl_1_hw_agc[i][2] !=0)
            btdrv_delay(rf_init_tbl_1_hw_agc[i][2]);//delay
    }
#else
#ifndef BT_I2V_VAL_RD
    bt_drv_rf_set_bt_hw_agc_enable(0);
    bt_drv_rf_set_ble_hw_agc_enable(0);
#endif
#endif

#ifdef __NEW_SWAGC_MODE__
    bt_drv_rf_set_bt_sync_agc_enable(true);
#else
    bt_drv_rf_set_bt_sync_agc_enable(false);
#endif

#ifdef __BLE_NEW_SWAGC_MODE__
    bt_drv_rf_set_ble_sync_agc_enable(true);
#else
    bt_drv_rf_set_ble_sync_agc_enable(false);
#endif

#ifdef __AFH_ASSESS__
    bt_drv_rf_set_afh_monitor_gain();
#endif

    btdrv_2500p_cap_delay_cal();
    //need before rf log delay cal
//    btdtv_rf_image_calib();
    btdrv_rf_log_delay_cal_init();

    btdrv_tx_iq_manual_cal();

    bt_drv_rf_init_xtal_fcap();

    return 1;
}

void bt_drv_rf_set_bt_sync_agc_enable(bool enable)
{
    uint16_t val_e2 = 0;
    btdrv_read_rf_reg(0xb6,&val_e2);
    if(enable)
    {
        //open rf new sync agc mode
        val_e2 |= (1<<3);
    }
    else
    {
        //close rf new sync agc mode
        val_e2 &= ~(1<<3);
    }
    btdrv_write_rf_reg(0xb6,val_e2);
}

void bt_drv_rf_set_ble_sync_agc_enable(bool enable)
{
    uint16_t val_e2 = 0;
    btdrv_read_rf_reg(0xb6,&val_e2);
    if(enable)
    {
        //open rf new sync agc mode
        val_e2 |= (1<<2);
    }
    else
    {
        //close rf new sync agc mode
        val_e2 &= ~(1<<2);
    }
    btdrv_write_rf_reg(0xb6,val_e2);
}

//switch old swagc and new sync swagc
void bt_drv_select_agc_mode(enum BT_WORK_MODE_T mode)
{
#ifdef __A2DP_ESCO_DYNAMIC_AGC_TABLE__
    bt_drv_reg_op_agc_table_mode(mode);
#endif
#ifdef __NEW_SWAGC_MODE__
    static uint8_t agc_mode_bak = SWAGC_INIT_MODE;
    uint8_t agc_mode = OLD_SWAGC_MODE;

    switch(mode)
    {
        case BT_A2DP_WORK_MODE:
            agc_mode = NEW_SYNC_SWAGC_MODE;
            break;
        case BT_IDLE_MODE:
        case BT_HFP_WORK_MODE:
            agc_mode = OLD_SWAGC_MODE;
            break;
        default:
            DRIVERS_TRACE(1,"BT_DRV:set error mork mode=%d",mode);
            break;
    }

    if(agc_mode_bak != agc_mode)
    {
        agc_mode_bak = agc_mode;
        DRIVERS_TRACE(1,"BT_DRV:use SW AGC mode=%d[2:OLD,1:NEW sync]",agc_mode);
        bt_drv_reg_op_swagc_mode_set(agc_mode);
    }
#endif
}

void btdrv_txpower_calib(void)
{
    uint16_t read_val;

    pmu_get_efuse(PMU_EFUSE_PAGE_SW_CFG, &read_val);
    DRIVERS_TRACE(2, "%s PMU_EFUSE_PAGE_SW_CFG val=0x%x",__func__, read_val);

    //high tx pwr calib
    if(0x3 == (0x3 & read_val)) {
        BTRF_REG_SET_FIELD(0x2A0, 0x1,14,0);
    }

    //low tx pwr calib
    if(getbit(read_val, 2)) {
        //reg_bt_padrv_ictrl[14:10]
        BTRF_REG_SET_FIELD(0x2A0, 0x1F, 10, 0x1F);
        //reg_bt_pa_ictrl[13:9]
        BTRF_REG_SET_FIELD(0x2A2, 0x1F, 9, 0x1F);
    }
}

#define BT_SMALL_GAIN_TXPWR_10DBM    0x7
#define BT_SMALL_GAIN_TXPWR_9P5DBM   0x6
#define BT_SMALL_GAIN_TXPWR_8P7DBM   0x5

void btdrv_tx_large_power_cal(void)
{
    uint16_t read_val;

    pmu_get_efuse(PMU_EFUSE_PAGE_MODEL, &read_val);
    read_val &= 0x7;
    DRIVERS_TRACE(2, "%s PMU_EFUSE_PAGE_RESERVED_14 val=0x%x",__func__, read_val);

    switch (read_val) {
        case BT_SMALL_GAIN_TXPWR_8P7DBM:
            BTRF_REG_SET_FIELD(0x2A0, 0x1F,10,0x14);
            break;
        case BT_SMALL_GAIN_TXPWR_9P5DBM:
            BTRF_REG_SET_FIELD(0x2A0, 0x1F,10,0x8);
            break;
        case BT_SMALL_GAIN_TXPWR_10DBM:
            BTRF_REG_SET_FIELD(0x2A0, 0x1F,10,0x0);
            break;
        default:
            break;
    }
}

void bt_drv_rf_high_efficency_tx_pwr_ctrl(bool limit_tx_idx, bool limit_pa_en)
{
#ifdef HIGH_EFFICIENCY_TX_PWR_CTRL
    uint16_t read_2a2_value;
    //PA limit
    btdrv_read_rf_reg(0x2a2, &read_2a2_value);
    if(limit_pa_en)
    {
        read_2a2_value |= (1 << 8 |1 << 0);
    }
    else
    {
        read_2a2_value &= ~(1 << 8 |1 << 0);
    }
    btdrv_write_rf_reg(0x2a2, read_2a2_value);

    //tx power limit
    struct btdrv_customer_rf_config_t cfg = btdrv_rf_customer_config;
    if(limit_tx_idx)
    {
        bt_drv_rf_txpwr_set(TX_PWR_IDX_4, cfg.bt_tx_idx3_pwr, cfg.le_tx_idx3_pwr);
        bt_drv_rf_txpwr_set(TX_PWR_IDX_MAX, cfg.bt_tx_idx3_pwr, cfg.le_tx_idx3_pwr);
        btdrv_txpwr_conv_tbl[4] = cfg.le_tx_idx3_pwr;
        btdrv_txpwr_conv_tbl[5] = cfg.le_tx_idx3_pwr;
    }
    else
    {
        bt_drv_rf_txpwr_set(TX_PWR_IDX_4, cfg.bt_tx_idx4_pwr, cfg.le_tx_idx4_pwr);
        bt_drv_rf_txpwr_set(TX_PWR_IDX_MAX, cfg.bt_tx_max_pwr, cfg.le_tx_max_pwr);
        btdrv_txpwr_conv_tbl[4] = cfg.le_tx_idx4_pwr;
        btdrv_txpwr_conv_tbl[5] = cfg.le_tx_max_pwr;
    }
#endif
}

void bt_drv_rf_high_efficency_tx_pwr_ctrl_for_testmode()
{
#ifdef HIGH_EFFICIENCY_TX_PWR_CTRL
    uint16_t read_2a2_value;
    btdrv_read_rf_reg(0x2a2, &read_2a2_value);
    read_2a2_value &= ~(1 << 8 |1 << 0);
    btdrv_write_rf_reg(0x2a2, read_2a2_value);
#endif
}