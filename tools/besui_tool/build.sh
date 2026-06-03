
#!/bin/bash
#build command "./tools/besui_tool/build.sh TWSPRO OTA"

# DUT_CMD="FORCE_SIGNALINGMODE=0 FORCE_NOSIGNALINGMODE=0"    
# NTC_CMD="PMU_NTC_MONITOR=1 NTC_MONITOR_USE_OSTIMER=1 BESUI_NTC_EN=1"
# I2C_CMD="DEBUG_PORT=2 MCU_I2C_SLAVE=1 NO_SLEEP=1"
CAP_CMD="CAPSENSOR_ENABLE=1 CAPSENSOR_TOUCH=1 CAPSENSOR_SPP_SERVER=1"
RAM_CMD="A2DP_CP_ACCEL=1 SCO_CP_ACCEL=1 CHIP_HAS_CP=1 UNIFY_HEAP_ENABLED=1 RAMCPX_SIZE=0xE000 RAMCP_SIZE=0x60000"
FLASH_CMD="FLASH_SIZE=0x400000"

CMD1=$1
CMD2=$2
echo "CMD1 = "$CMD1
echo "CMD2 = "$CMD2

PRONAME=$CMD1
CHIP_TYPE="best1306p" #best1306 best1306p best1307p best1501 best1501p best1502x best1502p best1503
CPU_TYPE="BES2710IHC"
BES_LIB_DIR="BES_LIB_DIR=lib/besui/$CHIP_TYPE "

if [ "$CHIP_TYPE" = "best1306" ]
then
RAM_CMD="A2DP_CP_ACCEL=1 SCO_CP_ACCEL=1 CHIP_HAS_CP=1 UNIFY_HEAP_ENABLED=1 RAMCPX_SIZE=0xBC00 RAMCP_SIZE=0x2B000 FAST_XRAM_SECTION_SIZE=0x2800"
elif [ "$CHIP_TYPE" = "best1307p" ]
then
RAM_CMD="RAMCPX_SIZE=0xC000 RAMCP_SIZE=0x2C000"
FLASH_CMD="FLASH_SIZE=0x200000"
CAP_CMD="CAPSENSOR_ENABLE=1 CAPSENSOR_AT_MCU=1 CAP_CHNL_BONDING_WITH_GPIO=1 CHIP_CAPSENSOR_VER=4 CAPSENSOR_FP_MODE=1 \
CAPSENSOR_TOUCH=1 CAPSENSOR_WEAR=0 CAPSENSOR_SLIDE=0 CAPSENSOR_TRACE_DEBUG=0 CAPSENSOR_SPP_SERVER=1"
elif [ "$CHIP_TYPE" = "best1501p" ] || [ "$CHIP_TYPE" = "best1502x" ] || [ "$CHIP_TYPE" = "best1501" ]
then
CAP_CMD="CAPSENSOR_ENABLE=1 CAPSENSOR_TOUCH=1 CAPSENSOR_SPP_SERVER=0 \
CHIP_CAPSENSOR_VER=0 SENSOR_HUB=1 CAPSENSOR_AT_SENS=1 SENS_TRC_TO_MCU=1"
SENSORHUB_CMD="make T=sensor_hub -j CHIP=$CHIP_TYPE SENSOR_HUB=1 CAPSENSOR_ENABLE=1 \
SENS_CAP_SENS_DMA_ENABLE=1 SENS_TRC_TO_MCU=1 CHIP_CAPSENSOR_VER=0 \
CAPSENSOR_WEAR=1 CAPSENSOR_TOUCH=1 CAPSENSOR_TRACE_DEBUG=0 CAPSENSOR_SPP_SERVER=1 \
"
$SENSORHUB_CMD
fi

if [ "$CMD2" = "OTA" ] || [ "$CMD2" = "ota" ]
then
echo "|***********************************************|"
echo "|                start build ota                |"
echo "|***********************************************|"
make T=prod_test/ota_copy CHIP=$CHIP_TYPE -j DEBUG=1 BES_OTA=1 OTA_BIN_COMPRESSED=1 FLASH_REMAP=0 \
$FLASH_CMD OTA_BOOT_SIZE=0x18000 CHIP_HAS_CP=0 SINGLE_WIRE_DOWNLOAD=1 GEN_LIB=0 $BES_LIB_DIR
fi

