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
RM_LIB=$4


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


################################### build  start ###################################


build_ota_copy_cmd="make T=prod_test/ota_copy -j CHIP=best1502p OTA_BIN_COMPRESSED=1 SINGLE_WIRE_DOWNLOAD=1"
build_sensor_hub_cmd="make T=sensor_hub -j CHIP=best1502p SENS_TRC_TO_MCU=1 VPU_CFG_ON_SENSOR_HUB=1 VPU_NAME=LSM6DSV16BX"
build_app_tws_cmd="make T=best1502p -j A2DP_LHDC_ON=1 A2DP_LHDC_V3=1 A2DP_LHDCV5_ON=1 A2DP_LDAC_ON=1 BLE=1 GFPS_ENABLE=1 FREE_TWS_PAIRING_ENABLED=1 \
ANC_ENABLE=1 ANC_ASSIST_ENABLE=1  SENSOR_HUB=1 SENS_TRC_TO_MCU=1 SPEECH_BONE_SENSOR=1 VPU_NAME=LSM6DSV16BX VPU_CFG_ON_SENSOR_HUB=1 CHIP_HAS_TDM=1 BT_SVC_MODULE_TWS_BLE_SEAMLESS_SWITCH=1 \
BLE=1 APP_BLE_DEMO_APP_ENABLED=1 BES_OTA=1 OTA_BIN_COMPRESSED=1 VOICE_ASSIST_PROMPT_LEAK_DETECT=1 AUDIO_ANC_TT_HW=1"

if [[ $RELEASE == "build" ]];
then
    ### build ###
    echo "build_ota_copy_cmd:"
    $build_ota_copy_cmd ||{ echo "$LINENO command failed"; exit 1; }
    echo "build_sensor_hub_cmd:"
    $build_sensor_hub_cmd ||{ echo "$LINENO command failed"; exit 1; }
    echo "build_app_tws_cmd:"
    $build_app_tws_cmd ||{ echo "$LINENO command failed"; exit 1; }

elif [[ $RELEASE == "release" ]];
then
    echo "build_ota_copy_cmd:rel"
    $build_ota_copy_cmd GEN_LIB=1 ||{ echo "$LINENO command failed"; exit 1; }
    echo "build_sensor_hub_cmd:rel"
    $build_sensor_hub_cmd  GEN_LIB=1 ||{ echo "$LINENO command failed"; exit 1; }
    echo "build_app_tws_cmd:rel"
    $build_app_tws_cmd  GEN_LIB=1 ||{ echo "$LINENO command failed"; exit 1; }
fi
################################## end of build  start ###################################

# ################################### release script ###################################
# if [[ $RELEASE == "release" ]];
# then


# . `dirname $0`/relsw_ibrt_edf.sh

# fi
# ################################### end of release script ###################################
