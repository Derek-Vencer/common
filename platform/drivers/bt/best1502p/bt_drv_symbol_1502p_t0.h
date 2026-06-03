
/*******************************************************************************************************************************/
/*********************************************bt controller symbol**************************************************************/

#ifndef  __BT_CONTROLLER_1502P_T0_SYMBOL_H__
#define  __BT_CONTROLLER_1502P_T0_SYMBOL_H__

/*****    Macro Definition of Relative Offset of Variables    *****/

// ld_acl_env_tag/rssi_record
#define OFF_RR_IN_LAET_LD_T0                                          0XE6
// ld_acl_env_tag/last_sync_bit_off
#define OFF_LSBO_IN_LAET_LD_T0                                        0XCE
// ld_acl_env_tag/t_poll
#define OFF_TP_IN_LAET_LD_T0                                          0XF0
// ld_acl_env_tag/last_sync_clk_off
#define OFF_LSCO_IN_LAET_LD_T0                                        0XB8
// ld_acl_env_tag/rxbit_1us
#define OFF_R1_IN_LAET_LD_T0                                          0XCA
// bt_util_buf_env_tag/acl_rx_free
#define OFF_ARF_IN_BUBET_BT_T0                                        0X14
// bt_util_buf_env_tag/acl_tx_free
#define OFF_ATF_IN_BUBET_BT_T0                                        0X28
// ble_util_buf_env_tag/acl_tx_free
#define OFF_ATF_IN_BUBET_BLE_T0                                       0X28
// dbg_bt_sche_setting_cmd/music_playing_link
#define OFF_MPL_IN_DBSSC_DBG_T0                                       0XA
// dbg_bt_sche_setting_cmd/acl_slot_in_ibrt_mode
#define OFF_ASIIM_IN_DBSSC_DBG_T0                                     0XC
// dbg_bt_sche_setting_cmd/acl_interv_in_ibrt_normal_mode
#define OFF_AIIINM_IN_DBSSC_DBG_T0                                    0X2
// dbg_bt_common_setting_cmd/trace_level
#define OFF_TL_IN_DBCSC_DBG_T0                                        0X8
// ld_bes_bt_env_tag/tws_device_env/link_id
#define OFF_LI_IN_LBBET_LD_T0                                         0X12
// dbg_bt_ibrt_setting_cmd/accept_remote_enter_sniff
#define OFF_ARES_IN_DBISC_DBG_T0                                      0XF
// dbg_bt_ibrt_setting_cmd/ibrt_second_sco_decision
#define OFF_ISSD_IN_DBISC_DBG_T0                                      0XE
// lc_sco_env_tag/p_link_params/link_id
#define OFF_LI_IN_LSET_LC_T0                                          0X1E
// lc_sco_nego_params_tag/air_params_received/m2s_pkt_type
#define OFF_MPT_IN_LSNPT_LC_T0                                        0X47
// lc_sco_nego_params_tag/air_params_received/air_mode
#define OFF_AM_IN_LSNPT_LC_T0                                         0X4E
// dbg_bt_hw_feat_setting_cmd/bt_sync_swagc_en
#define OFF_BSSE_IN_DBHFSC_DBG_T0                                     0XF
// dbg_bt_hw_feat_setting_cmd/le_sync_swagc_en
#define OFF_LSSE_IN_DBHFSC_DBG_T0                                     0X10
// rwip_env_tag/lp_clk
#define OFF_LC_IN_RET_RWIP_T0                                         0X3A4
// rwip_env_tag/last_bt_samp_time/hs
#define OFF_H_IN_RET_RWIP_0_T0                                        0X28
// rwip_env_tag/last_bt_samp_time/hus
#define OFF_H_IN_RET_RWIP_1_T0                                        0X2C
// rwip_env_tag/last_us_samp_time
#define OFF_LUST_IN_RET_RWIP_T0                                       0X34
// lld_cis_env/grp_hdl
#define OFF_GH_IN_LCE_LLD_T0                                          0X1A
// lld_cig_env/anchor_hs
#define OFF_AH_IN_LCE_LLD_0_T0                                        0X80
// lld_cig_env/anchor_hus
#define OFF_AH_IN_LCE_LLD_1_T0                                        0X84
// lld_cig_env/iso_interval
#define OFF_II_IN_LCE_LLD_T0                                          0X92
// lld_bis_env/grp_hdl
#define OFF_GH_IN_LBE_LLD_T0                                          0X1C
// lld_big_env/anchor_hs
#define OFF_AH_IN_LBE_LLD_0_T0                                        0XE8
// lld_big_env/anchor_hus
#define OFF_AH_IN_LBE_LLD_1_T0                                        0XEC
// lld_big_env/iso_interval
#define OFF_II_IN_LBE_LLD_T0                                          0XEE
// dbg_bt_common_setting_t2/tws_sleep_dura_dec
#define OFF_TSDD_IN_DBCST_DBG_T0                                      0X3A
// dbg_bt_common_setting_t2/normal_sleep_dura_inc
#define OFF_NSDI_IN_DBCST_DBG_T0                                      0X3B
// dbg_bt_common_setting_t2/en_sec_con_base_on_le_aud_sup
#define OFF_ESCBOLAS_IN_DBCST_DBG_T0                                  0X2C