#-------------------------------------------------------------------------------------------------------------------------
[[ $PRONAME == TWSPRO ]] &&{
    build_cmd="make T=$CHIP_TYPE -j DEBUG=1 \
    $CAP_CMD \
    A2DP_VIRTUAL_SURROUND=1 AUDIO_LIMITER=0 A2DP_LDAC_ON=1 EQ_SET_CUSTOMER_EN=1 \
    SPEECH_TX_2MIC_NS8=1 \
    A2DP_CP_ACCEL=1 SCO_CP_ACCEL=1 CHIP_HAS_CP=1 UNIFY_HEAP_ENABLED=1 \
    BESUI_APP_EN=1 TOTA_v2=1 GFPS_ENABLE=0 BLE=1 \
    BES_OTA=1 TRACE_BUF_SIZE=4*1024 $RAM_CMD $FLASH_CMD \
    BES_TWSPRO_EN=1 BESUI_TWS_EN=1 \
    ANC_APP=1 ANC_ENABLE=1 SPP_EQ_TUNING=1 APP_ANC_TEST=1 \
    USER_SPEECH_DUMP_EN=0 BTLE_NAME_FIXED_EN=0 PAIRMODE_AUTO_POWEROFF_EN=1 \
	BTLENAME_USE_SCANRSP_EN=0 CRASH_FAST_RST_EN=0 \
    $DUT_CMD USER_BURN_ADDR_PAIR_EN=0 EVB_DEMO_EN=0 GEN_LIB=0 $BES_LIB_DIR"
    btname="(TWSPRO)"
}
[[ $PRONAME == NOTWS ]] &&{
    build_cmd="make T=$CHIP_TYPE -j DEBUG=1 \
    CAPSENSOR_ENABLE=0 CAPSENSOR_SPP_SERVER=0 \
    A2DP_VIRTUAL_SURROUND_STEREO=1 AUDIO_LIMITER=0 A2DP_LDAC_ON=1 EQ_SET_CUSTOMER_EN=1 \
    SPEECH_TX_2MIC_NS8=1 \
    A2DP_CP_ACCEL=1 SCO_CP_ACCEL=1 CHIP_HAS_CP=1 UNIFY_HEAP_ENABLED=1 \
    BESUI_APP_EN=1 TOTA_v2=1 GFPS_ENABLE=0 BLE=1 \
    BES_OTA=1 TRACE_BUF_SIZE=4*1024 $RAM_CMD $FLASH_CMD \
    BES_NOTWS_EN=1 BESUI_STEREO_EN=1 FREEMAN_ENABLED_STERO=1 FREEMAN_OTA_ENABLE=1\
    ANC_APP=1 ANC_ENABLE=1 SPP_EQ_TUNING=1 APP_ANC_TEST=1 \
    USER_SPEECH_DUMP_EN=0 BTLE_NAME_FIXED_EN=0 PAIRMODE_AUTO_POWEROFF_EN=1 \
	BTLENAME_USE_SCANRSP_EN=0 CRASH_FAST_RST_EN=0 \
    $DUT_CMD USER_BURN_ADDR_PAIR_EN=1 EVB_DEMO_EN=1 GEN_LIB=0 $BES_LIB_DIR"
    btname="(STEREO)"
}
#-------------------------------------------------------------------------------------------------------------------------
chmod 777 ./tools/fill_sec_base.pl
if [ "$CMD2" = "clean" ] || [ "$CMD2" = "CLEAN" ]
then
    echo "clean"
    $build_cmd clean
    exit 
fi

if [ "$CMD2" = "lst" ] || [ "$CMD2" = "LST" ]
then
    echo "all lst"
    $build_cmd all lst
    exit
fi

rm -rf out/$CHIP_TYPE/apps/besui/besui_common.o
echo "|***********************************************|"
echo "|                start build app                |"
echo "|***********************************************|"

#-------------------------------------------------------------------------------------------------------------------------
BIN_FILE="out/$CHIP_TYPE/$CHIP_TYPE.bin"
COMPILE_CMD=$build_cmd

check_bin_exists() {
    if [ -f "$BIN_FILE" ]; then
        return 0
    else
        return 1
    fi
}

