#!/bin/bash

TARGET_LIST="best1306p"
CHIPID="best1306p"
COMMAND=$1
set -e

build_app_cmd="make T=best1306p CHIP=best1306p -j64 BES_LIB_DIR=lib/bes/best1306p/PRO_0003 \
BES_OTA=1 POWER_ON_ENTER_TWS_PAIRING_ENABLED=1 BLE=1 USB_PLUGOUT_DET=1 \
PMU_USB_PIN_CHECK=0 USB_AUDIO_DYN_CFG=0 USB_AUDIO_48K=1 \
USB_AUDIO_24BIT=1 USB_AUDIO_SEND_24BIT=1 AUDIO_PLAYBACK_24BIT=0 USB_AUDIO_SEND_CHAN=1 \
BT_USB_AUDIO_DUAL_MODE=1 BLE_USB_AUDIO_SUPPORT=1 AF_DEVICE_I2S=1 PLAYBACK_USE_I2S=1 CAPTURE_USE_I2S=0 \
IOS_IAP2_BLUETOOTH=1 TWS_SINGLE_EAR_PLAY_LRMERGE=1"

build_ota_copy_cmd="make T=prod_test/ota_copy -j64 CHIP=best1306p DEBUG=1"

if [[ "$COMMAND" == "clean" ]];
then
    $build_app_cmd clean
    echo "clean end"
elif [[ "$COMMAND" == "gen_lib" ]];
then
    $build_app_cmd GEN_LIB=1 ||{ echo "$LINENO command failed"; exit 1; }
    $build_ota_copy_cmd GEN_LIB=1 ||{ echo "$LINENO command failed"; exit 1; }
elif [[ "$COMMAND" == 'lst' ]];
then
    $build_app_cmd lst all ||{ echo "$LINENO command failed"; exit 1; }
    echo "lst build end"
else
    $build_app_cmd ||{ echo "$LINENO command failed"; exit 1; }
    $build_ota_copy_cmd ||{ echo "$LINENO command failed"; exit 1; }
    echo "normal build"
    echo " "
fi