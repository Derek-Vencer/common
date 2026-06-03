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
#ifndef __BTS_MODULE_IF_H__
#define __BTS_MODULE_IF_H__

#include "bts_core_if.h"

#ifdef BT_SVC_MODULE_BT_ENABLED
#include "bts_bt_if.h"
#endif

#ifdef BT_SVC_MODULE_IBRT_ENABLED
#include "bts_ibrt_if.h"
#include "bts_ibrt_tws_switch.h"
#endif

#ifdef BT_SVC_MODULE_TWS_ENABLED
#include "bts_tws_if.h"
#endif

#endif /*__BTS_MODULE_IF_H__*/
