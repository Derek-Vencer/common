#!/bin/bash
TARGET_LIST="best1502p"
CHIPID="best1502p"
COMMAND=$1
BES_LIB_DIR="lib/bes/best1502p/PRO_0001"
set -e

################################### var define ###################################
BES_LIB="BES_LIB_DIR=$BES_LIB_DIR"

APP_CUSTOMER_CFG="IGNORE_POWER_ON_KEY_DURING_BOOT_UP=1 IS_AUTOPOWEROFF_ENABLED=0 APP_TRACE_RX_ENABLE=1 APP_RX_API_ENABLE=1 AUDIO_DEBUG_CMD=0 \
                TRACE_BUF_SIZE=40*1024 USE_TRACE_ID=0 SENSOR_HUB=1 SENS_TRC_TO_MCU=1"

BLE_CFG="BLE=1 GATT_OVER_BR_EDR=1"

OTA_CFG="BES_OTA=1 OTA_BIN_COMPRESSED=1"

TWS_CFG="POWER_ON_ENTER_TWS_PAIRING_ENABLED=1"

SPEECH_CFG="SPEECH_TX_DC_FILTER=1 SPEECH_TX_AEC2FLOAT=1 SPEECH_TX_1MIC_NS=1 SPEECH_TX_EQ=1 SPEECH_ALGO_DSP=SENS"

SPEECH_2MIC_CFG="SPEECH_TX_DC_FILTER=1 SPEECH_TX_2MIC_NS8=1 SPEECH_TX_EQ=0"

DECODE_DECODER_CFG="A2DP_AAC_ON=1 A2DP_LDAC_ON=0 A2DP_LHDC_ON=0 A2DP_LHDC_V3=0 A2DP_LHDCV5_ON=0 AAC_IN_ROM=1 CVSD_IN_ROM=1"

CP_CFG="A2DP_CP_ACCEL=1 SCO_CP_ACCEL=1 CHIP_HAS_CP=1 RAMCP_SIZE=0x4000 RAMCPX_SIZE=0x10000 FAST_XRAM_SECTION_SIZE=0x6000 UNIFY_HEAP_ENABLED=0"

SYS_CFG=" ALLOW_WARNING=1"

SMART_VOICE="SMART_VOICE=1 NN_KWS=1"

SENSOR_PARAMS="SENS_TRC_TO_MCU=1 SPEECH_ALGO_DSP=thirdparty"

################################### end of var define ###################################


build_app_cmd="make T=$TARGET_LIST $APP_CUSTOMER_CFG $BLE_CFG $OTA_CFG $TWS_CFG $SPEECH_CFG $DECODE_DECODER_CFG $CP_CFG $SYS_CFG $SMART_VOICE $BES_LIB -j64"
build_ota_copy_cmd="make T=prod_test/ota_copy CHIP=$CHIPID DEBUG=1 -j64"

build_sensor_cmd="make T=sensor_hub CHIP=$CHIPID $SENSOR_PARAMS $BES_LIB -j64"

if [[ "$COMMAND" == "clean" ]];
then
    $build_sensor_cmd clean
    $build_app_cmd clean
    echo "clean end"
elif [[ "$COMMAND" == "gen_lib" ]];
then
    $build_sensor_cmd GEN_LIB=1
    $build_app_cmd GEN_LIB=1
    $build_ota_copy_cmd
elif [[ "$COMMAND" == 'lst' ]];
then
    $build_sensor_cmd lst all
    $build_app_cmd lst all
    echo "lst build end"
else
    $build_ota_copy_cmd ||{ echo "$LINENO command failed"; exit 1; }

    echo " "
    $build_sensor_cmd ||{ echo "$LINENO command failed"; exit 1; }
    echo "build cmd: "
    echo "$build_app_cmd"
    echo "normal build"
    echo " "

    $build_app_cmd ||{ echo "$LINENO command failed"; exit 1; }
    echo "build cmd: "
    echo "$build_app_cmd"
    echo "normal build"
    echo " "
fi

