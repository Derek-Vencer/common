/***************************************************************************
 * Copyright 2022-2023 BES.
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
 ***************************************************************************/
#ifdef CAPSENSOR_FAC_CALCULATE
#include "cmsis.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "string.h"
#include "hal_trace.h"
#include "capsensor_factory_cal.h"
#include "tgt_hardware_capsensor.h"
#ifdef CHIP_SUBSYS_SENS
#include "app_sensor_hub.h"
#endif
#include "capsensor_algorithm.h"
#if defined(CHIP_BEST1501P)
#include "reg_analog_best1501p.h"
#endif
#ifdef BESUI_CAPSENSOR_FACTORY_EN
#include "stdlib.h"
#include "math.h"
#include "nvrecord_extension.h"
#include "nvrecord_env.h"
#include "app_hfp.h"

#if defined(BESUI_TWS_EN) || defined(BESUI_STEREO_EN)
#include "besui_hfp.h"
#endif
#include CHIP_SPECIFIC_HDR(capsensor_driver)
#endif

static int cal_count = 0;
static bool cal_offset_flag = false;
static bool cal_diff_flag = false;
static int cal_data_sum[CAP_CHNUM] = {0};
static int cal_data_offset_avg[CAP_CHNUM] = {0};
static int cal_data_diff_avg[CAP_CHNUM] = {0};
static int cal_data_max[CAP_CHNUM] = {0};
static int cal_data_min[CAP_CHNUM] = {0};
#if defined(CAPSENSOR_WEAR)
capsensor_wear_cal_data wear_cal_data;
capsensor_wear_cal_range wear_cal_range;
#endif
#if defined(CAPSENSOR_TOUCH)
capsensor_touch_cal_range touch_cal_range;
capsensor_touch_cal_data touch_cal_data;
#endif

#if defined(CHIP_SUBSYS_SENS)
void app_send_capsensor_factory_test_data(uint8_t * ptr, uint16_t len)
{
    //CAPSENSOR_TRACE(2, "%s  send capsensor test %d", __func__, len);
    app_core_bridge_send_cmd(MCU_SENSOR_HUB_TASK_CMD_CAPSENSOR_FACTORY_TEST, ptr, len);
}

static void app_sensor_hub_capsensor_factory_test_cmd_received_handler(uint8_t* ptr, uint16_t len)
{
    CAPSENSOR_FACTORY_CAL_FLAG cap_cal_flag;
    cap_cal_flag = *((CAPSENSOR_FACTORY_CAL_FLAG*)ptr);
    //CAPSENSOR_TRACE(2, "%s receive sensor hub data len %d", __func__, len);
    capsensor_factory_calculate_flag_set(cap_cal_flag.offset_cal, cap_cal_flag.diff_cal);
}

static void app_sensor_hub_spp_transmit_capsensor_test_cmd_handler(uint8_t* ptr, uint16_t len)
{
    //CAPSENSOR_TRACE(2, "%s len %d", __func__, len);
    app_core_bridge_send_data_without_waiting_rsp(MCU_SENSOR_HUB_TASK_CMD_CAPSENSOR_FACTORY_TEST, ptr, len);
}

static void app_sensor_hub_capsensor_factory_test_tx_done_handler(uint16_t cmdCode, uint8_t* ptr, uint16_t len)
{
    //CAPSENSOR_TRACE(2, "%s cmdCode 0x%x", __func__, cmdCode);
}

CORE_BRIDGE_TASK_COMMAND_TO_ADD(MCU_SENSOR_HUB_TASK_CMD_CAPSENSOR_FACTORY_TEST,
                                "spp receive no rsp req from sensor",
                                app_sensor_hub_spp_transmit_capsensor_test_cmd_handler,
                                app_sensor_hub_capsensor_factory_test_cmd_received_handler,
                                0,
                                NULL,
                                NULL,
                                app_sensor_hub_capsensor_factory_test_tx_done_handler);
#endif

#ifdef BESUI_CAPSENSOR_FACTORY_EN
bool offset_result = 0;
void capsensor_offset_set_result(bool dat)
{
    BESUI_TRACE(0, "[UICAP][%s], offset_set_result = %d", __func__, dat);
    for(uint8_t i = 0; i < CAP_CHNUM; i++)
    {
        cal_data_sum[i] = 0;
        cal_data_offset_avg[i] = 0;
        cal_data_diff_avg[i] = 0;
        cal_data_max[i] = 0;
        cal_data_min[i] = 0;
    }
    offset_result = dat;
}

bool capsensor_offset_get_result(void)
{
    BESUI_TRACE(0, "[UICAP][%s], offset_result = %d", __func__, offset_result);
    return offset_result;
}
#endif

