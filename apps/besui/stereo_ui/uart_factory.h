#ifndef __UART_FACTORY_H__
#define __UART_FACTORY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

#if defined(BESUI_STEREO_EN) && defined(USER_FACTORY_TRACE_RX_EN)
#define UART_RX_DUT                 0x01
#define UART_RX_VER                 0x04
#define UART_RX_MAC                 0x08

//0x60~0xDF
#define UART_RX_ENC                 0x60
#define UART_RX_MIC                 0x61


#define SPEECH_TEST_MIC0            0x01
#define SPEECH_TEST_MIC1            0x02


#define UART_RX_DATA_LEN            20



void stereo_uart_rx_process(uint8_t *buf, uint32_t len);

#endif

#ifdef __cplusplus
}
#endif

#endif //#ifndef __UART_FACTORY_H__