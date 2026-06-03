#!/bin/bash

TARGET_LIST="best1306p"
CHIPID="best1306p"
COMMAND=$1
set -e

APP_CUSTOMER_CFG="IGNORE_POWER_ON_KEY_DURING_BOOT_UP=1 IS_AUTOPOWEROFF_ENABLED=0 APP_TRACE_RX_ENABLE=1 APP_RX_API_ENABLE=1 AUDIO_DEBUG_CMD=0 \
                 TRACE_BUF_SIZE=40*1024 USE_TRACE_ID=0 SENS_TRC_TO_MCU=1 CAPSENSOR_ENABLE=1"

BLE_CFG="BLE=1 GATT_OVER_BR_EDR=1 GFPS_ENABLE=1 BT_SVC_MODULE_TWS_BLE_SEAMLESS_SWITCH=1 APP_BLE_DEMO_APP_ENABLED=1"

OTA_CFG="BES_OTA=1 OTA_BIN_COMPRESSED=1"

TWS_CFG="POWER_ON_ENTER_TWS_PAIRING_ENABLED=1 FREE_TWS_PAIRING_ENABLED=0"

SPEECH_3MIC_CFG="SPEECH_TX_DC_FILTER=1 SPEECH_TX_3MIC_NS=1 SPEECH_TX_EQ=1 SPEECH_RX_EQ=1 SPEECH_RX_NS2FLOAT=1"

ANC_CFG="ANC_ENABLE=1 ANC_ASSIST_ENABLED=1 ANC_PROD_TEST=1 TOTA_v2=1"

DECODE_DECODER_CFG="A2DP_AAC_ON=1 A2DP_LDAC_ON=1 AAC_IN_ROM=1 CVSD_IN_ROM=1"

AUDIO_CFG="AUDIO_DRC=1"

CP_CFG="A2DP_CP_ACCEL=1 SCO_CP_ACCEL=1 CHIP_HAS_CP=1 RAMCP_SIZE=0x6000 RAMCPX_SIZE=0x10000 FAST_XRAM_SECTION_SIZE=0x6000 UNIFY_HEAP_ENABLED=0"

SYS_CFG="ALLOW_WARNING=1 BES_LIB_DIR=lib/bes/best1306p/PRO_0013"

build_ota_copy_cmd="make T=prod_test/ota_copy -j64 CHIP=best1306p OTA_BIN_COMPRESSED=1 SINGLE_WIRE_DOWNLOAD=1 DEBUG=1 BES_LIB_DIR=lib/bes/best1306p/PRO_0013"
build_app_cmd="make T=best1306p -j64 DEBUG=1"

if [[ "$COMMAND" == "clean" ]];
then
    $build_app_cmd clean
    $build_ota_copy_cmd clean
    echo "clean end"
elif [[ "$COMMAND" == "gen_lib" ]];
then
    $build_app_cmd $APP_CUSTOMER_CFG $BLE_CFG $OTA_CFG $TWS_CFG $SPEECH_3MIC_CFG $ANC_CFG $DECODE_DECODER_CFG $CP_CFG $SYS_CFG GEN_LIB=1 ||{ echo "$LINENO command failed"; exit 1; }
    $build_ota_copy_cmd GEN_LIB=1 ||{ echo "$LINENO command failed"; exit 1; }
elif [[ "$COMMAND" == 'lst' ]];
then
    $build_app_cmd $APP_CUSTOMER_CFG $BLE_CFG $OTA_CFG $TWS_CFG $SPEECH_3MIC_CFG $ANC_CFG $DECODE_DECODER_CFG $CP_CFG $SYS_CFG lst all ||{ echo "$LINENO command failed"; exit 1; }
    $build_ota_copy_cmd lst all ||{ echo "$LINENO command failed"; exit 1; }
    echo "lst build end"
else
    $build_app_cmd $APP_CUSTOMER_CFG $BLE_CFG $OTA_CFG $TWS_CFG $SPEECH_3MIC_CFG $ANC_CFG $DECODE_DECODER_CFG $CP_CFG $SYS_CFG ||{ echo "$LINENO command failed"; exit 1; }
    $build_ota_copy_cmd ||{ echo "$LINENO command failed"; exit 1; }
    echo "normal build"
    echo " "
fi