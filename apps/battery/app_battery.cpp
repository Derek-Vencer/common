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
#include "cmsis_os.h"
#include "tgt_hardware.h"
#include "pmu.h"
#include "hal_timer.h"
#include "hal_gpadc.h"
#include "hal_trace.h"
#include "hal_gpio.h"
#include "hal_iomux.h"
#include "hal_chipid.h"
#include "app_thread.h"
#include "app_battery.h"
#include "audio_policy.h"
#include "app_bt.h"
#include "apps.h"
#include "app_hfp.h"
#include "bts_tws_if.h"
#include "earbud_ux_api.h"
#include "bts_core_type.h"
#include "app_ui_api.h"
#include "../earbuds/conn/app_ibrt_customif_cmd.h"
#include "../btapp/bt_app/app_keyhandle.h"

#ifdef APP_BATTERY_ENABLE
#include "app_status_ind.h"
#include "bluetooth_bt_api.h"
#include "app_media_player.h"
#ifdef BT_USB_AUDIO_DUAL_MODE
#include "btusb_audio.h"
#endif
#include <stdlib.h>

#ifdef __INTERCONNECTION__
#include "bluetooth_ble_api.h"
#endif

#if defined(BESUI_TWS_EN)
#include "besui_define.h"
#include "twsui_comm.h"
#include "twsui_charge.h"
#include "app_ui_api.h"
#ifdef BESUI_1WIRE_EN
#include "communication_svr.h"
#endif
#endif
#if defined(BESUI_TWS_EN) || defined(BESUI_STEREO_EN)
#include "besui_common.h"
#endif
#ifdef BESUI_STEREO_EN
#include "stereoui.h"
#endif

#include "app_ble.h"
#include "communication_svr.h"

#if (defined(BTUSB_AUDIO_MODE) || defined(BTUSB_AUDIO_MODE))
extern "C" bool app_usbaudio_mode_on(void);
#endif

#ifdef MORE_THAN_ONE_TYPE_OF_CHARGER
#include CHIP_SPECIFIC_HDR(charger)
#endif

#ifndef APP_BATTERY_GPADC_CH_NUM
#define APP_BATTERY_GPADC_CH_NUM (HAL_GPADC_CHAN_BATTERY)
#endif

#ifndef GPADC_VBAT_VOLT_DIV
#define GPADC_VBAT_VOLT_DIV (4)
#endif

#ifndef APP_BATTERY_ERR_MV
#define APP_BATTERY_ERR_MV (2800)
#endif

#ifndef APP_BATTERY_MIN_MV
#define APP_BATTERY_MIN_MV (3656)//3350
#endif

#ifndef APP_BATTERY_MAX_MV
#define APP_BATTERY_MAX_MV (4200)
#endif

#ifndef APP_BATTERY_PD_MV
#define APP_BATTERY_PD_MV   (3100)
#endif

#ifndef APP_BATTERY_CHARGE_TIMEOUT_MIN
#define APP_BATTERY_CHARGE_TIMEOUT_MIN (90)
#endif

#ifndef APP_BATTERY_CHARGE_OFFSET_MV
#define APP_BATTERY_CHARGE_OFFSET_MV (20)
#endif

#ifndef CHARGER_PLUGINOUT_RESET
#define CHARGER_PLUGINOUT_RESET (1)
#endif

#ifndef CHARGER_PLUGINOUT_DEBOUNCE_MS
#define CHARGER_PLUGINOUT_DEBOUNCE_MS (200)//50
#endif

#ifndef CHARGER_PLUGINOUT_DEBOUNCE_CNT
#define CHARGER_PLUGINOUT_DEBOUNCE_CNT (3)
#endif

#ifdef BESUI_TWS_EN
#undef CHARGER_PLUGINOUT_DEBOUNCE_CNT
#define CHARGER_PLUGINOUT_DEBOUNCE_CNT (10)
#endif

#define APP_BATTERY_CHARGING_PLUGOUT_DEDOUNCE_CNT (APP_BATTERY_CHARGING_PERIODIC_MS<500?3:1)

#define APP_BATTERY_CHARGING_EXTPIN_MEASURE_CNT (APP_BATTERY_CHARGING_PERIODIC_MS<2*1000?2*1000/APP_BATTERY_CHARGING_PERIODIC_MS:1)
#define APP_BATTERY_CHARGING_EXTPIN_DEDOUNCE_CNT (6)

#define APP_BATTERY_CHARGING_OVERVOLT_MEASURE_CNT (APP_BATTERY_CHARGING_PERIODIC_MS<2*1000?2*1000/APP_BATTERY_CHARGING_PERIODIC_MS:1)
#define APP_BATTERY_CHARGING_OVERVOLT_DEDOUNCE_CNT (3)

#define APP_BATTERY_CHARGING_SLOPE_MEASURE_CNT (APP_BATTERY_CHARGING_PERIODIC_MS<20*1000?20*1000/APP_BATTERY_CHARGING_PERIODIC_MS:1)
#define APP_BATTERY_CHARGING_SLOPE_TABLE_COUNT (6)


#define APP_BATTERY_REPORT_INTERVAL (5)

#define APP_BATTERY_MV_BASE ((APP_BATTERY_MAX_MV-APP_BATTERY_PD_MV)/(APP_BATTERY_LEVEL_NUM))

#define APP_BATTERY_STABLE_COUNT (5)
#define APP_BATTERY_MEASURE_PERIODIC_FAST_MS (200)
#ifdef BLE_ONLY_ENABLED
#define APP_BATTERY_MEASURE_PERIODIC_NORMAL_MS (25000)
#else
#define APP_BATTERY_MEASURE_PERIODIC_NORMAL_MS (10*1000) // 10sec
#endif
#define APP_BATTERY_CHARGING_PERIODIC_MS (APP_BATTERY_MEASURE_PERIODIC_NORMAL_MS)

#define LOW_BAT_VOICE_INTERVAL_COUNT    (30) //5min

static uint16_t g_low_bat_voice_count = 0;
static bool g_low_bat_voice_first = true;

#define APP_BATTERY_SET_MESSAGE(appevt, status, volt) (appevt = (((uint32_t)status&0xffff)<<16)|(volt&0xffff))
#define APP_BATTERY_GET_STATUS(appevt, status) (status = (appevt>>16)&0xffff)
#define APP_BATTERY_GET_VOLT(appevt, volt) (volt = appevt&0xffff)
#define APP_BATTERY_GET_PRAMS(appevt, prams) ((prams) = appevt&0xffff)
#if defined(IBRT)
extern void app_ibrt_customif_cmd_sync_battery_level(uint8_t current_level);
extern "C" uint8_t icp1205_get_chrg_sts2(void);
/*
 * Implemented by the Sparrow BLE service. It sends the three-byte battery
 * payload through the proactive 0x31 Notify packet.
 */
extern void sparrow_push_battery_level_notify(bool slave_disconnected);
#endif
enum APP_BATTERY_MEASURE_PERIODIC_T
{
    APP_BATTERY_MEASURE_PERIODIC_FAST = 0,
    APP_BATTERY_MEASURE_PERIODIC_NORMAL,
    APP_BATTERY_MEASURE_PERIODIC_CHARGING,

    APP_BATTERY_MEASURE_PERIODIC_QTY,
};

struct APP_BATTERY_MEASURE_CHARGER_STATUS_T
{
    HAL_GPADC_MV_T prevolt;
    int32_t slope_1000[APP_BATTERY_CHARGING_SLOPE_TABLE_COUNT];
    int slope_1000_index;
    int cnt;
};

typedef void (*APP_BATTERY_EVENT_CB_T)(enum APP_BATTERY_STATUS_T, APP_BATTERY_MV_T volt);

struct APP_BATTERY_MEASURE_T
{
    uint32_t start_time;
    enum APP_BATTERY_STATUS_T status;
#ifdef __INTERCONNECTION__
    uint8_t currentBatteryInfo;
    uint8_t lastBatteryInfo;
    uint8_t isMobileSupportSelfDefinedCommand;
#else
    uint8_t currlevel;
#endif
    APP_BATTERY_MV_T currvolt;
    APP_BATTERY_MV_T lowvolt;
    APP_BATTERY_MV_T highvolt;
    APP_BATTERY_MV_T pdvolt;
    uint32_t chargetimeout;
    enum APP_BATTERY_MEASURE_PERIODIC_T periodic;
    HAL_GPADC_MV_T voltage[APP_BATTERY_STABLE_COUNT];
    uint16_t index;
    struct APP_BATTERY_MEASURE_CHARGER_STATUS_T charger_status;
    APP_BATTERY_EVENT_CB_T cb;
    APP_BATTERY_CB_T user_cb;
};

static enum APP_BATTERY_CHARGER_T g_ntt_last_case_state = APP_BATTERY_CHARGER_QTY;

#ifdef IS_BES_BATTERY_MANAGER_ENABLED

static enum APP_BATTERY_CHARGER_T app_battery_charger_forcegetstatus(void);

static void app_battery_pluginout_debounce_start(void);
static void app_battery_pluginout_debounce_handler(void const *param);
osTimerDef (APP_BATTERY_PLUGINOUT_DEBOUNCE, app_battery_pluginout_debounce_handler);
static osTimerId app_battery_pluginout_debounce_timer = NULL;
static uint32_t app_battery_pluginout_debounce_ctx = 0;
static uint32_t app_battery_pluginout_debounce_cnt = 0;
#ifdef IBRT
extern "C" void ntt_case_close_role_switch_reset(void);
extern "C" bool ntt_case_close_role_switch_can_shutdown(void);
static uint8_t aiWangReportNormalLevelHandler(uint16_t current_voltage);
/*
 * Battery measurement valid flag.
 * False means currvolt/currlevel still contain boot default values.
 */
static bool g_battery_measure_valid = false;

/*
 * Last valid battery percentage.
 * Used to prevent boot default 4200mV from being reported as 100%.
 */
static uint8_t g_battery_last_valid_percent = 0xFF;
/*
 * True means OPEN_CASE has canceled the current case-close transaction.
 */
static bool g_ntt_case_open_cancel = false;

extern void wired_uart_get_battery_level(void);

#endif
#ifdef IBRT
extern "C" void ntt_case_state_sync_local_update(
    bool in_case);
#endif
#ifndef MORE_THAN_ONE_TYPE_OF_CHARGER
const
#endif
static enum HAL_GPADC_CHAN_T app_vbat_ch = APP_BATTERY_GPADC_CH_NUM;

#ifndef MORE_THAN_ONE_TYPE_OF_CHARGER
const
#endif
static uint8_t app_vbat_volt_div = GPADC_VBAT_VOLT_DIV;

static void app_battery_timer_handler(void const *param);
osTimerDef (APP_BATTERY, app_battery_timer_handler);
static osTimerId app_battery_timer = NULL;
static struct APP_BATTERY_MEASURE_T app_battery_measure;

//fixed wrong close the earBuds,when charger open
static void earBudsCloseOff_PogonIn_handler(void const *param);
osTimerDef (POGONIN_CLOSE_TIMER, earBudsCloseOff_PogonIn_handler);
static osTimerId pogonPinCloseTimer = NULL;
extern bool  aiWangIsNeedOpenEarBuds(void);

#ifdef IBRT
extern "C" bool ntt_case_close_try_role_switch_before_shutdown(void);
#endif
void earBudsCloseOff_PogonIn_StartTimer(void);
void earBudsCloseOff_PogonIn_StopTimer(void);
extern bool ntt_charging_pwron_pending_shutdown;
extern "C" void wired_uart_mobile_connected_get_box_battery(void);

#define POGONIN_CLOSE_CHECK_INTERVAL_MS          (3000)
#define POGONIN_ROLE_SWITCH_WAIT_MAX_COUNT       (10)

