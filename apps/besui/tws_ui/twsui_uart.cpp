#ifdef BESUI_TWS_EN
#include "twsui_comm.h"

#include "twsui_uart.h"
#include "twsui_btmsg.h"
#include "twsui_charge.h"

#include "hal_bootmode.h"
#include "app_thread.h"
#include "apps.h"
#include "tgt_hardware.h"
#include "app_tws_ibrt.h"
#include "app_bt.h"
#include "app_ibrt_internal.h"
#include "earbud_ux_api.h"
#include "app_ibrt_customif_cmd.h"
#include "app_ui_param_config.h"
#include "besbt.h"
#include "bts_tws_if.h"
#include "bts_core_if.h"

#include "communication_svr.h"
#include "nvrecord_extension.h"
#include "nvrecord_env.h"
#include "twsui_key.h"
#include "app_battery.h"
#include "app_factory_bt.h"
#include "factory_section.h"

#if !defined(CHIP_BEST1501P) && !defined(CHIP_BEST1502X)
#include CHIP_SPECIFIC_HDR(charger)
#endif
#if defined(CAPSENSOR_ENABLE)
#include "app_capsensor.h"
#endif
#ifdef GFPS_ENABLED
#include "ble_gfps.h"
#include "gfps.h"
#endif

#ifdef BESUI_CAPSENSOR_FACTORY_EN
#include "capsensor_factory_cal.h"
#if defined(CAPSENSOR_ENABLE)
#include CHIP_SPECIFIC_HDR(capsensor_driver)
#endif
#endif

#include "besui_common.h"


#ifdef BESUI_1WIRE_EN
type_uiuart_t uiuart;

bool case_open_tx_bat = false;

static int app_uart_communication_handle_process(APP_MESSAGE_BODY *msg_body);


void communication_receive_rx_data_prec(uint8_t *data_buf, uint8_t data_len)
{
    if(((data_buf[0]&0x0f) != UART_HEAD_RIGHT)&&(besui_get_lr_sta() == RIGHT_SIDE))
    {
        BESUI_TRACE(0,"[UIUART]rx right side do not need replay left cmd");
        return;
    }

    if(((data_buf[0]&0x0f) != UART_HEAD_LEFT)&&(besui_get_lr_sta() == LEFT_SIDE))
    {
        BESUI_TRACE(0,"[UIUART]rx left side do not need replay right cmd");
        return;
    }

    memset(uiuart.rx_dat, 0x00, 40);
    memcpy(uiuart.rx_dat, data_buf, data_len);
    app_uart_communication_post_msg(data_buf, data_len);
}

void app_uart_communication_modual_init(void)
{
    if(!uiuart.init_flag)
    {
        BESUI_TRACE(0, "[UIUART]%s", __func__);
        app_set_threadhandle(APP_MODUAL_COMMUNICATION_UART, app_uart_communication_handle_process);

        communication_init();
        communication_receive_register_callback(communication_receive_rx_data_prec);
        uiuart.init_flag = true;
    }
}


void app_uart_communication_post_msg(uint8_t *uart_data, uint8_t len)
{
    BESUI_TRACE(1, "[UIUART][%s] len: %d ", __func__, len);

    APP_MESSAGE_BLOCK msg;
    msg.mod_id = APP_MODUAL_COMMUNICATION_UART;
    msg.msg_body.message_Param2 = len;
    app_mailbox_put(&msg);
}

void app_uart_set_need_twspair_init(void)
{
    struct nvrecord_env_t *nvrecord_env;
    nv_record_env_get(&nvrecord_env);
    uiuart.twspair_flag = nvrecord_env->need_twspair_flag;

    BESUI_TRACE(0,"[UIUART]%s, twspair_flag=%d", __func__, uiuart.twspair_flag);
}

void app_uart_clear_need_twspair_process(void)
{
    struct nvrecord_env_t *nvrecord_env;

    if(uiuart.twspair_flag)
    {
        nv_record_env_get(&nvrecord_env);
        uiuart.twspair_flag = false;
        nvrecord_env->need_twspair_flag = uiuart.twspair_flag;
        if(!bts_tws_if_is_tws_link_connected())
        {
            memset(nvrecord_env->another_addr, 0x00, 6);
        }
        nv_record_env_set(nvrecord_env);
    }
}

void app_uart_enter_twspair_config_set(bool isRightMasterFlag)
{
    struct nvrecord_env_t *nvrecord_env;
    uint8_t bt_default_addr[6] = {0};
    uint8_t *bt_local_addr;

    nv_record_env_get(&nvrecord_env);

    bt_local_addr = bt_get_local_address();

    BESUI_TRACE(1,"[UIUART]%s uiuart.twspair_flag %d", __func__, uiuart.twspair_flag);
    DUMP8("%02x ",bt_local_addr, 6);
    DUMP8("%02x ",nvrecord_env->another_addr, 6);

    if(uiuart.twspair_flag)
    {
        if(besui_get_lr_sta() == LEFT_SIDE)
        {
            app_tws_ibrt_reconfig_role(IBRT_MASTER, bt_local_addr, nvrecord_env->another_addr, isRightMasterFlag);
            return;
        }
        else if(besui_get_lr_sta() == RIGHT_SIDE)
        {
            app_tws_ibrt_reconfig_role(IBRT_SLAVE, nvrecord_env->another_addr, bt_local_addr, isRightMasterFlag);
            return;
        }
    }
    else
    {
        if(memcmp(bt_default_addr, nvrecord_env->another_addr, 6) == 0)
        {
            if(besui_get_lr_sta() == LEFT_SIDE)
            {
                app_tws_ibrt_reconfig_role(IBRT_MASTER, bt_local_addr, bt_local_addr, isRightMasterFlag);
                return;
            }
            else if(besui_get_lr_sta() == RIGHT_SIDE)
            {
                app_tws_ibrt_reconfig_role(IBRT_SLAVE, bt_local_addr, bt_local_addr, isRightMasterFlag);
                return;
            }
        }
        else
        {
            if(besui_get_lr_sta() == LEFT_SIDE)
            {
                app_tws_ibrt_reconfig_role(IBRT_MASTER, bt_local_addr, nvrecord_env->another_addr, isRightMasterFlag);
                return;
            }
            else if(besui_get_lr_sta() == RIGHT_SIDE)
            {
                app_tws_ibrt_reconfig_role(IBRT_SLAVE, nvrecord_env->another_addr, bt_local_addr, isRightMasterFlag);
                return;
            }
        }
    }
}

#if 0
void app_uart_open_box_get_box_battery(uint8_t *recive_data)
{
    return;
    uint8_t data_buff[DATA_MAX_LEN];
    uint8_t data_length = 0;

    memset(data_buff, 0x00, DATA_MAX_LEN);
    memcpy(data_buff, recive_data, 4);

    uicom.bat_box_percent = recive_data[4];

    data_buff[4] = 0x01;
    data_buff[5] = 0x01;
    data_buff[6] = 0xF5;
    data_length = 7;
    communication_send_buf(data_buff, data_length);
}


void app_uart_get_right_btaddr_process(uint8_t *recive_data)
{
    uint8_t data_buff[DATA_MAX_LEN];
    uint8_t data_length = 0;
    uint8_t l_r_side = recive_data[2];
    uint8_t *bt_right_addr;
    uint8_t i = 0;

    memset(data_buff, 0x00, DATA_MAX_LEN);

    memcpy(data_buff, recive_data, 4);

    if(l_r_side == besui_get_lr_sta())
    {
        bt_right_addr = bt_get_local_address();
        for(i = 0; i < 6; i++)
        {
            data_buff[4+i] = bt_right_addr[i];
        }

        data_buff[4+6] = 0x01;
        data_buff[4+6+1] = 0xF5;

        data_length = 12;

        BESUI_TRACE(0,"[UIUART]app_uart_get_right_btaddr_process");
        DUMP8("%02x ",data_buff, data_length);

        communication_send_buf(data_buff, data_length);
    }
}


void app_uart_send_right_addr_to_left_process(uint8_t *recive_data)
{
    return;
    uint8_t data_buff[DATA_MAX_LEN];
    uint8_t data_length = 0;
    uint8_t l_r_side = recive_data[2];
    uint8_t bt_right_addr[6] = {0};
    uint8_t *bt_left_addr;
    uint8_t i = 0;
    bool need_twspair_flag = false;
    struct nvrecord_env_t *nvrecord_env;

    nv_record_env_get(&nvrecord_env);

    memset(data_buff, 0x00, DATA_MAX_LEN);

    if((besui_get_lr_sta() == LEFT_SIDE)&&(l_r_side == RIGHT_SIDE))
    {
        for(i = 0; i < 6; i++)
        {
            bt_right_addr[i] = recive_data[4+i];
        }

        bt_left_addr = (uint8_t *)bt_get_local_address();

        if((bt_left_addr[5] == bt_right_addr[5])&&(bt_left_addr[4] == bt_right_addr[4])&&(bt_left_addr[3] == bt_right_addr[3]))
        {
            if(bts_tws_if_is_tws_link_connected())
            {
                #if 0
                //dangqian twsconnected addr
                if(memcmp(bt_right_addr, nvrecord_env->another_addr, 6) == 0)
                {
                    
                }
                else
                #endif
                {
                    //store addr to flash
                    memcpy(nvrecord_env->another_addr, bt_right_addr, 6);
                    need_twspair_flag = true;
                    nv_record_env_set(nvrecord_env);
                }
            }
            else
            {
                //store addr to flash
                memcpy(nvrecord_env->another_addr, bt_right_addr, 6);
                need_twspair_flag = true;
                nv_record_env_set(nvrecord_env);
            }
        }
        else
        {
            //nothing
            BESUI_TRACE(0,"[UIUART]gao san zijie bu yizhi");
        }


        if(need_twspair_flag)
        {
            data_buff[0] = 0xF5;
            data_buff[1] = 0x01;
            data_buff[2] = 0x00;
            data_buff[3] = 0x02;
            for(i = 0; i < 6; i++)
            {
                data_buff[4+i] = bt_left_addr[i];
            }
            data_buff[10] = 0x01;
            data_buff[11] = 0xF5;
            data_length = 12;
        }
        else
        {
            data_buff[0] = 0xF5;
            data_buff[1] = 0x01;
            data_buff[2] = 0x00;
            data_buff[3] = 0x02;
            data_buff[4] = 0x00;
            data_buff[5] = 0x01;
            data_buff[6] = 0xF5;
            data_length = 7;
        }

        communication_send_buf(data_buff, data_length);
    }
}

