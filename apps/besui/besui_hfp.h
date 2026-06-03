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
#ifndef __BESUI_HFP_H__
#define __BESUI_HFP_H__

#if defined(BESUI_TWS_EN) || defined(BESUI_STEREO_EN)

#ifdef __cplusplus
extern "C" {
#endif

//---------------------------------------------------------------------------
#define ATOK                        "AT+OK"
#define AT_SW_VER                   "AT+SW_VER,H"

#define AT_TALK_MIC                 0x00//test talk mic 
#define AT_FF_MIC                   0x01//test ff mic
#define AT_FB_MIC                   0x02//test fb mic
#define AT_ENC_MIC                  0x03//open enc suanfa
#define AT_SHIP_MODE                0x04
#define AT_GET_SWVER                0x05
#define AT_ANC_ON                   0x06
#define AT_ANC_AA                   0x07
#define AT_ANC_OFF                  0x08
#define AT_ENC_ON                   0x09
#define AT_ENC_OFF                  0x0a
#define AT_BTADDR                   0x0b
#define AT_BTNAME                   0x0c
#define AT_BATTERY                  0x0d
#define AT_SURROUND_ON              0x0e
#define AT_SURROUND_OFF             0x0f
#define AT_INEAR_ON                 0x10
#define AT_INEAR_OFF                0x11
#define AT_POWER_OFF                0x12
#ifdef BESUI_CAPSENSOR_FACTORY_EN
#define AT_CAP_RESTART              0x13
#define AT_CAP_OFFSET               0x14
#define AT_CAP_DIFF                 0x15
#endif
#define AT_NORMAL_MODE              0x16
#define AT_RESET                    0x17
#define AT_ADAPT_ANC_ON             0x18
#define AT_ADAPT_ANC_OFF            0x19

void app_at_cmd_at_cap_diff_return(uint8_t len, uint8_t *cmd_data);
int app_at_msg_handle_process(uint8_t device_id, char *cmd_data);
//---------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif //#if defined(BESUI_TWS_EN) || defined(BESUI_STEREO_EN)

#endif//__BESUI_HFP_H__
