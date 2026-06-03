rm -rf out/

TARGET_LIST="best1306p"
CHIPID="best1306p"
COMMAND=$1
set -e

BUILD_CUSTOMER_CFG="IGNORE_POWER_ON_KEY_DURING_BOOT_UP=1 IS_AUTOPOWEROFF_ENABLED=0 APP_TRACE_RX_ENABLE=1 APP_RX_API_ENABLE=1 AUDIO_DEBUG_CMD=0 \
                    TRACE_BUF_SIZE=40*1024 USE_TRACE_ID=0 SPEECH_TX_DC_FILTER=1 SPEECH_TX_2MIC_NS8=1 SPEECH_TX_EQ=0 BES_OTA=1 OTA_BIN_COMPRESSED=1\
                    AMA_VOICE=1 ALEXA_WWE_LITE=0 ALEXA_WWE=0 AI_VOICE_TRACE_ENABLE=1 IS_CTKD_OVER_BR_EDR_ENABLED=0 POWER_ON_ENTER_TWS_PAIRING_ENABLED=1\
                    A2DP_LDAC_ON=1 CAPSENSOR_ENABLE=1 BES_LIB_DIR=lib/bes/best1306p/PRO_0004"


if [[ "$COMMAND" == "clean" ]];
then
    make T=$TARGET_LIST clean ||{ echo "$LINENO command failed"; exit 1; }
elif [[ "$COMMAND" == "gen_lib" ]];
then
    make T=$TARGET_LIST $BUILD_CUSTOMER_CFG -j64 GEN_LIB=1 ||{ echo "$LINENO command failed"; exit 1; }
    make T=prod_test/ota_copy CHIP=$CHIPID DEBUG=1 -j64 GEN_LIB=1 ||{ echo "$LINENO command failed"; exit 1; }
elif [[ $1 == 'lst' ]];
then
    make T=$TARGET_LIST $BUILD_CUSTOMER_CFG -j64 lst all ||{ echo "$LINENO command failed"; exit 1; }
else
    make T=prod_test/ota_copy CHIP=$CHIPID DEBUG=1 -j64 ||{ echo "$LINENO command failed"; exit 1; }
    make T=$TARGET_LIST $BUILD_CUSTOMER_CFG -j64 ||{ echo "$LINENO command failed"; exit 1; }
fi

echo $BUILD_CUSTOMER_CFG