#if defined(BESUI_STEREO_EN) && defined(USER_FACTORY_TRACE_RX_EN)
#include "string.h"
#include "plat_types.h"
#include "hal_trace.h"
#include "hal_bootmode.h"
#include "pmu.h"
#include "tgt_hardware.h"

#include "app_bt_stream.h"
#include "app_audio.h"

#include "stereoui.h"
#include "uart_factory.h"
#include "besui_common.h"

static uint16_t uart_rx_crc16(const uint8_t * buffer, uint32_t size)
{
    uint16_t crc = 0xFFFF;

    if (NULL != buffer && size > 0)
    {
        while (size--) 
        {
            crc = (crc >> 8) | (crc << 8);
            crc ^= *buffer++;
            crc ^= ((unsigned char) crc) >> 4;
            crc ^= crc << 12;
            crc ^= (crc & 0xFF) << 5;
        }
    }
    // BESUI_TRACE(1,"[UIUART]%s crc 0x%04x", __func__, crc);
    return crc;
}

void stereo_uart_tx(uint8_t *buf, uint32_t len)
{
    DUMP8("%02X ", buf, len);
}

// bool enc_onoff = true;
// uint8_t mic_type = OPEN_MASTER_SLAVE_MIC;
// void app_common_mic_hfp_enc_control(bool enc, uint8_t mic)
// {
//     enc_onoff = enc;
//     mic_type = mic;
// }

// bool app_get_mic_hfp_enc_onoff(void)
// {
//     return enc_onoff;
// }

// uint8_t app_get_mic_hfp_enc_test_type(void)
// {
//     return mic_type;
// }

static uint8_t app_get_software_version_high(void)
{
    uint16_t version = SOFTWARE_VERSION_INFO;
    uint8_t software_version_high = 0;

    software_version_high = (version/100);

    return software_version_high;
}

static uint8_t app_get_software_version_middle(void)
{
    uint16_t version = SOFTWARE_VERSION_INFO;
    uint8_t software_version_middle = 0;
    
    software_version_middle = (version%100);
    software_version_middle = (software_version_middle/10);

    return software_version_middle;
}

static uint8_t app_get_software_version_low(void)
{
    uint16_t version = SOFTWARE_VERSION_INFO;
    uint8_t software_version_low = 0;

    software_version_low = (version%10);
    
    return software_version_low;
}

void uart_factory_dut(uint8_t *buf, uint32_t len)
{
    uint16_t crc_data = 0;
    uint8_t data_buff[UART_RX_DATA_LEN];
    uint8_t data_length = 3;
    crc_data = uart_rx_crc16(buf, data_length);
    BESUI_TRACE(0, "[%s], crc_data=%04X", __func__, crc_data);

    data_buff[0] = 0x43;
    data_buff[1] = buf[1];
    data_buff[2] = buf[2];
    data_length = 3;
    crc_data = uart_rx_crc16(data_buff, data_length);

    data_buff[3] = (uint8_t)(crc_data >> 8);
    data_buff[4] = (uint8_t)crc_data;
    data_length = data_length+2;
    stereo_uart_tx(data_buff, data_length);

    hal_sw_bootmode_clear(HAL_SW_BOOTMODE_REBOOT);
    hal_sw_bootmode_set(HAL_SW_BOOTMODE_TEST_MODE|HAL_SW_BOOTMODE_TEST_SIGNALINGMODE);
    pmu_reboot();
}

void uart_factory_ver(uint8_t *buf, uint32_t len)
{
    uint16_t crc_data = 0;
    uint8_t data_buff[UART_RX_DATA_LEN];
    uint8_t data_length = 3;
    crc_data = uart_rx_crc16(buf, data_length);
    BESUI_TRACE(0, "[%s], crc_data=%04X", __func__, crc_data);

    data_buff[0] = 0x43;
    data_buff[1] = buf[1];
    data_buff[2] = 0x03;
    data_buff[3] = app_get_software_version_high();
    data_buff[4] = app_get_software_version_middle();
    data_buff[5] = app_get_software_version_low();

    data_length = 6;
    crc_data = uart_rx_crc16(data_buff, data_length);

    data_buff[6] = (uint8_t)(crc_data >> 8);
    data_buff[7] = (uint8_t)crc_data;
    data_length = data_length+2;
    stereo_uart_tx(data_buff, data_length);
}

void uart_factory_mac(uint8_t *buf, uint32_t len)
{
    uint16_t crc_data = 0;
    uint8_t data_buff[UART_RX_DATA_LEN];
    uint8_t data_length = 3;
    crc_data = uart_rx_crc16(buf, data_length);
    BESUI_TRACE(0, "[%s], crc_data=%04X", __func__, crc_data);

    uint8_t *bt_mac_addr;
    data_buff[0] = 0x43;
    data_buff[1] = buf[1];
    data_buff[2] = 0x06;

    bt_mac_addr = bt_global_addr;

    for(uint8_t i = 0; i < 6; i++)
    {
        data_buff[3+i] = bt_mac_addr[i];
    }

    data_length = 9;
    crc_data = uart_rx_crc16(data_buff, data_length);

    data_buff[data_length] = (uint8_t)(crc_data >> 8);
    data_buff[data_length+1] = (uint8_t)crc_data;
    data_length = data_length+2;
    stereo_uart_tx(data_buff, data_length);
}

