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

#ifndef __CRC_C_H
#define __CRC_C_H

extern unsigned short const crc16_table[256];

#ifdef __cplusplus
extern "C" {
#endif

unsigned char crc8_c(const unsigned char *input, int length);

unsigned short _crc16(unsigned short crc, const unsigned char *buffer, unsigned int len);

int crc16ccitt_c(int crc, unsigned char * bytes, int start, int len);

unsigned long crc32_c(unsigned long crc, const unsigned char *buf, unsigned int len);

#ifdef __cplusplus
}
#endif

static inline unsigned short crc16_byte(unsigned short crc, const unsigned char data)
{
    return (crc >> 8) ^ crc16_table[(crc ^ data) & 0xff];
}

#endif /* __CRC_C_H */