#ifdef IBRT
static bool g_pogonin_role_switch_requested = false;
static uint8_t g_pogonin_role_switch_wait_count = 0;
#endif

#define LOW_BAT_ROLE_SWITCH_PENDING_TIMEOUT_COUNT    (3)

/*
 * ntt_low_battery_role_switch_check() is called every 10 seconds.
 * 30 calls = 300 seconds.
 */
#define LOW_BAT_ROLE_BATTERY_CHECK_COUNT             (30)

static bool g_low_bat_role_switch_pending = false;
static uint8_t g_low_bat_role_switch_pending_count = 0;
static uint8_t g_low_bat_role_battery_check_count = 0;

static void ntt_low_battery_role_switch_check(void)
{
    uint8_t local_level;
    uint8_t peer_percent;
    uint8_t peer_level;
    TWS_UI_ROLE_E current_role;

    current_role = app_ibrt_if_get_ui_role();

    /*
     * Clear pending state after local ear becomes Slave.
     */
    if (current_role != TWS_UI_MASTER)
    {
        if (g_low_bat_role_switch_pending)
        {
            BATTERY_TRACE(0,
                          "[LOW_BAT_ROLE] switch complete, local is SLAVE");
        }

        g_low_bat_role_switch_pending = false;
        g_low_bat_role_switch_pending_count = 0;
        g_low_bat_role_battery_check_count = 0;
        return;
    }

    /*
     * TWS link must be connected.
     */
    if (!bts_tws_if_is_tws_link_connected())
    {
        g_low_bat_role_switch_pending = false;
        g_low_bat_role_switch_pending_count = 0;
        g_low_bat_role_battery_check_count = 0;

        BATTERY_TRACE(0,
                      "[LOW_BAT_ROLE] TWS disconnected, skip");
        return;
    }

    /*
     * Avoid repeated requests while role switch is in progress.
     * This function is called every 10 seconds.
     */
    if (g_low_bat_role_switch_pending)
    {
        g_low_bat_role_switch_pending_count++;

        BATTERY_TRACE(2,
                      "[LOW_BAT_ROLE] switch pending cnt=%d",
                      g_low_bat_role_switch_pending_count);

        /*
         * If role switch does not complete within about 30 seconds,
         * clear pending state and allow another request.
         */
        if (g_low_bat_role_switch_pending_count <
            LOW_BAT_ROLE_SWITCH_PENDING_TIMEOUT_COUNT)
        {
            return;
        }

        BATTERY_TRACE(0,
                      "[LOW_BAT_ROLE] switch pending timeout, retry allowed");

        g_low_bat_role_switch_pending = false;
        g_low_bat_role_switch_pending_count = 0;
    }

    /*
     * Battery comparison is performed once every 300 seconds.
     *
     * This function is called every 10 seconds:
     * 10 seconds x 30 = 300 seconds.
     */
    g_low_bat_role_battery_check_count++;

    if (g_low_bat_role_battery_check_count <
        LOW_BAT_ROLE_BATTERY_CHECK_COUNT)
    {
        return;
    }

    g_low_bat_role_battery_check_count = 0;

    BATTERY_TRACE(0,
                  "[LOW_BAT_ROLE] 300s battery check");

    /*
     * Local battery uses the same HFP level table.
     */
    local_level =
        aiWangReportNormalLevelHandler(
            app_battery_measure.currvolt);

    /*
     * Peer battery is synchronized as 0~100%.
     */
    peer_percent =
        app_ibrt_customif_get_tws_peer_battery_level();

    if ((peer_percent == 0xFF) ||
        (peer_percent > 100))
    {
        BATTERY_TRACE(1,
                      "[LOW_BAT_ROLE] invalid peer=%d",
                      peer_percent);
        return;
    }

    /*
     * Convert peer percentage to HFP-style 0~9 level.
     */
    if (peer_percent >= 100)
    {
        peer_level = 9;
    }
    else if (peer_percent < 10)
    {
        peer_level = 0;
    }
    else
    {
        peer_level =
            (uint8_t)((peer_percent / 10) - 1);
    }

    BATTERY_TRACE(5,
                  "[LOW_BAT_ROLE] role=%d volt=%d local=L%d peer=%d%% L%d",
                  current_role,
                  app_battery_measure.currvolt,
                  local_level,
                  peer_percent,
                  peer_level);

    /*
     * Peer must be at least one level higher.
     */
    if (peer_level <= local_level)
    {
        BATTERY_TRACE(0,
                      "[LOW_BAT_ROLE] no switch");
        return;
    }

    BATTERY_TRACE(0,
                  "[LOW_BAT_ROLE] peer higher -> role switch");

    /*
     * Set pending before sending the asynchronous role-switch request.
     */
    g_low_bat_role_switch_pending = true;
    g_low_bat_role_switch_pending_count = 0;

    app_ui_user_role_switch(false);
}

void ntt_case_poweroff_cancel(void)
{
    /*
     * Mark the current case-close transaction as canceled.
     */
    g_ntt_case_open_cancel = true;

    /*
     * Invalidate pending shutdown request first.
     */
    ntt_charging_pwron_pending_shutdown = false;

    /*
     * Stop pending Pogo-In shutdown timer.
     */
    earBudsCloseOff_PogonIn_StopTimer();

#ifdef IBRT
    /*
     * Cancel pending role-switch state.
     */
    g_pogonin_role_switch_requested = false;
    g_pogonin_role_switch_wait_count = 0;
    ntt_case_close_role_switch_reset();
#endif

    BATTERY_TRACE(0,"[POWER_OFF] canceled by OPEN_CASE");
}

void ntt_case_poweroff_enable(void)
{
    g_ntt_case_open_cancel = false;
    ntt_charging_pwron_pending_shutdown = true;
    BATTERY_TRACE(0,"[POWER_OFF] enabled by POWER_OFF");
    BATTERY_TRACE(2,"[POWER_OFF] enabled cancel=%d pending=%d",g_ntt_case_open_cancel,ntt_charging_pwron_pending_shutdown);
}

bool ntt_case_poweroff_is_open_cancelled(void)
{
    return g_ntt_case_open_cancel;
}

static void earBudsCloseOff_PogonIn_handler(void const *param)
{
    int8_t charging = app_battery_is_charging();
    uint8_t chrg_sts2 = icp1205_get_chrg_sts2();
    uint8_t case_state = ntt_case_state_get_local();

    bool no_charge_case_closed =
        ((chrg_sts2 == 0x00) ||
         (chrg_sts2 == 0x80));

    BATTERY_TRACE(
        5,
        "[POWER_OFF] PogonIn handler charging=%d pending=%d "
        "STS2=0x%02X open_cancel=%d case=%d",
        charging,
        ntt_charging_pwron_pending_shutdown,
        chrg_sts2,
        g_ntt_case_open_cancel,
        case_state);

    /*
     * Timer callback already obsolete.
     */
    if (!ntt_charging_pwron_pending_shutdown)
    {
        BATTERY_TRACE(
            0,
            "[POWER_OFF] pending=0 -> ignore stale timer callback");
        return;
    }

    /*
     * Priority 1:
     * Explicit OPEN_CASE always cancels shutdown.
     */
    if (g_ntt_case_open_cancel)
    {
        BATTERY_TRACE(
            3,
            "[POWER_OFF] OPEN_CASE confirmed "
            "charging=%d STS2=0x%02X case=%d "
            "-> cancel shutdown",
            charging,
            chrg_sts2,
            case_state);

        ntt_charging_pwron_pending_shutdown = false;

#ifdef IBRT
        g_pogonin_role_switch_requested = false;
        g_pogonin_role_switch_wait_count = 0;
        ntt_case_close_role_switch_reset();
#endif

        return;
    }

    /*
     * Priority 2:
     * Confirmed OUT_CASE must override STS2=0x00 / 0x80.
     *
     * This is required for OTA reboot outside the charging case:
     *   charging=0
     *   STS2=0x00/0x80
     *   case=OUT_CASE
     *
     * In this situation the earbud must stay ON.
     */
    if (case_state == NTT_CASE_STATE_OUT_CASE)
    {
        BATTERY_TRACE(
            3,
            "[POWER_OFF] OUT_CASE confirmed "
            "charging=%d STS2=0x%02X "
            "-> cancel pending shutdown",
            charging,
            chrg_sts2);

        ntt_charging_pwron_pending_shutdown = false;

#ifdef IBRT
        g_pogonin_role_switch_requested = false;
        g_pogonin_role_switch_wait_count = 0;
        ntt_case_close_role_switch_reset();
#endif

        return;
    }

    /*
     * Priority 3:
     * If charging is removed, normally cancel shutdown.
     *
     * Exception:
     * STS2=0x00 / 0x80 is allowed only when OUT_CASE
     * has NOT been confirmed.
     */
    if (!charging)
    {
        if (no_charge_case_closed)
        {
            BATTERY_TRACE(
                3,
                "[NTT_POWER] charging=0 STS2=0x%02X "
                "case=%d -> allow Case-Close shutdown",
                chrg_sts2,
                case_state);
        }
        else
        {
            BATTERY_TRACE(
                4,
                "[POWER_OFF] charging=0 STS2=0x%02X "
                "case=%d -> cancel shutdown",
                chrg_sts2,
                case_state);

            ntt_charging_pwron_pending_shutdown = false;

#ifdef IBRT
            g_pogonin_role_switch_requested = false;
            g_pogonin_role_switch_wait_count = 0;
            ntt_case_close_role_switch_reset();
#endif

            return;
        }
    }

    /*
     * UNKNOWN may occur during early boot.
     * Do not cancel here yet.
     *
     * OUT_CASE has already been handled above.
     */
    if (case_state != NTT_CASE_STATE_IN_CASE)
    {
        BATTERY_TRACE(
            3,
            "[POWER_OFF] case not IN_CASE yet "
            "case=%d charging=%d STS2=0x%02X "
            "-> continue pending validation",
            case_state,
            charging,
            chrg_sts2);
    }

    BATTERY_TRACE(
        0,
        "[POWER_OFF] condition valid -> check role switch");

#ifdef IBRT

    if (g_pogonin_role_switch_requested)
    {
        /*
         * Case state may change while waiting role switch.
         * Check again before continuing.
         */
        if (g_ntt_case_open_cancel ||
            ntt_case_state_get_local() == NTT_CASE_STATE_OUT_CASE)
        {
            BATTERY_TRACE(
                2,
                "[POWER_OFF] role switch wait canceled "
                "by OPEN_CASE/OUT_CASE");

            g_pogonin_role_switch_requested = false;
            g_pogonin_role_switch_wait_count = 0;
            ntt_case_close_role_switch_reset();

            ntt_charging_pwron_pending_shutdown = false;

            return;
        }

        if (ntt_case_close_role_switch_can_shutdown())
        {
            set_pair_status(1);
            wired_uart_get_battery_level();

            /*
             * Final check before shutdown.
             */
            if (g_ntt_case_open_cancel ||
                ntt_case_state_get_local() == NTT_CASE_STATE_OUT_CASE)
            {
                BATTERY_TRACE(
                    2,
                    "[POWER_OFF] final shutdown canceled "
                    "by OPEN_CASE/OUT_CASE");

                g_pogonin_role_switch_requested = false;
                g_pogonin_role_switch_wait_count = 0;
                ntt_case_close_role_switch_reset();

                ntt_charging_pwron_pending_shutdown = false;

                return;
            }

            BATTERY_TRACE(
                0,
                "[POWER_OFF] role switch done, shutdown now");

            g_pogonin_role_switch_requested = false;
            g_pogonin_role_switch_wait_count = 0;
            ntt_charging_pwron_pending_shutdown = false;

            app_shutdown();
            return;
        }

        g_pogonin_role_switch_wait_count++;

        BATTERY_TRACE(
            1,
            "[POWER_OFF] wait role switch cnt=%d",
            g_pogonin_role_switch_wait_count);

        if (g_pogonin_role_switch_wait_count <
            POGONIN_ROLE_SWITCH_WAIT_MAX_COUNT)
        {
            earBudsCloseOff_PogonIn_StartTimer();
            return;
        }

        /*
         * Role switch timeout:
         * Validate again before forced shutdown.
         */
        if (g_ntt_case_open_cancel ||
            ntt_case_state_get_local() == NTT_CASE_STATE_OUT_CASE)
        {
            BATTERY_TRACE(
                2,
                "[POWER_OFF] role switch timeout but "
                "OPEN_CASE/OUT_CASE -> cancel shutdown");

            g_pogonin_role_switch_requested = false;
            g_pogonin_role_switch_wait_count = 0;
            ntt_case_close_role_switch_reset();

            ntt_charging_pwron_pending_shutdown = false;

            return;
        }

        BATTERY_TRACE(
            0,
            "[POWER_OFF] role switch timeout, shutdown");

        g_pogonin_role_switch_requested = false;
        g_pogonin_role_switch_wait_count = 0;
        ntt_charging_pwron_pending_shutdown = false;

        app_shutdown();
        return;
    }

    /*
     * Check before requesting role switch.
     */
    if (g_ntt_case_open_cancel ||
        ntt_case_state_get_local() == NTT_CASE_STATE_OUT_CASE)
    {
        BATTERY_TRACE(
            2,
            "[POWER_OFF] before role switch "
            "OPEN_CASE/OUT_CASE -> cancel");

        ntt_charging_pwron_pending_shutdown = false;
        return;
    }

    if (ntt_case_close_try_role_switch_before_shutdown())
    {
        BATTERY_TRACE(
            0,
            "[POWER_OFF] role switch requested");

        g_pogonin_role_switch_requested = true;
        g_pogonin_role_switch_wait_count = 0;

        earBudsCloseOff_PogonIn_StartTimer();
        return;
    }

#endif

    /*
     * Final protection before app_shutdown().
     */
    if (g_ntt_case_open_cancel ||
        ntt_case_state_get_local() == NTT_CASE_STATE_OUT_CASE)
    {
        BATTERY_TRACE(
            3,
            "[POWER_OFF] final shutdown canceled "
            "open_cancel=%d case=%d",
            g_ntt_case_open_cancel,
            ntt_case_state_get_local());

        ntt_charging_pwron_pending_shutdown = false;
        return;
    }

    BATTERY_TRACE(
        0,
        "[POWER_OFF] shutdown now");

    ntt_charging_pwron_pending_shutdown = false;

    app_shutdown();
}

