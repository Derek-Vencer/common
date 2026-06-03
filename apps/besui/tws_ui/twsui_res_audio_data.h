/***************************************************************************
 *
 * Copyright 2015-2019 BES.
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a database or retrieval system, without prior written
 * permission of BES
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary information of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/
#ifndef __RES_AUDIO_DATA_H__
#define __RES_AUDIO_DATA_H__

#ifdef MEDIA_PLAYER_SUPPORT
const uint8_t EN_POWER_ON [] = {
#include "res/en/voice_tws/SOUND_POWER_ON.txt"
};

const uint8_t EN_POWER_OFF [] = {
#include "res/en/voice_tws/SOUND_POWER_OFF.txt"
};

const uint8_t EN_BT_MAX_VOL[] = {
//#include "res/en/voice_tws/SOUND_MAX_VOL.txt"
};

const uint8_t EN_BT_REFUSE[] = {
#ifndef BESUI_FW_LOG_WN
#include "res/en/voice_tws/SOUND_REFUSE.txt"
#endif
};

const uint8_t EN_BT_HUNG_UP[] = {
#ifndef BESUI_FW_LOG_WN
#include "res/en/voice_tws/SOUND_HUNG_UP.txt"
#endif
};

const uint8_t EN_BT_CONNECTED [] = {
#include "res/en/voice_tws/SOUND_CONNECTED.txt"
};

const uint8_t EN_CHARGE_PLEASE[] = {
#include "res/en/voice_tws/SOUND_CHARGE_PLEASE.txt"
};

const uint8_t EN_BT_PAIR_ENABLE[] = {
#include "res/en/voice_tws/SOUND_PAIR_ENABLE.txt"
};

const uint8_t EN_BT_DIS_CONNECT [] = {
#include "res/en/voice_tws/SOUND_DIS_CONNECT.txt"
};

const uint8_t EN_BT_INCOMING_CALL [] = {
#ifndef BESUI_FW_LOG_WN
#include "res/en/voice_tws/SOUND_INCOMING_CALL.txt"
#endif
};

const uint8_t EN_BT_ALEXA_STOP[] = {
#include "res/en/voice_tws/SOUND_GFPS_FIND.txt"
};

const uint8_t EN_BT_ANSWER[] = {
#ifdef BESUI_TWS_EN
#include "res/en/voice_tws/SOUND_ANSWER.txt"
#endif
};

const uint8_t EN_BT_BEASTGAME_MODE[] = {
#ifndef BESUI_FW_LOG_WN
// #include "res/en/voice_tws/SOUND_BEASTMODE_GAME.txt"
#endif
};

const uint8_t EN_GAME_OFF[] = {
#ifndef BESUI_FW_LOG_WN
// #include "res/en/voice_tws/SOUND_GAME_OFF.txt"
#endif
};

const uint8_t EN_TWS_CONN[] = {
#include "res/en/voice_tws/SOUND_TWS_CONN.txt"
};

// ##################################### sdk prompt ##################################
const uint8_t EN_SOUND_ZERO[] = {
// #include "res/en/SOUND_ZERO.txt"
};
const uint8_t EN_SOUND_ONE[] = {
// #include "res/en/SOUND_ONE.txt"
};
const uint8_t EN_SOUND_TWO[] = {
//#include "res/en/SOUND_THREE.txt"
};

const uint8_t EN_SOUND_THREE[] = {
//#include "res/en/SOUND_THREE.txt"
};
const uint8_t EN_SOUND_FOUR[] = {
//#include "res/en/SOUND_FOUR.txt"
};
const uint8_t EN_BT_OVER[] = {
//#include "res/en/SOUND_OVER.txt"
};
const uint8_t EN_SOUND_FIVE[] = {
//#include "res/en/SOUND_FIVE.txt"
};
const uint8_t EN_SOUND_SIX[] = {
//#include "res/en/SOUND_SIX.txt"
};
const uint8_t EN_SOUND_SEVEN [] = {
//#include "res/en/SOUND_SEVEN.txt"
};
const uint8_t EN_SOUND_EIGHT [] = {
//#include "res/en/SOUND_EIGHT.txt"
};
const uint8_t EN_SOUND_NINE [] = {
//#include "res/en/SOUND_NINE.txt"
};

const uint8_t EN_SOUND_PROMPT_ADAPTIVE_ANC [] = {
//#include "res/adapt_anc/SOUND_PROMPT_ADAPTIVE_ANC.txt"
};
const uint8_t EN_SOUND_CUSTOM_LEAK_DETECT [] = {
//#include "res/ld/SOUND_PROMPT_CUSTOM_LEAK.txt"
};

const uint8_t EN_BT_PAIRING[] = {
//#include "res/en/SOUND_PAIRING.txt"
};

const uint8_t EN_BT_PAIRING_FAIL[] = {
//#include "res/en/SOUND_PAIRING_FAIL.txt"
};

const uint8_t EN_BT_PAIRING_SUCCESS[] = {
//#include "res/en/SOUND_PAIRING_SUCCESS.txt"
};


const uint8_t EN_BT_WARNING[] = {
#include "res/en/SOUND_WARNING.txt"
};

const uint8_t EN_CHARGE_FINISH[] = {
//#include "res/en/SOUND_CHARGE_FINISH.txt"
};

const uint8_t EN_LANGUAGE_SWITCH[] = {
//#include "res/en/SOUND_LANGUAGE_SWITCH.txt"
};

const uint8_t EN_BT_ALEXA_START[] = {
//#include "res/en/SOUND_ALEXA_START.txt"
};

const uint8_t EN_BT_GSOUND_MIC_OPEN[] = {
//#include "res/en/SOUND_GSOUND_MIC_OPEN.txt"
};

const uint8_t EN_BT_GSOUND_MIC_CLOSE[] = {
//#include "res/en/SOUND_GSOUND_MIC_CLOSE.txt"
};

const uint8_t EN_BT_GSOUND_NC[] = {
//#include "res/en/SOUND_GSOUND_NC.txt"
};

#ifdef __INTERACTION__
const uint8_t EN_BT_FINDME[] = {
//#include "res/en/SOUND_FINDME.txt"
};
#endif

const uint8_t EN_BT_MUTE[] = {
//#include "res/SOUND_MUTE.txt"
};


const uint8_t EN_ANC_ON_MODE[] = {
#if defined(ANC_APP)
// #include "res/en/voice_tws/SOUND_ANC_ON.txt"
#endif
};

const uint8_t EN_ANC_AA_MODE[] = {
#if defined(ANC_APP)
// #include "res/en/voice_tws/SOUND_ANC_AA.txt"
#endif
};

const uint8_t EN_BT_SPATIAL_ON[] = {
#if defined(__VIRTUAL_SURROUND__) || defined(__VIRTUAL_SURROUND_STEREO__)
// #include "res/en/voice_tws/SOUND_SPATIAL_ON.txt"
#endif
};

const uint8_t EN_BT_SPATIAL_OFF[] = {
// #include "res/en/voice_tws/SOUND_SPATIAL_OFF.txt"
};

const uint8_t EN_BT_WEAR_DETECT[] = {
// #include "res/en/voice_tws/SOUND_WEAR_DETECT.txt"
};

const uint8_t EN_BT_SIGNATURE[] = {
#if defined(DOLBY_AUDIO_ENABLE)
// #include "res/en/voice_tws/SOUND_DOLBY_OFF.txt"
#endif
};

const uint8_t EN_BT_DOLBY[] = {
#if defined(DOLBY_AUDIO_ENABLE)
// #include "res/en/voice_tws/SOUND_DOLBY_ON.txt"
#endif
};

const uint8_t EN_ANC_ST_MODE[] = {
// #include "res/en/voice_tws/SOUND_ANC_ST.txt"
};

#endif
#endif /* __RES_AUDIO_DATA_H__ */

