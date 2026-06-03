#ifdef BESUI_TWS_EN
#include "twsui_spp.h"

#include "twsui_comm.h"
#include "twsui_uart.h"
#include "twsui_btmsg.h"
#include "app_thread.h"
#include "apps.h"
#include "tgt_hardware.h"
#include "app_tws_ibrt.h"
#include "app_bt.h"
#include "app_ibrt_internal.h"
#include "earbud_ux_api.h"
#include "app_ibrt_customif_cmd.h"
#include "app_ui_param_config.h"
#include "twsui_comm.h"
#include "twsui_charge.h"
#include "communication_svr.h"
#include "besbt.h"
#include "nvrecord_extension.h"
#include "nvrecord_env.h"
#include "twsui_key.h"
#include "app_battery.h"
#if defined(CHIP_BEST1306)
#include "charger_best1306.h"
#endif
#if defined(CAPSENSOR_ENABLE)
#include "app_capsensor.h"
#endif
#ifdef GFPS_ENABLED
#include "ble_gfps.h"
#endif

#ifdef BESUI_CAPSENSOR_FACTORY_EN
#include "capsensor_factory_cal.h"
#if defined(CAPSENSOR_ENABLE)
#include CHIP_SPECIFIC_HDR(capsensor_driver)
#endif
#endif

#include "besui_common.h"

#ifdef USER_SPP_CMD_EN

uint8_t spp_data[MAX_SPP_LEN] = {0};
uint8_t spp_data_len = 0;

extern bool app_spp_tota_send_data(uint8_t* ptrData, uint16_t length);

static int app_spp_msg_handle_process(APP_MESSAGE_BODY *msg_body);

void app_spp_msg_modual_init(void)
{
    BESUI_TRACE(0, "[UIUART][%s]", __func__);
    app_set_threadhandle(APP_MUDUAL_UART_MSG, app_spp_msg_handle_process);
}

void app_spp_msg_post_msg(uint8_t *buffer_data, uint8_t data_len)
{
    BESUI_TRACE(0, "[UIUART][%s] data_len: %d ", __func__, data_len);

    APP_MESSAGE_BLOCK msg;

    msg.mod_id = APP_MUDUAL_SPP_MSG;

    if(data_len > MAX_SPP_LEN)
    {
        return;
    }

    memset(spp_data, 0x00, 20);
    memcpy(spp_data, buffer_data, data_len);
    spp_data_len = data_len;

    app_mailbox_put(&msg);
}

void app_spp_only_open_master_mic_test(uint8_t *spp_reply, uint8_t data_len)
{
    app_common_mic_hfp_enc_control(false, ONLY_OPEN_MASTER_MIC);
    spp_reply[4] = 0x01;
    app_spp_tota_send_data(spp_reply, data_len);
}

void app_spp_only_open_slave_mic_test(uint8_t *spp_reply, uint8_t data_len)
{
    app_common_mic_hfp_enc_control(false, ONLY_OPEN_SLAVE_MIC);
    spp_reply[4] = 0x01;
    app_spp_tota_send_data(spp_reply, data_len);
}

void app_spp_open_master_slave_mic_test(uint8_t *spp_reply, uint8_t data_len)
{
    app_common_mic_hfp_enc_control(true, OPEN_MASTER_SLAVE_MIC);
    spp_reply[4] = 0x01;
    app_spp_tota_send_data(spp_reply, data_len);
}

void app_spp_get_software_version(uint8_t *spp_reply, uint8_t data_len)
{
    spp_reply[4] = app_get_software_version_high();
    spp_reply[5] = app_get_software_version_middle();
    spp_reply[6] = app_get_software_version_low();
    spp_reply[7] = 0x01;
    spp_reply[8] = 0xF5;
    data_len = 9;

    app_spp_tota_send_data(spp_reply, data_len);
}

