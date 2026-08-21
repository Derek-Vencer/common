#ifdef BESUI_TWS_EN
#ifdef BESUI_BTMSG_EN
#include "twsui_btmsg.h"
#include "app_thread.h"
#include "apps.h"
#include "tgt_hardware.h"
#include "app_tws_ibrt.h"
#include "app_bt.h"
#include "app_ibrt_internal.h"
#include "earbud_ux_api.h"
#include "app_media_player.h"
#include "app_ibrt_customif_cmd.h"
#include "app_ui_param_config.h"
#include "app_ibrt_configure.h"
#include "audio_policy.h"
#include "bts_tws_if.h"
#include "bts_core_if.h"

#include "twsui_comm.h"
#include "twsui_charge.h"
#include "twsui_key.h"
#ifdef BESUI_1WIRE_EN
#include "twsui_uart.h"
#endif
#ifdef GFPS_ENABLED
#include "gfps_ble.h"
#endif
#include "besbt.h"
#ifdef BESUI_APP_EN
#include "app_tota_general.h"
#include "app_tota.h"
#endif
#include "nvrecord_env.h"
#include "ota_spp.h"
#include "bes_gap_api.h"

#if defined(CAPSENSOR_ENABLE)
#include "app_capsensor.h"
#endif

#if defined(ANC_APP)
#include "app_anc.h"
#endif

#include "besui_common.h"

ibrt_mobile_open_reconnect_info mobile_open_reconnect_info[2];
ibrt_mobile_reconnect_info mobile_reconnect_info[2];
ibrt_mobile_connect_info mobile_connect_info[2];
bool app_bt_system_phone_cancel_enter_pairmode_flag = false;

/*
 * The mobile profile can be restored before the TWS BESAUD command channel
 * becomes usable.  Playing AUD_ID_BT_CONNECTED immediately in that window
 * makes VOICE_REPORT_START/LET_PEER_PLAY_PROMPT fail with "tws link missing".
 * Delay the request for 2.5 seconds, then retry while TWS/role setup settles.
 */
#define NTT_BT_CONNECTED_PROMPT_DELAY_MS  3000
#define NTT_BT_CONNECTED_PROMPT_RETRY_MS  200
#define NTT_BT_CONNECTED_PROMPT_MAX_RETRY 30

static osTimerId ntt_bt_connected_prompt_timer_id = NULL;
static bool ntt_bt_connected_prompt_pending = false;
static uint8_t ntt_bt_connected_prompt_device_id = 0;
static uint8_t ntt_bt_connected_prompt_retry_count = 0;

static void ntt_bt_connected_prompt_timehandler(void const *param)
{
    uint8_t device_id = ntt_bt_connected_prompt_device_id;
    bool tws_connected = bts_tws_if_is_tws_link_connected();
    uint8_t ui_role = app_ibrt_if_get_ui_role();

    if (!ntt_bt_connected_prompt_pending)
    {
        return;
    }

    BESUI_TRACE(4,
                "[NTT_CONN_PROMPT] check dev=%u tws=%u role=%u retry=%u",
                device_id,
                tws_connected,
                ui_role,
                ntt_bt_connected_prompt_retry_count);

    if (tws_connected)
    {
        /* Only Master starts the synchronized prompt transaction. */
        if (ui_role == TWS_UI_MASTER)
        {
            ntt_bt_connected_prompt_pending = false;
            ntt_bt_connected_prompt_retry_count = 0;

            BESUI_TRACE(1,
                        "[NTT_CONN_PROMPT] master play dev=%u",
                        device_id);

            media_PlayAudio(AUD_ID_BT_CONNECTED, device_id);
            return;
        }

        /*
         * During fast power-on reconnect the role can still be UNKNOWN or
         * temporarily SLAVE.  Keep the request instead of dropping it.
         */
        if (ntt_bt_connected_prompt_retry_count <
            NTT_BT_CONNECTED_PROMPT_MAX_RETRY)
        {
            ntt_bt_connected_prompt_retry_count++;
            osTimerStart(ntt_bt_connected_prompt_timer_id,
                         NTT_BT_CONNECTED_PROMPT_RETRY_MS);

            BESUI_TRACE(2,
                        "[NTT_CONN_PROMPT] wait master role=%u retry=%u",
                        ui_role,
                        ntt_bt_connected_prompt_retry_count);
            return;
        }

        /* This ear stayed Slave; the peer Master owns prompt playback. */
        ntt_bt_connected_prompt_pending = false;
        ntt_bt_connected_prompt_retry_count = 0;
        BESUI_TRACE(0,
                    "[NTT_CONN_PROMPT] slave timeout, wait peer prompt");
    }
    else
    {
        /* Give a slow TWS reconnect/role setup an additional three seconds. */
        if (ntt_bt_connected_prompt_retry_count <
            NTT_BT_CONNECTED_PROMPT_MAX_RETRY)
        {
            ntt_bt_connected_prompt_retry_count++;
            osTimerStart(ntt_bt_connected_prompt_timer_id,
                         NTT_BT_CONNECTED_PROMPT_RETRY_MS);

            BESUI_TRACE(1,
                        "[NTT_CONN_PROMPT] wait tws retry=%u",
                        ntt_bt_connected_prompt_retry_count);
            return;
        }

        /* No peer after timeout: treat this as a true single-bud session. */
        ntt_bt_connected_prompt_pending = false;
        ntt_bt_connected_prompt_retry_count = 0;
        BESUI_TRACE(1,
                    "[NTT_CONN_PROMPT] no tws, local play dev=%u",
                    device_id);
        media_PlayAudio_locally(AUD_ID_BT_CONNECTED, device_id);
    }
}

osTimerDef(NTT_BT_CONNECTED_PROMPT_TIMER,
           (void (*)(void const *))ntt_bt_connected_prompt_timehandler);

void ntt_bt_connected_prompt_request(uint8_t device_id)
{
    ntt_bt_connected_prompt_device_id = device_id;

    if (ntt_bt_connected_prompt_timer_id == NULL)
    {
        ntt_bt_connected_prompt_timer_id =
            osTimerCreate(osTimer(NTT_BT_CONNECTED_PROMPT_TIMER),
                          osTimerOnce,
                          NULL);
        ASSERT(ntt_bt_connected_prompt_timer_id,
               "[NTT_CONN_PROMPT] timer create failed");
    }

    /* HFP/A2DP and UI callbacks can report the same connection. */
    if (ntt_bt_connected_prompt_pending)
    {
        BESUI_TRACE(1,
                    "[NTT_CONN_PROMPT] duplicate ignored dev=%u",
                    device_id);
        return;
    }

    ntt_bt_connected_prompt_pending = true;
    ntt_bt_connected_prompt_retry_count = 0;
    osTimerStop(ntt_bt_connected_prompt_timer_id);
    osTimerStart(ntt_bt_connected_prompt_timer_id,
                 NTT_BT_CONNECTED_PROMPT_DELAY_MS);

    BESUI_TRACE(2,
                "[NTT_CONN_PROMPT] defer dev=%u delay=%u ms",
                device_id,
                NTT_BT_CONNECTED_PROMPT_DELAY_MS);
}

void ntt_bt_connected_prompt_cancel(void)
{
    ntt_bt_connected_prompt_pending = false;
    ntt_bt_connected_prompt_retry_count = 0;

    if (ntt_bt_connected_prompt_timer_id != NULL)
    {
        osTimerStop(ntt_bt_connected_prompt_timer_id);
    }

    BESUI_TRACE(0, "[NTT_CONN_PROMPT] cancel");
}

void app_bt_mobile_connect_info_init(void)
{
    mobile_connect_info[0].mobile_connected = false;
    memset(mobile_connect_info[0].connect_mobile_addr.address, 0x00, 6);

    mobile_connect_info[1].mobile_connected = false;
    memset(mobile_connect_info[1].connect_mobile_addr.address, 0x00, 6);
}

bool app_bt_get_mobile_connected_status(uint8_t device_id)
{
    return mobile_connect_info[device_id].mobile_connected;
}