void app_uart_send_left_addr_to_right_process(uint8_t *recive_data)
{
    return;
    uint8_t data_buff[DATA_MAX_LEN];
    uint8_t data_length = 0;
    uint8_t l_r_side = recive_data[2];
    uint8_t i = 0;
    struct nvrecord_env_t *nvrecord_env;

    nv_record_env_get(&nvrecord_env);

    memset(data_buff, 0x00, DATA_MAX_LEN);

    if((besui_get_lr_sta() == RIGHT_SIDE)&&(l_r_side == LEFT_SIDE))
    {
        for(i = 0; i < 6; i++)
        {
            nvrecord_env->another_addr[i] = recive_data[4+i];
            nv_record_env_set(nvrecord_env);
        }

        data_buff[0] = 0xF5;
        data_buff[1] = 0x01;
        data_buff[2] = 0x01;
        data_buff[3] = 0x03;
        data_buff[4] = 0x01;
        data_buff[5] = 0x01;
        data_buff[6] = 0xF5;

        data_length = 7;
        communication_send_buf(data_buff, data_length);
    }
}


void app_uart_need_twspair_process(uint8_t *recive_data)
{
    return;
    uint8_t data_buff[DATA_MAX_LEN];
    uint8_t data_length = 0;
    struct nvrecord_env_t *nvrecord_env;

    memset(data_buff, 0x00, DATA_MAX_LEN);

    nv_record_env_get(&nvrecord_env);
    nvrecord_env->need_twspair_flag = true;
    nv_record_env_set(nvrecord_env);

    data_buff[0] = 0xF5;
    data_buff[1] = 0x01;
    data_buff[2] = 0x00;
    data_buff[3] = 0x04;
    data_buff[4] = 0x01;
    data_buff[5] = 0x01;
    data_buff[6] = 0xF5;

    data_length = 7;
    communication_send_buf(data_buff, data_length);
    //fast shutdown
    uictl.poweroff_fast_flag = true;
    app_shutdown();
}
#endif //#if 0

uint16_t CRC16(const uint8_t * buffer, uint32_t size)
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
    BESUI_TRACE(1,"[UIUART]%s crc 0x%04x", __func__, crc);
    return crc;
}


void app_uart_open_charge_box_handle(uint8_t *uart_dat)
{
    uint8_t data_buff[DATA_MAX_LEN];
    uint8_t data_length = 0;
    uint16_t crc_data = 0;
    uint8_t ear_battery_level = 10;

    memset(data_buff, 0x00, DATA_MAX_LEN);

    if(((uart_dat[0]&0x0f) == UART_HEAD_LEFT)&&(besui_get_lr_sta() == LEFT_SIDE))
    {
        data_buff[0] = (UART_HEAD_LEFT << 4) | UART_HEAD_BOX;
    }
    else if(((uart_dat[0]&0x0f) == UART_HEAD_RIGHT)&&(besui_get_lr_sta() == RIGHT_SIDE))
    {
        data_buff[0] = (UART_HEAD_RIGHT << 4) | UART_HEAD_BOX;
    }
    
    data_buff[1] = uart_dat[1];
    data_buff[2] = uart_dat[2];
    uicom.bat_box_percent = uart_dat[3];
    BESUI_TRACE(1,"[UIUART]%s box level 0x%02x", __func__, uicom.bat_box_percent);
    ear_battery_level = app_battery_current_level();
    BESUI_TRACE(1,"[UIUART]%s EAR level %d", __func__, ear_battery_level);
    data_buff[3] = ((ear_battery_level+1)*10);
    BESUI_TRACE(1,"[UIUART]%s EAR level 0x%02x", __func__, data_buff[3]);

    data_length = 4;
    crc_data = CRC16(data_buff, data_length);
    data_buff[4] = (uint8_t)(crc_data >> 8);
    data_buff[5] = (uint8_t)crc_data;

    data_length = 6;
    communication_send_buf(data_buff, data_length);

    app_uart_open_box_handle();
    case_open_tx_bat = false;
}

void app_uart_close_charge_box_handle(uint8_t *uart_dat)
{
    uint8_t data_buff[DATA_MAX_LEN];
    uint8_t data_length = 0;
    uint16_t crc_data = 0;
    uint8_t ear_battery_level = 10;

    memset(data_buff, 0x00, DATA_MAX_LEN);

    if(((uart_dat[0]&0x0f) == UART_HEAD_LEFT)&&(besui_get_lr_sta() == LEFT_SIDE))
    {
        data_buff[0] = (UART_HEAD_LEFT << 4) | UART_HEAD_BOX;
    }
    else if(((uart_dat[0]&0x0f) == UART_HEAD_RIGHT)&&(besui_get_lr_sta() == RIGHT_SIDE))
    {
        data_buff[0] = (UART_HEAD_RIGHT << 4) | UART_HEAD_BOX;
    }
    
    data_buff[1] = uart_dat[1];
    data_buff[2] = uart_dat[2];
    uicom.bat_box_percent = uart_dat[3];
    BESUI_TRACE(1,"[UIUART]%s box level 0x%02x", __func__, uicom.bat_box_percent);
    ear_battery_level = app_battery_current_level();
    BESUI_TRACE(1,"[UIUART]%s EAR level %d", __func__, ear_battery_level);
    data_buff[3] = ((ear_battery_level+1)*10);
    BESUI_TRACE(1,"[UIUART]%s EAR level 0x%02x", __func__, data_buff[3]);
    data_length = 4;
    crc_data = CRC16(data_buff, data_length);

    data_buff[4] = (uint8_t)(crc_data >> 8);
    data_buff[5] = (uint8_t)crc_data;

    data_length = 6;
    communication_send_buf(data_buff, data_length);

    // uicom.box_rx_open = 3;
    app_uart_close_box_handle();
}


void app_uart_get_ear_battery_level_handle(uint8_t *uart_dat)
{
    uint8_t data_buff[DATA_MAX_LEN];
    uint8_t data_length = 0;
    uint16_t crc_data = 0;
    uint8_t ear_battery_level = 10;

    memset(data_buff, 0x00, DATA_MAX_LEN);

    if(((uart_dat[0]&0x0f) == UART_HEAD_LEFT)&&(besui_get_lr_sta() == LEFT_SIDE))
    {
        data_buff[0] = (UART_HEAD_LEFT << 4) | UART_HEAD_BOX;
    }
    else if(((uart_dat[0]&0x0f) == UART_HEAD_RIGHT)&&(besui_get_lr_sta() == RIGHT_SIDE))
    {
        data_buff[0] = (UART_HEAD_RIGHT << 4) | UART_HEAD_BOX;
    }
    
    data_buff[1] = uart_dat[1];
    data_buff[2] = 0x2;

    if(charger_charge_status_get() == CHARGER_CHARGE_STATUS_DONE || besui_bat_full_sta_get())
        data_buff[3] = 1; //bat full
    else
        data_buff[3] = 0;

    uicom.bat_box_percent = uart_dat[3];
    BESUI_TRACE(1,"[UIUART]%s box level 0x%02x", __func__, uicom.bat_box_percent);
    ear_battery_level = app_battery_current_level();
    BESUI_TRACE(1,"[UIUART]%s EAR level %d", __func__, ear_battery_level);
    data_buff[4] = ((ear_battery_level+1)*10);

    if(besui_bat_charge_sta_get())
        data_buff[4] = data_buff[4]|0x80;
    else
        data_buff[4] = data_buff[4]&0x7F;

    if(data_buff[4] > 100)
        data_buff[4] = 100;
    BESUI_TRACE(1,"[UIUART]%s EAR level 0x%02x", __func__, data_buff[4]);
    data_length = 5;
    crc_data = CRC16(data_buff, data_length);

    data_buff[5] = (uint8_t)(crc_data >> 8);
    data_buff[6] = (uint8_t)crc_data;

    data_length = 7;
    communication_send_buf(data_buff, data_length);
}


void app_uart_sw_reset_handle(uint8_t *uart_dat)
{
    uint8_t data_buff[DATA_MAX_LEN];
    uint8_t data_length = 0;
    uint16_t crc_data = 0;

    memset(data_buff, 0x00, DATA_MAX_LEN);

    if(((uart_dat[0]&0x0f) == UART_HEAD_LEFT)&&(besui_get_lr_sta() == LEFT_SIDE))
    {
        data_buff[0] = (UART_HEAD_LEFT << 4) | UART_HEAD_BOX;
    }
    else if(((uart_dat[0]&0x0f) == UART_HEAD_RIGHT)&&(besui_get_lr_sta() == RIGHT_SIDE))
    {
        data_buff[0] = (UART_HEAD_RIGHT << 4) | UART_HEAD_BOX;
    }
    
    data_buff[1] = uart_dat[1];
    data_buff[2] = uart_dat[2];

    data_length = 3;
    crc_data = CRC16(data_buff, data_length);

    data_buff[3] = (uint8_t)(crc_data >> 8);
    data_buff[4] = (uint8_t)crc_data;

    data_length = 5;
    communication_send_buf(data_buff, data_length);

    uiuart.event_type = UART_EVENT_SW_RESET;

    app_uart_event_timer_onoff(true);
}