static void capsensor_cal_offset(struct capsensor_sample_data * data, int len)
{
    uint32_t i = 0;
    uint32_t chan_data_sum[CAP_CHNUM*CAP_REPNUM];
    calcuate_ch_adc_sum(data, CAP_CHNUM, chan_data_sum, len);
    for(i = 0; i < CAP_CHNUM; i++)
    {
        cal_data_sum[i] += chan_data_sum[i];
        CAPSENSOR_TRACE(0, "capsensor_cal_offset chan_data_sum[%d] = %d", i, chan_data_sum[i]);
        CAPSENSOR_TRACE(0, "capsensor_cal_offset cal_data_sum[%d] = %d", i, cal_data_sum[i]);

        if(cal_count == 0)
        {
            cal_data_min[i] = chan_data_sum[i];
            CAPSENSOR_TRACE(0, "capsensor_cal_offset cal_data_min[%d] = %d", i, cal_data_min[i]);
        }

        if(chan_data_sum[i] > cal_data_max[i])
        {
            cal_data_max[i] = chan_data_sum[i];
            CAPSENSOR_TRACE(0, "capsensor_cal_offset cal_data_max[%d] = %d", i, cal_data_max[i]);
        }

        if(chan_data_sum[i] < cal_data_min[i])
        {
            cal_data_min[i] = chan_data_sum[i];
            CAPSENSOR_TRACE(0, "capsensor_cal_offset cal_data_min[%d] = %d", i, cal_data_min[i]);
        }
    }

    if(cal_count >= 9)
    {
        cal_offset_flag = false;

        for(i = 0; i < CAP_CHNUM; i++)
        {
            cal_data_offset_avg[i] = cal_data_sum[i] / 10;
            CAPSENSOR_TRACE(0, "capsensor_cal_offset cal_data_offset_avg[%d] = %d", i, cal_data_offset_avg[i]);
            cal_data_sum[i] = 0;
        }

#if defined(CAPSENSOR_WEAR)
        wear_cal_data.wear_offset0 = cal_data_offset_avg[cap_wear_config.wear_detect_channel_0] - cal_data_offset_avg[cap_wear_config.wear_reference_channel_0];
        wear_cal_data.wear_offset1 = cal_data_offset_avg[cap_wear_config.wear_detect_channel_1] - cal_data_offset_avg[cap_wear_config.wear_reference_channel_1];
        CAPSENSOR_TRACE(0, "capsensor_cal_offset wear_offset0 = %d", wear_cal_data.wear_offset0);
        CAPSENSOR_TRACE(0, "capsensor_cal_offset wear_offset1 = %d", wear_cal_data.wear_offset1);
        wear_cal_data.wear_noise0  = cal_data_max[cap_wear_config.wear_detect_channel_0] - cal_data_min[cap_wear_config.wear_detect_channel_0];
        wear_cal_data.wear_noise1  = cal_data_max[cap_wear_config.wear_detect_channel_1] - cal_data_min[cap_wear_config.wear_detect_channel_1];
        CAPSENSOR_TRACE(0, "capsensor_cal_offset wear_max0 = %d", cal_data_max[cap_wear_config.wear_detect_channel_0]);
        CAPSENSOR_TRACE(0, "capsensor_cal_offset wear_max1 = %d", cal_data_max[cap_wear_config.wear_detect_channel_1]);
        CAPSENSOR_TRACE(0, "capsensor_cal_offset wear_min0 = %d", cal_data_min[cap_wear_config.wear_detect_channel_0]);
        CAPSENSOR_TRACE(0, "capsensor_cal_offset wear_min1 = %d", cal_data_min[cap_wear_config.wear_detect_channel_1]);
        CAPSENSOR_TRACE(0, "capsensor_cal_offset wear_noise0 = %d", wear_cal_data.wear_noise0);
        CAPSENSOR_TRACE(0, "capsensor_cal_offset wear_noise1 = %d", wear_cal_data.wear_noise1);
#endif

#if defined(CAPSENSOR_TOUCH)
#ifdef CAPSENSOR_SLIDE
        touch_cal_data.touch_noise0 = cal_data_max[cap_touch_config.slide_channel_0] - cal_data_min[cap_touch_config.slide_channel_0];
        touch_cal_data.touch_noise1 = cal_data_max[cap_touch_config.slide_channel_1] - cal_data_min[cap_touch_config.slide_channel_1];
        touch_cal_data.touch_noise2 = cal_data_max[cap_touch_config.slide_channel_2] - cal_data_min[cap_touch_config.slide_channel_2];
        CAPSENSOR_TRACE(0, "capsensor_cal_offset touch_max0 = %d", cal_data_max[cap_touch_config.slide_channel_0]);
        CAPSENSOR_TRACE(0, "capsensor_cal_offset touch_max1 = %d", cal_data_max[cap_touch_config.slide_channel_1]);
        CAPSENSOR_TRACE(0, "capsensor_cal_offset touch_max2 = %d", cal_data_max[cap_touch_config.slide_channel_2]);
        CAPSENSOR_TRACE(0, "capsensor_cal_offset touch_min0 = %d", cal_data_min[cap_touch_config.slide_channel_0]);
        CAPSENSOR_TRACE(0, "capsensor_cal_offset touch_min1 = %d", cal_data_min[cap_touch_config.slide_channel_1]);
        CAPSENSOR_TRACE(0, "capsensor_cal_offset touch_min2 = %d", cal_data_min[cap_touch_config.slide_channel_2]);
        CAPSENSOR_TRACE(0, "capsensor_cal_offset touch_noise0 = %d", touch_cal_data.touch_noise0);
        CAPSENSOR_TRACE(0, "capsensor_cal_offset touch_noise1 = %d", touch_cal_data.touch_noise1);
        CAPSENSOR_TRACE(0, "capsensor_cal_offset touch_noise2 = %d", touch_cal_data.touch_noise2);
#else
        touch_cal_data.touch_noise0 = cal_data_max[cap_touch_config.slide_channel_0] - cal_data_min[cap_touch_config.slide_channel_0];
        CAPSENSOR_TRACE(0, "capsensor_cal_offset touch_max0 = %d", cal_data_max[cap_touch_config.slide_channel_0]);
        CAPSENSOR_TRACE(0, "capsensor_cal_offset touch_min0 = %d", cal_data_min[cap_touch_config.slide_channel_0]);
        CAPSENSOR_TRACE(0, "capsensor_cal_offset touch_noise0 = %d", touch_cal_data.touch_noise0);
#endif
#endif
    }
}

