/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
#include "bt_drv.h"
#include "cmsis.h"
#include "hal_trace.h"
#include "bt_drv_1501p_internal.h"
#include "hal_chipid.h"
#include "bt_drv_interface.h"
#include "bt_drv_reg_op.h"
#include "bt_drv_internal.h"
#include "pmu.h"
#ifdef __RF_INIT_XTAL_CAP_FROM_NV__
#include "nvrecord_dev.h"
#endif

struct TX_PWR_SELECT_ITEM
{
    float pwr_dbm;
    uint16_t pwr_val;
    uint32_t pa_sel;
};

#define TX_PWR_PA_2P5 1
#define TX_PWR_PA_1P8 0

static const struct TX_PWR_SELECT_ITEM tx_pwr_select_metal_id_0[] =
{
    {15, 0x6dbf, TX_PWR_PA_2P5},    //15dbm
    {14, 0x6dbe, TX_PWR_PA_2P5},    //14dbm
    {13, 0x6dbd, TX_PWR_PA_2P5},    //13dbm
    {12, 0x6dbc, TX_PWR_PA_2P5},    //12dbm
    {11, 0x4dad, TX_PWR_PA_2P5},    //11dbm
    {10, 0x4dac, TX_PWR_PA_2P5},    //10dbm
    {9,  0x4dab, TX_PWR_PA_2P5},    //9dbm
    {8,  0x4daa, TX_PWR_PA_2P5},    //8dbm
    {7,  0x89d7, TX_PWR_PA_1P8},    //7dbm
    {6,  0x89d6, TX_PWR_PA_1P8},    //6dbm
    {5,  0x89d5, TX_PWR_PA_1P8},    //5dbm
    {4,  0x89d4, TX_PWR_PA_1P8},    //4dbm
    {3,  0x89d3, TX_PWR_PA_1P8},    //3dbm
    {2,  0x8d57, TX_PWR_PA_1P8},    //2dbm
    {1,  0x8d56, TX_PWR_PA_1P8},    //1dbm
    {0,  0x8d55, TX_PWR_PA_1P8},    //0dbm
    {-1, 0x8d54, TX_PWR_PA_1P8},    //-1dbm
    {-2, 0x8d53, TX_PWR_PA_1P8},    //-2dbm
    {-3, 0x8d52, TX_PWR_PA_1P8},    //-3dbm
    {-4, 0x8d51, TX_PWR_PA_1P8},    //-4dbm
};

static const struct TX_PWR_SELECT_ITEM tx_pwr_select_metal_id_1[] =
{
    {15, 0x6dbc, TX_PWR_PA_2P5},    //15dbm
    {14, 0x6dbb, TX_PWR_PA_2P5},    //14dbm
    {13, 0x6dba, TX_PWR_PA_2P5},    //13dbm
    {12, 0x6db9, TX_PWR_PA_2P5},    //12dbm
    {11, 0x4daa, TX_PWR_PA_2P5},    //11dbm
    {10, 0x4da9, TX_PWR_PA_2P5},    //10dbm
    {9,  0x4da8, TX_PWR_PA_2P5},    //9dbm
    {8,  0x4d9c, TX_PWR_PA_2P5},    //8dbm
    {7,  0x89e1, TX_PWR_PA_1P8},    //7dbm
    {6,  0x89d4, TX_PWR_PA_1P8},    //6dbm
    {5,  0x89d3, TX_PWR_PA_1P8},    //5dbm
    {4,  0x89d2, TX_PWR_PA_1P8},    //4dbm
    {3,  0x89d1, TX_PWR_PA_1P8},    //3dbm
    {2,  0x8d54, TX_PWR_PA_1P8},    //2dbm
    {1,  0x8d53, TX_PWR_PA_1P8},    //1dbm
    {0,  0x8d52, TX_PWR_PA_1P8},    //0dbm
    {-1, 0x8d51, TX_PWR_PA_1P8},    //-1dbm
    {-2, 0x8d50, TX_PWR_PA_1P8},    //-2dbm
    {-3, 0x8d45, TX_PWR_PA_1P8},    //-3dbm
    {-4, 0x8d44 , TX_PWR_PA_1P8},    //-4dbm
};

#define BT_Rf_REG_PA_SEL 0x162
#define BLE_Rf_REG_PA_SEL 0x161

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

#define RF_REG_XTAL_FCAP                        0x103

static uint16_t xtal_fcap = DEFAULT_XTAL_FCAP;
static uint16_t init_xtal_fcap = DEFAULT_XTAL_FCAP;

struct RF_SYS_INIT_ITEM
{
    uint16_t reg;
    uint16_t set;
    uint16_t mask;
    uint16_t delay;
};

