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

#pragma once

#ifndef _BECO_DEBUG_H
#define _BECO_DEBUG_H

#include "beco_common.h"

BECO_C_DECLARATIONS_START

void beco_debug_acc_dump(int acc);
void print_vector32_hex(const uint32_t *tm, int w, int h, size_t stride);

BECO_C_DECLARATIONS_END

#endif
