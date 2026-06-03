
/*******************************************************************************************************************************/
/*********************************************bt controller symbol**************************************************************/

#ifndef  __BT_CONTROLLER_SYMBOL_H__
#define  __BT_CONTROLLER_SYMBOL_H__


#define HCI_FC_ENV_ADDR                                               0XC00074B4
#define LD_ACL_ENV_ADDR                                               0XC0005108
#define BT_UTIL_BUF_ENV_ADDR                                          0XC0004600
#define BLE_UTIL_BUF_ENV_ADDR                                         0XC000545C
#define LD_BES_BT_ENV_ADDR                                            0XC00052A4
#define DBG_STATE_ADDR                                                0XC00075A8
#define LC_STATE_ADDR                                                 0XC0004C70
#define LD_SCO_ENV_ADDR                                               0XC0005124
#define RX_MONITOR_ADDR                                               0XC0008008
#define LC_ENV_ADDR                                                   0XC0004C5C
#define LM_NB_SYNC_ACTIVE_ADDR                                        0XC0004C56
#define LM_ENV_ADDR                                                   0XC0004904
#define LM_KEY_ENV_ADDR                                               0XC0004A74
#define HCI_ENV_ADDR                                                  0XC00073D4
#define LC_SCO_ENV_ADDR                                               0XC0004C38
#define LLM_ENV_ADDR                                                  0XC0005980
#define LD_ENV_ADDR                                                   0XC0004D7C
#define RWIP_ENV_ADDR                                                 0XC000817C
#define BLE_RX_MONITOR_ADDR                                           0XC0007FB0
#define LLC_ENV_ADDR                                                  0XC0006838
#define RWIP_RF_ADDR                                                  0XC0008114
#define LD_ACL_METRICS_ADDR                                           0XC0004DC0
#define RF_RX_HWGAIN_TBL_ADDR                                         0XC00043AA
#define RF_RX_GAIN_FIXED_TBL_ADDR                                     0XC0007F8A
#define HCI_DBG_EBQ_TEST_MODE_ADDR                                    0XC00076A4
#define DBG_BT_COMMON_SETTING_ADDR                                    0XC0007604
#define DBG_BT_SCHE_SETTING_ADDR                                      0XC00076D4
#define DBG_BT_IBRT_SETTING_ADDR                                      0XC000752E
#define DBG_BT_HW_FEAT_SETTING_ADDR                                   0XC00075D8
#define HCI_DBG_SET_SW_RSSI_ADDR                                      0XC00076FC
#define LP_CLK_ADDR                                                   0XC0008178
#define RWIP_PROG_DELAY_ADDR                                          0XC0008175
#define DATA_BACKUP_CNT_ADDR                                          0XC0004080
#define DATA_BACKUP_ADDR_PTR_ADDR                                     0XC0004084
#define DATA_BACKUP_VAL_PTR_ADDR                                      0XC0004088
#define SCH_MULTI_IBRT_ADJUST_ENV_ADDR                                0XC0007208
#define RF_RX_GAIN_THS_TBL_LE_ADDR                                    0XC000436E
#define RF_RX_GAIN_THS_TBL_LE_2M_ADDR                                 0XC000438C
#define RF_RPL_TX_PW_CONV_TBL_ADDR                                    0XC00042D8
#define REPLACE_MOBILE_ADDR_ADDR                                      0XC0004CD0
#define REPLACE_ADDR_VALID_ADDR                                       0XC0004586
#define PCM_NEED_START_FLAG_ADDR                                      0XC00045C0
#define RT_SLEEP_FLAG_CLEAR_ADDR                                      0XC00045F0
#define RF_RX_GAIN_THS_TBL_BT_3M_ADDR                                 0XC0007F6C
#define NORMAL_IQTAB_ADDR                                             0XC0007FD8
#define NORMAL_IQTAB_EN_ADDR                                          0XC00045D8
#define POWER_ADJUST_EN_ADDR                                          0XC00045DC
#define LD_IBRT_ENV_ADDR                                              0XC00053D8
#define LLM_LOCAL_LE_FEATS_ADDR                                       0XC0004290
#define ISOOHCI_ENV_ADDR                                              0XC000724C
#define RF_RX_GAIN_THS_TBL_BT_ADDR                                    0XC0004314
#define DBG_BT_COMMON_SETTING_T2_ADDR                                 0XC0007652
#define LLD_CON_ENV_ADDR                                              0XC0006CAC
#define POWER_ADJUST_EN_ADDR                                          0XC00045DC
#define LLD_ISO_ENV_ADDR                                              0XC0006CEC
#define ECC_RX_MONITOR_ADDR                                           0XC0007F64
#define __STACKLIMIT_ADDR                                             0XC0008D80
#define PER_MONITOR_PARAMS_ADDR                                       0XC0004282
#define TX_POWER_VAL_BKUP_ADDR                                        0XC00080C4
#define DBG_ENV_ADDR                                                  0XC000776C
#define RF_RX_GAIN_THS_TBL_ECC_ADDR                                   0XC0004350
#define MASTER_CON_SUPPORT_LE_AUDIO_ADDR                              0XC0004584
#define DBG_BT_COMMON_SETTING_T2_ADDR                                 0XC0007652
#define RX_RECORD_ADDR                                                0XC0005168
#define SCH_PROG_DBG_ENV_ADDR                                         0XC0006F2C
#define LLD_PER_ADV_ENV_ADDR                                          0XC0006C14
#define HCI_DBG_BLE_ADDR                                              0XC0007594
#define LLD_ENV_ADDR                                                  0XC0006B00
#define RWIP_RST_STATE_ADDR                                           0XC0008174
#define SENS2BT_EN_ADDR                                               0XC00045EC

//commit 2522ad63a45f8c7b09dcc77136f42d6661d4135d
//Author: duoli <liduo@bestechnic.com>
//Date:   Wed Oct 9 17:09:10 2024 +0800
//    [LLC]report unknown con id when host send ibrt sw cmd after disc
//    
//    Change-Id: Iae879074b01c0e275cb83c01b23856b499d1580b

#ifndef  BT_CONTROLLER_COMMIT_ID
#define  BT_CONTROLLER_COMMIT_ID                            "commit 2522ad63a45f8c7b09dcc77136f42d6661d4135d"
#endif
#ifndef  BT_CONTROLLER_COMMIT_AUTHOR
#define  BT_CONTROLLER_COMMIT_AUTHOR                        "Author: duoli <liduo@bestechnic.com>"
#endif
#ifndef  BT_CONTROLLER_COMMIT_DATE
#define  BT_CONTROLLER_COMMIT_DATE                          "Date:   Wed Oct 9 17:09:10 2024 +0800"
#endif
#ifndef  BT_CONTROLLER_COMMIT_MESSAGE
#define  BT_CONTROLLER_COMMIT_MESSAGE                       "[LLC]report unknown con id when host send ibrt sw cmd after disc  Change-Id: Iae879074b01c0e275cb83c01b23856b499d1580b "
#endif
#ifndef  BT_CONTROLLER_BUILD_TIME
#define  BT_CONTROLLER_BUILD_TIME                           "2024-10-12 13:39:37"
#endif
#ifndef  BT_CONTROLLER_BUILD_OWNER
#define  BT_CONTROLLER_BUILD_OWNER                          "jiansong@bestechnic.com"
#endif

#endif
