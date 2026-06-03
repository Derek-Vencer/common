/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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
#ifndef __APP_UI_EVT_H__
#define __APP_UI_EVT_H__

#if BLE_AUDIO_ENABLED
#include "bluetooth_ble_api.h"
#endif
#include "app_ibrt_conn_evt.h"

#define BD_ADDR_LEN     6

typedef  uint16_t   bud_box_state;
/*
 * BOX state
 */
#define IBRT_BOX_UNKNOWN            (0)
#define IBRT_IN_BOX_CLOSED          (1)
#define IBRT_IN_BOX_OPEN            (2)
#define IBRT_OUT_BOX                (3)
#define IBRT_OUT_BOX_WEARED         (4)

#define APP_UI_BOX_EVT_BASE           0x0010
#define APP_UI_PEER_BOX_EVT_BASE      0x0020
#define APP_UI_TW_CONNECTED_EVT_BASE  0x0040
#define APP_UI_TWS_PAIRING_EVT_BASE   0x0080
#define APP_UI_MOBILE_EVT_BASE        0x0100
#define APP_UI_RELOAD_EVT_BASE        0x0200
#define APP_UI_IBRT_EVT_BASE          0x0400
#define APP_UI_DESTROY_EVT_BASE       0x0800
#define APP_UI_ROLE_SWITCH_EVT_BASE   0x1000
#define APP_UI_EXCHANG_INFO_EVT_BASE  0x2000
#define APP_UI_IGNORE_EVT_BASE        0x4000
#define APP_UI_ACTIVE_EVT_BASE        0x8000
#define APP_UI_LE_EVT_BASE            0x10000

#define IBRT_MGR_BOX_EVT_MASK       APP_UI_BOX_EVT_BASE
#define IBRT_MGR_PEER_BOX_EVT_MASK  APP_UI_PEER_BOX_EVT_BASE
#define LE_DEVICE_CONN_EVT_MASK     APP_UI_LE_EVT_BASE
#define IBRT_MGR_IBRT_ACTION        APP_UI_ACTIVE_EVT_BASE

/* Exchange info status */
#define EXCHANGE_INFO_SUCCESS       (0)
#define EXCHANGE_INFO_FAILED        (1)
#define EXCHANGE_INFO_TIMEOUT       (2)
#define EXCHANGE_INFO_RESTART       (4)

#define MOBILE_LINK_ID_INVALID      0xFF

typedef enum
{
    EVT_OPEN_CLOSE_GROUP,
    EVT_OUT_IN_GROUP,
    EVT_UP_DOWN_GROUP,
    EVT_GROUP_MAX
} app_ui_evt_group_type_t;

