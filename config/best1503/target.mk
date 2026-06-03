CHIP        ?= best1503

DEBUG       ?= 1

FPGA        ?= 0

RTOS        ?= 1

LIBC_ROM    ?= 1

export LIBC_OVERRIDE ?= 1

KERNEL      ?= RTX5
VERSION_INFO ?= best1503_ibrt

export USER_SECURE_BOOT	?= 0
# enable:1
# disable:0

WATCHER_DOG ?= 1

DEBUG_PORT  ?= 1
# 0: usb
# 1: uart0
# 2: uart1

FLASH_CHIP	?= ALL
# GD25Q80C
# GD25Q32C
# ALL

export INTERSYS_DEBUG ?= 1

export FORCE_SIGNALINGMODE ?= 0

export FORCE_NOSIGNALINGMODE ?= 0

export BT_TEST_CURRENT_KEY ?= 0

export PROFILE_DEBUG ?= 0

export BTDUMP_ENABLE ?= 0

export TPORTS_KEY_COEXIST ?= 0

ifeq ($(BTH_IN_ROM),1)
ifeq ($(CHIP),best1503)
LIB_LDFLAGS += -Wl,--just-symbols=$(srctree)/bthost/rom/1503/lib/bth_rom.elf
else
$(error This chip not support bth-rom)
endif
endif

export DISPLAY_PREFIX_HCI_CMD_EVT_ ?= 0

AUDIO_OUTPUT_MONO ?= 0

AUDIO_OUTPUT_DIFF ?= 0

# When change music volume level, will select differnet EQ parameters
# Phone needs to support Bluetooth Absolute Volume Control
export AUDIO_VOL_CTRL_EQ ?= 0

HW_FIR_EQ_PROCESS ?= 0

SW_IIR_EQ_PROCESS ?= 0

SW_IIR_PROMPT_EQ_PROCESS ?= 0

HW_DAC_IIR_EQ_PROCESS ?= 1

HW_IIR_EQ_PROCESS ?= 0

HW_DC_FILTER_WITH_IIR ?= 0

AUDIO_DYNAMIC_BOOST ?= 0

AUDIO_DRC ?= 0

AUDIO_LIMITER ?= 0

AUDIO_HEARING_COMPSATN ?= 0

PC_CMD_UART ?= 0

TOTA_EQ_TUNING ?= 0

AUDIO_SECTION_ENABLE ?= 0

AUDIO_RESAMPLE ?= 1

RESAMPLE_ANY_SAMPLE_RATE ?= 1

OSC_26M_X4_AUD2BB ?= 1

export SYS_USE_BBPLL ?= 1

AUDIO_OUTPUT_VOLUME_DEFAULT ?= 16

# range:1~16

CODEC_DAC_MULTI_VOLUME_TABLE ?= 0

AUDIO_INPUT_CAPLESSMODE ?= 0

AUDIO_INPUT_LARGEGAIN ?= 0

AUDIO_CODEC_ASYNC_CLOSE ?= 0

AUDIO_SCO_BTPCM_CHANNEL ?= 1

export A2DP_KARAOKE ?= 0

export KARAOKE_ALGO_EQ ?= 0

export KARAOKE_ALGO_NS ?= 0

export A2DP_CP_ACCEL ?= 1

export SCO_CP_ACCEL ?= 1

export SCO_TRACE_CP_ACCEL ?= 0

LARGE_RAM ?= 1

HSP_ENABLE ?= 0

SBC_FUNC_IN_ROM ?= 0

ifneq ($(filter 1 ,$(ARM_CMNS)),)
ROM_UTILS_ON ?= 0
else
export ROM_UTILS_ON ?= 1
export AAC_IN_ROM ?=1
export CVSD_IN_ROM ?=1
export LC3_IN_ROM_V2 ?=1
endif

APP_LINEIN_A2DP_SOURCE ?= 0

APP_I2S_A2DP_SOURCE ?= 0

VOICE_PROMPT ?= 1

TWS_PROMPT_SYNC ?= 1

REPORT_CONNECTIVITY_LOG ?= 0

ifneq ($(AUDIO_RESAMPLE),1)
export AUDIO_USE_BBPLL := 1
endif

# TOTA1: old tota, TOTA2: new tota(debuging)
export TOTA ?= 0
export TOTA_v2 ?= 0

ifeq ($(TOTA_v2),1)
export TOTA := 0
endif

BT_DIP_SUPPORT ?= 1

AUDIO_RMS_MONITOR_ENABLE ?= 0

BES_OTA ?= 0

TILE_DATAPATH_ENABLED ?= 0

CUSTOM_INFORMATION_TILE_ENABLE ?= 0

