
/*******************************************************************************************************************************/
/*********************************************bt controller symbol**************************************************************/

#ifndef  __BT_CONTROLLER_SYMBOL_H__
#define  __BT_CONTROLLER_SYMBOL_H__

/*****   Macro Definition of Relative Offset of Variables   *****/

// ld_acl_env_tag/rssi_record
#define OFF_RR_IN_LAET_LD                                             0XEA
// ld_acl_env_tag/last_sync_bit_off
#define OFF_LSBO_IN_LAET_LD                                           0XD2
// ld_acl_env_tag/t_poll
#define OFF_TP_IN_LAET_LD                                             0XF4
// ld_acl_env_tag/last_sync_clk_off
#define OFF_LSCO_IN_LAET_LD                                           0XBC
// ld_acl_env_tag/rxbit_1us
#define OFF_R1_IN_LAET_LD                                             0XCE
// ld_acl_env_tag/allowed_edr_packet_types
#define OFF_AEPT_IN_LAET_LD                                           0XEC
// bt_util_buf_env_tag/acl_rx_free
#define OFF_ARF_IN_BUBET_BT                                           0X14
// bt_util_buf_env_tag/acl_tx_free
#define OFF_ATF_IN_BUBET_BT                                           0X28
// ble_util_buf_env_tag/acl_tx_free
#define OFF_ATF_IN_BUBET_BLE                                          0X28
// dbg_bt_sche_setting_cmd/music_playing_link
#define OFF_MPL_IN_DBSSC_DBG                                          0XA
// dbg_bt_sche_setting_cmd/acl_slot_in_ibrt_mode
#define OFF_ASIIM_IN_DBSSC_DBG                                        0XC
// dbg_bt_sche_setting_cmd/acl_interv_in_ibrt_normal_mode
#define OFF_AIIINM_IN_DBSSC_DBG                                       0X2
// dbg_bt_common_setting_cmd/trace_level
#define OFF_TL_IN_DBCSC_DBG                                           0X8
// ld_bes_bt_env_tag/tws_device_env/link_id
#define OFF_LI_IN_LBBET_LD                                            0X12
// dbg_bt_ibrt_setting_cmd/accept_remote_enter_sniff
#define OFF_ARES_IN_DBISC_DBG                                         0XF
// dbg_bt_ibrt_setting_cmd/ibrt_second_sco_decision
#define OFF_ISSD_IN_DBISC_DBG                                         0XE
// lc_sco_env_tag/p_link_params/link_id
#define OFF_LI_IN_LSET_LC                                             0X1E
// lc_sco_nego_params_tag/air_params_received/m2s_pkt_type
#define OFF_MPT_IN_LSNPT_LC                                           0X47
// lc_sco_nego_params_tag/air_params_received/air_mode
#define OFF_AM_IN_LSNPT_LC                                            0X4E
// dbg_bt_hw_feat_setting_cmd/bt_sync_swagc_en
#define OFF_BSSE_IN_DBHFSC_DBG                                        0XF
// dbg_bt_hw_feat_setting_cmd/le_sync_swagc_en
#define OFF_LSSE_IN_DBHFSC_DBG                                        0X10
// rwip_env_tag/lp_clk
#define OFF_LC_IN_RET_RWIP                                            0X39C
// rwip_env_tag/last_bt_samp_time/hs
#define OFF_H_IN_RET_RWIP_0                                           0X28
// rwip_env_tag/last_bt_samp_time/hus
#define OFF_H_IN_RET_RWIP_1                                           0X2C
// rwip_env_tag/last_us_samp_time
#define OFF_LUST_IN_RET_RWIP                                          0X34
// lld_cis_env/grp_hdl
#define OFF_GH_IN_LCE_LLD                                             0X1A
// lld_cig_env/anchor/hs
#define OFF_H_IN_LCE_LLD_0                                            0X5C
// lld_cig_env/anchor/hus
#define OFF_H_IN_LCE_LLD_1                                            0X60
// lld_cig_env/iso_interval
#define OFF_II_IN_LCE_LLD                                             0X9E
// lld_bis_env/grp_hdl
#define OFF_GH_IN_LBE_LLD                                             0X20
// lld_big_env/anchor/hs
#define OFF_H_IN_LBE_LLD_0                                            0XC4
// lld_big_env/anchor/hus
#define OFF_H_IN_LBE_LLD_1                                            0XC8
// lld_big_env/iso_interval
#define OFF_II_IN_LBE_LLD                                             0XFC
// dbg_bt_common_setting_t2/bt_master_sleep_en
#define OFF_BMSE_IN_DBCST_DBG                                         0X2F
// dbg_bt_common_setting_t2/bt_master_tx_silence_en
#define OFF_BMTSE_IN_DBCST_DBG                                        0X30
// dbg_bt_common_setting_t2/tws_sleep_dura_dec
#define OFF_TSDD_IN_DBCST_DBG                                         0X3A
// dbg_bt_common_setting_t2/normal_sleep_dura_inc
#define OFF_NSDI_IN_DBCST_DBG                                         0X3B
// dbg_bt_common_setting_t2/ull_cig_enable
#define OFF_UCE_IN_DBCST_DBG                                          0X35
// ld_acl_env_tag/cur_txpwr_idx
#define OFF_CTI_IN_LAET_LD                                            0X190
// rwip_rf_api/ble_txpwr_max
#define OFF_BTM_IN_RRA_RWIP                                           0X5A
// lld_con_env/next_ts
#define OFF_NT_IN_LCE_LLD                                             0XAC
// lld_con_env/interval
#define OFF_I_IN_LCE_LLD                                              0XD4
// lld_con_env/evt_cnt
#define OFF_EC_IN_LCE_LLD                                             0XE6
// lld_con_env_tag/tx_pwr_lvl
#define OFF_TPL_IN_LCET_LLD                                           0X10A
// lc_env/link/Role
#define OFF_R_IN_LE_LC                                                0X3A
// dbg_bt_common_setting_t2/en_sec_con_base_on_le_aud_sup
#define OFF_ESCBOLAS_IN_DBCST_DBG                                     0X2C


