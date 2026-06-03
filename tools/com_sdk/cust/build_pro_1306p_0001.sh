#!/bin/bash

TARGET_LIST="best1306p"
CHIPID="best1306p"
COMMAND=$1
set -e

BUILD_CUSTOMER_CFG="BES_OTA=1 OTA_BIN_COMPRESSED=1 ANC_ENABLE=1 ANC_ASSIST_ENABLE=1 BLE=1 LEA_ENABLE=1 AOB_CODEC_CP=1 LC3_IN_ROM_V2=1 \
                    SPEECH_TX_DC_FILTER=1 SPEECH_TX_3MIC_NS=1 SPEECH_TX_EQ=1 \
                    POWER_ON_ENTER_TWS_PAIRING_ENABLED=1 APP_TRACE_RX_ENABLE=1 APP_RX_API_ENABLE=1 BES_LIB_DIR=lib/bes/best1306p/PRO_0001 \
                    A2DP_AAC_ON=1 AAC_IN_ROM=1 CVSD_IN_ROM=1 A2DP_VIRTUAL_SURROUND=1 HEAD_TRACK_ENABLE=1 CUSTOM_BITRATE=1"

if [[ "$COMMAND" == "clean" ]];
then
    make T=$TARGET_LIST clean ||{ echo "$LINENO command failed"; exit 1; }
elif [[ "$COMMAND" == "gen_lib" ]];
then
    make T=$TARGET_LIST $BUILD_CUSTOMER_CFG -j64 GEN_LIB=1 ||{ echo "$LINENO command failed"; exit 1; }
    make T=prod_test/ota_copy CHIP=$CHIPID OTA_BIN_COMPRESSED=1 DEBUG=1 -j64 GEN_LIB=1 ||{ echo "$LINENO command failed"; exit 1; }
elif [[ "$COMMAND" == 'lst' ]];
then
    make T=$TARGET_LIST $BUILD_CUSTOMER_CFG -j64 lst all ||{ echo "$LINENO command failed"; exit 1; }
else
    make T=$TARGET_LIST $BUILD_CUSTOMER_CFG -j64 ||{ echo "$LINENO command failed"; exit 1; }
    make T=prod_test/ota_copy CHIP=$CHIPID OTA_BIN_COMPRESSED=1 DEBUG=1 -j64 ||{ echo "$LINENO command failed"; exit 1; }
fi
echo $BUILD_CUSTOMER_CFG