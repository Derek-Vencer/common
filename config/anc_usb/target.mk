CHIP		?= best1700

DEBUG		?= 1

FPGA		?= 0

FULL_APP	?= 0

ifeq ($(FULL_APP),1)
RTOS		?= 1
APP_TEST_MODE	?= 1
APP_TEST_AUDIO	?= 1
endif

ifeq ($(RTOS),1)
MBED		?= 1
else
export INTSRAM_RUN	?= 0
endif

ifeq ($(INTSRAM_RUN),1)
ULTRA_LOW_POWER	?= 0
else
ULTRA_LOW_POWER	?= 0
endif

ifeq ($(filter best1000 best2000,$(CHIP)),)
export AUDIO_RESAMPLE	?= 1
endif

ifeq ($(USB_ANC_MC_EQ_TUNING),1)
ifeq ($(AUDIO_ANC_FB_MC_HW),1)
export AUDIO_ANC_FB_MC	:= 0
export HW_IIR_EQ_PROCESS:= 0
else
export AUDIO_ANC_FB_MC	:= 1
export HW_IIR_EQ_PROCESS:= 1
endif
export AUDIO_RESAMPLE	:= 0
export PROGRAMMER	:= 1
export VENDOR_MSG_SUPPT	:= 1
export ANC_INIT_OFF	:= 1
export AUDIO_OUTPUT_DC_CALIB_ANA	:= 0
export USB_AUDIO_DYN_CFG	:= 0
export AUDIO_OUTPUT_VOLUME_DEFAULT	:= 17
endif # USB_ANC_MC_EQ_TUNING

ifneq ($(filter best1600 best1603 best1700,$(CHIP)),)
ifeq ($(CHIP_SUBSYS),bth)
export BTH_AS_MAIN_MCU 				:= 1
export BTH_USE_SYS_PERIPH			:= 1
ifeq ($(PROGRAMMER),1)
export BTH_USE_SYS_FLASH			:= 1
export PROGRAMMER_LOAD_RAM			:= 1
export PIE							:= 0
export PROGRAMMER_BOOT_ENTRY_HOOK	:= 1
endif
endif
endif

NOSTD		?= 0

DEBUG_PORT	?= 1

FLASH_CHIP	?= ALL

export AUDIO_OUTPUT_DIFF ?= 1

export AUDIO_ANC_FB_MC ?= 0

export ANC_FB_CHECK ?= 0

export HW_FIR_DSD_PROCESS			?= 0
ifeq ($(HW_FIR_DSD_PROCESS),1)
ifeq ($(CHIP),best2300)
export HW_FIR_DSD_BUF_MID_ADDR		?= 0x200A0000
export RAM_SIZE						?= 0x00080000
export DATA_BUF_START				?= 0x20040000
endif
endif

export HW_FIR_EQ_PROCESS			?= 0

export SW_IIR_EQ_PROCESS			?= 0

export HW_DAC_IIR_EQ_PROCESS		?= 0

export HW_IIR_EQ_PROCESS			?= 0

export USB_AUDIO_APP ?= 1

export USB_HID_COMMAND_ENABLE ?= 0
ifeq ($(USB_HID_COMMAND_ENABLE),1)
KBUILD_CPPFLAGS += -DUSB_HID_COMMAND_ENABLE
endif

ifeq ($(CHIP),best1400)
export ANA_26M_X6_ENABLE ?= 1
endif

ifeq ($(CHIP_SUBTYPE),best1402)
export ANC_APP ?= 1
export ANC_FB_ENABLED   ?= 0
endif
ifneq ($(filter best3001 best1400 best2001 best2003,$(CHIP)),)
export ANC_APP ?= 0
else
export ANC_APP ?= 1
endif

ifeq ($(CHIP),best1400)
export USB_AUDIO_SEND_CHAN ?= 1
endif

ifeq ($(ANC_APP),1)
ifeq ($(CHIP),best1000)
AUD_PLL_DOUBLE	?= 0
DUAL_AUX_MIC	?= 1
else
export ANC_FF_ENABLED	?= 1
export ANC_FB_ENABLED	?= 1
export AUDIO_SECTION_SUPPT ?= 1
endif
export ANC_INIT_SPEEDUP	?= 1
endif

export NO_LIGHT_SLEEP	?= 0

export USB_ISO ?= 1

export USB_HIGH_SPEED ?= 0

export USB_AUDIO_UAC2 ?= 0

export USB_AUDIO_DYN_CFG ?= 1

export DELAY_STREAM_OPEN ?= 0

export SPEECH_TX_DC_FILTER ?= 0

export SPEECH_TX_AEC2FLOAT ?= 0

export SPEECH_TX_NS2FLOAT ?= 0

export SPEECH_TX_2MIC_NS2 ?= 0

export SPEECH_TX_COMPEXP ?= 0

export SPEECH_TX_EQ ?= 0

export SPEECH_TX_POST_GAIN ?= 0

export POWER_MODE	?= DIG_DCDC

export BLE	:= 0

export MIC_KEY   ?= 0

LETV		?= 0
ifeq ($(LETV),1)
export USB_AUDIO_VENDOR_ID	?= 0x262A
export USB_AUDIO_PRODUCT_ID	?= 0x1534
endif

