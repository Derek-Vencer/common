
/*******************************************************************************************************************************/
/*********************************************bt controller symbol**************************************************************/

#ifndef  __BT_CONTROLLER_SYMBOL_H__
#define  __BT_CONTROLLER_SYMBOL_H__


#define HCI_FC_ENV_ADDR                                               0XC00082D8
#define LD_ACL_ENV_ADDR                                               0XC0000698
#define BT_UTIL_BUF_ENV_ADDR                                          0XC000085C
#define BLE_UTIL_BUF_ENV_ADDR                                         0XC0006F14
#define LD_BES_BT_ENV_ADDR                                            0XC0001704
#define DBG_STATE_ADDR                                                0XC0005D7C
#define LC_STATE_ADDR                                                 0XC0000B64
#define LD_SCO_ENV_ADDR                                               0XC00006B0
#define RX_MONITOR_ADDR                                               0XC000682C
#define LC_ENV_ADDR                                                   0XC0000B54
#define LM_NB_SYNC_ACTIVE_ADDR                                        0XC00003CE
#define LM_ENV_ADDR                                                   0XC00001AC
#define LM_KEY_ENV_ADDR                                               0XC0000304
#define HCI_ENV_ADDR                                                  0XC0008208
#define LC_SCO_ENV_ADDR                                               0XC00003B0
#define LLM_ENV_ADDR                                                  0XC0007304
#define LD_ENV_ADDR                                                   0XC0000C38
#define RWIP_ENV_ADDR                                                 0XC0005B3C
#define BLE_RX_MONITOR_ADDR                                           0XC00067DE
#define LLC_ENV_ADDR                                                  0XC0007D28
#define RWIP_RF_ADDR                                                  0XC0005AA4
#define LD_ACL_METRICS_ADDR                                           0XC00003E8
#define RF_RX_HWGAIN_TBL_ADDR                                         0XC00043F6
#define RF_HWAGC_RSSI_CORRECT_TBL_ADDR                                0XC000436E
#define RF_RX_GAIN_FIXED_TBL_ADDR                                     0XC00067D2
#define HCI_DBG_EBQ_TEST_MODE_ADDR                                    0XC0005E54
#define HOST_REF_CLK_ADDR                                             0XC0005A5C
#define DBG_TRC_TL_ENV_ADDR                                           0XC0005F18
#define DBG_TRC_MEM_ENV_ADDR                                          0XC0005F24
#define DBG_BT_COMMON_SETTING_ADDR                                    0XC0005DE0
#define DBG_BT_SCHE_SETTING_ADDR                                      0XC0005E84
#define DBG_BT_IBRT_SETTING_ADDR                                      0XC0005D0A
#define DBG_BT_HW_FEAT_SETTING_ADDR                                   0XC0005DAC
#define HCI_DBG_SET_SW_RSSI_ADDR                                      0XC0005EAC
#define LP_CLK_ADDR                                                   0XC0005B38
#define RWIP_PROG_DELAY_ADDR                                          0XC0005B30
#define DATA_BACKUP_CNT_ADDR                                          0XC0004060
#define DATA_BACKUP_ADDR_PTR_ADDR                                     0XC0004064
#define DATA_BACKUP_VAL_PTR_ADDR                                      0XC0004068
#define SCH_MULTI_IBRT_ADJUST_ENV_ADDR                                0XC0008678
#define RF_HWAGC_RSSI_CORRECT_TBL_ADDR                                0XC000436E
#define RF_RX_GAIN_THS_TBL_LE_ADDR                                    0XC00043D8
#define REPLACE_MOBILE_ADDR_ADDR                                      0XC0000B90
#define REPLACE_ADDR_VALID_ADDR                                       0XC00000B8
#define PCM_NEED_START_FLAG_ADDR                                      0XC00000AC
#define I2V_VAL_ADDR                                                  0XC000009C
#define I2V_THD_ADDR                                                  0XC0000046
#define RT_SLEEP_FLAG_CLEAR_ADDR                                      0XC0005A8C
#define RF_RX_GAIN_THS_TBL_BT_3M_ADDR                                 0XC00067B4
#define TESTMODE_3M_FLAG_ADDR                                         0XC0005A80
#define TESTMODE_3M_EN_ADDR                                           0XC0005A7F
#define TESTMODE_1M_FLAG_ADDR                                         0XC0005A7E
#define TESTMODE_1M_EN_ADDR                                           0XC0005A7D
#define NORMAL_IQTAB_ADDR                                             0XC00067FC
#define NORMAL_IQTAB_EN_ADDR                                          0XC0005A78
#define POWER_ADJUST_EN_ADDR                                          0XC0005A7C
#define LD_IBRT_ENV_ADDR                                              0XC0001718
#define LLM_LOCAL_LE_FEATS_ADDR                                       0XC0004470
#define ISOOHCI_ENV_ADDR                                              0XC00086B4
#define RF_RX_GAIN_THS_TBL_BT_ADDR                                    0XC00043BA
#define LLD_CON_ENV_ADDR                                              0XC0008128
#define ISOOHCI_ENV_ADDR                                              0XC00086B4
#define I2V_VAL_ADDR                                                  0XC000009C
#define RF_RX_GAIN_THS_TBL_BT_3M_ADDR                                 0XC00067B4
#define TESTMODE_3M_FLAG_ADDR                                         0XC0005A80
#define NORMAL_IQTAB_ADDR                                             0XC00067FC
#define POWER_ADJUST_EN_ADDR                                          0XC0005A7C
#define LLD_ISO_ENV_ADDR                                              0XC0008154
#define MASTER_CON_SUPPORT_LE_AUDIO_ADDR                              0XC00000B6
#define DBG_BT_COMMON_SETTING_T2_ADDR                                 0XC0005E2C
#define RX_RECORD_ADDR                                                0XC00006FC
#define RF_RPL_TX_PW_CONV_TBL_ADDR                                    0XC000437E
#define HCI_DBG_BLE_AUDIO_ADDR                                        0XC0005E48

//commit 5ae5f70a628544949c6f9e0cde34041c58588c52
//Author: Leo <junhongliu@bestechnic.com>
//Date:   Thu Jan 4 11:08:09 2024 +0800
//    [sys]:set system to 48M when boot up
//    
//    Change-Id: Iedce58a11c3cc6836bfb1648dc9cda6f18f537c9

#ifndef  BT_CONTROLLER_COMMIT_ID
#define  BT_CONTROLLER_COMMIT_ID                    "commit 5ae5f70a628544949c6f9e0cde34041c58588c52"
#endif
#ifndef  BT_CONTROLLER_COMMIT_AUTHOR
#define  BT_CONTROLLER_COMMIT_AUTHOR                "Author: Leo <junhongliu@bestechnic.com>"
#endif
#ifndef  BT_CONTROLLER_COMMIT_DATE
#define  BT_CONTROLLER_COMMIT_DATE                  "Date:   Thu Jan 4 11:08:09 2024 +0800"
#endif
#ifndef  BT_CONTROLLER_COMMIT_MESSAGE
#define  BT_CONTROLLER_COMMIT_MESSAGE               "    [sys]:set system to 48M when boot up          Change-Id: Iedce58a11c3cc6836bfb1648dc9cda6f18f537c9 "
#endif

#endif
