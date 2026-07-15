#!/bin/bash

TARGET_LIST="best1502p"
CHIPID="best1502p"
COMMAND=$1
set -e

APP_CUSTOMER_CFG="IGNORE_POWER_ON_KEY_DURING_BOOT_UP=1 IS_AUTOPOWEROFF_ENABLED=0 APP_TRACE_RX_ENABLE=1 APP_RX_API_ENABLE=1 AUDIO_DEBUG_CMD=0 \
                 TRACE_BUF_SIZE=40*1024 USE_TRACE_ID=0 SENSOR_HUB=1 SENS_TRC_TO_MCU=1 CAPSENSOR_ENABLE=1 INBAND_RINGTONE_PREEMPTION_ENABLED=1"

BLE_CFG="BLE=1 GATT_OVER_BR_EDR=1 CTKD_ENABLE=1 IS_CTKD_OVER_BR_EDR_ENABLED=1 BT_SVC_MODULE_TWS_BLE_SEAMLESS_SWITCH=1"

OTA_CFG="BES_OTA=1 OTA_BIN_COMPRESSED=1"

TWS_CFG="POWER_ON_ENTER_TWS_PAIRING_ENABLED=0 FREE_TWS_PAIRING_ENABLED=1"

SPEECH_2MIC_CFG="SPEECH_TX_DC_FILTER=1 SPEECH_TX_2MIC_NS8=1 SPEECH_TX_EQ=1 SPEECH_RX_EQ=1 SPEECH_RX_NS2FLOAT=1"

DECODE_DECODER_CFG="A2DP_AAC_ON=1 A2DP_LDAC_ON=0 AAC_IN_ROM=1 CVSD_IN_ROM=1"

AUDIO_CFG="AUDIO_DYNAMIC_EQ=1 A2DP_VIRTUAL_SURROUND=1 HEAD_TRACK_ENABLE=1 DUAL_MIC_RECORDING=1 RECORDING_USE_OPUS=1 SINGLE_DEVICE_REC=1"

CP_CFG="A2DP_CP_ACCEL=1 SCO_CP_ACCEL=1 CHIP_HAS_CP=1 RAMCP_SIZE=0x6000 RAMCPX_SIZE=0x10000 FAST_XRAM_SECTION_SIZE=0x6000 UNIFY_HEAP_ENABLED=0"

SYS_CFG=" ALLOW_WARNING=1 BES_LIB_DIR=lib/bes/best1502p/PRO_0003"

build_sensorhub_cmd="make T=sensor_hub CHIP=best1502p SENS_TRC_TO_MCU=1 -j64"
build_ota_copy_cmd="make T=prod_test/ota_copy -j64 CHIP=best1502p OTA_BIN_COMPRESSED=1 SINGLE_WIRE_DOWNLOAD=1 DEBUG=1 BES_LIB_DIR=lib/bes/best1502p/PRO_0003"
build_app_cmd="make T=best1502p -j64 DEBUG=1"

if [[ "$COMMAND" == "clean" ]];
then
    $build_sensorhub_cmd clean
    $build_app_cmd clean
    $build_ota_copy_cmd clean
    echo "clean end"
elif [[ "$COMMAND" == "gen_lib" ]];
then
    $build_sensorhub_cmd GEN_LIB=1
    $build_app_cmd $APP_CUSTOMER_CFG $BLE_CFG $OTA_CFG $TWS_CFG $SPEECH_2MIC_CFG $DECODE_DECODER_CFG $CP_CFG $SYS_CFG $AUDIO_CFG GEN_LIB=1
    $build_ota_copy_cmd GEN_LIB=1
elif [[ "$COMMAND" == 'lst' ]];
then
    $build_sensorhub_cmd lst all ||{ echo "$LINENO command failed"; exit 1; }
    $build_app_cmd $APP_CUSTOMER_CFG $BLE_CFG $OTA_CFG $TWS_CFG $SPEECH_2MIC_CFG $DECODE_DECODER_CFG $CP_CFG $SYS_CFG $AUDIO_CFG lst all ||{ echo "$LINENO command failed"; exit 1; }
    $build_ota_copy_cmd lst all ||{ echo "$LINENO command failed"; exit 1; }
    echo "lst build end"
else
    $build_sensorhub_cmd ||{ echo "$LINENO command failed"; exit 1; }
    $build_app_cmd $APP_CUSTOMER_CFG $BLE_CFG $OTA_CFG $TWS_CFG $SPEECH_2MIC_CFG $DECODE_DECODER_CFG $CP_CFG $SYS_CFG $AUDIO_CFG ||{ echo "$LINENO command failed"; exit 1; }
    $build_ota_copy_cmd ||{ echo "$LINENO command failed"; exit 1; }
    echo "normal build"
    echo " "
fi