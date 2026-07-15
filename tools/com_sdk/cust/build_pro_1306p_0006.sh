#!/bin/bash

TARGET_LIST="best1306p"
CHIPID="best1306p"
BES_LIB_DIR="lib/bes/best1306p/PRO_0006"
COMMAND=$1
set -e

APP_CUSTOMER_CFG="IGNORE_POWER_ON_KEY_DURING_BOOT_UP=1 IS_AUTOPOWEROFF_ENABLED=0 \
                APP_TRACE_RX_ENABLE=1 APP_RX_API_ENABLE=1 AUDIO_DEBUG_CMD=0 \
                TRACE_BUF_SIZE=40*1024 USE_TRACE_ID=0 CTKD_ENABLE=1 IS_CTKD_OVER_BR_EDR_ENABLED=1 \
                HSP_ENABLE=1"

BLE_CFG="BLE=1 GATT_OVER_BR_EDR=0 BLE_AUDIO_24BIT=0"

OTA_CFG="BES_OTA=1 OTA_BIN_COMPRESSED=1"

TWS_CFG="FREEMAN_ENABLED_STERO=1"

SPEECH_2MIC_CFG="SPEECH_TX_DC_FILTER=1 SPEECH_TX_2MIC_N1S11=1 SPEECH_TX_EQ=0"

ANC_CFG="ANC_ENABLE=1 AUDIO_ANC_TT_HW=1 APP_ANC_TEST=0 "

DECODE_DECODER_CFG="A2DP_AAC_ON=0 A2DP_LDAC_ON=0 A2DP_LHDC_ON=0 A2DP_LHDC_V3=0 A2DP_LHDCV5_ON=0 AAC_IN_ROM=1 CVSD_IN_ROM=1"

CP_CFG="A2DP_CP_ACCEL=1 SCO_CP_ACCEL=1 CHIP_HAS_CP=1 RAMCP_SIZE=0x4000 RAMCPX_SIZE=0x10000 FAST_XRAM_SECTION_SIZE=0x6000 UNIFY_HEAP_ENABLED=0"

SYS_CFG=" ALLOW_WARNING=1"

USB_CFG="USB_AUDIO_DYN_CFG=1 USB_AUDIO_SEND_24BIT=0 USB_AUDIO_SEND_CHAN=1 
        BT_USB_AUDIO_DUAL_MODE=1 BLE_USB_AUDIO_SUPPORT=1 PMU_USB_PIN_CHECK=0 USB_PLUGOUT_DET=1"

BOOT_SECURE_CFG="SECURE_BOOT=1 USER_SECURE_BOOT=1 FLASH_SECURITY_REGISTER=1"
APP_SECURE_CFG="SECURE_BOOT=1 FLASH_SECURITY_REGISTER=1"


build_app_cmd="make T=$TARGET_LIST BES_LIB_DIR=$BES_LIB_DIR $APP_CUSTOMER_CFG $BLE_CFG $OTA_CFG $TWS_CFG $SPEECH_2MIC_CFG $DECODE_DECODER_CFG $CP_CFG $SYS_CFG $USB_CFG $ANC_CFG -j64"
build_ota_copy_cmd="make T=prod_test/ota_copy CHIP=$CHIPID BES_LIB_DIR=$BES_LIB_DIR DEBUG=1 -j64 OTA_BIN_COMPRESSED=1"

if [[ "$COMMAND" == "clean" ]];
then
    $build_app_cmd clean
elif [[ "$COMMAND" == "gen_lib" ]];
then
    rm -rf $BES_LIB_DIR
    rm -rf out/
    $build_ota_copy_cmd GEN_LIB=1
    $build_app_cmd GEN_LIB=1
    build_ota_copy_cmd="${build_ota_copy_cmd} ${BOOT_SECURE_CFG}"
    build_app_cmd="${build_app_cmd} ${APP_SECURE_CFG}"
    $build_ota_copy_cmd GEN_LIB=1
    $build_app_cmd GEN_LIB=1
elif [[ "$COMMAND" == 'lst' ]];
then
    $build_ota_copy_cmd ||{ echo "$LINENO command failed"; exit 1; }
    $build_app_cmd lst all ||{ echo "$LINENO command failed"; exit 1; }
elif [[ "$COMMAND" == 'sercure' ]];
then
    build_ota_copy_cmd="${build_ota_copy_cmd} ${BOOT_SECURE_CFG}"
    build_app_cmd="${build_app_cmd} ${APP_SECURE_CFG}"
    $build_ota_copy_cmd ||{ echo "$LINENO command failed"; exit 1; } 
    $build_app_cmd ||{ echo "$LINENO command failed"; exit 1; } 
else
    $build_ota_copy_cmd ||{ echo "$LINENO command failed"; exit 1; }
    $build_app_cmd ||{ echo "$LINENO command failed"; exit 1; }
fi

echo "ota cmd: "
echo "$build_ota_copy_cmd"
echo "build cmd: "
echo "$build_app_cmd"
