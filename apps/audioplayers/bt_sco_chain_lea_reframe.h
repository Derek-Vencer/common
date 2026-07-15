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
#ifndef __BT_SCO_CHAIN_LEA_REFRAME_H__
#define __BT_SCO_CHAIN_LEA_REFRAME_H__

#include "plat_types.h"
#include "stdint.h"
#include "stdbool.h"

#ifdef __cplusplus
extern "C" {
#endif

int32_t bt_sco_chain_lea_reframe_open(int tx_sample_rate, int rx_sample_rate,
                                int tx_frame_len, int rx_frame_len,
                                int sco_frame_len,
                                uint8_t *buf, int len);
int32_t bt_sco_chain_lea_reframe_close(void);
int32_t bt_sco_chain_lea_reframe_tx_process(void *pcm_buf, void *ref_buf, int *pcm_len);
int32_t bt_sco_chain_lea_reframe_rx_process(void *pcm_buf, int *pcm_len);

#ifdef __cplusplus
}
#endif

#endif
