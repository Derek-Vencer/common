#!/bin/bash

COMMAND=$1
set -e
build_ota_copy_cmd="make T=prod_test/ota_copy CHIP=best1502p OTA_BIN_COMPRESSED=1 OTA_CODE_OFFSET=0x20000 DEBUG=1 BES_LIB_DIR=lib/bes/best1502p/PRO_0011 -j64"
build_cust_cmd="make T=best1502p FLASH_SIZE=0x800000 BES_OTA=1 BLE=1 BLE_AUDIO_ENABLED=0 ANC_ENABLE=1 APP_TRACE_RX_ENABLE=1 GFPS_ENABLE=1 IGNORE_POWER_ON_KEY_DURING_BOOT_UP=1 CUST_MUTE_FRAME_DETECT_CFG=1 \
    AUDIO_DYNAMIC_EQ=1 A2DP_VIRTUAL_SURROUND=1 VOICE_ASSIST_PROMPT_LEAK_DETECT=0 USE_BASIC_THREADS=1 ANC_ASSIST_ENABLE=1 ASSIST_WIND=1 USE_MOBILE_SECURITY_AUTH=1 \
    APP_RX_API_ENABLE=1 APP_MCPP_CLI=BTH APP_MCPP_BTH_SENS=0 AUDIO_DEBUG_CMD=1 APP_RPC_ENABLE=1 VOICE_ASSIST_FF_FIR_LMS=0 VOICE_ASSIST_FF_FIR_LMS_BTH_SENS=0 GATT_OVER_BR_EDR=1 \
    LARGE_SENS_RAM=0 SENS_FIR_LMS_SIZE=0x20000 APP_ANC_ASSIST_THREAD=1 AUDIO_ANC_TT_HW=1 SENSORHUB_START_WHEN_MAIN_MCU_ON=0 OTA_CODE_OFFSET=0x20000 MUTE_FRAME_DETECT=1 CTKD_ENABLE=1 \
    VOICE_ASSIST_CUSTOM_LEAK_DETECT=0 POWER_ON_ENTER_TWS_PAIRING_ENABLED=1 A2DP_LDAC_ON=1 A2DP_LDAC_PLC_ENABLED=1 BESBT_STACK_SIZE=1024*4 BES_LIB_DIR=lib/bes/best1502p/PRO_0011 -j64"

AI_RECORDING_CFG="DUAL_MIC_RECORDING=1 RECORDING_USE_OPUS=1 SINGLE_DEVICE_REC=1 AI_VOICE_TRACE_ENABLE=1 \
    RAMCP_SIZE=128*1024 ANC_ASSIST_ENABLED=1 ANC_ASSIST_PROCESS_THREAD=1 TOTA_v2=1"


if [[ "$COMMAND" == "clean" ]];
then
    $build_cust_cmd clean ||{ echo "$LINENO command failed"; exit 1; }
    $build_ota_copy_cmd clean ||{ echo "$LINENO command failed"; exit 1; }
elif [[ "$COMMAND" == "gen_lib" ]];
then
    $build_cust_cmd $AI_RECORDING_CFG GEN_LIB=1 ||{ echo "$LINENO command failed"; exit 1; }
    $build_ota_copy_cmd GEN_LIB=1 ||{ echo "$LINENO command failed"; exit 1; }

elif [[ "$COMMAND" == 'lst' ]];
then
    $build_cust_cmd lst all ||{ echo "$LINENO command failed"; exit 1; }
elif [[ "$COMMAND" == 'ota' ]];
then
    $build_ota_copy_cmd ||{ echo "$LINENO command failed"; exit 1; }
else
    $build_cust_cmd $AI_RECORDING_CFG ||{ echo "$LINENO command failed"; exit 1; }
    $build_ota_copy_cmd ||{ echo "$LINENO command failed"; exit 1; }
fi
echo $build_cust_cmd
