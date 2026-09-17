#!/bin/bash

TARGET_LIST="best1306p"
CHIPID="best1306p"
COMMAND=$1

OTA_CFG="BES_OTA=1 OTA_BIN_COMPRESSED=1"
ENC_CFG="SPEECH_TX_1MIC_NS=1 SPEECH_TX_AEC2FLOAT=1 SPEECH_TX_COMPEXP=1 SPEECH_TX_POST_GAIN=1 SPEECH_RX_EQ=1 SPEECH_RX_POST_GAIN=1"
GFPS_CFG="BLE=1 GFPS_ENABLE=1 BLE_BATT=1"
ANC_CFG="ANC_ENABLE=1"
LIB_CFG="BES_LIB_DIR=lib/bes/best1306p/PRO_0015"
set -e

BUILD_CUSTOMER_CFG="$OTA_CFG $ENC_CFG $GFPS_CFG $ANC_CFG $LIB_CFG \
                    POWER_ON_ENTER_TWS_PAIRING_ENABLED=1 CUSTOM_BITRATE=1 \
                    AUDIO_DRC=1 AUDIO_LIMITER=0 AUDIO_DEBUG=0 USER_SPEECH_DUMP_EN=1 AUDIO_DEBUG_CMD=0 TRACE_BUF_SIZE=40*1024"

if [[ "$COMMAND" == "clean" ]];
then
    make T=$TARGET_LIST clean ||{ echo "$LINENO command failed"; exit 1; }
elif [[ "$COMMAND" == "gen_lib" ]];
then
    make T=$TARGET_LIST $BUILD_CUSTOMER_CFG -j64 GEN_LIB=1 ||{ echo "$LINENO command failed"; exit 1; }
    make T=prod_test/ota_copy CHIP=$CHIPID OTA_BIN_COMPRESSED=1 DEBUG=1 -j64 GEN_LIB=1 $LIB_CFG ||{ echo "$LINENO command failed"; exit 1; }
elif [[ "$COMMAND" == 'lst' ]];
then
    make T=$TARGET_LIST $BUILD_CUSTOMER_CFG -j64 lst all ||{ echo "$LINENO command failed"; exit 1; }
else
    make T=$TARGET_LIST $BUILD_CUSTOMER_CFG -j64 ||{ echo "$LINENO command failed"; exit 1; }
    make T=prod_test/ota_copy CHIP=$CHIPID OTA_BIN_COMPRESSED=1 DEBUG=1 $LIB_CFG -j64 ||{ echo "$LINENO command failed"; exit 1; }
fi
echo $BUILD_CUSTOMER_CFG
