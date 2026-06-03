#ifndef __APPLE_CERT_ADAPTER__
#define __APPLE_CERT_ADAPTER__
#include <unistd.h>
#include "plat_types.h"

int apple_cert_adpt_read_accessory_certificate_serial_number(u8 *buf,u16 buf_len);
int apple_cert_adpt_read_challenge_response_data(u8 *challenge_buf,u16 challenge_len,u8 *challenge_response_buf,u16 response_buf_len);
int apple_cert_adpt_read_accessory_certificate_data(u8 *buf,u16 buf_len);
int apple_cert_adpt_iap2_buf_parse(u8 *buf, u32 length);
void apple_cert_adpt_i2c_init(void);
#endif // __APPLE_CERT_ADAPTER__