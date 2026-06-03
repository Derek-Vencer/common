CHIP		?= best1501

CHIP_SUBSYS ?= sensor_hub

DEBUG		?= 1

DEBUG_PORT	?= 2

FAULT_DUMP	?= 1

NO_ISPI ?= 1
export NO_ISPI

NOSTD		?= 0

ifneq ($(NOSTD),1)
RTOS 		?= 1
#KERNEL 		:=  FREERTOS
endif

ifeq ($(RTOS),1)
USE_MEM_CFG ?= 1
else
NOAPP		?= 1
export NOAPP
endif

ifeq ($(PT_APP),1)
USE_MEM_CFG ?= 1
endif

export SENSOR_BT_ENALBE ?= 0

AUDIO_RESAMPLE ?= 1

export OSC_26M_X4_AUD2BB ?= 1

export ULTRA_LOW_POWER ?= 1

export AF_DEVICE_BT_PCM ?= 0

export NO_OVERLAY ?= 1

export USE_OS_MAIN ?= 1

ifeq ($(CHIP),best1502x)
export OSTICK_USE_FAST_TIMER ?= 1
export FAST_TIMER_COMPENSATE ?= 1
endif

ifeq ($(SENS_CODEC_ADDA_LOOP),1)
export NOAPP			:= 1
export VAD_CODEC_TEST	?= 1
else ifeq ($(SENS_CODEC_VAD_PWR_TEST),1)
export SENSOR_HUB_MINIMA	?= 0
export VOICE_DETECTOR_EN	?= 1
export VAD_USE_SAR_ADC		?= 0
export VAD_ADPT_CLOCK_FREQ	?= 2
export VAD_ADPT_TEST_ENABLE	?= 1
export FULL_WORKLOAD_MODE_ENABLED ?= 1
export THIRDPARTY_GSOUND	?=0
endif
ifeq ($(MINIMA_TEST),1)
export SENSOR_HUB_MINIMA ?= 1
endif

export OPTIMIZE_CODE_SIZE ?= 1

export FULL_WORKLOAD_MODE_ENABLED ?= 0

init-y		:=
EXT_SRC_DIR		+= \
	platform/cmsis/ \
	platform/hal/ \

ifeq ($(USE_OS_MAIN),1)
ifneq ($(KERNEL),NUTTX) # FIXME:need better method
EXT_SRC_DIR		+= tests/sensor_hub/ utils/hwtimer_list/
endif
endif

ifeq ($(SENSOR_HUB_BSP_TEST), 1)
EXT_SRC_DIR 		+= \
	tests/besair_platform/cmd_test/ \
	tests/besair_platform/bsp_tester/gpio_tester/ \
	tests/besair_platform/bsp_tester/i2c_tester/ \
	tests/besair_platform/bsp_tester/spi_tester/
KBUILD_CPPFLAGS += \
	-Itests/besair_platform
endif

export VPU_NAME ?= LSM6DSV16BX

export VPU_CFG_ON_SENSOR_HUB ?= 0

ifeq ($(VPU_CFG_ON_SENSOR_HUB),1)
EXT_SRC_DIR      += services/bone_sensor/vpu_cfg/
# KBUILD_CPPFLAGS += -DVPU_CFG_TEST
KBUILD_CPPFLAGS += -DVPU_I2C_ID=3
endif

ifeq ($(SNDP_VAD_ENABLE),1)
EXT_SRC_DIR		+= thirdparty/soundplus_lib/soundplus_voice_ai/
export DSP_LIB ?= 1
KBUILD_CPPFLAGS += -DSNDP_VAD_ENABLE
endif

export THIRDPARTY_ALEXA		?= 0
export THIRDPARTY_GSOUND	?= 0
export THIRDPARTY_BIXBY		?= 0
export THIRDPARTY_BES		?= 0

ifneq ($(filter 1, $(THIRDPARTY_ALEXA) $(THIRDPARTY_GSOUND) $(THIRDPARTY_BES) $(THIRDPARTY_BIXBY)),)
export THIRDPARTY_LIB := 1
export AI_VOICE ?= 1
endif

ifneq ($(THIRDPARTY_LIB),)
ifeq ($(THIRDPARTY_BIXBY),1)
NOSTD	:= 0
SOFT_FLOAT_ABI := 1
EXT_SRC_DIR		+= thirdparty/senshub_lib/bixby/
endif 	### ifeq ($(THIRDPARTY_BIXBY),1)

