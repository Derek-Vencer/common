
/*******************************************************************************************************************************/
/*********************************************bt controller symbol**************************************************************/

#ifndef  __BT_CONTROLLER_1502P_T1_SYMBOL_H__
#define  __BT_CONTROLLER_1502P_T1_SYMBOL_H__

/*****   Macro Definition of Relative Offset of Variables   *****/

// ld_acl_env_tag/rssi_record
#define OFF_RR_IN_LAET_LD_1502P_T1                                    0XEA
// ld_acl_env_tag/last_sync_bit_off
#define OFF_LSBO_IN_LAET_LD_1502P_T1                                  0XD2
// ld_acl_env_tag/t_poll
#define OFF_TP_IN_LAET_LD_1502P_T1                                    0XF4
// ld_acl_env_tag/last_sync_clk_off
#define OFF_LSCO_IN_LAET_LD_1502P_T1                                  0XBC
// ld_acl_env_tag/rxbit_1us
#define OFF_R1_IN_LAET_LD_1502P_T1                                    0XCE
// bt_util_buf_env_tag/acl_rx_free
#define OFF_ARF_IN_BUBET_BT_1502P_T1                                  0X14
// bt_util_buf_env_tag/acl_tx_free
#define OFF_ATF_IN_BUBET_BT_1502P_T1                                  0X28
// ble_util_buf_env_tag/acl_tx_free
#define OFF_ATF_IN_BUBET_BLE_1502P_T1                                 0X28
// dbg_bt_sche_setting_cmd/music_playing_link
#define OFF_MPL_IN_DBSSC_DBG_1502P_T1                                 0XA
// dbg_bt_sche_setting_cmd/acl_slot_in_ibrt_mode
#define OFF_ASIIM_IN_DBSSC_DBG_1502P_T1                               0XC
// dbg_bt_sche_setting_cmd/acl_interv_in_ibrt_normal_mode
#define OFF_AIIINM_IN_DBSSC_DBG_1502P_T1                              0X2
// dbg_bt_common_setting_cmd/trace_level
#define OFF_TL_IN_DBCSC_DBG_1502P_T1                                  0X8
// ld_bes_bt_env_tag/tws_device_env/link_id
#define OFF_LI_IN_LBBET_LD_1502P_T1                                   0X12
// dbg_bt_ibrt_setting_cmd/accept_remote_enter_sniff
#define OFF_ARES_IN_DBISC_DBG_1502P_T1                                0XF
// dbg_bt_ibrt_setting_cmd/ibrt_second_sco_decision
#define OFF_ISSD_IN_DBISC_DBG_1502P_T1                                0XE
// lc_sco_env_tag/p_link_params/link_id
#define OFF_LI_IN_LSET_LC_1502P_T1                                    0X1E
// lc_sco_nego_params_tag/air_params_received/m2s_pkt_type
#define OFF_MPT_IN_LSNPT_LC_1502P_T1                                  0X47
// lc_sco_nego_params_tag/air_params_received/air_mode
#define OFF_AM_IN_LSNPT_LC_1502P_T1                                   0X4E
// dbg_bt_hw_feat_setting_cmd/bt_sync_swagc_en
#define OFF_BSSE_IN_DBHFSC_DBG_1502P_T1                               0XF
// dbg_bt_hw_feat_setting_cmd/le_sync_swagc_en
#define OFF_LSSE_IN_DBHFSC_DBG_1502P_T1                               0X10
// rwip_env_tag/lp_clk
#define OFF_LC_IN_RET_RWIP_1502P_T1                                   0X39C
// rwip_env_tag/last_bt_samp_time/hs
#define OFF_H_IN_RET_RWIP_0_1502P_T1                                  0X28
// rwip_env_tag/last_bt_samp_time/hus
#define OFF_H_IN_RET_RWIP_1_1502P_T1                                  0X2C
// rwip_env_tag/last_us_samp_time
#define OFF_LUST_IN_RET_RWIP_1502P_T1                                 0X34
// lld_cis_env/grp_hdl
#define OFF_GH_IN_LCE_LLD_1502P_T1                                    0X1A
// lld_cig_env/anchor/hs
#define OFF_H_IN_LCE_LLD_0_1502P_T1                                   0X5C
// lld_cig_env/anchor/hus
#define OFF_H_IN_LCE_LLD_1_1502P_T1                                   0X60
// lld_cig_env/iso_interval
#define OFF_II_IN_LCE_LLD_1502P_T1                                    0X9E
// lld_bis_env/grp_hdl
#define OFF_GH_IN_LBE_LLD_1502P_T1                                    0X20
// lld_big_env/anchor/hs
#define OFF_H_IN_LBE_LLD_0_1502P_T1                                   0XC4
// lld_big_env/anchor/hus
#define OFF_H_IN_LBE_LLD_1_1502P_T1                                   0XC8
// lld_big_env/iso_interval
#define OFF_II_IN_LBE_LLD_1502P_T1                                    0XFC
// dbg_bt_common_setting_t2/bt_master_sleep_en
#define OFF_BMSE_IN_DBCST_DBG_1502P_T1                                0X2F
// dbg_bt_common_setting_t2/bt_master_tx_silence_en
#define OFF_BMTSE_IN_DBCST_DBG_1502P_T1                               0X30
// dbg_bt_common_setting_t2/en_sec_con_base_on_le_aud_sup
#define OFF_ESCBOLAS_IN_DBCST_DBG_1502P_T1                            0X2C


