/***************************************************************************
 *
 * Copyright 2015-2023 BES.
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
#ifndef __ANALOG_BEST1502P_H__
#define __ANALOG_BEST1502P_H__

#ifdef __cplusplus
extern "C" {
#endif

#define ISPI_ANA_REG(r)                     (((r) & 0xFFF) | 0x1000)

#define MAX_ANA_MIC_CH_NUM                  5

void analog_aud_pll_set_dig_div(uint32_t div);

uint32_t analog_aud_get_max_dre_gain(void);

void analog_aud_codec_anc_boost(bool en);

int analog_debug_config_vad_mic(bool enable);

void analog_aud_vad_enable(enum AUD_VAD_TYPE_T type, bool en);

void analog_aud_vad_adc_enable(bool en);

void analog_aud_classd_pa_enable(bool en);

enum ANALOG_RPC_ID_T {
    ANALOG_RPC_FREQ_PLL_CFG = 0,
    ANALOG_RPC_OSC_CLK_ENABLE,
    ANALOG_RPC_CODEC_OPEN,
    ANALOG_RPC_CODEC_ADC_ENABLE,
    ANALOG_RPC_VAD_ENABLE,
    ANALOG_RPC_VAD_ADC_ENABLE,
};

#ifdef CHIP_SUBSYS_SENS

typedef int (*ANALOG_RPC_REQ_CALLBACK)(enum ANALOG_RPC_ID_T id, uint32_t param0, uint32_t param1, uint32_t param2);

void analog_aud_register_rpc_callback(ANALOG_RPC_REQ_CALLBACK cb);

#else
void analog_aud_adc_dc_auto_calib_enable(enum AUD_CHANNEL_MAP_T ch_map, bool on);

void analog_aud_adc_dc_calib_set_step(enum AUD_CHANNEL_MAP_T ch_map, uint16_t step);

void analog_aud_adc_dc_calib_offset_update_enable(enum AUD_CHANNEL_MAP_T ch_map, bool on);

void analog_aud_adc_dc_calib_offset_update(enum AUD_CHANNEL_MAP_T ch_map, uint16_t value);

void analog_capsensor_open(void);

void analog_capsensor_rc_clk_en(bool en);

void analog_capsensor_reset_set(void);

void analog_capsensor_reset_clear(void);

void analog_capsensor_clk_gate_on(void);

void analog_capsensor_clk_gate_off(void);

uint16_t analog_capsensor_clk_is_ready(void);

void analog_aud_adc_force_precharge_enable(uint32_t mic_map, bool enable);

void analog_aud_adc_force_precharge_disable(uint32_t mic_map);

void analog_aud_codec_press_enable(bool en);

#endif

#ifdef __cplusplus
}
#endif

#endif