void app_bt_set_mobile_connected_status(uint8_t device_id, bool connected_flag)
{
    mobile_connect_info[device_id].mobile_connected = connected_flag;
}

void app_bt_set_mobile_connected_info(uint8_t device_id, uint8_t *bt_addr)
{
    memcpy(mobile_connect_info[device_id].connect_mobile_addr.address, bt_addr, 6);
}

void app_bt_clear_mobile_connect_info(uint8_t device_id)
{
    mobile_connect_info[device_id].mobile_connected = false;
    memset(mobile_connect_info[device_id].connect_mobile_addr.address, 0x00, 6);
}


void app_bt_mobile_reconnect_info_init(void)
{
    memset(mobile_reconnect_info[0].current_id_addr.address, 0x00, 6);
    mobile_reconnect_info[0].reconnected_type = CONNECT_DEFAULT;
    mobile_reconnect_info[0].reconnected_count = 0;

    memset(mobile_reconnect_info[1].current_id_addr.address, 0x00, 6);
    mobile_reconnect_info[1].reconnected_type = CONNECT_DEFAULT;
    mobile_reconnect_info[1].reconnected_count = 0;
}

void app_bt_mobile_set_reconnect_info(uint8_t device_id, uint8_t *mobile_addr)
{
    if(device_id < 2)
    {
        mobile_reconnect_info[device_id].reconnected_type = CONNECT_RECONNECT;
        mobile_reconnect_info[device_id].reconnected_count = 0;
        memcpy(mobile_reconnect_info[device_id].current_id_addr.address, mobile_addr, 6);
    }
    else
    {
        BESUI_TRACE(0,"[UIBT]only last two");
        return;
    }

    BESUI_TRACE(1,"[UIBT]%s, device_id = %d", __func__, device_id);
    DUMP8("%02x ", mobile_reconnect_info[device_id].current_id_addr.address, 6);
}

void app_bt_mobile_clear_reconnect_info(uint8_t device_id)
{
    if(device_id < 2)
    {
        mobile_reconnect_info[device_id].reconnected_type = CONNECT_DEFAULT;
        mobile_reconnect_info[device_id].reconnected_count = 0;
        memset(mobile_reconnect_info[device_id].current_id_addr.address, 0x00, 6);
    }
}

bool app_bt_mobile_get_reconnect_status(void)
{
    if((mobile_reconnect_info[0].reconnected_type == CONNECT_RECONNECT)||(mobile_reconnect_info[1].reconnected_type == CONNECT_RECONNECT))
        return true;
    return false;
}


void app_bt_mobile_open_reconnect_info_init(void)
{
    memset(mobile_open_reconnect_info[0].current_id_addr.address, 0x00, 6);
    mobile_open_reconnect_info[0].open_reconnected_type = CONNECT_DEFAULT;
    mobile_open_reconnect_info[0].open_reconnected_count = 0;

    memset(mobile_open_reconnect_info[1].current_id_addr.address, 0x00, 6);
    mobile_open_reconnect_info[1].open_reconnected_type = CONNECT_DEFAULT;
    mobile_open_reconnect_info[1].open_reconnected_count = 0;
}


void app_bt_set_mobile_reconnect_addr(uint8_t device_id, uint8_t *mobile_addr)
{
    if(device_id == 0)
    {
        memcpy(mobile_open_reconnect_info[device_id].current_id_addr.address, mobile_addr, 6);
    }
    else if(device_id == 1)
    {
        memcpy(mobile_open_reconnect_info[device_id].current_id_addr.address, mobile_addr, 6);
    }
    else
    {
        BESUI_TRACE(0,"[UIBT]only last two");
        return;
    }

    BESUI_TRACE(1,"[UIBT]%s, device_id = %d", __func__, device_id);
    DUMP8("%02x ", mobile_open_reconnect_info[device_id].current_id_addr.address, 6);
}

//clear reconnect info, slave use
void app_bt_mobile_clear_openreconnect_info(uint8_t device_id, bool clear_all_flag)
{
    BESUI_TRACE(3,"[UIBT]%s, device_id=%d clear_all_flag=%d", __func__, device_id, clear_all_flag);
    if(clear_all_flag)
    {
        mobile_open_reconnect_info[0].open_reconnected_type = CONNECT_DEFAULT;
        mobile_open_reconnect_info[0].open_reconnected_count = 0;
        mobile_open_reconnect_info[1].open_reconnected_type = CONNECT_DEFAULT;
        mobile_open_reconnect_info[1].open_reconnected_count = 0;
        memset(mobile_open_reconnect_info[0].current_id_addr.address, 0x00, 6);
        memset(mobile_open_reconnect_info[1].current_id_addr.address, 0x00, 6);
    }
    else
    {
        mobile_open_reconnect_info[device_id].open_reconnected_type = CONNECT_DEFAULT;
        mobile_open_reconnect_info[device_id].open_reconnected_count = 0;
        memset(mobile_open_reconnect_info[device_id].current_id_addr.address, 0x00, 6);
    }
}

//open box set headset reconnecte status or phone connected clear status
void app_bt_mobile_set_openreconnect_info(uint8_t device_id, uint8_t recon_type, bool set_all_flag)
{
    BESUI_TRACE(3,"[UIBT]%s id %d type %d flag %d", __func__, device_id, recon_type, set_all_flag);
    if(set_all_flag)
    {
        mobile_open_reconnect_info[0].open_reconnected_type = recon_type;
        mobile_open_reconnect_info[0].open_reconnected_count = 0;
        mobile_open_reconnect_info[1].open_reconnected_type = recon_type;
        mobile_open_reconnect_info[1].open_reconnected_count = 0;
    }
    else
    {
        mobile_open_reconnect_info[device_id].open_reconnected_type = recon_type;
        mobile_open_reconnect_info[device_id].open_reconnected_count = 0;
    }
}

// bool app_bt_system_get_openreconnected_status(void)
// {
//     BESUI_TRACE(2,"[UIBT]%s recon %d %d", __func__, mobile_open_reconnect_info[0].open_reconnected_type, 
//                                               mobile_open_reconnect_info[1].open_reconnected_type);

//     if((mobile_open_reconnect_info[0].open_reconnected_type == CONNECT_OPEN_RECON)
//      ||(mobile_open_reconnect_info[1].open_reconnected_type == CONNECT_OPEN_RECON))
//         return true;
//     return false;
// }

uint8_t app_bt_get_reconnected_type(void)
{
    BESUI_TRACE(2,"[UIBT]%s recon %d %d", __func__, mobile_reconnect_info[0].reconnected_type, mobile_reconnect_info[1].reconnected_type);

    if((mobile_reconnect_info[0].reconnected_type == CONNECT_RECONNECT)&&(mobile_reconnect_info[1].reconnected_type == CONNECT_RECONNECT))
        return 3;
    else if((mobile_reconnect_info[0].reconnected_type == CONNECT_RECONNECT)&&(mobile_reconnect_info[1].reconnected_type == CONNECT_DEFAULT))
        return 1;
    else if((mobile_reconnect_info[0].reconnected_type == CONNECT_DEFAULT)&&(mobile_reconnect_info[1].reconnected_type == CONNECT_RECONNECT))
        return 2;
    else if((mobile_reconnect_info[0].reconnected_type == CONNECT_DEFAULT)&&(mobile_reconnect_info[1].reconnected_type == CONNECT_DEFAULT))
        return 0;

    return 0;
}


uint8_t app_bt_compare_openreconnect_addr(uint8_t *addr)
{
    uint8_t i = 0;

    for(i = 0; i < 2; i++)
    {
        if(memcmp(addr, mobile_open_reconnect_info[i].current_id_addr.address, 6) == 0)
        {
            BESUI_TRACE(1,"[UIBT]%s pipei id %d", __func__, i);
            return i;
        }
    }

    return i;
}


uint8_t app_bt_compare_reconnect_addr(uint8_t *addr)
{
    uint8_t i = 0;

    for(i = 0; i < 2; i++)
    {
        if(memcmp(addr, mobile_reconnect_info[i].current_id_addr.address, 6) == 0)
        {
            BESUI_TRACE(1,"[UIBT]%s pipei id %d", __func__, i);
            return i;
        }
    }

    return i;
}