static const struct RF_SYS_INIT_ITEM rf_sys_init_tbl[] =
{
    //   {0x00E9,0x0000,0x000F,1},
    {0x0122, 0x0000, 0x1000, 1},
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

const uint16_t rf_init_tbl_1_common[][3] =
{
    {0x0002,0x0c03,1},
    {0x0008,0x0320,1},
    {0x000e,0x0812,1},
    {0x008F,0xff42,1},//logen ldo
    {0x002c,0x2446,1},//txf gain
    {0x00cb,0x00e0,1},//pad i
    {0x00c1,0x1a80,1},//pad gain
    {0x00c2,0x6858,1},//adc cap
    {0x0049,0x9004,1},//ind v
    {0x0147,0x4055,1},//pa i
    {0x00CC,0x81c4,1},// pa i
    {0x00CA,0x7800,1},// pa bias
    {0x0160,0x040c,1},
    {0x015C,0x38a2,1},
    {0x004C,0x008F,1}, //drv cap
    {0x0051,0x035e,1},
    {0x00bf,0x1002,1},
    {0x00c5,0x0043,1},
    {0x002b,0x0e00,1},//flt max gain
    {0x0081,0x0008,1},//i2v rin
    {0x0013,0x0000,1},
    {0x0014,0x2060,1},//vco i
    {0x0016,0x0150,1},
    {0x0012,0x006a,1},
    {0x009a,0x0451,1},
    {0x009F,0x0100,1},
    {0x00ea,0x1f47,1},
    {0x0016,0x0150,1},
    {0x0052,0x0010,1},
    {0x00a0,0x0d08,1},
    {0x00a2,0x3286,1},
    {0x009e,0x160c,1},
    {0x0092,0x7298,1},//tmx bias
    {0x0027,0x4a04,1},//flt bw
    {0x0146,0x461b,1},//adc cplx
    {0x0028,0x3e10,1},//i2v_corner
    {0x0090,0x3808,1},//dwa
    {0x00d0,0x03a4,1},
    {0x009c,0x0000,1},
    {0x009d,0x0503,1},
    {0x0019,0xc000,1},
    {0x0042,0x01c4,1},
    {0x0043,0x01c4,1},
    {0x0044,0x01b4,1},
    {0x0045,0x05bd,1},
    {0x0046,0x05ad,1},
    {0x0047,0x059d,1},
    {0x0048,0x058d,1},
    {0x003a,0xe018,1},
    {0x003b,0xe008,1},
    {0x003c,0xe008,1},
    {0x003d,0x6048,1},
    {0x003e,0x6048,1},
    {0x003f,0x6048,1},
    {0x0040,0x6048,1},
    {0x012c,0x003e,1},//rx2
    {0x012d,0x007c,1},
    {0x012e,0x007c,1},
    {0x012f,0x007c,1},
    {0x0130,0x007c,1},
    {0x0131,0x007c,1},
    {0x0132,0x007c,1},
    {0x0133,0x007c,1},
    {0x0085,0x0092,1},
    {0x002d,0x3b20,1},
    {0x00cf,0xff00,1},
    {0x00d1,0xa200,1},
    {0x00d9,0x0307,1},
    {0x0188,0x003f,1},
    {0x005f,0x01cf,1},//BLE
    {0x0060,0x01cf,1},
    {0x0061,0x024f,1},
    {0x0062,0x01cc,1},
    {0x0063,0x01c4,1},
    {0x0064,0x05b4,1},
    {0x0065,0x0da4,1},
    {0x0066,0x0d84,1},
    {0x0067,0xe03f,1},//
    {0x0068,0xe018,1},
    {0x0069,0x6048,1},
    {0x006a,0x6048,1},
    {0x006b,0x6088,1},
    {0x006c,0x6088,1},
    {0x006d,0x6088,1},
    {0x006e,0x6088,1},
    {0x013c,0x003e,1},//
    {0x013d,0x026f,1},
    {0x013e,0x003e,1},
    {0x013f,0x003e,1},
    {0x0140,0x003e,1},
    {0x0141,0x003e,1},
    {0x0142,0x003e,1},
    {0x0143,0x003e,1},
    {0x0082,0x9294,1},
    {0x0083,0x0024,1},
    {0x009b,0x5748,1},
    {0x00a4,0x8164,1},
    {0x0023,0x28bf,1},
    {0x0001,0x0101,1},
    {0x000a,0x0c01,1},
    {0x00d7,0x0447,1},
    {0x00da,0x0077,1},
    {0x00d8,0x36bf,1},
    {0x0150,0x4dbd,1},
    {0x0096,0x1977,1},
    {0x0097,0x3001,1},
    {0x0098,0x2461,1},
};

const uint16_t rf_init_tbl_1_t0[][3] =
{
    {0x00ce,0x0085,1},
    {0x0041,0x01cc,1},//rx0
    {0x0039,0xe03f,1},//rx1
    {0X0091,0X8220,1},
    {0x0084,0x46db,1},
    {0x00e6,0xf533,1},      //reg_bbpll_vres_ldo[5:3] from A to C, Increase bbpll drive capability
    {0x00c4,0x7c80,1},
};

#define RFREG_VER_B_C_COMMON_VAL_0ce        0x0087
#define RFREG_VER_B_C_COMMON_VAL_041        0x0245
#define RFREG_VER_B_C_COMMON_VAL_039        0xf03f
#define RFREG_VER_B_C_COMMON_VAL_147        0x4054
#define RFREG_VER_B_C_COMMON_VAL_144        0x502A
#define RFREG_VER_B_C_COMMON_VAL_145        0x4944
#define RFREG_VER_B_C_COMMON_VAL_146        0x401b
#define RFREG_VER_B_C_COMMON_VAL_145        0x4944
#define RFREG_VER_B_C_COMMON_VAL_124        0x1f3e
#define RFREG_VER_B_C_COMMON_VAL_125        0x1f3e
#define RFREG_VER_B_C_COMMON_VAL_126        0x1f3e
#define RFREG_VER_B_C_COMMON_VAL_127        0x1f3e
#define RFREG_VER_B_C_COMMON_VAL_04b        0x0500
#define RFREG_VER_B_C_COMMON_VAL_004        0x0F18
#define RFREG_VER_B_C_COMMON_VAL_009        0x6656
#define RFREG_VER_B_C_COMMON_VAL_084        0x46dc

const uint16_t rf_init_tbl_1_t1[][3] =
{
    {0x00ce,RFREG_VER_B_C_COMMON_VAL_0ce,1},
    {0x0041,RFREG_VER_B_C_COMMON_VAL_041,1},//rx0
    {0x0039,RFREG_VER_B_C_COMMON_VAL_039,1},//rx1
    {0x0147,RFREG_VER_B_C_COMMON_VAL_147,1},//pa i
    {0x0144,RFREG_VER_B_C_COMMON_VAL_144,1},
    {0x0145,RFREG_VER_B_C_COMMON_VAL_145,1},
    {0x0146,RFREG_VER_B_C_COMMON_VAL_146,1},//adc
    {0x0145,RFREG_VER_B_C_COMMON_VAL_145,1},
    {0x0124,RFREG_VER_B_C_COMMON_VAL_124,1},
    {0x0125,RFREG_VER_B_C_COMMON_VAL_125,1},
    {0x0126,RFREG_VER_B_C_COMMON_VAL_126,1},
    {0x0127,RFREG_VER_B_C_COMMON_VAL_127,1},
    {0x004b,RFREG_VER_B_C_COMMON_VAL_04b,1},
    {0x0004,RFREG_VER_B_C_COMMON_VAL_004,1},
    {0x0009,RFREG_VER_B_C_COMMON_VAL_009,1},
    {0x0084,RFREG_VER_B_C_COMMON_VAL_084,1},
    {0X0091,0x8230,1},
    {0x00e6,0xf533,1},      //reg_bbpll_vres_ldo[5:3] from A to C, Increase bbpll drive capability
    {0x00c4,0x7c80,1},
};

const uint16_t rf_init_tbl_1_t2[][3] =
{
    {0x00ce,RFREG_VER_B_C_COMMON_VAL_0ce,1},
    {0x0041,RFREG_VER_B_C_COMMON_VAL_041,1},//rx0
    {0x0039,RFREG_VER_B_C_COMMON_VAL_039,1},//rx1
    {0x0147,RFREG_VER_B_C_COMMON_VAL_147,1},//pa i
    {0x0144,RFREG_VER_B_C_COMMON_VAL_144,1},
    {0x0145,RFREG_VER_B_C_COMMON_VAL_145,1},
    {0x0146,RFREG_VER_B_C_COMMON_VAL_146,1},//adc
    {0x0145,RFREG_VER_B_C_COMMON_VAL_145,1},
    {0x0124,RFREG_VER_B_C_COMMON_VAL_124,1},
    {0x0125,RFREG_VER_B_C_COMMON_VAL_125,1},
    {0x0126,RFREG_VER_B_C_COMMON_VAL_126,1},
    {0x0127,RFREG_VER_B_C_COMMON_VAL_127,1},
    {0x004b,RFREG_VER_B_C_COMMON_VAL_04b,1},
    {0x0004,RFREG_VER_B_C_COMMON_VAL_004,1},
    {0x0009,RFREG_VER_B_C_COMMON_VAL_009,1},
    {0x0084,RFREG_VER_B_C_COMMON_VAL_084,1},
    {0x0091,0x8200,1},
    {0x00e6,0xf5B3,1},      //reg_bbpll_vres_ldo[5:3] from C to E, Increase bbpll drive capability
    {0x00c4,0x2c80,1},
};

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

#ifdef TX_PULLING_CAL

const uint16_t tx_cal_rfreg_set[][2] =
{
    //[tstbuf]
    {0x2b, 0x0700},
    {0x2c, 0x2606},
    //{0x51, 0x27f9},
    {0x81, 0x0009},
    {0x95, 0x0031},
    {0xbf, 0x3c32},
    {0xa6, 0x1e4a},
    {0x160,0x140c},

    //[ddrpll prepare]
    {0xe7, 0xe04e},
    {0xe8, 0x4030},
    {0xdd, 0x0000},
    {0xde, 0x9800},
    {0xdf, 0x0003},
    {0xe0, 0x0110},

    //[pulling mixer]
    {0xd1, 0x02c8},
    //{0xd0, 0x0224},

    //[adc rst]
    {0x0f, 0x0330},
    {0xc1, 0x1a03},
    {0x2b, 0x0710},
    {0x2b, 0x0730},
    {0x23, 0x28BE},
    {0xC8, 0x5B0D},
    {0x27, 0x2A10},
    {0x27, 0x2A30},
    {0x15c,0x38a2},
    {0x147,0x4055},
};

const uint16_t tx_cal_rfreg_store[][1] =
{
    {0x2b},
    {0x2c},
    //{0x51},
    {0x81},
    {0x95},
    {0xbf},
    {0xa6},
    {0xe7},
    {0xe8},
    {0xdd},
    {0xde},
    {0xdf},
    {0xe0},
    {0xd1},
    //{0xd0},
    {0x0f},
    {0xc1},
    {0x2b},
    {0x2b},
    {0x23},
    {0x24},
    {0xC8},
    {0x27},
    {0x27},
    {0x15c},
    {0x147},
    {0x160},
};


int bt_iqimb_test_ex (int mismatch_type);
extern int iq_gain;
extern int iq_phy;
#define BT_RF_DIG_TXPULLING_CALIB_TXPOWER_REDUCE_3DBM            6

void btdrv_tx_pulling_cal(void)
{
    int i = 0;
    uint16_t value = 0;
    uint32_t tx_cal_digreg_store[10];
    const uint16_t (*tx_cal_rfreg_set_p)[2];
    const uint16_t (*tx_cal_rfreg_store_p)[1];
    uint32_t tx_reg_set_tbl_size = 0;
    uint32_t gsg_nom_i = 0;
    uint32_t gsg_nom_q = 0;

    tx_cal_rfreg_store_p = &tx_cal_rfreg_store[0];
    uint32_t rx_reg_store_tbl_size = ARRAY_SIZE(tx_cal_rfreg_store);
    uint16_t tx_rf_local[rx_reg_store_tbl_size];
    DRIVERS_TRACE(0,"rx reg_store:\n");
    for(i=0; i< rx_reg_store_tbl_size; i++)
    {
        btdrv_read_rf_reg(tx_cal_rfreg_store_p[i][0],&value);
        tx_rf_local[i] = value;
        DRIVERS_TRACE(2,"rx reg=%x,v=%x",tx_cal_rfreg_store_p[i][0],value);
    }

    tx_cal_digreg_store[0] = BTDIGITAL_REG(0xd0350248);
    tx_cal_digreg_store[1] = BTDIGITAL_REG(0xd0340020);
    tx_cal_digreg_store[2] = BTDIGITAL_REG(0xd0220c00);
    tx_cal_digreg_store[3] = BTDIGITAL_REG(0xD0330020);
    tx_cal_digreg_store[4] = BTDIGITAL_REG(0xD0350218);
    tx_cal_digreg_store[5] = BTDIGITAL_REG(0xD0350300);
    tx_cal_digreg_store[6] = BTDIGITAL_REG(0xD0350308);
    tx_cal_digreg_store[7] = BTDIGITAL_REG(0xD0330058);
    tx_cal_digreg_store[8] = BTDIGITAL_REG(0xD0330060);
    tx_cal_digreg_store[9] = BTDIGITAL_REG(0xD0330064);

    DRIVERS_TRACE(1,"0xd0350248:%x\n",tx_cal_digreg_store[0]);
    DRIVERS_TRACE(1,"0xd0340020:%x\n",tx_cal_digreg_store[1]);
    DRIVERS_TRACE(1,"0xd0220c00:%x\n",tx_cal_digreg_store[2]);
    DRIVERS_TRACE(1,"0xD0330020:%x\n",tx_cal_digreg_store[3]);
    DRIVERS_TRACE(1,"0xD0350218:%x\n",tx_cal_digreg_store[4]);
    DRIVERS_TRACE(1,"0xD0350300:%x\n",tx_cal_digreg_store[5]);
    DRIVERS_TRACE(1,"0xD0350308:%x\n",tx_cal_digreg_store[6]);
    DRIVERS_TRACE(1,"0xD0330058:%x\n",tx_cal_digreg_store[7]);
    DRIVERS_TRACE(1,"0xD0330060:%x\n",tx_cal_digreg_store[8]);
    DRIVERS_TRACE(1,"0xD0330064:%x\n",tx_cal_digreg_store[9]);

    tx_cal_rfreg_set_p = &tx_cal_rfreg_set[0];
    tx_reg_set_tbl_size = ARRAY_SIZE(tx_cal_rfreg_set);
    DRIVERS_TRACE(0,"rx reg_set:\n");
    for(i=0; i< tx_reg_set_tbl_size; i++)
    {
        btdrv_write_rf_reg(tx_cal_rfreg_set_p[i][0],tx_cal_rfreg_set_p[i][1]);
        //BTRF_REG_DUMP(tx_cal_rfreg_set_p[i][0]);
    }

    BTDIGITAL_REG_SET_FIELD(0xd0340020, 0x1 , 0, 0);
    BTDIGITAL_REG_SET_FIELD(0xd0340020, 0x1 , 25, 1);
    BTDIGITAL_REG_SET_FIELD(0xd0350240, 0x1, 12, 1);

    BTDIGITAL_REG_SET_FIELD(0xD0330020, 0x1, 21, 1);
    BTDIGITAL_REG_SET_FIELD(0xD0330020, 0x1, 1, 1);
    BTDIGITAL_REG_WR(0xD0350218, 0x583);
    BTDIGITAL_REG_WR(0xD0350300, 0x3FF00405);
    BTDIGITAL_REG_GET_FIELD(0xD0350308, 0x1f, 0, gsg_nom_q);
    BTDIGITAL_REG_GET_FIELD(0xD0350308, 0x1f, 10, gsg_nom_i);
    gsg_nom_q -= BT_RF_DIG_TXPULLING_CALIB_TXPOWER_REDUCE_3DBM;
    gsg_nom_i -= BT_RF_DIG_TXPULLING_CALIB_TXPOWER_REDUCE_3DBM;
    BTDIGITAL_REG_SET_FIELD(0xD0350308, 0x1F, 0, gsg_nom_q);
    BTDIGITAL_REG_SET_FIELD(0xD0350308, 0x1F, 10, gsg_nom_i);

    DRIVERS_TRACE(0," 0xd0350248 : %x " , BTDIGITAL_REG(0xd0350248));
    DRIVERS_TRACE(0," 0xd0340020 : %x " , BTDIGITAL_REG(0xd0340020));
    DRIVERS_TRACE(0," 0xd0350240 : %x " , BTDIGITAL_REG(0xd0350240));
    DRIVERS_TRACE(0," 0xd0220c00 : %x " , BTDIGITAL_REG(0xd0220c00));
    DRIVERS_TRACE(0," 0xD0330020 : %x " , BTDIGITAL_REG(0xD0330020));
    DRIVERS_TRACE(0," 0xD0350218 : %x " , BTDIGITAL_REG(0xD0350218));

    btdrv_delay(1);
    bt_iqimb_test_ex(1);

    //BTRF_REG_SET_FIELD(0x22, 0x1, 1, 1);
    BTRF_REG_SET_FIELD(0x2d, 0x1, 14, 1);
    BTRF_REG_SET_FIELD(0x2d, 0x1, 15, 1);
    BTRF_REG_SET_FIELD(0xdf, 0x1, 0, 0);
    BTRF_REG_SET_FIELD(0xdf, 0x1, 1, 0);

    DRIVERS_TRACE(0,"reg_reset:\n");
    for(i=0; i< rx_reg_store_tbl_size; i++)
    {
        btdrv_write_rf_reg(tx_cal_rfreg_store_p[i][0],tx_rf_local[i]);
        //BTRF_REG_DUMP(tx_cal_rfreg_store_p[i][0]);
    }

    BTDIGITAL_REG_WR(0xd0350248, tx_cal_digreg_store[0]);
    BTDIGITAL_REG_WR(0xd0340020, tx_cal_digreg_store[1]);
    BTDIGITAL_REG_WR(0xd0220c00, tx_cal_digreg_store[2]);
    BTDIGITAL_REG_WR(0xD0330020, tx_cal_digreg_store[3]);
    BTDIGITAL_REG_WR(0xD0350218, tx_cal_digreg_store[4]);
    BTDIGITAL_REG_WR(0xD0350300, tx_cal_digreg_store[5]);
    BTDIGITAL_REG_WR(0xD0350308, tx_cal_digreg_store[6]);
    BTDIGITAL_REG_WR(0xD0330058, tx_cal_digreg_store[7]);
    BTDIGITAL_REG_WR(0xD0330060, tx_cal_digreg_store[8]);
    BTDIGITAL_REG_WR(0xD0330064, tx_cal_digreg_store[9]);

    DRIVERS_TRACE(1,"0xd0350248:%x\n",tx_cal_digreg_store[0]);
    DRIVERS_TRACE(1,"0xd0340020:%x\n",tx_cal_digreg_store[1]);
    DRIVERS_TRACE(1,"0xd0220c00:%x\n",tx_cal_digreg_store[2]);
    DRIVERS_TRACE(1,"0xD0330020:%x\n",tx_cal_digreg_store[3]);
    DRIVERS_TRACE(1,"0xD0350218:%x\n",tx_cal_digreg_store[4]);
    DRIVERS_TRACE(1,"0xD0350300:%x\n",tx_cal_digreg_store[5]);
    DRIVERS_TRACE(1,"0xD0350308:%x\n",tx_cal_digreg_store[6]);
    DRIVERS_TRACE(1,"0xD0330058:%x\n",tx_cal_digreg_store[7]);
    DRIVERS_TRACE(1,"0xD0330060:%x\n",tx_cal_digreg_store[8]);
    DRIVERS_TRACE(1,"0xD0330064:%x\n",tx_cal_digreg_store[9]);

}
#endif

struct btdrv_customer_rf_config_t btdrv_rf_customer_config;
void bt_drv_rf_sdk_init(void)
{
    DRIVERS_TRACE(0,"SDK default rf config");
    btdrv_rf_customer_config.config_xtal_en = false;//get value from NV
    btdrv_rf_customer_config.xtal_cap_val = 0;//get value from NV
    //config TX power in dbm
    btdrv_rf_customer_config.config_tx_pwr_en = true;
    btdrv_rf_customer_config.bt_tx_page_pwr = TX_PWR_8DBM;//in dbm
    btdrv_rf_customer_config.bt_tx_page_high_pwr = TX_PWR_12DBM;//in dbm
    btdrv_rf_customer_config.bt_tx_idx3_pwr = TX_PWR_12DBM;//in dbm
    btdrv_rf_customer_config.bt_tx_idx2_pwr = TX_PWR_7DBM;//in dbm
    btdrv_rf_customer_config.bt_tx_idx1_pwr = TX_PWR_2DBM;//in dbm
    btdrv_rf_customer_config.bt_tx_idx0_pwr = TX_PWR_N2DBM;//in dbm

    btdrv_rf_customer_config.le_tx_idx3_pwr = TX_PWR_12DBM;//in dbm
    btdrv_rf_customer_config.le_tx_idx2_pwr = TX_PWR_7DBM;//in dbm
    btdrv_rf_customer_config.le_tx_idx1_pwr = TX_PWR_2DBM;//in dbm
    btdrv_rf_customer_config.le_tx_idx0_pwr = TX_PWR_N2DBM;//in dbm
    //init BLE convert table
    btdrv_txpwr_conv_tbl[0] = btdrv_rf_customer_config.le_tx_idx0_pwr;
    btdrv_txpwr_conv_tbl[1] = btdrv_rf_customer_config.le_tx_idx1_pwr;
    btdrv_txpwr_conv_tbl[2] = btdrv_rf_customer_config.le_tx_idx2_pwr;
    btdrv_txpwr_conv_tbl[3] = btdrv_rf_customer_config.le_tx_idx3_pwr;
    btdrv_txpwr_conv_tbl[4] = btdrv_rf_customer_config.le_tx_max_pwr;
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

static uint16_t bt_rf_txpwr_reg_get(uint8_t idx)
{
    uint16_t rf_reg = 0;
    switch(idx)
    {
        case TX_PWR_IDX_0:
            rf_reg = RF_BT_TX_PWR_IDX0_REG;
            break;
        case TX_PWR_IDX_1:
            rf_reg = RF_BT_TX_PWR_IDX1_REG;
            break;
        case TX_PWR_IDX_2:
            rf_reg = RF_BT_TX_PWR_IDX2_REG;
            break;
        case TX_PWR_IDX_3:
            rf_reg = RF_BT_TX_PWR_IDX3_REG;
            break;
        case TX_PWR_IDX_MAX:
            rf_reg = RF_BT_TX_PWR_IDX4_REG;
            break;
        case TX_PWR_IDX_PAGE:
            rf_reg = RF_BT_TX_PWR_IDX6_REG;
            break;
        case TX_HIGH_PWR_IDX_PAGE:
            rf_reg = RF_BT_TX_PWR_IDX7_REG;
            break;
        default:
            ASSERT(0, "BT_DRV:ERROR tx pwr idx=%d", idx);
            break;
    }

    return rf_reg;
}

uint16_t ble_rf_txpwr_reg_get(uint8_t idx)
{
    uint16_t rf_reg = 0;
    switch(idx)
    {
        case TX_PWR_IDX_0:
            rf_reg = RF_BLE_TX_PWR_IDX0_REG;
            break;
        case TX_PWR_IDX_1:
            rf_reg = RF_BLE_TX_PWR_IDX1_REG;
            break;
        case TX_PWR_IDX_2:
            rf_reg = RF_BLE_TX_PWR_IDX2_REG;
            break;
        case TX_PWR_IDX_3:
            rf_reg = RF_BLE_TX_PWR_IDX3_REG;
            break;
        case TX_PWR_IDX_MAX:
            rf_reg = RF_BLE_TX_PWR_IDX4_REG;
            break;
        default:
            ASSERT(0, "BT_DRV:ERROR BLE tx pwr idx=%d", idx);
            break;
    }

    return rf_reg;
}

void bt_drv_rf_bt_pa_select(uint8_t idx, uint16_t pu_2p5pa)
{
    uint16_t read_val=0;
    btdrv_read_rf_reg(BT_Rf_REG_PA_SEL, &read_val);
    if (pu_2p5pa){
        read_val |= (1<<(idx+8));
        read_val &= ~((1<<0)<<idx);
        btdrv_write_rf_reg(BT_Rf_REG_PA_SEL, read_val);
    }else{
        read_val |= (1<<idx);
        read_val &= ~((1<<8)<<idx);
        btdrv_write_rf_reg(BT_Rf_REG_PA_SEL, read_val);
    }
}

void bt_drv_rf_ble_pa_select(uint8_t idx, uint16_t pu_2p5pa)
{
    uint16_t read_val=0;
    btdrv_read_rf_reg(BLE_Rf_REG_PA_SEL, &read_val);
    if (pu_2p5pa){
        read_val |= (1<<(idx+8));
        read_val &= ~((1<<0)<<idx);
        btdrv_write_rf_reg(BLE_Rf_REG_PA_SEL, read_val);
    }else{
        read_val |= (1<<idx);
        read_val &= ~((1<<8)<<idx);
        btdrv_write_rf_reg(BLE_Rf_REG_PA_SEL, read_val);
    }
}

int rf_reg_txpwr_val_get(int8_t txpwr_dbm, uint16_t *reg_pwr, uint16_t *pa_sel)
{
    uint32_t txpwr_val = 0;
    uint16_t i = 0;
    uint16_t tbl_size;
    enum HAL_CHIP_METAL_ID_T metal_id;
    metal_id = hal_get_chip_metal_id();
    if(metal_id == HAL_CHIP_METAL_ID_0)
    {
        tbl_size = sizeof(tx_pwr_select_metal_id_0)/sizeof(tx_pwr_select_metal_id_0[0]);
        for(i=0; i< tbl_size; i++){
            if(tx_pwr_select_metal_id_0[i].pwr_dbm == txpwr_dbm){
                *reg_pwr = tx_pwr_select_metal_id_0[i].pwr_val;
                *pa_sel = tx_pwr_select_metal_id_0[i].pa_sel;
            }
        }
    }
    else
    {
        tbl_size = sizeof(tx_pwr_select_metal_id_1)/sizeof(tx_pwr_select_metal_id_1[0]);
        for(i=0; i< tbl_size; i++){
            if(tx_pwr_select_metal_id_1[i].pwr_dbm == txpwr_dbm){
                *reg_pwr = tx_pwr_select_metal_id_1[i].pwr_val;
                *pa_sel = tx_pwr_select_metal_id_1[i].pa_sel;
            }
        }
    }

    return txpwr_val;
}

void bt_drv_ble_rf_reg_txpwr_set(uint16_t rf_reg, uint16_t val)
{
    btdrv_write_rf_reg(rf_reg, val);
}

static void bt_drv_rf_bt_txpwr_set(uint8_t idx, int8_t val_in_dbm)
{
    uint16_t pa_sel = 0;
    uint16_t tx_pwr_val = 0;
    uint16_t rf_reg = bt_rf_txpwr_reg_get(idx);

    rf_reg_txpwr_val_get(val_in_dbm, &tx_pwr_val, &pa_sel);
    btdrv_write_rf_reg(rf_reg, tx_pwr_val);
    bt_drv_rf_bt_pa_select(idx, pa_sel);

    DRIVERS_TRACE(2, "BT:TX pwr reg=0x%x,[in:%d dbm,out:0x%04x]", rf_reg, val_in_dbm, tx_pwr_val);
}

static void bt_drv_rf_ble_txpwr_set(uint8_t idx, int8_t val_in_dbm)
{
    uint16_t pa_sel = 0;
    uint16_t tx_pwr_val = 0;
    uint16_t rf_reg = ble_rf_txpwr_reg_get(idx);

    rf_reg_txpwr_val_get(val_in_dbm, &tx_pwr_val, &pa_sel);
    btdrv_write_rf_reg(rf_reg, tx_pwr_val);
    bt_drv_rf_ble_pa_select(idx, pa_sel);

    DRIVERS_TRACE(2, "BLE:TX pwr reg=0x%x,[in:%d dbm,out:0x%04x]", rf_reg, val_in_dbm, tx_pwr_val);
}

void bt_drv_tx_pwr_init(void)
{
    btdrv_write_rf_reg(BT_Rf_REG_PA_SEL,0x0000);
    btdrv_write_rf_reg(BLE_Rf_REG_PA_SEL,0x0000);

    struct btdrv_customer_rf_config_t cfg = btdrv_rf_customer_config;

    if(cfg.config_tx_pwr_en == true)
    {
        //BT
        bt_drv_rf_bt_txpwr_set(TX_PWR_IDX_0, cfg.bt_tx_idx0_pwr);
        bt_drv_rf_bt_txpwr_set(TX_PWR_IDX_1, cfg.bt_tx_idx1_pwr);
        bt_drv_rf_bt_txpwr_set(TX_PWR_IDX_2, cfg.bt_tx_idx2_pwr);
        bt_drv_rf_bt_txpwr_set(TX_PWR_IDX_3, cfg.bt_tx_idx3_pwr);
        bt_drv_rf_bt_txpwr_set(TX_PWR_IDX_MAX, cfg.bt_tx_max_pwr);
        bt_drv_rf_bt_txpwr_set(TX_PWR_IDX_PAGE, cfg.bt_tx_page_pwr);
        bt_drv_rf_bt_txpwr_set(TX_HIGH_PWR_IDX_PAGE, cfg.bt_tx_page_high_pwr);
        //BLE
        bt_drv_rf_ble_txpwr_set(TX_PWR_IDX_0, cfg.le_tx_idx0_pwr);
        bt_drv_rf_ble_txpwr_set(TX_PWR_IDX_1, cfg.le_tx_idx1_pwr);
        bt_drv_rf_ble_txpwr_set(TX_PWR_IDX_2, cfg.le_tx_idx2_pwr);
        bt_drv_rf_ble_txpwr_set(TX_PWR_IDX_3, cfg.le_tx_idx3_pwr);
        bt_drv_rf_ble_txpwr_set(TX_PWR_IDX_MAX, cfg.le_tx_max_pwr);
    }
    else
    {
        ASSERT(0, "BT_DRV:pls check TX pwr config");
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

void bt_drv_ble_adv_txpwr_via_advhdl(uint8_t adv_hdl, uint8_t idx, int8_t txpwr_dbm)
{

    DRIVERS_TRACE(3,"set adv_txpwr_via_advhdl hdl 0x%02x idx %d dbm %d", adv_hdl, idx, txpwr_dbm);

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

    bt_drv_rf_ble_txpwr_set(idx, txpwr_dbm);
}

void btdrv_cap_delay_cal(void)
{
    uint16_t value = 0;;
    uint8_t tx_capdly, rx_capdly = 0;

    tx_capdly = value & 0xF;
    rx_capdly = ((value & 0xF0) >> 4);

    if ((0 == tx_capdly) || (0 == rx_capdly)) {
        DRIVERS_TRACE(0, "invalid efuse ,so use default value");
        tx_capdly = rx_capdly = 0x8;
    }

    BTRF_REG_SET_FIELD(0x295, 0xF, 6, tx_capdly);
    BTRF_REG_SET_FIELD(0x2C5, 0xF, 6, rx_capdly);
}

// PMU_EFUSE_PAGE_SW_CFG_5
#define PMU_EFUSE_BIN_METAL_SHIFT                               8
#define PMU_EFUSE_BIN_METAL_MASK                                (0x7 << PMU_EFUSE_BIN_METAL_SHIFT)
#define PMU_EFUSE_BIN_METAL(n)                                  BITFIELD_VAL(PMU_EFUSE_BIN_METAL, n)

// PMU_EFUSE_PAGE_RESERVED_15
#define PMU_EFUSE_RF_RX_DCDC_TONE_CALIBED                       (1 << 10)
#define PMU_EFUSE_RF_RX_DCDC_TONE_CALIB_SHIFT                   8
#define PMU_EFUSE_RF_RX_DCDC_TONE_CALIB_MASK                    (0x3 << PMU_EFUSE_RF_RX_DCDC_TONE_CALIB_SHIFT)
#define PMU_EFUSE_RF_RX_DCDC_TONE_CALIB(n)                      BITFIELD_VAL(PMU_EFUSE_RF_RX_DCDC_TONE_CALIB, n)

// PMU_EFUSE_PAGE_RESERVED_15_HIGH_CHIP_TYPE
#define PMU_EFUSE_RF_RX_DCDC_TONE_CALIBED_HIGH_TYPE_SHIFT       12
#define PMU_EFUSE_RF_RX_DCDC_TONE_CALIBED_HIGH_TYPE_MASK        (0x3 << PMU_EFUSE_RF_RX_DCDC_TONE_CALIBED_HIGH_TYPE_SHIFT)
#define PMU_EFUSE_RF_RX_DCDC_TONE_CALIB_HIGH_TYPE(n)            BITFIELD_VAL(PMU_EFUSE_RF_RX_DCDC_TONE_CALIB_HIGH_TYPE,n)

enum RF_DCDC_TONE_TYPE_T {
    RF_DCDC_TONE_TYPE_0,
    RF_DCDC_TONE_TYPE_1,
    RF_DCDC_TONE_TYPE_2,
    RF_DCDC_TONE_TYPE_3,
    RF_DCDC_TONE_TYPE_4,
    RF_DCDC_TONE_TYPE_5,
    RF_DCDC_TONE_TYPE_6,
    RF_DCDC_TONE_TYPE_7,

    RF_DCDC_TONE_TYPE_QTY
};

enum RF_DCDC_TONE_BG_CURRENT_LV_T {
    RF_DCDC_BG_CURRENT_LV0 = 0x0,
    RF_DCDC_BG_CURRENT_LV1 = 0x1,
    RF_DCDC_BG_CURRENT_LV2 = 0x2,
    RF_DCDC_BG_CURRENT_LV3 = 0x3,

    RF_DCDC_BG_CURRENT_QTY
};

struct RF_DCDC_TONE_CALIBRATION_VERSION_MAP {
    enum RF_DCDC_TONE_TYPE_T type;
    uint16_t version;
};

struct RF_DCDC_TONE_CALIBRATION_MAP {
    uint16_t bg_current_level;
    uint16_t rxvco_ldo_vres;
};

static const struct RF_DCDC_TONE_CALIBRATION_VERSION_MAP chip_tbl[] = {
    {RF_DCDC_TONE_TYPE_0,     0x0B},
    {RF_DCDC_TONE_TYPE_1,     0x0D},
    {RF_DCDC_TONE_TYPE_2,     0x03},
    {RF_DCDC_TONE_TYPE_3,     0x01},
    {RF_DCDC_TONE_TYPE_4,     0x05},
    {RF_DCDC_TONE_TYPE_5,     0x00},
    {RF_DCDC_TONE_TYPE_6,     0x21},
    {RF_DCDC_TONE_TYPE_7,     0x11},

};

static const struct RF_DCDC_TONE_CALIBRATION_MAP calib_val_tbl[] = {
    {RF_DCDC_BG_CURRENT_LV0,  0x5},
    {RF_DCDC_BG_CURRENT_LV1,  0x4},
    {RF_DCDC_BG_CURRENT_LV2,  0x3},
    {RF_DCDC_BG_CURRENT_LV3,  0x3},
};

static enum RF_DCDC_TONE_TYPE_T btdrv_rf_rx_dcdc_tone_type_get(void)
{
    int i;
    enum RF_DCDC_TONE_TYPE_T chip_type = RF_DCDC_TONE_TYPE_QTY;
    uint16_t val = 0;
    uint16_t bin_version = 0;
    uint16_t high_chip_type = 0;

    pmu_get_efuse(PMU_EFUSE_PAGE_RESERVED_15, &val);
    bin_version = (val & PMU_EFUSE_RF_RX_DCDC_TONE_CALIBED) >> 10;
    high_chip_type = (val & PMU_EFUSE_RF_RX_DCDC_TONE_CALIBED_HIGH_TYPE_MASK) >> 12;

    pmu_get_efuse(PMU_EFUSE_PAGE_SW_CFG, &val);
    val = GET_BITFIELD(val, PMU_EFUSE_BIN_METAL);
    bin_version |= val << 1;
    bin_version = ((high_chip_type << 4) | (bin_version));

    for (i = 0; i < ARRAY_SIZE(chip_tbl); i++) {
        if (chip_tbl[i].version == bin_version) {
            chip_type = chip_tbl[i].type;
            break;
        }
    }
    DRIVERS_TRACE(0, "chip_type/version:0x%x/0x%x", chip_type, bin_version);

    return chip_type;
}

static void btdrv_rf_rx_vco_drv_set(void)
{
    int i;
    uint16_t val = 0;
    uint16_t bg_current_level = 0;
    uint16_t rxvco_ldo_vres = 0xffff;

    pmu_get_efuse(PMU_EFUSE_PAGE_RESERVED_15, &val);
    bg_current_level = GET_BITFIELD(val, PMU_EFUSE_RF_RX_DCDC_TONE_CALIB);

    for (i = 0; i < ARRAY_SIZE(calib_val_tbl); i++) {
        if (calib_val_tbl[i].bg_current_level == bg_current_level) {
            rxvco_ldo_vres = calib_val_tbl[i].rxvco_ldo_vres;
            break;
        }
    }

    if (rxvco_ldo_vres == 0xffff) {
        DRIVERS_TRACE(0, "invalid efuse:0x%x", val);
    } else {
        BTRF_REG_SET_FIELD(0x9B, 0x7, 12, rxvco_ldo_vres);
        DRIVERS_TRACE(0, "bg_current_lv/rxvco_ldo_vres:0x%x/0x%x", bg_current_level, rxvco_ldo_vres);
    }
}

static void btdrv_rf_rx_dcdc_tone_calibration(void)
{
    enum RF_DCDC_TONE_TYPE_T chip_type = btdrv_rf_rx_dcdc_tone_type_get();

    DRIVERS_TRACE(0, "%s chip_type:%d", __func__, chip_type);
    if (chip_type >= RF_DCDC_TONE_TYPE_QTY) {
        ASSERT(0, "chip match error!");
        return;
    }

    switch (chip_type) {
        case RF_DCDC_TONE_TYPE_0:
        case RF_DCDC_TONE_TYPE_1:
        case RF_DCDC_TONE_TYPE_3:
            btdrv_rf_rx_vco_drv_set();
            break;
        case RF_DCDC_TONE_TYPE_2:
            BTRF_REG_SET_FIELD(0x14, 0x3F, 1, 0x1A);
            break;
        case RF_DCDC_TONE_TYPE_4:
            BTRF_REG_SET_FIELD(0x14, 0x3F, 1, 0x30);
            btdrv_rf_rx_vco_drv_set();
            break;
        case RF_DCDC_TONE_TYPE_5:
            break;
        case RF_DCDC_TONE_TYPE_6:
            BTRF_REG_SET_FIELD(0x9B, 0x7, 12, 0x1);
            break;
        case RF_DCDC_TONE_TYPE_7:
            BTRF_REG_SET_FIELD(0x9B, 0x7, 12, 0x2);
            break;
        default:
            break;
    }
}

void btdrv_rfpll_frequency_range_calib(void)
{
    //calib by efuse
    uint16_t read_val;

    pmu_get_efuse(PMU_EFUSE_PAGE_RESERVED_13, &read_val);
    if (getbit(read_val, 8)) {
        BTRF_REG_SET_FIELD(0x9B, 0xF, 0, 0x2);
    }
}

uint8_t btdrv_rf_init(void)
{
    uint16_t value = 0;
    const uint16_t (*rf_init_tbl_p)[3] = NULL;
    uint32_t tbl_size = 0;
    uint8_t i;
    enum HAL_CHIP_METAL_ID_T metal_id;
    metal_id = hal_get_chip_metal_id();
    //common RF register init
    rf_init_tbl_p = &rf_init_tbl_1_common[0];
    tbl_size = sizeof(rf_init_tbl_1_common) / sizeof(rf_init_tbl_1_common[0]);

    for (i = 0; i < tbl_size; i++) {
        btdrv_write_rf_reg(rf_init_tbl_p[i][0], rf_init_tbl_p[i][1]);
        if(rf_init_tbl_p[i][2] != 0)
            btdrv_delay(rf_init_tbl_p[i][2]);//delay
        btdrv_read_rf_reg(rf_init_tbl_p[i][0], &value);
        DRIVERS_TRACE(2,"reg=%x,v=%x", rf_init_tbl_p[i][0], value);
    }

    if (metal_id == HAL_CHIP_METAL_ID_0) {
        rf_init_tbl_p = &rf_init_tbl_1_t0[0];
        tbl_size = sizeof(rf_init_tbl_1_t0) / sizeof(rf_init_tbl_1_t0[0]);
    }
    else if (metal_id == HAL_CHIP_METAL_ID_1) {
        rf_init_tbl_p = &rf_init_tbl_1_t1[0];
        tbl_size = sizeof(rf_init_tbl_1_t1) / sizeof(rf_init_tbl_1_t1[0]);
    } else if (metal_id == HAL_CHIP_METAL_ID_2) {
        rf_init_tbl_p = &rf_init_tbl_1_t2[0];
        tbl_size = sizeof(rf_init_tbl_1_t2) / sizeof(rf_init_tbl_1_t2[0]);
    }
    for (i = 0; i < tbl_size; i++) {
        btdrv_write_rf_reg(rf_init_tbl_p[i][0], rf_init_tbl_p[i][1]);
        if (rf_init_tbl_p[i][2] != 0)
            btdrv_delay(rf_init_tbl_p[i][2]);//delay
        btdrv_read_rf_reg(rf_init_tbl_p[i][0], &value);
        DRIVERS_TRACE(2,"reg=%x,v=%x", rf_init_tbl_p[i][0], value);
    }
    //system RF register init
    for (i = 0; i < ARRAY_SIZE(rf_sys_init_tbl); i++){
        btdrv_read_rf_reg(rf_sys_init_tbl[i].reg, &value);
        value = (value & ~rf_sys_init_tbl[i].mask) | (rf_sys_init_tbl[i].set & rf_sys_init_tbl[i].mask);
        if (rf_sys_init_tbl[i].delay) {
            btdrv_delay(rf_sys_init_tbl[i].delay);
        }
        btdrv_write_rf_reg(rf_sys_init_tbl[i].reg, value);
    }

    btdrv_rf_rx_dcdc_tone_calibration();
    //customer option
    bt_drv_rf_set_customer_config(NULL);
    //TX power init
    bt_drv_tx_pwr_init();
    //XTAL cap init
    bt_drv_rf_init_xtal_fcap();
    //rfpll_frequency_range_calib
    btdrv_rfpll_frequency_range_calib();
#ifdef __BLE_NEW_SWAGC_MODE__
    bt_drv_reg_op_ble_sync_agc_mode_set(true);
#endif
    return 0;
}

void bt_drv_rf_set_bt_sync_agc_enable(bool enable)
{
    uint16_t val = 0;
    btdrv_read_rf_reg(0x22,&val);
    DRIVERS_TRACE(2,"reg=22,v=%x",val);
    if(enable)
    {
        //open rf new sync agc mode
        val |= (1<<3);
    }
    else
    {
        //close rf new sync agc mode
        val &= ~(1<<3);
    }
    btdrv_write_rf_reg(0x22,val);

    btdrv_read_rf_reg(0x22,&val);
    DRIVERS_TRACE(2,"reg=22,new v=%x",val);
}

void bt_drv_rf_set_ble_sync_agc_enable(bool enable)
{
    uint16_t val = 0;
    btdrv_read_rf_reg(0x22,&val);
    if(enable)
    {
        //open rf new sync agc mode
        val |= (1<<2);
    }
    else
    {
        //close rf new sync agc mode
        val &= ~(1<<2);
    }
    btdrv_write_rf_reg(0x22,val);
}

//switch old swagc and new sync swagc
void bt_drv_select_agc_mode(enum BT_WORK_MODE_T mode)
{
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

static void btdrv_txpower_calib_dig_reg_set(uint16_t reg_val)
{
    BTDIGITAL_REG_SET_FIELD(0xD0350308, 0x1F, 0,  reg_val);
    BTDIGITAL_REG_SET_FIELD(0xD0350308, 0x1F, 10, reg_val);
    BTDIGITAL_REG_SET_FIELD(0xD0350344, 0x1F, 0,  reg_val);
}

void btdrv_txpower_calib_t0(void)
{
    uint16_t read_val;
    int16_t bt_pwr_freql, bt_pwr_freqm, bt_pwr_freqh;
    float bt_pwr_avg = 0.0f;
    int16_t calib_val = 0;

    pmu_get_efuse(PMU_EFUSE_PAGE_BT_POWER, &read_val);
    DRIVERS_TRACE(2, "%s efuse: 0x%x",__func__, read_val);
    bt_pwr_freql = read_val & 0x7;
    bt_pwr_freqm = (read_val >> 4) & 0x7;
    bt_pwr_freqh = (read_val >> 8) & 0x7;

    bt_pwr_freql = getbit(read_val, 3)  ? bt_pwr_freql : (-bt_pwr_freql);
    bt_pwr_freqm = getbit(read_val, 7)  ? bt_pwr_freqm : (-bt_pwr_freqm);
    bt_pwr_freqh = getbit(read_val, 11) ? bt_pwr_freqh : (-bt_pwr_freqh);

    bt_pwr_avg = ((float)(bt_pwr_freql + bt_pwr_freqm + bt_pwr_freqh) / 3.0f);
    //DRIVERS_TRACE(1, "bt_pwr_avg: %f", (double)bt_pwr_avg);     //Turn off LIBC_ROM to print float
    if ((read_val == 0) || (ABS(bt_pwr_avg) <= 0.5)) {
        DRIVERS_TRACE(1,"%s invalid efuse or invalid calib value.",__func__);
        return;
    }

    if (bt_pwr_avg > 0) {
        calib_val = (int16_t) (bt_pwr_avg + 0.5f);
        //reg_pad_i_sel[8:4]
        BTRF_REG_SET_FIELD(0xCB, 0x1F, 4, 0x1F);
        //reg_pa_i_sel_t[8:4]
        BTRF_REG_SET_FIELD(0xCC, 0x1F, 4, 0x1F);
    } else if (bt_pwr_avg < 0) {
        calib_val = (int16_t) (bt_pwr_avg - 0.5f);
        BTRF_REG_SET_FIELD(0xCB, 0x1F, 4, 0);
    }

    DRIVERS_TRACE(2, "%s final calib_val=%d", __func__, calib_val);

    //BT TX PWR fix center calib (8.5bm as the center, 1dbm as a step)
    if (calib_val >= 5) {       //<=3.5dBm
        btdrv_txpower_calib_dig_reg_set(0x11);
    } else if (calib_val >= 4 && calib_val < 5 ) {       //<= 4.5dBm
        btdrv_txpower_calib_dig_reg_set(0xF);
    } else if (calib_val >= 3 && calib_val < 4) {    //(4.5dBm,5.5dBm]
        btdrv_txpower_calib_dig_reg_set(0xD);
    } else if (calib_val >= 2 && calib_val < 3) {   //(5.5dBm, 6.5dBm]
        //do nth
    } else if (calib_val >= 1 && calib_val < 2) {
        BTRF_REG_SET_FIELD(0xCB, 0x1F, 4, 0x1A);
        BTRF_REG_SET_FIELD(0xCC, 0x1F, 4, 0);
    }

    if (calib_val <= -4) {      //>=12.5dBm
        btdrv_txpower_calib_dig_reg_set(0x9);
    } else if (calib_val <= -3 && calib_val > -4) {     //[11.5dBm, 12.5dBm)
        btdrv_txpower_calib_dig_reg_set(0xA);
    } else if (calib_val > -3 && calib_val <= -2) {       //[10.5dBm, 11.5dBm)
        btdrv_txpower_calib_dig_reg_set(0xB);
    } else if (calib_val > -2 && calib_val <= -1) {       //[9.5dBm, 10.5dBm)
        BTRF_REG_SET_FIELD(0xCB, 0x1F, 4, 6);
    }
}

void btdrv_txpower_calib_t1(void)
{
    uint16_t read_val;
    int16_t bt_pwr_freql, bt_pwr_freqm, bt_pwr_freqh;
    float bt_pwr_avg = 0.0f;
    int16_t calib_val = 0;

    pmu_get_efuse(PMU_EFUSE_PAGE_BT_POWER, &read_val);
    DRIVERS_TRACE(2, "%s efuse: 0x%x",__func__, read_val);
    bt_pwr_freql = read_val & 0x7;
    bt_pwr_freqm = (read_val >> 4) & 0x7;
    bt_pwr_freqh = (read_val >> 8) & 0x7;

    bt_pwr_freql = getbit(read_val, 3)  ? bt_pwr_freql : (-bt_pwr_freql);
    bt_pwr_freqm = getbit(read_val, 7)  ? bt_pwr_freqm : (-bt_pwr_freqm);
    bt_pwr_freqh = getbit(read_val, 11) ? bt_pwr_freqh : (-bt_pwr_freqh);

    bt_pwr_avg = ((float)(bt_pwr_freql + bt_pwr_freqm + bt_pwr_freqh) / 3.0f);
    //DRIVERS_TRACE(1, "bt_pwr_avg: %f", (double)bt_pwr_avg);     //Turn off LIBC_ROM to print float
    if ((read_val == 0) || (ABS(bt_pwr_avg) <= 0.5)) {
        DRIVERS_TRACE(1,"%s invalid efuse or invalid calib value.",__func__);
        return;
    }

    if (bt_pwr_avg > 0) {
        calib_val = (int16_t) (bt_pwr_avg + 0.5f);
        //reg_pad_i_sel[8:4]
        BTRF_REG_SET_FIELD(0xCB, 0x1F, 4, 0x1F);
        //reg_pa_i_sel_t[8:4]
        BTRF_REG_SET_FIELD(0xCC, 0x1F, 4, 0x1F);
    } else if (bt_pwr_avg < 0) {
        calib_val = (int16_t) (bt_pwr_avg - 0.5f);
        BTRF_REG_SET_FIELD(0xCB, 0x1F, 4, 0);
    }

    DRIVERS_TRACE(2, "%s final calib_val=%d", __func__, calib_val);

    //BT TX PWR fix center calib (8.5bm as the center, 1dbm as a step)
    if (calib_val >= 5) {       //<=3.5dBm
        btdrv_txpower_calib_dig_reg_set(0x14);
    } else if (calib_val >= 4 && calib_val < 5 ) {       //<= 4.5dBm
        btdrv_txpower_calib_dig_reg_set(0x12);
    } else if (calib_val >= 3 && calib_val < 4) {    //(4.5dBm,5.5dBm]
        btdrv_txpower_calib_dig_reg_set(0x12);
    } else if (calib_val >= 2 && calib_val < 3) {   //(5.5dBm, 6.5dBm]
        //do nth
    } else if (calib_val >= 1 && calib_val < 2) {
        BTRF_REG_SET_FIELD(0xCB, 0x1F, 4, 0x1A);
        BTRF_REG_SET_FIELD(0xCC, 0x1F, 4, 0);
    }

    if (calib_val <= -4) {      //>=12.5dBm
        btdrv_txpower_calib_dig_reg_set(0xE);
    } else if (calib_val <= -3 && calib_val > -4) {     //[11.5dBm, 12.5dBm)
        btdrv_txpower_calib_dig_reg_set(0xF);
    } else if (calib_val > -3 && calib_val <= -2) {       //[10.5dBm, 11.5dBm)
        btdrv_txpower_calib_dig_reg_set(0x10);
    } else if (calib_val > -2 && calib_val <= -1) {       //[9.5dBm, 10.5dBm)
        BTRF_REG_SET_FIELD(0xCB, 0x1F, 4, 6);
    }
}

void btdrv_txpower_calib(void)
{
    enum HAL_CHIP_METAL_ID_T metal_id;
    metal_id = hal_get_chip_metal_id();
    if(metal_id == HAL_CHIP_METAL_ID_0)
    {
        btdrv_txpower_calib_t0();
    }
    else
    {
        btdrv_txpower_calib_t1();
    }
}

//factor=N/512
void btdrv_set_bdr_ble_txpower(uint8_t txpwr_idx, uint16_t n)
{
    uint32_t reg_base = 0xd03500C4;
    uint32_t reg;
    if(txpwr_idx > 7)
    {
        DRIVERS_TRACE(1, "%s txpwr idx err:%d\n", __func__, txpwr_idx);
        return;
    }

    if(n > 1023)
    {
        DRIVERS_TRACE(1, "%s n err:%d\n", __func__, n);
        return;
    }

    reg = (txpwr_idx / 3) * 4 + reg_base;

    BTDIGITAL_REG_SET_FIELD(reg, 0x3ff, (txpwr_idx % 3) * 10, n);
}