static void capsensor_cal_diff(struct capsensor_sample_data * data, int len)
{
    uint32_t i = 0;
    uint32_t chan_data_sum[CAP_CHNUM*CAP_REPNUM];
    calcuate_ch_adc_sum(data, CAP_CHNUM, chan_data_sum, len);
    for(i = 0; i < CAP_CHNUM; i++)
    {
        cal_data_sum[i] += chan_data_sum[i];
        CAPSENSOR_TRACE(0, "capsensor_cal_diff chan_data_sum[%d] = %d", i, chan_data_sum[i]);
    }

    if(cal_count >= 9)
    {
        cal_diff_flag = false;

        for(i = 0; i < CAP_CHNUM; i++)
        {
            cal_data_diff_avg[i] = cal_data_sum[i] / 10;
            CAPSENSOR_TRACE(0, "capsensor_cal_diff cal_data_diff_avg[%d] = %d", i, cal_data_diff_avg[i]);
        }

#if defined(CAPSENSOR_WEAR)
        wear_cal_data.wear_snr_result = 1;
        wear_cal_data.wear_diff_result = 1;

        wear_cal_data.wear_diff0 = cal_data_diff_avg[cap_wear_config.wear_detect_channel_0] - cal_data_diff_avg[cap_wear_config.wear_reference_channel_0] - wear_cal_data.wear_offset0;
        wear_cal_data.wear_diff1 = cal_data_diff_avg[cap_wear_config.wear_detect_channel_1] - cal_data_diff_avg[cap_wear_config.wear_reference_channel_1] - wear_cal_data.wear_offset1;
        CAPSENSOR_TRACE(0, "capsensor_cal_diff wear_diff0 = %d", wear_cal_data.wear_diff0);
        CAPSENSOR_TRACE(0, "capsensor_cal_diff wear_diff1 = %d", wear_cal_data.wear_diff1);

        if(!wear_cal_data.wear_noise0 || !wear_cal_data.wear_noise1)
        {
            CAPSENSOR_TRACE(0, "capsensor_cal_diff wear noise = 0, return");
            wear_cal_data.wear_snr_result = 0;
            return;
        }

        wear_cal_data.wear_snr0 = wear_cal_data.wear_diff0 / wear_cal_data.wear_noise0;
        wear_cal_data.wear_snr1 = wear_cal_data.wear_diff1 / wear_cal_data.wear_noise1;
        CAPSENSOR_TRACE(0, "capsensor_cal_diff wear_snr0 = %d", wear_cal_data.wear_snr0);
        CAPSENSOR_TRACE(0, "capsensor_cal_diff wear_snr1 = %d", wear_cal_data.wear_snr1);

#if (CHIP_CAPSENSOR_VER < 1)
        if(wear_cal_data.wear_snr0 > wear_cal_range.wear_snr0_down || wear_cal_data.wear_snr0 > wear_cal_range.wear_snr1_down)
#else
        if(wear_cal_data.wear_snr0 < wear_cal_range.wear_snr0_down || wear_cal_data.wear_snr0 < wear_cal_range.wear_snr1_down)
#endif
        {
            wear_cal_data.wear_snr_result = 0;
            CAPSENSOR_TRACE(0, "capsensor_cal_diff wear_snr_result = %d", wear_cal_data.wear_snr_result);
        }

        if(wear_cal_data.wear_diff0 > wear_cal_range.wear_diff0_up || wear_cal_data.wear_diff0 < wear_cal_range.wear_diff0_down)
        {
            wear_cal_data.wear_diff_result = 0;
            CAPSENSOR_TRACE(0, "capsensor_cal_diff wear_diff0_result = %d", wear_cal_data.wear_diff_result);
        }

        if(wear_cal_data.wear_diff1 > wear_cal_range.wear_diff1_up || wear_cal_data.wear_diff1 < wear_cal_range.wear_diff1_down)
        {
            wear_cal_data.wear_diff_result = 0;
            CAPSENSOR_TRACE(0, "capsensor_cal_diff wear_diff1_result = %d", wear_cal_data.wear_diff_result);
        }
#endif

#if defined(CAPSENSOR_TOUCH)
        touch_cal_data.touch_snr_result = 1;
        touch_cal_data.touch_diff_result = 1;
#ifdef CAPSENSOR_SLIDE
        touch_cal_data.touch_diff0 = cal_data_diff_avg[cap_touch_config.slide_channel_0] - cal_data_offset_avg[cap_touch_config.slide_channel_0];
        touch_cal_data.touch_diff1 = cal_data_diff_avg[cap_touch_config.slide_channel_1] - cal_data_offset_avg[cap_touch_config.slide_channel_1];
        touch_cal_data.touch_diff2 = cal_data_diff_avg[cap_touch_config.slide_channel_2] - cal_data_offset_avg[cap_touch_config.slide_channel_2];
        CAPSENSOR_TRACE(0, "capsensor_cal_diff touch_diff0 = %d", touch_cal_data.touch_diff0);
        CAPSENSOR_TRACE(0, "capsensor_cal_diff touch_diff1 = %d", touch_cal_data.touch_diff1);
        CAPSENSOR_TRACE(0, "capsensor_cal_diff touch_diff2 = %d", touch_cal_data.touch_diff2);

        if(!touch_cal_data.touch_noise0 || !touch_cal_data.touch_noise1 || !touch_cal_data.touch_noise2)
        {
            CAPSENSOR_TRACE(0, "capsensor_cal_diff touch noise = 0, return");
            touch_cal_data.touch_snr_result = 0;
            return;
        }

        touch_cal_data.touch_snr0 = touch_cal_data.touch_diff0 / touch_cal_data.touch_noise0;
        touch_cal_data.touch_snr1 = touch_cal_data.touch_diff1 / touch_cal_data.touch_noise1;
        touch_cal_data.touch_snr2 = touch_cal_data.touch_diff2 / touch_cal_data.touch_noise2;
        CAPSENSOR_TRACE(0, "capsensor_cal_diff touch_snr0 = %d", touch_cal_data.touch_snr0);
        CAPSENSOR_TRACE(0, "capsensor_cal_diff touch_snr1 = %d", touch_cal_data.touch_snr1);
        CAPSENSOR_TRACE(0, "capsensor_cal_diff touch_snr2 = %d", touch_cal_data.touch_snr2);

        if(touch_cal_data.touch_snr0 > touch_cal_range.touch_snr0_down || touch_cal_data.touch_snr1 > touch_cal_range.touch_snr1_down || touch_cal_data.touch_snr2 > touch_cal_range.touch_snr2_down)
        {
            touch_cal_data.touch_snr_result = 0;
            CAPSENSOR_TRACE(0, "capsensor_cal_diff touch_snr_result = %d", touch_cal_data.touch_snr_result);
        }

        if(touch_cal_data.touch_diff0 > touch_cal_range.touch_diff0_up || touch_cal_data.touch_diff0 < touch_cal_range.touch_diff0_down)
        {
            touch_cal_data.touch_diff_result = 0;
            CAPSENSOR_TRACE(0, "capsensor_cal_diff touch_diff0_result = %d", touch_cal_data.touch_diff_result);
        }

        if(touch_cal_data.touch_diff1 > touch_cal_range.touch_diff1_up || touch_cal_data.touch_diff1 < touch_cal_range.touch_diff1_down)
        {
            touch_cal_data.touch_diff_result = 0;
            CAPSENSOR_TRACE(0, "capsensor_cal_diff touch_diff1_result = %d", touch_cal_data.touch_diff_result);
        }

        if(touch_cal_data.touch_diff2 > touch_cal_range.touch_diff2_up || touch_cal_data.touch_diff2 < touch_cal_range.touch_diff2_down)
        {
            touch_cal_data.touch_diff_result = 0;
            CAPSENSOR_TRACE(0, "capsensor_cal_diff touch_diff2_result = %d", touch_cal_data.touch_diff_result);
        }
#else
        touch_cal_data.touch_diff0 = cal_data_diff_avg[cap_touch_config.slide_channel_0] - cal_data_offset_avg[cap_touch_config.slide_channel_0];
        CAPSENSOR_TRACE(0, "capsensor_cal_diff touch_diff0 = %d", touch_cal_data.touch_diff0);

        if(!touch_cal_data.touch_noise0)
        {
            CAPSENSOR_TRACE(0, "capsensor_cal_diff touch noise = 0, return");
            touch_cal_data.touch_snr_result = 0;
            return;
        }

        touch_cal_data.touch_snr0 = touch_cal_data.touch_diff0 / touch_cal_data.touch_noise0;
        CAPSENSOR_TRACE(0, "capsensor_cal_diff touch_snr0 = %d", touch_cal_data.touch_snr0);

        if(touch_cal_data.touch_snr0 > touch_cal_range.touch_snr0_down)
        {
            touch_cal_data.touch_snr_result = 0;
            CAPSENSOR_TRACE(0, "capsensor_cal_diff touch_snr0_result = %d", touch_cal_data.touch_snr_result);
        }

        if(touch_cal_data.touch_diff0 > touch_cal_range.touch_diff0_up || touch_cal_data.touch_diff0 < touch_cal_range.touch_diff0_down)
        {
            touch_cal_data.touch_diff_result = 0;
            CAPSENSOR_TRACE(0, "capsensor_cal_diff touch_diff0_result = %d", touch_cal_data.touch_diff_result);
        }
#endif
#endif

        for(i = 0; i < CAP_CHNUM; i++)
        {
            cal_data_sum[i] = 0;
            cal_data_offset_avg[i] = 0;
            cal_data_diff_avg[i] = 0;
            cal_data_max[i] = 0;
            cal_data_min[i] = 0;
        }
    }
}