//
uint8_t app_bt_compare_connect_addr(uint8_t *addr)
{
    uint8_t i = 0;

    for(i = 0; i < 2; i++)
    {
        if(memcmp(addr, mobile_connect_info[i].connect_mobile_addr.address, 6) == 0)
        {
            BESUI_TRACE(1,"[UIBT]%s pipei id %d", __func__, i);
            return i;
        }
    }
    return i;
}

static int besui_bt_msg_handler(APP_MESSAGE_BODY *msg_body);
void besui_bt_msg_modual_init(void)
{
    app_set_threadhandle(APP_MODUAL_SYSTEM_MSG, besui_bt_msg_handler);
}

void besui_bt_msg_put(uint8_t id, uint8_t type, uint8_t device_id)
{
    BESUI_TRACE(0, "[UIBT][%s], 0x%02X, 0x%02X, 0x%02X", __func__, id, type, device_id);

    APP_MESSAGE_BLOCK msg;

    msg.mod_id = APP_MODUAL_SYSTEM_MSG;

    msg.msg_body.message_id = id;
    msg.msg_body.message_Param0 = type;
    msg.msg_body.message_Param2 = device_id;

    app_mailbox_put(&msg);

    uictl.role_switch_no = true;
}


void app_enter_phone_pairmode_delay_process(void)
{
    ibrt_ctrl_t *p_ibrt_ctrl = NULL;
    p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    uint8_t conn_devices = besui_get_profile_conn_num();//app_bt_count_connected_device();

    BESUI_TRACE(3,"[UIBT]%s access mode %d nv_role %d, ui_role %d, conn_devices %d", __func__, 
    app_bt_get_curr_access_mode(), p_ibrt_ctrl->nv_role, app_ibrt_if_get_ui_role(), conn_devices);

    if(conn_devices)
        return;

    if(bts_tws_if_is_tws_link_connected())
    {
        //if((app_bt_get_curr_access_mode() == BTIF_BAM_GENERAL_ACCESSIBLE)&&(TWS_UI_MASTER == app_ibrt_if_get_ui_role()))
        if(TWS_UI_MASTER == app_ibrt_if_get_ui_role())
        {
            app_bt_mobile_clear_reconnect_info(0);
            app_bt_mobile_clear_reconnect_info(1);
            app_bt_mobile_set_openreconnect_info(0, CONNECT_DEFAULT, true);
            app_status_indication_set(APP_STATUS_INDICATION_BOTHSCAN);
            media_PlayAudio(AUD_ID_BT_PAIR_ENABLE, 0);
            app_start_10_second_timer(APP_PAIR_TIMER_ID);
            app_stop_10_second_timer(APP_POWEROFF_TIMER_ID);
            app_pairmode_timer_onoff(true);
            app_poweroff_timer_onoff(false);
#ifdef GFPS_ENABLED
            if(conn_devices == 0)
            {
                if(!gfps_get_wait_connect_phone())
                {
                    BESUI_TRACE(0,"[UIBT]%s, gfps_enter_pairmode 1", __func__);
                    app_enter_fastpairing_mode();                    
                }
            }
#endif
        }
        else if(TWS_UI_SLAVE == app_ibrt_if_get_ui_role())
        {
            app_bt_mobile_clear_reconnect_info(0);
            app_bt_mobile_clear_reconnect_info(1);
            app_bt_mobile_set_openreconnect_info(0, CONNECT_DEFAULT, true);
#ifdef USER_LED_BREATH_EN
            app_status_indication_set(APP_STATUS_INDICATION_BOTHSCAN);
#else
            app_status_indication_set(APP_STATUS_INDICATION_TWS_CONNECTED);
#endif
            app_stop_10_second_timer(APP_PAIR_TIMER_ID);
            app_stop_10_second_timer(APP_POWEROFF_TIMER_ID);
            app_pairmode_timer_onoff(false);
            app_poweroff_timer_onoff(false);
        }
    }
    else
    {
        //if(app_bt_get_curr_access_mode() == BTIF_BAM_GENERAL_ACCESSIBLE)
        if(conn_devices == 0 && app_bt_get_curr_access_mode() == BTIF_BAM_GENERAL_ACCESSIBLE)
        {
            app_bt_mobile_clear_reconnect_info(0);
            app_bt_mobile_clear_reconnect_info(1);
            app_bt_mobile_set_openreconnect_info(0, CONNECT_DEFAULT, true);
            #ifdef BESUI_1WIRE_EN
            app_uart_clear_need_twspair_process();
            #endif
            app_status_indication_set(APP_STATUS_INDICATION_BOTHSCAN);
            media_PlayAudio(AUD_ID_BT_PAIR_ENABLE, 0);
            app_start_10_second_timer(APP_PAIR_TIMER_ID);
            app_stop_10_second_timer(APP_POWEROFF_TIMER_ID);
            app_pairmode_timer_onoff(true);
            app_poweroff_timer_onoff(false);
#ifdef GFPS_ENABLED
            if(conn_devices == 0)
            {
                if(!gfps_get_wait_connect_phone())
                {
                    BESUI_TRACE(0,"[UIBT]%s, gfps_enter_pairmode 2", __func__);
                    app_enter_fastpairing_mode();                    
                }
            }
#endif
        }
    }
}

//use timer,because access mode status to pairmode need 100ms,so delay deal
osTimerId app_enter_phone_pairmode_delay_process_timer = NULL;

void app_enter_phone_pairmode_delay_process_timehandler(void const *param)
{
    if(uictl.auth_start_flag)
    {
        app_phone_pairmode_timer_onoff(true);
        return;
    }
	BESUI_TRACE(1, "[UIBT][UITIMER]%s ", __func__);
    app_enter_phone_pairmode_delay_process();
}

osTimerDef (APP_ENTER_PHONE_PAIRMODE_DELAY_PROCESS_TIMER, (void (*)(void const *))app_enter_phone_pairmode_delay_process_timehandler);

void app_phone_pairmode_timer_onoff(bool timer_en)
{
	BESUI_TRACE(1, "[UIBT][UITIMER]%s timer_en %d ", __func__, timer_en);

	if(app_enter_phone_pairmode_delay_process_timer == NULL)
	{
    	app_enter_phone_pairmode_delay_process_timer = osTimerCreate(osTimer(APP_ENTER_PHONE_PAIRMODE_DELAY_PROCESS_TIMER),osTimerOnce,NULL);
        if(!app_enter_phone_pairmode_delay_process_timer)
        {
             ASSERT(0, "[UIBT][UITIMER][%s] osTimerCreate error", __func__);
        }
	}

	if(timer_en)
	{
		osTimerStart(app_enter_phone_pairmode_delay_process_timer, 600);
	}
	else
	{
		osTimerStop(app_enter_phone_pairmode_delay_process_timer);
	}
}

void app_enter_pairmode_event_process(void)
{
    if(!app_bt_system_phone_cancel_enter_pairmode_flag)
    {
        app_phone_pairmode_timer_onoff(true);
    }
    else
    {
        app_bt_system_phone_cancel_enter_pairmode_flag = false;
    }
}

void app_exit_pairmode_event_process(void)
{
    uint8_t conn_devices = besui_get_profile_conn_num();//app_bt_count_connected_device();

    if(conn_devices == 0)
    {
        app_poweroff_shutdown_ui_post_msg(false, AUTO_POWEROFF);
    }
}


//timer out. if timer out , use,  poweroff time id
void app_timeout_poweroff_event_process(void)
{
    uint8_t conn_devices = besui_get_profile_conn_num();//app_bt_count_connected_device();

    if(conn_devices == 0)
    {
        app_poweroff_shutdown_ui_post_msg(false, AUTO_POWEROFF);
    }
}