void earBudsCloseOff_PogonIn_StartTimer(void)
{
    if (NULL == pogonPinCloseTimer)
    {
        pogonPinCloseTimer = osTimerCreate(osTimer(POGONIN_CLOSE_TIMER), osTimerOnce, NULL);
    }

    if (NULL == pogonPinCloseTimer)
    {
        BATTERY_TRACE(0,"[POWER_OFF] create close-case timer failed");
        return;
    }

    osTimerStop(pogonPinCloseTimer);
    osTimerStart(pogonPinCloseTimer, POGONIN_CLOSE_CHECK_INTERVAL_MS);
}

void earBudsCloseOff_PogonIn_StopTimer(void)
{
    if (NULL != pogonPinCloseTimer)
    {
        osTimerStop(pogonPinCloseTimer);
    }
    BATTERY_TRACE(0,"[CASE_OPEN] stop close-case power off timer");
}

void earBudsCloseOff_PowerOff_StartTimer(void)
{
    if (NULL == pogonPinCloseTimer)
    {
        pogonPinCloseTimer = osTimerCreate(osTimer(POGONIN_CLOSE_TIMER),osTimerOnce,NULL);
    }

    osTimerStop(pogonPinCloseTimer);
    osTimerStart(pogonPinCloseTimer, 1600);
    BATTERY_TRACE(0,"[POWER_OFF] start 1.6s charger check timer");
}

//-------------------------------------------------------------------------------------------


extern "C" void aw_ntc_detect_process(uint16_t ad_volt);
static int app_battery_charger_handle_process(void);
static uint8_t aiWangReportNormalLevelHandler(uint16_t current_voltage){
	static const int battery_table_level[11] = {4140,4077,3987,3918,3847,3800,3768,3742,3705,3656}; //unit:mv
	uint8_t level = 0;
	uint8_t index = 0;
	//uint16_t last_mv = battery_table_level[0];
	for (index = 0; index < sizeof(battery_table_level)/sizeof(battery_table_level[0]); index++)
	{
		if(app_battery_measure.currvolt >= battery_table_level[index]) {
			level   = 10 - index;
			//last_mv = battery_table_level[0];
			break;
		}
	}
	return level;
}

/*
 * App 專用精細電量百分比。
 *
 * 依照 SBC Battery Voltage Curve：
 *
 *   0% = 3200mV
 *  10% = 3656mV
 *  20% = 3705mV
 *  30% = 3742mV
 *  40% = 3768mV
 *  50% = 3800mV
 *  60% = 3847mV
 *  70% = 3918mV
 *  80% = 3987mV
 *  90% = 4077mV
 * 100% = 4140mV
 *
 * 各區間內使用線性內插。
 */
uint8_t app_battery_get_precise_percent(void)
{
    uint16_t volt;
    uint8_t percent = 0;

    /*
     * Do not use the boot default voltage before the first
     * valid ADC battery measurement is available.
     */
    if (!g_battery_measure_valid)
    {
        BATTERY_TRACE(2,"[BAT_PRECISE] invalid, boot volt=%d last=%d",app_battery_measure.currvolt,g_battery_last_valid_percent);
        return 0xFF;
    }

    /*
     * Battery voltage is valid after the first ADC measurement.
     */
    volt = app_battery_measure.currvolt;

    /*
     * Battery voltage thresholds in ascending order.
     */
    static const uint16_t voltage_table[] =
    {
        3200,   //   0%
        3656,   //  10%
        3705,   //  20%
        3742,   //  30%
        3768,   //  40%
        3800,   //  50%
        3847,   //  60%
        3918,   //  70%
        3987,   //  80%
        4077,   //  90%
        4140    // 100%
    };

    static const uint8_t percent_table[] =
    {
          0,
         10,
         20,
         30,
         40,
         50,
         60,
         70,
         80,
         90,
        100
    };

    const uint8_t table_count = sizeof(voltage_table) / sizeof(voltage_table[0]);

    /*
     * 3200mV or below = 0%.
     */
    if (volt <= voltage_table[0])
    {
        percent = 0;
    }
    /*
     * 4195mV or above = 100%.
     */
    else if (volt >= voltage_table[table_count - 1])
    {
        percent = 100;
    }
    else
    {
        for (uint8_t i = 0; i < (table_count - 1); i++)
        {
            uint16_t low_volt = voltage_table[i];
            uint16_t high_volt = voltage_table[i + 1];
            if ((volt >= low_volt) && (volt < high_volt))
            {
                uint8_t low_percent = percent_table[i];
                uint8_t high_percent = percent_table[i + 1];
                uint32_t volt_offset = (uint32_t)(volt - low_volt);
                uint32_t volt_range = (uint32_t)(high_volt - low_volt);
                uint32_t percent_range = (uint32_t)(high_percent - low_percent);

                /*
                 * Linear interpolation.
                 */
                percent = low_percent + (uint8_t)((volt_offset * percent_range) / volt_range);
                break;
            }
        }
    }

    /*
     * Save the latest valid percentage.
     */
    g_battery_last_valid_percent = percent;
    BATTERY_TRACE(2,"[BAT_PRECISE] volt=%d percent=%d",volt,percent);
    return percent;
}

uint8_t app_battery_get_percent(void)
{
    static const uint16_t battery_voltage_table[] =
    {
        3200, /*   0% */
        3656, /*  10% */
        3705, /*  20% */
        3742, /*  30% */
        3768, /*  40% */
        3800, /*  50% */
        3847, /*  60% */
        3918, /*  70% */
        3987, /*  80% */
        4077, /*  90% */
        4140, /* 100% */
    };

    static const uint8_t battery_percent_table[] =
    {
          0,
         10,
         20,
         30,
         40,
         50,
         60,
         70,
         80,
         90,
        100,
    };

    const uint8_t table_count = sizeof(battery_voltage_table) / sizeof(battery_voltage_table[0]);
    uint16_t volt = app_battery_measure.currvolt;
    uint8_t percent = 0;
    uint8_t index;

    /*
     * Battery discharge curve:
     *
     * 4195mV = 100%
     * 4077mV =  90%
     * 3987mV =  80%
     * 3918mV =  70%
     * 3847mV =  60%
     * 3800mV =  50%
     * 3768mV =  40%
     * 3742mV =  30%
     * 3705mV =  20%
     * 3656mV =  10%
     * 3200mV =   0%
     */

    if (volt <= battery_voltage_table[0])
    {
        percent = 0;
    }
    else if (volt >= battery_voltage_table[table_count - 1])
    {
        percent = 100;
    }
    else
    {
        for (index = 1; index < table_count; index++)
        {
            if (volt <= battery_voltage_table[index])
            {
                uint16_t volt_low;
                uint16_t volt_high;
                uint16_t volt_range;
                uint16_t volt_offset;

                uint8_t percent_low;
                uint8_t percent_high;
                uint8_t percent_range;

                volt_low = battery_voltage_table[index - 1];
                volt_high = battery_voltage_table[index];
                percent_low = battery_percent_table[index - 1];
                percent_high = battery_percent_table[index];
                volt_range = volt_high - volt_low;
                volt_offset = volt - volt_low;
                percent_range = percent_high - percent_low;

                /*
                 * Perform piecewise linear interpolation.
                 * Add half of the divisor for nearest rounding.
                 */
                percent = percent_low + (uint8_t)(((uint32_t)volt_offset * percent_range + (volt_range / 2)) / volt_range);

                break;
            }
        }
    }

    BATTERY_TRACE(2,"[BAT_PERCENT] volt=%d percent=%d",volt,percent);
    return percent;
}

uint8_t app_battery_get_display_percent(void)
{
    uint8_t level =
        aiWangReportNormalLevelHandler(app_battery_measure.currvolt);

    /*
     * 與目前 app_status_battery_report() 的 BLE 顯示規則一致。
     *
     * level 0 -> 10%
     * level 1 -> 20%
     * ...
     * level 8 -> 90%
     * level 9/10 -> 100%
     */
    if (level >= 9)
    {
        return 100;
    }

    return (uint8_t)((level + 1) * 10);
}

void app_battery_notify_peer_percent_changed(void)
{
#if defined(IBRT)
    if (app_ibrt_if_get_ui_role() != TWS_UI_MASTER)
    {
        BATTERY_TRACE(1,
                      "[BAT31] peer update ignored: local is not MASTER");
        return;
    }

    BATTERY_TRACE(1, "[BAT31] peer battery changed -> push");
    sparrow_push_battery_level_notify(false);
#endif
}

void app_battery_notify_tws_slave_disconnected(void)
{
#if defined(IBRT)
    if (app_ibrt_if_get_ui_role() != TWS_UI_MASTER)
    {
        BATTERY_TRACE(1,
                      "[BAT31] slave disconnect ignored: local is not MASTER");
        return;
    }

    BATTERY_TRACE(0, "[BAT31] slave disconnected -> push peer=FF");
    sparrow_push_battery_level_notify(true);
#endif
}

