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

#include <stdio.h>
#include "hal_trace.h"

#define __is_print(ch) ((unsigned int)((ch) - ' ') < 127u - ' ')
void app_dbg_hex_dump(const char* tag, const unsigned char* mem, const unsigned int size)
{
	if(mem == NULL || size == 0) {
		return;
	}
	// printf("%s : Addr:%#x Len:%d\n", tag == NULL ? "nil" : tag, mem, size);
	unsigned char* buf = (unsigned char*)mem;
	int i, j;
	for( i = 0; i < size; i+=16) {
		printf("%08X: ", i);
		for(j = 0; j < 16; j++) {
			if(i + j < size) {
				printf("%02X ", buf[i + j]);
			} else {
				printf("   ");
			}
		}
		printf(" ");
		for(j = 0; j < 16; j ++) {
			if(i+j < size) {
				printf("%c", __is_print(buf[i+j]) ? buf[i + j] : '.');
			}
		}
		printf("\n");
	}
}
