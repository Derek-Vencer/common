#!/bin/bash


[ -z "$1" ] &&{
    echo The Chip is Null !!!
    exit
}

[ -z "$2" ] &&{
    echo The Target is Null !!!
    exit
}

[ -z "$3" ] &&{
    echo "The Target_Customer is Null !!! \n"
    echo "hearing_aid_customer or audio_customer "
    exit
}


CHIPID=$1
TARGET_LIST=$2
RELEASE=$3

DOLBY_EN=$4
UAC_EN=$5
RM_LIB=$6


echo "param opt:"
echo "CHIPID=$CHIPID TARGET_LIST=$TARGET_LIST RELEASE=$RELEASE DOLBY=$DOLBY_EN UAC_EN=$UAC_EN RM_LIB=$RM_LIB"


TARGET_DIR="config/"$TARGET_LIST
TARGET_NAME="$TARGET_LIST"

#git clean -d -fx
DATE=`date +%F | sed 's/-//g'`
commitid=`git rev-parse --short HEAD`

export CROSS_COMPILE="ccache arm-none-eabi-"

if [[ $RELEASE == "clean" ]];
then
    echo -e "rm old lib"
    LIB_DIR=lib/bes/
    rm build_err.log
    rm -rf $LIB_DIR
    rm out -rf
    exit;
elif [[ $RELEASE == "release" ]];
then
    echo -e "release"
    if [[$RM_LIB == "rm_lib"]];
    then
        LIB_DIR=lib/bes/
        rm -rf $LIB_DIR
    fi
    rm build_err.log
    rm out -rf
fi

# Exit when error
set -e

################################### var define ###################################

if [[ $CHIPID == "best1306p" ]];
then
    TARGET_LIST="best1306p"
    echo "1306p"
    CHIP_HAS_USB="YES"
elif [[ $CHIPID == "best1502p" ]];
then
    TARGET_LIST="best1502p"
    echo "1502p"
    CHIP_HAS_USB="YES"
elif [[ $CHIPID == "best1503" ]];
then
    TARGET_LIST="best1503"
    echo "1503"
    CHIP_HAS_USB="NO"
fi

BT_STACK_COMMON_CFG="BT_RF_PREFER=2M BTH_IN_ROM=0 BT_DIP_SUPPORT=0 "

BT_APP_FEATURE_CFG="GFPS_ENABLE=1 SASS_ENABLE=1 SWIFT_ENABLE=1 SPOT_ENABLE=0 HOST_GEN_ECDH_KEY=1 GATT_OVER_BR_EDR=1\
IS_CTKD_OVER_BR_EDR_ENABLED=0 "

LE_AUDIO_CFG="BLE=1 BLE_AUDIO_ENABLED=1 BLE_CONNECTION_MAX=2 BT_RAMRUN_NEW=1 BT_DEBUG_TPORTS=0xB8B8 AOB_LOW_LATENCY_MODE=0 \
                REPORT_EVENT_TO_CUSTOMIZED_UX=1 CTKD_ENABLE=1 IS_CTKD_OVER_BR_EDR_ENABLED=1 \
                BLE_ADV_RPA_ENABLED=1 BLE_AOB_VOLUME_SYNC_ENABLED=0 \
                AOB_MOBILE_ENABLED=0 BLE_AUDIO_TEST_ENABLED=0 BT_SEC_CON_BASED_ON_COD_LE_AUD=0\
                BT_SVC_MODULE_LEA_ENABLED=1"

APP_CUSTOMER_CFG="BES_OTA=1 IGNORE_POWER_ON_KEY_DURING_BOOT_UP=1 \
                IS_AUTOPOWEROFF_ENABLED=0 POWER_ON_ENTER_TWS_PAIRING_ENABLED=1 FREE_TWS_PAIRING_ENABLED=1 \
                APP_TRACE_RX_ENABLE=1 APP_RX_API_ENABLE=1 AUDIO_DEBUG_CMD=0 BT_BLEAUDIO_COEXIST=0 IBRT_SEARCH_UI=1 SEARCH_UI_COMPATIBLE_UI_V2=1 TOTA_v2=1\
                TRACE_BUF_SIZE=20*1024 OTA_BIN_COMPRESSED=1 FREEMAN_ENABLED_STERO=0  ANC_APP=1 USE_TRACE_ID=0"

OTA_COPY_OFFSET_CFG="OTA_CODE_OFFSET=0x18000 OTA_BOOT_SIZE=0x10000 OTA_BOOT_INFO_OFFSET=0x10000 OTA_BIN_COMPRESSED=1"

DECODE_DECODER_CFG="A2DP_AAC_ON=1 A2DP_LDAC_ON=1 A2DP_LHDC_ON=1 A2DP_LHDC_V3=1 A2DP_LHDCV5_ON=1 A2DP_SCALABLE_ON=0 A2DP_LC3_ON=0 A2DP_LC3_HR=0 \
                    LC3_IN_ROM_V2=0 AAC_IN_ROM=1 CVSD_IN_ROM=1 GAF_LC3_BES_PLC_ON=0 GAF_LC3_MUSIC_PLC_ON=0  A2DP_CP_ACCEL=1 SCO_CP_ACCEL=1 CHIP_HAS_CP=1"