ifeq ($(THIRDPARTY_GSOUND),1)
export DSP_LIB ?= 1
NOSTD	:= 0
EXT_SRC_DIR		+= thirdparty/senshub_lib/gsound/
endif 	### ifeq ($(THIRDPARTY_GSOUND),1)

ifeq ($(THIRDPARTY_ALEXA),1)
export DSP_LIB ?= 1
NOSTD	:= 0
#export AF_STACK_SIZE ?= 5120	##5*1024
EXT_SRC_DIR		+= thirdparty/senshub_lib/alexa/
endif	### ifeq ($(THIRDPARTY_ALEXA),1)

endif	### ifneq ($(THIRDPARTY_LIB),)

ifeq ($(AUDIO_DSP_ACCEL_USE_SENS),1)
KBUILD_CPPFLAGS += -DAUDIO_DSP_ACCEL_USE_SENS
export APP_MCPP_CLI := SENS
export APP_MCPP_SRV ?= thirdparty
export APP_MCPP_BTH_SENS := 1
endif

ifneq ($(SPEECH_ALGO_DSP),)
export APP_MCPP_CLI := SENS
export APP_MCPP_SRV := $(SPEECH_ALGO_DSP)
export APP_MCPP_BTH_SENS := 1
endif

ifeq ($(APP_MCPP_CLI),SENS)
EXT_SRC_DIR += services/mcpp/ utils/kfifo/ utils/stream_mcps/
KBUILD_CPPFLAGS += -DAPP_MCPP_CLI
KBUILD_CPPFLAGS += -DAPP_MCPP_CLI_SENS
export APP_RPC := 1
export SPEECH_LIB := 1
export APP_RPC_MCU_SENSOR_EN := 1
endif

ifneq ($(APP_MCPP_SRV),)
KBUILD_CPPFLAGS += -DAPP_MCPP_SRV
endif

ifeq ($(APP_RPC_MCU_SENSOR_EN), 1)
EXT_SRC_DIR += apps/app_rpc/
KBUILD_CPPFLAGS += -DAPP_RPC_MCU_SENSOR_EN
endif

ifeq ($(SENS_TRC_TO_MCU),1)
export TRACE_BUF_SIZE ?= 0
export TRACE_GLOBAL_TAG ?= 1
export DUMP_LOG_ENABLE ?= 1
export LOG_DUMP_SECTION_SIZE ?= 0
KBUILD_CFLAGS += -DSENS_TRC_TO_MCU
endif

#capsensor
# export CAPSENSOR_WEAR ?= 1
ifeq ($(CAPSENSOR_WEAR),1)
KBUILD_CFLAGS += -DCAPSENSOR_WEAR
endif
ifeq ($(CAPSENSOR_TOUCH),1)
KBUILD_CFLAGS += -DCAPSENSOR_TOUCH
endif

ifeq ($(CAPSENSOR_DEBUG),1)
KBUILD_CFLAGS += -DCAPSENSOR_DEBUG
endif

ifeq ($(I2S_TEST),1)
export SENS_I2S_DMA_ENABLE ?= 1
endif

KBUILD_CPPFLAGS += -Iplatform/cmsis/inc -Iplatform/hal -Itests/sensor_hub/inc
# FIXME:need better method to solve compile err for SPEECH_ARMCC_LIB=0 in nuttx
ifeq ($(KERNEL),NUTTX)
KBUILD_CPPFLAGS	+= -I../../rtos/nuttx/libs/libxx/libcxx/include
endif

ifeq ($(RTOS),1)
KBUILD_CPPFLAGS += -DconfigTOTAL_HEAP_SIZE=0x4000
endif

export VOICE_DETECTOR_EN ?= 0

ifeq ($(SENSOR_HUB_MINIMA),1)
LARGE_SENS_RAM ?= 1
else ifeq ($(VOICE_DETECTOR_EN),1)
ifneq ($(filter best1501 best1501p,$(CHIP)),)
LARGE_SENS_RAM ?= 1
else
LARGE_SENS_RAM ?= 0
endif
else
LARGE_SENS_RAM ?= 0
endif

