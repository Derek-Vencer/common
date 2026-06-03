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
#ifndef __BT_SERVICE_DBG_H__
#define __BT_SERVICE_DBG_H__

#ifdef __cplusplus
extern "C"{
#endif

/*****************************header include********************************/
#include "hal_trace.h"

/******************************macro defination*****************************/
#ifndef MULTIMEDIA_MODULE_TRACE_LEVEL
#define MULTIMEDIA_MODULE_TRACE_LEVEL  TR_LEVEL_DEBUG
#endif

#ifdef MULTIMEDIA_DEBUG
#define LOG_V(str, ...) if (MULTIMEDIA_MODULE_TRACE_LEVEL >= TR_LEVEL_VERBOSE) TR_VERBOSE(TR_MOD(BT_SVC), str, ##__VA_ARGS__)
#define LOG_D(str, ...) if (MULTIMEDIA_MODULE_TRACE_LEVEL >= TR_LEVEL_DEBUG) TR_DEBUG(TR_MOD(BT_SVC), str, ##__VA_ARGS__)
#define LOG_I(str, ...) if (MULTIMEDIA_MODULE_TRACE_LEVEL >= TR_LEVEL_INFO) TR_INFO(TR_MOD(BT_SVC), str, ##__VA_ARGS__)
#define LOG_W(str, ...) if (MULTIMEDIA_MODULE_TRACE_LEVEL >= TR_LEVEL_WARN) TR_WARN(TR_MOD(BT_SVC), str, ##__VA_ARGS__)
#define LOG_E(str, ...) if (MULTIMEDIA_MODULE_TRACE_LEVEL >= TR_LEVEL_ERROR) TR_ERROR(TR_MOD(BT_SVC), str, ##__VA_ARGS__)
#else
#define LOG_V(str, ...)
#define LOG_D(str, ...)
#define LOG_I(str, ...)
#define LOG_W(str, ...)
#define LOG_E(str, ...)
#endif

#define LOG_ASSERT(en, str, ...)  ASSERT(en, "[%s][%d] !" str , __FUNCTION__,  __LINE__, ##__VA_ARGS__); 

#ifdef __cplusplus
}
#endif

#endif /* __UX_SERVICE_DBG_H__ */
