/***************************************************************************
 *
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
 *
 ****************************************************************************/
#ifndef CHIP_SUBSYS_SENS
#include "app_capsensor.h"
#include "stdint.h"
#include "touch_wear_core.h"
#include "plat_addr_map.h"
#include CHIP_SPECIFIC_HDR(capsensor_driver)
#include "capsensor_driver.h"
#if defined(CAPSENSOR_AT_SENS)
#include "app_sensor_hub.h"
#endif
#include "hal_trace.h"
#include "app_key.h"
#include "stdint.h"
#include "string.h"
#ifdef BES_OTA
#include "ota_control.h"
#endif
#ifdef BESUI_TWS_EN
#include "app_factory_bt.h"
#endif
#ifdef BESUI_BTMSG_EN
#include "besui_common.h"
#include "twsui_btmsg.h"
#endif
#include "hal_key.h"

#ifdef BESUI_APP_EN
#include "app_tota.h"
#include "apps.h"
#include "besui_common.h"
#ifdef APP_TOUCH_ONOFF_EN
#include "nvrecord_extension.h"
#include "nvrecord_env.h"
#endif

static bool inear_detect_status =false;
void app_inear_detect_set_on_ear(void)
{
    inear_detect_status = true;
    CAPSENSOR_TRACE(1, "[UICAP]%s, ear = %d\n", __func__, inear_detect_status);
}
void app_inear_detect_set_off_ear(void)
{
    inear_detect_status = false;
    CAPSENSOR_TRACE(1, "[UICAP]%s, ear = %d\n", __func__, inear_detect_status);
}
bool app_inear_get_status(void)
{
    CAPSENSOR_TRACE(1, "[UICAP]%s, ear = %d\n", __func__, inear_detect_status);
    return inear_detect_status;
}

bool besui_ear_sta_get(bool param)
{
    CAPSENSOR_TRACE(1, "[UICAP]%s, ear_sta = %d, %d\n", __func__, uictl.ear_current_sta, uictl.ear_another_sta);    
    if(param == false)
        return uictl.ear_current_sta;
    else
        return uictl.ear_another_sta;
}

#endif //#ifdef BESUI_APP_EN

void app_capsensor_click_event(uint8_t key_event);

#if defined(CAPSENSOR_AT_SENS)
static void app_mcu_sensor_hub_transmit_touch_no_rsp_cmd_handler(uint8_t* ptr, uint16_t len)
{
    app_core_bridge_send_data_without_waiting_rsp(MCU_SENSOR_HUB_TASK_CMD_TOUCH_REQ_NO_RSP, ptr, len);
}

static void app_mcu_sensor_hub_touch_no_rsp_cmd_received_handler(uint8_t* ptr, uint16_t len)
{
    APP_KEY_STATUS status = *((APP_KEY_STATUS *)ptr);
    CAPSENSOR_TRACE(0, "Get touch no rsp command from sensor hub core, status.event: %d", status.event);
    app_capsensor_click_event(status.event);
}

static void app_mcu_sensor_hub_touch_no_rsp_cmd_tx_done_handler(uint16_t cmdCode,
    uint8_t* ptr, uint16_t len)
{
    CAPSENSOR_TRACE(0, "cmdCode 0x%x tx done", cmdCode);
}

CORE_BRIDGE_TASK_COMMAND_TO_ADD(MCU_SENSOR_HUB_TASK_CMD_TOUCH_REQ_NO_RSP,
                                "touch no rsp req to sensor hub core",
                                app_mcu_sensor_hub_transmit_touch_no_rsp_cmd_handler,
                                app_mcu_sensor_hub_touch_no_rsp_cmd_received_handler,
                                0,
                                NULL,
                                NULL,
                                app_mcu_sensor_hub_touch_no_rsp_cmd_tx_done_handler);
#endif

/***************************************************************************
 * @brief app_capsensor_click_event function
 *
 * @param key_event touch or wear event
 ***************************************************************************/
