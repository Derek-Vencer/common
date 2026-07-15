#!/bin/bash
[ -z "$1" ] &&{
    echo The Chip is Null !!!
    exit
}

CHIPID=$1
PRODUCT_TYPE=${2:-""}
COMMAND=${3:-""}

PRODUCT_TYPE=${PRODUCT_TYPE:-""}
COMMAND=${COMMAND:-""}
ANC_USB_ENABLED=0

if [[ $CHIPID == "best1306p" ]];
then
    TARGET_LIST="best1306p"
    echo "1306p"
    if [[ "$PRODUCT_TYPE" == "typec" ]];
    then
        TARGET_LIST="anc_usb"
        echo "anc_usb"
    fi
elif [[ $CHIPID == "best1502p" ]];
then
    TARGET_LIST="best1502p"
    echo "1502p"
elif [[ $CHIPID == "best1503" ]];
then
    TARGET_LIST="best1503"
    echo "1503"
fi

set -e

################################### var define ###################################
APP_CUSTOMER_CFG="IGNORE_POWER_ON_KEY_DURING_BOOT_UP=1 IS_AUTOPOWEROFF_ENABLED=0 APP_TRACE_RX_ENABLE=1 APP_RX_API_ENABLE=1 AUDIO_DEBUG_CMD=0 \
                 TRACE_BUF_SIZE=40*1024 USE_TRACE_ID=0"

BLE_CFG="BLE=1 GATT_OVER_BR_EDR=0"

OTA_CFG="BES_OTA=1 OTA_BIN_COMPRESSED=1"

TWS_CFG="POWER_ON_ENTER_TWS_PAIRING_ENABLED=1"

FREEMAN_CFG="POWER_ON_ENTER_FREEMAN_PAIRING_ENABLED=1 FREEMAN_ENABLED_STERO=1"

SPEECH_CFG="SPEECH_TX_DC_FILTER=1 SPEECH_TX_AEC2FLOAT=1 SPEECH_TX_1MIC_NS=1 SPEECH_TX_EQ=1"

SPEECH_2MIC_CFG="SPEECH_TX_DC_FILTER=1 SPEECH_TX_2MIC_NS8=1 SPEECH_TX_EQ=0"

DECODE_DECODER_CFG="A2DP_AAC_ON=1 A2DP_LDAC_ON=0 A2DP_LHDC_ON=0 A2DP_LHDC_V3=0 A2DP_LHDCV5_ON=0 AAC_IN_ROM=1 CVSD_IN_ROM=1"

CP_CFG="A2DP_CP_ACCEL=1 SCO_CP_ACCEL=1 CHIP_HAS_CP=1 RAMCP_SIZE=0x4000 RAMCPX_SIZE=0x10000 FAST_XRAM_SECTION_SIZE=0x6000 UNIFY_HEAP_ENABLED=0"

NO_CP_CFG="A2DP_CP_ACCEL=0 SCO_CP_ACCEL=0 CHIP_HAS_CP=0 RAMCP_SIZE=0 RAMCPX_SIZE=0 FAST_XRAM_SECTION_SIZE=0x12000"

SYS_CFG=" ALLOW_WARNING=1"

ANC_CFG="ANC_ENABLE=1 ANC_ASSIST_ENABLED=0 VOICE_ASSIST_NOISE=0 VOICE_ASSIST_CUSTOM_LEAK_DETECT=0 VOICE_ASSIST_CUSTOM_LEAK_DETECT=0 ANC_PROD_TEST=1 TOTA_v2=1"

ANC_USB_CFG="USB_AUDIO_APP=1 ULTRA_LOW_POWER=1 PMU_USB_PIN_CHECK=0 OSC_26M_X4_AUD2BB=1 USB_AUDIO_DYN_CFG=0 RTOS=0"
################################### end of var define ###################################
if [[ "$COMMAND" == "clean" ]];
then
    make T=$TARGET_LIST clean ||{ echo "$LINENO command failed"; exit 1; }