//bescause tws connected,accessmode change slow,so need delay handle
osTimerId app_tws_connected_event_timer_id = NULL;
void app_tws_connected_event_timehandler(void const *param)
{
    ibrt_ctrl_t *p_ibrt_ctrl = NULL;
    p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    uint8_t conn_devices = besui_get_profile_conn_num();

    BESUI_TRACE(2, "[UIBT][UITIMER]%s tws connect %d access_mode %d", __func__, bts_tws_if_is_tws_link_connected(), app_bt_get_curr_access_mode());

    BESUI_TRACE(3, "[UIBT][UITIMER]%s conn_devices %d accessmode %d nv_role %d ui_role %d", __func__, conn_devices, 
            app_bt_get_curr_access_mode(), p_ibrt_ctrl->nv_role, app_ibrt_if_get_ui_role());

    app_ibrt_customif_cmd_sync_battery_level(app_battery_current_level());

    uint8_t *bt_local_addr;
    bt_local_addr = bt_get_local_address();
    app_ibrt_customif_cmd_sync_btaddr(bt_local_addr);

#ifdef BESUI_1WIRE_EN
    app_uart_clear_need_twspair_process(); //TWSPAIR successfull
#endif

#ifdef USER_APP_BLE_DIS_EN
    uint8_t ptrParam[3] = {0};
    ptrParam[0] = USER_TWS_CMD_RANDOM;
    memcpy((uint8_t *)(ptrParam+1), (uint8_t *)(uictl.random_local), 2);
    // tws_ctrl_send_cmd(APP_TWS_CMD_SYNC_SOMETHING, ptrParam, 3);
    DUMP8("%02X ", ptrParam, 3);
#endif

    if(conn_devices > 0)
    {
        if(TWS_UI_MASTER == app_ibrt_if_get_ui_role())
        {
#if defined(ANC_APP)
            besui_anc_openbox_tws_sync();
#endif
            app_tws_ledsta_to_slave_process(conn_devices, APP_STATUS_INDICATION_NUM);
#ifdef ALGO_INFO_SYNC_EN
            algo_sync_to_slave();
#endif
        }
        else
        {
            //app_status_indication_set(APP_STATUS_INDICATION_CONNECTED);
            if(app_bt_mobile_get_reconnect_status())
            {
                app_status_indication_set(APP_STATUS_INDICATION_LINKLOSS_DISCONNECTED);
            }
            else
            {
                app_status_indication_set(APP_STATUS_INDICATION_TWS_CONNECTED);
            }
            app_stop_10_second_timer(APP_PAIR_TIMER_ID);
            app_stop_10_second_timer(APP_POWEROFF_TIMER_ID);
            app_pairmode_timer_onoff(false);
            app_poweroff_timer_onoff(false);
            app_bt_mobile_clear_openreconnect_info(0, true);
        }
#ifdef CAPSENSOR_ENABLE
        twsui_wear_pp_tx(false, uicom.wear_curr_sta);
#endif
    }
    else
    {
        if(TWS_UI_MASTER == app_ibrt_if_get_ui_role())
        {
#if defined(ANC_APP)
            besui_anc_openbox_tws_sync();
#endif

            if(app_bt_get_curr_access_mode() == BTIF_BAM_GENERAL_ACCESSIBLE)
            {
                app_phone_pairmode_timer_onoff(true);
            }
#ifdef ALGO_INFO_SYNC_EN
            algo_sync_to_slave();
#endif
        }
        else if(TWS_UI_SLAVE == app_ibrt_if_get_ui_role())
        {
            BESUI_TRACE(0,"[UIBT]TWS CONNECT SLAVE");
            //app_status_indication_set(APP_STATUS_INDICATION_TWS_CONNECTED);
            if(app_bt_mobile_get_reconnect_status())
            {
                app_status_indication_set(APP_STATUS_INDICATION_LINKLOSS_DISCONNECTED);
            }
            else
            {
                app_status_indication_set(APP_STATUS_INDICATION_TWS_CONNECTED);
            }
            app_stop_10_second_timer(APP_PAIR_TIMER_ID);
            app_stop_10_second_timer(APP_POWEROFF_TIMER_ID);
            app_pairmode_timer_onoff(false);
            app_poweroff_timer_onoff(false);
            app_bt_mobile_clear_openreconnect_info(0, true);
            //app_phone_linkloss_pairmode_timer_onoff(false);
        }
#ifdef CAPSENSOR_ENABLE
        twsui_wear_pp_tx(false, uicom.wear_curr_sta);
#endif
    }

    osTimerDelete(app_tws_connected_event_timer_id);
    app_tws_connected_event_timer_id = NULL;
}

osTimerDef (APP_TWS_CONNECTED_EVENT_TIMER_NAME, (void (*)(void const *))app_tws_connected_event_timehandler);

void app_tws_connected_event_timer_onoff(bool timer_en)
{
	BESUI_TRACE(1, "[UIBT][UITIMER]%s timer_en %d ", __func__, timer_en);

	if(app_tws_connected_event_timer_id == NULL)
	{
    	app_tws_connected_event_timer_id = osTimerCreate(osTimer(APP_TWS_CONNECTED_EVENT_TIMER_NAME),osTimerOnce,NULL);
        if(!app_tws_connected_event_timer_id)
             ASSERT(0, "[UIBT][UITIMER][%s] osTimerCreate error", __func__);
	}

	if(timer_en)
		osTimerStart(app_tws_connected_event_timer_id, 200);
	else
		osTimerStop(app_tws_connected_event_timer_id);
}

void app_tws_connected_event_process(void)
{
    app_tws_connected_event_timer_onoff(true);
}

//because gfps, dut disconnected, delay 5S enter pairmode
osTimerId app_tws_linkloss_to_pairmode_timer_id = NULL;
void app_tws_linkloss_to_pairmode_process_timehandler(void const *param)
{
    uint8_t conn_devices = besui_get_profile_conn_num();
    // ibrt_ctrl_t *p_ibrt_ctrl = NULL;
    // p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();

	BESUI_TRACE(1, "[UIBT][UITIMER]%s conn_devices %d", __func__, conn_devices);
	if(conn_devices == 0)
    {
        if(app_bt_get_curr_access_mode() == BTIF_BAM_GENERAL_ACCESSIBLE)
            app_enter_phone_pairmode_delay_process();
        if(app_bt_get_curr_access_mode() != BTIF_BAM_CONNECTABLE_ONLY)
            besui_enter_pairmode();

    }

    osTimerDelete(app_tws_linkloss_to_pairmode_timer_id);
    app_tws_linkloss_to_pairmode_timer_id = NULL;
}

osTimerDef (APP_TWS_LINKLOSS_TO_PAIRMODE_TIMER_NAME, (void (*)(void const *))app_tws_linkloss_to_pairmode_process_timehandler);

void app_tws_linkloss_pairmode_timer_onoff(bool timer_en)
{
	BESUI_TRACE(1, "[UIBT][UITIMER]%s timer_en %d ", __func__, timer_en);

	if(app_tws_linkloss_to_pairmode_timer_id == NULL)
	{
    	app_tws_linkloss_to_pairmode_timer_id = osTimerCreate(osTimer(APP_TWS_LINKLOSS_TO_PAIRMODE_TIMER_NAME),osTimerOnce,NULL);
        if(!app_tws_linkloss_to_pairmode_timer_id)
             ASSERT(0, "[UIBT][UITIMER][%s] osTimerCreate error", __func__);
    }

	if(timer_en)
		osTimerStart(app_tws_linkloss_to_pairmode_timer_id, 10000);
	else
		osTimerStop(app_tws_linkloss_to_pairmode_timer_id);
}


osTimerId app_tws_linkloss_timer_id = NULL;
void app_tws_linkloss_timehandler(void const *param)
{
    uint8_t conn_devices = besui_get_profile_conn_num();
	BESUI_TRACE(1, "[UIBT][UITIMER]%s conn_devices %d", __func__, conn_devices);
    if(conn_devices == 0)
    {
        app_status_indication_set(APP_STATUS_INDICATION_LINKLOSS_DISCONNECTED);
        app_tws_linkloss_pairmode_timer_onoff(true);
    }
}

