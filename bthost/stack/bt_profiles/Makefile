
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)bt/*/*.S))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)bt/*/*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)bt/*/*.cpp))

obj_s += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)gap/*.S))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)gap/*.c))
obj_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)gap/*.cpp))

obj_s += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)gatt/*.S))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)gatt/*.c))
obj_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)gatt/*.cpp))

obj_s += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)l2cap/*.S))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)l2cap/*.c))
obj_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)l2cap/*.cpp))

obj_s += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)sdp/*.S))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)sdp/*.c))
obj_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)sdp/*.cpp))

ifeq ($(IBRT), 1)
GEN_LIB_NAME := ibrt_libbt_profiles
else
GEN_LIB_NAME := $(CHIP)_libbt_profiles
endif

ifeq ($(HFP_1_9_ENABLE), 1)
GEN_LIB_NAME := $(GEN_LIB_NAME)_sbc_enc
endif

ifeq ($(BTH_IN_ROM),1)
GEN_LIB_NAME := $(GEN_LIB_NAME)_romaac
endif

ifeq ($(BT_HFP_SUPPORT), 1)
GEN_LIB_NAME := $(GEN_LIB_NAME)_hfp
endif

ifeq ($(BT_RF_PREFER), 2M)
GEN_LIB_NAME := $(GEN_LIB_NAME)_2m
endif

ifeq ($(BT_RF_PREFER), 3M)
GEN_LIB_NAME := $(GEN_LIB_NAME)_3m
endif

ifeq ($(SUPPORT_REMOTE_COD), 1)
GEN_LIB_NAME := $(GEN_LIB_NAME)_cod
endif

ifeq ($(BLE_AUDIO_ENABLED), 1)
GEN_LIB_NAME := $(GEN_LIB_NAME)_aob
endif

ifeq ($(BT_DIP_SUPPORT), 1)
GEN_LIB_NAME := $(GEN_LIB_NAME)_dip
endif

ifeq ($(ARM_CMNS), 1)
GEN_LIB_NAME := $(GEN_LIB_NAME)_ARM_CMNS
endif

ifeq ($(HFP_SUPPORT_LC3_SWB), 1)
GEN_LIB_NAME := $(GEN_LIB_NAME)_swb
endif

ifeq ($(mHDT_LE_SUPPORT),1)
GEN_LIB_NAME := $(GEN_LIB_NAME)_mhdt
endif

ifneq ($(BTHOST_DEBUG), 1)
GEN_LIB_NAME := $(GEN_LIB_NAME)_nodebug
endif

root_dir := $(abspath $(dir $(realpath $(firstword $(MAKEFILE_LIST))))/..)
sinclude $(root_dir)/config/lib.mk
$(GEN_LIB_NAME)-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

obj-y += $(GEN_LIB_NAME).a

ifeq ($(OPT_LEVEL),)
CFLAGS_a2dp.o += -O3
endif

CFLAGS_btm.o += -DBESLIB_INFO=$(BESLIB_INFO)

ccflags-y += -DBLUETOOTH_BT_IMPL

subdir-ccflags-y += \
    -Iservices/osif/ \
    -Iservices/auto_test \
    -Ibthost/porting/ \
    -Ibthost/stack/ble_if/inc \
    $(BLUETOOTH_ADAPTER_INCLUDES) \
    $(BES_BT_IMPL_INCLUDES) \
    $(BLE_STACK_INCLUDES) \
    -Iplatform/drivers/uarthci \
    -Iplatform/drivers/ana \
    -Iplatform/drivers/bt \
    -Iutils/cqueue \
    -Iutils/heap \
    -Iutils/intersyshci \
    -Iservices/audioflinger \
    -Iservices/resources \
    $(BT_SERVICE_UX_INCLUDES) \
    -Iutils/lockcqueue \
    -Iapps/key \
    $(SMF_DEFINES) \
    -Iapps/audioplayers \
    -I$(BT_ROM_DBG_PATH) \
    -Ibthost/service/ble_app_new/inc \
    -Ibthost/porting \
    -Ibthost/stack/common/patch

ifeq ($(BLE_AUDIO_ENABLED),1)
subdir-ccflags-y += \
    -Iservices/audio_manager
endif