#if defined(CAPSENSOR_WEAR)
void capsensor_set_wear_cal_range(capsensor_wear_cal_range* range)
{
    wear_cal_range.wear_offset_up = range->wear_offset_up;
    wear_cal_range.wear_offset_down = range->wear_offset_down;
    wear_cal_range.wear_diff0_up = range->wear_diff0_up;
    wear_cal_range.wear_diff0_down = range->wear_diff0_down;
    wear_cal_range.wear_diff1_up = range->wear_diff1_up;
    wear_cal_range.wear_diff1_down = range->wear_diff1_down;
    wear_cal_range.wear_snr0_down = range->wear_snr0_down;
    wear_cal_range.wear_snr1_down = range->wear_snr1_down;
}

void capsensor_get_wear_cal_data(capsensor_wear_cal_data* data)
{
    data->wear_offset0  = wear_cal_data.wear_offset0;
    data->wear_offset1  = wear_cal_data.wear_offset1;
    data->wear_noise0 = wear_cal_data.wear_noise0;
    data->wear_noise1 = wear_cal_data.wear_noise1;
    data->wear_diff0 = wear_cal_data.wear_diff0;
    data->wear_diff1 = wear_cal_data.wear_diff1;
    data->wear_snr0 = wear_cal_data.wear_snr0;
    data->wear_snr1 = wear_cal_data.wear_snr1;
    data->wear_snr_result = wear_cal_data.wear_snr_result;
    data->wear_diff_result  = wear_cal_data.wear_diff_result;
}
#endif