void app_spp_v_get_software_version(uint8_t *spp_reply, uint8_t data_len)
{
    spp_reply[1] = app_get_software_version_high()+'0';
    spp_reply[2] = '.';
    spp_reply[3] = app_get_software_version_middle()+'0';
    spp_reply[4] = '.';
    spp_reply[5] = app_get_software_version_low()+'0';
    spp_reply[6] = '\r';
	spp_reply[7] = '\n';
	spp_reply[8] = 0;
    data_len = 9;

    app_spp_tota_send_data(spp_reply, data_len);
}

void app_spp_get_bt_local_addr(uint8_t *spp_reply, uint8_t data_len)
{
    uint8_t *btaddr = (uint8_t *)bt_get_local_address();
    uint8_t i = 0;

    for(i = 0; i < 6; i++)
    {
        spp_reply[4+i] = btaddr[5-i];
    }

    spp_reply[10] = 0x01;
    spp_reply[11] = 0xF5;
    data_len = 12;

    app_spp_tota_send_data(spp_reply, data_len);
}

void app_spp_get_bt_local_name(uint8_t *spp_reply, uint8_t data_len)
{
    const char *localname = bt_get_local_name();
    uint8_t name_len = strlen(localname);
    uint8_t i = 0;

    for(i = 0; i < name_len; i++)
    {
        spp_reply[4+i] = localname[i];
    }

    spp_reply[name_len+4] = 0x01;
    spp_reply[name_len+1+4] = 0xF5;
    data_len = name_len+4+1+1;

    app_spp_tota_send_data(spp_reply, data_len);
}


void app_spp_get_L_R_side(uint8_t *spp_reply, uint8_t data_len)
{
    spp_reply[4] = besui_get_lr_sta();

    app_spp_tota_send_data(spp_reply, data_len);
}


static int app_spp_msg_handle_process(APP_MESSAGE_BODY *msg_body)
{
    uint8_t app_spp_cmd_event = spp_data[3];

    BESUI_TRACE(3, "[UISPP][%s] %d, %d", __func__, msg_body->message_id, msg_body->message_Param0);

    if(spp_data_len != SPP_DATA_LEN)
    {
        if((spp_data_len == 1)&&((spp_data[0] == 0x56)||(spp_data[0] == 0x76)))
        {
            app_spp_v_get_software_version(spp_data, spp_data_len);
            return 0;
        }
        else
        {
            BESUI_TRACE(0,"[UIUART]app_spp_msg_handle_process len error");
            return -1;
        }
    }

    DUMP8("0x%02x ", spp_data, spp_data_len);

    if((spp_data[0] == 0xF5)&&(spp_data[1] == 0x01)&&(spp_data[spp_data_len-2] == 0x01)&&(spp_data[spp_data_len-1] == 0xF5))
    {
        BESUI_TRACE(1,"[UIUART]%s cmd event %d", __func__, app_spp_cmd_event);

        switch(app_spp_cmd_event)
        {
            case SPP_OPEN_MASTER_MIC_ONLY:
                app_spp_only_open_master_mic_test(spp_data, spp_data_len);
                break;

            case SPP_OPEN_SLAVE_MIC_ONLY:
                app_spp_only_open_slave_mic_test(spp_data, spp_data_len);
                break;

            case SPP_OPEN_MASTER_SLAVE_MIC:
                app_spp_open_master_slave_mic_test(spp_data, spp_data_len);
                break;

            case SPP_GET_SOFTWARE_VERSION:
                app_spp_get_software_version(spp_data, spp_data_len);
                break;

            case SPP_GET_BTADDR:
                app_spp_get_bt_local_addr(spp_data, spp_data_len);
                break;

            case SPP_GET_BTNAME:
                app_spp_get_bt_local_name(spp_data, spp_data_len);
                break;
            
            case SPP_GET_LRSIDE:
                app_spp_get_L_R_side(spp_data, spp_data_len);
                break;
            
            default:

                break;
        }

        return 0;
    }
    else
    {
        BESUI_TRACE(0,"[UIUART]%s crc faile", __func__);
        return -1;
    }
}

#endif //USER_SPP_CMD_EN

#endif //BESUI_TWS_EN