#ifdef BESUI_STEREO_EN
int app_ui_battery_charger_handle_process(void)
{
    return app_battery_charger_handle_process();
}
APP_BATTERY_STATUS_T app_battery_status(void)
{
    return app_battery_measure.status;
}
void app_ui_power_off_event();
#ifdef STEREO_HALL_EN
bool app_ui_charging_io_read(HAL_GPIO_PIN_T hal_pin);
bool app_get_charging_status(void);
#endif
#endif

#ifdef __INTERCONNECTION__
uint8_t* app_battery_get_mobile_support_self_defined_command_p(void)
{
    return &app_battery_measure.isMobileSupportSelfDefinedCommand;
}
#endif

void app_battery_irqhandler(uint16_t irq_val, HAL_GPADC_MV_T volt)
{
    uint8_t i;
    uint32_t meanBattVolt = 0;
    HAL_GPADC_MV_T vbat = volt;
    BATTERY_TRACE(2,"%s %dmv app_vbat_volt_div=%d",__func__, vbat, app_vbat_volt_div);
    if ((vbat == HAL_GPADC_BAD_VALUE) || ((vbat * app_vbat_volt_div) <= APP_BATTERY_ERR_MV))
    {
        app_battery_measure.cb(APP_BATTERY_STATUS_INVALID, vbat);
        return;
    }

#if (defined(BTUSB_AUDIO_MODE) || defined(BTUSB_AUDIO_MODE))
    if(app_usbaudio_mode_on()) return ;
#endif

    app_battery_measure.voltage[app_battery_measure.index++%APP_BATTERY_STABLE_COUNT] = vbat * app_vbat_volt_div;
    if (app_battery_measure.index > APP_BATTERY_STABLE_COUNT)
    {
        for (i=0; i<APP_BATTERY_STABLE_COUNT; i++)
        {
            meanBattVolt += app_battery_measure.voltage[i];
        }
        meanBattVolt /= APP_BATTERY_STABLE_COUNT;
        if (app_battery_measure.cb)
        {
            if (meanBattVolt>app_battery_measure.highvolt)
            {
                app_battery_measure.cb(APP_BATTERY_STATUS_OVERVOLT, meanBattVolt);
            }
            else if((meanBattVolt>app_battery_measure.pdvolt) && (meanBattVolt<app_battery_measure.lowvolt))
            {
                app_battery_measure.cb(APP_BATTERY_STATUS_UNDERVOLT, meanBattVolt);
            }
            else if(meanBattVolt<app_battery_measure.pdvolt)
            {
                app_battery_measure.cb(APP_BATTERY_STATUS_PDVOLT, meanBattVolt);
            }
            else
            {
                app_battery_measure.cb(APP_BATTERY_STATUS_NORMAL, meanBattVolt);
            }
        }
    }
    else
    {
        int8_t level = 0;
        meanBattVolt = vbat * app_vbat_volt_div;
        level = (meanBattVolt-APP_BATTERY_PD_MV)/APP_BATTERY_MV_BASE;

        if (level<APP_BATTERY_LEVEL_MIN)
            level = APP_BATTERY_LEVEL_MIN;
        if (level>APP_BATTERY_LEVEL_MAX)
            level = APP_BATTERY_LEVEL_MAX;

        app_battery_measure.currvolt = meanBattVolt;
#ifdef __INTERCONNECTION__
        APP_BATTERY_INFO_T* pBatteryInfo = (APP_BATTERY_INFO_T*)&app_battery_measure.currentBatteryInfo;
        pBatteryInfo->batteryLevel = level;
#else
        app_battery_measure.currlevel = level;
#endif
#if defined(BESUI_TWS_EN)
        app_battery_measure.currlevel = app_battery_level_tran_process(app_battery_measure.currvolt);
        BATTERY_TRACE(1, "[UIBAT]%s app_battery_measure.currlevel %d", __func__, app_battery_measure.currlevel);
#elif defined(BESUI_STEREO_EN)
        app_battery_measure.currlevel = stereo_battery_level_process(app_battery_measure.status, app_battery_measure.currvolt);
        BATTERY_TRACE(1, "[UIBAT]%s app_battery_measure.currlevel %d", __func__, app_battery_measure.currlevel);
#endif
    }
#if defined(BESUI_TWS_EN) || defined(BESUI_STEREO_EN)
    user_bat_volt_set(app_battery_measure.currvolt);
    if (app_battery_measure.index > APP_BATTERY_STABLE_COUNT)
        besui_system_info_trace();
#endif
}

#ifdef BESUI_TWS_EN
void app_battery_clear_index(void)
{
    app_battery_measure.index = 0;
}
#endif

#if defined(BESUI_TWS_EN) || defined(BESUI_STEREO_EN)
void besui_bat_timer_restart(void)
{
    osTimerStop(app_battery_timer);
    osTimerStart(app_battery_timer, 50);
}
#endif

static void app_battery_timer_start(enum APP_BATTERY_MEASURE_PERIODIC_T periodic)
{
    uint32_t periodic_millisec = 0;

    if (app_battery_measure.periodic != periodic){
        app_battery_measure.periodic = periodic;
        switch (periodic)
        {
            case APP_BATTERY_MEASURE_PERIODIC_FAST:
                periodic_millisec = APP_BATTERY_MEASURE_PERIODIC_FAST_MS;
                break;
            case APP_BATTERY_MEASURE_PERIODIC_CHARGING:
                periodic_millisec = APP_BATTERY_CHARGING_PERIODIC_MS;
                break;
            case APP_BATTERY_MEASURE_PERIODIC_NORMAL:
                periodic_millisec = APP_BATTERY_MEASURE_PERIODIC_NORMAL_MS;
                break;
            default:
                break;
        }
        osTimerStop(app_battery_timer);
        osTimerStart(app_battery_timer, periodic_millisec);
    }
}

static void app_battery_timer_handler(void const *param)
{
#ifdef CHARGER_1802
    charger_vbat_div_adc_enable(true);
    hal_gpadc_open(HAL_GPADC_CHAN_5, HAL_GPADC_ATP_ONESHOT, app_battery_irqhandler);
#else
    hal_gpadc_open(app_vbat_ch, HAL_GPADC_ATP_ONESHOT, app_battery_irqhandler);
#endif
}

static void app_battery_event_process(enum APP_BATTERY_STATUS_T status, APP_BATTERY_MV_T volt)
{
    uint32_t app_battevt;
    APP_MESSAGE_BLOCK msg;

    BATTERY_TRACE(3,"%s %d,Voltage : %dmV",__func__, status, volt);
    msg.mod_id = APP_MODULE_BATTERY;
#if defined(USE_BASIC_THREADS)
    msg.mod_level = APP_MOD_LEVEL_0;
#endif
    APP_BATTERY_SET_MESSAGE(app_battevt, status, volt);
    msg.msg_body.message_id = app_battevt;
    msg.msg_body.message_ptr = (uint32_t)NULL;
    app_mailbox_put(&msg);

}

#ifdef BT_BUILD_WITH_CUSTOMER_HOST
int app_status_battery_report(uint8_t level)
{
    return 0;
}
#else
int app_status_battery_report(uint8_t level)
{
    /* HFP battery level only supports 0~9 */
    uint8_t hfp_level = (level >= 9) ? 9 : level;

    BATTERY_TRACE(2,"%s level=%d hfp_level=%d",
                  __func__, level, hfp_level);

#if defined(APP_10_SECOND_TIMER_EN)
    app_10_second_timer_check();
#endif

    if (app_is_stack_ready())
    {
#if defined(SUPPORT_BATTERY_REPORT) || defined(SUPPORT_HF_INDICATORS)

#if defined(IBRT)
        uint8_t hfp_device = app_bt_audio_get_curr_hfp_device();
        struct BT_DEVICE_T *curr_device = app_bt_get_device(hfp_device);

        if (curr_device->hf_conn_flag)
        {
            BATTERY_TRACE(0,"[PHONE_CONNECTED] request case battery");
            //wired_uart_mobile_connected_get_box_battery();

            BATTERY_TRACE(2,
                          "[BATT] HFP connected raw=%d report=%d",
                          level,
                          hfp_level);

            app_hfp_set_battery_level(hfp_level);
        }
        else
        {
#if defined(BLE_BATT_ENABLE)

            /* BLE 使用百分比，可維持原始 level 換算 */
            uint8_t percent =
                (level >= 9) ? 100 : ((level + 1) * 10);

            DEBUG_WARNING(0,
                          "[BLE_BATT][STATUS] level=%d percent=%d",
                          level,
                          percent);

            for (int i = 0; i < BT_DEVICE_NUM; i++)
            {
                DEBUG_WARNING(0,
                              "[BLE_BATT][STATUS] dev=%d report=%d%%",
                              i,
                              percent);

                app_ble_report_battery_level(i, percent);
            }

#endif
        }

#elif defined(BT_HFP_SUPPORT)

        app_hfp_set_battery_level(hfp_level);

#endif

#else

        BATTERY_TRACE(1,
                      "[%s] Can not enable SUPPORT_BATTERY_REPORT",
                      __func__);

#endif

        bes_bt_osapi_notify_evm();
    }

    return 0;
}
#endif

bool app_battery_is_measurement_valid(void)
{
    return g_battery_measure_valid;
}

static bool g_battery_full_charged = false;

