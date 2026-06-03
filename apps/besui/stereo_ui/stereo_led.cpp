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
#ifdef BESUI_STEREO_EN
#include "cmsis_os.h"
#include "stdbool.h"
#include "hal_trace.h"
#include "app_pwl.h"
#include "stereo_led.h"
#include "app_status_ind.h"
#include "string.h"

#include "besui_common.h"

static APP_STATUS_INDICATION_T app_ui_status = APP_STATUS_INDICATION_NUM_MAX;
static APP_STATUS_INDICATION_T app_status_ind_filter = APP_STATUS_INDICATION_NUM;
static APP_STATUS_INDICATION_T app_last_status = APP_STATUS_INDICATION_NUM;

int app_pwl_low_level_drive_stop(enum APP_PWL_ID_T id);


uint8_t app_ui_status_indication_init(void)
{
    BESUI_TRACE(0, "%s", __func__);
    struct APP_PWL_CFG_T cfg;
    memset(&cfg, 0, sizeof(struct APP_PWL_CFG_T));
    stereo_ui_pwl_open();
    app_pwl_setup(APP_PWL_ID_0, &cfg);
    stereo_ui_pwl_setup(APP_PWL_ID_1, &cfg);
    return 0;
}

void app_ui_status_indication_close(void)
{
    app_pwl_close();
}

int app_status_indication_filter_set(APP_STATUS_INDICATION_T status)
{
    app_status_ind_filter = status;
    return 0;
}
APP_STATUS_INDICATION_T app_status_indication_get(void)
{
    return app_ui_status;
}


APP_STATUS_INDICATION_T app_last_status_indication_get(void)
{
    return app_last_status;
}


