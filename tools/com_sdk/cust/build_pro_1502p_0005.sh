#!/bin/bash

TARGET_LIST="best1502p"
CHIPID="best1502p"
FLASH_SIZE="0x400000"

COMMAND=$1
# set -e

BES_LIB_DIR="lib/bes/best1502p/PRO_0005"

build_otacopy_cmd="make T=prod_test/ota_copy CHIP=$CHIPID DEBUG=1 -j64 \
	FLASH_SECURITY_REGISTER=1 FLASH_SIZE=$FLASH_SIZE \
	SPA_AUDIO_ENABLE=1 SPA_AUDIO_SEC=1 \
	SECURE_BOOT=1 USER_SECURE_BOOT=1 OTA_REBOOT_FLASH_REMAP=1 \
	DOLBY_AUDIO_ENABLE=1 DOLBY_AUDIO_DAW_ENABLE=1 \
    BES_LIB_DIR=$BES_LIB_DIR \
"
build_cmse_cmd="make T=arm_cmse DEBUG=1 -j64 CHIP=$CHIPID \
	FLASH_SIZE=$FLASH_SIZE FLASH_SECURITY_REGISTER=1 \
	TZ_ROM_UTILS_IF=0 ROM_UTILS_ON=0 CRC32_ROM=1 CMSE_RAM_RAMX_LEND_NSE=1 \
	SPA_AUDIO_ENABLE=1 SPA_AUDIO_SEC=1 SE_OTP_DEMO_TEST=1 CMSE_CRYPT_TEST_DEMO=1 MBEDTLS_CONFIG_FILE="config-rsa.h" \
	SECURE_BOOT=1 USER_SECURE_BOOT=1 \
	OTA_CODE_OFFSET=0x20000 LARGE_SE_RAM=0 FLASH_LOW_SPEED=1 ULTRA_LOW_POWER=1 \
	DOLBY_AUDIO_ENABLE=1 DOLBY_AUDIO_DAW_ENABLE=1 GEN_LIB=1 \
    BES_LIB_DIR=$BES_LIB_DIR \
"
build_app_cmd="make T=$CHIPID -j64 DEBUG=1 \
	A2DP_CP_ACCEL=1 SCO_CP_ACCEL=1 CHIP_HAS_CP=1 \
	LIBC_ROM=0 ARM_CMNS=1 TZ_ROM_UTILS_IF=0 ROM_UTILS_ON=0 LARGE_SE_RAM=0 \
	SECURE_BOOT=1 USER_SECURE_BOOT=0 \
	FLASH_SIZE=$FLASH_SIZE TRACE_BUF_SIZE=24*1024 RAMCP_SIZE=0x4E000 RAMCPX_SIZE=0x11000 OS_DYNAMIC_MEM_SIZE=0x5900 FAST_XRAM_SECTION_SIZE=0x9800 \
	DOLBY_AUDIO_ENABLE=1 DOLBY_AUDIO_DAW_ENABLE=1 SPA_AUDIO_ENABLE=1 SPA_AUDIO_SEC=1 \
	TOTA_v2=1 BLE=0 GFPS_ENABLE=0 \
    ANC_ENABLE=1 \
    SPEECH_TX_DC_FILTER=1 SPEECH_TX_2MIC_NS8=1 SPEECH_TX_EQ=1 \
	BES_OTA=1 ARM_CMNS_OTA=1 OTA_BIN_COMPRESSED=1 OTA_CODE_OFFSET=0x80000 \
	POWER_ON_ENTER_TWS_PAIRING_ENABLED=1 FREEMAN_ENABLED_STERO=1 \
    BES_LIB_DIR=$BES_LIB_DIR \
"

build_ota_test_cmd="make T=prod_test/ota_copy CHIP=$CHIPID DEBUG=1 -j64 \
	BES_OTA=1 OTA_BIN_COMPRESSED=1 FLASH_REMAP=0 \
	FLASH_SIZE=$FLASH_SIZE OTA_BOOT_SIZE=0x18000 CHIP_HAS_CP=0 SINGLE_WIRE_DOWNLOAD=1
