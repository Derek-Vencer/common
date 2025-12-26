/*
 * charge_ntc.cpp
 *
 *  Created on: 2025年12月25日
 *      Author: zhangyijun
 */
#include <stdio.h>
#include <string.h>
#include "cmsis_os.h"
#include "cmsis.h"
#include "cqueue.h"
#include "pmu.h"
#include "app_battery.h"
#include "apps.h"
#include "hal_iomux_best1306p.h"
#include "plat_addr_map_best1306p.h"
#include "hal_gpio.h"
#include "hal_trace.h"
#include "charger_ntc.h"

#undef printf
#undef DBGPRINT

#define printf(fmt,...)     hal_trace_printf(2, fmt, ##__VA_ARGS__)
#define DBGPRINT(fmt,...)   hal_trace_printf(0, fmt, ##__VA_ARGS__)

#define NTC_SHAKE_CNT               5
#define NTC_TYPE_CHARGER_STOP       1
#define NTC_TYPE_CHARGER_20MA       2
#define NTC_TYPE_CHARGER_80MA       3
#define NTC_TYPE_POWEROFF           4

#define NTC_TEMPERATURE_45          5
#define NTC_TEMPERATURE_00          6
#define NTC_TEMPERATURE_VALID       7

#define NTC_IOMUX_GPIO_15             (HAL_IOMUX_PIN_P1_5)  //outPut

//Charge Box
const uint16_t ntc_temp_tab[]={
	  600,
	  1520, //45°
	  1650, //41°
	  3640, //4°
	  3880  //0°
};

osTimerId aw_ntc_open_process_timer = NULL;
static void aw_ntc_detect_timehandler(void const *param);

osTimerDef (AW_NTC_TIMER_NAME, (void (*)(void const *))aw_ntc_detect_timehandler);

static void aw_ntc_detect_volt_timer_onoff(bool timer_en)
{
	DBGPRINT("%s timer_en %d ", __func__, timer_en);

	if(aw_ntc_open_process_timer == NULL)
	{
		aw_ntc_open_process_timer = osTimerCreate(osTimer(AW_NTC_TIMER_NAME), osTimerOnce, NULL);
	}

	if(timer_en)
	{
		osTimerStart(aw_ntc_open_process_timer, 2000); //10*1000ms
	}
	else
	{
		osTimerStop(aw_ntc_open_process_timer);
	}
}

static void aw_ntc_detect_timehandler(void const *param)
{
	DBGPRINT("%s ", __func__);
    ntc_capture_start();
    aw_ntc_detect_volt_timer_onoff(true);
}

void aw_ntc_detect_init(void)
{
	DBGPRINT("%s ", __func__);
	struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux_ntc[] = {
        {HAL_IOMUX_PIN_P1_5, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE},
    };
    hal_iomux_init(pinmux_ntc, ARRAY_SIZE(pinmux_ntc));
    ntc_capture_open();
    aw_ntc_detect_volt_timer_onoff(true);
}


void aw_ntc_detect_process(uint16_t ad_volt)
{

    DBGPRINT("%s ntc_volt = %d", __func__, ad_volt);
    uint8_t ntc_current_type = 0;
    static uint8_t ntc_type  = 0;
    static uint8_t ntc_pre_type  = 0;
    static uint8_t shake_buf = 0;
    static bool ntc_start_process = false;

   if (ad_volt <= ntc_temp_tab[0]) {
	   ntc_current_type = NTC_TEMPERATURE_45;
   } else if (ad_volt >= ntc_temp_tab[3]) {
	   ntc_current_type = NTC_TEMPERATURE_00;
   } else {
	   ntc_current_type = NTC_TEMPERATURE_VALID;
   }

   if(ntc_current_type != ntc_pre_type)
   {
	   ntc_pre_type = ntc_current_type;
   }
   else
   {
	   shake_buf ++;
	   if(ntc_pre_type != ntc_type)
	   {
		   if(shake_buf >= NTC_SHAKE_CNT)
		   {
			   shake_buf = 0;
			   ntc_type = ntc_current_type;
			   ntc_start_process = true;
			   DBGPRINT("%s, ntc_type = %d", __func__, ntc_type);
		   }
	   }
   }
    if(ntc_start_process == true)
    {
        ntc_start_process = false;
        if(ntc_type == NTC_TEMPERATURE_45 || ntc_type == NTC_TEMPERATURE_00)
        {
            printf("%s, temp too high/low, poweroff!!!", __func__);
            app_shutdown();
        }
    }
}



