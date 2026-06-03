/***************************************************************************
 *
 * Copyright 2015-2025 BES.
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
#ifndef __NN_KWS_MODEL_PARAMS_H__
#define __NN_KWS_MODEL_PARAMS_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SpeechNNModelParams {
    unsigned char *weight_data;
    unsigned char *scale_data;
    unsigned char *quant_params_data;
    unsigned int weight_data_len;
    unsigned int scale_data_len;
    unsigned int quant_params_data_len;
    unsigned int out_dim;
}SpeechNNModelParams;

#ifdef __cplusplus
}
#endif

#endif