if [[ $CHIP_HAS_USB == "YES" ]];
then
    USB_AUDIO_CFG="BT_USB_AUDIO_DUAL_MODE=1 USB_ISO=1 DELAY_STREAM_OPEN=1 USB_AUDIO_DYN_CFG=1 HID_ULL_ENABLE=1"
else
    USB_AUDIO_CFG=" "
fi

SENSOR_HUB_CFG="SENSOR_HUB=1 SENS_TRC_TO_MCU=1"

CAPSENSOR_CFG="CAPSENSOR_ENABLE=1 CAPSENSOR_WEAR=1 CAPSENSOR_TOUCH=1 CAPSENSOR_SLIDE=0 CAPSENSOR_TRACE_DEBUG=0 CAPSENSOR_SPP_SERVER=1 CAPSENSOR_AT_SENS=1"

IIS_CFG="AF_DEVICE_INT_CODEC=1 AF_DEVICE_I2S=1 AUDIO_LINEIN=0"


################################### end of var define ###################################



################################### build  start ###################################

SECURE_BOOT_FLAGS="SECURE_BOOT=1 USER_SECURE_BOOT=1 FLASH_SECURITY_REGISTER=1 "
PROJECT_CFG_FLAGS="FLASH_SIZE=0x800000 FLASH_PROTECTION=1 SPA_AUDIO_ENABLE=1 SPA_AUDIO_SEC=1 "
OTA_TZ_FLAGS="OTA_TZ_ENABLE=1 "

build_secur_otacopy_cmd="make T=prod_test/ota_copy  CHIP=$CHIPID DEBUG=1 -j40 \
    $SECURE_BOOT_FLAGS $OTA_TZ_FLAGS \
    SPA_AUDIO_ENABLE=1 SPA_AUDIO_SEC=1 \
        OTA_REBOOT_FLASH_REMAP=0 DOLBY_AUDIO_ENABLE=1 OTA_BIN_COMPRESSED=1\
    "
build_cmse_cmd="make T=arm_cmse DEBUG=1 -j40  CHIP=$CHIPID \
    $PROJECT_CFG_FLAGS $SECURE_BOOT_FLAGS \
    TZ_ROM_UTILS_IF=0 ROM_UTILS_ON=0 CRC32_ROM=1 CMSE_RAM_RAMX_LEND_NSE=1 \
    SE_OTP_DEMO_TEST=0 CMSE_CRYPT_TEST_DEMO=1 MBEDTLS_CONFIG_FILE="config-rsa.h" \
    OTA_CODE_OFFSET=0x20000 LARGE_SE_RAM=1 FLASH_LOW_SPEED=1 ULTRA_LOW_POWER=1\
    DOLBY_AUDIO_ENABLE=1 DOLBY_AUDIO_DAW_ENABLE=0  $OTA_TZ_FLAGS\
"
build_app_dolby_cmd="make T=$TARGET_LIST -j40 DEBUG=1 \
    A2DP_CP_ACCEL=1 SCO_CP_ACCEL=1 CHIP_HAS_CP=1 \
    LIBC_ROM=0 ARM_CMNS=1 TZ_ROM_UTILS_IF=0 ROM_UTILS_ON=0 LARGE_SE_RAM=1 \
    SECURE_BOOT=1 USER_SECURE_BOOT=0 \
    FLASH_SIZE=0x800000 TRACE_BUF_SIZE=24*1024  RAMCP_SIZE=0x4E000 RAMCPX_SIZE=0x11000 OS_DYNAMIC_MEM_SIZE=0x5900 FAST_XRAM_SECTION_SIZE=0x18000 \
    DOLBY_AUDIO_ENABLE=1 DOLBY_AUDIO_DAW_ENABLE=0 SPA_AUDIO_ENABLE=1 SPA_AUDIO_SEC=1 \
    TOTA_v2=1 GFPS_ENABLE=0 FLASH_REMAP=0 COMBO_CUSBIN_IN_FLASH=1 \
    BES_OTA=1 ARM_CMNS_OTA=1 OTA_TZ_ENABLE=1 OTA_CODE_OFFSET=0x80000 OTA_BIN_COMPRESSED=1 \
    APP_TRACE_RX_ENABLE=1 \
    ANC_ENABLE=1 FREEMAN_ENABLED_STERO=1 AUDIO_OUTPUT_SW_GAIN=1 A2DP_AAC_ON=1 A2DP_LDAC_ON=1  \
    BLE=1 AUDIO_BASS_ENHANCER=1 ANC_ASSIST_ENABLED=1 AUDIO_DRC=1 HW_DAC_IIR_EQ_PROCESS=1 \
    FAST_TIMER_COMPENSATE=1 BT_DISC_ACL_AFTER_AUTH_KEY_MISSING=1 PC_CMD_UART=0 AUDIO_DYNAMIC_EQ=0 \
"
if [[ $RELEASE == "build" ]];
then
    ### build ###
    if [[ $DOLBY_EN == "dolby" ]];
    then
        echo "build_secur_otacopy_cmd:"
        $build_secur_otacopy_cmd
        echo "build_cmse_cmd:"
        $build_cmse_cmd
        echo "build_app_dolby_cmd:"
        $build_app_dolby_cmd

    elif [[ $UAC_EN == "uac" ]];
    then
        make T=$TARGET_LIST $BT_STACK_COMMON_CFG $APP_CUSTOMER_CFG $DECODE_DECODER_CFG $USB_AUDIO_CFG  -j8 ||{ echo " $LINENO command failed"; exit 1; }
    else
        make T=$TARGET_LIST $BT_STACK_COMMON_CFG $APP_CUSTOMER_CFG $DECODE_DECODER_CFG  -j8 ||{ echo " $LINENO command failed"; exit 1; }
    fi


