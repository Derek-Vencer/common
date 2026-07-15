CHIP		?= best1501p

ifeq ($(SLEEP_PD_CURRENT_TEST),1)
KBUILD_CPPFLAGS += -DSLEEP_PD_CURRENT_TEST

CURRENT_TEST 				?= 1
CORE_SLEEP_POWER_DOWN 		?= 1
endif

ifeq ($(CURRENT_TEST),1)
DEBUG_PORT 					?= 2
NOSTD						?= 1
export OSC_26M_X4_AUD2BB	?= 1
export ULTRA_LOW_POWER		?= 1
export POWER_MODE			?= DIG_DCDC
export SLEEP_TEST			?= 1
export NO_TIMER				?= 1
export FLASH_DEEP_SLEEP		?= 1
export FORCE_FLASH1_SLEEP 	?= 1
ifeq ($(CHIP),best1600)
ifeq ($(CHIP_SUBSYS),)
SYS_USE_BTH_FLASH 			?=1
else ifeq ($(CHIP_SUBSYS),bth)
BTH_USE_SYS_FLASH 			?=1
endif
endif
else ifeq ($(SENS_CPU_BOOT_TEST),1)
DEBUG_PORT 					?= 2
NOSTD						?= 1
AUDIO_RESAMPLE				?= 1
export OSC_26M_X4_AUD2BB	?= 1
export ULTRA_LOW_POWER		?= 1
export POWER_MODE			?= DIG_DCDC
export SENSOR_HUB_TEST		?= 1
export SENSOR_HUB			?= 1
else ifeq ($(SENS_CODEC_ADDA_LOOP),1)
DEBUG_PORT 					?= 2
NOSTD						?= 1
AUDIO_RESAMPLE				?= 1
export AF_DEVICE_I2S		?= 0
export AF_DEVICE_INT_CODEC	?= 1
export OSC_26M_X4_AUD2BB	?= 1
export ULTRA_LOW_POWER		?= 1
export POWER_MODE			?= DIG_DCDC
export SENSOR_HUB_TEST		?= 1
export SENSOR_HUB			?= 1
export VAD_CODEC_TEST		?= 1
export VAD_CODEC_LOOP		?= 1
export VAD_USE_SAR_ADC		?= 0
ifeq ($(VAD_USE_SAR_ADC),1)
export VOICE_DETECTOR_SENS_EN	?= 1
endif
else ifeq ($(SENS_CODEC_VAD_PWR_TEST),1)
DEBUG_PORT 					?= 2
AUDIO_RESAMPLE				?= 1
LARGE_SENS_RAM				?= 1
export AF_DEVICE_I2S		?= 0
export AF_DEVICE_INT_CODEC	?= 1
export OSC_26M_X4_AUD2BB	?= 1
export ULTRA_LOW_POWER		?= 1
export POWER_MODE			?= DIG_DCDC
export FLASH_LOW_SPEED      ?= 1
export SENSOR_HUB			?= 1
export SENSOR_HUB_TEST		?= 1
export SENSOR_HUB_MINIMA	?= 0
export VAD_CODEC_TEST		?= 0
export VAD_APP_TEST			?= 1
export VAD_KEY_EVT			?= 1
export VAD_USE_SAR_ADC		?= 0
export VOICE_DETECTOR_SENS_EN ?= 1
endif

ifeq ($(AF_TEST),1)
ifneq ($(filter 1,$(AF_EQ_TEST) $(ANC_APP) $(AUDIO_GENERATOR_TONE_TEST)),)
NOSTD						:= 0
endif

export AUDIO_OUTPUT_DC_AUTO_CALIB ?= 0
ifeq ($(AUDIO_OUTPUT_DC_AUTO_CALIB), 1)
export AUDIO_OUTPUT_DC_CALIB := 1
export AUDIO_OUTPUT_DC_CALIB_ANA := 0
export AUDIO_OUTPUT_SET_LARGE_ANA_DC ?= 0
export AUDIO_OUTPUT_DC_CALIB_DUAL_CHAN ?= 0
export AUDIO_OUTPUT_DIG_DC_DEEP_CALIB ?= 1
ifeq ($(CHIP),best1603)
export AUDIO_ANA_DC_CALIB_USE_COMP ?= 1
else ifeq ($(CHIP),best1700)
export AUDIO_ANA_DC_CALIB_USE_SDM1B ?= 1
endif
export DAC_DRE_ENABLE ?= 1
export CODEC_DAC_DC_NV_DATA ?= 0
export CODEC_DAC_DC_DYN_BUF ?= 1
export CODEC_DAC_DC_CHECK ?= 0
KBUILD_CPPFLAGS += -DAUDIO_OUTPUT_DC_AUTO_CALIB
endif

