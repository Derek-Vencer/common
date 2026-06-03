
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))

obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)patch/*.c))

obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)l2cap/*.c))

ifeq ($(IBRT), 1)
GEN_LIB_NAME := ibrt_libbt_common
else
GEN_LIB_NAME := $(CHIP)_libbt_common
endif

ifeq ($(BTH_IN_ROM),1)
GEN_LIB_NAME := $(GEN_LIB_NAME)_romaac
endif

ifeq ($(BT_RF_PREFER), 2M)
GEN_LIB_NAME := $(GEN_LIB_NAME)_2m
endif

ifeq ($(BT_RF_PREFER), 3M)
GEN_LIB_NAME := $(GEN_LIB_NAME)_3m
endif

ifeq ($(BLE_AUDIO_ENABLED), 1)
GEN_LIB_NAME := $(GEN_LIB_NAME)_aob
endif

ifneq ($(BTHOST_DEBUG), 1)
GEN_LIB_NAME := $(GEN_LIB_NAME)_nodebug
endif

root_dir := $(abspath $(dir $(realpath $(firstword $(MAKEFILE_LIST))))/..)
sinclude $(root_dir)/config/lib.mk
$(GEN_LIB_NAME)-y := $(obj_c:.c=.o)

obj-y := $(GEN_LIB_NAME).a

ccflags-y += -DBLUETOOTH_BT_IMPL

ccflags-y += \
	-Iutils/intersyshci/ \
	-Iutils/cqueue/ \
	-Iplatform/drivers/bt/ \
	-Iservices/osif/ \
	-Iapps/common/ \
	-Iutils/heap/ \
	-Iservices/audio_manager/ \
	$(BLUETOOTH_ADAPTER_INCLUDES) \
	$(NEAR_STACK_INCLUDES) \
	$(BES_BT_IMPL_INCLUDES) \
	$(BLE_STACK_INCLUDES) \
    -I$(BT_ROM_PATCH_PATH)/ \
	$(BLE_STACK_INC_INTERNAL) \
    -Ibthost/porting
