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
#if defined(NEW_NV_RECORD_ENABLED)
#ifdef FINDMY_ENABLED
#ifndef __NVRECORD_FINDMY_H__
#define __NVRECORD_FINDMY_H__
#include "nvrecord_extension.h"
#ifdef __cplusplus
extern "C" {
#endif
NV_FINDMY_INFO_T* nv_record_get_findmy_ptr(void);
void nv_record_findmy_init(void);
void nv_record_findmy_update_info(NV_FINDMY_INFO_T *info);
#ifdef __cplusplus
}
#endif
#endif
#endif
#endif // #if defined(NEW_NV_RECORD_ENABLED)