void app_capsensor_click_event(uint8_t key_event)
{
    APP_KEY_STATUS status;

    status.event = key_event;

#ifdef BES_OTA
    if(ota_is_in_progress())
    {
        CAPSENSOR_TRACE(0,"[%s], OTA in progress...",__func__);
        return;
    }
#endif

#if defined(CAPSENSOR_ENABLE) && (defined(BESUI_TWS_EN) || defined(BESUI_STEREO_EN))
    if(uictl.poweron_init_ok != 0xA5)
    {
        CAPSENSOR_TRACE(0,"[%s], capsensor can not run, system init is not ok",__func__);
        return;
    }
#endif
    
#ifdef APP_TOUCH_ONOFF_EN
    if(!nvrecord_uienv->touch_onoff)
    {
#if defined(CAPSENSOR_ENABLE)
        if(status.event != CAP_KEY_EVENT_ON_EAR && status.event != CAP_KEY_EVENT_OFF_EAR)
        {
            return;
        }
#endif
    }
#endif
	
#if defined(CAPSENSOR_ENABLE) && (defined(BESUI_TWS_EN) || defined(BESUI_STEREO_EN))
    if(uictl.capsensor_onoff == 0xA5)
    {
        CAPSENSOR_TRACE(0,"[%s], factory test need off capsensor!",__func__);
        return;
    }
#endif

#ifdef BESUI_TWS_EN
    if(bt_test_mode_dut_get())
    {
        CAPSENSOR_TRACE(0,"[%s], test mode no send app!",__func__);
        return;
    }
#endif

#if defined(BESUI_APP_EN) && defined(CAPSENSOR_WEAR)
    if(app_wear_det_onoff_get() == true) //if wear off
    {
        if(status.event == CAP_KEY_EVENT_ON_EAR)
        {
            app_inear_detect_set_on_ear();
            slave_ear_to_master();
            return;
        }
        else if(status.event == CAP_KEY_EVENT_OFF_EAR)
        {
            app_inear_detect_set_off_ear();
            slave_ear_to_master();
            return;            
        }
    }
#endif
    CAPSENSOR_TRACE(1, "[%s]:%d",  __func__, status.event);

#ifdef BESUI_TWS_EN
	if(status.event == CAP_KEY_EVENT_OFF_EAR || status.event == CAP_KEY_EVENT_ON_EAR)
            status.code = HAL_KEY_CODE_FN1;
	else
	      	status.code = HAL_KEY_CODE_PWR;
#endif

    switch (status.event)
    {
        case CAP_KEY_EVENT_OFF_EAR:
            CAPSENSOR_TRACE(1, "capsensor state= off ear\n");
            status.event = APP_KEY_EVENT_OFF_EAR;
#ifdef BESUI_APP_EN
            app_inear_detect_set_off_ear();
            app_tota_inear_status_event_report();
            uictl.ear_current_sta = false;
#endif
            break;

        case CAP_KEY_EVENT_ON_EAR:
            CAPSENSOR_TRACE(1, "capsensor state= on ear\n");
            status.event = APP_KEY_EVENT_ON_EAR;
#ifdef BESUI_APP_EN
            app_inear_detect_set_on_ear();
            app_tota_inear_status_event_report();
            uictl.ear_current_sta = true;
#endif
#if defined(WEAR_DETECT_PROMPT_EN)
#ifdef BESUI_TWS_EN
            besui_bt_msg_put(BT_MSG_SOMETHING_EVENT, USER_MSG_CMD_WEAR_PROMPT, 0);
#endif
#endif
            break;

        case CAP_KEY_EVENT_UP:
            CAPSENSOR_TRACE(1, "capsensor state= key up\n");
            status.event = APP_KEY_EVENT_UP;
            break;

        case CAP_KEY_EVENT_DOWN:
            CAPSENSOR_TRACE(1, "capsensor state= key down\n");
            status.event = APP_KEY_EVENT_DOWN;
            break;

        case CAP_KEY_EVENT_UPSLIDE:
            CAPSENSOR_TRACE(1, "capsensor state= up slide\n");
            status.event = APP_KEY_EVENT_UPSLIDE;
            break;

        case CAP_KEY_EVENT_DOWNSLIDE:
            CAPSENSOR_TRACE(1, "capsensor state= down slide\n");
            status.event = APP_KEY_EVENT_DOWNSLIDE;
            break;

        case CAP_KEY_EVENT_CLICK:
            CAPSENSOR_TRACE(1, "capsensor state= single click\n");
            status.event = APP_KEY_EVENT_CLICK;
            break;

        case CAP_KEY_EVENT_DOUBLECLICK:
            CAPSENSOR_TRACE(1, "capsensor state= double click\n");
            status.event = APP_KEY_EVENT_DOUBLECLICK;
            break;

        case CAP_KEY_EVENT_TRIPLECLICK:
            CAPSENSOR_TRACE(1, "capsensor state= triple click\n");
            status.event = APP_KEY_EVENT_TRIPLECLICK;
            break;

        case CAP_KEY_EVENT_ULTRACLICK:
            CAPSENSOR_TRACE(1, "capsensor state= ultra click\n");
            status.event = APP_KEY_EVENT_ULTRACLICK;
            break;

        case CAP_KEY_EVENT_RAMPAGECLICK:
            CAPSENSOR_TRACE(1, "capsensor state= rampage click\n");
            status.event = APP_KEY_EVENT_RAMPAGECLICK;
            break;

        case CAP_KEY_EVENT_SIXCLICK:
            CAPSENSOR_TRACE(1, "capsensor state= six click\n");
            break;

        case CAP_KEY_EVENT_SEVENCLICK:
            CAPSENSOR_TRACE(1, "capsensor state= seven click\n");
            break;

        case CAP_KEY_EVENT_LONGPRESS:
            CAPSENSOR_TRACE(1, "capsensor state= long press\n");
            status.event = APP_KEY_EVENT_LONGPRESS;
            break;

        case CAP_KEY_EVENT_LONGLONGPRESS:
            CAPSENSOR_TRACE(1, "capsensor state= long  long press\n");
            status.event = APP_KEY_EVENT_LONGLONGPRESS;
            break;

        case CAP_KEY_EVENT_CLICK_AND_LONGPRESS:
            CAPSENSOR_TRACE(1, "capsensor state= click and long press\n");
            break;

        case CAP_KEY_EVENT_CLICK_AND_LONGLONGPRESS:
            CAPSENSOR_TRACE(1, "capsensor state= click and longlongpress\n");
            break;

        case CAP_KEY_EVENT_DOUBLECLICK_AND_LONGLONGPRESS:
            CAPSENSOR_TRACE(1, "capsensor state= double click and longlongpress\n");
            break;

        case CAP_KEY_EVENT_TRIPLECLICK_AND_LONGLONGPRESS:
            CAPSENSOR_TRACE(1, "capsensor state= triple click and longlongpress\n");
            break;

        default:
            break;
    }
#ifdef BESUI_TWS_EN
    if(status.event != APP_KEY_EVENT_NONE)
    {
        CAPSENSOR_TRACE(1,"[UICAP]capsensor key event %d", key_event);
        if((status.event == APP_KEY_EVENT_ON_EAR)||(status.event == APP_KEY_EVENT_OFF_EAR))
        {
            besui_send_key_event(HAL_KEY_CODE_FN1, status.event);
        }
        else
        {
            besui_send_key_event(HAL_KEY_CODE_PWR, status.event);
        }
    }
#endif
}

/***************************************************************************
 * @brief  app_mcu_core_capsensor_init function
 *
 ***************************************************************************/
void app_mcu_core_capsensor_init(void)
{
    int ret = -1;
    ret = register_capsensor_click_event_callback(app_capsensor_click_event);
    if (ret) {
        CAPSENSOR_TRACE(1,"register_capsensor_click_event_callback failed:%d\n", ret);
    }

    capsensor_driver_init();

#ifdef CAPSENSOR_AT_MCU
    capsensor_sens2mcu_irq_set();
    cap_sensor_core_thread_init();
#endif
}

#endif

