#!/bin/bash

TARGET_LIST="best1306p"
CHIPID="best1306p"
COMMAND=$1
set -e

BUILD_CUSTOMER_CFG="BES_OTA=1 OTA_BIN_COMPRESSED=1 BLE=1 SPEECH_TX_POST_GAIN=1 SPEECH_TX_2MIC_NS8=1 SPEECH_TX_EQ=1\
                    CTKD_ENABLE=1 FLASH_SIZE=0x400000\
                    POWER_ON_ENTER_TWS_PAIRING_ENABLED=0 APP_TRACE_RX_ENABLE=0 APP_RX_API_ENABLE=0\
                    BES_LIB_DIR=lib/bes/best1306p/PRO_0016 CHIP_HAS_SPI=1 FREE_TWS_PAIRING_ENABLED=1 CAPSENSOR_ENABLE=1\
                    CAPSENSOR_TRACE_DEBUG=0 CAPSENSOR_SPP_SERVER=1 CAPSENSOR_ENABLE=1 CAPSENSOR_TOUCH=1 CAPSENSOR_WEAR=0 CAPSENSOR_SLIDE=0\
                    GATT_OVER_BR_EDR=1\
                    A2DP_LHDCV5_ON=1 A2DP_LDAC_ON=1\
                    ANC_ENABLE=1 TRACE_BUF_SIZE=32*1024\
                    LEA_ENABLE=0 AOB_CODEC_CP=0 LC3_IN_ROM_V2=0\
                    A2DP_LHDC_ON=1 A2DP_LHDC_V3=1\
                    ANC_ASSIST_ENABLED=1"

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