"
build_app_test_cmd="make T=$CHIPID -j64 DEBUG=1 \
	A2DP_CP_ACCEL=1 SCO_CP_ACCEL=1 CHIP_HAS_CP=1 \
	FLASH_SIZE=$FLASH_SIZE TRACE_BUF_SIZE=24*1024 RAMCP_SIZE=0x4E000 RAMCPX_SIZE=0x11000 OS_DYNAMIC_MEM_SIZE=0x5900 FAST_XRAM_SECTION_SIZE=0x9800 \
	TOTA_v2=1 BLE=0 GFPS_ENABLE=0 \
    ANC_ENABLE=1 \
    SPEECH_TX_DC_FILTER=1 SPEECH_TX_2MIC_NS8=1 SPEECH_TX_EQ=1 \
	BES_OTA=1 OTA_BIN_COMPRESSED=1 OTA_CODE_OFFSET=0x80000 \
	POWER_ON_ENTER_TWS_PAIRING_ENABLED=1 FREEMAN_ENABLED_STERO=1 \
"

if [[ "$COMMAND" == "clean" ]];
then
    $build_otacopy_cmd clean
    $build_cmse_cmd clean
    $build_app_cmd clean
elif [[ "$COMMAND" == "gen_lib" ]];
then
echo -e "\n*********** ota  build start gen_lib ***********\n"
    $build_otacopy_cmd GEN_LIB=1  
echo -e "\n*********** cmse build start gen_lib ***********\n"
    $build_cmse_cmd GEN_LIB=1
echo -e "\n*********** app  build start gen_lib ***********\n"
    $build_app_cmd GEN_LIB=1
	exit 0
elif [[ "$COMMAND" == 'lst' ]];
then
    $build_otacopy_cmd lst all ||{ echo "$LINENO command failed"; exit 1; }
    $build_cmse_cmd lst all ||{ echo "$LINENO command failed"; exit 1; }
    $build_app_cmd lst all ||{ echo "$LINENO command failed"; exit 1; }
elif [[ "$COMMAND" == 'all' ]];
then
echo -e "\n*********** ota  build start all ***********\n"
    $build_otacopy_cmd ||{ echo "$LINENO command failed"; exit 1; }
echo -e "\n*********** cmse build start all ***********\n"
    $build_cmse_cmd ||{ echo "$LINENO command failed"; exit 1; }  
echo -e "\n*********** app  build start all ***********\n"
    $build_app_cmd ||{ echo "$LINENO command failed"; exit 1; }
else
    $build_ota_test_cmd ||{ echo "$LINENO command failed"; exit 1; }
    $build_app_test_cmd ||{ echo "$LINENO command failed"; exit 1; }
	exit 0
fi

#-------------------------------------------------------------------------------------------------------------------------
#copy ota bin
otabin_path=tools/algo_tool/ota_bin
rm -rf $otabin_path
mkdir $otabin_path

if [[ "$build_otacopy_cmd" ]];then
cp -f out/prod_test/ota_copy/ota_copy.bin $otabin_path
fi

if [[ "$build_cmse_cmd" ]];then
cp -f out/arm_cmse/arm_cmse.bin $otabin_path
fi
#-------------------------------------------------------------------------------------------------------------------------
chmod 777 ./tools/algo_tool/find.sh
dirname="tools/algo_tool"
sed -i 's/\r//' $dirname/find.sh
. $dirname/find.sh

#-------------------------------------------------------------------------------------------------------------------------
# [ "$bin_ota_sh" ] &&{
	chmod 777 ./tools/algo_tool/bin_ota.sh
	dirname="tools/algo_tool"
	sed -i 's/\r//' $dirname/bin_ota.sh
	. $dirname/bin_ota.sh
# }
#-------------------------------------------------------------------------------------------------------------------------
