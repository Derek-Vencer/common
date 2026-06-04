/*
 * charge_ntc.h
 *
 *  Created on: 2025年12月25日
 *      Author: zhangyijun
 */

#ifndef __SERVICES_AIWANG_SERVICES_CHARGER_NTC_H__
#define __SERVICES_AIWANG_SERVICES_CHARGER_NTC_H__

#ifdef __cplusplus
extern "C" {
#endif

void aw_ntc_detect_init(void);
void aw_ntc_detect_process(uint16_t ad_volt);
void aw_ntc_detect_volt_timer_onoff(bool timer_en);
#ifdef __cplusplus
}
#endif

#endif /* __SERVICES_AIWANG_SERVICES_CHARGER_NTC_H__ */