void app_uart_bt_pairmode_handle(uint8_t *uart_dat)
{
    uint8_t data_buff[DATA_MAX_LEN];
    uint8_t data_length = 0;
    uint16_t crc_data = 0;

    memset(data_buff, 0x00, DATA_MAX_LEN);

    if(((uart_dat[0]&0x0f) == UART_HEAD_LEFT)&&(besui_get_lr_sta() == LEFT_SIDE))
    {
        data_buff[0] = (UART_HEAD_LEFT << 4) | UART_HEAD_BOX;
    }
    else if(((uart_dat[0]&0x0f) == UART_HEAD_RIGHT)&&(besui_get_lr_sta() == RIGHT_SIDE))
    {
        data_buff[0] = (UART_HEAD_RIGHT << 4) | UART_HEAD_BOX;
    }
    
    data_buff[1] = uart_dat[1];
    data_buff[2] = uart_dat[2];

    data_length = 3;
    crc_data = CRC16(data_buff, data_length);

    data_buff[3] = (uint8_t)(crc_data >> 8);
    data_buff[4] = (uint8_t)crc_data;

    data_length = 5;
    communication_send_buf(data_buff, data_length);

    uiuart.event_type = UART_EVENT_BT_PAIRMODE;
    app_uart_event_timer_onoff(true);
}

void app_uart_get_ear_addr_handle(uint8_t *uart_dat)
{
    uint8_t data_buff[DATA_MAX_LEN];
    uint8_t data_length = 0;
    uint16_t crc_data = 0;
    uint8_t *bt_left_addr;
    uint8_t i = 0;

    BESUI_TRACE(0,"[UIUART]%s", __func__);

    memset(data_buff, 0x00, DATA_MAX_LEN);

    if(((uart_dat[0]&0x0f) == UART_HEAD_LEFT)&&(besui_get_lr_sta() == LEFT_SIDE))
    {
        data_buff[0] = (UART_HEAD_LEFT << 4) | UART_HEAD_BOX;
    }
    else if(((uart_dat[0]&0x0f) == UART_HEAD_RIGHT)&&(besui_get_lr_sta() == RIGHT_SIDE))
    {
        data_buff[0] = (UART_HEAD_RIGHT << 4) | UART_HEAD_BOX;
    }
    
    data_buff[1] = uart_dat[1];
    data_buff[2] = 0x0c;

    bt_left_addr = (uint8_t *)bt_get_local_address();
    for(i = 0; i < 6; i++)
    {
        data_buff[3+i] = bt_left_addr[i];
    }

    for(i = 0; i < 6; i++)
    {
        data_buff[9+i] = 0x00;
    }

    data_length = 15;
    crc_data = CRC16(data_buff, data_length);

    data_buff[15] = (uint8_t)(crc_data >> 8);
    data_buff[16] = (uint8_t)crc_data;

    data_length = 17;
    communication_send_buf(data_buff, data_length);
}

void app_uart_get_other_ear_addr_handle(uint8_t *uart_dat)
{
    uint8_t data_buff[DATA_MAX_LEN];
    uint8_t data_length = 0;
    uint16_t crc_data = 0;
    //uint8_t *bt_local_addr;
    uint8_t other_ear_addr[6] = {0};
    uint8_t i = 0;
    struct nvrecord_env_t *nvrecord_env;

    BESUI_TRACE(0,"[UIUART]%s", __func__);

    nv_record_env_get(&nvrecord_env);

    memset(data_buff, 0x00, DATA_MAX_LEN);

    if(((uart_dat[0]&0x0f) == UART_HEAD_LEFT)&&(besui_get_lr_sta() == LEFT_SIDE))
    {
        data_buff[0] = (UART_HEAD_LEFT << 4) | UART_HEAD_BOX;
    }
    else if(((uart_dat[0]&0x0f) == UART_HEAD_RIGHT)&&(besui_get_lr_sta() == RIGHT_SIDE))
    {
        data_buff[0] = (UART_HEAD_RIGHT << 4) | UART_HEAD_BOX;
    }
    
    data_buff[1] = uart_dat[1];
    data_buff[2] = 0x00;

    data_length = 3;
    crc_data = CRC16(data_buff, data_length);

    data_buff[3] = (uint8_t)(crc_data >> 8);
    data_buff[4] = (uint8_t)crc_data;

    data_length = 5;
    communication_send_buf(data_buff, data_length);

    for(i = 0; i < 6; i++)
    {
        other_ear_addr[i] = uart_dat[3+i]; //other ear addr
    }
    //store addr to flash
    //bt_local_addr = (uint8_t *)bt_get_local_address();
    //if((bt_local_addr[5] == other_ear_addr[5])&&(bt_local_addr[4] == other_ear_addr[4])&&(bt_local_addr[3] == other_ear_addr[3]))
    {
        if(memcmp(other_ear_addr, nvrecord_env->another_addr, 6) == 0)
        {
            uiuart.twspair_flag = false;
            BESUI_TRACE(0,"[UIUART]do not need twspair, ben shen jiushi yidui");
        }
        else
        {
            uiuart.twspair_flag = true;
            // if(bts_tws_if_is_tws_link_connected())
            // {
                memcpy(nvrecord_env->another_addr, other_ear_addr, 6);
                nv_record_env_set(nvrecord_env);
            // }
            // else
            // {
            //     memcpy(nvrecord_env->another_addr, other_ear_addr, 6);
            //     nv_record_env_set(nvrecord_env);
            // }
        }
    }
    // else
    // {
    //     uiuart.twspair_flag = false;
    //     BESUI_TRACE(0,"[UIUART]gao san zijie bu yizhi");
    // }
    BESUI_TRACE(0,"[UIUART]%s, uiuart.twspair_flag = %d", __func__, uiuart.twspair_flag);
}

void app_uart_tx_peer_addr(uint8_t *uart_dat)
{
    uint8_t data_buff[DATA_MAX_LEN] = {0};
    uint8_t data_length = 0;
    uint16_t crc_data = 0;
    uint8_t i = 0;
    struct nvrecord_env_t *nvrecord_env;

    nv_record_env_get(&nvrecord_env);

    BESUI_TRACE(0,"[UIUART]%s", __func__);

    if(((uart_dat[0]&0x0f) == UART_HEAD_LEFT)&&(besui_get_lr_sta() == LEFT_SIDE))
    {
        data_buff[0] = (UART_HEAD_LEFT << 4) | UART_HEAD_BOX;
    }
    else if(((uart_dat[0]&0x0f) == UART_HEAD_RIGHT)&&(besui_get_lr_sta() == RIGHT_SIDE))
    {
        data_buff[0] = (UART_HEAD_RIGHT << 4) | UART_HEAD_BOX;
    }
    
    data_buff[1] = uart_dat[1];
    data_buff[2] = 0x06;

    for(i = 0; i < 6; i++)
    {
        data_buff[3+i] = nvrecord_env->another_addr[i]; //other ear addr
    }

    data_length = 9;
    crc_data = CRC16(data_buff, data_length);

    data_buff[data_length] = (uint8_t)(crc_data >> 8);
    data_buff[data_length+1] = (uint8_t)crc_data;

    data_length += 2;
    communication_send_buf(data_buff, data_length);
}

void app_uart_addr_change_compele_handle(uint8_t *uart_dat)
{
    uint8_t data_buff[DATA_MAX_LEN];
    uint8_t data_length = 0;
    uint16_t crc_data = 0;
    struct nvrecord_env_t *nvrecord_env;

    memset(data_buff, 0x00, DATA_MAX_LEN);

    if(((uart_dat[0]&0x0f) == UART_HEAD_LEFT)&&(besui_get_lr_sta() == LEFT_SIDE))
    {
        data_buff[0] = (UART_HEAD_LEFT << 4) | UART_HEAD_BOX;
    }
    else if(((uart_dat[0]&0x0f) == UART_HEAD_RIGHT)&&(besui_get_lr_sta() == RIGHT_SIDE))
    {
        data_buff[0] = (UART_HEAD_RIGHT << 4) | UART_HEAD_BOX;
    }
    
    data_buff[1] = uart_dat[1];
    data_buff[2] = 0x00;

    data_length = 3;
    crc_data = CRC16(data_buff, data_length);

    data_buff[3] = (uint8_t)(crc_data >> 8);
    data_buff[4] = (uint8_t)crc_data;

    data_length = 5;
    communication_send_buf(data_buff, data_length);

    if(uiuart.twspair_flag)
    {
        nv_record_env_get(&nvrecord_env);
        nvrecord_env->need_twspair_flag = true;
        nv_record_env_set(nvrecord_env);
        uictl.poweroff_fast_flag = true;
        //app_reset();
        if(uicom.bt_pairlist_clear_flag)
        {
            app_uart_factory_poweroff_timer_onoff(true);                
        }
        else
        {
            uictl.shutdown_type = SHUTDOWN_ADDR_SWAP_OK;
            app_shutdown();
        }
    }
}


void app_uart_heartbeat_handle(uint8_t *uart_dat)
{
    uint8_t data_buff[DATA_MAX_LEN];
    uint8_t data_length = 0;
    uint16_t crc_data = 0;
    uint8_t ear_battery_level = 10;

    memset(data_buff, 0x00, DATA_MAX_LEN);

    if(((uart_dat[0]&0x0f) == UART_HEAD_LEFT)&&(besui_get_lr_sta() == LEFT_SIDE))
    {
        data_buff[0] = (UART_HEAD_LEFT << 4) | UART_HEAD_BOX;
    }
    else if(((uart_dat[0]&0x0f) == UART_HEAD_RIGHT)&&(besui_get_lr_sta() == RIGHT_SIDE))
    {
        data_buff[0] = (UART_HEAD_RIGHT << 4) | UART_HEAD_BOX;
    }
    
    data_buff[1] = uart_dat[1];
    data_buff[2] = uart_dat[2];
    uicom.bat_box_percent = uart_dat[3];
    BESUI_TRACE(1,"[UIUART]%s box level 0x%02x", __func__, uicom.bat_box_percent);
    ear_battery_level = app_battery_current_level();
    BESUI_TRACE(1,"[UIUART]%s EAR level %d", __func__, ear_battery_level);
    data_buff[3] = ((ear_battery_level+1)*10);
    BESUI_TRACE(1,"[UIUART]%s EAR level 0x%02x", __func__, data_buff[3]);
    data_length = 4;
    crc_data = CRC16(data_buff, data_length);

    data_buff[4] = (uint8_t)(crc_data >> 8);
    data_buff[5] = (uint8_t)crc_data;

    data_length = 6;
    communication_send_buf(data_buff, data_length);

    if(case_open_tx_bat)
    {
        case_open_tx_bat = false;
        app_tws_battery_update(true);
    }
}