elif [[ "$COMMAND" == "gen_lib" ]];
then
    if [[ "$PRODUCT_TYPE" == "typec" ]];
    then
        make T=$TARGET_LIST CHIP=$CHIPID $ANC_USB_CFG -j64 GEN_LIB=1 || { echo "$LINENO command failed"; exit 1; }
    else
        make T=$TARGET_LIST $APP_CUSTOMER_CFG $BLE_CFG $OTA_CFG $TWS_CFG $SPEECH_CFG $DECODE_DECODER_CFG $NO_CP_CFG $SYS_CFG -j64 GEN_LIB=1 ||{ echo "$LINENO command failed"; exit 1; }
        make T=$TARGET_LIST $APP_CUSTOMER_CFG $BLE_CFG $OTA_CFG $TWS_CFG $SPEECH_CFG $DECODE_DECODER_CFG $CP_CFG $SYS_CFG -j64 GEN_LIB=1 ||{ echo "$LINENO command failed"; exit 1; }
        make T=$TARGET_LIST $APP_CUSTOMER_CFG $BLE_CFG $OTA_CFG $FREEMAN_CFG $SPEECH_CFG $DECODE_DECODER_CFG $NO_CP_CFG $SYS_CFG -j64 GEN_LIB=1 ||{ echo "$LINENO command failed"; exit 1; }
        make T=$TARGET_LIST $APP_CUSTOMER_CFG $BLE_CFG $OTA_CFG $FREEMAN_CFG $SPEECH_CFG $DECODE_DECODER_CFG $CP_CFG $SYS_CFG -j64 GEN_LIB=1 ||{ echo "$LINENO command failed"; exit 1; }
        make T=$TARGET_LIST $APP_CUSTOMER_CFG $BLE_CFG $OTA_CFG $TWS_CFG $SPEECH_CFG $DECODE_DECODER_CFG $NO_CP_CFG $SYS_CFG  $ANC_CFG-j64 GEN_LIB=1 ||{ echo "$LINENO command failed"; exit 1; }
        make T=$TARGET_LIST $APP_CUSTOMER_CFG $BLE_CFG $OTA_CFG $TWS_CFG $SPEECH_CFG $DECODE_DECODER_CFG $CP_CFG $SYS_CFG $ANC_CFG -j64 GEN_LIB=1 ||{ echo "$LINENO command failed"; exit 1; }
        make T=$TARGET_LIST $APP_CUSTOMER_CFG $BLE_CFG $OTA_CFG $FREEMAN_CFG $SPEECH_CFG $DECODE_DECODER_CFG $NO_CP_CFG $SYS_CFG $ANC_CFG -j64 GEN_LIB=1 ||{ echo "$LINENO command failed"; exit 1; }
        make T=$TARGET_LIST $APP_CUSTOMER_CFG $BLE_CFG $OTA_CFG $FREEMAN_CFG $SPEECH_CFG $DECODE_DECODER_CFG $CP_CFG $SYS_CFG $ANC_CFG -j64 GEN_LIB=1 ||{ echo "$LINENO command failed"; exit 1; }
    fi
    make T=prod_test/ota_copy CHIP=$CHIPID DEBUG=1 -j64 GEN_LIB=1 ||{ echo "$LINENO command failed"; exit 1; }
elif [[ "$COMMAND" == 'lst' ]];
then
    make T=$TARGET_LIST $APP_CUSTOMER_CFG $BLE_CFG $OTA_CFG $TWS_CFG $SPEECH_CFG $DECODE_DECODER_CFG $CP_CFG $SYS_CFG $ANC_CFG -j64 lst all ||{ echo "$LINENO command failed"; exit 1; }
else
    if [[ "$PRODUCT_TYPE" == "typec" ]];
    then
        make T=$TARGET_LIST CHIP=$CHIPID $ANC_USB_CFG -j64 ||{ echo "$LINENO command failed"; exit 1; }
    else
        make T=$TARGET_LIST $APP_CUSTOMER_CFG $BLE_CFG $OTA_CFG $TWS_CFG $SPEECH_CFG $DECODE_DECODER_CFG $CP_CFG $SYS_CFG $ANC_CFG -j64 ||{ echo "$LINENO command failed"; exit 1; }
    fi
    make T=prod_test/ota_copy CHIP=$CHIPID DEBUG=1 -j64 ||{ echo "$LINENO command failed"; exit 1; }
fi