osTimerDef (APP_TWS_LINKLOSS_TIMER_NAME, (void (*)(void const *))app_tws_linkloss_timehandler);

void app_tws_linkloss_timer_onoff(bool timer_en)
{
	BESUI_TRACE(1, "[UIBT][UITIMER]%s timer_en %d ", __func__, timer_en);
	if(app_tws_linkloss_timer_id == NULL)
	{
    	app_tws_linkloss_timer_id = osTimerCreate(osTimer(APP_TWS_LINKLOSS_TIMER_NAME),osTimerOnce,NULL);
        if(!app_tws_linkloss_timer_id)
        {
             ASSERT(0, "[UIBT][UITIMER][%s] osTimerCreate error", __func__);
        }
	}

	if(timer_en)
		osTimerStart(app_tws_linkloss_timer_id, 500);
	else
		osTimerStop(app_tws_linkloss_timer_id);
}


void app_tws_disconnected_event_process(uint8_t discon_type)
{
    ibrt_ctrl_t *p_ibrt_ctrl = NULL;
    p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();

    if(!app_current_box_status_open_status()||(app_poweroff_flag))
    {
        BESUI_TRACE(0,"[UIBT]%s charging or openrecon do not need ",__func__);
        return;
    }

    uint8_t conn_devices = besui_get_profile_conn_num();

    BESUI_TRACE(2,"[UIBT]%s, conn_devices=%d,discon_type=0x%X, p_ibrt_ctrl->nv_role=%d", __func__, conn_devices, discon_type, p_ibrt_ctrl->nv_role);

    app_tws_battery_update(false);

    app_common_clear_other_earstatus();

#ifdef DOLBY_AUDIO_ENABLE
    if(dolby_audio_onoff_get() && conn_devices && besui_ear_sta_get(false))
    {
#ifdef ALGO_DELAY_ONOFF_EN
        algo_dolby_delay_onoff(ALGO_OFF, uictl.dolby_mode);
#else
        algo_send_request(ALGO_ID_DOLBY, ALGO_OFF, 0, 0, 0, 0); //dolby off
#endif
    }
#endif

    if(discon_type == 0x08)
    {
        app_tws_linkloss_timer_onoff(true);
    }
    else
    {
        if(conn_devices == 0)
        {
            if(app_bt_get_curr_access_mode() == BTIF_BAM_GENERAL_ACCESSIBLE)
            {
                app_phone_pairmode_timer_onoff(true);
            }
        }
    }
    if(app_ui_is_enter_pairing_mode() && !besui_bat_charge_sta_get())
    {
        app_status_indication_set(APP_STATUS_INDICATION_BOTHSCAN);
    }
}

void app_bt_phone_connected_event_process(void)
{
    uint8_t conn_devices = besui_get_profile_conn_num();//app_bt_count_connected_device();

    BESUI_TRACE(2,"[UIBT]%s conn_devices %d ui_role %d", __func__, conn_devices, app_ibrt_if_get_ui_role());

    app_phone_discon_pairmode_timer_onoff(false);
    app_tws_linkloss_pairmode_timer_onoff(false);

    nv_record_all_ddbrec_print();
    BESUI_TRACE(0,"[UIBT]PHONE_CONNECTED");
	
#if BT_DEVICE_NUM > 1
    uicom.phone_pair_cnt ++;
#endif

//--------------------------------------------------------------------------------
//APP SYNC data
#ifdef BESUI_APP_EN
    app_tota_phone_num_to_app(0xFF);
#ifdef CODEC_TYPE_APP_EN
    codec_type_to_app();
#endif
#endif
#ifdef APP_SYNC_VOLUME_EN
    app_sync_earbuds_volume();
#endif

#ifdef GFPS_ENABLED
    app_tws_battery_update(true);
#endif
//--------------------------------------------------------------------------------

    app_stop_10_second_timer(APP_PAIR_TIMER_ID);
    app_stop_10_second_timer(APP_POWEROFF_TIMER_ID);
    app_pairmode_timer_onoff(false);
    app_poweroff_timer_onoff(false);

    if (!bts_tws_if_is_tws_link_connected())
    {
        if(conn_devices == 1)
        {
            uicom.led_bt_conn_flag = true;
            app_status_indication_set(APP_STATUS_INDICATION_PHONE_CONNECTED);
            app_tws_ledsta_to_slave_process(conn_devices, APP_STATUS_INDICATION_PHONE_CONNECTED);

            app_recover_led_timer_onoff(true, true);
            app_status_indication_set(APP_STATUS_INDICATION_CONNECTED);
            app_tws_ledsta_to_slave_process(conn_devices, APP_STATUS_INDICATION_CONNECTED);

            ntt_bt_connected_prompt_request(0);
#ifdef BESUI_BOX_PUTINOUT_EN
            app_putinout_box_event_process(uicom.box_put_sta, true);
#endif
        }
        else if(conn_devices == 2)
        {
            ntt_bt_connected_prompt_request(0);
#ifdef BESUI_BOX_PUTINOUT_EN
            app_putinout_box_event_process(uicom.box_put_sta, true);
#endif
        }
    }
    else
    {
        if(conn_devices == 1)
        {
            if(TWS_UI_MASTER == app_ibrt_if_get_ui_role())
            {
                uicom.led_bt_conn_flag = true;
                app_status_indication_set(APP_STATUS_INDICATION_PHONE_CONNECTED);
                app_tws_ledsta_to_slave_process(conn_devices, APP_STATUS_INDICATION_PHONE_CONNECTED);

                app_recover_led_timer_onoff(true, true);
                app_status_indication_set(APP_STATUS_INDICATION_CONNECTED);
                app_tws_ledsta_to_slave_process(conn_devices, APP_STATUS_INDICATION_CONNECTED);

                ntt_bt_connected_prompt_request(0);
#ifdef BESUI_BOX_PUTINOUT_EN
                app_putinout_box_event_process(uicom.box_put_sta, true);
#endif
            }
            else if(TWS_UI_SLAVE == app_ibrt_if_get_ui_role())
            {
                besui_delay_putinout_timer_onoff(true);
            }

        }
        else if(conn_devices == 2)
        {
            if(TWS_UI_MASTER == app_ibrt_if_get_ui_role())
            {
                ntt_bt_connected_prompt_request(0);
                #ifdef BESUI_BOX_PUTINOUT_EN
                app_putinout_box_event_process(uicom.box_put_sta, true);
                #endif
            }
            else if(TWS_UI_SLAVE == app_ibrt_if_get_ui_role())
            {
                besui_delay_putinout_timer_onoff(true);
            }
        }
    }
}

void besui_enter_pairmode(void)
{
    BESUI_TRACE(1, "[UIBT]%s tws connect %d", __func__, bts_tws_if_is_tws_link_connected());
    if(bts_tws_if_is_tws_link_connected())
    {
        if(TWS_UI_MASTER == app_ibrt_if_get_ui_role())
        {
            app_ibrt_if_enter_pairing_after_tws_connected();
        }
    }
    else
    {
        app_ui_enter_pairing_mode(IBRT_UI_DISABLE_BT_SCAN_TIMEOUT, false);
    }
}

osTimerId app_phone_discon_to_pairmode_timer_id = NULL;
void app_phone_discon_to_pairmode_timehandler(void const *param)
{
	BESUI_TRACE(1, "[UIBT][UITIMER]%s", __func__);
	
    if(uictl.auth_start_flag)
    {
        app_phone_discon_pairmode_timer_onoff(true);
        return;
    }
    besui_enter_pairmode();
}

osTimerDef (APP_PHONE_DISCON_TO_PAIRMODE_TIMER_NAME, (void (*)(void const *))app_phone_discon_to_pairmode_timehandler);

