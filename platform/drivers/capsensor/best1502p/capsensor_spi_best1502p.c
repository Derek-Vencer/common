/***************************************************************************
 *
 * Copyright 2024-2024 BES.
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

#include "hal_trace.h"
#include "cmsis.h"
#include "hal_timer.h"
#include "plat_addr_map.h"
#include CHIP_SPECIFIC_HDR(reg_capsensor)
#include CHIP_SPECIFIC_HDR(capsensor_spi)
#include CHIP_SPECIFIC_HDR(capsensor_driver)
#include "capsensor_driver.h"
#include "hal_analogif.h"
#include "pmu.h"
#include "analog.h"
#include "hal_sleep.h"
#include "tgt_hardware_capsensor.h"

#define CAP_REG(r)                                  (((r) & 0xFFF) | 0x0000)

#define capsensorif_reg_read(reg,val)               hal_analogif_reg_read(CAP_REG(reg),val)
#define capsensorif_reg_write(reg,val)              hal_analogif_reg_write(CAP_REG(reg),val)
static enum CAPSENSOR_READ_DATA_STATE_T cap_status = CAPSENSOR_READ_DATA_IDLE;

static void capsensor_set_cur_status(enum CAPSENSOR_READ_DATA_STATE_T status)
{
    cap_status = status;
}

static enum CAPSENSOR_READ_DATA_STATE_T capsensor_get_cur_status(void)
{
    return cap_status;
}

static void capsensor_sw_control_baseline_en(void)
{
    uint16_t value;
    for (int i = 0; i < 8; i++)
    {
        capsensorif_reg_read(CAP_REG_0C + i, &value);
        value |= CDC_BASELINE_P_BIT_IN_CH0_DR;
        capsensorif_reg_write(CAP_REG_0C + i, value);
    }

    capsensorif_reg_read(CAP_REG_3D, &value);
    value |= CDC_BASELINE_N_BIT_IN_CH0_DR | CDC_BASELINE_N_BIT_IN_CH1_DR | CDC_BASELINE_N_BIT_IN_CH2_DR | CDC_BASELINE_N_BIT_IN_CH3_DR
        | CDC_BASELINE_N_BIT_IN_CH4_DR | CDC_BASELINE_N_BIT_IN_CH5_DR | CDC_BASELINE_N_BIT_IN_CH6_DR | CDC_BASELINE_N_BIT_IN_CH7_DR;
    capsensorif_reg_write(CAP_REG_3D, value);
}

void capsensor_baseline_reg_read(uint32_t* baseline_value_p, uint32_t* baseline_value_n)
{
    uint16_t baseline_val[8] = {0};
    uint16_t value = 0;

    for (int i = 0; i < 8; i++)
    {
        //capsensorif_reg_read(CAP_REG_14 + i, &baseline_val[i]);
        // DRIVERS_TRACE(0, "baseline[%d]:%x", i, baseline_val[i]);
        capsensorif_reg_read(CAP_REG_0C + i, &value);
        if (value & (1<<10)) { //cdc_baseline_p_bit_in_ch0_dr
            baseline_val[i] = value&0x3FF;
        } else {
            capsensorif_reg_read(CAP_REG_14 + i, &baseline_val[i]);
        }
        baseline_value_p[i] = baseline_val[i];
    }

    baseline_value_n[0] = baseline_val[1] << 8 | baseline_val[0];
    baseline_value_n[1] = baseline_val[3] << 8 | baseline_val[2];
    baseline_value_n[2] = baseline_val[5] << 8 | baseline_val[4];
    baseline_value_n[3] = baseline_val[7] << 8 | baseline_val[6];
}

void capsensor_baseline_dr(uint32_t* baseline_value_p, uint32_t* baseline_value_n)
{
    for (int i = 0; i < 8; i++)
    {
        capsensorif_reg_write(CAP_REG_0C + i, baseline_value_p[i]);
    }

    capsensorif_reg_write(CAP_REG_1C, baseline_value_n[0]);
    capsensorif_reg_write(CAP_REG_1D, baseline_value_n[1]);
    capsensorif_reg_write(CAP_REG_1E, baseline_value_n[2]);
    capsensorif_reg_write(CAP_REG_1F, baseline_value_n[3]);

    capsensor_sw_control_baseline_en();
}

static float formula_convert_sar_vtoc(uint32_t sar_val)
{
    float sar_value = 0;

    sar_value += ((sar_val>>9&0x1)*39/10.0);
    sar_value += ((sar_val>>8&0x1)*39/10.0/2);
    sar_value += ((sar_val>>7&0x1)*39/10.0/4);
    sar_value += ((sar_val>>6&0x1)*39/10.0/4);
    sar_value += ((sar_val>>5&0x1)*39/10.0/8);
    sar_value += ((sar_val>>4&0x1)*39/10.0/16);
    sar_value += ((sar_val>>3&0x1)*39/10.0/32);
    sar_value += ((sar_val>>2&0x1)*39/10.0/32);
    sar_value += ((sar_val>>1&0x1)*39/10.0/64);
    sar_value += ((sar_val>>0&0x1)*39/10.0/128);
    return sar_value;
}

void capsensor_fp_mode_int(void)
{
    uint16_t value = 0;

    // capsensorif_reg_read(CAP_REG_88, &value);
    // value &= ~(FP_MODE_RD_INT_MASK);//fp_mode_rd_int_mask
    // capsensorif_reg_write(CAP_REG_88, value);

    // capsensorif_reg_read(CAP_REG_87, &value);
    // value |= FP_MODE_RD_INT_RAW_EN_REG;//fp_mode_rd_int_raw_en_reg
    // capsensorif_reg_write(CAP_REG_87, value);

    capsensorif_reg_read(CAP_REG_02, &value);
    value |= POWER_MODE_REG; //power_mode_reg
    capsensorif_reg_write(CAP_REG_02, value);
}

void capsensor_press_int(void)
{
    uint16_t value = 0;

    capsensorif_reg_read(CAP_REG_88, &value);
    value &= ~(PRESS_INT_MASK);//press_int_mask
    capsensorif_reg_write(CAP_REG_88, value);

    capsensorif_reg_read(CAP_REG_87, &value);
    value |= PRESS_INT_EN_REG;//press_int_en_reg
    capsensorif_reg_write(CAP_REG_87, value);

    capsensorif_reg_read(CAP_REG_02, &value);
    value |= POWER_MODE_REG; //power_mode_reg
    capsensorif_reg_write(CAP_REG_02, value);

    capsensorif_reg_write(CAP_REG_93, 0xFFFF); //threshold_value_ch0
    capsensorif_reg_write(CAP_REG_94, 0xFFFF); //threshold_value_ch1
    capsensorif_reg_write(CAP_REG_95, 0xFFFF); //threshold_value_ch2
    capsensorif_reg_write(CAP_REG_96, 0xFFFF); //threshold_value_ch3
    capsensorif_reg_write(CAP_REG_97, 0xFFFF); //threshold_value_ch4
}

static void capsensor_fifo_num_and_capture_config(uint8_t cap_ch_num, uint8_t cap_conversion_num, uint16_t cap_samp_fs)
{
    uint16_t value = 0;

    capsensorif_reg_read(CAP_REG_80, &value);
    value |= FIFO_DATA_SDM_ONLY; //fifo_data_sdm_only
    value &= ~(FIFO_TAG_EN); //fifo_tag_en
    capsensorif_reg_write(CAP_REG_80, value);

    capsensorif_reg_read(CAP_REG_02, &value);
    value |= (1 << 14);
    value = SET_BITFIELD(value, ACTIVE_CH_NUM, cap_ch_num); //active_ch_num
    capsensorif_reg_write(CAP_REG_02, value);

    capsensorif_reg_read(CAP_REG_3E, &value);
    value = SET_BITFIELD(value, FIFO_DATA_NUM_TH, (cap_ch_num+1)*cap_conversion_num); //fifo_data_num_th
    capsensorif_reg_write(CAP_REG_3E, value);

    capsensorif_reg_read(CAP_REG_3C, &value);
    value = SET_BITFIELD(value, LP_SLEEP_TIME, (cap_samp_fs*33)); //sleep time 10ms 0x14D  , 20ms 0x29A, 50ms 0x681
    capsensorif_reg_write(CAP_REG_3C, value);
}

static void capsensor_ch_map_config(uint8_t chan_num, uint8_t cap_ch_map)
{
    uint16_t value = 0;
    uint8_t ch_map[8]={0};

    for(uint8_t i=0,j=0; i<8; i++) {
        if(cap_ch_map & (1<<i)) {
            ch_map[j++] = i;
            if(j >= (chan_num+1))
                break;
        }
    }

    capsensorif_reg_read(CAP_REG_55, &value);
    value = SET_BITFIELD(value, CH_CONFIG_IDX_0, ch_map[0]);
    value = SET_BITFIELD(value, CH_CONFIG_IDX_1, ch_map[1]);
    capsensorif_reg_write(CAP_REG_55, value);
    capsensorif_reg_read(CAP_REG_56, &value);
    value = SET_BITFIELD(value, CH_CONFIG_IDX_2, ch_map[2]);
    value = SET_BITFIELD(value, CH_CONFIG_IDX_3, ch_map[3]);
    capsensorif_reg_write(CAP_REG_56, value);
    capsensorif_reg_read(CAP_REG_57, &value);
    value = SET_BITFIELD(value, CH_CONFIG_IDX_4, ch_map[4]);
    value = SET_BITFIELD(value, CH_CONFIG_IDX_5, ch_map[5]);
    capsensorif_reg_write(CAP_REG_57, value);
    capsensorif_reg_read(CAP_REG_58, &value);
    value = SET_BITFIELD(value, CH_CONFIG_IDX_6, ch_map[6]);
    value = SET_BITFIELD(value, CH_CONFIG_IDX_7, ch_map[7]);
    capsensorif_reg_write(CAP_REG_58, value);
}

static void capsensor_down_sel_and_cin_config(void)
{
    uint16_t value = 0;

    capsensorif_reg_read(CAP_REG_60, &value);
    value = SET_BITFIELD(value, DOWN_SEL_CH0, 0x1);
    value = SET_BITFIELD(value, DOWN_SEL_CH1, 0x1);
    value = SET_BITFIELD(value, DOWN_SEL_CH2, 0x1);
    value = SET_BITFIELD(value, DOWN_SEL_CH3, 0x1);
    value = SET_BITFIELD(value, DOWN_SEL_CH4, 0x1);
    capsensorif_reg_write(CAP_REG_60, value);
    capsensorif_reg_read(CAP_REG_61, &value);
    value = SET_BITFIELD(value, DOWN_SEL_CH5, 0x1);
    value = SET_BITFIELD(value, DOWN_SEL_CH6, 0x1);
    value = SET_BITFIELD(value, DOWN_SEL_CH7, 0x1);
    capsensorif_reg_write(CAP_REG_61, value);

    capsensorif_reg_read(CAP_REG_66, &value);
    value = SET_BITFIELD(value, CDC_SDM_OP_CIN1_CH0_REG, 0x1);
    value = SET_BITFIELD(value, CDC_SDM_OP_CIN1_CH1_REG, 0x1);
    value = SET_BITFIELD(value, CDC_SDM_OP_CIN1_CH2_REG, 0x1);
    value = SET_BITFIELD(value, CDC_SDM_OP_CIN1_CH3_REG, 0x1);
    capsensorif_reg_write(CAP_REG_66, value);

    capsensorif_reg_read(CAP_REG_67, &value);
    value = SET_BITFIELD(value, CDC_SDM_OP_CIN1_CH4_REG, 0x1);
    value = SET_BITFIELD(value, CDC_SDM_OP_CIN1_CH5_REG, 0x1);
    value = SET_BITFIELD(value, CDC_SDM_OP_CIN1_CH6_REG, 0x1);
    value = SET_BITFIELD(value, CDC_SDM_OP_CIN1_CH7_REG, 0x1);
    capsensorif_reg_write(CAP_REG_67, value);
}

static void capsensor_fir_filter_and_smaple_point_config(void)
{
    uint16_t value = 0;

    capsensorif_reg_read(CAP_REG_5F, &value);
    value = 0XFF;  //enable fir0 & fir1
    capsensorif_reg_write(CAP_REG_5F, value);

    capsensorif_reg_read(CAP_REG_04, &value);
    value = SET_BITFIELD(value, CH_IDX_0_SAMPLE_POINT, 0x8);
    value = SET_BITFIELD(value, CH_IDX_1_SAMPLE_POINT, 0x8);
    capsensorif_reg_write(CAP_REG_04, value);
    capsensorif_reg_read(CAP_REG_05, &value);
    value = SET_BITFIELD(value, CH_IDX_2_SAMPLE_POINT, 0x8);
    value = SET_BITFIELD(value, CH_IDX_3_SAMPLE_POINT, 0x8);
    capsensorif_reg_write(CAP_REG_05, value);
    capsensorif_reg_read(CAP_REG_06, &value);
    value = SET_BITFIELD(value, CH_IDX_4_SAMPLE_POINT, 0x8);
    value = SET_BITFIELD(value, CH_IDX_5_SAMPLE_POINT, 0x8);
    capsensorif_reg_write(CAP_REG_06, value);
    capsensorif_reg_read(CAP_REG_07, &value);
    value = SET_BITFIELD(value, CH_IDX_6_SAMPLE_POINT, 0x8);
    value = SET_BITFIELD(value, CH_IDX_7_SAMPLE_POINT, 0x8);
    capsensorif_reg_write(CAP_REG_07, value);

    capsensorif_reg_read(CAP_REG_7F, &value);
    value = SET_BITFIELD(value, CH_SAMPLE_START, 0x7);
    capsensorif_reg_write(CAP_REG_7F, value);
}

static const uint16_t capsensor_cin_value_table[][3] =
{
    {0,  204,  3},   //0PF   < x <= 15PF,  15.12PF => 204
    {204, 650, 3},   //15PF  < x <= 35PF,  35PF => 650
    {650, 1023, 3},  //35PF  < x <= 60PF,  61.46PF => 1023
};

void hal_capsensor_cap_ch_cin_write(void)
{
    uint16_t value = 0;
    uint16_t baseline_val[8] = {0};

    for (int i = 0; i < 8; i++)
    {
        capsensorif_reg_read(CAP_REG_0C + i, &value);
        if (value & (1<<10)) { //cdc_baseline_p_bit_in_ch0_dr
            baseline_val[i] = value&0x3FF;
        } else {
            capsensorif_reg_read(CAP_REG_14 + i, &baseline_val[i]);
        }
        // DRIVERS_TRACE(0, "baseline[%d]:%x ", i, baseline_val[i]);
    }

    const uint16_t (*capsensor_cin_value_table_p)[3] = NULL;
    uint32_t tbl_size = 0;
    capsensor_cin_value_table_p = &capsensor_cin_value_table[0];
    tbl_size = sizeof(capsensor_cin_value_table)/sizeof(capsensor_cin_value_table[0]);

    for (uint8_t j = 0; j < tbl_size; j++) {
        if(baseline_val[0] <= capsensor_cin_value_table_p[j][1]) {
            capsensorif_reg_read(CAP_REG_66, &value);
            value = SET_BITFIELD(value, CDC_SDM_OP_CIN1_CH0_REG, capsensor_cin_value_table_p[j][2]);
            capsensorif_reg_write(CAP_REG_66, value);
            break;
        }
    }

    for (uint8_t j = 0; j < tbl_size; j++) {
        if(baseline_val[1] <= capsensor_cin_value_table_p[j][1]) {
            capsensorif_reg_read(CAP_REG_66, &value);
            value = SET_BITFIELD(value, CDC_SDM_OP_CIN1_CH1_REG, capsensor_cin_value_table_p[j][2]);
            capsensorif_reg_write(CAP_REG_66, value);
            break;
        }
    }

    for (uint8_t j = 0; j < tbl_size; j++) {
        if(baseline_val[2] <= capsensor_cin_value_table_p[j][1]) {
            capsensorif_reg_read(CAP_REG_66, &value);
            value = SET_BITFIELD(value, CDC_SDM_OP_CIN1_CH2_REG, capsensor_cin_value_table_p[j][2]);
            capsensorif_reg_write(CAP_REG_66, value);
            break;
        }
    }

    for (uint8_t j = 0; j < tbl_size; j++) {
        if(baseline_val[3] <= capsensor_cin_value_table_p[j][1]) {
            capsensorif_reg_read(CAP_REG_66, &value);
            value = SET_BITFIELD(value, CDC_SDM_OP_CIN1_CH3_REG, capsensor_cin_value_table_p[j][2]);
            capsensorif_reg_write(CAP_REG_66, value);
            break;
        }
    }

    for (uint8_t j = 0; j < tbl_size; j++) {
        if(baseline_val[4] <= capsensor_cin_value_table_p[j][1]) {
            capsensorif_reg_read(CAP_REG_67, &value);
            value = SET_BITFIELD(value, CDC_SDM_OP_CIN1_CH4_REG, capsensor_cin_value_table_p[j][2]);
            capsensorif_reg_write(CAP_REG_67, value);
            break;
        }
    }

    for (uint8_t j = 0; j < tbl_size; j++) {
        if(baseline_val[5] <= capsensor_cin_value_table_p[j][1]) {
            capsensorif_reg_read(CAP_REG_67, &value);
            value = SET_BITFIELD(value, CDC_SDM_OP_CIN1_CH5_REG, capsensor_cin_value_table_p[j][2]);
            capsensorif_reg_write(CAP_REG_67, value);
            break;
        }
    }

    for (uint8_t j = 0; j < tbl_size; j++) {
        if(baseline_val[6] <= capsensor_cin_value_table_p[j][1]) {
            capsensorif_reg_read(CAP_REG_67, &value);
            value = SET_BITFIELD(value, CDC_SDM_OP_CIN1_CH6_REG, capsensor_cin_value_table_p[j][2]);
            capsensorif_reg_write(CAP_REG_67, value);
            break;
        }
    }

    for (uint8_t j = 0; j < tbl_size; j++) {
        if(baseline_val[7] <= capsensor_cin_value_table_p[j][1]) {
            capsensorif_reg_read(CAP_REG_67, &value);
            value = SET_BITFIELD(value, CDC_SDM_OP_CIN1_CH7_REG, capsensor_cin_value_table_p[j][2]);
            capsensorif_reg_write(CAP_REG_67, value);
            break;
        }
    }
}

void capsensor_setup_cfg(struct CAPSENSOR_CFG_T * cap_cfg)
{
    uint16_t value = 0;

    capsensorif_reg_read(CAP_REG_84, &value);
    value &= ~(PU_OSC_CAP_SENSOR_DR);//pu_osc_cap_sensor_dr
    capsensorif_reg_write(CAP_REG_84, value);

#ifdef CAPSENSOR_FP_MODE
    capsensor_fp_mode_int();
#elif CAPSENSOR_PRESS_INT
    capsensor_press_int();
#endif

    capsensor_fifo_num_and_capture_config(cap_cfg->ch_num, cap_cfg->conversion_num, cap_cfg->samp_fs);
    capsensor_ch_map_config(cap_cfg->ch_num, cap_cfg->ch_map);
    capsensor_down_sel_and_cin_config();
    capsensor_fir_filter_and_smaple_point_config();

    capsensorif_reg_read(CAP_REG_8C, &value);
    value |= (1 << 1 | 1<<3);
    capsensorif_reg_write(CAP_REG_8C, value);

    capsensorif_reg_read(CAP_REG_8B, &value);
    for(uint8_t i = 0; i < 8; i++)
    {
        if(cap_cfg->ch_map & (1<<i))
        {
            value |= (1<<(i+1));
        }
    }
    //value = SET_BITFIELD(value, CDC_PD_IDLE_CHAN, 0xFF);
    capsensorif_reg_write(CAP_REG_8B, value);

    capsensorif_reg_read(CAP_REG_92, &value);
    value |= 1 << 4;
    capsensorif_reg_write(CAP_REG_92, value);

    capsensorif_reg_read(CAP_REG_01, &value);
    value |= CDC_PU_VREF0P7 | CDC_PU_LDO | CDC_PU_CDC_REG;
    capsensorif_reg_write(CAP_REG_01, value);
#ifdef CAPSENSOR_USE_RC
    analog_capsensor_rc_clk_en(true); //osc clk -> rc clk
#endif
    hal_sys_timer_delay(MS_TO_TICKS(50));
    value |= FSM_EN;
    value |= CDC_SAR_P_MODE_DR;
    value &= ~CDC_SAR_P_MODE_REG;
    capsensorif_reg_write(CAP_REG_01, value);

    hal_sys_timer_delay(US_TO_TICKS(100*(cap_cfg->ch_num+1))); // one ch:1/333.3K * 10 = 30us
    hal_capsensor_cap_ch_cin_write();
    capsensorif_reg_read(CAP_REG_01, &value);
    value &= ~CDC_SAR_P_MODE_DR;
    capsensorif_reg_write(CAP_REG_01, value);
}

int get_sample(struct capsensor_sample_data *sample, int i)
{
    uint32_t sdm_low;
    uint32_t sdm_high;
    uint16_t value = 0;

    capsensorif_reg_read(CAP_REG_63, &value);
    // capsensorif_reg_read(CAP_REG_82, &value);
    // DRIVERS_TRACE_IMM(0, "0x82:%x rd_addr_ptr1:%x wr_addr_ptr1:%x", value, (value>>8)&0x7f, value&0x7f);
    capsensorif_reg_read(CAP_REG_43, &value);
    sample[i].ch = value & (~0xfff8); //0x43 [2:0]  ch_count
    sdm_low = (value>>4)&0x0F; //0x43 [7:4]  sdm[3:0]
    capsensorif_reg_read(CAP_REG_44, &value);
    sdm_high = value; //0x44 [15:0] sdm[19:4]
    sample[i].sdm = (sdm_high<<4) | sdm_low; //sdm(high,low)

    return 0;
}

int stop_hw_wr_start_sw_rd(uint8_t cap_num)
{
    uint16_t value = 0;
    uint16_t ptr_reg_value = 0;

    capsensorif_reg_read(CAP_REG_54, &value);
    value |= FIFO_CLK_DR | FIFO_RD_DIRECTION_REG; //fifo_clk_dr
    value &= ~FIFO_RD_CLK_DR; //fifo_rd_clk_dr , fifo_rd_direction_reg:0 add,1 reduce
    capsensorif_reg_write(CAP_REG_54, value);

    capsensorif_reg_read(CAP_REG_82, &ptr_reg_value);

    capsensorif_reg_read(CAP_REG_8A, &value); //rd_ptr_dr
    value = ((ptr_reg_value&0x7f)-cap_num) | RD_PTR_DR;
    capsensorif_reg_write(CAP_REG_8A, value);

    capsensorif_reg_read(CAP_REG_8A, &value); //rd_ptr_dr
    value &= ~RD_PTR_DR;
    capsensorif_reg_write(CAP_REG_8A, value);

    return 0;
}

int stop_sw_rd_start_hw_wr(void)
{
    uint16_t value = 0;

    capsensorif_reg_read(CAP_REG_54, &value);
    value &= ~(FIFO_CLK_DR| FIFO_RD_START_REG);
    capsensorif_reg_write(CAP_REG_54, value);

    return 0;
}

void select_sort_sample_data(struct capsensor_sample_data *sample, int chnum)
{
    uint32_t swap;
    for (int i = 0; i < chnum; i++)
    {
        for(int j = i+1 ; j < chnum; j++)
        {
                if(sample[j].ch < sample[i].ch)
                {
                    swap = sample[i].ch;
                    sample[i].ch = sample[j].ch;
                    sample[j].ch = swap;

                    swap = sample[i].sdm;
                    sample[i].sdm = sample[j].sdm;
                    sample[j].sdm = swap;
                }
        }
    }
}

void count_sar_value(struct capsensor_sample_data *sample_value)
{
    float sar_value = 0;

    sar_value = formula_convert_sar_vtoc(sample_value->sar);
    sample_value->sar_int = (int)sar_value;
    sample_value->sar_float = (int)((sar_value-(float)(sample_value->sar_int))*1000000);
}

void count_sdm_value(struct capsensor_sample_data *sample_value)
{
    float sdm_value = 0;

    sdm_value += ((sample_value->sdm)/1048575.0f*10.0f);  //20bit sdm/1048575.0 = (0-1)pf
    sample_value->sdm_int = (int)sdm_value;
    sample_value->sdm_float = (int)((sdm_value-(float)(sample_value->sdm_int))*1000000);
}

void count_sar_sdm_value(struct capsensor_sample_data *sample_value)
{
    float sar_value = 0;
    float sdm_value = 0;

    sar_value = formula_convert_sar_vtoc(sample_value->sar);
    sdm_value += ((sample_value->sdm)/1048575.0f*10.0f);  //20bit sdm/1048575.0 = (0-1)pf
    sample_value->sar_sdm_int = (int)(sar_value + sdm_value);
    sample_value->sar_sdm_float = (int)((sar_value + sdm_value-(float)(sample_value->sar_sdm_int))*1000000);
}

void calculate_sample_data(struct capsensor_sample_data *sample, uint8_t cap_num) //calculate V->C
{
    for(int data = 0; data < cap_num; data++)
    {
        count_sar_value(&sample[data]);
        count_sdm_value(&sample[data]);
        count_sar_sdm_value(&sample[data]);
    }
}

int capsensor_irq_type_judge(void)
{
    uint16_t value = 0;

    capsensorif_reg_read(CAP_REG_88, &value);
    if((value & (1<<12)) == (1<<12)) { //FP_MODE_RD
        value |= 1<<15; //fp_mode_rd_int_clr_reg
        capsensorif_reg_write(CAP_REG_88, value);
        return FP_MODE_RD_INT;
    } else if((value & (1<<8)) == (1<<8)) { //press mode
        return PRESS_INT;
    }else {
        return -1;
    }
}

void capsensor_module_reset(void)
{
    capsensorif_reg_write(CAP_REG_91, 0x1F);
    capsensorif_reg_write(CAP_REG_91, 0x01);

    capsensorif_reg_write(CAP_REG_91, 0x1E);
}

void capsensor_baseline_read(uint8_t num)
{
    uint16_t value = 0;
    uint16_t baseline_val[8] = {0};

    for (int i = 0; i < num; i++)
    {
        capsensorif_reg_read(CAP_REG_0C + i, &value);
        if (value & (1<<10)) { //cdc_baseline_p_bit_in_ch0_dr
            baseline_val[i] = value&0x3FF;
        } else {
            capsensorif_reg_read(CAP_REG_14 + i, &baseline_val[i]);
            baseline_val[i] |= CDC_BASELINE_P_BIT_IN_CH0_DR;
            capsensorif_reg_write(CAP_REG_0C + i, baseline_val[i]); //0x0c-0x13
        }
        DRIVERS_TRACE_IMM(0, "baseline[%d]:%x ", i, baseline_val[i]&0x3FF);
    }
}

void capsensor_pu_rc_dr(bool en)
{
    uint16_t value = 0;

    capsensorif_reg_read(CAP_REG_84, &value);
    if(en) {
        value |= PU_OSC_CAP_SENSOR_DR | PU_OSC_CAP_SENSOR_REG;
    } else {
        value &= ~PU_OSC_CAP_SENSOR_DR;
    }
    capsensorif_reg_write(CAP_REG_84, value);
}

void capsensor_wakeup_start_capture(void)
{
    uint16_t value = 0;

#ifdef CAPSENSOR_USE_RC
    capsensor_pu_rc_dr(true);
#endif
    capsensor_resume();

    capsensorif_reg_read(CAP_REG_01, &value);
    value |= FSM_EN;
    capsensorif_reg_write(CAP_REG_01, value);
}

void capsensor_read_capture_data(struct capsensor_sample_data *sample, int num)
{
    static bool baseline_dr_flag = 0;

    stop_hw_wr_start_sw_rd(num);
    capsensor_set_cur_status(CAPSENSOR_READ_DATA_START);
    for(int i=0; i<num; i++) {
        get_sample(sample, i);
    }
    stop_sw_rd_start_hw_wr();
    capsensor_set_cur_status(CAPSENSOR_READ_DATA_DONE);

    select_sort_sample_data(sample, num);
    if(baseline_dr_flag == 0) {
        capsensor_baseline_read(num);
        baseline_dr_flag = 1;
    }
#if 0
    calculate_sample_data(sample, num);

    for (int k = 0; k < num; k++) {
        DRIVERS_TRACE(0 ,"cap_data:%d\t%x\t%d.%06d\t%x\t%d.%06d\t%d.%06d", sample[k].ch, sample[k].sar,
            sample[k].sar_int, sample[k].sar_float,
            sample[k].sdm, sample[k].sdm_int, sample[k].sdm_float,
            sample[k].sar_sdm_int, sample[k].sar_sdm_float);
    }
    DRIVERS_TRACE(0, "\n");
#endif
}

void capsensor_state_machine_reset(void)
{
    capsensor_suspend(); //clk gate enable
    capsensor_module_reset(); //reset fsm en
#ifdef CAPSENSOR_USE_RC
    capsensor_pu_rc_dr(false);
#endif
}

void capsensor_write_pointer_position(uint16_t * write_addr)
{
    uint16_t ptr_reg_value = 0;

    *write_addr = 0;

    capsensorif_reg_read(CAP_REG_82, &ptr_reg_value);
    *write_addr = ptr_reg_value&0x7f;
}

int capsensor_get_raw_data(struct capsensor_sample_data *sample, int num)
{
    int count = 0;
    uint16_t value = 0;
    uint16_t write_addr = 0;

    value = capsensor_clk_is_ready();
    if(!value) {
        capsensor_wakeup_start_capture();
        capsensor_set_cur_status(CAPSENSOR_READ_DATA_WAIT_READY);
    } else {
        do {
            capsensor_write_pointer_position(&write_addr);
            if (write_addr <= num) {
                hal_sys_timer_delay(MS_TO_TICKS(1));
            }
        } while(write_addr <= num && ++count < 15);

        capsensor_write_pointer_position(&write_addr);
        if(write_addr <= num) {
            capsensor_set_cur_status(CAPSENSOR_READ_DATA_NOT_ENOUGH);
            DRIVERS_TRACE(0, "%s , capsensor samp num not enough !!!", __func__);
            return capsensor_get_cur_status();
        }

        capsensor_write_pointer_position(&write_addr);
        if(write_addr > num) {
            capsensor_read_capture_data(sample, num);
            capsensor_set_cur_status(CAPSENSOR_READ_DATA_READY);
        } else {
            capsensor_set_cur_status(CAPSENSOR_READ_DATA_NOT_ENOUGH);
            DRIVERS_TRACE(0, "%s , capsensor samp num not enough !!!", __func__);
            return capsensor_get_cur_status();
        }
    }
    return capsensor_get_cur_status();
}

void capsensor_fp_mode_set_mask(void)
{
    uint16_t value = 0;

    capsensorif_reg_read(CAP_REG_88, &value); //fp_mode_rd_int_mask
    value |= FP_MODE_RD_INT_MASK;
    capsensorif_reg_write(CAP_REG_88, value);
}

void capsensor_fp_mode_clr_mask(void)
{
    uint16_t value = 0;

    capsensorif_reg_read(CAP_REG_88, &value); //fp_mode_rd_int_mask
    value &= ~FP_MODE_RD_INT_MASK;
    capsensorif_reg_write(CAP_REG_88, value);
}

void capsensor_fp_mode_clear_irq(void)
{
    uint16_t value = 0;

    capsensorif_reg_read(CAP_REG_88, &value); //fp_mode_rd_int_clr_reg
    value |= FP_MODE_RD_INT_CLR_REG;
    capsensorif_reg_write(CAP_REG_88, value);
}

void capsensor_reg_val_print(void)
{
    uint16_t value, reg;

    DRIVERS_TRACE(0, "%s start", __func__);
    for (reg = 0; reg <= 0xA3; reg++) {
        capsensorif_reg_read(reg, &value);
        DRIVERS_TRACE_IMM(0, "0x%04X=0x%04X", reg, value);
    }
    DRIVERS_TRACE(0, "%s done", __func__);
}
