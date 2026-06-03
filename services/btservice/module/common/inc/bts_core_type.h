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
#ifndef __BTS_MODULE_TYPE_H__
#define __BTS_MODULE_TYPE_H__

typedef uint8_t TWS_UI_ROLE_E;
#define TWS_UI_MASTER       0       //IBRT_MASTER
#define TWS_UI_SLAVE        1       //IBRT_SLAVE
#define TWS_UI_UNKNOWN      0xff    //IBRT_UNKNOW

typedef enum {
    EAR_SIDE_UNKNOWN = 0,
    EAR_SIDE_LEFT    = 1,
    EAR_SIDE_RIGHT   = 2,
    EAR_SIDE_NUM,
} APP_TWS_SIDE_T;

#endif /*__BTS_MODULE_TYPE_H__*/
