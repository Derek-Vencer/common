/***************************************************************************
 *
 * Copyright 2015-2019 BES.
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a database or retrieval system, without prior written
 * permission of BES.
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary information of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/
#ifndef __APP_IBRT_SM_H__
#define __APP_IBRT_SM_H__

#if defined(IBRT) && defined(IBRT_UI)

#include "hsm.h"
#include "app_ibrt_conn_evt.h"

typedef void (*ibrt_sm_post_func)(void);

typedef struct
{
    Hsm ibrt_sm;

    State idle;
    State w4_profile_exchanged;
    State w4_mss_changed;
    State w4_set_env;
    State w4_start_ibrt_complete;
    State w4_active_mode;
    State w4_stop_ibrt_complete;

    uint8_t used;
    uint8_t device_id;
    bt_bdaddr_t current_mobile_addr;
    ibrt_sm_post_func stop_ibrt_post_func;
    ibrt_mobile_info_t* ibrt_extra_mobile_info;
    uint8_t reconnect_ibrt_max_times;
    uint8_t reconnect_ibrt_try_times;
    uint8_t restop_ibrt_try_times;
    uint8_t try_mss_max_times;
    uint8_t try_mss_times;

    osTimerId           restart_ibrt_timer_id;
    osTimerDefEx_t      restart_ibrt_timer_def;

    // inner use
    bool already_report_ibrt_connected;
} ibrt_state_machine_t;

#ifdef __cplusplus
extern "C" {
#endif

void bts_ibrt_sm_start(ibrt_state_machine_t *ibrt_sm);
bool bts_ibrt_sm_is_idle_state(ibrt_state_machine_t* ibrt_sm);
bool bts_ibrt_sm_is_w4_data_exchange(ibrt_state_machine_t* ibrt_sm);
bool bts_ibrt_sm_is_w4_ibrt(ibrt_state_machine_t* ibrt_sm);
int bts_ibrt_send_ibrt_msg(uint32_t ibrt_sm_ptr,ibrt_sm_message_e evt,uint32_t param0,uint32_t param1);
int bts_ibrt_sm_on_event(unsigned int event,unsigned int ibrt_link_ptr,unsigned int param0, unsigned int param1);
void bts_ibrt_set_exchange_profiles_status(ibrt_state_machine_t *ibrt_sm,bool status);
void bts_ibrt_fast_free_ibrt_link_sm(ibrt_state_machine_t *p_ibrt_link_sm,uint8_t reason);
void app_tws_ibrt_register_need_reconnect_cb(void (*cb)(struct bdaddr_t *addr, uint8 reason));


#ifdef __cplusplus
}
#endif

#endif

#endif /*__APP_IBRT_SM_H__ */
