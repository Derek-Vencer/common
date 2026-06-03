/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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
#ifdef MBEDTLS_TEST

#include "hal_trace.h"

extern void rsa_test(void);
extern int mbedtls_aes_self_test( int verbose );
void mbedtls_test(void)
{
    MBEDTLS_TRACE(0, "  ");
    MBEDTLS_TRACE(1, "------ Enter %s ------", __func__);

//    rsa_test();

    mbedtls_aes_self_test(1);

    MBEDTLS_TRACE(1, "------ Exit %s -------", __func__);
}

#endif

