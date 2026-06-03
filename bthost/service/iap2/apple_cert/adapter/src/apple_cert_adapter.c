#include "apple_cert_adapter.h"
#include "apple_cert.h"
#include "simulate_i2c.h"

int apple_cert_adpt_read_apple_version(void)
{
    return read_apple_version();
}

int apple_cert_adpt_read_accessory_certificate_serial_number(u8 *buf,u16 buf_len)
{
    return read_accessory_certificate_serial_number(buf, buf_len);
}

int apple_cert_adpt_read_challenge_response_data(u8 *challenge_buf,u16 challenge_len,u8 *challenge_response_buf,u16 response_buf_len)
{
    return read_challenge_response_data(challenge_buf, challenge_len, challenge_response_buf, response_buf_len);
}

int apple_cert_adpt_read_accessory_certificate_data(u8 *buf,u16 buf_len)
{
    return read_apple_cert_data(buf, buf_len);
}

int apple_cert_adpt_iap2_buf_parse(u8 *buf, u32 length)
{
    return 0;
}

void apple_cert_adpt_i2c_init(void)
{
#ifdef BOARD_HW_GPIO_I2C_MODULE
    i2c_init();
#endif
}
