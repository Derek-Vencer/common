CHIP		?= best1501

DEBUG		?= 0

NOSTD		?= 1

LIBC_ROM	?= 1

PROGRAMMER	:= 1

FAULT_DUMP	?= 0

WATCHER_DOG ?= 1

ULTRA_LOW_POWER	?= 1

OSC_26M_X4_AUD2BB ?= 0

export SYS_USE_BBPLL ?= 1

export PROGRAMMER_SET_EXT_GPIO ?= 1

init-y		:=
core-y		:= tests/programmer/ platform/cmsis/ platform/hal/
core-y          += tests/programmer_inflash/

ifeq ($(OTA_BIN_COMPRESSED),1)
core-y		+= utils/lzma/
core-y		+= utils/heap/

KBUILD_CPPFLAGS += -Iutils/lzma/
KBUILD_CPPFLAGS += -Iutils/heap/

export OTA_REBOOT_FLASH_REMAP := 0
endif

LDS_FILE	:= programmer_inflash.lds

export CRC32_ROM ?= 1

KBUILD_CPPFLAGS += -Iplatform/cmsis/inc -Iplatform/hal

KBUILD_CPPFLAGS += -DPROGRAMMER_INFLASH

KBUILD_CFLAGS +=

LIB_LDFLAGS +=

CFLAGS_IMAGE +=

LDFLAGS_IMAGE +=

OTA_ENABLE ?= 1
ifeq ($(OTA_ENABLE),1)
KBUILD_CPPFLAGS += -DOTA_ENABLE
endif

ifeq ($(OTA_ENABLE),1)
KBUILD_CPPFLAGS += -DOTA_ENABLE
endif

export OTA_REBOOT_FLASH_REMAP ?= 0

# change the flash size to 0x1000000 if 1600 uses 16MB single flash die
export SECURE_BOOT ?= 0
ifeq ($(SECURE_BOOT),1)
KBUILD_CPPFLAGS += -DSECURE_BOOT
export OTA_BOOT_SIZE ?= 0x18000
export OTA_CODE_OFFSET ?= 0x20000
export FLASH_S_SIZE ?= 0x80000
export OTA_SEC_BOOT_INFO_OFFSET ?= 0x18000
ifneq ($(FLASH_S_SIZE), 0)
KBUILD_CPPFLAGS += -DFLASH_S_SIZE=$(FLASH_S_SIZE)
endif
ifneq ($(OTA_SEC_BOOT_INFO_OFFSET), 0)
KBUILD_CPPFLAGS += -DOTA_SEC_BOOT_INFO_OFFSET=$(OTA_SEC_BOOT_INFO_OFFSET)
$(info  ******************************************************************************)
$(info  <<<<<<------        OTA_SEC_BOOT_INFO_OFFSET $(OTA_SEC_BOOT_INFO_OFFSET)        ------>>>>>>)
$(info  ******************************************************************************)
endif
endif

export FLASH_SIZE ?= 0x400000

export USE_MULTI_FLASH ?= 0
ifeq ($(USE_MULTI_FLASH),1)
export BTH_USE_SYS_FLASH ?= 1
export BTH_USE_SYS_PERIPH ?= 1
export FLASH1_SIZE ?= 0x800000
KBUILD_CPPFLAGS += \
        -DUSE_MULTI_FLASH \
        -DFLASH1_SIZE=$(FLASH1_SIZE)
LDS_CPPFLAGS += \
        -DUSE_MULTI_FLASH \
        -DFLASH1_SIZE=$(FLASH1_SIZE)
endif

KBUILD_CPPFLAGS += -DDOWNLOAD_UART_BANDRATE=921600

ifeq ($(OTA_ENABLE),1)
ifneq ($(filter best1501 best1502x best1502p best1306p,$(CHIP)),)
export OTA_BOOT_SIZE ?= 0x18000
export OTA_CODE_OFFSET ?= 0x18000
else
export OTA_BOOT_SIZE ?= 0x10000
export OTA_CODE_OFFSET ?= 0x10000
endif
else    #OTA_ENABLE
ifeq ($(OTA_BIN_COMPRESSED),1)
$(error "OTA_ENABLE conflicts with OTA_BIN_COMPRESSED")
endif
ifneq ($(filter best1306,$(CHIP)),)
export OTA_BOOT_SIZE ?= 0x8000               #31K
export OTA_CODE_OFFSET ?= 0x8000               #31K
else
export OTA_BOOT_SIZE ?= 0x10000
export OTA_CODE_OFFSET ?= 0x10000
endif
endif

export APP_ENTRY_ADDRESS ?= 0x2c000000+$(OTA_CODE_OFFSET)

KBUILD_CPPFLAGS += -DOTA_CODE_OFFSET=$(OTA_CODE_OFFSET)
KBUILD_CPPFLAGS += -DAPP_ENTRY_ADDRESS=$(APP_ENTRY_ADDRESS)

export SINGLE_WIRE_DOWNLOAD ?= 1
export UNCONDITIONAL_ENTER_SINGLE_WIRE_DOWNLOAD ?= 1

export PROGRAMMER_WATCHDOG ?= 1
export FLASH_UNIQUE_ID ?= 1
export TRACE_BAUD_RATE := 10*115200

ifeq ($(OTA_REBOOT_FLASH_REMAP),1)
KBUILD_CPPFLAGS += -DOTA_REBOOT_FLASH_REMAP
endif # REMAP

ifeq ($(USER_SECURE_BOOT),1)
## same with next jump entry addr offset##
export USER_SECURE_BOOT_JUMP_ENTRY_ADDR ?= $(OTA_CODE_OFFSET)
endif

ifneq ($(filter best1501 best1501p best1306 best1502x,$(CHIP)),)
ifeq ($(CHIP),best1501)
export RAM_SIZE := 0x160000
else ifeq ($(CHIP),best1501p)
export RAM_SIZE := 0x160000
else ifeq ($(CHIP),best1306)
export RAM_SIZE := 0x80000
else ifeq ($(CHIP),best1502x)
export RAM_SIZE := 0x180000
endif
endif

export DOLBY_AUDIO_ENABLE ?= 0
ifeq ($(DOLBY_AUDIO_ENABLE),1)
KBUILD_CPPFLAGS += -DDOLBY_AUDIO_ENABLE
endif
