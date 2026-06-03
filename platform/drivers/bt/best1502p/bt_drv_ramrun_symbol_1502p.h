
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
#define OFF_CTI_IN_LAET_LD                                            0X194
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


#define HCI_FC_ENV_ADDR                                               0XC000967C
#define LD_ACL_ENV_ADDR                                               0XC0005138
#define BT_UTIL_BUF_ENV_ADDR                                          0XC0004C48
#define BLE_UTIL_BUF_ENV_ADDR                                         0XC0005C28
#define LD_BES_BT_ENV_ADDR                                            0XC00059F4
#define DBG_STATE_ADDR                                                0XC0009960
#define LC_STATE_ADDR                                                 0XC0005050
#define LD_SCO_ENV_ADDR                                               0XC0005890
#define RX_MONITOR_ADDR                                               0XC000A39E
#define LC_ENV_ADDR                                                   0XC0004FF0
#define LM_ENV_ADDR                                                   0XC0004DA8
#define HCI_ENV_ADDR                                                  0XC0009598
#define LC_SCO_ENV_ADDR                                               0XC0004FCC
#define LLM_ENV_ADDR                                                  0XC0005E0C
#define LD_ENV_ADDR                                                   0XC00050BC
#define RWIP_ENV_ADDR                                                 0XC000A448
#define BLE_RX_MONITOR_ADDR                                           0XC000A32B
#define LLC_ENV_ADDR                                                  0XC00088C0
#define RWIP_RF_ADDR                                                  0XC000A3D4
#define RF_RX_GAIN_FIXED_TBL_ADDR                                     0XC000A244
#define HCI_DBG_EBQ_TEST_MODE_ADDR                                    0XC0009978
#define DBG_BT_COMMON_SETTING_ADDR                                    0XC000973C
#define DBG_BT_SCHE_SETTING_ADDR                                      0XC0009804
#define DBG_BT_IBRT_SETTING_ADDR                                      0XC00097E8
#define DBG_BT_HW_FEAT_SETTING_ADDR                                   0XC00097C4
#define HCI_DBG_SET_SW_RSSI_ADDR                                      0XC00099A8
#define SCH_MULTI_IBRT_ADJUST_ENV_ADDR                                0XC00094F8
#define RF_RX_GAIN_THS_TBL_LE_ADDR                                    0XC00049BA
#define RF_RX_GAIN_THS_TBL_LE_2M_ADDR                                 0XC00049D8
#define REPLACE_MOBILE_ADDR_ADDR                                      0XC000505C
#define REPLACE_ADDR_VALID_ADDR                                       0XC0005058
#define PCM_NEED_START_FLAG_ADDR                                      0XC00058DC
#define RF_RX_GAIN_THS_TBL_BT_3M_ADDR                                 0XC000A250
#define LD_IBRT_ENV_ADDR                                              0XC0005B80
#define LLM_LOCAL_LE_FEATS_ADDR                                       0XC00047C2
#define RF_RX_GAIN_THS_TBL_BT_ADDR                                    0XC0004960
#define DBG_BT_COMMON_SETTING_T2_ADDR                                 0XC0009762
#define LLD_CON_ENV_ADDR                                              0XC0008D78
#define LLD_ISO_ENV_ADDR                                              0XC0008DF8
#define ECC_RX_MONITOR_ADDR                                           0XC000A357
#define __STACKLIMIT_ADDR                                             0XC000AE30
#define PER_MONITOR_PARAMS_ADDR                                       0XC0004AAA
#define MASTER_CON_SUPPORT_LE_AUDIO_ADDR                              0XC0005000
#define RX_RECORD_ADDR                                                0XC00058E8
#define SCH_PROG_DBG_ENV_ADDR                                         0XC0009340
#define RF_RX_GAIN_THS_TBL_ECC_ADDR                                   0XC000499C
#define RX_MONITOR_3M_RXGAIN_ADDR                                     0XC00047C1
#define LLD_PER_ADV_ENV_ADDR                                          0XC0008CE4
#define MHDT_SWAGC_EN_ADDR                                            0XC000A370
#define RF_RX_GAIN_THS_TBL_MHDT_BT2M_2M_ADDR                          0XC000A26E
#define RF_RX_GAIN_THS_TBL_MHDT_BT2M_3M_ADDR                          0XC000A28C
#define RF_RX_GAIN_THS_TBL_MHDT_BT4M_2M_ADDR                          0XC000A2AA
#define LE_CON_SKIP_3_INTERVAL_ADDR                                   0XC0008D74
#define LLD_BIS_ENV_ADDR                                              0XC00091D4
#define LLD_BIG_ENV_ADDR                                              0XC00091C4
#define LLD_CIS_ENV_ADDR                                              0XC0009184
#define LLD_CIG_ENV_ADDR                                              0XC0009174
#define LLD_ADV_BT2MCU_INTERSYS1_ADDR                                 0XC0008C28
#define LLD_CON_BT2MCU_INTERSYS1_ADDR                                 0XC0008C30
#define SENS2BT_EN_ADDR                                               0XC0004090
#define DBG_BT_SCHE_SETTING_COEX_CIS_ADDR                             0XC000982A
#define DBG_BT_SCHE_SETTING_COEX_BIS_ADDR                             0XC000981C
#define REPLACE_MOBILE_LE_LINK_ID_ADDR                                0XC00047E0
#define REPLACE_MOBILE_LE_LINK_ID_VALID_ADDR                          0XC0008930
#define LLI_ENV_ADDR                                                  0XC0008804
#define LLI_CIS_ENV_ADDR                                              0XC0008880
#define AFH_ASSESS_REPORT_TRACE_ENABLE_ADDR                           0XC000AADC
#define CONTROLLER_VERISON_ADDR                                       0XC00040A0

//commit 743917306960f949244b7b2538b32baf4c59a346
//Author: donghuiguo <donghuiguo@bestechnic.com>
//Date:   Thu Apr 17 20:03:32 2025 +0800
//    [MHDT]:Add mhdt link throughput calc log.
//    
//    Change-Id: I0a6acbf9cadaf07fd9d1bfa1069fe78c0284aed2

#ifndef  BT_CONTROLLER_COMMIT_ID
#define  BT_CONTROLLER_COMMIT_ID                            "commit 743917306960f949244b7b2538b32baf4c59a346"
#endif
#ifndef  BT_CONTROLLER_COMMIT_AUTHOR
#define  BT_CONTROLLER_COMMIT_AUTHOR                        "Author: donghuiguo <donghuiguo@bestechnic.com>"
#endif
#ifndef  BT_CONTROLLER_COMMIT_DATE
#define  BT_CONTROLLER_COMMIT_DATE                          "Date:   Thu Apr 17 20:03:32 2025 +0800"
#endif
#ifndef  BT_CONTROLLER_COMMIT_MESSAGE
#define  BT_CONTROLLER_COMMIT_MESSAGE                       "[MHDT]:Add mhdt link throughput calc log.  Change-Id: I0a6acbf9cadaf07fd9d1bfa1069fe78c0284aed2 "
#endif
#ifndef  BT_CONTROLLER_BUILD_TIME
#define  BT_CONTROLLER_BUILD_TIME                           "2025-04-18 14:07:24"
#endif
#ifndef  BT_CONTROLLER_BUILD_OWNER
#define  BT_CONTROLLER_BUILD_OWNER                          "donghuiguo"
#endif

#endif