#define HCI_FC_ENV_ADDR                                               0XC0009EBC
#define LD_ACL_ENV_ADDR                                               0XC0005408
#define BT_UTIL_BUF_ENV_ADDR                                          0XC0004C50
#define BLE_UTIL_BUF_ENV_ADDR                                         0XC0005CBC
#define LD_BES_BT_ENV_ADDR                                            0XC00058E4
#define DBG_STATE_ADDR                                                0XC000A1A4
#define LC_STATE_ADDR                                                 0XC0005320
#define LD_SCO_ENV_ADDR                                               0XC00057A0
#define RX_MONITOR_ADDR                                               0XC000ABA6
#define LC_ENV_ADDR                                                   0XC0005094
#define LM_ENV_ADDR                                                   0XC0004E00
#define HCI_ENV_ADDR                                                  0XC0009DB4
#define LC_SCO_ENV_ADDR                                               0XC0005070
#define LLM_ENV_ADDR                                                  0XC0005F6C
#define LD_ENV_ADDR                                                   0XC0005390
#define RWIP_ENV_ADDR                                                 0XC000AC58
#define BLE_RX_MONITOR_ADDR                                           0XC000AB21
#define LLC_ENV_ADDR                                                  0XC0008D28
#define RWIP_RF_ADDR                                                  0XC000ABE4
#define RF_RX_GAIN_FIXED_TBL_ADDR                                     0XC000AA94
#define HCI_DBG_EBQ_TEST_MODE_ADDR                                    0XC000A1BC
#define DBG_BT_COMMON_SETTING_ADDR                                    0XC0009F84
#define DBG_BT_SCHE_SETTING_ADDR                                      0XC000A048
#define DBG_BT_IBRT_SETTING_ADDR                                      0XC000A02C
#define DBG_BT_HW_FEAT_SETTING_ADDR                                   0XC000A008
#define HCI_DBG_SET_SW_RSSI_ADDR                                      0XC000A1EC
#define SCH_MULTI_IBRT_ADJUST_ENV_ADDR                                0XC0009CF0
#define RF_RX_GAIN_THS_TBL_LE_ADDR                                    0XC00049BA
#define RF_RX_GAIN_THS_TBL_LE_2M_ADDR                                 0XC00049D8
#define REPLACE_MOBILE_ADDR_ADDR                                      0XC0005330
#define REPLACE_ADDR_VALID_ADDR                                       0XC000532C
#define PCM_NEED_START_FLAG_ADDR                                      0XC00057C0
#define RF_RX_GAIN_THS_TBL_BT_3M_ADDR                                 0XC000AAA0
#define LD_IBRT_ENV_ADDR                                              0XC0005A6C
#define LLM_LOCAL_LE_FEATS_ADDR                                       0XC00047C2
#define RF_RX_GAIN_THS_TBL_BT_ADDR                                    0XC0004960
#define DBG_BT_COMMON_SETTING_T2_ADDR                                 0XC0009FAA
#define LLD_CON_ENV_ADDR                                              0XC00092D0
#define LLD_ISO_ENV_ADDR                                              0XC0009320
#define ECC_RX_MONITOR_ADDR                                           0XC000AB55
#define __STACKLIMIT_ADDR                                             0XC000B600
#define PER_MONITOR_PARAMS_ADDR                                       0XC0004AAA
#define MASTER_CON_SUPPORT_LE_AUDIO_ADDR                              0XC00050A8
#define RX_RECORD_ADDR                                                0XC00057CE
#define SCH_PROG_DBG_ENV_ADDR                                         0XC0009B38
#define RF_RX_GAIN_THS_TBL_ECC_ADDR                                   0XC000499C
#define RX_MONITOR_3M_RXGAIN_ADDR                                     0XC00047C1
#define LLD_PER_ADV_ENV_ADDR                                          0XC000921C
#define LE_CON_SKIP_3_INTERVAL_ADDR                                   0XC00092CC
#define LLD_BIS_ENV_ADDR                                              0XC00099EC
#define LLD_BIG_ENV_ADDR                                              0XC00099CC
#define LLD_CIS_ENV_ADDR                                              0XC000997C
#define LLD_CIG_ENV_ADDR                                              0XC000995C
#define LLD_ADV_BT2MCU_INTERSYS1_ADDR                                 0XC0009130
#define LLD_CON_BT2MCU_INTERSYS1_ADDR                                 0XC0009138
#define SENS2BT_EN_ADDR                                               0XC0004090
#define DBG_BT_SCHE_SETTING_COEX_CIS_ADDR                             0XC000A06E
#define DBG_BT_SCHE_SETTING_COEX_BIS_ADDR                             0XC000A060
#define REPLACE_MOBILE_LE_LINK_ID_ADDR                                0XC00047E0
#define REPLACE_MOBILE_LE_LINK_ID_VALID_ADDR                          0XC0008DB4
#define LLI_ENV_ADDR                                                  0XC0008C38
#define LLI_CIS_ENV_ADDR                                              0XC0008CD8
#define AFH_ASSESS_REPORT_TRACE_ENABLE_ADDR                           0XC000B2D4
#define CONTROLLER_VERISON_ADDR                                       0XC00040A0