#if defined(CAPSENSOR_TOUCH)
void capsensor_set_touch_cal_range(capsensor_touch_cal_range* range)
{
#ifdef CAPSENSOR_SLIDE
    touch_cal_range.touch_diff0_up = range->touch_diff0_up;
    touch_cal_range.touch_diff0_down = range->touch_diff0_down;
    touch_cal_range.touch_diff1_up = range->touch_diff1_up;
    touch_cal_range.touch_diff1_down = range->touch_diff1_down;
    touch_cal_range.touch_diff2_up = range->touch_diff2_up;
    touch_cal_range.touch_diff2_down = range->touch_diff2_down;
    touch_cal_range.touch_snr0_down = range->touch_snr0_down;
    touch_cal_range.touch_snr1_down = range->touch_snr1_down;
    touch_cal_range.touch_snr2_down = range->touch_snr2_down;
#else
    touch_cal_range.touch_diff0_up = range->touch_diff0_up;
    touch_cal_range.touch_diff0_down = range->touch_diff0_down;
    touch_cal_range.touch_snr0_down = range->touch_snr0_down;
#endif
}

void capsensor_get_touch_cal_data(capsensor_touch_cal_data* data)
{
#ifdef CAPSENSOR_SLIDE
    data->touch_diff0 = touch_cal_data.touch_diff0;
    data->touch_diff1 = touch_cal_data.touch_diff1;
    data->touch_diff2 = touch_cal_data.touch_diff2;
    data->touch_noise0 = touch_cal_data.touch_noise0;
    data->touch_noise1 = touch_cal_data.touch_noise1;
    data->touch_noise2 = touch_cal_data.touch_noise2;
    data->touch_snr0 = touch_cal_data.touch_snr0;
    data->touch_snr1 = touch_cal_data.touch_snr1;
    data->touch_snr2 = touch_cal_data.touch_snr2;
#else
    data->touch_diff0 = touch_cal_data.touch_diff0;
    data->touch_noise0 = touch_cal_data.touch_noise0;
    data->touch_snr0 = touch_cal_data.touch_snr0;
#endif
    data->touch_snr_result = touch_cal_data.touch_snr_result;
    data->touch_diff_result = touch_cal_data.touch_diff_result;
}
#endif