void app_phone_discon_pairmode_timer_onoff(bool timer_en)
{
    return;
/* 
	BESUI_TRACE(1, "[UIBT][UITIMER]%s timer_en %d ", __func__, timer_en);

    if(!app_current_box_status_open_status()||(app_bt_system_get_openreconnected_status()))
    {
        if(timer_en == true)
        {
            BESUI_TRACE(0,"[UIBT][UITIMER]charge disconnected do not need enter pairmode");
            return;
        }
    }

	if(app_phone_discon_to_pairmode_timer_id == NULL)
	{
    	app_phone_discon_to_pairmode_timer_id = osTimerCreate(osTimer(APP_PHONE_DISCON_TO_PAIRMODE_TIMER_NAME),osTimerOnce,NULL);
        if(!app_phone_discon_to_pairmode_timer_id)
        {
             ASSERT(0, "[UIBT][UITIMER][%s] osTimerCreate error", __func__);
        }
	}

	if(timer_en)
		osTimerStart(app_phone_discon_to_pairmode_timer_id, 5000);
	else
		osTimerStop(app_phone_discon_to_pairmode_timer_id);
     */    
}


osTimerId bt_reconnect_fail_to_pairmode_timerid = NULL;
void app_open_reconnect_fail_to_pairmode_timehandler(void const *param)
{
	BESUI_TRACE(0, "[UIBT][UITIMER]%s", __func__);
    besui_enter_pairmode();
}

osTimerDef (BT_RECONNECT_FAIL_TO_PAIRMODE_TIMER_NAME, (void (*)(void const *))app_open_reconnect_fail_to_pairmode_timehandler);

void app_open_reconnect_fail_pairmode_timer_onoff(bool timer_en)
{
    return;
	BESUI_TRACE(1, "[UIBT][UITIMER]%s timer_en %d ", __func__, timer_en);

    if(!app_current_box_status_open_status())
    {
        if(timer_en == true)
        {
            BESUI_TRACE(0,"[UIBT][UITIMER]charge disconnected do not need enter pairmode");
            return;
        }
    }

	if(bt_reconnect_fail_to_pairmode_timerid == NULL)
	{
    	bt_reconnect_fail_to_pairmode_timerid = osTimerCreate(osTimer(BT_RECONNECT_FAIL_TO_PAIRMODE_TIMER_NAME),osTimerOnce,NULL);
        if(!bt_reconnect_fail_to_pairmode_timerid)
        {
             ASSERT(0, "[UIBT][UITIMER][%s] osTimerCreate error", __func__);
        }
    }

	if(timer_en)
		osTimerStart(bt_reconnect_fail_to_pairmode_timerid, 200);
	else
		osTimerStop(bt_reconnect_fail_to_pairmode_timerid);
}

uint8_t phone_disc_type = 0;
void besui_set_phone_disc_type(uint8_t param)
{
    BESUI_TRACE(0,"[UIBT]%s, phone_disc_type = %02X",__func__, phone_disc_type);
    phone_disc_type = param;
}
uint8_t besui_get_phone_disc_type(void)
{
    BESUI_TRACE(0,"[UIBT]%s, phone_disc_type = %02X",__func__, phone_disc_type);
    return phone_disc_type;
}

void app_bt_phone_disconnected_event_process(uint8_t discon_type)
{
    uint8_t conn_devices = besui_get_profile_conn_num();
    POSSIBLY_UNUSED uint8_t slave_need_enter_pairmode = 0;

    /* Ignore transient single-profile disconnects during HFP/A2DP restore. */
    if (conn_devices == 0)
    {
        ntt_bt_connected_prompt_cancel();
    }
    else
    {
        BESUI_TRACE(1,
                    "[NTT_CONN_PROMPT] disconnect event ignored conn=%u",
                    conn_devices);
    }

    BESUI_TRACE(1,"[UIBT]%s discon_type 0x%02X conn %d ui role %d mode %d", __func__, discon_type, conn_devices, app_ibrt_if_get_ui_role(), app_bt_get_curr_access_mode());
    app_bt_system_phone_cancel_enter_pairmode_flag = false;
    app_open_reconnect_fail_pairmode_timer_onoff(false);
    
#ifdef BESUI_APP_EN
    app_tota_phone_num_to_app(0xFF);
#endif

    if(!app_current_box_status_open_status()||(app_poweroff_flag))
    {
        BESUI_TRACE(0,"[UIBT]%s charging or openrecon do not need ",__func__);
        return;
    }

    besui_set_phone_disc_type(discon_type);

    if(discon_type == 0x08)
    {
        app_tws_linkloss_timer_onoff(false);
        if(conn_devices == 0)
        {
            uicom.led_bt_conn_flag = false;
            app_recover_led_timer_onoff(false, false);
            if(TWS_UI_SLAVE != app_ibrt_if_get_ui_role())
            {
                BESUI_TRACE(0,"[UIBT]phone linkloss");
                media_PlayAudio(AUD_ID_BT_DIS_CONNECT, 0);
                app_status_indication_set(APP_STATUS_INDICATION_LINKLOSS_DISCONNECTED);
                app_start_10_second_timer(APP_POWEROFF_TIMER_ID);
                app_poweroff_timer_onoff(true);
            }
            else //if(TWS_UI_SLAVE == app_ibrt_if_get_ui_role())
            {
                app_status_indication_set(APP_STATUS_INDICATION_LINKLOSS_DISCONNECTED);
                slave_need_enter_pairmode = 2;
            }
        }
        else if(conn_devices == 1)
        {
            if(TWS_UI_SLAVE != app_ibrt_if_get_ui_role())
            {
                media_PlayAudio(AUD_ID_BT_DIS_CONNECT, 0);
            }
        }
    }
    else
    {  
        if(conn_devices == 0)
        {
            uicom.led_bt_conn_flag = false;
            app_recover_led_timer_onoff(false, false);
            if(TWS_UI_SLAVE != app_ibrt_if_get_ui_role())
            {
                if(uicom.phone_pair_cnt == 0)
                {
                    besui_enter_pairmode();
                }
                else
                {
                    app_status_indication_set(APP_STATUS_INDICATION_PAGESCAN);
                    media_PlayAudio(AUD_ID_BT_DIS_CONNECT, 0);
                    app_start_10_second_timer(APP_POWEROFF_TIMER_ID);
                    app_phone_discon_pairmode_timer_onoff(true);
                    app_poweroff_timer_onoff(true);
                }
            }
            else //if(TWS_UI_SLAVE == app_ibrt_if_get_ui_role())
            {
                app_status_indication_set(APP_STATUS_INDICATION_TWS_CONNECTED);
                slave_need_enter_pairmode = 1;
            }
        }
        else if(conn_devices == 1)
        {
            if(TWS_UI_SLAVE != app_ibrt_if_get_ui_role())
            {
                media_PlayAudio(AUD_ID_BT_DIS_CONNECT, 0);
            }
        }
    }

#if 0//def BESUI_COMM_EN
    uint8_t ptrParam[2] = {0};
    if(slave_need_enter_pairmode == 1) //enter_pairmode
    {
        ptrParam[0] = USER_TWS_CMD_ENTER_PAIRMODE;
        tws_ctrl_send_cmd(APP_TWS_CMD_SYNC_SOMETHING, ptrParam, 1);
    }
    else if(slave_need_enter_pairmode == 2) //linkloss
    {
        ptrParam[0] = USER_TWS_CMD_SLAVE_LINKLOSS;
        //tws_ctrl_send_cmd(APP_TWS_CMD_SYNC_SOMETHING, ptrParam, 1);
    }
#endif
}