void app_uart_get_sw_version_handle(uint8_t *uart_dat)
{
    uint8_t data_buff[DATA_MAX_LEN];
    uint8_t data_length = 0;
    uint16_t crc_data = 0;

    memset(data_buff, 0x00, DATA_MAX_LEN);

    if(((uart_dat[0]&0x0f) == UART_HEAD_LEFT)&&(besui_get_lr_sta() == LEFT_SIDE))
    {
        data_buff[0] = (UART_HEAD_LEFT << 4) | UART_HEAD_BOX;
    }
    else if(((uart_dat[0]&0x0f) == UART_HEAD_RIGHT)&&(besui_get_lr_sta() == RIGHT_SIDE))
    {
        data_buff[0] = (UART_HEAD_RIGHT << 4) | UART_HEAD_BOX;
    }
    
    data_buff[1] = uart_dat[1];
    data_buff[2] = 0x03;
    data_buff[3] = app_get_software_version_high();
    data_buff[4] = app_get_software_version_middle();
    data_buff[5] = app_get_software_version_low();

    data_length = 6;
    crc_data = CRC16(data_buff, data_length);

    data_buff[6] = (uint8_t)(crc_data >> 8);
    data_buff[7] = (uint8_t)crc_data;

    data_length = 8;
    communication_send_buf(data_buff, data_length);
}


void app_uart_read_sn_handle(uint8_t *uart_dat)
{
    uint8_t data_buff[DATA_MAX_LEN];
    uint8_t data_length = 0;
    uint16_t crc_data = 0;
    uint8_t i = 0;
    uint8_t sn_len = 0;
    struct nvrecord_env_t *nvrecord_env;
    nv_record_env_get(&nvrecord_env);

    sn_len = nvrecord_env->sn_len;
    
    memset(data_buff, 0x00, DATA_MAX_LEN);

    if(((uart_dat[0]&0x0f) == UART_HEAD_LEFT)&&(besui_get_lr_sta() == LEFT_SIDE))
    {
        data_buff[0] = (UART_HEAD_LEFT << 4) | UART_HEAD_BOX;
    }
    else if(((uart_dat[0]&0x0f) == UART_HEAD_RIGHT)&&(besui_get_lr_sta() == RIGHT_SIDE))
    {
        data_buff[0] = (UART_HEAD_RIGHT << 4) | UART_HEAD_BOX;
    }
    
    data_buff[1] = uart_dat[1];
    data_buff[2] = sn_len;

    for(i = 0; i < sn_len; i++)
    {
        data_buff[3+i] = nvrecord_env->sn_data[i];
    }

    data_length = sn_len+3;
    crc_data = CRC16(data_buff, data_length);

    data_buff[sn_len+3] = (uint8_t)(crc_data >> 8);
    data_buff[sn_len+3+1] = (uint8_t)crc_data;

    data_length = sn_len+3+1+1;
    communication_send_buf(data_buff, data_length);
}

void app_uart_wirte_sn_handle(uint8_t *uart_dat)
{
    uint8_t data_buff[DATA_MAX_LEN];
    uint8_t data_length = 0;
    uint16_t crc_data = 0;
    uint8_t i = 0;
    uint8_t sn_len = uart_dat[2];
    struct nvrecord_env_t *nvrecord_env;

    memset(data_buff, 0x00, DATA_MAX_LEN);

    nv_record_env_get(&nvrecord_env);
    nvrecord_env->sn_len = sn_len;
    for(i = 0; i < sn_len; i++)
    {
        nvrecord_env->sn_data[i] = uart_dat[i+3];
    }
    nv_record_env_set(nvrecord_env);

    if(((uart_dat[0]&0x0f) == UART_HEAD_LEFT)&&(besui_get_lr_sta() == LEFT_SIDE))
    {
        data_buff[0] = (UART_HEAD_LEFT << 4) | UART_HEAD_BOX;
    }
    else if(((uart_dat[0]&0x0f) == UART_HEAD_RIGHT)&&(besui_get_lr_sta() == RIGHT_SIDE))
    {
        data_buff[0] = (UART_HEAD_RIGHT << 4) | UART_HEAD_BOX;
    }
    
    data_buff[1] = uart_dat[1];
    data_buff[2] = 0x00;

    data_length = 3;
    crc_data = CRC16(data_buff, data_length);

    data_buff[3] = (uint8_t)(crc_data >> 8);
    data_buff[4] = (uint8_t)crc_data;

    data_length = 5;
    communication_send_buf(data_buff, data_length);
}

void app_uart_get_bt_local_name(uint8_t *uart_dat) //mike
{
    uint8_t data_buff[DATA_MAX_LEN];
    uint8_t data_length = 0;
    uint16_t crc_data = 0;
    uint8_t i = 0;
    const char *localname = bt_get_local_name();
    uint8_t name_len = strlen(localname);

    memset(data_buff, 0x00, DATA_MAX_LEN);

    if(((uart_dat[0]&0x0f) == UART_HEAD_LEFT)&&(besui_get_lr_sta() == LEFT_SIDE))
    {
        data_buff[0] = (UART_HEAD_LEFT << 4) | UART_HEAD_BOX;
    }
    else if(((uart_dat[0]&0x0f) == UART_HEAD_RIGHT)&&(besui_get_lr_sta() == RIGHT_SIDE))
    {
        data_buff[0] = (UART_HEAD_RIGHT << 4) | UART_HEAD_BOX;
    }
    
    data_buff[1] = uart_dat[1];
    data_buff[2] = name_len;

    for(i = 0; i < name_len; i++)
    {
        data_buff[3+i] = localname[i];
    }

    data_length = 3+name_len;
    crc_data = CRC16(data_buff, data_length);

    data_buff[data_length] = (uint8_t)(crc_data >> 8);
    data_buff[data_length+1] = (uint8_t)crc_data;

    data_length = data_length+2;
    communication_send_buf(data_buff, data_length);
}

void app_uart_get_bt_mac(uint8_t *uart_dat) //mike
{
    uint8_t data_buff[DATA_MAX_LEN];
    uint8_t data_length = 0;
    uint16_t crc_data = 0;
    uint8_t i = 0;
    //const char *localname = bt_get_local_name();
    //uint8_t name_len = strlen(localname);    
    uint8_t *bt_mac_addr;
    // ibrt_ctrl_t *p_ibrt_ctrl = app_ibrt_if_get_bt_ctrl_ctx();
    // struct nvrecord_env_t *nvrecord_env;
    // nv_record_env_get(&nvrecord_env);

    memset(data_buff, 0x00, DATA_MAX_LEN);

    if(((uart_dat[0]&0x0f) == UART_HEAD_LEFT)&&(besui_get_lr_sta() == LEFT_SIDE))
    {
        data_buff[0] = (UART_HEAD_LEFT << 4) | UART_HEAD_BOX;
    }
    else if(((uart_dat[0]&0x0f) == UART_HEAD_RIGHT)&&(besui_get_lr_sta() == RIGHT_SIDE))
    {
        data_buff[0] = (UART_HEAD_RIGHT << 4) | UART_HEAD_BOX;
    }
    
    data_buff[1] = uart_dat[1];
    data_buff[2] = 6;

    //bt_mac_addr = bt_get_local_address();
    // DUMP8("%02x ", p_ibrt_ctrl->local_addr.address, BT_ADDR_OUTPUT_PRINT_NUM);
    // DUMP8("%02x ", p_ibrt_ctrl->peer_addr.address, BT_ADDR_OUTPUT_PRINT_NUM);
    // DUMP8("%02x ", nvrecord_env->tws_con_addr, BT_ADDR_OUTPUT_PRINT_NUM);
    // if(bts_tws_if_is_tws_link_connected())
    // {
    //     bt_mac_addr = p_ibrt_ctrl->local_addr.address;
    // }
    // else
    // {
    //     if(nvrecord_env->tws_con_addr[0]==0x00 && nvrecord_env->tws_con_addr[1]==0x00 && nvrecord_env->tws_con_addr[2]==0x00 
    //     && nvrecord_env->tws_con_addr[3]==0x00 && nvrecord_env->tws_con_addr[4]==0x00 && nvrecord_env->tws_con_addr[5]==0x00)
    //     {
    //         bt_mac_addr = p_ibrt_ctrl->local_addr.address;
    //     }
    //     else
    //     {
    //         bt_mac_addr = nvrecord_env->tws_con_addr;            
    //     }
    // }


    DUMP8("%02x ", bt_global_addr, BT_ADDR_OUTPUT_PRINT_NUM);
    bt_mac_addr = bt_global_addr;

    for(i = 0; i < 6; i++)
    {
        data_buff[3+i] = bt_mac_addr[i];
    }

    data_length = 9;
    crc_data = CRC16(data_buff, data_length);

    data_buff[data_length] = (uint8_t)(crc_data >> 8);
    data_buff[data_length+1] = (uint8_t)crc_data;

    data_length = data_length+2;
    communication_send_buf(data_buff, data_length);
}

