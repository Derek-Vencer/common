/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
#ifndef __TGT_HARDWARE__
#define __TGT_HARDWARE__

#ifdef __cplusplus
extern "C" {
#endif
#include "hal_iomux.h"
#include "hal_gpio.h"
#include "hal_key.h"
#include "hal_aud.h"

#if defined(__NuttX__)
#include "arch/board/board.h"
#endif

#define BTA_AV_CO_SBC_MAX_BITPOOL  53

// audio codec
#define CFG_HW_AUD_INPUT_PATH_NUM           3
extern const struct AUD_IO_PATH_CFG_T cfg_audio_input_path_cfg[CFG_HW_AUD_INPUT_PATH_NUM];

#define CFG_HW_AUD_OUTPUT_PATH_SPEAKER_DEV  (AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1)

#define CFG_HW_AUD_SIDETONE_MIC_DEV         (AUD_CHANNEL_MAP_CH0)
#define CFG_HW_AUD_SIDETONE_GAIN_DBVAL      (-20)

//bt config
extern const char *BT_LOCAL_NAME;
extern const char *BLE_DEFAULT_NAME;
extern uint8_t ble_global_addr[6];
extern uint8_t bt_global_addr[6];

#define ANC_FF_MIC_CH_L                     (0)
#define ANC_FB_MIC_CH_L                     (0)
#define ANC_FF_MIC_CH_R                     (0)
#define ANC_FB_MIC_CH_R                     (0)
#define ANC_TALK_MIC_CH_L                   (0)
#define ANC_TALK_MIC_CH_R                   (0)
#define ANC_REF_MIC_CH_L                    (0)
#define ANC_REF_MIC_CH_R                    (0)

#ifdef VAD_USE_SAR_ADC
#define CODEC_SADC_VOL (15)
#else
#define CODEC_SADC_VOL (7)
#endif

extern const struct CODEC_DAC_VOL_T codec_dac_vol[TGT_VOLUME_LEVEL_QTY];

#define CFG_AUD_EQ_IIR_NUM_BANDS (4)

#define CODEC_OUTPUT_DEV    0
#ifdef __cplusplus
}
#endif

#endif