typedef enum {

     /**********Device State Events**********/
    APP_UI_EV_CASE_OPEN = APP_UI_BOX_EVT_BASE+1,  // Device case opened
    APP_UI_EV_CASE_CLOSE,                         // Device case closed
    APP_UI_EV_DOCK,                               // Device docked
    APP_UI_EV_UNDOCK,                             // Device undocked
    APP_UI_EV_WEAR_UP,                            // Device worn on ear
    APP_UI_EV_WEAR_DOWN,                          // Device removed from ear

    /**********Peer Device State Events**********/
    APP_UI_EV_PEER_CASE_OPEN = APP_UI_PEER_BOX_EVT_BASE+1,// Peer device case opened
    APP_UI_EV_PEER_CASE_CLOSE,                            // Peer device case closed
    APP_UI_EV_PEER_DOCK,                                  // Peer device docked
    APP_UI_EV_PEER_UNDOCK,                                // Peer device undocked
    APP_UI_EV_PEER_WEAR_UP,                               // Peer device worn on ear
    APP_UI_EV_PEER_WEAR_DOWN,                             // Peer device removed from ear

     /**********TWS Connection Events**********/
    APP_UI_EV_TW_CONNECTED = APP_UI_TW_CONNECTED_EVT_BASE,// TWS connection established
    APP_UI_EV_TW_DISCONNECTED,                            // TWS disconnected
    APP_UI_EV_TW_CONNECTING,                              // TWS connecting
    APP_UI_EV_TW_CONNECTING_FAILURE,                      // TWS connection failure
    APP_UI_EV_BESAUD_CONNECTED,                           // BES audio connected

     /**********TWS Pairing Events**********/
    APP_UI_EV_TWS_PAIRING_COMPLETE = APP_UI_TWS_PAIRING_EVT_BASE,// TWS pairing completed
    APP_UI_EV_TW_CANCEL_PAGE,                                   // Cancel TWS page

    /**********Mobile Device Connection and Encryption Events**********/
    APP_UI_EV_MOBILE_CONNECTED = APP_UI_MOBILE_EVT_BASE, // Mobile device connected
    APP_UI_EV_MOBILE_DISCONNECTED,                       // Mobile device disconnected
    APP_UI_EV_MOBILE_CONNECTING,                         // Mobile device connecting
    APP_UI_EV_MOBILE_CONNECTING_FAILURE,                 // Mobile device connecting failure
    APP_UI_EV_MOBILE_SIMPLE_PIARING_COMPLETE,            // Mobile device simple pairing complete
    APP_UI_EV_MOBILE_AUTH_COMPLETE,                      // Mobile device authentication complete
    APP_UI_EV_MOBILE_RUN_COMPLETE,                       // Mobile device run complete
    APP_UI_EV_MOBILE_CANCEL,                             // Mobile device cancel

    /**********reload complete Events**********/
    APP_UI_EV_PEER_RELOAD_COMPLETE = APP_UI_RELOAD_EVT_BASE,  //Reload Complete

    /**********IBRT Connection Events**********/
    APP_UI_EV_IBRT_CONNECTING_FAILURE = APP_UI_IBRT_EVT_BASE,// IBRT connection failure
    APP_UI_EV_IBRT_CONNECTED,                                // IBRT connected
    APP_UI_EV_IBRT_DISCONNECTED,                             // IBRT disconnected
    APP_UI_EV_IBRT_ACL_CONNECTED,                            // IBRT ACL connected

    /**********Deletion Related Events**********/
    APP_UI_EV_DESTROY_DEVICE_SUCCESS = APP_UI_DESTROY_EVT_BASE,// Device destroyed successfully
    APP_UI_EV_DELETE_RECORD_SUCCESS,                           // Record deleted successfully


    /**********Role Switch Events**********/
    APP_UI_EV_ROLE_SWITCH_COMPLETE = APP_UI_ROLE_SWITCH_EVT_BASE,// Role switch completed
    APP_UI_EV_ROLE_STATUS_CHANGED,                               // Role status changed
    

    /**********Information Exchange Events**********/
    APP_UI_EV_EXCHANG_INFO_COMPLETE = APP_UI_EXCHANG_INFO_EVT_BASE,// Information exchange completed
    APP_UI_EV_EXCHANG_INFO_FAIL,                                   // Information exchange failed
    APP_UI_EV_EXCHANGE_INFO_RESTART,                               // Information exchange restarted

    /**********Ignore Events**********/
    APP_UI_EV_TWS_FSM_IGNORE = APP_UI_IGNORE_EVT_BASE,// TWS FSM ignores this event
    APP_UI_EV_UI_IGNORE,                              // Ignore event
    APP_UI_EV_NO_USED_PARAM_IGNORE,
    APP_UI_EV_NONE = APP_UI_BOX_EVT_BASE,
    APP_UI_EV_PEER_EV_NONE = APP_UI_PEER_BOX_EVT_BASE,

     /**********Information Exchange Events**********/
    APP_UI_EV_TWS_CONNECT_SIG = APP_UI_ACTIVE_EVT_BASE,// TWS connection signal
    APP_UI_EV_AUTO_CONNECT_SIG,                             // Auto connect signal
    APP_UI_EV_PEER_AUTO_CONNECT_SIG,                        // Peer auto connect signal
    APP_UI_EV_TWS_PAIRING,                           // TWS pairing started
    APP_UI_EV_DELETE_RECORD,
    APP_UI_EV_FREE_MAN_MODE,
    APP_UI_EV_MOBILE_RECONNECT,
    APP_UI_EV_PHONE_CONNECT,
    APP_UI_EV_TWS_RECONNECT,
    APP_UI_EV_PEER_MOBILE_RECONNECT,
    APP_UI_EV_USER_SWITCH2MASTER,   // User switched to master
    APP_UI_EV_USER_SWITCH2SLAVE,// User switched to slave
    APP_UI_EV_KEEP_ONLY_MOB_LINK,
    APP_UI_EV_ALL_DISCONNECT_SIG,
    APP_UI_EV_SYS_SHUTDOWN,
    APP_UI_EV_EXIT_EARBUD_MODE,
    APP_UI_EV_DISCONNECT_MOBILE,
    APP_UI_EV_DESTROY_DEVICE,

    /**********LE Audio Related Events (0x10000)**********/
#if BLE_AUDIO_ENABLED
    /****** LE conn*********/
    APP_UI_EV_LE_MOB_CONNECTED = APP_UI_LE_EVT_BASE,// LE mobile device connected
    APP_UI_EV_LE_MOB_DISCONNECTED,                  // LE mobile device disconnected

    /****** LE encrypt*********/
    APP_UI_EV_LE_MOB_ENCRYPT_FAILURE,               // LE mobile device bonding failed
    APP_UI_EV_LE_MOB_ENCRYPT,                       // LE mobile device encrypted
    APP_UI_EV_LE_MOB_AUD_ATTR_BOND,                 // LE mobile device audio attribute bonded
    APP_UI_EV_SMP_IA_EXCHED,                        // SMP IA exchanged
#endif
} app_ui_evt_t;