void uart_factory_enc(uint8_t *buf, uint32_t len)
{
    uint16_t crc_data = 0;
    uint8_t data_buff[UART_RX_DATA_LEN];
    uint8_t data_length = 3;
    crc_data = uart_rx_crc16(buf, data_length);
    BESUI_TRACE(0, "[%s], crc_data=%04X", __func__, crc_data);

    if(buf[2] == 0x01) //enc on
    {
        BESUI_TRACE(0, "[%s], enc on", __func__);
        app_common_mic_hfp_enc_control(true, OPEN_MASTER_SLAVE_MIC);
    }
    else if(buf[2] == 0x00) //enc off
    {
        BESUI_TRACE(0, "[%s], enc off", __func__);
        app_common_mic_hfp_enc_control(false, OPEN_MASTER_SLAVE_MIC);
    }
    memcpy(data_buff, buf, len);
    data_buff[0] = 0x43;
    data_buff[1] = buf[1];
    data_buff[2] = buf[2];
    data_length = 3;
    crc_data = uart_rx_crc16(data_buff, data_length);

    data_buff[3] = (uint8_t)(crc_data >> 8);
    data_buff[4] = (uint8_t)crc_data;
    data_length = data_length+2;
    stereo_uart_tx(data_buff, data_length);
}

void uart_factory_mic(uint8_t *buf, uint32_t len)
{
    uint16_t crc_data = 0;
    uint8_t data_buff[UART_RX_DATA_LEN];
    uint8_t data_length = 3;
    crc_data = uart_rx_crc16(buf, data_length);
    BESUI_TRACE(0, "[%s], crc_data=%04X", __func__, crc_data);

    if(buf[2] == 0x01)
    {
        BESUI_TRACE(0, "[%s], FF_ON", __func__);
        speech_mic_set_onoff(SPEECH_TEST_MIC0);
    }
    else if(buf[2] == 0x02)
    {
        BESUI_TRACE(0, "[%s], FB_ON", __func__);
        speech_mic_set_onoff(SPEECH_TEST_MIC1);
    }
    else
    {
        speech_mic_set_onoff(0);
    }

    memcpy(data_buff, buf, len);
    data_buff[0] = 0x43;
    data_buff[1] = buf[1];
    data_buff[2] = buf[2];
    data_length = 3;
    crc_data = uart_rx_crc16(data_buff, data_length);

    data_buff[3] = (uint8_t)(crc_data >> 8);
    data_buff[4] = (uint8_t)crc_data;
    data_length = data_length+2;
    stereo_uart_tx(data_buff, data_length);
}

void stereo_uart_rx_process(uint8_t *buf, uint32_t len)
{
    uint16_t crc_data = 0;
    uint8_t data_length = 3;
    uint8_t cmd_event = 0xFF;    
    uint16_t crc_rx_data = (buf[len-2]<<8) | buf[len-1];
    crc_data = uart_rx_crc16(buf, data_length);

    BESUI_TRACE(2, "[%s] len: %d", __func__, len);
    DUMP8("%02X ", buf, len);

    BESUI_TRACE(0, "[%s], crc_data=%04X", __func__, crc_data);

    if(len < 5)
    {
        BESUI_TRACE(0, "[%s], buf len < 5, err", __func__);
        return;
    }
    if(buf[0] != 0x34) //left / sreteos
    {
        BESUI_TRACE(0, "[%s], right cmd error", __func__);
        return;
    }
    if(crc_rx_data != crc_data)
    {
        BESUI_TRACE(0, "[%s], crc error", __func__);
        return;
    }
    cmd_event = buf[1];
    BESUI_TRACE(0, "[%s], cmd_event = %d", __func__, cmd_event);
    switch(cmd_event)
    {
        case UART_RX_DUT:
            uart_factory_dut(buf, len);
        break;
        case UART_RX_VER: 
            uart_factory_ver(buf, len);
        break;
        case UART_RX_MAC: 
            uart_factory_mac(buf, len);
        break;
        case UART_RX_ENC: 
            uart_factory_enc(buf, len);
        break;
        case UART_RX_MIC: 
            uart_factory_mic(buf, len);
        break;
        default:
        BESUI_TRACE(0, "[%s], cmd_event error", __func__);
        break;
    }
}

#endif //#if defined(BESUI_STEREO_EN) && defined(USER_FACTORY_TRACE_RX_EN)