ifeq ($(VOICE_DETECTOR_EN),1)
CODEC_VAD_CFG_BUF_SIZE	?= 0x18000
SENS_RAM_USED_SIZE		?= 0x68000
else
SENS_RAM_USED_SIZE		?= 0x80000
endif

ifeq ($(CHIP),best1600)
ifneq ($(NOAPP),1)
ifneq ($(LARGE_SENS_RAM),1)
KBUILD_CPPFLAGS += -DSENS_RAM_SIZE=$(SENS_RAM_USED_SIZE)
endif
endif
endif

ifeq ($(OPTIMIZE_CODE_SIZE),1)
export TRACE_BUF_SIZE			?= 0x400
export AF_STACK_SIZE			?= 0x400
export APP_THREAD_STACK_SIZE	?= 0x400
export OS_STACK_SIZE			?= 0x400
export OS_IDLESTKSIZE			?= 0x400
export OS_TIMER_THREAD_STACK_SIZE	?= 0x400
export OS_DYNAMIC_MEM_SIZE			?= 0x4000

# Rx/Tx msg packet data size
export APP_CORE_BRIDGE_MAX_DATA_PACKET_SIZE ?= 0x200
# the size of Tx mailbox = 0x200*8/2=2KB
export APP_CORE_BRIDGE_MAX_XFER_DATA_SIZE ?= 0x200
# the size of Rx Queue
export APP_CORE_BRIDGE_RX_BUFF_SIZE ?= 0x800
# The stack size of Core bridge RX thread
export APP_CORE_BRIDGE_RX_THREAD_STACK_SIZE ?= 0x1000
# The stack size of Core bridge TX thread
export APP_CORE_BRIDGE_TX_THREAD_STACK_SIZE ?= 0xC00
ifeq ($(CAPSENSOR_ENABLE),1)
export APP_CORE_BRIDGE_MAX_DATA_PACKET_SIZE := 0x200
export APP_CORE_BRIDGE_MAX_XFER_DATA_SIZE   := 0x100
export APP_CORE_BRIDGE_RX_BUFF_SIZE			:= 0x480
export APP_CORE_BRIDGE_RX_THREAD_STACK_SIZE := 0xC00
export APP_CORE_BRIDGE_TX_THREAD_STACK_SIZE := 0x800
endif
ifeq ($(VOICE_DETECTOR_EN),1)
export APP_THREAD_STACK_SIZE				:= 0x800
export APP_CORE_BRIDGE_MAX_DATA_PACKET_SIZE := 0x200
export APP_CORE_BRIDGE_MAX_XFER_DATA_SIZE   := 0x100
export APP_CORE_BRIDGE_RX_BUFF_SIZE			:= 0x800
export APP_CORE_BRIDGE_RX_THREAD_STACK_SIZE := 0x1000
export APP_CORE_BRIDGE_TX_THREAD_STACK_SIZE := 0xC00
endif
endif

ifeq ($(VOICE_DETECTOR_EN),1)
KBUILD_CPPFLAGS += -DVOICE_DETECTOR_EN -Iservices/audioflinger -Iapps/common
endif

export OS_THREAD_TIMING_STATISTICS_ENABLE ?= 0
ifeq ($(OS_THREAD_TIMING_STATISTICS_ENABLE),1)
KBUILD_CPPFLAGS += -DOS_THREAD_TIMING_STATISTICS_ENABLE
KBUILD_CPPFLAGS += -DOS_THREAD_TIMING_STATISTICS_PEROID_MS=6000
endif

ifeq ($(USE_MEM_CFG),1)
-include $(srctree)/config/$(CHIP)/mem_cfg.mk
endif

KBUILD_CFLAGS +=

LIB_LDFLAGS +=

CFLAGS_IMAGE +=

LDFLAGS_IMAGE +=

ifneq ($(filter 1,$(SENSOR_HUB_MINIMA) $(SENSOR_TEST)),)
CFLAGS_IMAGE += -u _printf_float -u _scanf_float
endif