export VENDOR_MSG_SUPPT	?= 0
export USB_EQ_TUNING    ?= 0
export FACTORY_SECTION_SUPPT    ?= 0

export SPEECH_TX_2MIC_NS3 ?= 0

export AUDIO_OUTPUT_DC_AUTO_CALIB ?= 0
ifeq ($(AUDIO_OUTPUT_DC_AUTO_CALIB), 1)
export AUDIO_OUTPUT_DC_CALIB := 1
export AUDIO_OUTPUT_DC_CALIB_ANA := 0
ifeq ($(filter best1600 best1502x,$(CHIP)),)
export AUDIO_OUTPUT_SET_LARGE_ANA_DC ?= 1
else
export AUDIO_OUTPUT_DC_CALIB_DUAL_CHAN ?= 1
export AUDIO_OUTPUT_SET_LARGE_ANA_DC ?= 0
export AUDIO_OUTPUT_DIG_DC_DEEP_CALIB ?= 1
ifneq ($(filter best1600,$(CHIP)),)
export AUDIO_OUTPUT_USE_FIXED_DC ?= 1
endif
endif
export DAC_DRE_ENABLE ?= 1
export CODEC_DAC_DC_NV_DATA ?= 0
export CODEC_DAC_DC_DYN_BUF ?= 0
endif

export AUDIO_ADC_DC_AUTO_CALIB ?= 0
ifeq ($(AUDIO_ADC_DC_AUTO_CALIB), 1)
export CODEC_ADC_DC_NV_DATA ?= 0

ifeq ($(CODEC_ADC_DC_NV_DATA), 1)
KBUILD_CPPFLAGS += -DCODEC_ADC_DC_NV_DATA
NEW_NV_RECORD_ENABLED ?= 1
ifeq ($(NEW_NV_RECORD_ENABLED),1)
KBUILD_CPPFLAGS += -DNEW_NV_RECORD_ENABLED
KBUILD_CPPFLAGS += -Iservices/nv_section/userdata_section
endif
endif
ifeq ($(AUDIO_ADC_DC_AUTO_CALIB), 1)
KBUILD_CPPFLAGS += -DAUDIO_ADC_DC_AUTO_CALIB
endif
endif

export BT_ADDR_PRINT_CONFIG ?= 0
ifeq ($(BT_ADDR_PRINT_CONFIG),1)
KBUILD_CFLAGS += -DBT_ADDR_OUTPUT_PRINT_NUM=2
else
KBUILD_CFLAGS += -DBT_ADDR_OUTPUT_PRINT_NUM=6
endif

init-y		:=
ifeq ($(FULL_APP),1)
core-y		:= platform/ services/ apps/ utils/cqueue/ utils/list/ multimedia/ utils/intersyshci/
KBUILD_CPPFLAGS += -Iplatform/cmsis/inc -Iservices/audioflinger -Iplatform/hal -Iservices/fs/ -Iservices/fs/sd -Iservices/fs/fat -Iservices/fs/fat/ChaN
else
core-y		:= tests/anc_usb/ platform/cmsis/ platform/hal/ platform/drivers/usb/usb_dev/ platform/drivers/norflash/ platform/drivers/ana/ platform/drivers/codec/ services/audioflinger/ utils/hwtimer_list/
KBUILD_CPPFLAGS += -Iplatform/cmsis/inc -Iplatform/hal -Iplatform/drivers/ana -Iservices/audioflinger
ifeq ($(RTOS),1)
# core-y		+= services/fs/
# KBUILD_CPPFLAGS += -Iservices/fs/fat/ChaN
endif
endif

ifeq ($(BLE),1)
core-y		+= utils/jansson/
KBUILD_CPPFLAGS += -D__IAG_BLE_INCLUDE__=1
#KBUILD_CPPFLAGS += -DGATT_CLIENT=1
endif

KBUILD_CPPFLAGS +=
#-D_AUTO_SWITCH_POWER_MODE__
#-D_BEST1000_QUAL_DCDC_
#-DSPEECH_TX_AEC
#-DSPEECH_TX_NS
#-DMEDIA_PLAYER_SUPPORT

ifeq ($(PROGRAMMER),1)
export PMU_INIT		?= 1
LDS_FILE	:= programmer.lds
else ifeq ($(INTSRAM_RUN),1)
LDS_FILE	:= best1000_intsram.lds
else
LDS_FILE	:= best1000.lds
endif

KBUILD_CPPFLAGS +=

ifeq ($(MIC_KEY), 1)
KBUILD_CFLAGS += -DCFG_MIC_KEY
endif

ifeq ($(AUDIO_OUTPUT_DC_AUTO_CALIB),1)
KBUILD_CPPFLAGS += -DAUDIO_OUTPUT_DC_AUTO_CALIB
endif

ifeq ($(CODEC_DAC_DC_NV_DATA), 1)
KBUILD_CPPFLAGS += -DCODEC_DAC_DC_NV_DATA
endif

ifeq ($(OTAGEN_SLAVE_BIN),1)
export OTA_CODE_OFFSET ?= 0x18000+0x100000
endif

KBUILD_CPPFLAGS += -DFLASH_API_NORMAL

LIB_LDFLAGS += -lstdc++ -lsupc++

#CFLAGS_IMAGE += -u _printf_float -u _scanf_float

#LDFLAGS_IMAGE += --wrap main