void app_open_reconnect_phone_fail_process(uint8_t curr_device_id)
{
    bool enter_pair_flag = false;
    uint8_t conn_devices = besui_get_profile_conn_num();
    uint8_t other_device_id = (curr_device_id+1)%2;

    app_open_reconnect_fail_pairmode_timer_onoff(false);    

    mobile_open_reconnect_info[curr_device_id].open_reconnected_count++;

#if BT_DEVICE_NUM > 1
    if(uicom.phone_pair_cnt > 0)
    {
        if(conn_devices == 0)
        {
            if(mobile_open_reconnect_info[curr_device_id].open_reconnected_type == CONNECT_OPEN_RECON)
            {
                if(mobile_open_reconnect_info[curr_device_id].open_reconnected_count >= (IBRT_UI_OPEN_RECONNECT_MOBILE_MAX_TIMES+1))
                {
                    mobile_open_reconnect_info[curr_device_id].open_reconnected_type = CONNECT_DEFAULT;
                    mobile_open_reconnect_info[curr_device_id].open_reconnected_count = 0;
                    app_ibrt_customif_cmd_sync_set_reconnect_status(curr_device_id, CONNECT_DEFAULT);
                    enter_pair_flag = true;
                }
            }

            if(enter_pair_flag)
            {
                if((mobile_open_reconnect_info[other_device_id].open_reconnected_type == CONNECT_OPEN_RECON)
                ||(app_bt_mobile_get_reconnect_status()))
                {
                    enter_pair_flag = false;
                }
            }
        }
        else if(conn_devices == 1)
        {
            if(mobile_open_reconnect_info[curr_device_id].open_reconnected_type == CONNECT_OPEN_RECON)
            {
                if(mobile_open_reconnect_info[curr_device_id].open_reconnected_count >= (IBRT_UI_OPEN_RECONNECT_MOBILE_MAX_TIMES+1))
                {
                    mobile_open_reconnect_info[curr_device_id].open_reconnected_type = CONNECT_DEFAULT;
                    mobile_open_reconnect_info[curr_device_id].open_reconnected_count = 0;
                    app_ibrt_customif_cmd_sync_set_reconnect_status(curr_device_id, CONNECT_DEFAULT);
                }
            }
        }
    }
#else
    if(conn_devices == 0)
    {
        if(mobile_open_reconnect_info[curr_device_id].open_reconnected_type == CONNECT_OPEN_RECON)
        {
            if(mobile_open_reconnect_info[curr_device_id].open_reconnected_count >= (IBRT_UI_OPEN_RECONNECT_MOBILE_MAX_TIMES+1))
            {
                mobile_open_reconnect_info[curr_device_id].open_reconnected_type = CONNECT_DEFAULT;
                mobile_open_reconnect_info[curr_device_id].open_reconnected_count = 0;
                app_ibrt_customif_cmd_sync_set_reconnect_status(curr_device_id, CONNECT_DEFAULT);
                enter_pair_flag = true;
            }
        }

        if(enter_pair_flag)
        {
            if((mobile_open_reconnect_info[other_device_id].open_reconnected_type == CONNECT_OPEN_RECON)
            ||(app_bt_mobile_get_reconnect_status()))
            {
                enter_pair_flag = false;
            }
        }
    }
#endif

    BESUI_TRACE(6,"[UIBT]%s pairlist_count %d id %d %d count %d %dflag %d conn_devices %d", __func__, uicom.phone_pair_cnt, curr_device_id, other_device_id,
            mobile_open_reconnect_info[curr_device_id].open_reconnected_count, mobile_open_reconnect_info[other_device_id].open_reconnected_count, enter_pair_flag, conn_devices);

    BESUI_TRACE(4,"[UIBT]%s id %d %d reconnecte type %d %d", __func__, curr_device_id, other_device_id, mobile_open_reconnect_info[curr_device_id].open_reconnected_type, mobile_open_reconnect_info[other_device_id].open_reconnected_type);

    if(enter_pair_flag == true)
    {
        app_open_reconnect_fail_pairmode_timer_onoff(true);
    }
}

void app_reconnect_phone_fail_process(uint8_t curr_device_id)
{
    mobile_reconnect_info[curr_device_id].reconnected_count++;

    BESUI_TRACE(1, "[UIBT]%s id %d cnt %d", __func__, curr_device_id, mobile_reconnect_info[curr_device_id].reconnected_count);

    if(mobile_reconnect_info[curr_device_id].reconnected_count >= IBRT_UI_RECONNECT_MOBILE_MAX_TIMES)
    {
        app_bt_mobile_clear_reconnect_info(curr_device_id);
        app_timeout_poweroff_event_process(); //08 reconnect fail poweroff
    }
}

void app_bt_system_phone_cancel_disconnected_process(uint8_t curr_device_id)
{
    uint8_t conn_devices = besui_get_profile_conn_num();
    if(conn_devices > 0)
    {
        BESUI_TRACE(0,"[UIBT]you shouji lianjie, do not need enter pairmode");
        return;        
    }

    app_bt_system_phone_cancel_enter_pairmode_flag = true;
    app_open_reconnect_fail_pairmode_timer_onoff(true);
}

#if defined(WEAR_DETECT_PROMPT_EN)
#define WEAR_ON_TIMEOUT         (5*2) //5s-500ms
static uint8_t wear_on_cnt = 0;
static uint8_t wear_prompt_start = 1; //1:on  0:off
osTimerId wear_process_timerid = NULL;
void app_wear_timer_onoff(bool timer_en);
void app_wear_process_timehandler(void const *param)
{
	BESUI_TRACE(1, "[UICAP][UITIMER]%s wear_on_cnt = %d", __func__, wear_on_cnt);
	wear_on_cnt ++;
    if(wear_on_cnt >= WEAR_ON_TIMEOUT)
    {
        wear_on_cnt = 0;
        wear_prompt_start = 1;
        osTimerDelete(wear_process_timerid);
        wear_process_timerid = NULL;
    }
    else
    {
        app_wear_timer_onoff(true);
    }
}
osTimerDef (WEAR_PROCESS_TIMER_NAME, (void (*)(void const *))app_wear_process_timehandler);

void app_wear_timer_onoff(bool timer_en)
{
	BESUI_TRACE(1, "[UICAP][UITIMER]%s timer_en %d ", __func__, timer_en);

	if(wear_process_timerid == NULL)
	{
    	wear_process_timerid = osTimerCreate(osTimer(WEAR_PROCESS_TIMER_NAME),osTimerOnce,NULL);
        if(!wear_process_timerid)
             ASSERT(0, "[UICAP][UITIMER][%s] osTimerCreate error", __func__);
	}

	if(timer_en)
		osTimerStart(wear_process_timerid, 500);
	else
		osTimerStop(wear_process_timerid);
}
#endif //#if defined(WEAR_DETECT_PROMPT_EN)

void user_tws_tx_nbytes(uint8_t cmd, uint8_t byte1, uint8_t byte2)
{
    BESUI_TRACE(0, "[UITWSTX]%s", __func__);
    uint8_t ptrParam[3] = {0};
    ptrParam[0] = cmd;
    ptrParam[1] = byte1;
    ptrParam[2] = byte2;
    // tws_ctrl_send_cmd(APP_TWS_CMD_SYNC_SOMETHING, ptrParam, 3);
    DUMP8("%02X ", ptrParam, 3);
}

#ifdef BESUI_PROMPT_ISSUE_EN
bool prompt_flag = 0;
void prompt_set_flag(bool param)
{
    prompt_flag = param;
}
bool prompt_get_flag(void)
{
    return prompt_flag;
}

osTimerId prompt_issue_process_timerid = NULL;
void prompt_issue_timer_onoff(bool timer_en);
void prompt_issue_process_timehandler(void const *param)
{
    static uint8_t cnt = 0;
	BESUI_TRACE(0, "[UITIMER]%s", __func__);

    if(prompt_flag)
    {
        prompt_flag = 0;
        BESUI_TRACE(0, "[UITIMER]%s, prompt already play ...", __func__);
        goto TIMER_END;
    }
    if(besui_get_profile_conn_num())    
    {
        if(!bts_tws_if_is_tws_link_connected())
        {
            media_PlayAudio_locally(AUD_ID_BT_CONNECTED, 0);
        }
        else
        {
            if(user_get_peer_box_sta() == IBRT_IN_BOX_CLOSED) //peer close
            {
                media_PlayAudio_locally(AUD_ID_BT_CONNECTED, 0);
            }
            else
            {
                prompt_issue_timer_onoff(true, 400);
                cnt ++;
                if(cnt < 3)
                    return;
            }
        }
    }
TIMER_END:
    cnt = 0;
    osTimerDelete(prompt_issue_process_timerid);
    prompt_issue_process_timerid = NULL;
}
osTimerDef (PROMPT_ISSUE_PROCESS_TIMER_NAME, (void (*)(void const *))prompt_issue_process_timehandler);