elif [[ $RELEASE == "release" ]];
then

    if [[ $DOLBY_EN == "dolby" ]];
    then
        echo "build_secur_otacopy_cmd:"
        $build_secur_otacopy_cmd  GEN_LIB=1
        echo "build_cmse_cmd:"
        $build_cmse_cmd  GEN_LIB=1
        echo "build_app_dolby_cmd:"
        $build_app_dolby_cmd GEN_LIB=1
    else
        ###sub bin####
        make T=prod_test/ota_copy GEN_LIB=1 DEBUG=1 -j8 CHIP=$CHIPID  $OTA_COPY_OFFSET_CFG ||{ echo " $LINENO command failed"; exit 1; }
        make T=prod_test/ota_copy GEN_LIB=1 DEBUG=1 -j8 CHIP=$CHIPID OTA_BIN_COMPRESSED=1 $OTA_COPY_OFFSET_CFG ||{ echo " $LINENO command failed"; exit 1; }
        # make T=sensor_hub GEN_LIB=1 CHIP=$CHIPID SENS_TRC_TO_MCU=1 DEBUG=1 VOICE_DETECTOR_EN=0 -j8

        ### app bin ###
        make T=$TARGET_LIST GEN_LIB=1  -j8  BLE=0 ||{ echo " $LINENO command failed"; exit 1; }
        make T=$TARGET_LIST GEN_LIB=1  -j8  BLE=1 GFPS_ENABLE=0 ||{ echo " $LINENO command failed"; exit 1; }
        make T=$TARGET_LIST GEN_LIB=1  -j8  BLE=0 $USB_AUDIO_CFG ||{ echo " $LINENO command failed"; exit 1; }
        make T=$TARGET_LIST GEN_LIB=1  -j8  BLE=1 $USB_AUDIO_CFG GFPS_ENABLE=0 ||{ echo " $LINENO command failed"; exit 1; }
        make T=$TARGET_LIST GEN_LIB=1  -j8  BLE=1 $USB_AUDIO_CFG $DECODE_DECODER_CFG GFPS_ENABLE=1 ||{ echo " $LINENO command failed"; exit 1; }
        make T=$TARGET_LIST GEN_LIB=1  -j8  BLE=1 $DECODE_DECODER_CFG GFPS_ENABLE=1 ||{ echo " $LINENO command failed"; exit 1; }
        make T=$TARGET_LIST GEN_LIB=1  -j8  BLE=1 $USB_AUDIO_CFG $DECODE_DECODER_CFG GATT_OVER_BR_EDR=0 GFPS_ENABLE=0 FREEMAN_ENABLED_STERO=1  ANC_APP=1 ||{ echo " $LINENO command failed"; exit 1; }
        make T=$TARGET_LIST GEN_LIB=1  -j8  BLE=1 $USB_AUDIO_CFG $DECODE_DECODER_CFG GATT_OVER_BR_EDR=1 GFPS_ENABLE=1 FREEMAN_ENABLED_STERO=1  ANC_APP=1 ||{ echo " $LINENO command failed"; exit 1; }
        make T=$TARGET_LIST GEN_LIB=1  -j8  BLE=1 GATT_OVER_BR_EDR=1 GFPS_ENABLE=1 FREEMAN_ENABLED_STERO=1  ANC_APP=1 ||{ echo " $LINENO command failed"; exit 1; }
        make T=$TARGET_LIST GEN_LIB=1  -j8  BLE=1 $USB_AUDIO_CFG GATT_OVER_BR_EDR=1 GFPS_ENABLE=1 FREEMAN_ENABLED_STERO=1  ANC_APP=1 ||{ echo " $LINENO command failed"; exit 1; }
    fi

fi
################################### end of build  start ###################################

# ################################### release script ###################################
# if [[ $RELEASE == "release" ]];
# then


# . `dirname $0`/relsw_ibrt_common.sh

# fi
# ################################### end of release script ###################################