void app_uart_get_tws_addr(uint8_t *uart_dat) //mike
{
    uint8_t data_buff[DATA_MAX_LEN];
    uint8_t data_length = 0;
    uint16_t crc_data = 0;
    uint8_t i = 0;

    ibrt_ctrl_t *p_ibrt_ctrl = app_ibrt_if_get_bt_ctrl_ctx();
    struct nvrecord_env_t *nvrecord_env;
    nv_record_env_get(&nvrecord_env);

    memset(data_buff, 0x00, DATA_MAX_LEN);

    if(((uart_dat[0]&0x0f) == UART_HEAD_LEFT)&&(besui_get_lr_sta() == LEFT_SIDE))
    {
        data_buff[0] = (UART_HEAD_LEFT << 4) | UART_HEAD_BOX;
    }
    else if(((uart_dat[0]&0x0f) == UART_HEAD_RIGHT)&&(besui_get_lr_sta() == RIGHT_SIDE))
    {
        data_buff[0] = (UART_HEAD_RIGHT << 4) | UART_HEAD_BOX;
    }
    
    data_buff[1] = uart_dat[1];
    data_buff[2] = 6;

    DUMP8("%02x ", p_ibrt_ctrl->local_addr.address, BT_ADDR_OUTPUT_PRINT_NUM);
    DUMP8("%02x ", p_ibrt_ctrl->peer_addr.address, BT_ADDR_OUTPUT_PRINT_NUM);
    DUMP8("%02x ", nvrecord_env->tws_con_addr, BT_ADDR_OUTPUT_PRINT_NUM);
	
    //have tws revord
    if(nvrecord_env->tws_con_addr[0]==0x00 && nvrecord_env->tws_con_addr[1]==0x00 && nvrecord_env->tws_con_addr[2]==0x00 
    && nvrecord_env->tws_con_addr[3]==0x00 && nvrecord_env->tws_con_addr[4]==0x00 && nvrecord_env->tws_con_addr[5]==0x00)
    {
        for(i = 0; i < 6; i++)
        {
            data_buff[3+i] = 0;
        }
    }
    else
    {
        for(i = 0; i < 6; i++)
        {
            if(besui_get_lr_sta() == LEFT_SIDE)
            {
                data_buff[3+i] = bt_global_addr[i];
            }
            else if(besui_get_lr_sta() == RIGHT_SIDE)
            {
                data_buff[3+i] = nvrecord_env->another_addr[i];
            }
        }
    }
/*
    if(bts_tws_if_is_tws_link_connected()) //fixed left addr
    {
        for(i = 0; i < 6; i++)
        {
            data_buff[3+i] = p_ibrt_ctrl->local_addr.address[i];
            if(besui_get_lr_sta() == LEFT_SIDE)
            {
                data_buff[3+i] = p_ibrt_ctrl->local_addr.address[i];
            }
            else if(besui_get_lr_sta() == RIGHT_SIDE)
            {
                data_buff[3+i] = p_ibrt_ctrl->peer_addr.address[i];
            }
        }
    }
    else
    {
        if(nvrecord_env->tws_con_addr[0]==0x00 && nvrecord_env->tws_con_addr[1]==0x00 && nvrecord_env->tws_con_addr[2]==0x00 
        && nvrecord_env->tws_con_addr[3]==0x00 && nvrecord_env->tws_con_addr[4]==0x00 && nvrecord_env->tws_con_addr[5]==0x00)
        {
            for(i = 0; i < 6; i++)
            {
                data_buff[3+i] = p_ibrt_ctrl->local_addr.address[i];
            } 
        }
        else
        {
            for(i = 0; i < 6; i++)
            {
                data_buff[3+i] = nvrecord_env->tws_con_addr[i];
            } 
        }
    }
*/

    data_length = 9;
    crc_data = CRC16(data_buff, data_length);

    data_buff[data_length] = (uint8_t)(crc_data >> 8);
    data_buff[data_length+1] = (uint8_t)crc_data;

    data_length = data_length+2;
    communication_send_buf(data_buff, data_length);
}

void app_uart_get_ear_det(uint8_t *uart_dat) //mike
{
    uint8_t data_buff[DATA_MAX_LEN];
    uint8_t data_length = 0;
    uint16_t crc_data = 0;

    memset(data_buff, 0x00, DATA_MAX_LEN);

    if(((uart_dat[0]&0x0f) == UART_HEAD_LEFT)&&(besui_get_lr_sta() == LEFT_SIDE))
    {
        data_buff[0] = (UART_HEAD_LEFT << 4) | UART_HEAD_BOX;
    }
    else if(((uart_dat[0]&0x0f) == UART_HEAD_RIGHT)&&(besui_get_lr_sta() == RIGHT_SIDE))
    {
        data_buff[0] = (UART_HEAD_RIGHT << 4) | UART_HEAD_BOX;
    }
    
    data_buff[1] = uart_dat[1];
    data_buff[2] = 1;

#if defined(CAPSENSOR_ENABLE)
    data_buff[3] = app_inear_get_status();
#else
    data_buff[3] = 0;
#endif

    data_length = 4;
    crc_data = CRC16(data_buff, data_length);

    data_buff[data_length] = (uint8_t)(crc_data >> 8);
    data_buff[data_length+1] = (uint8_t)crc_data;

    data_length = data_length+2;
    communication_send_buf(data_buff, data_length);
}

void app_uart_ear_poweroff(uint8_t *uart_dat) //mike
{
    uint8_t data_buff[DATA_MAX_LEN];
    uint8_t data_length = 0;

    memset(data_buff, 0x00, DATA_MAX_LEN);

    if(((uart_dat[0]&0x0f) == UART_HEAD_LEFT)&&(besui_get_lr_sta() == LEFT_SIDE))
    {
        data_buff[0] = (UART_HEAD_LEFT << 4) | UART_HEAD_BOX;
        data_buff[3] = 0x12; //crc
        data_buff[4] = 0x09;
    }
    else if(((uart_dat[0]&0x0f) == UART_HEAD_RIGHT)&&(besui_get_lr_sta() == RIGHT_SIDE))
    {
        data_buff[0] = (UART_HEAD_RIGHT << 4) | UART_HEAD_BOX;
        data_buff[3] = 0x51; //crc
        data_buff[4] = 0x6A;
    }
    
    data_buff[1] = uart_dat[1];

    data_length = 5;

    // uint16_t crc_data = 0;
    // crc_data = CRC16(data_buff, data_length);

    // data_buff[data_length] = (uint8_t)(crc_data >> 8);
    // data_buff[data_length+1] = (uint8_t)crc_data;

    // data_length = data_length+2;

    communication_send_buf(data_buff, data_length);

    uictl.poweroff_fast_flag = true;
    uictl.shutdown_type = SHUTDOWN_BOX_CMD;
    app_shutdown();
}

void app_uart_ear_close(uint8_t *uart_dat) //mike
{
    uint8_t data_buff[DATA_MAX_LEN];
    uint8_t data_length = 0;

    memset(data_buff, 0x00, DATA_MAX_LEN);

    if(((uart_dat[0]&0x0f) == UART_HEAD_LEFT)&&(besui_get_lr_sta() == LEFT_SIDE))
    {
        data_buff[0] = (UART_HEAD_LEFT << 4) | UART_HEAD_BOX;
        data_buff[3] = 0xC3; //crc
        data_buff[4] = 0xB3;
    }
    else if(((uart_dat[0]&0x0f) == UART_HEAD_RIGHT)&&(besui_get_lr_sta() == RIGHT_SIDE))
    {
        data_buff[0] = (UART_HEAD_RIGHT << 4) | UART_HEAD_BOX;
        data_buff[3] = 0x80; //crc
        data_buff[4] = 0xD0;
    }
    
    data_buff[1] = uart_dat[1];

    data_length = 5;

    // uint16_t crc_data = 0;
    // crc_data = CRC16(data_buff, data_length);

    // data_buff[data_length] = (uint8_t)(crc_data >> 8);
    // data_buff[data_length+1] = (uint8_t)crc_data;

    // data_length = data_length+2;

    communication_send_buf(data_buff, data_length);

#ifdef GFPS_ENABLED
    gfps_set_battery_datatype(HIDE_UI_INDICATION);
#endif
}

#ifdef BESUI_CAPSENSOR_FACTORY_EN
void app_uart_capsensor_test(uint8_t *uart_dat) //mike
{
    uint8_t data_buff[DATA_MAX_LEN];
    uint8_t data_length = 0;

    memset(data_buff, 0x00, DATA_MAX_LEN);

    if(((uart_dat[0]&0x0f) == UART_HEAD_LEFT)&&(besui_get_lr_sta() == LEFT_SIDE))
    {
        data_buff[0] = (UART_HEAD_LEFT << 4) | UART_HEAD_BOX;
        data_buff[3] = 0xF0; //crc
        data_buff[4] = 0x82;
    }
    else if(((uart_dat[0]&0x0f) == UART_HEAD_RIGHT)&&(besui_get_lr_sta() == RIGHT_SIDE))
    {
        data_buff[0] = (UART_HEAD_RIGHT << 4) | UART_HEAD_BOX;
        data_buff[3] = 0xB3; //crc
        data_buff[4] = 0xE1;
    }
    
    data_buff[1] = uart_dat[1];

    data_length = 5;

    // uint16_t crc_data = 0;
    // crc_data = CRC16(data_buff, data_length);

    // data_buff[data_length] = (uint8_t)(crc_data >> 8);
    // data_buff[data_length+1] = (uint8_t)crc_data;

    // data_length = data_length+2;

    communication_send_buf(data_buff, data_length);


#ifdef CAPSENSOR_ENABLE
    uictl.capsensor_uart_factory_mode = true;
/*     str_2_int(cmd_data);
    capsensor_set_wear_cal_range_buf(at_dat);
    capsensor_set_touch_cal_range_buf((int *)(at_dat+8)); */
    capsensor_restart(true);
    capsensor_factory_calculate_flag_set(1, 0); 
#endif
}
void app_uart_capsensor_test_result(uint8_t *uart_dat) //mike
{
    uint8_t data_buff[DATA_MAX_LEN];
    uint8_t data_length = 0;

    memset(data_buff, 0x00, DATA_MAX_LEN);

    data_buff[3] = capsensor_offset_get_result();
    if(((uart_dat[0]&0x0f) == UART_HEAD_LEFT)&&(besui_get_lr_sta() == LEFT_SIDE))
    {
        data_buff[0] = (UART_HEAD_LEFT << 4) | UART_HEAD_BOX;
        if(data_buff[3])
        {
            data_buff[4] = 0x17;
            data_buff[5] = 0x5F;
        }
        else
        {
            data_buff[4] = 0x07;
            data_buff[5] = 0x7E;
        } 
    }
    else if(((uart_dat[0]&0x0f) == UART_HEAD_RIGHT)&&(besui_get_lr_sta() == RIGHT_SIDE))
    {
        data_buff[0] = (UART_HEAD_RIGHT << 4) | UART_HEAD_BOX;
        if(data_buff[3])
        {
            data_buff[4] = 0x0C;
            data_buff[5] = 0xF8;
        }
        else
        {
            data_buff[4] = 0x1C;
            data_buff[5] = 0xD9;
        } 
    }
    
    data_buff[1] = uart_dat[1];
    data_buff[2] = 0x01;

    data_length = 6;

    // uint16_t crc_data = 0;
    // crc_data = CRC16(data_buff, data_length);

    // data_buff[data_length] = (uint8_t)(crc_data >> 8);
    // data_buff[data_length+1] = (uint8_t)crc_data;

    // data_length = data_length+2;

    communication_send_buf(data_buff, data_length);
}