typedef enum {
    APP_UI_EVENT_FIRST                  = IBRT_CONN_EVENT_LAST,

    // Extend for ibrt manager
    APP_UI_EVENT_BOX_STATE,
    APP_UI_EVENT_OPERATION_STATE,
    APP_UI_EVENT_EXCHANGE_LINK_INFO,
    APP_UI_MOBILE_SM_RUN_COMPLETE,

    APP_UI_EVENT_LAST                  = 0x2000
} app_ui_event_type;

typedef enum {
    Enter_pairing,
    Exit_pairing,
} pairing_evt_e;

typedef enum {
    APP_UI_UI_MODE_INFO = 0,
    APP_UI_MULTIPOINT_INFO,
    APP_UI_RELOAD_COMPLETE_INFO,
    APP_UI_BOX_STATE_INFO,
    APP_UI_PRIRING_INFO,
    APP_UI_SWITCH_UI_ROLE,
    APP_UI_SWITCH_UI_ROLE_COMPLETE,
    APP_UI_TWS_ROLE_SWITCH_REQ_NOTIFY,
    APP_UI_LEA_ADV_INFO,
    APP_UI_LEA_CONNECTED,
} app_ui_info_type;

typedef struct {
    app_ui_event_type      type;
    uint16_t               length;
} ibrt_ui_evt_header;

typedef struct {
    ibrt_ui_evt_header     header;
    app_ui_evt_t           box_evt;
} app_ui_box_state_pkt;

typedef enum
{
    // Request/response information types
    REQ_NONE,
    REQ_LOCAL_RUN,        /* Request running of initiator*/
    REQ_WAIT_DECISION,    /* Request running is decision by responder*/
    REQ_LOCAL_RUN_PREFER, /* if peer mobile connected,peer run,otherwise local run */

    // Response exchange infor types from deciesion
    RSP_LOCAL_RUN,         /* Running of initiator*/
    RSP_LOCAL_RUN_DEFER,   /* Running of initiator,but defered until respondor idle*/
    RSP_PEER_RUN,          /* Running of responder */
    RSP_RESTART,

    // Notify information types
    NOTIFY_LINK_IDLE,              /* Notify peer side current link is idle */
    NOTIFY_PEER_EVENT_COMPLETE,    /* Notify peer side peer event run complete */
} ibrt_ui_req_info_type_t;

typedef struct
{
    ibrt_ui_req_info_type_t   req_type;
    ibrt_ui_req_info_type_t   rsp_type;
    uint8_t                   req_link_id;
} ibrt_ui_req_hdr_t;

typedef struct {
    ibrt_ui_req_hdr_t   req_hdr;
    bt_bdaddr_t         addr;
    uint8_t             link_id;
    app_ui_evt_t        box_event;
    bool                mobile_link_status;
    bool                ibrt_link_status;
    bool                is_busy;
    bool                exchangeinfo_ongoing;
    bool                peer_busy_status;
    bool                is_sco_active;
} ibrt_ui_share_link_info; //aligment. 18 bytes

typedef struct {
    nvrec_btdevicerecord  dev_record;
    bool                  mobile_link_status;
} ibrt_ui_nv_info_rec; //26 bytes

typedef enum {
    SINGLE_POINT_MODE = 1,
    DOUBLE_POINT_MODE,
    TRIPLE_POINT_MODE,
    MAX_POINT_MODE = TRIPLE_POINT_MODE,
} multipoint_mode_t;

typedef struct {
    ibrt_ui_evt_header       header;
    uint32_t                 status;     /* 0:SUCCESS,1:FAIL */
    uint16_t                 rsp_req;
    uint16_t                 restart_cnt;

    union
    {
        ibrt_ui_share_link_info share_link_info[BT_DEVICE_NUM];
    }link_info;
} app_ui_link_info_pkt;

typedef struct {
    ibrt_ui_evt_header      header;
    app_ui_evt_t            ibrt_evt;
    uint16_t                link_id;
    bt_bdaddr_t             addr;
} app_ui_ibrt_pkt;