int app_battery_handle_process_normal(uint32_t status,  union APP_BATTERY_MSG_PRAMS prams)
{
    BATTERY_TRACE(0,"app_battery_handle_process_normal status = %d",status);
    int8_t level = 0;
    switch (status)
    {
        case APP_BATTERY_STATUS_UNDERVOLT:
        {
            BATTERY_TRACE(1, "UNDERVOLT:%d", prams.volt);

            app_status_indication_set(
                APP_STATUS_INDICATION_CHARGENEED);

            /*
            * Check whether Master should be switched
            * to the higher-battery peer.
            */
            //ntt_low_battery_role_switch_check();

        #ifdef MEDIA_PLAYER_SUPPORT

            /*
            * Battery check runs every 10 seconds.
            * Play immediately on first low-battery detection,
            * then repeat every 5 minutes.
            */
            if (g_low_bat_voice_first)
            {
                g_low_bat_voice_first = false;
                g_low_bat_voice_count = 0;

                BATTERY_TRACE(1,
                            "[LOW_BAT] first play, volt=%d",
                            prams.volt);

                media_PlayAudio(
                    AUD_ID_BT_BATTERY_LOW,
                    0);
            }
            else
            {
                g_low_bat_voice_count++;

                if (g_low_bat_voice_count >=
                    LOW_BAT_VOICE_INTERVAL_COUNT)
                {
                    g_low_bat_voice_count = 0;

                    BATTERY_TRACE(1,
                                "[LOW_BAT] 5min repeat play, volt=%d",
                                prams.volt);

                    media_PlayAudio(
                        AUD_ID_BT_BATTERY_LOW,
                        0);
                }
            }

        #endif
        }
    // FALLTHROUGH
            // FALLTHROUGH
        case APP_BATTERY_STATUS_NORMAL:
        case APP_BATTERY_STATUS_OVERVOLT:
        {
            /*
            * Reset low-battery voice state only when
            * battery is no longer under-voltage.
            */
            if (status != APP_BATTERY_STATUS_UNDERVOLT)
            {
                g_low_bat_voice_count = 0;
                g_low_bat_voice_first = true;
            }

            /*
            * prams.volt is an actual ADC measurement.
            * From this point battery voltage can be used for reporting.
            */
            app_battery_measure.currvolt = prams.volt;

            if (!g_battery_measure_valid)
            {
                g_battery_measure_valid = true;

                BATTERY_TRACE(2,
                            "[BAT_VALID] first ADC valid, volt=%d",
                            app_battery_measure.currvolt);
            }              

            level =
                (prams.volt - APP_BATTERY_PD_MV) /
                APP_BATTERY_MV_BASE;

            if (level < APP_BATTERY_LEVEL_MIN)
            {
                level = APP_BATTERY_LEVEL_MIN;
            }

            if (level > APP_BATTERY_LEVEL_MAX)
            {
                level = APP_BATTERY_LEVEL_MAX;
            }

            /*
            * App 與 TWS Peer 使用精細的 0～100%。
            */
            /*
            * App and TWS Peer use precise 0~100% battery level.
            */
            uint8_t app_percent = app_battery_get_precise_percent();

#if defined(IBRT)
        {
            static int16_t last_sync_percent = -1;

            /*
            * 0xFF means battery ADC is not valid yet.
            * Never synchronize the boot default battery value.
            */
            if ((app_percent != 0xFF) &&
                (app_percent <= 100) &&
                (last_sync_percent != app_percent))
            {
                last_sync_percent = app_percent;

                app_ibrt_customif_cmd_sync_battery_level(
                    app_percent);

                BATTERY_TRACE(1,
                            "[BAT_SYNC] percent=%d",
                            app_percent);

                /*
                 * The phone's BLE session belongs to the TWS Master.
                 * On a local percentage change, the Master can immediately
                 * update App battery data through proactive 0x31 Notify.
                 * A Slave only synchronizes its value above; the Master's
                 * TWS receive handler calls
                 * app_battery_notify_peer_percent_changed().
                 */
                if (app_ibrt_if_get_ui_role() == TWS_UI_MASTER)
                {
                    BATTERY_TRACE(1,
                                  "[BAT31] local battery changed=%d -> push",
                                  app_percent);

                    sparrow_push_battery_level_notify(false);
                }
            }
            else if (app_percent == 0xFF)
            {
                BATTERY_TRACE(0,
                            "[BAT_SYNC] skip - battery not valid");
            }

            /*
            * Battery role-switch policy:
            *
            * <= 75% : Start checking whether Peer should become Master.
            * <= 50% : Continue checking with the same conditions.
            * Low battery is naturally included in this range.
            *
            * Actual role switch still requires:
            * 1. TWS connected
            * 2. Local ear is Master
            * 3. Peer battery is valid
            * 4. Peer battery level is at least one level higher
            */
            if ((app_percent != 0xFF) && (app_percent <= 75))
            {
                BATTERY_TRACE(2,
                            "[BAT_ROLE_POLICY] local=%d%% -> check role switch",
                            app_percent);

                ntt_low_battery_role_switch_check();
            }
        }
#endif

        #ifdef __INTERCONNECTION__

            APP_BATTERY_INFO_T *pBatteryInfo;

            pBatteryInfo =
                (APP_BATTERY_INFO_T *)
                &app_battery_measure.currentBatteryInfo;

            pBatteryInfo->batteryLevel = level;

            if (level == APP_BATTERY_LEVEL_MAX)
            {
                level = 9;
            }
            else
            {
                level /= 10;
            }

        #else

        #ifdef BESUI_TWS_EN
            level = app_battery_level_tran_process(app_battery_measure.currvolt);
            app_battery_measure.currlevel = level;
            app_battery_low_voice_enable_set(app_battery_measure.currlevel);
            level = app_battery_level_compare();
            app_battery_low_voice_play_process();
        #ifdef BATTERY_SWITCH_ROLE_EN
            BATTERY_TRACE(0, "[BAT_ROLE] call besui_battery_role_switch");
            besui_battery_role_switch();
        #endif

        #else

        #if defined(BESUI_STEREO_EN)
            level = stereo_battery_level_process(app_battery_measure.status,app_battery_measure.currvolt);
        #endif

            app_battery_measure.currlevel = level;

        #endif
        #endif

            /*
            * 手機 HFP 使用 0～9 level。
            */
            level = aiWangReportNormalLevelHandler(app_battery_measure.currvolt);
            app_status_battery_report(level);

            break;
        }

        case APP_BATTERY_STATUS_PDVOLT:
#ifndef BT_USB_AUDIO_DUAL_MODE
            BATTERY_TRACE(1,"PDVOLT-->POWEROFF:%d", prams.volt);
            osTimerStop(app_battery_timer);
#if defined(BESUI_TWS_EN)
			app_ui_shutdown();
#else
            app_shutdown();
#endif
#endif
            break;
        case APP_BATTERY_STATUS_CHARGING:
            /*
            * Debug actual charger plug-in / plug-out event.
            */
            BATTERY_TRACE(4,
                        "[BAT_CHG] evt=%d charger=%d volt=%d level=%d",
                        status,
                        prams.charger,
                        app_battery_measure.currvolt,
                        app_battery_measure.currlevel);

            BATTERY_TRACE(1,"CHARGING-->APP_BATTERY_CHARGER :%d", prams.charger);
            BATTERY_TRACE(1,
                        "CHARGING-->APP_BATTERY_CHARGER :%d full=%d",
                        prams.charger,
                        g_battery_full_charged);

            if ((ntt_case_state_get_local() == NTT_CASE_STATE_IN_CASE) &&
                ((prams.charger == 1) || g_battery_full_charged))
            {
                BATTERY_TRACE(2,
                            "[POWER_OFF] IN_CASE charger=%d full=%d -> start timer",
                            prams.charger,
                            g_battery_full_charged);

                if (!ntt_charging_pwron_pending_shutdown)
                {
                    ntt_charging_pwron_pending_shutdown = true;
                    earBudsCloseOff_PogonIn_StartTimer();
                }
            }
            else if ((prams.charger == 0) && !g_battery_full_charged)
            {
                BATTERY_TRACE(0,
                            "[POWER_OFF] charger=0 and not full -> cancel shutdown timer");

                /*
                * Clear pending first so a callback already queued by the
                * timer cannot continue the old shutdown request.
                */
                ntt_charging_pwron_pending_shutdown = false;
                earBudsCloseOff_PogonIn_StopTimer();

            #ifdef IBRT
                g_pogonin_role_switch_requested = false;
                g_pogonin_role_switch_wait_count = 0;
                ntt_case_close_role_switch_reset();
            #endif
            }
            if (prams.charger == APP_BATTERY_CHARGER_PLUGIN)
            {
#ifdef BT_USB_AUDIO_DUAL_MODE
                BATTERY_TRACE(1,"%s:PLUGIN.", __func__);
                btusb_switch(BTUSB_MODE_USB);
#else

#if CHARGER_PLUGINOUT_RESET
                //app_reset();
#else
                app_battery_measure.status = APP_BATTERY_STATUS_CHARGING;
#endif
#endif
            }
            break;
        case APP_BATTERY_STATUS_INVALID:
        default:
            break;
    }

    app_battery_timer_start(APP_BATTERY_MEASURE_PERIODIC_NORMAL);
    return 0;
}

int app_battery_handle_process_charging(uint32_t status,  union APP_BATTERY_MSG_PRAMS prams)
{
    uint8_t level = 0;
#if defined(BESUI_TWS_EN)
#error BESUI_TWS_EN
    BATTERY_TRACE(1,"[UIBAT] status = %d, volt %d",status, prams.volt);
    app_battery_level_tran_process(prams.volt);
#elif defined(BESUI_STEREO_EN)
#error BESUI_STEREO_EN
    level = stereo_battery_level_process(status, prams.volt);
    BATTERY_TRACE(1,"[UIBAT] status=%d, volt=%d, level=%d",status, prams.volt, level);
#endif
    switch (status)
    {
        case APP_BATTERY_STATUS_OVERVOLT:
        case APP_BATTERY_STATUS_NORMAL:
        case APP_BATTERY_STATUS_UNDERVOLT:
            app_battery_measure.currvolt = prams.volt;
            level = aiWangReportNormalLevelHandler(app_battery_measure.currvolt);
            BATTERY_TRACE(1,"[UIBAT] status=%d, volt=%d, level=%d",status, prams.volt, level);
            app_status_battery_report(level/*prams.volt*/);
            break;
        case APP_BATTERY_STATUS_CHARGING:
            BATTERY_TRACE(1,"[UIBAT] CHARGING:%d", prams.charger);
            if (prams.charger == APP_BATTERY_CHARGER_PLUGOUT)
            {
#ifndef BT_USB_AUDIO_DUAL_MODE
#if CHARGER_PLUGINOUT_RESET
                BATTERY_TRACE(0,"CHARGING-->RESET");
                osTimerStop(app_battery_timer);
                //app_shutdown();
                app_reset();
#else
                app_battery_measure.status = APP_BATTERY_STATUS_NORMAL;
#endif
#endif
            }
            else if (prams.charger == APP_BATTERY_CHARGER_PLUGIN)
            {
#if defined(BT_USB_AUDIO_DUAL_MODE)
                BATTERY_TRACE(1,"%s:PLUGIN.", __func__);
                btusb_switch(BTUSB_MODE_USB);
#endif
                BATTERY_TRACE(1,"%s:PLUGIN.", __func__);
                osTimerStop(app_battery_timer);
            }
            break;
        case APP_BATTERY_STATUS_INVALID:
        default:
        	BATTERY_TRACE(0,"[UIBAT] APP_BATTERY_STATUS_INVALID");
            break;
    }

    if (app_battery_charger_handle_process()<=0)
    {
        if (app_status_indication_get() != APP_STATUS_INDICATION_FULLCHARGE)
        {
            BATTERY_TRACE(1,"FULL_CHARGING:%d", app_battery_measure.currvolt);
#ifdef BESUI_TWS_EN
            besui_bat_full_sta_set(true);
#endif
#if (!defined(BESUI_TWS_EN) && !defined(BESUI_STEREO_EN))
            app_status_indication_set(APP_STATUS_INDICATION_FULLCHARGE);
#ifdef MEDIA_PLAYER_SUPPORT
#if defined(BT_USB_AUDIO_DUAL_MODE) || defined(IBRT)
#else
            media_PlayAudio(AUD_ID_BT_CHARGE_FINISH, 0);
#endif
#endif
#endif
        }
    }
    app_battery_timer_start(APP_BATTERY_MEASURE_PERIODIC_CHARGING);
    return 0;
}

extern "C"     bool pmu_ana_volt_is_high(void);
static int app_battery_handle_process(APP_MESSAGE_BODY *msg_body)
{
    uint8_t status;
    union APP_BATTERY_MSG_PRAMS msg_prams;

    APP_BATTERY_GET_STATUS(msg_body->message_id, status);
    APP_BATTERY_GET_PRAMS(msg_body->message_id, msg_prams.prams);

    uint32_t generatedSeed = hal_sys_timer_get();
    for (uint8_t index = 0; index < sizeof(bt_global_addr); index++)
    {
        generatedSeed ^= (((uint32_t)(bt_global_addr[index])) << (hal_sys_timer_get() & 0xF));
    }
    srand(generatedSeed);


    if (status == APP_BATTERY_STATUS_PLUGINOUT)
    {
        app_battery_pluginout_debounce_start();
    }
    else
    {
        switch (app_battery_measure.status)
        {
            case APP_BATTERY_STATUS_NORMAL:
                app_battery_handle_process_normal((uint32_t)status, msg_prams);

#if defined(CHIP_BEST1501P)
                if (pmu_ana_volt_is_high() == false)
                {
                    pmu_ntc_capture_start(NULL);
                }
                else
                {
                    BATTERY_TRACE(1, "%s stop ana check", __func__);
                }
#endif
                break;

            case APP_BATTERY_STATUS_CHARGING:
                app_battery_handle_process_charging((uint32_t)status, msg_prams);
                break;

            default:
                break;
        }

        if (app_battery_measure.currlevel <= 100)
        {
            app_ibrt_customif_cmd_sync_battery_level(app_battery_measure.currlevel);
        }
    }

    if (NULL != app_battery_measure.user_cb)
    {
        uint8_t batteryLevel;

#ifdef __INTERCONNECTION__
        APP_BATTERY_INFO_T *pBatteryInfo;
        pBatteryInfo = (APP_BATTERY_INFO_T *)&app_battery_measure.currentBatteryInfo;
        pBatteryInfo->chargingStatus =
            ((app_battery_measure.status == APP_BATTERY_STATUS_CHARGING) ? 1 : 0);
        batteryLevel = pBatteryInfo->batteryLevel;
#else
        batteryLevel = app_battery_measure.currlevel;
#endif

        app_battery_measure.user_cb(app_battery_measure.currvolt,
                                    batteryLevel,
                                    app_battery_measure.status,
                                    status,
                                    msg_prams);
    }

    return 0;
}

