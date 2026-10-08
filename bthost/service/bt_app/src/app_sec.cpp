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
#include "apps.h"
#include "hal_trace.h"
#include "me_api.h"
#include "bt_if.h"
#include "hfp_api.h"
#include "a2dp_api.h"
#include "spp_api.h"
#include "nvrecord_bt.h"
#include "app_bt.h"
#include "btm_i.h"
#include "app_trace_rx.h"
#include "app_media_player.h"
#include "audio_player_adapter.h"
#include "app_ibrt_internal.h"

uint8_t pair_status = 0;
uint8_t enter_pair_status = 0;
uint8_t er_into_discover_connectable = 0;
static void app_pair_handler_func(enum pair_event evt, const btif_event_t *event)
{
    switch(evt) {
    case PAIR_EVENT_NUMERIC_REQ:
        break;
    case PAIR_EVENT_COMPLETE:
#if defined(_AUTO_TEST_)
        AUTO_TEST_SEND("Pairing ok.");
#endif

#ifndef FPGA
#ifdef MEDIA_PLAYER_SUPPORT
        if (btif_me_get_callback_event_err_code(event) == BTIF_BEC_NO_ERROR) {
        } else {
            audio_player_play_prompt(AUD_ID_BT_PAIRING_FAIL, 0);
        }
#endif
#endif
#if defined(IBRT)
        nv_record_execute_async_flush();
#elif !defined(__BT_ONE_BRING_TWO__)
        nv_record_flash_flush();
#endif
        break;
    default:
        break;
    }
}

extern "C" void ntt_mobile_pairing_mode_exit(bool pairing_success);

static void pair_handler_func(enum pair_event event, void *data)
{
    event_t _event;
    int err_code = 0;
    enum pair_event app_pair_evt = PAIR_EVENT_COMPLETE;
    bt_pair_state_change_cb_t cb = NULL;

    DEBUG_INFO(1, "!!!pair_handler_func event:%d\n", event);

    switch (event)
    {
        case PAIRING_OK:
        {
            bt_bdaddr_t *paired_addr = (bt_bdaddr_t *)data;
            bool is_tws_peer = false;
        #ifdef IBRT
            ibrt_ctrl_t *p_ibrt_ctrl =
                app_ibrt_if_get_bt_ctrl_ctx();

            /*
            * PAIRING_OK may come from:
            *
            * 1. TWS peer pairing
            * 2. Mobile phone pairing
            *
            * TWS pairing completion must NOT terminate
            * the mobile pairing flow.
            */
            if ((paired_addr != NULL) && (p_ibrt_ctrl != NULL))
            {
                is_tws_peer = (memcmp(paired_addr->address,p_ibrt_ctrl->peer_addr.address,BTIF_BD_ADDR_SIZE) == 0);

                DEBUG_INFO(1,"[NTT_PAIR] PAIRING_OK is_tws_peer=%d\n",is_tws_peer);
                DEBUG_INFO(6,"[NTT_PAIR] paired=%02x:%02x:%02x:%02x:%02x:%02x\n",
                    paired_addr->address[0],
                    paired_addr->address[1],
                    paired_addr->address[2],
                    paired_addr->address[3],
                    paired_addr->address[4],
                    paired_addr->address[5]);

                DEBUG_INFO(6,"[NTT_PAIR] tws_peer=%02x:%02x:%02x:%02x:%02x:%02x\n",
                    p_ibrt_ctrl->peer_addr.address[0],
                    p_ibrt_ctrl->peer_addr.address[1],
                    p_ibrt_ctrl->peer_addr.address[2],
                    p_ibrt_ctrl->peer_addr.address[3],
                    p_ibrt_ctrl->peer_addr.address[4],
                    p_ibrt_ctrl->peer_addr.address[5]);
            }
        #endif

            app_pair_evt = PAIR_EVENT_COMPLETE;
            err_code = 0;
            cb = app_bt_get_pair_state_callback();

            if (cb)
            {
                cb(paired_addr,APP_BT_PAIRED);
            }

            /*
            * TWS peer pairing completed.
            *
            * Do NOT:
            *   - set pair_status = 1
            *   - exit mobile pairing
            *   - stop charging-case pairing LED
            */
            if (is_tws_peer)
            {
                DEBUG_INFO(0,"[NTT_PAIR] TWS PAIRING_OK -> keep mobile pairing active\n");
                break;
            }

            /*
            * Real mobile phone pairing success.
            */
            DEBUG_INFO(0,"[NTT_PAIR] MOBILE PAIRING_OK -> finish pairing / stop case LED\n");
            pair_status = 1;
            set_er_discover_connectable_status(0);
            ntt_mobile_pairing_mode_exit(true);

            break;
        }

        case PAIRING_TIMEOUT:
        {
            DEBUG_INFO(0,"PAIRING_TIMEOUT\n");

            /*
             * Charging-case protocol:
             *
             * pair=1 = pairing finished / LED OFF
             */
            pair_status = 1;

            set_er_discover_connectable_status(0);

            ntt_mobile_pairing_mode_exit(false);

            break;
        }

        case PAIRING_FAILED:
        {
            DEBUG_INFO(0,"PAIRING_FAILED\n");

            err_code = 1;

            app_pair_evt = PAIR_EVENT_COMPLETE;

            /*
             * Pairing failed -> stop charging-case LED.
             */
            pair_status = 1;

            set_er_discover_connectable_status(0);

            ntt_mobile_pairing_mode_exit(false);

            break;
        }

        default:
            break;
    }

    _event.errCode = err_code;

    app_pair_handler_func(app_pair_evt,(btif_event_t *)&_event);

    return;
}

uint8_t get_pair_status(void)
{
    DEBUG_INFO(1,"get_pair_status status:%d\n", pair_status);
	return pair_status;
}

void set_pair_status(uint8_t status)
{
    DEBUG_INFO(1,"set_pair_status status:%d\n", pair_status);
	pair_status = status;
}

uint8_t get_enable_pair_status(void)
{
    DEBUG_INFO(1,"get_enable_pair_status status:%d\n", enter_pair_status);
	return enter_pair_status;
}

void enable_pair_status(uint8_t status)
{
    DEBUG_INFO(1,"enable_pair_status status:%d\n", enter_pair_status);
	enter_pair_status = status;
}

uint8_t get_er_discover_connectable_status(void)
{
    DEBUG_INFO(1,"get_er_discover_connectable_status status:%d\n", er_into_discover_connectable);
	return er_into_discover_connectable;
}

void set_er_discover_connectable_status(uint8_t status)
{
    DEBUG_INFO(3,
        "[NTT][DISCOVER_STATUS] old=%d new=%d LR=%p",
        er_into_discover_connectable,
        status,
        __builtin_return_address(0));

    er_into_discover_connectable = status;
}

int bt_pairing_init(void)
{
    btif_pairing_register_callback(pair_handler_func);
    return 0;
}

