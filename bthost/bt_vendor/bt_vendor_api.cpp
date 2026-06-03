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
#include "bt_drv_reg_op.h"
#include "bt_vnd_api.h"
#include "bt_if.h"

#ifdef DEBUG
#define BT_VND_TRACE    TR_INFO
#else
#define BT_VND_TRACE(...)
#endif

void bt_vnd_set_le_con_allow_use_same_addr(bool enable)
{
    static uint8_t balance = 0;
    static bool curr_en = false;

    uint8_t new_balance = balance + (enable ? 1 : -1);

    BT_VND_TRACE(0, "%s: en curr=%d new=%d, balance %d -> %d", __func__, curr_en, enable, balance, new_balance);
    balance = new_balance;

    // * balance > 0 && curr_en == false || balance <= 0 && curr_enable == true
    // * need to revert set le con allow flag
    if ((balance > 0) ^ curr_en)
    {
        curr_en = !curr_en;
        bt_drv_reg_op_set_le_con_allow_use_same_addr(!curr_en);
    }
}