//commit 5277e2b19880656046d15b15986c6cf9caa238cb
//Author: jiansong <jiansong@bestechnic.com>
//Date:   Thu Feb 27 16:05:52 2025 +0800
//    reduce the max_slot limit
//    
//    Change-Id: I220f0ffabbae1eda2480216440ee61f088c62940

#ifndef  BT_CONTROLLER_COMMIT_ID
#define  BT_CONTROLLER_COMMIT_ID                            "commit 5277e2b19880656046d15b15986c6cf9caa238cb"
#endif
#ifndef  BT_CONTROLLER_COMMIT_AUTHOR
#define  BT_CONTROLLER_COMMIT_AUTHOR                        "Author: jiansong <jiansong@bestechnic.com>"
#endif
#ifndef  BT_CONTROLLER_COMMIT_DATE
#define  BT_CONTROLLER_COMMIT_DATE                          "Date:   Thu Feb 27 16:05:52 2025 +0800"
#endif
#ifndef  BT_CONTROLLER_COMMIT_MESSAGE
#define  BT_CONTROLLER_COMMIT_MESSAGE                       "reduce the max_slot limit  Change-Id: I220f0ffabbae1eda2480216440ee61f088c62940 "
#endif
#ifndef  BT_CONTROLLER_BUILD_TIME
#define  BT_CONTROLLER_BUILD_TIME                           "2025-03-03 20:16:59"
#endif
#ifndef  BT_CONTROLLER_BUILD_OWNER
#define  BT_CONTROLLER_BUILD_OWNER                          "liduo"
#endif

#endif
