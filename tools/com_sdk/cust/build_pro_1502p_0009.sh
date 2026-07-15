#!/bin/bash

TARGET_LIST="best1502p"
CHIPID="best1502p"
COMMAND=$1
BUILD_VARIANT=$2
set -e

BUILD_CUSTOMER_CFG="BES_OTA=1 OTA_BIN_COMPRESSED=1 BLE=1 FLASH_SIZE=0x400000\
                    POWER_ON_ENTER_TWS_PAIRING_ENABLED=0 APP_TRACE_RX_ENABLE=1 APP_RX_API_ENABLE=1\
                    BES_LIB_DIR=lib/bes/best1502p/PRO_0009 FREE_TWS_PAIRING_ENABLED=1\
                    CTKD_ENABLE=1 IS_CTKD_OVER_BR_EDR_ENABLED=1 BT_SVC_MODULE_TWS_BLE_SEAMLESS_SWITCH=1 SPEECH_TX_2MIC_NS8=1\
                    CLEAN_OPUS=1 NN_KWS=1"

if [[ "$COMMAND" == "clean" ]];
then
    make T=$TARGET_LIST clean ||{ echo "$LINENO command failed"; exit 1; }
elif [[ "$COMMAND" == "gen_lib" ]];
then
    make T=$TARGET_LIST $BUILD_CUSTOMER_CFG -j64 GEN_LIB=1 ||{ echo "$LINENO command failed"; exit 1; }
    make T=$TARGET_LIST $BUILD_CUSTOMER_CFG GFPS_ENABLE=1 -j64 GEN_LIB=1 ||{ echo "$LINENO command failed"; exit 1; }
    make T=$TARGET_LIST $BUILD_CUSTOMER_CFG GATT_OVER_BR_EDR=1 -j64 GEN_LIB=1 ||{ echo "$LINENO command failed"; exit 1; }
    make T=$TARGET_LIST $BUILD_CUSTOMER_CFG GATT_OVER_BR_EDR=1 GFPS_ENABLE=1 -j64 GEN_LIB=1 ||{ echo "$LINENO command failed"; exit 1; }
    make T=prod_test/ota_copy CHIP=$CHIPID OTA_BIN_COMPRESSED=1 DEBUG=1 -j64 GEN_LIB=1 ||{ echo "$LINENO command failed"; exit 1; }
elif [[ "$COMMAND" == "lst" ]]; 
then
    make T=$TARGET_LIST $BUILD_CUSTOMER_CFG -j64 lst all || { echo "$LINENO command failed"; exit 1; }
elif [[ "$COMMAND" == 'gfps' ]]; 
then
    if [[ "$BUILD_VARIANT" == "lst" ]]; 
    then
        make T=$TARGET_LIST $BUILD_CUSTOMER_CFG GFPS_ENABLE=1 -j64 lst all || { echo "$LINENO command failed"; exit 1; }
    else
        make T=$TARGET_LIST $BUILD_CUSTOMER_CFG GFPS_ENABLE=1 -j64 || { echo "$LINENO command failed"; exit 1; }
        make T=prod_test/ota_copy CHIP=$CHIPID OTA_BIN_COMPRESSED=1 DEBUG=1 -j64 ||{ echo "$LINENO command failed"; exit 1; }
    fi 
elif [[ "$COMMAND" == 'gfps_gatt' ]]; 
then
    if [[ "$BUILD_VARIANT" == "lst" ]]; 
    then
        make T=$TARGET_LIST $BUILD_CUSTOMER_CFG GATT_OVER_BR_EDR=1 GFPS_ENABLE=1 -j64 lst all || { echo "$LINENO command failed"; exit 1; }
    else 
        make T=$TARGET_LIST $BUILD_CUSTOMER_CFG GATT_OVER_BR_EDR=1 GFPS_ENABLE=1 -j64 || { echo "$LINENO command failed"; exit 1; }
        make T=prod_test/ota_copy CHIP=$CHIPID OTA_BIN_COMPRESSED=1 DEBUG=1 -j64 ||{ echo "$LINENO command failed"; exit 1; }
    fi
elif [[ "$COMMAND" == 'gatt' ]]; 
then
    if [[ "$BUILD_VARIANT" == "lst" ]]; 
    then
        make T=$TARGET_LIST $BUILD_CUSTOMER_CFG GATT_OVER_BR_EDR=1 -j64 lst all || { echo "$LINENO command failed"; exit 1; }
    else
        make T=$TARGET_LIST $BUILD_CUSTOMER_CFG GATT_OVER_BR_EDR=1 -j64 || { echo "$LINENO command failed"; exit 1; }
        make T=prod_test/ota_copy CHIP=$CHIPID OTA_BIN_COMPRESSED=1 DEBUG=1 -j64 ||{ echo "$LINENO command failed"; exit 1; }
    fi 
else
    make T=$TARGET_LIST $BUILD_CUSTOMER_CFG -j64 ||{ echo "$LINENO command failed"; exit 1; }
    make T=prod_test/ota_copy CHIP=$CHIPID OTA_BIN_COMPRESSED=1 DEBUG=1 -j64 ||{ echo "$LINENO command failed"; exit 1; }
fi