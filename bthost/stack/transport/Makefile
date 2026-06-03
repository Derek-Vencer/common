
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)hci_transport.c))

obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)hci.c))

ifeq ($(IBRT), 1)
GEN_LIB_NAME := ibrt_libbt_hci
else
GEN_LIB_NAME := $(CHIP)_libbt_hci
endif

ifeq ($(HFP_1_9_ENABLE), 1)
GEN_LIB_NAME := $(GEN_LIB_NAME)_sbc_enc
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

ifeq ($(ARM_CMNS), 1)
GEN_LIB_NAME := $(GEN_LIB_NAME)_ARM_CMNS
endif

ifeq ($(mHDT_LE_SUPPORT),1)
GEN_LIB_NAME := $(GEN_LIB_NAME)_mhdt
endif

ifneq ($(BTHOST_DEBUG), 1)
GEN_LIB_NAME := $(GEN_LIB_NAME)_nodebug
endif

root_dir := $(abspath $(dir $(realpath $(firstword $(MAKEFILE_LIST))))/..)
sinclude $(root_dir)/config/lib.mk
$(GEN_LIB_NAME)-y := $(obj_c:.c=.o)

obj-y := $(GEN_LIB_NAME).a

CFLAGS_hci.o += -DBESLIB_INFO=$(BESLIB_INFO)

ccflags-y += -DBLUETOOTH_BT_IMPL

ccflags-y += \
	-Iutils/intersyshci/ \
	-Iutils/cqueue/ \
	-Iplatform/drivers/bt/ \
	-Iservices/osif/ \
	-Iservices/audio_manager/ \
	-Iapps/common/ \
	-Iutils/heap/ \
	-Ibthost/stack/common/patch \
	$(BLUETOOTH_ADAPTER_INCLUDES) \
	$(BES_BT_IMPL_INCLUDES) \
	$(BLE_STACK_INCLUDES) \
        -Ibthost/porting