void capsensor_factory_calculate_flag_set(bool m_cal_offset_flag, bool m_cal_diff_flag)
{
    cal_offset_flag = m_cal_offset_flag;
    cal_diff_flag = m_cal_diff_flag;
}

void capsensor_factory_calculate(struct capsensor_sample_data * data, int len)
{
    if(cal_offset_flag)
    {
        capsensor_cal_offset(data, len);
        CAPSENSOR_TRACE(0, "capsensor_cal_offset cal_count = %d", cal_count);
        if(cal_count >= 9)
        {
            cal_count = 0;
        }
        else
        {
            cal_count++;
        }
    }

    if(cal_diff_flag)
    {
        CAPSENSOR_TRACE(0, "capsensor_cal_diff cal_count = %d", cal_count);
        capsensor_cal_diff(data, len);
        if(cal_count >= 9)
        {
            cal_count = 0;
        }
        else
        {
            cal_count++;
        }
    }
}

#ifdef BESUI_CAPSENSOR_FACTORY_EN
void nv_record_capsensor_clear_calib(void)
{
    struct nvrecord_env_t *nvrecord_env;
    nv_record_env_get(&nvrecord_env);
    memset(nvrecord_env->sar_value, 0, sizeof(nvrecord_env->sar_value));
    memset(nvrecord_env->baseline_value_p, 0, sizeof(nvrecord_env->baseline_value_p));
    memset(nvrecord_env->baseline_value_n, 0, sizeof(nvrecord_env->baseline_value_n));
    memset(nvrecord_env->wear_offset, 0, sizeof(nvrecord_env->wear_offset));

    nv_record_env_set(nvrecord_env);
}

void nv_record_get_capsensor(void)
{
    struct nvrecord_env_t *nvrecord_env;
    nv_record_env_get(&nvrecord_env);
#if defined(CHIP_BEST1306)
    capsensor_baseline_sar_reg_read(nvrecord_env->sar_value, nvrecord_env->baseline_value_p, nvrecord_env->baseline_value_n);
#elif defined(CHIP_BEST1306P)
    capsensor_driver_baseline_reg_read(nvrecord_env->baseline_value_p, nvrecord_env->baseline_value_n);
#endif

    nv_record_env_set(nvrecord_env);
}

void nv_record_wear_offset(int wear_offset0, int wear_offset1)
{
    struct nvrecord_env_t *nvrecord_env;
    nv_record_env_get(&nvrecord_env);
    nvrecord_env->wear_offset[0] = wear_offset0;
    nvrecord_env->wear_offset[1] =wear_offset1;
    nv_record_env_set(nvrecord_env);
    BESUI_TRACE(1, "[UICAP]%s", __func__);
}