typedef struct {
    ibrt_ui_evt_header      header;
    uint16_t                link_type;
    uint16_t                link_id;        /* 0:BT, 1:LE */
    uint16_t                complete_evt;
} app_ui_mobile_link_pkt;

typedef struct {
    ibrt_ui_evt_header      header;
    uint8_t                 type;
} app_ui_link_status_pkt;

typedef struct {
    ibrt_ui_evt_header      header;
    app_ui_evt_t            ui_evt;
    uint16_t                param_0;
    uint16_t                param_1;
} app_ui_event_pkt;

typedef struct {
    uint16_t rsp_seq;
    bt_bdaddr_t addr;
    bool delete_record;
} app_ui_destroy_pkt;

typedef struct {
    bool support_leaudio;
    bool support_multipoint;
    bt_bdaddr_t addr;
} app_ui_mode_pkt;

typedef struct {
    multipoint_mode_t mode;
    int reserved_count;
    bt_bdaddr_t reserved_addrs[BT_DEVICE_NUM];
} app_ui_multipoint_pkt;

typedef struct {
    pairing_evt_e   evt;
    uint32_t    timeout;
} app_ui_pairing_pkt;

typedef struct {
    bool         switch2master;
} app_ui_rs_pkt;

typedef struct {
    bool         switch2master;
    uint8_t      errCode;
} app_ui_rs_cmp_pkt;

#if BLE_AUDIO_ENABLED
typedef struct {
    bool                start;
    uint32_t         duration;
    ble_bdaddr_t       remote;
} ibrt_ui_lea_adv_pkt;

typedef struct {
    ble_bdaddr_t       remote;
} ibrt_ui_lea_connected_pkt;
#endif

typedef struct {
    app_ui_info_type     type;
    union
    {
        app_ui_mode_pkt               uimode;
        app_ui_multipoint_pkt     multipoint;
        app_ui_pairing_pkt           pairing;
        bud_box_state              box_state;
        app_ui_rs_pkt         switch_ui_role;
        app_ui_rs_cmp_pkt switch_ui_role_cmp;
    #if BLE_AUDIO_ENABLED
        ibrt_ui_lea_adv_pkt          lea_adv;
        ibrt_ui_lea_connected_pkt    lea_connected;
    #endif
    } info;
} app_ui_info_pkt;

#if BLE_AUDIO_ENABLED
typedef struct {
    uint8_t irk[BLE_IRK_SIZE];
    ble_bdaddr_t ble_addr;
} app_ui_smp_info_pkt;
#endif

//Add more packet
typedef union {
    app_ui_box_state_pkt        box_state;
    app_ui_link_info_pkt        ibrt_link_info;
    app_ui_ibrt_pkt             ibrt_event;
    app_ui_mobile_link_pkt      mobile_complete_evt;
    app_ui_link_status_pkt      link_status;
    app_ui_event_pkt            peer_side_event;
    bt_bdaddr_t                 address;
#if BLE_AUDIO_ENABLED
    app_ui_smp_info_pkt         smp_pkt;
#endif
} app_ui_evt_pkt;

typedef struct {
    uint32_t func;
    uint32_t type;
    uint32_t para0;
    uint32_t para1;
    uint32_t para2;
    uint32_t para3;
} app_ui_func_pkt;

/* super sm state define */
typedef enum {
    SUPER_STATE_IDLE,
    SUPER_STATE_W4_TWS,
    SUPER_STATE_W4_DEVICE,
    SUPER_STATE_RELOAD,
    SUPER_STATE_EXCHANGE_INFO,
} app_ui_super_state;

const char *super_state_to_string(app_ui_super_state state);
const char *app_ui_event_to_string(app_ui_evt_t type);
const char *app_ui_box_state_to_string(bud_box_state box_state);
bool app_ui_is_user_action_event(app_ui_evt_t evt);
bool app_ui_is_peer_box_event(app_ui_evt_t evt);
bool app_ui_is_peer_event(app_ui_evt_t evt);
bool app_ui_is_box_event(app_ui_evt_t evt);
bool app_ui_is_high_priority_event(app_ui_evt_t event);
int app_ui_evt_to_group(app_ui_evt_t evt);
const char *app_ui_req_type_to_string(ibrt_ui_req_info_type_t type);
app_ui_evt_t app_ui_local_evt_convert_to_peer_evt(app_ui_evt_t evt);
app_ui_evt_t app_ui_peer_evt_convert_to_local_evt(app_ui_evt_t evt);
app_ui_evt_group_type_t app_ui_get_event_group_type(app_ui_evt_t evt);
bool app_ui_is_event_group(app_ui_evt_t event_a, app_ui_evt_t event_b);

#endif
