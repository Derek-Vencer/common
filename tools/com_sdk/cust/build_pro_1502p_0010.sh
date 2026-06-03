#!/bin/bash

COMMAND=$1
set -e

build_ota_copy_cmd="make T=prod_test/ota_copy -j CHIP=best1502p OTA_CODE_OFFSET=0x18000 SINGLE_WIRE_DOWNLOAD=0 USB_CDC_DOWNLOAD=1 \
    PROGRAMMER_WATCHDOG=1 ALLOW_WARNING=1 DEBUG=1 BES_LIB_DIR=lib/bes/best1502p/PRO_0010 -j64"
build_cust_cmd="make T=best1502p -j IGNORE_POWER_ON_KEY_DURING_BOOT_UP=1 IS_AUTOPOWEROFF_ENABLED=0 APP_TRACE_RX_ENABLE=0 APP_RX_API_ENABLE=0 \
    TRACE_BUF_SIZE=40*1024 POWER_ON_ENTER_FREEMAN_PAIRING_ENABLED=1 FREEMAN_ENABLED_STERO=1 BLE=1 USB_CDC_DOWNLOAD=1 ALLOW_WARNING=1 \
    LEA_ENABLE=1 AOB_CODEC_CP=1 BES_OTA=1 OTA_CODE_OFFSET=0x18000 TOTA_v2=1 LC3_IN_ROM_V2=1 \
    FAST_XRAM_SECTION_SIZE=0x8000 UNIFY_HEAP_ENABLED=0 \
    BES_LIB_DIR=lib/bes/best1502p/PRO_0010 -j64"

if [[ "$COMMAND" == "clean" ]];
then
    $build_cust_cmd clean ||{ echo "$LINENO command failed"; exit 1; }
    $build_ota_copy_cmd clean ||{ echo "$LINENO command failed"; exit 1; }
elif [[ "$COMMAND" == "gen_lib" ]];
then
    $build_cust_cmd GEN_LIB=1 ||{ echo "$LINENO command failed"; exit 1; }
    $build_ota_copy_cmd GEN_LIB=1 ||{ echo "$LINENO command failed"; exit 1; }
elif [[ "$COMMAND" == 'lst' ]];
then
    $build_cust_cmd lst all ||{ echo "$LINENO command failed"; exit 1; }
else
    $build_cust_cmd ||{ echo "$LINENO command failed"; exit 1; }
    $build_ota_copy_cmd||{ echo "$LINENO command failed"; exit 1; }
fi
echo $build_cust_cmd