int app_battery_register(APP_BATTERY_CB_T user_cb)
{
    if(NULL == app_battery_measure.user_cb)
    {
        app_battery_measure.user_cb = user_cb;
        return 0;
    }
    return 1;
}

int app_battery_get_info(APP_BATTERY_MV_T *currvolt, uint8_t *currlevel, enum APP_BATTERY_STATUS_T *status)
{
    if (currvolt)
    {
        *currvolt = app_battery_measure.currvolt;
    }

    if (currlevel)
    {
#ifdef __INTERCONNECTION__
        *currlevel = app_battery_measure.currentBatteryInfo;
#else
        *currlevel = app_battery_measure.currlevel;
#endif
    }

    if (status)
    {
        *status = app_battery_measure.status;
    }

    return 0;
}

#ifdef MORE_THAN_ONE_TYPE_OF_CHARGER
static void app_battery_gpadc_configuration_init(void)
{
    if (charger_package_type_get() == CHARGER_PACKAGE_TYPE_SIP_1620)
    {
        app_vbat_ch = HAL_GPADC_CHAN_BATTERY;
        app_vbat_volt_div = 4;
    }
}
#endif

int app_battery_open(void)
{
    BATTERY_TRACE(3,"%s batt range:%d~%d",__func__, APP_BATTERY_MIN_MV, APP_BATTERY_MAX_MV);

    int nRet = APP_BATTERY_OPEN_MODE_INVALID;
        /*
     * Battery ADC data is not valid yet.
     */
    g_battery_measure_valid = false;
    g_battery_last_valid_percent = 0xFF;

#ifdef MORE_THAN_ONE_TYPE_OF_CHARGER
    app_battery_gpadc_configuration_init();
#endif

    BATTERY_TRACE(0, "%s: app_vbat_ch=%d app_vbat_volt_div=%d", __func__, app_vbat_ch, app_vbat_volt_div);

    if (app_battery_timer == NULL)
        app_battery_timer = osTimerCreate (osTimer(APP_BATTERY), osTimerPeriodic, NULL);

    if (app_battery_pluginout_debounce_timer == NULL)
        app_battery_pluginout_debounce_timer = osTimerCreate (osTimer(APP_BATTERY_PLUGINOUT_DEBOUNCE), osTimerOnce, &app_battery_pluginout_debounce_ctx);

    app_battery_measure.status = APP_BATTERY_STATUS_NORMAL;
#ifdef __INTERCONNECTION__
    app_battery_measure.currentBatteryInfo = APP_BATTERY_DEFAULT_INFO;
    app_battery_measure.lastBatteryInfo = APP_BATTERY_DEFAULT_INFO;
    app_battery_measure.isMobileSupportSelfDefinedCommand = 0;
#else
    app_battery_measure.currlevel = APP_BATTERY_LEVEL_MAX;
#endif
    app_battery_measure.currvolt = APP_BATTERY_MAX_MV;
    app_battery_measure.lowvolt = APP_BATTERY_MIN_MV;
    app_battery_measure.highvolt = APP_BATTERY_MAX_MV;
    app_battery_measure.pdvolt = APP_BATTERY_PD_MV;
    app_battery_measure.chargetimeout = APP_BATTERY_CHARGE_TIMEOUT_MIN;

    app_battery_measure.periodic = APP_BATTERY_MEASURE_PERIODIC_QTY;
    app_battery_measure.cb = app_battery_event_process;
    app_battery_measure.user_cb = NULL;

    app_battery_measure.charger_status.prevolt = 0;
    app_battery_measure.charger_status.slope_1000_index = 0;
    app_battery_measure.charger_status.cnt = 0;

    /*
     * Debug:
     * Check the initialized battery values before charger detection.
     */
    BATTERY_TRACE(3,
                  "[BAT_BOOT] status=%d volt=%d level=%d",
                  app_battery_measure.status,
                  app_battery_measure.currvolt,
                  app_battery_measure.currlevel);

    app_set_threadhandle(APP_MODULE_BATTERY, app_battery_handle_process);

    if (app_battery_ext_charger_detecter_cfg.pin != HAL_IOMUX_PIN_NUM)
    {
        hal_iomux_init((struct HAL_IOMUX_PIN_FUNCTION_MAP *)&app_battery_ext_charger_detecter_cfg, 1);
        hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)app_battery_ext_charger_detecter_cfg.pin, HAL_GPIO_DIR_IN, 1);
    }

#ifndef BESUI_TWS_EN
    if (app_battery_ext_charger_enable_cfg.pin != HAL_IOMUX_PIN_NUM)
    {
        hal_iomux_init((struct HAL_IOMUX_PIN_FUNCTION_MAP *)&app_battery_ext_charger_detecter_cfg, 1);
        hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)app_battery_ext_charger_detecter_cfg.pin, HAL_GPIO_DIR_OUT, 1);
    }
#endif

    if (app_battery_charger_indication_open() == APP_BATTERY_CHARGER_PLUGIN)
    {
        app_battery_measure.status = APP_BATTERY_STATUS_CHARGING;
        app_battery_measure.start_time = hal_sys_timer_get();
        //pmu_charger_plugin_config();
#ifndef BESUI_TWS_EN
        if (app_battery_ext_charger_enable_cfg.pin != HAL_IOMUX_PIN_NUM)
        {
            hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)app_battery_ext_charger_detecter_cfg.pin, HAL_GPIO_DIR_OUT, 0);
        }
#endif

        /*
         * Debug:
         * Check whether battery voltage/level still contains
         * the initialized MAX values after charger detection.
         */
        BATTERY_TRACE(3,
                      "[BAT_BOOT_CHG] status=%d volt=%d level=%d",
                      app_battery_measure.status,
                      app_battery_measure.currvolt,
                      app_battery_measure.currlevel);

#if (CHARGER_PLUGINOUT_RESET == 0)
        nRet = APP_BATTERY_OPEN_MODE_CHARGING_PWRON;
#else
        nRet = APP_BATTERY_OPEN_MODE_CHARGING;
#endif
    }
    else
    {
        app_battery_measure.status = APP_BATTERY_STATUS_NORMAL;
        //pmu_charger_plugout_config();
        nRet = APP_BATTERY_OPEN_MODE_NORMAL;
        /*
         * Debug normal power-on state.
         */
        BATTERY_TRACE(3,
                      "[BAT_BOOT_NORMAL] status=%d volt=%d level=%d",
                      app_battery_measure.status,
                      app_battery_measure.currvolt,
                      app_battery_measure.currlevel);
    }
    return nRet;
}

int app_battery_start(void)
{
    BATTERY_TRACE(2,"%s %d",__func__, APP_BATTERY_MEASURE_PERIODIC_FAST_MS);

    app_battery_timer_start(APP_BATTERY_MEASURE_PERIODIC_FAST);

    return 0;
}

int app_battery_stop(void)
{
    osTimerStop(app_battery_timer);

    return 0;
}

int app_battery_close(void)
{
    hal_gpadc_close(HAL_GPADC_CHAN_BATTERY);

    return 0;
}


static int32_t app_battery_charger_slope_calc(int32_t t1, int32_t v1, int32_t t2, int32_t v2)
{
    int32_t slope_1000;
    slope_1000 = (v2-v1)*1000/(t2-t1);
    return slope_1000;
}

static int app_battery_charger_handle_process(void)
{
    int nRet = 1;
    int8_t i=0,cnt=0;
    uint32_t slope_1000 = 0;
    uint32_t charging_min;
    static uint8_t overvolt_full_charge_cnt = 0;
    static uint8_t ext_pin_full_charge_cnt = 0;

    charging_min = hal_sys_timer_get() - app_battery_measure.start_time;
    charging_min = TICKS_TO_MS(charging_min)/1000/60;
    if (charging_min >= app_battery_measure.chargetimeout)
    {
        // BATTERY_TRACE(0,"TIMEROUT-->FULL_CHARGING");
        nRet = -1;
        goto exit;
    }

    if ((app_battery_measure.charger_status.cnt++%APP_BATTERY_CHARGING_OVERVOLT_MEASURE_CNT) == 0)
    {
        if (app_battery_measure.currvolt>=(app_battery_measure.highvolt+APP_BATTERY_CHARGE_OFFSET_MV))
        {
            overvolt_full_charge_cnt++;
        }
        else
        {
            overvolt_full_charge_cnt = 0;
        }
        if (overvolt_full_charge_cnt>=APP_BATTERY_CHARGING_OVERVOLT_DEDOUNCE_CNT)
        {
            //BATTERY_TRACE(0,"OVERVOLT-->FULL_CHARGING");
            nRet = -1;
            goto exit;
        }
    }

    if ((app_battery_measure.charger_status.cnt++%APP_BATTERY_CHARGING_EXTPIN_MEASURE_CNT) == 0)
    {
        if (app_battery_ext_charger_detecter_cfg.pin != HAL_IOMUX_PIN_NUM)
        {
            if (hal_gpio_pin_get_val((enum HAL_GPIO_PIN_T)app_battery_ext_charger_detecter_cfg.pin))
            {
                ext_pin_full_charge_cnt++;
            }
            else
            {
                ext_pin_full_charge_cnt = 0;
            }
            if (ext_pin_full_charge_cnt>=APP_BATTERY_CHARGING_EXTPIN_DEDOUNCE_CNT)
            {
                BATTERY_TRACE(0,"EXT PIN-->FULL_CHARGING");
                nRet = -1;
                goto exit;
            }
        }
    }

    if ((app_battery_measure.charger_status.cnt++%APP_BATTERY_CHARGING_SLOPE_MEASURE_CNT) == 0)
    {
        if (!app_battery_measure.charger_status.prevolt)
        {
            app_battery_measure.charger_status.slope_1000[app_battery_measure.charger_status.slope_1000_index%APP_BATTERY_CHARGING_SLOPE_TABLE_COUNT] = slope_1000;
            app_battery_measure.charger_status.prevolt = app_battery_measure.currvolt;
            for (i=0; i<APP_BATTERY_CHARGING_SLOPE_TABLE_COUNT; i++)
            {
                app_battery_measure.charger_status.slope_1000[i]=100;
            }
        }
        else
        {
            slope_1000 = app_battery_charger_slope_calc(0, app_battery_measure.charger_status.prevolt,
                         APP_BATTERY_CHARGING_PERIODIC_MS*APP_BATTERY_CHARGING_SLOPE_MEASURE_CNT/1000, app_battery_measure.currvolt);
            app_battery_measure.charger_status.slope_1000[app_battery_measure.charger_status.slope_1000_index%APP_BATTERY_CHARGING_SLOPE_TABLE_COUNT] = slope_1000;
            app_battery_measure.charger_status.prevolt = app_battery_measure.currvolt;
            for (i=0; i<APP_BATTERY_CHARGING_SLOPE_TABLE_COUNT; i++)
            {
                if (app_battery_measure.charger_status.slope_1000[i]>0)
                    cnt++;
                else
                    cnt--;
                BATTERY_TRACE(3,"slope_1000[%d]=%d cnt:%d", i,app_battery_measure.charger_status.slope_1000[i], cnt);
            }
            BATTERY_TRACE(3,"app_battery_charger_slope_proc slope*1000=%d cnt:%d nRet:%d", slope_1000, cnt, nRet);
            if (cnt>1)
            {
                nRet = 1;
            }/*else (3>=cnt && cnt>=-3){
                nRet = 0;
            }*/else
            {
                if (app_battery_measure.currvolt>=(app_battery_measure.highvolt-APP_BATTERY_CHARGE_OFFSET_MV))
                {
                    BATTERY_TRACE(0,"SLOPE-->FULL_CHARGING");
                    nRet = -1;
                }
            }
        }
        app_battery_measure.charger_status.slope_1000_index++;
    }
exit:
    g_battery_full_charged = (nRet == -1);
    return nRet;
}