#endif //#ifdef BESUI_CAPSENSOR_FACTORY_EN

void app_uart_capsensor_onoff_control(uint8_t *uart_dat)
{
    uint8_t data_buff[DATA_MAX_LEN];
    uint8_t data_length = 0;
    uint16_t crc_data = 0;

    memset(data_buff, 0x00, DATA_MAX_LEN);

    if(((uart_dat[0]&0x0f) == UART_HEAD_LEFT)&&(besui_get_lr_sta() == LEFT_SIDE))
    {
        data_buff[0] = (UART_HEAD_LEFT << 4) | UART_HEAD_BOX;
    }
    else if(((uart_dat[0]&0x0f) == UART_HEAD_RIGHT)&&(besui_get_lr_sta() == RIGHT_SIDE))
    {
        data_buff[0] = (UART_HEAD_RIGHT << 4) | UART_HEAD_BOX;
    }
    
    data_buff[1] = uart_dat[1];
    data_buff[2] = uart_dat[2];
    data_buff[3] = uart_dat[3];
    data_length = 3;
    crc_data = CRC16(data_buff, data_length);

    data_buff[4] = (uint8_t)(crc_data >> 8);
    data_buff[5] = (uint8_t)crc_data;

    data_length = 6;
    communication_send_buf(data_buff, data_length);
#ifdef CAPSENSOR_ENABLE
    if(uart_dat[3] == 0x00) //off capsensor
        uictl.capsensor_onoff = 0xA5;
    else
        uictl.capsensor_onoff = 0;
#endif
    //BESUI_TRACE(0,"[%s], uictl.capsensor_onoff = %d",__func__, uictl.capsensor_onoff);        
}

void app_uart_read_color_handle(uint8_t *uart_dat)
{
    uint8_t data_buff[DATA_MAX_LEN];
    uint8_t data_length = 0;
    uint16_t crc_data = 0;

    struct nvrecord_env_t *nvrecord_env;
    nv_record_env_get(&nvrecord_env);
    
    memset(data_buff, 0x00, DATA_MAX_LEN);

    if(((uart_dat[0]&0x0f) == UART_HEAD_LEFT)&&(besui_get_lr_sta() == LEFT_SIDE))
    {
        data_buff[0] = (UART_HEAD_LEFT << 4) | UART_HEAD_BOX;
    }
    else if(((uart_dat[0]&0x0f) == UART_HEAD_RIGHT)&&(besui_get_lr_sta() == RIGHT_SIDE))
    {
        data_buff[0] = (UART_HEAD_RIGHT << 4) | UART_HEAD_BOX;
    }
    
    data_buff[1] = uart_dat[1];
    data_buff[2] = 0x01;
    data_buff[3] = nvrecord_env->color_data;

    data_length = 4;
    crc_data = CRC16(data_buff, data_length);

    data_buff[4] = (uint8_t)(crc_data >> 8);
    data_buff[5] = (uint8_t)crc_data;

    data_length = 6;
    communication_send_buf(data_buff, data_length);
}

void app_uart_wirte_color_handle(uint8_t *uart_dat)
{
    uint8_t data_buff[DATA_MAX_LEN];
    uint8_t data_length = 0;
    uint16_t crc_data = 0;

    struct nvrecord_env_t *nvrecord_env;
    nv_record_env_get(&nvrecord_env);

    memset(data_buff, 0x00, DATA_MAX_LEN);

    nvrecord_env->color_data = uart_dat[3];
    nv_record_env_set(nvrecord_env);

    if(((uart_dat[0]&0x0f) == UART_HEAD_LEFT)&&(besui_get_lr_sta() == LEFT_SIDE))
    {
        data_buff[0] = (UART_HEAD_LEFT << 4) | UART_HEAD_BOX;
        data_buff[3] = 0x69;
        data_buff[4] = 0x15;
    }
    else if(((uart_dat[0]&0x0f) == UART_HEAD_RIGHT)&&(besui_get_lr_sta() == RIGHT_SIDE))
    {
        data_buff[0] = (UART_HEAD_RIGHT << 4) | UART_HEAD_BOX;
        data_buff[3] = 0x2A;
        data_buff[4] = 0x76;
    }
    
    data_buff[1] = uart_dat[1];
    data_buff[2] = 0x00;

    data_length = 5;
    crc_data = CRC16(data_buff, data_length);

    data_buff[data_length] = (uint8_t)(crc_data >> 8);
    data_buff[data_length+1] = (uint8_t)crc_data;

    data_length = data_length+2;
    communication_send_buf(data_buff, data_length);
}


void app_uart_set_enter_ota_updatemode_handle(uint8_t *uart_dat)
{
    uint8_t data_buff[DATA_MAX_LEN];
    uint8_t data_length = 0;
    uint16_t crc_data = 0;

    memset(data_buff, 0x00, DATA_MAX_LEN);

    if(((uart_dat[0]&0x0f) == UART_HEAD_LEFT)&&(besui_get_lr_sta() == LEFT_SIDE))
    {
        data_buff[0] = (UART_HEAD_LEFT << 4) | UART_HEAD_BOX;
    }
    else if(((uart_dat[0]&0x0f) == UART_HEAD_RIGHT)&&(besui_get_lr_sta() == RIGHT_SIDE))
    {
        data_buff[0] = (UART_HEAD_RIGHT << 4) | UART_HEAD_BOX;
    }
    
    data_buff[1] = uart_dat[1];
    data_buff[2] = uart_dat[2];

    data_length = 3;
    crc_data = CRC16(data_buff, data_length);

    data_buff[3] = (uint8_t)(crc_data >> 8);
    data_buff[4] = (uint8_t)crc_data;

    data_length = 5;
    communication_send_buf(data_buff, data_length);

    uiuart.event_type = UART_EVENT_OTA_UPGRADE;
    app_uart_event_timer_onoff(true);
}


void app_uart_set_enter_dut_mode_handle(uint8_t *uart_dat)
{
    uint8_t data_buff[DATA_MAX_LEN];
    uint8_t data_length = 0;
    uint16_t crc_data = 0;

    memset(data_buff, 0x00, DATA_MAX_LEN);

    if(((uart_dat[0]&0x0f) == UART_HEAD_LEFT)&&(besui_get_lr_sta() == LEFT_SIDE))
    {
        data_buff[0] = (UART_HEAD_LEFT << 4) | UART_HEAD_BOX;
    }
    else if(((uart_dat[0]&0x0f) == UART_HEAD_RIGHT)&&(besui_get_lr_sta() == RIGHT_SIDE))
    {
        data_buff[0] = (UART_HEAD_RIGHT << 4) | UART_HEAD_BOX;
    }
    
    data_buff[1] = uart_dat[1];
    data_buff[2] = uart_dat[2];

    data_length = 3;
    crc_data = CRC16(data_buff, data_length);

    data_buff[3] = (uint8_t)(crc_data >> 8);
    data_buff[4] = (uint8_t)crc_data;

    data_length = 5;
    communication_send_buf(data_buff, data_length);

    uiuart.event_type = UART_EVENT_ENTER_DUT;
    app_uart_event_timer_onoff(true);
}


void app_uart_set_enter_btpairmode_handle(uint8_t *uart_dat)
{
    uint8_t data_buff[DATA_MAX_LEN];
    uint8_t data_length = 0;
    uint16_t crc_data = 0;

    memset(data_buff, 0x00, DATA_MAX_LEN);

    if(((uart_dat[0]&0x0f) == UART_HEAD_LEFT)&&(besui_get_lr_sta() == LEFT_SIDE))
    {
        data_buff[0] = (UART_HEAD_LEFT << 4) | UART_HEAD_BOX;
    }
    else if(((uart_dat[0]&0x0f) == UART_HEAD_RIGHT)&&(besui_get_lr_sta() == RIGHT_SIDE))
    {
        data_buff[0] = (UART_HEAD_RIGHT << 4) | UART_HEAD_BOX;
    }
    
    data_buff[1] = uart_dat[1];
    data_buff[2] = uart_dat[2];

    data_length = 3;
    crc_data = CRC16(data_buff, data_length);

    data_buff[3] = (uint8_t)(crc_data >> 8);
    data_buff[4] = (uint8_t)crc_data;

    data_length = 5;
    communication_send_buf(data_buff, data_length);

    uiuart.event_type = UART_EVENT_ENTER_BT_MODE;
    app_uart_event_timer_onoff(true);
}