export AUDIO_ADC_DC_AUTO_CALIB ?= 0
ifeq ($(AUDIO_ADC_DC_AUTO_CALIB), 1)
export ADC_DC_CALIB_USE_FIXED_DC ?= 0
export AUDIO_ADC_DIG_DC_CALIB ?= 0
export CODEC_ADC_DC_DYN_BUF ?= 1
export CODEC_ADC_DC_NV_DATA ?= 0
ifeq ($(ADC_DC_CALIB_USE_FIXED_DC), 1)
KBUILD_CPPFLAGS += -DADC_DC_CALIB_USE_FIXED_DC
endif
ifeq ($(AUDIO_ADC_DIG_DC_CALIB), 1)
KBUILD_CPPFLAGS += -DAUDIO_ADC_DIG_DC_CALIB
endif
ifeq ($(CFG_ADC_DC_CALIB_MIC_DEV),)
CFG_ADC_DC_CALIB_MIC_DEV := 1
endif
ifeq ($(CFG_ADC_DC_CALIB_CH_NUM),)
CFG_ADC_DC_CALIB_CH_NUM := 1
endif
KBUILD_CPPFLAGS += -DAUDIO_ADC_DC_AUTO_CALIB
KBUILD_CPPFLAGS += -DCFG_ADC_DC_CALIB_MIC_DEV=$(CFG_ADC_DC_CALIB_MIC_DEV)
KBUILD_CPPFLAGS += -DCFG_ADC_DC_CALIB_CH_NUM=$(CFG_ADC_DC_CALIB_CH_NUM)
endif # AUDIO_ADC_DC_AUTO_CALIB
endif # AF_TEST

DEBUG		?= 1

FPGA		?= 0

DEBUG_PORT	?= 1

FLASH_CHIP	?= ALL

NOAPP		?= 1
export NOAPP

ifneq ($(NOSTD),1)

RTOS		?= 1

endif # NOSTD != 1

ifeq ($(MBED)-$(RTOS),1-1)
init-y		:=
core-y		:= platform/

else # !(MBED-RTOS)
init-y		:=
core-y		:= platform/main/ platform/cmsis/ platform/hal/ \
	platform/drivers/norflash/  platform/drivers/usb/usb_dev/

ifeq ($(NANDFLASH_SUPPORT),1)
core-y +=platform/drivers/nandflash/
endif

ifneq ($(ARM_CMSE),1)
core-y += platform/drivers/ana/  utils/heap/
endif

endif # !(MBED-RTOS)

ifneq ($(filter 1,$(RTOS) $(GRAPHIC_TEST)),)
USE_MEM_CFG ?= 1
endif


ifeq ($(BT_DRV_TEST),1)
ifneq ($(filter best2007 best2007p best1503 best1306p best1700 best1502p,$(CHIP)),)
include $(srctree)/config/$(CHIP)/bt_config.mk
ifeq ($(RF_INIT_XTAL_CAP_FROM_NV),1)
NEW_NV_RECORD_ENABLED ?= 1
ifeq ($(NEW_NV_RECORD_ENABLED),1)
KBUILD_CPPFLAGS += -DNEW_NV_RECORD_ENABLED
core-y      += services/nv_section/userdata_section/
endif
endif
endif
endif

export DMA_AUDIO_APP ?= 0
ifeq ($(DMA_AUDIO_APP),1)
export DMA_RPC_CLI ?= 1
-include $(srctree)/config/$(CHIP)/dma_aud_cfg.mk
endif

ifeq ($(SPEECH_ARMCC_LIB),1)
core-y += multimedia/speech/src/
endif

export TRACE_BAUD_RATE ?= 10*115200

KBUILD_CPPFLAGS += -Iplatform/cmsis/inc -Iplatform/hal -Iplatform/drivers/usb/usb_dev/inc

