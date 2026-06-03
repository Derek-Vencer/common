/***************************************************************************
 *
 * Copyright 2015-2024 BES.
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
#ifndef APP_SHP_H_
#define APP_SHP_H_

#if BLE_AUDIO_ENABLED

#include "bluetooth.h"

/*
Audio Config-uration for SHP SMT based Spatialization

AC-U1   1 DEV   SMT based SPA               headset one cis stereo but create two   @AOB_AUD_CFG_FREEMAN_STEREO_ONE_CIS
                ----->>
                ----->>
AC-U2   2 DEV   SMT based SPA               earbuds two cis stereo                  @AOB_AUD_CFG_TWS_STEREO_ONE_CIS
                ----->>
                ----->>
AC-U3   1 DEV   SMT based SPA               headset one cis stereo but create two   @AOB_AUD_CFG_FREEMAN_STEREO_ONE_CIS
                ----->>
                <---->>
AC-U4   2 DEV   SMT based SPA               earbuds two cis stereo                  @AOB_AUD_CFG_TWS_STEREO_ONE_CIS
                ----->>
                <---->>

Audio Config-uration for SHP SMG based Spatialization

6(v)    1 DEV   SMG SPA Without Multiplex   headset two cis stereo                  @AOB_AUD_CFG_FREEMAN_STEREO_TWO_CIS
                o------>
                ------->
6(vi)   2 DEV   SMG SPA Without Multiplex   earbuds two cis mono                    @AOB_AUD_CFG_TWS_MONO
                o------>
                ------->
8(iii)  1 DEV   SMG SPA Multiplex           headset two cis stereo                  @AOB_AUD_CFG_FREEMAN_STEREO_TWO_CIS
                ------->
                <o----->
8(iv)   2 DEV   SMG SPA Multiplex           earbuds two cis mono                    @AOB_AUD_CFG_TWS_MONO
                ------->
                <o----->
8(v)    1 DEV   SMG SPA Without Multiplex   headset two cis stereo                  @AOB_AUD_CFG_FREEMAN_STEREO_TWO_CIS
                o------>
                <------>
8(vi)   2 DEV   SMG SPA Without Multiplex   earbuds two cis mono                    @AOB_AUD_CFG_TWS_MONO
                o------>
                <------>
11(iii) 1 DEV   SMG SPA Multiplex           headset two cis stereo                  @AOB_AUD_CFG_FREEMAN_STEREO_TWO_CIS
                <o----->
                <------>
11(iv)  2 DEV   SMG SPA Multiplex           earbuds two cis mono                    @AOB_AUD_CFG_TWS_MONO
                <o----->
                <------>
11(v)   1 DEV   SMG SPA Multiplex           headset two cis stereo                  @AOB_AUD_CFG_FREEMAN_STEREO_TWO_CIS
                <o----->
                <o----->
11(vi)  2 DEV   SMG SPA Multiplex           earbuds two cis mono                    @AOB_AUD_CFG_TWS_MONO
                <o----->
                <o----->
*/