#define HCI_FC_ENV_1502P_T1_ADDR                                      0XC0009824
#define LD_ACL_ENV_1502P_T1_ADDR                                      0XC000531C
#define BT_UTIL_BUF_ENV_1502P_T1_ADDR                                 0XC0004C18
#define BLE_UTIL_BUF_ENV_1502P_T1_ADDR                                0XC0005DD0
#define LD_BES_BT_ENV_1502P_T1_ADDR                                   0XC0005BA8
#define DBG_STATE_1502P_T1_ADDR                                       0XC0009A46
#define LC_STATE_1502P_T1_ADDR                                        0XC0005238
#define LD_SCO_ENV_1502P_T1_ADDR                                      0XC0005A74
#define RX_MONITOR_1502P_T1_ADDR                                      0XC000A486
#define LC_ENV_1502P_T1_ADDR                                          0XC0004FC0
#define LM_NB_SYNC_ACTIVE_1502P_T1_ADDR                               0XC0004FAC
#define LM_ENV_1502P_T1_ADDR                                          0XC0004D78
#define HCI_ENV_1502P_T1_ADDR                                         0XC0009740
#define LC_SCO_ENV_1502P_T1_ADDR                                      0XC0004F9C
#define LLM_ENV_1502P_T1_ADDR                                         0XC0005FB4
#define LD_ENV_1502P_T1_ADDR                                          0XC00052A0
#define RWIP_ENV_1502P_T1_ADDR                                        0XC000A530
#define BLE_RX_MONITOR_1502P_T1_ADDR                                  0XC000A413
#define LLC_ENV_1502P_T1_ADDR                                         0XC0008A68
#define RWIP_RF_1502P_T1_ADDR                                         0XC000A4BC
#define LD_ACL_METRICS_1502P_T1_ADDR                                  0XC000532C
#define RF_RX_HWGAIN_TBL_1502P_T1_ADDR                                0XC00049CA
#define RF_HWAGC_RSSI_CORRECT_TBL_1502P_T1_ADDR                       0XC00048E8
#define RF_RX_GAIN_FIXED_TBL_1502P_T1_ADDR                            0XC000A32C
#define HCI_DBG_EBQ_TEST_MODE_1502P_T1_ADDR                           0XC0009A60
#define DBG_BT_COMMON_SETTING_1502P_T1_ADDR                           0XC00098E4
#define DBG_BT_SCHE_SETTING_1502P_T1_ADDR                             0XC00099A4
#define DBG_BT_IBRT_SETTING_1502P_T1_ADDR                             0XC0009988
#define DBG_BT_HW_FEAT_SETTING_1502P_T1_ADDR                          0XC0009964
#define HCI_DBG_SET_SW_RSSI_1502P_T1_ADDR                             0XC0009A90
#define RWIP_PROG_DELAY_1502P_T1_ADDR                                 0XC000A4B8
#define SCH_MULTI_IBRT_ADJUST_ENV_1502P_T1_ADDR                       0XC00096A0
#define RF_HWAGC_RSSI_CORRECT_TBL_1502P_T1_ADDR                       0XC00048E8
#define RF_RX_GAIN_THS_TBL_LE_1502P_T1_ADDR                           0XC000498E
#define RF_RX_GAIN_THS_TBL_LE_2M_1502P_T1_ADDR                        0XC00049AC
#define RF_RPL_TX_PW_CONV_TBL_1502P_T1_ADDR                           0XC00048F8
#define REPLACE_MOBILE_ADDR_1502P_T1_ADDR                             0XC0005244
#define REPLACE_ADDR_VALID_1502P_T1_ADDR                              0XC0005240
#define PCM_NEED_START_FLAG_1502P_T1_ADDR                             0XC0005A90
#define RF_RX_GAIN_THS_TBL_BT_3M_1502P_T1_ADDR                        0XC000A338
#define LD_IBRT_ENV_1502P_T1_ADDR                                     0XC0005D34
#define LLM_LOCAL_LE_FEATS_1502P_T1_ADDR                              0XC0004792
#define RF_RX_GAIN_THS_TBL_BT_1502P_T1_ADDR                           0XC0004934
#define DBG_BT_COMMON_SETTING_T2_1502P_T1_ADDR                        0XC000990A
#define LLD_CON_ENV_1502P_T1_ADDR                                     0XC0008F20
#define LLD_ISO_ENV_1502P_T1_ADDR                                     0XC0008FA0
#define ECC_RX_MONITOR_1502P_T1_ADDR                                  0XC000A43F
#define __STACKLIMIT_1502P_T1_ADDR                                    0XC000B118
#define PER_MONITOR_PARAMS_1502P_T1_ADDR                              0XC0004A7E
#define SENS2BT_EN_1502P_T1_ADDR                                      0XC0004060
#define RF_RX_GAIN_THS_TBL_ECC_1502P_T1_ADDR                          0XC0004970
#define LLD_BIS_ENV_1502P_T1_ADDR                                     0XC000937C
#define LLD_BIG_ENV_1502P_T1_ADDR                                     0XC000936C
#define LLD_CIS_ENV_1502P_T1_ADDR                                     0XC000932C
#define LLD_CIG_ENV_1502P_T1_ADDR                                     0XC000931C
#define RX_MONITOR_3M_RXGAIN_1502P_T1_ADDR                            0XC0004791
#define INTERSYS_MSG_TX_MODE_1502P_T1_ADDR                            0XC0004030
#define CON_LATENCY_APPLY_1502P_T1_ADDR                               0XC00047C0

#endif