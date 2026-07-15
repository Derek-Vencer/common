#!/bin/bash

TARGET_LIST="best1503"
CHIPID="best1503"
COMMAND=$1
set -e

BUILD_CUSTOMER_CFG="A2DP_LHDCV5_ON=1 A2DP_LHDC_ON=1 A2DP_LHDC_V3=1 A2DP_LC3_ON=1 A2DP_LDAC_ON=1 BES_OTA=1 \
                 CAPSENSOR_ENABLE=1 LEA_ENABLE=1 ANC_ENABLE=1 SPEECH_TX_2MIC_NS8=1 ANC_ASSIST_ENABLE=1 \
                 APP_ANC_TEST=1 BLE=1 CTKD_ENABLE=1 BLE_AUDIO_ENABLED=1 BT_SERVICE_ENABLE=1 VOICE_ASSIST_ONESHOT_ADAPTIVE_ANC=1\
                 BT_SVC_MODULE_LEA_ENABLED=1 RAMCP_SIZE=0x80000 GATT_OVER_BR_EDR=1 BES_OTA=1 BES_LIB_DIR=lib/bes/best1503/PRO_0003\
                 OTA_BIN_COMPRESSED=1 FLASH_SIZE=0x400000 ANC_ASSIST_ENABLED=1\
                 HUOSHAN_ENABLE=1 -j256"

if [[ "$COMMAND" == "clean" ]];
then
    make T=$TARGET_LIST clean ||{ echo "$LINENO command failed"; exit 1; }
elif [[ "$COMMAND" == "gen_lib" ]];
then
    make T=$TARGET_LIST $BUILD_CUSTOMER_CFG -j64 GEN_LIB=1 ||{ echo "$LINENO command failed"; exit 1; }
    make T=prod_test/ota_copy CHIP=$CHIPID OTA_BIN_COMPRESSED=1 BES_LIB_DIR=lib/bes/best1503/PRO_0003 DEBUG=1 -j64 GEN_LIB=1 ||{ echo "$LINENO command failed"; exit 1; }
elif [[ "$COMMAND" == 'lst' ]];
then
    make T=$TARGET_LIST $BUILD_CUSTOMER_CFG -j64 lst all ||{ echo "$LINENO command failed"; exit 1; }
else
    make T=$TARGET_LIST $BUILD_CUSTOMER_CFG -j64 ||{ echo "$LINENO command failed"; exit 1; }
    make T=prod_test/ota_copy CHIP=$CHIPID OTA_BIN_COMPRESSED=1 BES_LIB_DIR=lib/bes/best1503/PRO_0003 DEBUG=1 -j64 ||{ echo "$LINENO command failed"; exit 1; }
fi
echo $BUILD_CUSTOMER_CFG