typedef enum
{
    SHP_QOS_SETTING_MIN = 0,
    /// SMT based spatialization
    SHP_QOS_SETTING_SMT_SPA_MIN = SHP_QOS_SETTING_MIN,
    SHP_QOS_SETTING_32_1_AL = SHP_QOS_SETTING_SMT_SPA_MIN,
    SHP_QOS_SETTING_32_2_AL,
    SHP_QOS_SETTING_32_1_AH,
    SHP_QOS_SETTING_32_2_AH,
    SHP_QOS_SETTING_48_1_AL,
    SHP_QOS_SETTING_48_2_AL,
    SHP_QOS_SETTING_48_3_AL,
    SHP_QOS_SETTING_48_4_AL,
    SHP_QOS_SETTING_48_1_AH,
    SHP_QOS_SETTING_48_2_AH,
    SHP_QOS_SETTING_48_3_AH,
    SHP_QOS_SETTING_48_4_AH,
    SHP_QOS_SETTING_SMT_SPA_MAX,
    /// SMG based spatialization SINK
    SHP_QOS_SETTING_SMG_SPA_SINK_MIN = SHP_QOS_SETTING_SMT_SPA_MAX,
    SHP_QOS_SETTING_48_1_AR = SHP_QOS_SETTING_SMG_SPA_SINK_MIN,
    SHP_QOS_SETTING_48_2_AR,
    SHP_QOS_SETTING_48_3_AR,
    SHP_QOS_SETTING_48_4_AR,
    SHP_QOS_SETTING_SMG_SPA_SINK_MAX,
    /// SMG based spatialization SRC
    SHP_QOS_SETTING_SMG_SPA_SRC_MIN = SHP_QOS_SETTING_SMG_SPA_SINK_MAX,
    SHP_QOS_SETTING_24_1_AC = SHP_QOS_SETTING_SMG_SPA_SRC_MIN,
    SHP_QOS_SETTING_24_2_AC,
    SHP_QOS_SETTING_32_1_AC,
    SHP_QOS_SETTING_32_2_AC,
    SHP_QOS_SETTING_48_1_AC,
    SHP_QOS_SETTING_48_2_AC,
    SHP_QOS_SETTING_48_3_AC,
    SHP_QOS_SETTING_48_4_AC,
    SHP_QOS_SETTING_SMG_SPA_SRC_MAX,
    /// SMG based spatialization SRC HT frame
    SHP_QOS_SETTING_SMG_SPA_HT_MIN = SHP_QOS_SETTING_SMG_SPA_SRC_MAX,
    SHP_QOS_SETTING_HT_1_8V = SHP_QOS_SETTING_SMG_SPA_HT_MIN,
    SHP_QOS_SETTING_HT_2_8V,
    SHP_QOS_SETTING_HT_3_8V,
    SHP_QOS_SETTING_HT_4_8V,
    SHP_QOS_SETTING_HT_5_8V,
    SHP_QOS_SETTING_HT_6_8V,
    SHP_QOS_SETTING_HT_1_8VI,
    SHP_QOS_SETTING_HT_2_8VI,
    SHP_QOS_SETTING_HT_3_8VI,
    SHP_QOS_SETTING_HT_4_8VI,
    SHP_QOS_SETTING_HT_5_8VI,
    SHP_QOS_SETTING_HT_6_8VI,
    SHP_QOS_SETTING_SMG_SPA_HT_MAX,

    SHP_QOS_SETTING_MAX = SHP_QOS_SETTING_SMG_SPA_HT_MAX,
} app_shp_qos_setting_e;

/*
Audio Configuration for Unicast

Number of Servers                       Range: 1-31
Number of Sink ASEs                     Range: 0-31
Number of Source ASEs                   Range: 0-31
Audio Channels per Sink ASE             Range: 0-31
Min Sink Audio Locations per Server     Range: 0-31
Audio Channels per Source ASE           Range: 0-31
Min Source Audio Locations per Server   Range: 0-31
Number of CISes                         Range: 1-31
Number of Unicast Audio Streams         Range: 1-31
Central to Peripheral PHY               0x00: Parameter does not apply.
                                        Bit 0 = 0b1: Central-to-Peripheral PHY is LE 1M
                                        Bit 1 = 0b1: Central-to-Peripheral PHY is LE 2M
                                        Bit 2 = 0b1: Central-to-Peripheral PHY is LE Coded
                                        Bits 3 to 7: RFU
Central to Peripheral PHY subfield      RFU
Peripheral to Central PHY               0x00: Parameter does not apply.
                                        Bit 0 = 0b1: Peripheral-to-Central PHY is LE 1M
                                        Bit 1 = 0b1: Peripheral-to-Central PHY is LE 2M
                                        Bit 2 = 0b1: Peripheral-to-Central PHY is LE Coded
                                        Bits 3 to 7: RFU
Peripheral to Central PHY subfield      RFU
*/

