#!/bin/bash
input_value=$1

# SECURE_BOOT_FLAGS="SECURE_BOOT=1 USER_SECURE_BOOT=1"
# CRYPT_LOAD_FLAGS="LARGE_SE_RAM=1"
PROJECT_FUNC_DEMO="SE_OTP_DEMO_TEST=1 CMSE_CRYPT_TEST_DEMO=1 MBEDTLS_CONFIG_FILE="config-rsa.h""

CUSTOMER_LOAD_SECTION_FLAGS="CUSTOMER_LOAD_SRAM_TEXT_RAMX_SECTION_SIZE=0x1000 CUSTOMER_LOAD_RAM_DATA_SECTION_SIZE=0x1000 CUSTOMER_LOAD_ENC_DEC_RECORD_SECTION_SIZE=0x4000"
PROJECT_CFG_FLAGS="FLASH_SIZE=0x400000 SPA_AUDIO_ENABLE=1 SPA_AUDIO_SEC=1 "
CP_DISABLE_FLAGS="CHIP_HAS_CP=0 A2DP_CP_ACCEL=0 SCO_CP_ACCEL=0 NO_OVERLAY=1"

#EXTRA_PROJECT_FEATURE_CFG_FLAGS="SPEECH_TX_2MIC_NS7=1 SPEECH_TX_MIC_FIR_CALIBRATION=1 TOTA_v2=1 BESUI_APP_EN=1 GFPS_ENABLE=1"

CHIP_TYPES="CHIP=best1306 SECURE_BOOT_ED25519_DISABLE=1"-

function project_make() {
if [ "$input_value" = "arm_cmse" ] ;then
	echo "================================ make allclean =========================="
	make T=arm_cmse DEBUG=1 -j40 clean
	echo "================================ arm_cmse build =========================="
	# make T=arm_cmse DEBUG=1 -j40 FLASH_SECURITY_REGISTER=1 CHIP=best1501p TZ_ROM_UTILS_IF=1 ROM_UTILS_ON=1\
		# $PROJECT_CFG_FLAGS $CRYPT_LOAD_FLAGS $SECURE_BOOT_FLAGS
	make T=arm_cmse DEBUG=1 -j40 FLASH_SECURITY_REGISTER=1 TZ_ROM_UTILS_IF=1 ROM_UTILS_ON=1 CRC32_ROM=1 CMSE_RAM_RAMX_LEND_NSE=1 \
		$CHIP_TYPES \
		$PROJECT_CFG_FLAGS $CRYPT_LOAD_FLAGS $PROJECT_FUNC_DEMO $SECURE_BOOT_FLAGS 

elif [ "$input_value" = "arm_cmns" ] ;then
	echo "================================ make allclean =========================="
	make T=arm_cmns DEBUG=1 -j40 clean
	echo "================================ arm_cmns build =========================="
	make T=arm_cmns DEBUG=1 -j40  $CHIP_TYPES  $PROJECT_CFG_FLAGS $CRYPT_LOAD_FLAGS
elif [ "$input_value" = "bootloader" ] ;then
	echo "================================ make allclean =========================="
	make T=prod_test/ota_copy CHIP=best1501p DEBUG=1 -j40 clean
	echo "================================ arm_cmns build =========================="
	make T=prod_test/ota_copy DEBUG=1 -j40  FLASH_SECURITY_REGISTER=1 \
		$CHIP_TYPES \
		$PROJECT_CFG_FLAGS $SECURE_BOOT_FLAGS
elif [ "$input_value" = "best2600z_without_se" ] ;then
	echo "================================ make allclean =========================="
	make T=best1501p_ibrt DEBUG=1 -j40 clean
	echo "================================ best2600z build =========================="
	make T=best1501p_ibrt DEBUG=1 -j40 $PROJECT_CFG_FLAGS SPA_AUDIO_SEC=0
elif [ "$input_value" = "best2600z_with_se" ] ;then
	echo "================================ make allclean =========================="
	make T=best1501p_ibrt DEBUG=1 -j40 clean
	echo "================================ best2600z_with_se build =========================="
#	make T=best1501p_ibrt DEBUG=1 -j40 ARM_CMNS=1 LIBC_ROM=0 OTA_CODE_OFFSET=0x80000  LARGE_SE_RAM=1 NO_CP=1 A2DP_CP_ACCEL=0 SCO_CP_ACCEL=0 SPA_AUDIO_ENABLE=1
	# make T=best1501p_ibrt DEBUG=1 -j40 LIBC_ROM=0 OTA_CODE_OFFSET=0x80000 NO_CP=0 A2DP_CP_ACCEL=1 SCO_CP_ACCEL=1 ARM_CMNS=1 \
	# 	POWER_ON_ENTER_TWS_PAIRING_ENABLED=1 IGNORE_POWER_ON_KEY_DURING_BOOT_UP=1 TZ_ROM_UTILS_IF=1 A2DP_CP_ACCEL=0 SCO_CP_ACCEL=0\
	# 	$PROJECT_CFG_FLAGS $CRYPT_LOAD_FLAGS $SECURE_BOOT_FLAGS USER_SECURE_BOOT=0

	make T=best1501p_ibrt DEBUG=1 -j40 LIBC_ROM=0 OTA_CODE_OFFSET=0x80000 ARM_CMNS=1 TZ_ROM_UTILS_IF=1\
		POWER_ON_ENTER_TWS_PAIRING_ENABLED=1 IGNORE_POWER_ON_KEY_DURING_BOOT_UP=1 \
		$CP_DISABLE_FLAGS \
		$PROJECT_CFG_FLAGS $CRYPT_LOAD_FLAGS $SECURE_BOOT_FLAGS USER_SECURE_BOOT=0

elif [ "$input_value" = "best2600iuc_with_se" ] ;then
	echo "================================ make allclean =========================="
	make T=best1306_ibrt DEBUG=1 -j40 clean
	echo "================================ best2600iuc_with_se build =========================="
	make T=best1306_ibrt DEBUG=1 -j40 LIBC_ROM=0 OTA_CODE_OFFSET=0x80000 ARM_CMNS=1 TZ_ROM_UTILS_IF=1 ROM_UTILS_ON=0 \
		POWER_ON_ENTER_TWS_PAIRING_ENABLED=1 IGNORE_POWER_ON_KEY_DURING_BOOT_UP=1 \
		$CP_DISABLE_FLAGS $CUSTOMER_LOAD_SECTION_FLAGS \
		$PROJECT_CFG_FLAGS $CRYPT_LOAD_FLAGS $SECURE_BOOT_FLAGS USER_SECURE_BOOT=0 \
		$EXTRA_PROJECT_FEATURE_CFG_FLAGS
fi
}

project_make