ifeq ($(CHIP)-$(SENSOR_BT_ENALBE),best1502x-1)
EXT_SRC_DIR		:= apps/ services/
WATCHER_DOG := 0
LIBC_ROM    := 0
HW_DAC_IIR_EQ_PROCESS := 0
export SUPPORT_BATTERY_REPORT := 0
export VOICE_PROMPT := 0
export FACTORY_MODE := 0
export ENGINEER_MODE := 0
MIX_AUDIO_PROMPT_WITH_A2DP_MEDIA_ENABLED := 0
POWER_ON_ENTER_FREEMAN_PAIRING_ENABLED := 1
POWER_ON_ENTER_BOTH_SCAN_MODE := 1
IGNORE_POWER_ON_KEY_DURING_BOOT_UP := 1
export TRACE_BUF_SIZE ?= 4096
export TRACE_BAUD_RATE ?= 921600
export USE_OS_MAIN ?= 1
KBUILD_CFLAGS += -D__EARPHONE_STAY_BOTH_SCAN__

export APP_THREAD_STACK_SIZE	:= 0x800
export APP_CORE_BRIDGE_MAX_DATA_PACKET_SIZE := 0x800
export APP_CORE_BRIDGE_MAX_XFER_DATA_SIZE := 0x800
export APP_CORE_BRIDGE_RX_BUFF_SIZE := 0x2000
export APP_CORE_BRIDGE_RX_THREAD_STACK_SIZE := 0x2200
export APP_CORE_BRIDGE_TX_THREAD_STACK_SIZE := 0x1200

ifeq ($(USE_OS_MAIN),1)
EXT_SRC_DIR		+= apps/sensorhub/ tests/sensor_hub/
endif

ifneq ($(SUBSYS_FLASH_BOOT),1)
LDS_FILE	:= subsys.lds
endif
include config/best1502x_ep/target.mk
else
core-y := $(EXT_SRC_DIR)
ifeq ($(SUBSYS_FLASH_BOOT),1)
core-y		+= platform/drivers/norflash/
LDS_FILE	:= best1000.lds
else
LDS_FILE	:= subsys.lds
endif
endif

export SPA_AUDIO_ENABLE := 0
ifeq ($(SPA_AUDIO_ENABLE),1)
export USE_THIRDPARTY := 1
KBUILD_CPPFLAGS += -DSPA_AUDIO_ENABLE
KBUILD_CPPFLAGS += -DSPA_AUDIO_SENS
core-y		+= thirdparty/userapi/spa_dummy_app/
endif

ifeq ($(AUDIO_DEBUG),1)
export AUDIO_DEBUG
KBUILD_CPPFLAGS += -DAUDIO_DEBUG
endif

ifneq ($(SENS_RAMRET_BASE),)
KBUILD_CPPFLAGS += -DSENS_RAMRET_BASE=$(SENS_RAMRET_BASE)
endif

ifeq ($(AUDIO_ANC_FIR_HW),1)
export VOICE_ASSIST_FF_FIR_LMS_MULTI_THREAD ?= 1
KBUILD_CPPFLAGS += -DAUDIO_ANC_FIR_HW
core-y += apps/voice_assist/
core-y += multimedia/
endif

export VOICE_ASSIST_FF_FIR_LMS_CP_ACCEL ?= 0
ifeq ($(VOICE_ASSIST_FF_FIR_LMS_CP_ACCEL),1)
KBUILD_CPPFLAGS += -DVOICE_ASSIST_FF_FIR_LMS_CP_ACCEL
endif

export VOICE_ASSIST_FF_FIR_LMS_MULTI_THREAD ?= 0
ifeq ($(VOICE_ASSIST_FF_FIR_LMS_MULTI_THREAD),1)
KBUILD_CPPFLAGS += -DVOICE_ASSIST_FF_FIR_LMS_MULTI_THREAD
endif

export FREEMAN_ENABLED_STERO ?= 0
ifeq ($(FREEMAN_ENABLED_STERO),1)
export VOICE_ASSIST_STEREO ?= 1
KBUILD_CPPFLAGS += -DFREEMAN_ENABLED_STERO
endif

export VOICE_ASSIST_STEREO ?= 0
ifeq ($(VOICE_ASSIST_STEREO),1)
KBUILD_CPPFLAGS += -DVOICE_ASSIST_STEREO
endif

export VOICE_ASSIST_STEREO ?= 0
ifeq ($(ANC_ASSIST_VPU),1)
KBUILD_CPPFLAGS += -DANC_ASSIST_VPU
endif

ifeq ($(CP_IN_SAME_EE),1)
core-y += platform/drivers/cp_accel/
endif