#define HCI_FC_ENV_T0_ADDR                                            0XC0009AE0
#define LD_ACL_ENV_T0_ADDR                                            0XC000540C
#define BT_UTIL_BUF_ENV_T0_ADDR                                       0XC0004C04
#define BLE_UTIL_BUF_ENV_T0_ADDR                                      0XC0005A58
#define LD_BES_BT_ENV_T0_ADDR                                         0XC0005814
#define DBG_STATE_T0_ADDR                                             0XC0009CF6
#define LC_STATE_T0_ADDR                                              0XC0005324
#define LD_SCO_ENV_T0_ADDR                                            0XC00056E4
#define RX_MONITOR_T0_ADDR                                            0XC000A761
#define LC_ENV_T0_ADDR                                                0XC00050B4
#define LM_NB_SYNC_ACTIVE_T0_ADDR                                     0XC00050A0
#define LM_ENV_T0_ADDR                                                0XC0004E6C
#define HCI_ENV_T0_ADDR                                               0XC00099C0
#define LC_SCO_ENV_T0_ADDR                                            0XC0005090
#define LLM_ENV_T0_ADDR                                               0XC0005E2C
#define LD_ENV_T0_ADDR                                                0XC0005394
#define RWIP_ENV_T0_ADDR                                              0XC000A7FC
#define BLE_RX_MONITOR_T0_ADDR                                        0XC000A6B0
#define LLC_ENV_T0_ADDR                                               0XC0008948
#define RWIP_RF_T0_ADDR                                               0XC000A798
#define LD_ACL_METRICS_T0_ADDR                                        0XC000541C
#define RF_RX_HWGAIN_TBL_T0_ADDR                                      0XC000499B
#define RF_HWAGC_RSSI_CORRECT_TBL_T0_ADDR                             0XC00048B9
#define RF_RX_GAIN_FIXED_TBL_T0_ADDR                                  0XC000A5E8
#define HCI_DBG_EBQ_TEST_MODE_T0_ADDR                                 0XC0009D10
#define DBG_BT_COMMON_SETTING_T0_ADDR                                 0XC0009BA8
#define DBG_BT_SCHE_SETTING_T0_ADDR                                   0XC0009C54
#define DBG_BT_IBRT_SETTING_T0_ADDR                                   0XC0009C38
#define DBG_BT_HW_FEAT_SETTING_T0_ADDR                                0XC0009C14
#define HCI_DBG_SET_SW_RSSI_T0_ADDR                                   0XC0009D40
#define RWIP_PROG_DELAY_T0_ADDR                                       0XC000A794
#define DATA_BACKUP_CNT_T0_ADDR                                       0XC0004070
#define DATA_BACKUP_ADDR_PTR_T0_ADDR                                  0XC0004074
#define DATA_BACKUP_VAL_PTR_T0_ADDR                                   0XC0004078
#define SCH_MULTI_IBRT_ADJUST_ENV_T0_ADDR                             0XC0009920
#define RF_HWAGC_RSSI_CORRECT_TBL_T0_ADDR                             0XC00048B9
#define RF_RX_GAIN_THS_TBL_LE_T0_ADDR                                 0XC000495F
#define RF_RX_GAIN_THS_TBL_LE_2M_T0_ADDR                              0XC000497D
#define RF_RPL_TX_PW_CONV_TBL_T0_ADDR                                 0XC00048C9
#define REPLACE_MOBILE_ADDR_T0_ADDR                                   0XC000532C
#define REPLACE_ADDR_VALID_T0_ADDR                                    0XC0005328
#define PCM_NEED_START_FLAG_T0_ADDR                                   0XC0005700
#define RT_SLEEP_FLAG_CLEAR_T0_ADDR                                   0XC000B1CC
#define RF_RX_GAIN_THS_TBL_BT_3M_T0_ADDR                              0XC000A5F4
#define LD_IBRT_ENV_T0_ADDR                                           0XC0005828
#define LLM_LOCAL_LE_FEATS_T0_ADDR                                    0XC0004776
#define RF_RX_GAIN_THS_TBL_BT_T0_ADDR                                 0XC0004905
#define DBG_BT_COMMON_SETTING_T2_T0_ADDR                              0XC0009BCE
#define LLD_CON_ENV_T0_ADDR                                           0XC0008E20
#define LLD_ISO_ENV_T0_ADDR                                           0XC0008E68
#define ECC_RX_MONITOR_T0_ADDR                                        0XC000A6F8
#define __STACKLIMIT_T0_ADDR                                          0XC000B330
#define PER_MONITOR_PARAMS_T0_ADDR                                    0XC0004770
#define RF_RX_GAIN_THS_TBL_ECC_T0_ADDR                                0XC0004941
#define RX_MONITOR_3M_RXGAIN_T0_ADDR                                  0XC000476D
#define LLD_CIG_ENV_T0_ADDR                                           0XC00090D4
#define LLD_CIS_ENV_T0_ADDR                                           0XC00090E0
#define LLD_BIS_ENV_T0_ADDR                                           0XC0009134
#define LLD_BIG_ENV_T0_ADDR                                           0XC0009128
#define RX_RECORD_T0_ADDR                                             0XC000570C
#endif