# get time(Linux & macOS)
get_file_time() {
    local file=$1
    local os=$(uname)
    if [ "$os" = "Linux" ]; then
        stat -c "%Y" "$file"
    elif [ "$os" = "Darwin" ]; then
        stat -f "%B" "$file"
    else
        echo "Do not support os: $os"
        exit 1
    fi
}

main_build_check() 
{
    # record build time
    if check_bin_exists; then
        pre_time=$(get_file_time "$BIN_FILE")
        # echo "Before build $BIN_FILE time: $pre_time"
    else
        pre_time=0
        # echo "Can not find bin $BIN_FILE, record 0"
    fi
    
    # echo "Start build app: $COMPILE_CMD"
    $COMPILE_CMD
    
    if ! check_bin_exists; then
        echo "❌ Build failed: no $BIN_FILE"
        exit 1
    fi
    
    post_time=$(get_file_time "$BIN_FILE")
    # echo "After build $BIN_FILE time: $post_time"
    
    # compare time
    if [ $post_time -gt $pre_time ]; then
        echo "✅ Build success: $BIN_FILE already update"
        # exit 0
    else
        echo "❌ Build failed: $BIN_FILE no update"
        exit 1
    fi
}

main_build_check

#-------------------------------------------------------------------------------------------------------------------------
echo "|***********************************************|"
echo "|                ota process start              |"
echo "|***********************************************|"
# BESUI_TIME=$(date +"%s.%N")
BESUI_DATE_TIME=$(date +"%Y-%m-%d %T.%N")
printf "[BESUI_DATE_TIME = %-s]\n" "$BESUI_DATE_TIME"
echo " "

echo "project = "$PRONAME
echo "btname  = "$btname

tool_path=tools/besui_tool
path_bin=out/$CHIP_TYPE/$CHIP_TYPE.bin
ota_out_bin=out/prod_test/ota_copy/ota_copy.bin

#-------------------------------------------------------------------------------------------------------------------------
otabin_path=$tool_path/ota_bin
rm -rf $otabin_path
mkdir $otabin_path
cp -f $ota_out_bin $otabin_path

chmod 777 ./tools/build_compressed_ota.sh

path_del=$tool_path
delname="*BES2710*"
find $path_del -type f -name "$delname" -exec rm {} \;
echo "delete "$delname.bin

cp -f $path_bin $tool_path/123.bin
python tools/generate_crc32_of_image.py $tool_path/123.bin
mv $tool_path/123.bin.converted.bin $tool_path/crc123.bin
./tools/build_compressed_ota.sh $tool_path/crc123.bin $tool_path/converted.bin
path_ota_bin=$tool_path/converted.bin
# python tools/generate_crc32_of_image.py $tool_path/converted.bin
# path_ota_bin=$tool_path/converted.bin.converted.bin
# rm -rf $tool_path/converted.bin

#-------------------------------------------------------------------------------------------------------------------------
DATE=`date +%Y%m%d` 
echo date = $DATE

keyword_ver="SW_VERSION=" 
VERSION="V0.0.1"
VERSION=$(sed -n '/'$keyword_ver'/p' $path_bin) 
VERSION=${VERSION##*=}
echo VERSION=$VERSION

keyword_crc="CRC32_OF_IMAGE=0x"
crc_path=$tool_path/crc123.bin
# CRC_VAL="0x00000000"
CRC_VAL=$(sed -n '/'$keyword_crc'/p' $crc_path)
CRC_VAL=${CRC_VAL##*0x}
echo CRC = $CRC_VAL

name_wire=${PRONAME}_${CPU_TYPE}_${VERSION}_${DATE}_WIRE_$CRC_VAL${btname}.bin
echo "wire file = "$name_wire
mv $tool_path/crc123.bin "$tool_path/$name_wire"
rm -rf $tool_path/123.bin

#-------------------------------------------------------------------------------------------------------------------------
#OTA name process
name_ota=${PRONAME}_${CPU_TYPE}_${VERSION}_${DATE}_OTA_$CRC_VAL${btname}.bin  #ota file in phone
echo "ota  file = "$name_ota
mv $path_ota_bin "$tool_path/$name_ota"
#-------------------------------------------------------------------------------------------------------------------------

rm -rf crc.bin
rm -rf magic.bin
echo "delete no use bin"
echo "|***********************************************|"
echo "|                ota process end                |"
echo "|***********************************************|"
echo " "
exit 0

