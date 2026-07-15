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
#ifndef __BESUI_DEFINE_H__
#define __BESUI_DEFINE_H__

#ifdef __cplusplus
extern "C" {
#endif

#if defined(BESUI_TWS_EN) || defined(BESUI_STEREO_EN)

//------------------------------------------------------------------------------
#if defined(BES_TWSPRO_EN)
#define BESUI_PROJECT_ID            "TWSPRO"
#define BESUI_BT_NAME               "BESUI_BT"
#define BESUI_BLE_NAME              "BESUI_BLE"
#elif defined(BES_NOTWS_EN)
#define BESUI_PROJECT_ID            "STEREO"
#define BESUI_BT_NAME               "BESUI_BT"
#define BESUI_BLE_NAME              "BESUI_BLE"
#else
#define BESUI_PROJECT_ID            "TWSPRO"
#define BESUI_BT_NAME               "BESUI_BT"
#define BESUI_BLE_NAME              "BESUI_BLE"
#endif

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
#ifdef BESUI_GFPS_ID_EN
//----------------------------------------------------------------------------------------------------
#if defined(BES_TWSPRO_EN) || defined(BES_NOTWS_EN)
/*
Model ID   : 0xB30FF2
Public Key : XhyuuislD+doBXgp1aODVjQ/7pnRh0FCgWX2jF7trqtspxg/X3j27X03jzGK/X0wivwGPaZJCr8/BoPQiyq+jw==
Private Key: qZ3hejudpoPMTdim5TGrqfIMeRfl1ilNG8HTiEUTqpE=
*/
#define USER_GFPS_MODEL_ID          0xB30FF2
#define USER_GFPS_PUBLIC_KEY        {0x5E, 0x1C, 0xAE, 0xBA, 0x2B, 0x25, 0x0F, 0xE7, 0x68, 0x05,  \
                                     0x78, 0x29, 0xD5, 0xA3, 0x83, 0x56, 0x34, 0x3F, 0xEE, 0x99,  \
                                     0xD1, 0x87, 0x41, 0x42, 0x81, 0x65, 0xF6, 0x8C, 0x5E, 0xED,  \
                                     0xAE, 0xAB, 0x6C, 0xA7, 0x18, 0x3F, 0x5F, 0x78, 0xF6, 0xED,  \
                                     0x7D, 0x37, 0x8F, 0x31, 0x8A, 0xFD, 0x7D, 0x30, 0x8A, 0xFC,  \
                                     0x06, 0x3D, 0xA6, 0x49, 0x0A, 0xBF, 0x3F, 0x06, 0x83, 0xD0,  \
                                     0x8B, 0x2A, 0xBE, 0x8F} 
#define USER_GFPS_PRIVATE_KEY       {0xA9, 0x9D, 0xE1, 0x7A, 0x3B, 0x9D, 0xA6, 0x83, 0xCC, 0x4D,  \
                                     0xD8, 0xA6, 0xE5, 0x31, 0xAB, 0xA9, 0xF2, 0x0C, 0x79, 0x17,  \
                                     0xE5, 0xD6, 0x29, 0x4D, 0x1B, 0xC1, 0xD3, 0x88, 0x45, 0x13,  \
                                     0xAA, 0x91}
//----------------------------------------------------------------------------------------------------
#endif
#endif //#ifdef BESUI_GFPS_ID_EN
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------


#if 1
//tws cmd
enum{
   USER_TWS_CMD_INVALID = 0,
   USER_TWS_CMD_ENTER_PAIRMODE,
   USER_TWS_CMD_SLAVE_LINKLOSS,
   USER_TWS_CMD_RANDOM,
   USER_TWS_CMD_CLEAR_PHONE_NV,

   USER_TWS_CMD_TEST_EQTUNE,
   USER_TWS_CMD_MAX,
};

//msg cmd
enum{
   USER_MSG_CMD_INVALID = 0,
   USER_MSG_CMD_2BUDS_CLOSE,
   USER_MSG_CMD_LED_STA_NEXT,
   USER_MSG_CMD_WEAR_PROMPT,
   USER_MSG_CMD_CLEAR_PHONE_NV,
   USER_MSG_CMD_AI,
   USER_MSG_CMD_AI_KWS,

   USER_MSG_CMD_TEST_EQTUNE,
   USER_MSG_CMD_MAX,
};

#define TWS_CONNECTED_EVENT                             0x01
#define TWS_DISCONNECTED_EVENT                          0x02
#define PHONE_CONNECTED_EVENT                           0x03
#define PHONE_DISCONECTED_EVENT                         0x04
#define ENTER_PAIRMODE_EVENT                            0x05
#define EXIT_PHONEMODE_EVENT                            0x06
#define OPENRECONNECTED_TIMEOUT_ENTER_PAIRMODE_EVENT    0x07
#define RECONNECTED_TIMEOUT_ENTER_PAIRMODE_EVENT        0x08
#define TIMEROUT_POWEROFF_EVENT                         0x09
#define PHONE_CANCEL_DISCONNECTED_EVENT                 0x0a

#define SLAVE_ENTER_PAIRMODE_EVENT                      0x0b
#define SLAVE_LINKLOSS_EVENT                            0x0c
#define AUTH_START_EVENT                                0x0d
#define AUTH_SUCCESS_EVENT                              0x0e
#define TWS_STATUS_EVENT                                0x0f
#define BT_SINGLE_PAIRMODE                              0x10
#define BT_MSG_SOMETHING_EVENT                          0x11

#define CHARGE_PLUGIN                                   0x01
#define CHARGE_PLUGOUT                                  0x02
#define PUTIN_BOX                                       0x03
#define PUTOUT_BOX                                      0x04
#define WEAR_UP                                         0x05
#define WEAR_DOWN                                       0x06
#define PUTOUT_PUTIN_DETECT_EVENT                       0x07
#define POWERON_EVENT                                   0x08
#define NV_RECORD_FLUSH_EVENT                           0x09

#define AUTO_POWEROFF                                   0x01
#define LOWBATTERY_POWEROFF                             0x02
#define KEY_FUNCTION_POWEROFF                           0x03
#define CLEARPAIRLIST_POWEROFF                          0x04

#define DEFAULT_TYPE                                    0xff
#define END_CALL_TYPE                                   0x01
#define REJECT_CALL_TYPE                                0x02

#define DISCONNECTED_TYPE                               0x01
#define HANGUP_REJECT_TYPE                              0x02
#define AUDIO_CONNECTED_NOCALL                          0x03

#define INEAR_STATUS                                    0x01
#define OUTEAR_STATUS                                   0x02

#define UNKNOW_SIDE                                     0xff
#define LEFT_SIDE                                       0x00
#define RIGHT_SIDE                                      0x01

#define UNKNOW_STATUS                                   0xff
#define PUTINBOX_STATUS                                 0x03
#define PUTOUTBOX_STATUS                                0x04

#endif

#endif //#if defined(BESUI_TWS_EN) || defined(BESUI_STEREO_EN)

#ifdef __cplusplus
}
#endif

#endif//__BESUI_DEFINE_H__