INTERCONNECTION ?= 0

INTERACTION ?= 0

INTERACTION_FASTPAIR ?= 0

BT_ONE_BRING_TWO ?= 0

DSD_SUPPORT ?= 0

A2DP_EQ_24BIT ?= 1

A2DP_AAC_ON ?= 1

A2DP_SCALABLE_ON ?= 0

A2DP_LHDC_V3 ?= 0

A2DP_LHDC_ON ?= 0

A2DP_LHDCV5_ON ?= 0

A2DP_LDAC_ON ?= 0

export A2DP_LC3_ON ?= 0
export A2DP_LC3_HR ?= 0

export TX_RX_PCM_MASK ?= 0

FACTORY_MODE ?= 1

ENGINEER_MODE ?= 1

ULTRA_LOW_POWER	?= 1

DAC_CLASSG_ENABLE ?= 0

NO_SLEEP ?= 0

CORE_DUMP ?= 1

CORE_DUMP_TO_FLASH ?= 0

export SYNC_BT_CTLR_PROFILE ?= 0

export A2DP_AVDTP_CP ?= 0

export A2DP_DECODER_VER := 2

export AUDIO_TRIGGER_VER := 1

ifneq ($(BT_SERVICE_ENABLE),1)
ifneq ($(BT_BUILD_WITH_CUSTOMER_HOST),1)
export IBRT ?= 1

export IBRT_SEARCH_UI ?= 0

export IBRT_UI ?= 1
ifeq ($(IBRT_UI),1)
KBUILD_CPPFLAGS += -DIBRT_UI
endif

export BES_AUD ?= 1
endif
endif

export POWER_MODE   ?= DIG_DCDC

export MIX_AUDIO_PROMPT_WITH_A2DP_MEDIA_ENABLED ?= 0

export PROMPT_SELF_MANAGEMENT ?= 1

export IOS_IAP2_BLUETOOTH ?= 0

export FLASH_SIZE ?= 0x400000
export FLASH_SUSPEND ?= 1
export FLASH_PROTECTION_BOOT_SECTION_FIRST ?= 0
export NORFLASH_API_FLUSH_IN_SLEEP_HOOK ?= 1
export HOST_GEN_ECDH_KEY ?= 1

USE_THIRDPARTY ?= 0
export USE_KNOWLES ?= 0

export POWERKEY_I2C_SWITCH ?=0

AUTO_TEST ?= 0

BES_AUTOMATE_TEST ?= 0

export DUMP_LOG_ENABLE ?= 0

export DUMP_CRASH_LOG ?= 0

export CPU_PC_DUMP ?= LR

SUPPORT_BATTERY_REPORT ?= 1

SUPPORT_HF_INDICATORS ?= 0

SUPPORT_SIRI ?= 1

BES_AUDIO_DEV_Main_Board_9v0 ?= 0

APP_USE_LED_INDICATE_IBRT_STATUS ?= 0

export BT_HOST_REJECT_UNEXCEPT_SCO_PACKET := 1

export BT_PAUSE_A2DP := 1

export BT_PAUSE_A2DP_WHEN_CALL_EXIST := 1

export NORMAL_TEST_MODE_SWITCH ?= 0

#For ble feture verification
export BLE := 0
export GATT_OVER_BR_EDR ?= 0

export GFPS_ENABLE ?= 0

HAS_BT_SYNC ?= 1

#For free tws pairing feature
FREE_TWS_PAIRING_ENABLED ?= 0

APP_UART_MODULE ?= 0

export PROMPT_IN_FLASH ?= 0

export CALIB_SLOW_TIMER ?= 1
export BT_DONT_PLAY_MUTE_WHEN_A2DP_STUCK_PATCH ?= 1

export TRACE_BUF_SIZE ?= 32*1024
export TRACE_BAUD_RATE ?= 10*115200
export BTM_MAX_LINK_NUMS ?= 3
export BT_DEVICE_NUM ?= 2

ifeq ($(PSAP_SW_APP),1)
export DMA_AUDIO_APP ?= 1
endif

export DMA_AUDIO_APP ?= 0
ifeq ($(DMA_AUDIO_APP),1)
export SCO_CP_ACCEL := 0
export A2DP_CP_ACCEL := 0
export DMA_RPC_CLI ?= 1
DMA_AUD_CFG_PATH := best1503_ibrt
include $(srctree)/config/$(DMA_AUD_CFG_PATH)/dma_aud_cfg.mk
endif

init-y :=
core-y := platform/ utils/cqueue/ utils/list/ multimedia/ utils/intersyshci/ utils/sha256/ utils/lzma/

