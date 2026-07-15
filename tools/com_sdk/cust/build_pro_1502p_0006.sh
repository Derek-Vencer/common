#!/bin/bash

COMMAND=$1
set -e

build_ota_copy_cmd="make T=prod_test/ota_copy -j CHIP=best1502p OTA_BIN_COMPRESSED=1 OTA_CODE_OFFSET=0x10000 BES_LIB_DIR=lib/bes/best1502p/PRO_0006 -j64"
build_cust_cmd="make T=best1502p -j IGNORE_POWER_ON_KEY_DURING_BOOT_UP=1 IS_AUTOPOWEROFF_ENABLED=0 APP_TRACE_RX_ENABLE=0 APP_RX_API_ENABLE=0 \
TRACE_BUF_SIZE=40*1024 POWER_ON_ENTER_FREEMAN_PAIRING_ENABLED=1 FREEMAN_ENABLED_STERO=1 BLE=1 GATT_OVER_BR_EDR=1 SWIFT_ENABLE=1 \
LEA_ENABLE=0 AOB_CODEC_CP=1 BES_OTA=1 OTA_CODE_OFFSET=0x10000 OTA_BIN_COMPRESSED=1 TOTA_v2=1 A2DP_AAC_ON=1 A2DP_LDAC_ON=1 A2DP_LHDCV5_ON=0 AAC_IN_ROM=1 CVSD_IN_ROM=1 \
SPEECH_TX_DC_FILTER=1 SPEECH_TX_2MIC_NS8=1 SPEECH_TX_EQ=1 SPEECH_RX_NS2FLOAT=1 SPEECH_RX_EQ=1 A2DP_VIRTUAL_SURROUND_STEREO=1 HEAD_TRACK_ENABLE=0 AUDIO_ADJ_EQ_REV=1 AUDIO_LIMITER=1 \
ANC_ENABLE=1 ANC_ASSIST_ENABLED=1 VOICE_ASSIST_NOISE=1 ANC_FF_CHECK=1 USE_TRACE_ID=0 BTHOST_DEBUG=1 \
A2DP_CP_ACCEL=1 SCO_CP_ACCEL=1 CHIP_HAS_CP=1 RAMCP_SIZE=0x8000 RAMCPX_SIZE=0x10000 FAST_XRAM_SECTION_SIZE=0x8000 UNIFY_HEAP_ENABLED=0 \
AUDIO_LINEIN=1 BES_LIB_DIR=lib/bes/best1502p/PRO_0006 -j64"

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
