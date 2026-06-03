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
#include "cmsis_os.h"
#include "stdbool.h"
#include "hal_trace.h"
#include "app_pwl.h"
#include "string.h"

#ifdef BESUI_TWS_EN

#include "twsui_led.h"

#ifdef USER_LED_BREATH_EN
#include "pmu.h"
#endif

#ifdef USER_LED_CASE_OPEN_EN
#include "besui_common.h"
#endif

#include "twsui_comm.h"
#include "apps.h"

static APP_STATUS_INDICATION_T app_status = APP_STATUS_INDICATION_NUM;
static APP_STATUS_INDICATION_T app_status_ind_filter = APP_STATUS_INDICATION_NUM;
static APP_STATUS_INDICATION_T app_last_status = APP_STATUS_INDICATION_NUM;

static const char * const app_status_indication_str[] =
{
    "[POWERON]",
    "[INITIAL]",
    "[PAGESCAN]",
    "[POWEROFF]",
    "[CHARGENEED]",
    "[CHARGING]",
    "[FULLCHARGE]",
    /* repeatable status: */
    "[BOTHSCAN]",
    "[CONNECTING]",
    "[CONNECTED]",
    "[DISCONNECTED]",
    "[CALLNUMBER]",
    "[INCOMINGCALL]",
    "[PAIRSUCCEED]",
    "[PAIRFAIL]",
    "[HANGUPCALL]",
    "[REFUSECALL]",
    "[ANSWERCALL]",
    "[CLEARSUCCEED]",
    "[CLEARFAIL]",
    "[WARNING]",
    "[ALEXA_START]",
    "[ALEXA_STOP]",
    "[GSOUND_MIC_OPEN]",
    "[GSOUND_MIC_CLOSE]",
    "[GSOUND_NC]",
    "[INVALID]",
    "[MUTE]",
    "[TESTMODE]",
    "[TESTMODE1]",
    "[RING_WARNING]",
#ifdef __INTERACTION__	
    "[FINDME]",
#endif	
    "[MY_BUDS_FIND]",
    "[TILE_FIND]",
};


const char *status2str(uint16_t status)
{
    const char *str = NULL;

    if (status >= 0 && status < APP_STATUS_INDICATION_NUM)
    {
        str = app_status_indication_str[status];
    }
    else
    {
        str = "[UNKNOWN]";
    }

    return str;
}

int app_status_indication_filter_set(APP_STATUS_INDICATION_T status)
{
    app_status_ind_filter = status;
    return 0;
}

APP_STATUS_INDICATION_T app_status_indication_get(void)
{
    return app_status;
}


