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
//#include "cmsis_os.h"
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include "nvrecord_extension.h"
#include "nvrecord_findmy.h"
#include "hal_trace.h"
#ifdef FINDMY_ENABLED
static NV_FINDMY_INFO_T *nvrecord_findmy_p = NULL;
void nvrecord_rebuild_findmy(void)
{
    if (nvrecord_findmy_p)
    {
        memset((uint8_t *)nvrecord_findmy_p, 0, sizeof(NV_FINDMY_INFO_T));
    }
}
NV_FINDMY_INFO_T* nv_record_get_findmy_ptr(void)
{
    return nvrecord_findmy_p;
}
void nv_record_findmy_init(void)
{
    if (NULL == nvrecord_findmy_p)
    {
        nvrecord_findmy_p = &(nvrecord_extension_p->findmy_info);
    }
}
void nv_record_findmy_update_info(NV_FINDMY_INFO_T *info)
{
    uint32_t lock = nv_record_pre_write_operation();
    if (!info)
    {
        NV_SECTION_TRACE(0, "%s invalid parameter!!!", __func__);
        nv_record_post_write_operation(lock);
        return;
    }
    memcpy((uint8_t *)nvrecord_findmy_p, (uint8_t *)info, sizeof(NV_FINDMY_INFO_T));
    nv_record_post_write_operation(lock);
    nv_record_update_runtime_userdata();
}
#endif
#endif // #if defined(NEW_NV_RECORD_ENABLED)