void nv_record_set_capsensor_offset(void)
{
    struct nvrecord_env_t *nvrecord_env;
    nv_record_env_get(&nvrecord_env);
    if(nvrecord_env)
    {
        if((nvrecord_env->sar_value[0] != 0) && (nvrecord_env->sar_value[0] != 0xFFFFFFFF))
        {
            BESUI_TRACE(1, "[UICAP]%s", __func__);
#if defined(CHIP_BEST1306)
            capsensor_baseline_sar_dr(nvrecord_env->sar_value, nvrecord_env->baseline_value_p, nvrecord_env->baseline_value_n);
#elif defined(CHIP_BEST1306P)
            capsensor_driver_baseline_dr(nvrecord_env->baseline_value_p, nvrecord_env->baseline_value_n);
#endif
#if defined(CAPSENSOR_WEAR)
            capsensor_set_offset(nvrecord_env->wear_offset[0], nvrecord_env->wear_offset[1]);
            capsensor_set_calculate_flag(false);
#endif
        }
    }
}
#if defined(CAPSENSOR_WEAR)
void capsensor_set_wear_cal_range_buf(int *dat)
{
    wear_cal_range.wear_offset_up = dat[0];
    wear_cal_range.wear_offset_down = -dat[1];
    wear_cal_range.wear_diff0_up = dat[2];
    wear_cal_range.wear_diff0_down = dat[3];
    wear_cal_range.wear_diff1_up = dat[4];
    wear_cal_range.wear_diff1_down = dat[5];
    wear_cal_range.wear_snr0_down = -dat[6];
    wear_cal_range.wear_snr1_down = -dat[7];

    BESUI_TRACE(0,"[UIHFP]%s, wear_offset_up = %d", __func__, wear_cal_range.wear_offset_up);
    BESUI_TRACE(0,"[UIHFP]%s, wear_offset_down = %d", __func__, wear_cal_range.wear_offset_down);
    BESUI_TRACE(0,"[UIHFP]%s, wear_diff0_up = %d", __func__, (int)wear_cal_range.wear_diff0_up);
    BESUI_TRACE(0,"[UIHFP]%s, wear_diff0_down = %d", __func__, (int)wear_cal_range.wear_diff0_down);
    BESUI_TRACE(0,"[UIHFP]%s, wear_diff1_up = %d", __func__, (int)wear_cal_range.wear_diff1_up);
    BESUI_TRACE(0,"[UIHFP]%s, wear_diff1_down = %d", __func__, (int)wear_cal_range.wear_diff1_down);
    BESUI_TRACE(0,"[UIHFP]%s, wear_snr0_down = %d", __func__, wear_cal_range.wear_snr0_down);
    BESUI_TRACE(0,"[UIHFP]%s, wear_snr1_down = %d", __func__, wear_cal_range.wear_snr1_down);
}
#endif
#ifdef CAPSENSOR_SLIDE
void capsensor_set_touch_cal_range_buf(capsensor_touch_cal_range *range)
{
    touch_cal_range.touch_diff0_up = range->touch_diff0_up;
    touch_cal_range.touch_diff0_down = range->touch_diff0_down;
    touch_cal_range.touch_diff1_up = range->touch_diff1_up;
    touch_cal_range.touch_diff1_down = range->touch_diff1_down;
    touch_cal_range.touch_diff2_up = range->touch_diff2_up;
    touch_cal_range.touch_diff2_down = range->touch_diff2_down;
    touch_cal_range.touch_snr0_down = range->touch_snr0_down;
    touch_cal_range.touch_snr1_down = range->touch_snr1_down;
    touch_cal_range.touch_snr2_down = range->touch_snr2_down;
    BESUI_TRACE(0,"[UICAP]%s, touch_diff0_up = %d", __func__, (int)touch_cal_range.touch_diff0_up);
    BESUI_TRACE(0,"[UICAP]%s, touch_diff0_down = %d", __func__, (int)touch_cal_range.touch_diff0_down);
    BESUI_TRACE(0,"[UICAP]%s, touch_snr0_down = %d", __func__, (int)touch_cal_range.touch_snr0_down);
}
#else
void capsensor_set_touch_cal_range_buf(int *range)
{
    touch_cal_range.touch_diff0_up = range[0];
    touch_cal_range.touch_diff0_down = range[1];
    touch_cal_range.touch_snr0_down = -range[2];

    BESUI_TRACE(0,"[UICAP]%s, touch_diff0_up = %d", __func__, (int)touch_cal_range.touch_diff0_up);
    BESUI_TRACE(0,"[UICAP]%s, touch_diff0_down = %d", __func__, (int)touch_cal_range.touch_diff0_down);
    BESUI_TRACE(0,"[UICAP]%s, touch_snr0_down = %d", __func__, (int)touch_cal_range.touch_snr0_down);
}
#endif

uint8_t dat_is_bit(int dat)
{
    uint8_t len = 0;
    if(dat < 10)
        len = 1;
    else if(dat < 100)
        len = 2;
    else if(dat < 1000)
        len = 3;
    else if(dat < 10000)
        len = 4;
    else if(dat < 100000)
        len = 5;
    else if(dat < 1000000)
        len = 6;
    return len;
}