APP_STATUS_INDICATION_T app_last_status_indication_get(void)
{
    return app_last_status;
}
#ifdef BESUI_CHARGE_EN
extern bool besui_bat_charge_sta_get(void);
#endif
int app_status_indication_set(APP_STATUS_INDICATION_T status)
{
    struct APP_PWL_CFG_T cfg0;
    struct APP_PWL_CFG_T cfg1;

    BESUI_TRACE(2,"%s %d",__func__, status);

#ifdef USER_LED_CASE_OPEN_EN
    if(uictl.led_caseopen == true)
    {
        if(status != APP_STATUS_INDICATION_POWERON)
        {
            uictl.led_next = status;
            if((status != APP_STATUS_INDICATION_POWERON)&&(status != APP_STATUS_INDICATION_POWEROFF)
            &&(status != APP_STATUS_INDICATION_FULLCHARGE)&&(status != APP_STATUS_INDICATION_TESTMODE)
            &&(status != APP_STATUS_INDICATION_TESTMODE1)&&(status != APP_STATUS_INDICATION_CHARGING)
            &&(status != APP_STATUS_INDICATION_POWERON_BATTERY_DISPLAY)&&(status != APP_STATUS_INDICATION_POWERON_LOWBATTERY_DISPLAY)
            &&(status != APP_STATUS_INDICATION_PHONE_CONNECTED)&&(status != APP_STATUS_INDICATION_INITIAL)&&(status != APP_STATUS_INDICATION_CLEARPAIRLIST)
            &&(status != APP_STATUS_INDICATION_CHARGENEED))
            {
                app_last_status = status;
            }
            BESUI_TRACE(0,"[UILED]%s, uictl.led_next = %d", __func__, uictl.led_next);
            return 0;
        }
    }
#endif

#ifdef USER_LED_BREATH_EN
    static uint8_t breath_mode = 0;
    struct PMU_LED_BR_CFG_T pmu_led_breathing_cfg = {
        .off_time_ms = 500,
        .on_time_ms  = 400,
        .fade_time_ms = 600,
    };

    if(status == APP_STATUS_INDICATION_BOTHSCAN) //pair mode breath
    {
        if(!breath_mode)
        {
            breath_mode = 1;
            app_last_status = status;
            memset(&cfg0, 0, sizeof(struct APP_PWL_CFG_T));
            memset(&cfg1, 0, sizeof(struct APP_PWL_CFG_T));
            app_pwl_stop(APP_PWL_ID_0); //timer stop
            app_pwl_stop(APP_PWL_ID_1);
            pmu_led_breathing_enable(HAL_IOMUX_PIN_LED1, &pmu_led_breathing_cfg);//pin led1     
        }
        return 0;
    }
    else
    {
        if(breath_mode)
        {
            breath_mode = 0;
            pmu_led_breathing_disable(HAL_IOMUX_PIN_LED1);//pin led1
        }
    }
#endif

    static uint8_t clear_flag = 0;
    if(status == APP_STATUS_INDICATION_CLEARPAIRLIST)
    {
        if(!clear_flag)
            clear_flag = 1;
    }
    else
    {
        if(clear_flag)
        {
            BESUI_TRACE(1,"%s, APP_STATUS_INDICATION_CLEARPAIRLIST",__func__);
            return 0;            
        }
    }
    if (app_status == status)
        return 0;

    if (app_status_ind_filter == status)
        return 0;

    if(status == APP_STATUS_INDICATION_CHARGING)
    {
        BESUI_TRACE(1, "[UILED]%s, CHARGING is not need", __func__);
        return 0;
    }

    if((status != APP_STATUS_INDICATION_POWERON)&&(status != APP_STATUS_INDICATION_POWEROFF)
    &&(status != APP_STATUS_INDICATION_FULLCHARGE)&&(status != APP_STATUS_INDICATION_TESTMODE)
    &&(status != APP_STATUS_INDICATION_TESTMODE1)&&(status != APP_STATUS_INDICATION_CHARGING)
    &&(status != APP_STATUS_INDICATION_POWERON_BATTERY_DISPLAY)&&(status != APP_STATUS_INDICATION_POWERON_LOWBATTERY_DISPLAY)
    &&(status != APP_STATUS_INDICATION_PHONE_CONNECTED)&&(status != APP_STATUS_INDICATION_INITIAL)&&(status != APP_STATUS_INDICATION_CLEARPAIRLIST)
    &&(status != APP_STATUS_INDICATION_CHARGENEED))
    {
        app_last_status = status;
    }
    
    BESUI_TRACE(0,"[UILED]before recover flag");

    if(uicom.led_recover_last_flag)
    {
        BESUI_TRACE(0,"[UILED]poweron led do not recover");
        return 0;
    }
    

    if((uicom.led_bt_conn_flag)&&(status == APP_STATUS_INDICATION_PHONE_CONNECTED))
    {
        uicom.led_bt_conn_flag = false;
    }

    if(status != APP_STATUS_INDICATION_TESTMODE)
    {
        if(!app_current_box_status_open_status())
        {
            if(status != APP_STATUS_INDICATION_CHARGING)
            {
                memset(&cfg0, 0, sizeof(struct APP_PWL_CFG_T));
                memset(&cfg1, 0, sizeof(struct APP_PWL_CFG_T));
                app_pwl_stop(APP_PWL_ID_0);
                app_pwl_stop(APP_PWL_ID_1);
                return 0;
            }
        }

        if(app_poweroff_flag)
        {
            if(status != APP_STATUS_INDICATION_POWEROFF)
            {
                memset(&cfg0, 0, sizeof(struct APP_PWL_CFG_T));
                memset(&cfg1, 0, sizeof(struct APP_PWL_CFG_T));
                app_pwl_stop(APP_PWL_ID_0);
                app_pwl_stop(APP_PWL_ID_1);
                return 0;
            }
        }        
    }
    BESUI_TRACE(0,"[UILED]led set successfull");

    app_status = status;
    memset(&cfg0, 0, sizeof(struct APP_PWL_CFG_T));
    memset(&cfg1, 0, sizeof(struct APP_PWL_CFG_T));
    app_pwl_stop(APP_PWL_ID_0);
    app_pwl_stop(APP_PWL_ID_1);
    switch (status) {
        case APP_STATUS_INDICATION_POWERON:
#ifdef USER_LED_CASE_OPEN_EN
            uictl.led_caseopen = true;
            cfg0.part[0].level = 1;
            cfg0.part[0].time = (1000);
            cfg0.parttotal = 1;
            cfg0.startlevel = 1;
            cfg0.periodic = false;
            app_pwl_setup(APP_PWL_ID_0, &cfg0);
            app_pwl_start(APP_PWL_ID_0);
            break;
#endif
            cfg0.part[0].level = 0;
            cfg0.part[0].time = (100);
            cfg0.part[1].level = 1;
            cfg0.part[1].time = (1000);
            cfg0.part[2].level = 0;
            cfg0.part[2].time = (300);
            cfg0.parttotal = 3;
            cfg0.startlevel = 0;
            cfg0.periodic = false;


            app_pwl_setup(APP_PWL_ID_0, &cfg0);
            app_pwl_start(APP_PWL_ID_0);
            break;
        case APP_STATUS_INDICATION_INITIAL:
            cfg0.part[0].level = 0;
            cfg0.part[0].time = (1000);
            cfg0.parttotal = 1;
            cfg0.startlevel = 0;
            cfg0.periodic = false;

            cfg1.part[0].level = 0;
            cfg1.part[0].time = (1000);
            cfg1.parttotal = 1;
            cfg1.startlevel = 0;
            cfg1.periodic = false;

            app_pwl_setup(APP_PWL_ID_0, &cfg0);
            app_pwl_start(APP_PWL_ID_0);
            app_pwl_setup(APP_PWL_ID_1, &cfg1);
            app_pwl_start(APP_PWL_ID_1);
            break;
        case APP_STATUS_INDICATION_PAGESCAN:
            cfg0.part[0].level = 1;
            cfg0.part[0].time = (300);
            cfg0.part[1].level = 0;
            cfg0.part[1].time = (2000);
            cfg0.parttotal = 2;
            cfg0.startlevel = 1;
            cfg0.periodic = true;
            app_pwl_setup(APP_PWL_ID_0, &cfg0);
            app_pwl_start(APP_PWL_ID_0);
            break;
        case APP_STATUS_INDICATION_BOTHSCAN: //master pair mode
            cfg0.part[0].level = 0;
            cfg0.part[0].time = (270);
            cfg0.part[1].level = 1;
            cfg0.part[1].time = (300);
            cfg0.parttotal = 2;
            cfg0.startlevel = 0;
            cfg0.periodic = true;

            cfg1.part[0].level = 1;
            cfg1.part[0].time = (270);
            cfg1.part[1].level = 0;
            cfg1.part[1].time = (300);
            cfg1.parttotal = 2;
            cfg1.startlevel = 1;
            cfg1.periodic = true;

            app_pwl_setup(APP_PWL_ID_0, &cfg0);
            app_pwl_start(APP_PWL_ID_0);
            app_pwl_setup(APP_PWL_ID_1, &cfg1);
            app_pwl_start(APP_PWL_ID_1);
            break;
        case APP_STATUS_INDICATION_CONNECTING:

            break;
        case APP_STATUS_INDICATION_CONNECTED:
            cfg0.part[0].level = 1;
            cfg0.part[0].time = (300);
            cfg0.part[1].level = 0;
            cfg0.part[1].time = (20000);
            cfg0.parttotal = 2;
            cfg0.startlevel = 1;
            cfg0.periodic = true;
            app_pwl_setup(APP_PWL_ID_0, &cfg0);
            app_pwl_start(APP_PWL_ID_0);
            break;
        case APP_STATUS_INDICATION_CHARGING:
            cfg0.part[0].level = 0;
            cfg0.part[0].time = (5000);
            cfg0.parttotal = 1;
            cfg0.startlevel = 0;
            cfg0.periodic = false;
            app_pwl_setup(APP_PWL_ID_0, &cfg0);
            app_pwl_start(APP_PWL_ID_0);

            cfg1.part[0].level = 0;
            cfg1.part[0].time = (5000);
            cfg1.parttotal = 1;
            cfg1.startlevel = 0;
            cfg1.periodic = false;
            app_pwl_setup(APP_PWL_ID_1, &cfg1);
            app_pwl_start(APP_PWL_ID_1);
            break;
        case APP_STATUS_INDICATION_FULLCHARGE:
            cfg1.part[0].level = 0;
            cfg1.part[0].time = (5000);
            cfg1.parttotal = 1;
            cfg1.startlevel = 0;
            cfg1.periodic = true;
            app_pwl_setup(APP_PWL_ID_1, &cfg1);
            app_pwl_start(APP_PWL_ID_1);
            break;
        case APP_STATUS_INDICATION_POWEROFF:
            cfg1.part[0].level = 1; //red on 1s
            cfg1.part[0].time = (5000);
            cfg1.parttotal = 1;
            cfg1.startlevel = 1;
            cfg1.periodic = true;
            app_pwl_setup(APP_PWL_ID_1, &cfg1);
            app_pwl_start(APP_PWL_ID_1);

            cfg0.part[0].level = 0; //blue on 1s
            cfg0.part[0].time = (5000);
            cfg0.parttotal = 1;
            cfg0.startlevel = 0;
            cfg0.periodic = false;
            app_pwl_setup(APP_PWL_ID_0, &cfg0);
            app_pwl_start(APP_PWL_ID_0);
            break;
        case APP_STATUS_INDICATION_CHARGENEED:
            cfg1.part[0].level = 1;
            cfg1.part[0].time = (200);
            cfg1.part[1].level = 0;
            cfg1.part[1].time = (200);
            cfg1.part[2].level = 1;
            cfg1.part[2].time = (200);
            cfg1.part[3].level = 0;
            cfg1.part[3].time = (200);
            cfg1.part[4].level = 1;
            cfg1.part[4].time = (200);
            cfg1.part[5].level = 0;
            cfg1.part[5].time = (200);
            cfg1.parttotal = 6;
            cfg1.startlevel = 1;
            cfg1.periodic = false;
            app_pwl_setup(APP_PWL_ID_1, &cfg1);
            app_pwl_start(APP_PWL_ID_1);
            break;
        case APP_STATUS_INDICATION_TESTMODE:
            cfg0.part[0].level = 1;
            cfg0.part[0].time = (100);
            cfg0.part[1].level = 0;
            cfg0.part[1].time = (100);
            cfg0.parttotal = 2;
            cfg0.startlevel = 1;
            cfg0.periodic = true;

            cfg1.part[0].level = 1;
            cfg1.part[0].time = (100);
            cfg1.part[1].level = 0;
            cfg1.part[1].time = (100);
            cfg1.parttotal = 2;
            cfg1.startlevel = 1;
            cfg1.periodic = true;

            app_pwl_setup(APP_PWL_ID_0, &cfg0);
            app_pwl_start(APP_PWL_ID_0);
            app_pwl_setup(APP_PWL_ID_1, &cfg1);
            app_pwl_start(APP_PWL_ID_1);
            break;
        case APP_STATUS_INDICATION_TESTMODE1:
            cfg0.part[0].level = 0;
            cfg0.part[0].time = (1000);
            cfg0.part[1].level = 1;
            cfg0.part[1].time = (1000);
            cfg0.parttotal = 2;
            cfg0.startlevel = 0;
            cfg0.periodic = true;

            cfg1.part[0].level = 0;
            cfg1.part[0].time = (1000);
            cfg1.part[1].level = 1;
            cfg1.part[1].time = (1000);
            cfg1.parttotal = 2;
            cfg1.startlevel = 0;
            cfg1.periodic = true;

            app_pwl_setup(APP_PWL_ID_0, &cfg0);
            app_pwl_start(APP_PWL_ID_0);
            app_pwl_setup(APP_PWL_ID_1, &cfg1);
            app_pwl_start(APP_PWL_ID_1);
            break;

        case APP_STATUS_INDICATION_TWS_CONNECTED: //slave pair mode
            cfg0.part[0].level = 1;
            cfg0.part[0].time = (500);
            cfg0.part[1].level = 0;
            cfg0.part[1].time = (5000);
            cfg0.parttotal = 2;
            cfg0.startlevel = 1;
            cfg0.periodic = true;

            app_pwl_setup(APP_PWL_ID_0, &cfg0);
            app_pwl_start(APP_PWL_ID_0);
            break;

        case APP_STATUS_INDICATION_PHONE_CONNECTED:
            cfg0.part[0].level = 1;
            cfg0.part[0].time = (3000);
            cfg0.part[1].level = 0;
            cfg0.part[1].time = (200);
            cfg0.parttotal = 2;
            cfg0.startlevel = 1;
            cfg0.periodic = false;
            app_pwl_setup(APP_PWL_ID_0, &cfg0);
            app_pwl_start(APP_PWL_ID_0);
            break;
        
        case APP_STATUS_INDICATION_A2DP_STREAM_OPEN:
            cfg0.part[0].level = 0;
            cfg0.part[0].time = (3000);
            cfg0.part[1].level = 0;
            cfg0.part[1].time = (200);
            cfg0.parttotal = 2;
            cfg0.startlevel = 0;
            cfg0.periodic = true;
            app_pwl_setup(APP_PWL_ID_0, &cfg0);
            app_pwl_start(APP_PWL_ID_0);
            break;

        case APP_STATUS_INDICATION_LINKLOSS_DISCONNECTED:
            cfg1.part[0].level = 1;
            cfg1.part[0].time = (300);
            cfg1.part[1].level = 0;
            cfg1.part[1].time = (10000);
            cfg1.parttotal = 2;
            cfg1.startlevel = 1;
            cfg1.periodic = true;
            app_pwl_setup(APP_PWL_ID_1, &cfg1);
            app_pwl_start(APP_PWL_ID_1);
            break;
        
        case APP_STATUS_INDICATION_POWERON_BATTERY_DISPLAY:
            cfg0.part[0].level = 1;
            cfg0.part[0].time = (300);
            cfg0.part[1].level = 0;
            cfg0.part[1].time = (300);
            cfg0.part[2].level = 1;
            cfg0.part[2].time = (300);
            cfg0.part[3].level = 0;
            cfg0.part[3].time = (300);
            cfg0.part[4].level = 1;
            cfg0.part[4].time = (300);
            cfg0.part[5].level = 0;
            cfg0.part[5].time = (300);

            cfg0.parttotal = 6;
            cfg0.startlevel = 1;
            cfg0.periodic = false;

            app_pwl_setup(APP_PWL_ID_0, &cfg0);
            app_pwl_start(APP_PWL_ID_0);
            break;

        case APP_STATUS_INDICATION_POWERON_LOWBATTERY_DISPLAY:
            cfg1.part[0].level = 1;
            cfg1.part[0].time = (300);
            cfg1.part[1].level = 0;
            cfg1.part[1].time = (300);
            cfg1.part[2].level = 1;
            cfg1.part[2].time = (300);
            cfg1.part[3].level = 0;
            cfg1.part[3].time = (300);
            cfg1.part[4].level = 1;
            cfg1.part[4].time = (300);
            cfg1.part[5].level = 0;
            cfg1.part[5].time = (300);

            cfg1.parttotal = 6;
            cfg1.startlevel = 1;
            cfg1.periodic = false;
         
            app_pwl_setup(APP_PWL_ID_1, &cfg1);
            app_pwl_start(APP_PWL_ID_1);
            break;

        case APP_STATUS_INDICATION_INCOMINGCALL:
            cfg0.part[0].level = 0;
            cfg0.part[0].time = (300);
            cfg0.part[1].level = 0;
            cfg0.part[1].time = (300);
            cfg0.parttotal = 2;
            cfg0.startlevel = 0;
            cfg0.periodic = true;
            app_pwl_setup(APP_PWL_ID_0, &cfg0);
            app_pwl_start(APP_PWL_ID_0);
            break;

        case APP_STATUS_INDICATION_ANSWERCALL:
            cfg0.part[0].level = 0;
            cfg0.part[0].time = (1000);
            cfg0.part[1].level = 0;
            cfg0.part[1].time = (1000);
            cfg0.parttotal = 2;
            cfg0.startlevel = 0;
            cfg0.periodic = true;
            app_pwl_setup(APP_PWL_ID_0, &cfg0);
            app_pwl_start(APP_PWL_ID_0);
            break;

        case APP_STATUS_INDICATION_OUTCALL:
            cfg0.part[0].level = 0;
            cfg0.part[0].time = (1000);
            cfg0.part[1].level = 0;
            cfg0.part[1].time = (1000);
            cfg0.parttotal = 2;
            cfg0.startlevel = 0;
            cfg0.periodic = true;
            app_pwl_setup(APP_PWL_ID_0, &cfg0);
            app_pwl_start(APP_PWL_ID_0);
            break;

        case APP_STATUS_INDICATION_CLEARPAIRLIST:
            cfg1.part[0].level = 1;
            cfg1.part[0].time = (200);
            cfg1.part[1].level = 0;
            cfg1.part[1].time = (200);
            cfg1.part[2].level = 1;
            cfg1.part[2].time = (200);
            cfg1.part[3].level = 0;
            cfg1.part[3].time = (200);
            cfg1.part[4].level = 1;
            cfg1.part[4].time = (200);
            cfg1.part[5].level = 0;
            cfg1.part[5].time = (200);
            cfg1.part[6].level = 1;
            cfg1.part[6].time = (200);
            cfg1.part[7].level = 0;
            cfg1.part[7].time = (200);
            cfg1.part[8].level = 1;
            cfg1.part[8].time = (200);
            cfg1.part[9].level = 0;
            cfg1.part[9].time = (200);
            cfg1.parttotal = 10;
            cfg1.startlevel = 1;
            cfg1.periodic = false;
            app_pwl_setup(APP_PWL_ID_1, &cfg1);
            app_pwl_start(APP_PWL_ID_1);
            break;

        default:
            break;
    }
    return 0;
}

#endif //#ifdef BESUI_TWS_EN