ifeq ($(ROM_BUILD),1)
LDS_FILE	?= rom.lds
ROM_SIZE	?= 0x80000
else ifeq ($(INTSRAM_RUN),1)
LDS_FILE	?= best1000_intsram.lds
else ifeq ($(NANDFLASH_BUILD),1)
LDS_FILE	?= best1000_nand.lds
else
LDS_FILE	?= best1000.lds
endif

ifeq ($(KERNEL), LITEOS_M)
export OSTICK_USE_FAST_TIMER := 0
KBUILD_CPPFLAGS += -DOS_HEAP_LINK_NAME=m_aucSysMem0 -DOS_HEAP_SIZE=0x80000
KBUILD_CPPFLAGS += -DOS_WRAP_MALLOC
LDFLAGS_IMAGE += --wrap malloc --wrap calloc --wrap free --wrap realloc
endif

export CMSIS_TEST ?= 0
ifeq ($(CMSIS_TEST),1)
LDS_CPPFLAGS += \
        -DCMSIS_TEST
endif

export USE_MULTI_FLASH ?= 0
ifeq ($(USE_MULTI_FLASH),1)
export FLASH1_SIZE ?= 0x400000
KBUILD_CPPFLAGS += \
        -DUSE_MULTI_FLASH \
        -DFLASH1_SIZE=$(FLASH1_SIZE)
LDS_CPPFLAGS += \
        -DUSE_MULTI_FLASH \
        -DFLASH1_SIZE=$(FLASH1_SIZE)
endif

KBUILD_CPPFLAGS +=

KBUILD_CFLAGS +=

KBUILD_CFLAGS +=

ifeq ($(USE_MEM_CFG),1)
-include $(srctree)/config/$(CHIP)/mem_config.mk
endif

ifeq ($(SENSOR_HUB),1)
ifeq ($(VOICE_DETECTOR_SENS_EN),1)
CODEC_VAD_CFG_BUF_SIZE	?= 0x18000
SENS_RAM_USED_SIZE		?= 0x68000
else
SENS_RAM_USED_SIZE		?= 0x80000
endif
KBUILD_CPPFLAGS += -DSENS_RAM_SIZE=$(SENS_RAM_USED_SIZE)
endif

ifeq ($(DSP_HIFI4_TEST),1)
export BTH_AS_MAIN_MCU ?= 1
export DSP_HIFI4_TRC_TO_MCU ?= 1
ifeq ($(DSP_DEBUG_PORT_MAPOUT),1)
KBUILD_CPPFLAGS += -DDSP_DEBUG_PORT_MAPOUT
KBUILD_CPPFLAGS += -DUART0_IOMUX_INDEX=32
endif
ifeq ($(DSP_COMBINE_BIN),1)
KBUILD_CPPFLAGS += -DDSP_COMBINE_BIN
endif
endif

ifneq ($(filter best1600,$(CHIP)),)
ifeq ($(BTH_AS_MAIN_MCU),1)
KBUILD_CPPFLAGS += -DBTH_AS_MAIN_MCU
endif
ifeq ($(DSP_HIFI4_TRC_TO_MCU),1)
KBUILD_CPPFLAGS += -DDSP_HIFI4_TRC_TO_MCU -DRMT_TRC_IN_MSG_CHAN
endif
endif

LIB_LDFLAGS += -lstdc++ -lsupc++

CORE_DUMP ?= 1

ifneq ($(filter 1,$(COREMARK_TEST) $(DHRYSTONE_TEST) $(SENSOR_TEST)),)
CFLAGS_IMAGE += -u _printf_float -u _scanf_float
endif

#LDFLAGS_IMAGE += --wrap main

export FREEMAN_ENABLED_STERO ?= 0
ifeq ($(FREEMAN_ENABLED_STERO),1)
export VOICE_ASSIST_STEREO ?= 1
KBUILD_CPPFLAGS += -DFREEMAN_ENABLED_STERO
endif

export MULTIMEDIA_DEBUG ?=1
ifeq ($(MULTIMEDIA_DEBUG),1)
KBUILD_CPPFLAGS += -DMULTIMEDIA_DEBUG
else
KBUILD_CPPFLAGS += -DNDEBUG
endif