void capsensor_diff_return(void)
{
    //diff test end
    uint8_t i = 0;
    int dats = 0;
    uint8_t datlen = 0;
    POSSIBLY_UNUSED uint8_t cmd_data[100] = {0};
    int capdat[15] = {0};
#if defined(CAPSENSOR_WEAR)
    capdat[0] = abs(wear_cal_data.wear_offset0);
    capdat[1] = abs(wear_cal_data.wear_offset1);
    capdat[2] = abs(wear_cal_data.wear_noise0);
    capdat[3] = abs(wear_cal_data.wear_noise1);
    capdat[4] = abs(wear_cal_data.wear_diff0);
    capdat[5] = abs(wear_cal_data.wear_diff1);
    capdat[6] = abs(wear_cal_data.wear_snr0);
    capdat[7] = abs(wear_cal_data.wear_snr1);

    capdat[8] = abs(wear_cal_data.wear_snr_result);
    capdat[9] = abs(wear_cal_data.wear_diff_result);
#endif
    capdat[10] = abs(touch_cal_data.touch_diff0);
    capdat[11] = abs(touch_cal_data.touch_noise0);
    capdat[12] = abs(touch_cal_data.touch_snr0);

    capdat[13] = abs(touch_cal_data.touch_snr_result);
    capdat[14] = abs(touch_cal_data.touch_diff_result);

    for(i = 0;i < 15;i ++)
    {
        datlen = dat_is_bit(capdat[i]);
        BESUI_TRACE(0, "datlen = %d", datlen);

        if(datlen == 1)
        {
            cmd_data[dats] = capdat[i] + '0';
        }
        else if(datlen == 2)
        {
            cmd_data[dats] = capdat[i]/10+'0';
            cmd_data[++dats] = capdat[i]%10+'0';
        }
        else if(datlen == 3)
        {
            cmd_data[dats] = capdat[i]/100+'0';
            cmd_data[++dats] = capdat[i]%100/10+'0';
            cmd_data[++dats] = capdat[i]%100%10+'0';
        }
        else if(datlen == 4)
        {
            cmd_data[dats] = capdat[i]/1000+'0';
            cmd_data[++dats] = capdat[i]%1000/100+'0';
            cmd_data[++dats] = capdat[i]%1000%100/10+'0';
            cmd_data[++dats] = capdat[i]%1000%100%10+'0';
        }
        else if(datlen == 5)
        {
            cmd_data[dats] = capdat[i]/10000+'0';
            cmd_data[++dats] = capdat[i]%10000/1000+'0';
            cmd_data[++dats] = capdat[i]%10000%1000/100+'0';
            cmd_data[++dats] = capdat[i]%10000%1000%100/10+'0';
            cmd_data[++dats] = capdat[i]%10000%1000%100%10+'0';
        }
        else if(datlen == 6)
        {
            cmd_data[dats] = capdat[i]/100000+'0';
            cmd_data[++dats] = capdat[i]%100000/10000+'0';
            cmd_data[++dats] = capdat[i]%100000%10000/1000+'0';
            cmd_data[++dats] = capdat[i]%100000%10000%1000/100+'0';
            cmd_data[++dats] = capdat[i]%100000%1000%1000%100/10+'0';
            cmd_data[++dats] = capdat[i]%100000%1000%1000%100%10+'0';
        }
        if(i < 14)
            cmd_data[++dats] = ',';
        dats++;
    }
    BESUI_TRACE(0, "dats = %d", dats);
#ifdef BESUI_TWS_EN
    app_at_cmd_at_cap_diff_return(dats, cmd_data);
#endif 
}

void capsensor_restart(bool type)
{
#if !defined(CHIP_BEST1501P) && !defined(CHIP_BEST1502X) && !defined(CHIP_BEST1501)
    nv_record_capsensor_clear_calib();
    if(type == true)
    {
        osDelay(3000);
    }
    capsensor_close();
    osDelay(15);
    capsensor_open();
    osDelay(100);

    // capsensor_set_sdm_init_flag(true);
    osDelay(100);
    
    nv_record_get_capsensor();
    BESUI_TRACE(0, "[UICAP]%s", __func__);
#endif
}

#if defined(CHIP_BEST1501P)
#include "analog.h"
void analog_capsensor_close(void)
{
    // uint16_t val = 0;
    // analog_read(ANA_REG_17C, &val);
    // val &= ~ (REG_DIG_LVLSHIFT_0P40P8_EN | REG_DIG_LVLSHIFT_0P81P8_EN);
    // analog_inital_write(ANA_REG_17C, val);
}
#endif


#endif //#ifdef BESUI_CAPSENSOR_FACTORY_EN

#endif