static enum APP_BATTERY_CHARGER_T app_battery_charger_forcegetstatus(void)
{
    enum APP_BATTERY_CHARGER_T status = APP_BATTERY_CHARGER_QTY;
    enum PMU_CHARGER_STATUS_T charger;

    charger = pmu_charger_get_status();

    if (charger == PMU_CHARGER_PLUGIN)
    {
        status = APP_BATTERY_CHARGER_PLUGIN;
        // BATTERY_TRACE(0,"force APP_BATTERY_CHARGER_PLUGIN");
    }
    else
    {
        status = APP_BATTERY_CHARGER_PLUGOUT;
        // BATTERY_TRACE(0,"force APP_BATTERY_CHARGER_PLUGOUT");
    }

    return status;
}

static void app_battery_charger_handler(enum PMU_CHARGER_STATUS_T status)
{
    BATTERY_TRACE(2,"%s: status=%d", __func__, status);
    pmu_charger_set_irq_handler(NULL);
    app_battery_event_process(APP_BATTERY_STATUS_PLUGINOUT,
                              (status == PMU_CHARGER_PLUGIN) ? APP_BATTERY_CHARGER_PLUGIN : APP_BATTERY_CHARGER_PLUGOUT);
#ifdef BESUI_1WIRE_EN
    communication_uart_function_enable(true);
#endif
}

static void app_battery_pluginout_debounce_start(void)
{
    BATTERY_TRACE(1,"%s", __func__);
#if defined(BT_USB_AUDIO_DUAL_MODE)
    btusb_switch(BTUSB_MODE_BT);
#endif
    app_battery_pluginout_debounce_ctx = (uint32_t)app_battery_charger_forcegetstatus();
    app_battery_pluginout_debounce_cnt = 1;
    osTimerStart(app_battery_pluginout_debounce_timer, CHARGER_PLUGINOUT_DEBOUNCE_MS);
}

static void app_battery_pluginout_debounce_handler(void const *param)
{
    enum APP_BATTERY_CHARGER_T status_charger = app_battery_charger_forcegetstatus();

#ifdef BESUI_TWS_EN
    bool factory_flag = false;
#endif

    (void)param;

    if (app_battery_pluginout_debounce_ctx == (uint32_t)status_charger)
    {
        app_battery_pluginout_debounce_cnt++;
    }
    else
    {
        BATTERY_TRACE(2,"%s dithering cnt %u",__func__,app_battery_pluginout_debounce_cnt);

        /*
         * 新狀態這次已經讀到一次，
         * 所以建議從 1 開始。
         */
        app_battery_pluginout_debounce_cnt = 1;
        app_battery_pluginout_debounce_ctx = (uint32_t)status_charger;
    }

#ifdef BESUI_TWS_EN
    BATTERY_TRACE(2,"[UIBAT]%s cnt=%u",__func__,app_battery_pluginout_debounce_cnt);

#ifdef BESUI_1WIRE_EN
    if (status_charger == APP_BATTERY_CHARGER_PLUGOUT)
    {
        communication_uart_function_enable(true);
    }
#endif
#endif

    BATTERY_TRACE(3,
                "[CHG_DEBOUNCE] sample=%u cnt=%u ctx=%u",
                status_charger,
                app_battery_pluginout_debounce_cnt,
                app_battery_pluginout_debounce_ctx);

    BATTERY_TRACE(0,"[CHG_DEBOUNCE] app_battery_pluginout_debounce_cnt =%d",app_battery_pluginout_debounce_cnt);

    if (app_battery_pluginout_debounce_cnt >= CHARGER_PLUGINOUT_DEBOUNCE_CNT)
    {
        BATTERY_TRACE(2,"%s %s",__func__,status_charger == APP_BATTERY_CHARGER_PLUGOUT ?"PLUGOUT" :"PLUGIN");
        /*
         * ==================================================
         * NTT 左右耳 Case State 同步入口
         * 必須放在 debounce 穩定確認之後。
         * ==================================================
         */
        BATTERY_TRACE(2,"[NTT_CASE_BRIDGE] current=%d last=%d",status_charger,g_ntt_last_case_state);
        if (status_charger != g_ntt_last_case_state)
        {
            BATTERY_TRACE(2,"[NTT_CASE_BRIDGE] state changed %d -> %d",g_ntt_last_case_state,status_charger);
            g_ntt_last_case_state = status_charger;
            if (status_charger == APP_BATTERY_CHARGER_PLUGIN)
            {
                BATTERY_TRACE(0,"[NTT_CASE_HW] confirmed PLUGIN -> IN_CASE");
                ntt_case_state_sync_local_update(true);
                //app_key_handle_pause_music_on_pogo_in();
            }
            else if (status_charger == APP_BATTERY_CHARGER_PLUGOUT)
            {
                BATTERY_TRACE(0,"[NTT_CASE_HW] PMU PLUGOUT ignored; wait UART idle");
                //BATTERY_TRACE(0,"[NTT_CASE_HW] confirmed PLUGOUT -> OUT_CASE");
                //ntt_case_state_sync_local_update(false);
            }
        }
        else
        {
            BATTERY_TRACE(2,"[NTT_CASE_BRIDGE] duplicate ignored state=%d",status_charger);
        }

        /*
         * ==================================================
         * 原本 PLUGIN / PLUGOUT 處理流程
         * ==================================================
         */
        if (status_charger == APP_BATTERY_CHARGER_PLUGIN)
        {
            /*
             * Pogo pin confirmed inserted.
             * Pause music if A2DP is streaming.
             */
            //app_key_handle_pause_music_on_pogo_in();

#ifndef BESUI_TWS_EN
            if (app_battery_ext_charger_enable_cfg.pin != HAL_IOMUX_PIN_NUM)
            {
                hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)app_battery_ext_charger_enable_cfg.pin,HAL_GPIO_DIR_OUT,0);
            }
#endif

#ifdef BESUI_TWS_EN
            factory_flag = app_charge_fast_exit_factory_mode();

            if (factory_flag)
            {
                pmu_charger_set_irq_handler(app_battery_charger_handler);
                osTimerStop(app_battery_pluginout_debounce_timer);
                return;
            }
#endif

#ifdef BESUI_CHARGE_EN
            if (!besui_bat_charge_sta_get())
            {
                BATTERY_TRACE(0,"close box charging");
                besui_bat_charge_sta_set(true);
                if (uicom.poweron_bat_det_flag)
                {
                    app_charge_putinout_ui_post_msg(CHARGE_PLUGIN);
                }
            }
#endif

            app_battery_measure.start_time = hal_sys_timer_get();

#ifdef BESUI_STEREO_EN
            if ((!app_ui_charging_io_read((enum HAL_GPIO_PIN_T)HAL_IOMUX_PIN_P1_7)) && app_get_charging_status())
            {
                BATTERY_TRACE(0,"close box charging -2");
                app_shutdown();
            }
#endif
        }
        else
        {
#ifdef BESUI_TWS_EN
            besui_bat_full_sta_set(false);
#else
            if (app_battery_ext_charger_enable_cfg.pin != HAL_IOMUX_PIN_NUM)
            {
                hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)app_battery_ext_charger_enable_cfg.pin,HAL_GPIO_DIR_OUT,1);
            }
#endif

#ifdef BESUI_CHARGE_EN
#ifdef BESUI_NTC_EN
            if (!ntc_charger_status())
#endif
            {
                if (besui_bat_charge_sta_get())
                {
                    BATTERY_TRACE(0,"charging open box");
                    besui_bat_charge_sta_set(false);
                    if (uicom.poweron_bat_det_flag)
                    {
                        app_charge_putinout_ui_post_msg(CHARGE_PLUGOUT);
                    }
                }
            }
#endif
        }

        app_battery_event_process(APP_BATTERY_STATUS_CHARGING,status_charger);
        pmu_charger_set_irq_handler(app_battery_charger_handler);
        osTimerStop(app_battery_pluginout_debounce_timer);
    }
    else
    {
        osTimerStart(app_battery_pluginout_debounce_timer,CHARGER_PLUGINOUT_DEBOUNCE_MS);
    }
}

int app_battery_charger_indication_open(void)
{
    enum APP_BATTERY_CHARGER_T status = APP_BATTERY_CHARGER_QTY;
    uint8_t cnt = 0;

    BATTERY_TRACE(1,"%s",__func__);

    pmu_charger_init();

    do
    {
        status = app_battery_charger_forcegetstatus();
        if (status == APP_BATTERY_CHARGER_PLUGIN)
            break;
        osDelay(20);
    }
    while(cnt++<5);

#ifndef BESUI_TWS_EN
    if (app_battery_ext_charger_detecter_cfg.pin != HAL_IOMUX_PIN_NUM)
    {
        if (!hal_gpio_pin_get_val((enum HAL_GPIO_PIN_T)app_battery_ext_charger_detecter_cfg.pin))
        {
            status = APP_BATTERY_CHARGER_PLUGIN;
        }
    }
#endif

    pmu_charger_set_irq_handler(app_battery_charger_handler);

    return status;
}

int8_t app_battery_current_level(void)
{
#ifdef __INTERCONNECTION__
    return app_battery_measure.currentBatteryInfo & 0x7f;
#else
    return app_battery_measure.currlevel;
#endif
}

int8_t app_battery_is_charging(void)
{
    return (APP_BATTERY_STATUS_CHARGING == app_battery_measure.status);
}
typedef uint16_t NTP_VOLTAGE_MV_T;
typedef uint16_t NTP_TEMPERATURE_C_T;

#define NTC_CAPTURE_STABLE_COUNT (5)
#define NTC_CAPTURE_TEMPERATURE_STEP (4)
#define NTC_CAPTURE_TEMPERATURE_REF (15)
#define NTC_CAPTURE_VOLTAGE_REF (1100)

typedef void (*NTC_CAPTURE_MEASURE_CB_T)(NTP_TEMPERATURE_C_T);

struct NTC_CAPTURE_MEASURE_T
{
    NTP_TEMPERATURE_C_T temperature;
    NTP_VOLTAGE_MV_T currvolt;
    NTP_VOLTAGE_MV_T voltage[NTC_CAPTURE_STABLE_COUNT];
    uint16_t index;
    NTC_CAPTURE_MEASURE_CB_T cb;
};

static struct NTC_CAPTURE_MEASURE_T ntc_capture_measure;

