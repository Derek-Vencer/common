
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)intersyshci.cpp))

src_obj := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

GEN_LIB_NAME := libintersyshci_enhanced_stack

ifeq ($(INTERSYS_NO_THREAD),1)
GEN_LIB_NAME := $(GEN_LIB_NAME)_nothread
endif

ifeq ($(BYPASS_STACK_AUDIO_STREAM),1)
GEN_LIB_NAME := $(GEN_LIB_NAME)_bypasssas
endif

ifeq ($(BLE_AUDIO_ENABLED), 1)
GEN_LIB_NAME := $(GEN_LIB_NAME)_aob
endif

CFLAGS_intersyshci.o += -DBESLIB_INFO=$(BESLIB_INFO)
root_dir := $(abspath $(dir $(realpath $(firstword $(MAKEFILE_LIST))))/..)
include $(root_dir)/config/lib.mk

$(GEN_LIB_NAME)-y := $(src_obj)
obj-y += $(GEN_LIB_NAME).a
ccflags-y := \
    $(BLUETOOTH_ADAPTER_INCLUDES) \
    -Iapps/common/ \
    -Iutils/cqueue \
    -Iplatform/drivers/bt \
    -Itests/ota_boot/inc

ifeq ($(INTERSYS_NO_THREAD),1)
CFLAGS_intersyshci.o += -D_INTERSYS_NO_THREAD_
endif

ifdef DISPLAY_PREFIX_HCI_CMD_EVT_
ifneq ($(DISPLAY_PREFIX_HCI_CMD_EVT_),0)
subdir-ccflags-y += -DDISPLAY_PREFIX_HCI_CMD_EVT
endif
endif
