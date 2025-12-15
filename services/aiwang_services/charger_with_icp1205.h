/*
 * charger_with_icp1205.h
 *
 *  Created on: 2025年12月13日
 *      Author: zhangyijun
 */
#ifndef __CHARGER_WITH_ICP1205_H__
#define __CHARGER_WITH_ICP1205_H__
#ifdef __cplusplus
extern "C" {
#endif

#include "plat_types.h"

void ICP1205_Init(void);
void charger_manager_start(void);

uint32_t writeDataTo_ICP1205(unsigned char reg, unsigned char *data, unsigned char length);
uint32_t readDataFrom_ICP1205(unsigned char reg, unsigned char *data, unsigned char length);


#ifdef __cplusplus
}
#endif
#endif