ifneq ($(EXT_SRC_DIR),)
core-y += $(EXT_SRC_DIR)
else
core-y += apps/ services/
endif

KBUILD_CPPFLAGS += \
    -Iplatform/cmsis/inc \
    -Iservices/audioflinger \
    -Iplatform/hal

KBUILD_CPPFLAGS += \
    -DCHARGER_PLUGINOUT_RESET=0

KBUILD_CPPFLAGS += \
    -DBTM_MAX_LINK_NUMS=$(BTM_MAX_LINK_NUMS) \
    -DBT_DEVICE_NUM=$(BT_DEVICE_NUM)

ifeq ($(BES_AUDIO_DEV_Main_Board_9v0),1)
KBUILD_CPPFLAGS += -DBES_AUDIO_DEV_Main_Board_9v0
endif

ifeq ($(TPORTS_KEY_COEXIST),1)
KBUILD_CPPFLAGS += -DTPORTS_KEY_COEXIST
endif


#-DIBRT_LINK_LOWLAYER_MONITOR

#-D_AUTO_SWITCH_POWER_MODE__
#-D__APP_KEY_FN_STYLE_A__
#-D__APP_KEY_FN_STYLE_B__
#-D__EARPHONE_STAY_BOTH_SCAN__
#-D__POWERKEY_CTRL_ONOFF_ONLY__
#-DAUDIO_LINEIN

ifeq ($(CURRENT_TEST),1)
export SMALL_RET_RAM ?= 1
export CORE_SLEEP_POWER_DOWN ?= 1
#INTSRAM_RUN ?= 1
endif
ifeq ($(INTSRAM_RUN),1)
LDS_FILE ?= best1000_intsram.lds
else
LDS_FILE ?= best1000.lds
endif

export OTA_SUPPORT_SLAVE_BIN ?= 0

export AUDIO_OUTPUT_DC_AUTO_CALIB ?= 1

export AUDIO_ADC_DC_AUTO_CALIB ?= 1

KBUILD_CFLAGS +=

LIB_LDFLAGS += -lstdc++ -lsupc++

DUAL_MIC_RECORDING ?= 0
RECORDING_USE_SCALABLE ?= 0
RECORDING_USE_OPUS ?= 0
RECORDING_USE_OPUS_LOWER_BANDWIDTH ?= 0
BINAURAL_RECORD_PROCESS ?= 0

# mutex for power on tws pairing and freeman pairing
POWER_ON_ENTER_TWS_PAIRING_ENABLED ?= 0
POWER_ON_ENTER_FREEMAN_PAIRING_ENABLED ?= 0
POWER_ON_ENTER_BOTH_SCAN_MODE ?= 0

OS_THREAD_TIMING_STATISTICS_ENABLE ?= 0

#CFLAGS_IMAGE += -u _printf_float -u _scanf_float

#LDFLAGS_IMAGE += --wrap main


ifeq ($(AI_ENABLE),1)
include $(srctree)/config/$(CHIP)/ai_config.mk
endif

ifeq ($(ANC_ENABLE),1)
include $(srctree)/config/$(CHIP)/anc_config.mk
endif

ifeq ($(ANC_ASSIST_ENABLE),1)
include $(srctree)/config/$(CHIP)/anc_assist_cfg.mk
endif

#ifeq ($(CAPSENSOR_ENABLE),1)
include config/$(CHIP)/capsensor_cfg.mk
#endif

ifeq ($(LEA_ENABLE),1)
include $(srctree)/config/$(CHIP)/lea_config.mk
endif


ifeq ($(DOLBY_AUDIO_ENABLE),1)
include $(srctree)/config/$(CHIP)/dolby_config.mk
endif

ifeq ($(LARGE_SE_RAM),1)
KBUILD_CPPFLAGS += -DLARGE_SE_RAM
endif

ifeq ($(USER_SECURE_BOOT),1)
export OTA_SEC_BOOT_INFO_OFFSET ?= 0x18000
KBUILD_CPPFLAGS += -DOTA_SEC_BOOT_INFO_OFFSET=$(OTA_SEC_BOOT_INFO_OFFSET)
endif

include $(srctree)/config/$(CHIP)/speech_config.mk

include $(srctree)/config/$(CHIP)/mem_config.mk

include $(srctree)/config/$(CHIP)/bt_config.mk

ifeq ($(DOLBY_AUDIO_ENABLE),1)
include $(srctree)/config/$(CHIP)/dolby_config.mk
endif
ifeq ($(or $(BESUI_TWS_EN), $(BESUI_STEREO_EN)), 1)
include $(srctree)/config/besui.mk
endif