void prompt_issue_timer_onoff(bool timer_en, uint16_t param)
{
	BESUI_TRACE(0, "[UITIMER]%s timer_en %d ", __func__, timer_en);

    if(timer_en == false) //slave first play prompt,and then receive IBRT_CONN_IBRT_ACL_CONNECTED
    {
        BESUI_TRACE(0, "[UITIMER]%s,prompt_flag=%d ", __func__, prompt_flag);
        prompt_set_flag(true);
    }
	if(prompt_issue_process_timerid == NULL)
	{
    	prompt_issue_process_timerid = osTimerCreate(osTimer(PROMPT_ISSUE_PROCESS_TIMER_NAME),osTimerOnce,NULL);
        if(!prompt_issue_process_timerid)
             ASSERT(0, "[UITIMER][%s] osTimerCreate error", __func__);
	}

	if(timer_en)
    {
		osTimerStart(prompt_issue_process_timerid, param);        
    }
	else
    {
		osTimerStop(prompt_issue_process_timerid);
        osTimerDelete(prompt_issue_process_timerid);
        prompt_issue_process_timerid = NULL;
    }
}
#endif

static int besui_bt_msg_handler(APP_MESSAGE_BODY *msg_body)
{
    uint8_t btmsg_event = (uint8_t)msg_body->message_id;
    uint8_t btmsg_type = (uint8_t)msg_body->message_Param0;
    uint8_t btmsg_dat = (uint8_t)msg_body->message_Param2;
    uint8_t conn_devices = besui_get_profile_conn_num();

    BESUI_TRACE(2,"[UIBT]%s, message_id=0x%02X, message_Param0=%d, message_Param2=%d", __func__, btmsg_event, btmsg_type, btmsg_dat);

    if(btmsg_event == OPENRECONNECTED_TIMEOUT_ENTER_PAIRMODE_EVENT)
    {
       if(mobile_open_reconnect_info[btmsg_dat].open_reconnected_type == CONNECT_OPEN_RECON)
            btmsg_event = OPENRECONNECTED_TIMEOUT_ENTER_PAIRMODE_EVENT;
    }
    else if(PHONE_DISCONECTED_EVENT == btmsg_event)
    {
       if(mobile_open_reconnect_info[btmsg_dat].open_reconnected_type == CONNECT_OPEN_RECON)
            btmsg_event = OPENRECONNECTED_TIMEOUT_ENTER_PAIRMODE_EVENT;
    }
    BESUI_TRACE(2,"[UIBT]%s, btmsg_event=0x%02X", __func__, btmsg_event);
    switch(btmsg_event)
    {
        case TWS_CONNECTED_EVENT:
            app_tws_connected_event_process();
            break;

        case TWS_DISCONNECTED_EVENT:
            app_tws_disconnected_event_process(btmsg_type);
            break;

        case PHONE_CONNECTED_EVENT:
            app_bt_phone_connected_event_process();
            break;

        case PHONE_DISCONECTED_EVENT:
            app_bt_phone_disconnected_event_process(btmsg_type);
            break;

        case ENTER_PAIRMODE_EVENT:
            app_enter_pairmode_event_process();
            break;

        case EXIT_PHONEMODE_EVENT:
            app_exit_pairmode_event_process();
            break;

        case TIMEROUT_POWEROFF_EVENT:
            app_timeout_poweroff_event_process();
            break;

        case OPENRECONNECTED_TIMEOUT_ENTER_PAIRMODE_EVENT:
            app_open_reconnect_phone_fail_process(btmsg_dat);
            break;

        case RECONNECTED_TIMEOUT_ENTER_PAIRMODE_EVENT:
            //no use
            app_reconnect_phone_fail_process(btmsg_dat);
            break;

        case PHONE_CANCEL_DISCONNECTED_EVENT:
            app_bt_system_phone_cancel_disconnected_process(btmsg_dat);
            break;
#ifdef BESUI_COMM_EN
        case SLAVE_ENTER_PAIRMODE_EVENT:
            BESUI_TRACE(0,"[UIBT]SLAVE_ENTER_PAIRMODE_EVENT");
            //if((conn_devices == 0) && (app_bt_get_curr_access_mode() == BTIF_BAM_GENERAL_ACCESSIBLE))
            if(conn_devices == 0 && besui_get_phone_disc_type() == 0x08)
            {
                //media_PlayAudio(AUD_ID_BT_DIS_CONNECT, 0);
                app_status_indication_set(APP_STATUS_INDICATION_LINKLOSS_DISCONNECTED);
                app_start_10_second_timer(APP_POWEROFF_TIMER_ID);
                //app_phone_linkloss_pairmode_timer_onoff(true);
                app_poweroff_timer_onoff(true);
            }
            break;
        case SLAVE_LINKLOSS_EVENT:
            if(uicom.phone_pair_cnt == 0)
            {
                besui_enter_pairmode();
            }
            else
            {
/*                 app_status_indication_set(APP_STATUS_INDICATION_PAGESCAN);
                //media_PlayAudio(AUD_ID_BT_DIS_CONNECT, 0);
                app_start_10_second_timer(APP_POWEROFF_TIMER_ID);
                app_phone_discon_pairmode_timer_onoff(true);
                app_poweroff_timer_onoff(true); */
            }
            break;
#endif
        case AUTH_START_EVENT:
            uictl.auth_start_flag = 1;
            BESUI_TRACE(0,"[UIBT]%s, uictl.auth_start_flag = %d", __func__, uictl.auth_start_flag);
            break;
        case AUTH_SUCCESS_EVENT:
            app_phone_discon_pairmode_timer_onoff(false); //too late
            app_phone_pairmode_timer_onoff(false);
            BESUI_TRACE(0,"[UIBT]%s, AUTH_SUCCESS_EVENT, uictl.auth_start_flag = %d", __func__, uictl.auth_start_flag);
            break;
#ifdef BESUI_APP_EN
        case TWS_STATUS_EVENT:
            app_tota_tws_status_to_app(0xFF);
            break;
#endif
        case BT_SINGLE_PAIRMODE:
        app_status_indication_set(APP_STATUS_INDICATION_BOTHSCAN);
        if(conn_devices == 0)
        {
            app_phone_pairmode_timer_onoff(true);
        }
            break;
        case BT_MSG_SOMETHING_EVENT:
        if(btmsg_type == USER_MSG_CMD_2BUDS_CLOSE) //local=CASE_CLOSE   peer=CASE_CLOSE
        {
            BESUI_TRACE(0,"[UIBT]%s, spp ble disconnect", __func__);
#ifdef BESUI_APP_EN
            app_tota_phone_num_to_app(0);   //phone disconnect,number is 0
            app_tota_tws_status_to_app(0);  //tws disconnect
#endif
            ota_disconnect();               //spp disconnect
            bes_ble_gap_disconnect_all();   //ble disconnect
        }
#ifdef USER_LED_CASE_OPEN_EN
        else if(btmsg_type == USER_MSG_CMD_LED_STA_NEXT)
        {
            BESUI_TRACE(0,"[UILED]%s, uictl.led_next = %d", __func__, uictl.led_next);
            app_status_indication_set((APP_STATUS_INDICATION_T)uictl.led_next);
        }
#endif
#if defined(WEAR_DETECT_PROMPT_EN)
        else if(btmsg_type == USER_MSG_CMD_WEAR_PROMPT)
        {
            struct nvrecord_env_t *nvrecord_env;
            nv_record_env_get(&nvrecord_env);
            BESUI_TRACE(0,"[UICAP]%s, wear_prompt_onoff = %d", __func__, nvrecord_env->wear_prompt_onoff);
            if(nvrecord_env->wear_prompt_onoff)
            {
                if(wear_prompt_start)
                {
                    media_PlayAudio_locally(AUD_ID_WEAR_DETECT, 0);
                    wear_prompt_start = 0;
                    app_wear_timer_onoff(true);
                }
            }
        }
#endif
            break;

        default:

            break;
    }

    uictl.role_switch_no = false;
    return 0;
}


#endif
#endif
