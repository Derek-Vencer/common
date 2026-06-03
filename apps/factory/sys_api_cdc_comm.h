#ifndef __SYS_API_USB_CDC_H__
#define __SYS_API_USB_CDC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "tool_msg.h"
#include "hal_trace.h"
#include "hal_timer.h"

#define TIMEOUT_INFINITE                ((uint32_t)-1)
#define cdc_default_recv_timeout_short   (MS_TO_TICKS(500))
#define cdc_default_recv_timeout_idle    (TIMEOUT_INFINITE) //MS_TO_TICKS(10 * 60 * 1000);
#define cdc_default_recv_timeout_4k_data (MS_TO_TICKS(500))
#define cdc_default_send_timeout         (MS_TO_TICKS(500))

void reset_transport(void);

void set_recv_timeout(unsigned int timeout);
void set_send_timeout(unsigned int timeout);

int cdc_send_data(const unsigned char *buf, size_t len);
int recv_data_ex(unsigned char *buf, size_t len, size_t expect, size_t *rlen);
int handle_error(void);
int cancel_input(void);

void system_reboot(void);
void system_shutdown(void);
void system_flash_boot(void);
void system_set_bootmode(unsigned int bootmode);
void system_clear_bootmode(unsigned int bootmode);
unsigned int system_get_bootmode(void);

#ifdef __cplusplus
}
#endif

#endif

