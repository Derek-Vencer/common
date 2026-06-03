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
#ifndef __BLE_SHP_H__
#define __BLE_SHP_H__

#ifdef BLE_SHP_SERVER_SUPPORT
#include "gatt_service.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef enum shp_service_char_shp_role
{
    SHS_SHP_ROLE_SMG           = 0x01,
    SHS_SHP_ROLE_SMT           = 0x02,
    SHS_SHP_ROLE_MASK          = 0x03,
} __attribute__((packed)) shp_service_char_shp_role_t;

typedef enum shp_service_char_smg_features
{
    SHS_SMG_FEATURES_64K_SOURCE_SUPPORTED           = 0x01,
    SHS_SMG_FEATURES_96K_SOURCE_SUPPORTED           = 0x02,
    SHS_SMG_FEATURES_MASK                           = 0x03,
} __attribute__((packed)) shp_service_char_smg_features_t;

typedef enum shp_service_char_amt_features
{
    SHS_SMT_FEATURES_SOURCE_SUPPORTED               = 0x01,
    SHS_SMT_FEATURES_48K_SOURCE_SUPPORTED           = 0x02,
    SHS_SMT_FEATURES_64K_SOURCE_SUPPORTED           = 0x04,
    SHS_SMT_FEATURES_64K_SINK_SUPPORTED             = 0x08,
    SHS_SMT_FEATURES_96k_SINK_SUPPORTED             = 0x10,
    SHS_SMT_FEATURES_MASK                           = 0x1F,
} __attribute__((packed)) shp_service_char_amt_features_t;

#ifdef __cplusplus
    }
#endif
#endif /* BLE_SHP_SERVER_SUPPORT */
#endif /* __BLE_SHP_H__ */