void app_uart_clear_btpairlist_handle(uint8_t *uart_dat)
{
    uint8_t data_buff[DATA_MAX_LEN];
    uint8_t data_length = 0;
    uint16_t crc_data = 0;

    memset(data_buff, 0x00, DATA_MAX_LEN);

    if(((uart_dat[0]&0x0f) == UART_HEAD_LEFT)&&(besui_get_lr_sta() == LEFT_SIDE))
    {
        data_buff[0] = (UART_HEAD_LEFT << 4) | UART_HEAD_BOX;
    }
    else if(((uart_dat[0]&0x0f) == UART_HEAD_RIGHT)&&(besui_get_lr_sta() == RIGHT_SIDE))
    {
        data_buff[0] = (UART_HEAD_RIGHT << 4) | UART_HEAD_BOX;
    }
    
    data_buff[1] = uart_dat[1];
    data_buff[2] = uart_dat[2];

    data_length = 3;
    crc_data = CRC16(data_buff, data_length);
    data_buff[3] = (uint8_t)(crc_data >> 8);
    data_buff[4] = (uint8_t)crc_data;

    data_length = 5;
    communication_send_buf(data_buff, data_length);

    uiuart.event_type = UART_EVENT_CLEAR_BTPAIRLIST;
    app_uart_event_timer_onoff(true);
}


void app_uart_clear_bttwspairlist_handle(uint8_t *uart_dat)
{
    uint8_t data_buff[DATA_MAX_LEN];
    uint8_t data_length = 0;
    uint16_t crc_data = 0;

    memset(data_buff, 0x00, DATA_MAX_LEN);

    if(((uart_dat[0]&0x0f) == UART_HEAD_LEFT)&&(besui_get_lr_sta() == LEFT_SIDE))
    {
        data_buff[0] = (UART_HEAD_LEFT << 4) | UART_HEAD_BOX;
    }
    else if(((uart_dat[0]&0x0f) == UART_HEAD_RIGHT)&&(besui_get_lr_sta() == RIGHT_SIDE))
    {
        data_buff[0] = (UART_HEAD_RIGHT << 4) | UART_HEAD_BOX;
    }
    
    data_buff[1] = uart_dat[1];
    data_buff[2] = uart_dat[2];

    data_length = 3;
    crc_data = CRC16(data_buff, data_length);
    data_buff[3] = (uint8_t)(crc_data >> 8);
    data_buff[4] = (uint8_t)crc_data;

    data_length = 5;
    communication_send_buf(data_buff, data_length);

    uiuart.event_type = UART_EVENT_CLEAR_BTTWSPAIRLIST;
    app_uart_event_timer_onoff(true);
}

void app_uart_set_enter_single_update_mode_handle(uint8_t *uart_dat)
{
    uint8_t data_buff[DATA_MAX_LEN];
    uint8_t data_length = 0;
    uint16_t crc_data = 0;

    memset(data_buff, 0x00, DATA_MAX_LEN);

    if(((uart_dat[0]&0x0f) == UART_HEAD_LEFT)&&(besui_get_lr_sta() == LEFT_SIDE))
    {
        data_buff[0] = (UART_HEAD_LEFT << 4) | UART_HEAD_BOX;
    }
    else if(((uart_dat[0]&0x0f) == UART_HEAD_RIGHT)&&(besui_get_lr_sta() == RIGHT_SIDE))
    {
        data_buff[0] = (UART_HEAD_RIGHT << 4) | UART_HEAD_BOX;
    }
    
    data_buff[1] = uart_dat[1];
    data_buff[2] = uart_dat[2];

    data_length = 3;
    crc_data = CRC16(data_buff, data_length);
    data_buff[3] = (uint8_t)(crc_data >> 8);
    data_buff[4] = (uint8_t)crc_data;

    data_length = 5;
    communication_send_buf(data_buff, data_length);

    uiuart.event_type = UART_EVENT_1WIRE_UPGRADE;
    app_uart_event_timer_onoff(true);
}

void app_uart_set_enter_ship_mode_handle(uint8_t *uart_dat)
{
    uint8_t data_buff[DATA_MAX_LEN];
    uint8_t data_length = 0;
    uint16_t crc_data = 0;

    memset(data_buff, 0x00, DATA_MAX_LEN);

    if(((uart_dat[0]&0x0f) == UART_HEAD_LEFT)&&(besui_get_lr_sta() == LEFT_SIDE))
    {
        data_buff[0] = (UART_HEAD_LEFT << 4) | UART_HEAD_BOX;
    }
    else if(((uart_dat[0]&0x0f) == UART_HEAD_RIGHT)&&(besui_get_lr_sta() == RIGHT_SIDE))
    {
        data_buff[0] = (UART_HEAD_RIGHT << 4) | UART_HEAD_BOX;
    }
    
    data_buff[1] = uart_dat[1];
    data_buff[2] = uart_dat[2];

    data_length = 3;
    crc_data = CRC16(data_buff, data_length);
    data_buff[3] = (uint8_t)(crc_data >> 8);
    data_buff[4] = (uint8_t)crc_data;

    data_length = 5;
    communication_send_buf(data_buff, data_length);

    uiuart.event_type = UART_EVENT_ENTER_SHIPMODE;

    app_uart_event_timer_onoff(true);
}

void app_uart_get_imu_id(uint8_t *uart_dat)
{
    uint8_t data_buff[DATA_MAX_LEN];
    uint8_t data_length = 0;
    uint16_t crc_data = 0;

    memset(data_buff, 0x00, DATA_MAX_LEN);

    if(((uart_dat[0]&0x0f) == UART_HEAD_LEFT)&&(besui_get_lr_sta() == LEFT_SIDE))
    {
        data_buff[0] = (UART_HEAD_LEFT << 4) | UART_HEAD_BOX;
    }
    else if(((uart_dat[0]&0x0f) == UART_HEAD_RIGHT)&&(besui_get_lr_sta() == RIGHT_SIDE))
    {
        data_buff[0] = (UART_HEAD_RIGHT << 4) | UART_HEAD_BOX;
    }
    
    data_buff[1] = uart_dat[1];
    data_buff[2] = 0x01;
    // data_buff[3] = uictl.sensor_id; //sensor ID
    data_length = 4;
    crc_data = CRC16(data_buff, data_length);

    data_buff[4] = (uint8_t)(crc_data >> 8);
    data_buff[5] = (uint8_t)crc_data;

    data_length = 6;
    communication_send_buf(data_buff, data_length);
}

void app_uart_event_timehandler(void const *param)
{
    BESUI_TRACE(1,"[UIUART]%s uiuart.event_type %d", __func__, uiuart.event_type);

    switch(uiuart.event_type)
    {
        case UART_EVENT_OTA_UPGRADE:
            app_uart_post_msg(0x01, UART_EVENT_OTA_UPGRADE);
            break;

        case UART_EVENT_ENTER_DUT:
            app_uart_post_msg(0x01, UART_EVENT_ENTER_DUT);
            break;

        case UART_EVENT_1WIRE_UPGRADE:
            app_uart_post_msg(0x01, UART_EVENT_1WIRE_UPGRADE);
            break;

        case UART_EVENT_ENTER_SHIPMODE:
            app_uart_post_msg(0x01, UART_EVENT_ENTER_SHIPMODE);
            break;

        case UART_EVENT_ENTER_BT_MODE:
            app_uart_post_msg(0x01, UART_EVENT_ENTER_BT_MODE);
            break;

        case UART_EVENT_CLEAR_BTPAIRLIST:
            app_uart_post_msg(0x01, UART_EVENT_CLEAR_BTPAIRLIST);
            break;

        case UART_EVENT_CLEAR_BTTWSPAIRLIST:
            app_uart_post_msg(0x01, UART_EVENT_CLEAR_BTTWSPAIRLIST);
            break;

        case UART_EVENT_SW_RESET:
            app_uart_post_msg(0x01, UART_EVENT_SW_RESET);
            break;

        case UART_EVENT_BT_PAIRMODE:
            app_uart_post_msg(0x01, UART_EVENT_BT_PAIRMODE);
            break;

        default:

            break;
    }

    uiuart.event_type = 0xFF;
}

osTimerId app_uart_event_timer_id = NULL;
osTimerDef (APP_UART_EVENT_TIMER_NAME, (void (*)(void const *))app_uart_event_timehandler);
void app_uart_event_timer_onoff(bool timer_en)
{
	BESUI_TRACE(1, "[UIUART]%s timer_en %d ", __func__, timer_en);

	if(app_uart_event_timer_id == NULL)
    	app_uart_event_timer_id = osTimerCreate(osTimer(APP_UART_EVENT_TIMER_NAME),osTimerOnce,NULL);    

	if(timer_en)
        osTimerStart(app_uart_event_timer_id, 100);
	else
		osTimerStop(app_uart_event_timer_id);
}