void ntc_capture_irqhandler(uint16_t irq_val, HAL_GPADC_MV_T volt)
{
    uint32_t meanVolt = 0;
    //BATTERY_TRACE(3,"%s %d irq:0x%04x",__func__, volt, irq_val);

    if (volt == HAL_GPADC_BAD_VALUE)
    {
        return;
    }

    ntc_capture_measure.voltage[ntc_capture_measure.index++%NTC_CAPTURE_STABLE_COUNT] = volt;

    if (ntc_capture_measure.index > NTC_CAPTURE_STABLE_COUNT)
    {
        for (uint8_t i=0; i<NTC_CAPTURE_STABLE_COUNT; i++)
        {
            meanVolt += ntc_capture_measure.voltage[i];
        }
        meanVolt /= NTC_CAPTURE_STABLE_COUNT;
        ntc_capture_measure.currvolt = meanVolt;
    }
    else if (!ntc_capture_measure.currvolt)
    {
        ntc_capture_measure.currvolt = volt;
    }
    ntc_capture_measure.temperature = ((int32_t)ntc_capture_measure.currvolt - NTC_CAPTURE_VOLTAGE_REF)/NTC_CAPTURE_TEMPERATURE_STEP + NTC_CAPTURE_TEMPERATURE_REF;
    pmu_ntc_capture_disable();
    //BATTERY_TRACE(3,"%s ad:%d temperature:%d",__func__, ntc_capture_measure.currvolt, ntc_capture_measure.temperature);
#ifdef BESUI_NTC_EN
    app_ntc_detect_process(volt);
#endif
    aw_ntc_detect_process(volt);
}


int ntc_capture_open(void)
{

    ntc_capture_measure.currvolt = 0;
    ntc_capture_measure.index = 0;
    ntc_capture_measure.temperature = 0;
    ntc_capture_measure.cb = NULL;

    pmu_ntc_capture_enable();
    hal_gpadc_open(HAL_GPADC_CHAN_0, HAL_GPADC_ATP_ONESHOT, ntc_capture_irqhandler);
    return 0;
}

int ntc_capture_start(void)
{
    pmu_ntc_capture_enable();
    hal_gpadc_open(HAL_GPADC_CHAN_0, HAL_GPADC_ATP_ONESHOT, ntc_capture_irqhandler);
    return 0;
}
#else

#define IS_USE_SOC_PMU_PLUGINOUT

#ifdef IS_USE_SOC_PMU_PLUGINOUT

#ifndef CHARGER_PLUGINOUT_DEBOUNCE_MS
#define CHARGER_PLUGINOUT_DEBOUNCE_MS (50)
#endif

#ifndef CHARGER_PLUGINOUT_DEBOUNCE_CNT
#define CHARGER_PLUGINOUT_DEBOUNCE_CNT (3)
#endif

static void app_battery_pluginout_debounce_start(void);
static void app_battery_pluginout_debounce_handler(void const *param);
osTimerDef (APP_BATTERY_PLUGINOUT_DEBOUNCE, app_battery_pluginout_debounce_handler);
static osTimerId app_battery_pluginout_debounce_timer = NULL;
static uint32_t app_battery_pluginout_debounce_ctx = 0;
static uint32_t app_battery_pluginout_debounce_cnt = 0;

static int app_battery_handle_process(APP_MESSAGE_BODY *msg_body)
{
    uint8_t status;

    APP_BATTERY_GET_STATUS(msg_body->message_id, status);

    if (status == APP_BATTERY_STATUS_PLUGINOUT){
        app_battery_pluginout_debounce_start();
    }

    return 0;
}

static void app_battery_event_process(enum APP_BATTERY_STATUS_T status, APP_BATTERY_MV_T volt)
{
    uint32_t app_battevt;
    APP_MESSAGE_BLOCK msg;

    BATTERY_TRACE(3,"%s %d,%d",__func__, status, volt);
    msg.mod_id = APP_MODULE_BATTERY;
#if defined(USE_BASIC_THREADS)
    msg.mod_level = APP_MOD_LEVEL_0;
#endif
    APP_BATTERY_SET_MESSAGE(app_battevt, status, volt);
    msg.msg_body.message_id = app_battevt;
    msg.msg_body.message_ptr = (uint32_t)NULL;
    app_mailbox_put(&msg);
}

static void app_battery_charger_handler(enum PMU_CHARGER_STATUS_T status)
{
    BATTERY_TRACE(2,"%s: status=%d", __func__, status);
    pmu_charger_set_irq_handler(NULL);
    app_battery_event_process(APP_BATTERY_STATUS_PLUGINOUT,
                              (status == PMU_CHARGER_PLUGIN) ? APP_BATTERY_CHARGER_PLUGIN : APP_BATTERY_CHARGER_PLUGOUT);
}

static enum APP_BATTERY_CHARGER_T app_battery_charger_forcegetstatus(void)
{
    enum APP_BATTERY_CHARGER_T status = APP_BATTERY_CHARGER_QTY;
    enum PMU_CHARGER_STATUS_T charger;

    charger = pmu_charger_get_status();

    if (charger == PMU_CHARGER_PLUGIN)
    {
        status = APP_BATTERY_CHARGER_PLUGIN;
    }
    else
    {
        status = APP_BATTERY_CHARGER_PLUGOUT;
    }

    return status;
}

static void app_battery_pluginout_debounce_start(void)
{
    BATTERY_TRACE(1,"%s", __func__);

    app_battery_pluginout_debounce_ctx = (uint32_t)app_battery_charger_forcegetstatus();
    app_battery_pluginout_debounce_cnt = 1;
    osTimerStart(app_battery_pluginout_debounce_timer, CHARGER_PLUGINOUT_DEBOUNCE_MS);
}

static void app_battery_pluginout_event_callback(enum APP_BATTERY_CHARGER_T event);

static void app_battery_pluginout_debounce_handler(void const *param)
{
    enum APP_BATTERY_CHARGER_T status_charger = app_battery_charger_forcegetstatus();

	BATTERY_TRACE(1,"@@@app_battery_pluginout_debounce_handler");
	
#ifdef BESUI_TWS_EN
    bool factory_flag = false;
#endif
    if(app_battery_pluginout_debounce_ctx == (uint32_t) status_charger){
        app_battery_pluginout_debounce_cnt++;
    }
    else
    {
        BATTERY_TRACE(2,"%s dithering cnt %u", __func__, app_battery_pluginout_debounce_cnt);
        app_battery_pluginout_debounce_cnt = 0;
        app_battery_pluginout_debounce_ctx = (uint32_t)status_charger;
    }
#ifdef BESUI_TWS_EN
    BATTERY_TRACE(1, "[UIBAT]%s cnt %d", __func__, app_battery_pluginout_debounce_cnt);
#ifdef BESUI_1WIRE_EN
    if(status_charger == APP_BATTERY_CHARGER_PLUGOUT)
    {
        communication_uart_function_enable(true);
    }
#endif
#endif
    if (app_battery_pluginout_debounce_cnt >= CHARGER_PLUGINOUT_DEBOUNCE_CNT){
        BATTERY_TRACE(2,"%s %s", __func__, status_charger == APP_BATTERY_CHARGER_PLUGOUT ? "PLUGOUT" : "PLUGIN");
        if (status_charger == APP_BATTERY_CHARGER_PLUGIN)
        {
#ifndef BESUI_TWS_EN
            if (app_battery_ext_charger_enable_cfg.pin != HAL_IOMUX_PIN_NUM)
            {
                hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)app_battery_ext_charger_detecter_cfg.pin, HAL_GPIO_DIR_OUT, 0);
            }
#endif           
#ifdef BESUI_TWS_EN
            factory_flag = app_charge_fast_exit_factory_mode();

            if(factory_flag)
            {
                return;
            }
#endif
#ifdef BESUI_CHARGE_EN
            if(!besui_bat_charge_sta_get())
            {
                BATTERY_TRACE(0,"close box charging");
                besui_bat_charge_sta_set(true);

                if(uicom.poweron_bat_det_flag)
                {
                    app_charge_putinout_ui_post_msg(CHARGE_PLUGIN);
                }
            }
#endif
            app_battery_measure.start_time = hal_sys_timer_get();
        }
        else
        {
#ifdef BESUI_TWS_EN
            besui_bat_full_sta_set(false);
#else
            if (app_battery_ext_charger_enable_cfg.pin != HAL_IOMUX_PIN_NUM)
            {
                hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)app_battery_ext_charger_detecter_cfg.pin, HAL_GPIO_DIR_OUT, 1);
            }
#endif
#ifdef BESUI_CHARGE_EN
            {
                if(besui_bat_charge_sta_get())
                {
                    BATTERY_TRACE(0,"charging open box");
                    besui_bat_charge_sta_set(false);
                    if(uicom.poweron_bat_det_flag)
                    {
                        app_charge_putinout_ui_post_msg(CHARGE_PLUGOUT);
                    }
                }
            }
#endif
        }
        app_battery_pluginout_event_callback(status_charger);
        pmu_charger_set_irq_handler(app_battery_charger_handler);
        osTimerStop(app_battery_pluginout_debounce_timer);
    }else{
        osTimerStart(app_battery_pluginout_debounce_timer, CHARGER_PLUGINOUT_DEBOUNCE_MS);
    }
}

int app_battery_charger_indication_open(void)
{
    enum APP_BATTERY_CHARGER_T status = APP_BATTERY_CHARGER_QTY;

    BATTERY_TRACE(1,"%s",__func__);

    pmu_charger_init();

    pmu_charger_set_irq_handler(app_battery_charger_handler);

    return status;
}
#endif

int app_battery_open(void)
{
    app_battery_opened_callback();
#ifdef IS_USE_SOC_PMU_PLUGINOUT
    if (app_battery_pluginout_debounce_timer == NULL)
    {
        app_battery_pluginout_debounce_timer =
            osTimerCreate (osTimer(APP_BATTERY_PLUGINOUT_DEBOUNCE),
            osTimerOnce, &app_battery_pluginout_debounce_ctx);
    }

    app_set_threadhandle(APP_MODULE_BATTERY, app_battery_handle_process);

    app_battery_charger_indication_open();
#endif

    // initialize the custom battery manager here
    // returned value could be:
    // #define APP_BATTERY_OPEN_MODE_INVALID        (-1)
    // #define APP_BATTERY_OPEN_MODE_NORMAL         (0)
    // #define APP_BATTERY_OPEN_MODE_CHARGING       (1)
    // #define APP_BATTERY_OPEN_MODE_CHARGING_PWRON (2)
    return APP_BATTERY_OPEN_MODE_NORMAL;
}

int app_battery_start(void)
{
    // start battery measurement timer here
    return 0;
}

int app_battery_stop(void)
{
    // stop battery measurement timer here
    return 0;
}

int app_battery_get_info(APP_BATTERY_MV_T *currvolt, uint8_t *currlevel, enum APP_BATTERY_STATUS_T *status)
{
    // should just return battery level via currlevel for hfp battery level indication
    *currlevel = APP_BATTERY_LEVEL_MAX;
    return 0;
}

#ifdef IS_USE_SOC_PMU_PLUGINOUT
static void app_battery_pluginout_event_callback(enum APP_BATTERY_CHARGER_T event)
{
    if (APP_BATTERY_CHARGER_PLUGOUT == event)
    {
        BATTERY_TRACE(0, "Charger plug out.");
    }
    else if (APP_BATTERY_CHARGER_PLUGIN == event)
    {
        BATTERY_TRACE(0, "Charger plug in.");
    }
}
#endif

int app_battery_register(APP_BATTERY_CB_T user_cb)
{
    // register the battery level update event callback
    return 0;
}

#endif

WEAK void app_battery_opened_callback(void)
{

}

#else
int app_battery_open(void)
{
    return 0;
}

int app_battery_start(void)
{
    return 0;
}
#endif
