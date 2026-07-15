cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

#——————————————————————————————————————————————
# CONFIG
#——————————————————————————————————————————————
export BT_SVC_THREAD_STACK_SIZE ?= 3*1024
KBUILD_CPPFLAGS += -DBT_SVC_THREAD_STACK_SIZE=$(BT_SVC_THREAD_STACK_SIZE)

export BT_SERVICE_HEAP_SIZE ?= 3*1024
KBUILD_CPPFLAGS += -DBT_SERVICE_HEAP_SIZE=$(BT_SERVICE_HEAP_SIZE)

#——————————————————————————————————————————————
# build source code
#——————————————————————————————————————————————
GEN_LIB_NAME := btservice

ifeq ($(IBRT_UI),1)
GEN_LIB_NAME := $(GEN_LIB_NAME)_multipoint
endif

ifeq ($(AI_VOICE), 1)
GEN_LIB_NAME := $(GEN_LIB_NAME)_ai
endif

ifeq ($(BLE_AUDIO_ENABLED), 1)
GEN_LIB_NAME := $(GEN_LIB_NAME)_aob
endif

ifeq ($(SUPPORT_REMOTE_COD), 1)
GEN_LIB_NAME := $(GEN_LIB_NAME)_cod
endif

ifeq ($(ARM_CMNS), 1)
GEN_LIB_NAME := $(GEN_LIB_NAME)_ARM_CMNS
endif

ifeq ($(AOB_MOBILE_ENABLED), 1)
ifeq ($(BLE_USB_AUDIO_SUPPORT), 1)
GEN_LIB_NAME := $(GEN_LIB_NAME)_usb_dongle
endif
endif

ifeq ($(AOB_MOBILE_ENABLED), 1)
ifeq ($(BLE_USB_AUDIO_SUPPORT), 1)
GEN_LIB_NAME := $(GEN_LIB_NAME)_usb_dongle
endif
endif

ifeq ($(BT_SERVICE_TEST_ENABLE),1)
KBUILD_CPPFLAGS += -DBT_SERVICE_TEST_ENABLE
endif

ifeq ($(FREE_TWS_PAIRING_ENABLED), 1)
GEN_LIB_NAME := $(GEN_LIB_NAME)_free_tws
endif

ifneq ($(wildcard $(cur_dir)/base/Makefile),)
src_obj += base/
endif

ifneq ($(wildcard $(cur_dir)/common/Makefile),)
src_obj += common/
endif

ifneq ($(wildcard $(cur_dir)/product/Makefile),)
src_obj += product/
endif

ifeq ($(BT_SVC_MODULE_ENABLED),1)
ifneq ($(wildcard $(cur_dir)/module/Makefile),)
src_obj += module/
endif
endif

ifeq ($(BT_SVC_FW_ENABLED),1)
ifneq ($(wildcard $(cur_dir)/framework/Makefile),)
src_obj += framework/
endif
endif

root_dir := $(abspath $(dir $(realpath $(firstword $(MAKEFILE_LIST))))/..)
sinclude $(root_dir)/config/lib.mk
$(GEN_LIB_NAME)-y := $(src_obj)
obj-y := $(GEN_LIB_NAME).a

export BTSERVICE_DEBUG ?= 1
ifeq ($(BTSERVICE_DEBUG),1)
KBUILD_CPPFLAGS += -DBTSERVICE_DEBUG
endif