void app_uart_post_msg(uint32_t message_id, uint32_t param0) //receive uart msg start process
{
    BESUI_TRACE(0, "[UIUART][%s]", __func__);
    int status;
    APP_MESSAGE_BLOCK msg;

    msg.mod_id = APP_MUDUAL_UART_MSG;
#if defined(USE_BASIC_THREADS)
    msg.mod_level = APP_MOD_LEVEL_1;
#endif
    msg.msg_body.message_id = message_id;           //algo_id
    msg.msg_body.message_Param0 = param0;           //algo_onoff

    BESUI_TRACE(3,"[UIUART][%s], %d, %d", __func__, message_id, param0);

    status = app_mailbox_put(&msg);
    if(status)
        BESUI_TRACE(0,"[UIUART]app_mailbox_put error");
    else
        BESUI_TRACE(0,"[UIUART]app_mailbox_put ok");
}
static int app_uart_msg_handle_process(APP_MESSAGE_BODY *msg_body)
{
    BESUI_TRACE(3, "[UIUART][%s] %d, %d", __func__, msg_body->message_id, msg_body->message_Param0);
    if(msg_body->message_id == 0x01)
    {
        if(msg_body->message_Param0 == UART_EVENT_OTA_UPGRADE)
        {
            app_key_gui_to_otaboot_or_single(true, false);          
        }
        else if(msg_body->message_Param0 == UART_EVENT_ENTER_DUT)
        {
            app_key_gui_to_dut_test();
        }
        else if(msg_body->message_Param0 == UART_EVENT_1WIRE_UPGRADE)
        {
            app_key_gui_to_otaboot_or_single(false, true);
        }
        else if(msg_body->message_Param0 == UART_EVENT_ENTER_SHIPMODE)
        {
            uicom.ship_mode_flag = true;
            uictl.poweroff_fast_flag = true;
            uictl.shutdown_type = SHUTDOWN_SHIP_MODE;
            app_shutdown();
        }
        else if(msg_body->message_Param0 == UART_EVENT_ENTER_BT_MODE)
        {
            app_discon_enter_pairmode();
        }
        else if(msg_body->message_Param0 == UART_EVENT_CLEAR_BTPAIRLIST)
        {
            app_remove_all_phone_paired_list();
        }
        else if(msg_body->message_Param0 == UART_EVENT_CLEAR_BTTWSPAIRLIST) //factory reset
        {
            app_remove_all_paired_list();
            app_uart_factory_poweroff_timer_onoff(true);
        }
        else if(msg_body->message_Param0 == UART_EVENT_SW_RESET)
        {
            uictl.shutdown_type = SHUTDOWN_BOX_RST;
            uictl.poweroff_fast_flag = true;
            app_reset();
        }
        else if(msg_body->message_Param0 == UART_EVENT_BT_PAIRMODE)
        {
            app_discon_enter_pairmode();
        }        
    }
    return 0;
}

void app_uart_threadhandle_init(void)
{
    BESUI_TRACE(0, "[UIUART][%s]", __func__);
    app_set_threadhandle(APP_MUDUAL_UART_MSG, app_uart_msg_handle_process);
}

void app_uart_communication_cmd_handle_process(uint8_t *uart_cmd_dat, uint8_t uart_dat_len)
{
    if(bt_test_mode_dut_get())
    {
        return;
    }

    uint8_t cmd_event = 0xff; //uart_cmd_dat[1];
    uint16_t crc_dat = 0;
    //need handle
    //uint8_t l_r_side = uart_cmd_dat[2];

    /* no need
    if((besui_get_lr_sta() == RIGHT_SIDE)&&(l_r_side == LEFT_SIDE))
    {
        if(cmd_event == 2)
        {
            cmd_event++;
        }
    }
    */
    if(uart_dat_len >= 2)
    {
        cmd_event = uart_cmd_dat[1];
        crc_dat = crc_dat | uart_cmd_dat[uart_dat_len-2];
        crc_dat = (crc_dat<<8) | uart_cmd_dat[uart_dat_len-1];

        BESUI_TRACE(1,"[UIUART]%s crc dat 0x%04x", __func__, crc_dat);

        if(crc_dat == CRC16(uart_cmd_dat, uart_dat_len-2))
        {
            BESUI_TRACE(0,"[UIUART]crc OK");
        }
        else
        {
            BESUI_TRACE(0,"[UIUART]crc fail");
            return;
        }
    }
    else
    {
        BESUI_TRACE(0,"[UIUART]shuju tai shao");
        return;
    }
	/*
    if(((uart_cmd_dat[0]&0x0f) == UART_HEAD_LEFT)&&(besui_get_lr_sta() == RIGHT_SIDE))
    {
        BESUI_TRACE(0,"[UIUART]right side do not need replay left cmd");
        return;
    }

    if(((uart_cmd_dat[0]&0x0f) == UART_HEAD_RIGHT)&&(besui_get_lr_sta() == LEFT_SIDE))
    {
        BESUI_TRACE(0,"[UIUART]left side do not need replay right cmd");
        return;
    }
	*/
    BESUI_TRACE(0,"[UIUART]%s, cmd_event=0x%02X", __func__, cmd_event);

    switch(cmd_event)
    {
        case BOX_CMD_CASE_OPEN:
            app_uart_open_charge_box_handle(uart_cmd_dat);
            break;

        case BOX_CMD_CASE_CLOSE:
            app_uart_close_charge_box_handle(uart_cmd_dat);
            break;

        case BOX_CMD_GET_EAR_BATLEVEL:
            app_uart_get_ear_battery_level_handle(uart_cmd_dat);
            break;

        case BOX_CMD_SW_RESET:
            app_uart_sw_reset_handle(uart_cmd_dat);
            break;

        case BOX_CMD_BT_PAIRMODE:
            app_uart_bt_pairmode_handle(uart_cmd_dat);
            break;

        case BOX_CMD_GET_CURR_ADDR:
            app_uart_get_ear_addr_handle(uart_cmd_dat);
            break;

        case BOX_CMD_GET_PEER_ADDR:
            app_uart_get_other_ear_addr_handle(uart_cmd_dat);
            break;

        case BOX_CMD_TX_PEER_ADDDR:
            app_uart_tx_peer_addr(uart_cmd_dat);
            break;

        case BOX_CMD_ADDR_SWAP_OK:
            app_uart_addr_change_compele_handle(uart_cmd_dat);
            break;
        
        case BOX_CMD_HEARTBEAT:
            app_uart_heartbeat_handle(uart_cmd_dat);
            break;

        case BOX_CMD_GET_SW_VERSION:
            app_uart_get_sw_version_handle(uart_cmd_dat);
            break;

        case BOX_CMD_READ_SN:
            app_uart_read_sn_handle(uart_cmd_dat);
            break;

        case BOX_CMD_WRITE_SN:
            app_uart_wirte_sn_handle(uart_cmd_dat);
            break;
        
        case BOX_CMD_OTA_UPGRADE:
            app_uart_set_enter_ota_updatemode_handle(uart_cmd_dat);
            break;

        case BOX_CMD_ENTER_DUT:
            app_uart_set_enter_dut_mode_handle(uart_cmd_dat);
            break;

        case BOX_CMD_ENTER_BTMODE:
            app_uart_set_enter_btpairmode_handle(uart_cmd_dat);
            break;

        case BOX_CMD_CLEAR_BTPAIRLIST:
            app_uart_clear_btpairlist_handle(uart_cmd_dat);
            break;

        case BOX_CMD_CLEAR_BTTWSPAIRLIST:
            app_uart_clear_bttwspairlist_handle(uart_cmd_dat);
            break;

        case BOX_CMD_1WIRE_UPGRADE:
            app_uart_set_enter_single_update_mode_handle(uart_cmd_dat);
            break;

        case BOX_CMD_ENTER_SHIPMODE:
            app_uart_set_enter_ship_mode_handle(uart_cmd_dat);
            break;

//------------------------------------------------------------------------------------------
        case BOX_CMD_GET_BT_NAME:
        app_uart_get_bt_local_name(uart_cmd_dat);
            break;
        case BOX_CMD_GET_MAC_ADDR:
        //app_uart_get_right_btaddr_process(uart_cmd_dat);
        app_uart_get_bt_mac(uart_cmd_dat);
            break;
        case BOX_CMD_GET_MASTER_ADDR:
        app_uart_get_tws_addr(uart_cmd_dat);
            break;
        case BOX_CMD_WRITE_COLOR:
        app_uart_wirte_color_handle(uart_cmd_dat);
            break;
        case BOX_CMD_READ_COLOR:
        app_uart_read_color_handle(uart_cmd_dat);
            break;
        case BOX_CMD_GET_WEAR_STA:
        app_uart_get_ear_det(uart_cmd_dat);
            break;
        case BOX_CMD_POWEROFF:
        app_uart_ear_poweroff(uart_cmd_dat);
            break;
        case BOX_CMD_PRE_CASE_CLOSE:
        app_uart_ear_close(uart_cmd_dat);
            break;
#ifdef BESUI_CAPSENSOR_FACTORY_EN
        case BOX_CMD_CAP_TEST:
        app_uart_capsensor_test(uart_cmd_dat);
            break;
        case BOX_CMD_CAP_TEST_RETURN:
        app_uart_capsensor_test_result(uart_cmd_dat);
            break;            
#endif
        case BOX_CMD_CAP_ONOFF:
        app_uart_capsensor_onoff_control(uart_cmd_dat);
            break;

        case BOX_CMD_GET_IMU_ID:
        app_uart_get_imu_id(uart_cmd_dat);
            break;
//------------------------------------------------------------------------------------------
        /*
        case OPEN_CLOSE_CHARGE_BOX:
            app_uart_open_box_get_box_battery(uart_cmd_dat);
            break;

        //triple click tws change addr to tws pair
        case GET_RIGHT_ADDR:
            app_uart_get_right_btaddr_process(uart_cmd_dat);
            break;

        case SEND_ADDR_TO_LEFT:
            app_uart_send_right_addr_to_left_process(uart_cmd_dat);
            break;

        case SEMD_LEFT_ADDR_TO_RIGHT:
            app_uart_send_left_addr_to_right_process(uart_cmd_dat);
            break;
        
        case TWS_NEED_CONNECTED:
            app_uart_need_twspair_process(uart_cmd_dat);
            break;

        case ENTER_OTA_MODE:
            app_key_gui_to_otaboot_or_single(true, false);
            break;

        case ENTER_DUT_MODE:
            app_key_gui_to_dut_test();
            break;

        case CLEAR_PAIRLIST:
            app_common_clear_pairlist_process(false, true, true, true);
            break;

        case GET_SOFTWARE_VERSION:
            
            break;
        /*/
        default:

            break;
    }
}

static int app_uart_communication_handle_process(APP_MESSAGE_BODY *msg_body)
{
   uint8_t uart_data_len = 0;

    BESUI_TRACE(0,"[UIUART]%s need deal data:", __func__);
    uart_data_len = (uint8_t)msg_body->message_Param2;
    DUMP8("%02x ", uiuart.rx_dat, uart_data_len);

    app_uart_communication_cmd_handle_process(uiuart.rx_dat, uart_data_len);

    return 0;
}

#endif //#ifdef BESUI_1WIRE_EN


#endif