//cfg0-blue-1 on, cfg1-red-0 on
int app_status_indication_set(APP_STATUS_INDICATION_T status)
{
    if (status == app_ui_status && status!=APP_STATUS_INDICATION_BATTERY_LOW)
    {
        BESUI_TRACE(1, "%s is exist", __func__);
        return 0;
    }



    struct APP_PWL_CFG_T cfg0;
    struct APP_PWL_CFG_T cfg1;

    memset(&cfg0, 0, sizeof(struct APP_PWL_CFG_T));
    memset(&cfg1, 0, sizeof(struct APP_PWL_CFG_T));
    app_pwl_low_level_drive_stop(APP_PWL_ID_1);
    app_pwl_stop(APP_PWL_ID_0);  //cfg0-blue-1 on, cfg1-red-0 on
    // app_pwl_stop(APP_PWL_ID_1);

    if(uictl.batt_low_flag && status==APP_STATUS_INDICATION_PAIRING_CONNECTED)
    {
        BESUI_TRACE(0, "%s, battery low connect status no display", __func__);
        return 0;
    }

    app_ui_status = status;
    BESUI_TRACE(2,"%s %d",__func__, status);

    switch (status) {
        case APP_STATUS_INDICATION_POWER_ON:
            cfg0.part[0].level = 1;
            cfg0.part[0].time = (1000);
            cfg0.part[1].level = 0;
            cfg0.part[1].time = (300);
            cfg0.parttotal = 2;
            cfg0.startlevel = 1;
            cfg0.periodic = false;

            app_pwl_setup(APP_PWL_ID_0, &cfg0);
            app_pwl_start(APP_PWL_ID_0);
            break;
        case APP_STATUS_INDICATION_POWER_OFF:
            cfg1.part[0].level = 0;
            cfg1.part[0].time = (1000);
            cfg1.part[1].level = 1;
            cfg1.part[1].time = (300);
            cfg1.parttotal = 2;
            cfg1.startlevel = 0;
            cfg1.periodic = false;

            app_pwl_setup(APP_PWL_ID_1, &cfg1);
            app_pwl_start(APP_PWL_ID_1);
            break;
        case APP_STATUS_INDICATION_CLEAR_DEVICE_LIST:
            cfg0.part[0].level = 1;
            cfg0.part[0].time = (1000);
            cfg0.part[1].level = 0;
            cfg0.part[1].time = (1000);
            cfg0.parttotal = 2;
            cfg0.startlevel = 1;
            cfg0.periodic = false;

            cfg1.part[0].level = 0;
            cfg1.part[0].time = (1000);
            cfg1.part[1].level = 1;
            cfg1.part[1].time = (1000);
            cfg1.parttotal = 2;
            cfg1.startlevel = 1;
            cfg1.periodic = false;

            app_pwl_setup(APP_PWL_ID_0, &cfg0);
            app_pwl_start(APP_PWL_ID_0);
            app_pwl_setup(APP_PWL_ID_1, &cfg1);
            app_pwl_start(APP_PWL_ID_1);
            break;
        case APP_STATUS_INDICATION_PAIRING_MODE:
            cfg0.part[0].level = 1;
            cfg0.part[0].time = (500);
            cfg0.part[1].level = 0;
            cfg0.part[1].time = (500);
            cfg0.parttotal = 2;
            cfg0.startlevel = 1;
            cfg0.periodic = true;

            cfg1.part[0].level = 1;
            cfg1.part[0].time = (500);
            cfg1.part[1].level = 0;
            cfg1.part[1].time = (500);
            cfg1.parttotal = 2;
            cfg1.startlevel = 1;
            cfg1.periodic = true;

            app_pwl_setup(APP_PWL_ID_0, &cfg0);
            app_pwl_start(APP_PWL_ID_0);
            app_pwl_setup(APP_PWL_ID_1, &cfg1);
            app_pwl_start(APP_PWL_ID_1);
            break;
        case APP_STATUS_INDICATION_PAIRING_SUCCESSFUL:
            cfg0.part[0].level = 1;
            cfg0.part[0].time = (3000);
            cfg0.parttotal = 1;
            cfg0.startlevel = 1;
            cfg0.periodic = false;

            app_pwl_setup(APP_PWL_ID_0, &cfg0);
            app_pwl_start(APP_PWL_ID_0);
            break;
        case APP_STATUS_INDICATION_PAIRING_NOT_SUCCESSFUL:
            cfg1.part[0].level = 0;
            cfg1.part[0].time = (300);
            cfg1.part[1].level = 1;
            cfg1.part[1].time = (300);
            cfg1.parttotal = 2;
            cfg1.startlevel = 0;
            cfg1.periodic = false;

            app_pwl_setup(APP_PWL_ID_1, &cfg1);
            app_pwl_start(APP_PWL_ID_1);
            break;
        case APP_STATUS_INDICATION_PAIRING_CONNECTED:
            cfg0.part[0].level = 1;
            cfg0.part[0].time = (1000);
            cfg0.part[1].level = 0;
            cfg0.part[1].time = (1000);
            cfg0.parttotal = 2;
            cfg0.startlevel = 1;
            cfg0.periodic = true;

            cfg1.part[0].level = 1;
            cfg1.part[0].time = (1000);
            cfg1.parttotal = 1;
            cfg1.startlevel = 1;
            cfg1.periodic = false;

            app_pwl_setup(APP_PWL_ID_0, &cfg0);
            app_pwl_start(APP_PWL_ID_0);
            app_pwl_setup(APP_PWL_ID_1, &cfg1);
            app_pwl_start(APP_PWL_ID_1);
            break;
        case APP_STATUS_INDICATION_PAIRING_DISCONNECTED:
            cfg1.part[0].level = 0;
            cfg1.part[0].time = (300);
            cfg1.part[1].level = 1;
            cfg1.part[1].time = (300);
            cfg1.parttotal = 2;
            cfg1.startlevel = 0;
            cfg1.periodic = false;

            app_pwl_setup(APP_PWL_ID_1, &cfg1);
            app_pwl_start(APP_PWL_ID_1);
            break;
        case APP_STATUS_INDICATION_PAIRING_LINK_LOSS:
            cfg0.part[0].level = 0;
            cfg0.part[0].time = (500);
            cfg0.parttotal = 1;
            cfg0.startlevel = 0;
            cfg0.periodic = true;

            cfg1.part[0].level = 1;
            cfg1.part[0].time = (10000); //off 10s
            cfg1.part[1].level = 0;
            cfg1.part[1].time = (300);   //on 300ms
            cfg1.parttotal = 2;
            cfg1.startlevel = 0;
            cfg1.periodic = true;

            app_pwl_setup(APP_PWL_ID_0, &cfg0); //bule
            app_pwl_start(APP_PWL_ID_0);
            app_pwl_setup(APP_PWL_ID_1, &cfg1); //red
            app_pwl_start(APP_PWL_ID_1);
            break;
        case APP_STATUS_INDICATION_CHARG:
            cfg1.part[0].level = 0;
            cfg1.part[0].time = (5000);
            cfg1.parttotal = 1;
            cfg1.startlevel = 0;
            cfg1.periodic = true;

            app_pwl_setup(APP_PWL_ID_1, &cfg1);
            app_pwl_start(APP_PWL_ID_1);
            break;
        case APP_STATUS_INDICATION_CHARGED_FULLY:
            // red led off    blue led off
            cfg0.part[0].level = 0;
            cfg0.part[0].time = (1000);
            cfg0.parttotal = 1;
            cfg0.startlevel = 0;
            cfg0.periodic = false;

            cfg1.part[0].level = 1;
            cfg1.part[0].time = (1000);
            cfg1.parttotal = 1;
            cfg1.startlevel = 1;
            cfg1.periodic = false;

            app_pwl_setup(APP_PWL_ID_0, &cfg0);
            app_pwl_start(APP_PWL_ID_0);
            app_pwl_setup(APP_PWL_ID_1, &cfg1);
            app_pwl_start(APP_PWL_ID_1);
            break;
        case APP_STATUS_INDICATION_BATTERY_LOW:
            for (int i=0;i<20;i=i+4)
            {
                cfg1.part[i].level = 0;
                cfg1.part[i].time = (200);
                cfg1.part[i+1].level = 1;
                cfg1.part[i+1].time = (200);
                cfg1.part[i+2].level = 0;
                cfg1.part[i+2].time = (200);
                cfg1.part[i+3].level = 1;
                cfg1.part[i+3].time = (400);
            }
            cfg1.parttotal = 20;
            cfg1.startlevel = 1;
            cfg1.periodic = false;

            app_pwl_setup(APP_PWL_ID_1, &cfg1);
            app_pwl_start(APP_PWL_ID_1);

            break;
        case APP_STATUS_INDICATION_CALL_MUSIC:
            cfg0.part[0].level = 0;
            cfg0.part[0].time = (1000);
            cfg0.parttotal = 1;
            cfg0.startlevel = 0;
            cfg0.periodic = false;

            cfg1.part[0].level = 1;
            cfg1.part[0].time = (1000);
            cfg1.parttotal = 1;
            cfg1.startlevel = 1;
            cfg1.periodic = false;

            app_pwl_setup(APP_PWL_ID_0, &cfg0);
            app_pwl_start(APP_PWL_ID_0);
            app_pwl_setup(APP_PWL_ID_1, &cfg1);
            app_pwl_start(APP_PWL_ID_1);
            break;
        default:
            break;
    }
    return 0;
}

#endif //#ifdef BESUI_STEREO_EN