#define APP_BAP_AC_U1       {1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0,}
#define APP_BAP_AC_U2       {1, 0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0, 0,}
#define APP_BAP_AC_U3       {1, 1, 0, 1, 1, 1, 0, 1, 2, 0, 0, 0, 0,}
#define APP_BAP_AC_U4       {1, 1, 2, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0,}
#define APP_BAP_AC_U5       {1, 1, 2, 1, 1, 1, 1, 1, 2, 0, 0, 0, 0,}
#define APP_BAP_AC_U6I      {1, 2, 2, 0, 1, 0, 1, 2, 2, 0, 0, 0, 0,}
#define APP_BAP_AC_U6II     {2, 2, 1, 0, 1, 0, 1, 2, 2, 0, 0, 0, 0,}
#define APP_BAP_AC_U7I      {1, 1, 0, 1, 1, 1, 0, 2, 2, 0, 0, 0, 0,}
#define APP_BAP_AC_U7II     {2, 1, 0, 1, 1, 1, 0, 2, 2, 0, 0, 0, 0,}
#define APP_BAP_AC_U8I      {1, 2, 2, 1, 1, 1, 1, 2, 3, 0, 0, 0, 0,}
#define APP_BAP_AC_U8II     {2, 2, 1, 1, 1, 1, 1, 2, 3, 0, 0, 0, 0,}
#define APP_BAP_AC_U9I      {1, 0, 0, 2, 0, 1, 0, 2, 2, 0, 0, 0, 0,}
#define APP_BAP_AC_U9II     {2, 0, 0, 2, 0, 1, 0, 2, 2, 0, 0, 0, 0,}
#define APP_BAP_AC_U10      {1, 0, 0, 1, 0, 2, 0, 1, 1, 0, 0, 0, 0,}
#define APP_BAP_AC_U11I     {1, 2, 2, 2, 1, 1, 1, 2, 4, 0, 0, 0, 0,}
#define APP_BAP_AC_U11II    {2, 2, 1, 2, 1, 1, 1, 2, 4, 0, 0, 0, 0,}

#define APP_SHP_AC_U1       {1, 2, 0, 2, 1, 0, 0, 2, 2, 0, 0, 0, 0,}
#define APP_SHP_AC_U2       {2, 2, 0, 2, 1, 0, 0, 2, 2, 0, 0, 0, 0,}
#define APP_SHP_AC_U3       {1, 2, 1, 2, 1, 1, 1, 2, 3, 0, 0, 0, 0,}
#define APP_SHP_AC_U4       {2, 2, 1, 2, 1, 1, 1, 2, 3, 0, 0, 0, 0,}

/*
Audio Configuration for Broadcast

Audio Channels per BIS                  Range: 1-31
Number of BISes                         Range: 1-31
Number of Broadcast Audio Streams       Range: 1-31
Broadcast PHY                           0x00: Parameter does not apply.
                                        Bit 0 = 0b1: Broadcast PHY is LE 1M.
                                        Bit 1 = 0b1: Broadcast PHY is LE 2M.
                                        Bit 2 = 0b1: Broadcast PHY is LE Coded.
                                        Bits 3 to 7: RFU
*/

#define APP_BAP_AC_B12      {1, 1, 1, 0,}
#define APP_BAP_AC_B13      {1, 2, 2, 0,}
#define APP_BAP_AC_B14      {2, 1, 1, 0,}

#define APP_SHP_AC_B1       {1, 3, 3, 0,}
#define APP_SHP_AC_B2       {3, 1, 1, 0,}
#define APP_SHP_AC_B3       {1, 4, 4, 0,}
#define APP_SHP_AC_B4       {4, 1, 1, 0,}
#define APP_SHP_AC_B5       {2, 2, 2, 0,}

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief HT interval enumerate to interval (us)
 * 
 * @param  ht_intv_e
 * 
 * @return uint32_t
 */
uint32_t app_shp_ht_intv_enum_to_ht_intv_us(uint8_t ht_intv_e);


#ifdef __cplusplus
}
#endif

#endif
#endif // APP_SHP_H_