cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))
# -DLHDC_SE_VERSION
# -DHAVE_TEST_KEY
CORP_NAME	   := SAVI
LOG_BUF_SIZE   := 512
SV_LIB_NAME    := libLHDC_V5

VERSION_PREFIX		   := BES-LHDC-
NORMAL_VERSION_NUM	   := 5.0.9

auth_type	   :=
demo_flg	   ?= -DSAVI_DEMO_MODE
codec_type	   := -DLOG_BUF_LEN=$(LOG_BUF_SIZE) -D__LHDC_DEBUG__
VERSION_NUM    := $(NORMAL_VERSION_NUM)
build_time     := $(shell date +%y%m%d_%H%M%S)

ifneq ($(GEN_LIB), 1)
auth_type += -DHAVE_TEST_KEY
endif

ifneq ($(DEMO_MODE), 1)
demo_flg	    := 
endif

auth_type += -DLICE_KEY_TYPE_KEYPRO -DLHDC_VERIFY_UUID -DVERSION=\"$(VERSION_PREFIX)$(VERSION_NUM)_$(build_time)\"

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_s += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.s))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)dec_src/*.c))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)dec_src/svlice_v5/*.c))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)dec_src/lhdc_v5_dec/*.c))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

ifneq (,$(findstring dsp_, $(T)))
PLATFORM_NAME := m55
else
PLATFORM_NAME := m33
endif

$(warning $(PLATFORM_NAME))
ifeq ($(PLATFORM_NAME), m33)
ifneq (,$(findstring 1502, $(CHIP)))
LHDC_LIB_NAME := BEST2800_libLHDC_V5_5_4_6_SAVI_DEMO
else
LHDC_LIB_NAME := BEST2700P_libLHDC_V5_5_0_11_SAVI_KEYPRO_UUID
endif
else ifeq ($(PLATFORM_NAME), m55)
LHDC_LIB_NAME := DSP_M55_libLHDC_V5_5_0_11_SAVI_KEYPRO_UUID

endif


## Add Platform flag
DEF += -D$(PLATFORM_NAME)
## Modify Version number to a string for library name use.
VERSION_STRING := $(subst .,_,$(VERSION_NUM))

$(LHDC_LIB_NAME)-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

LIB_BIN_DIR := lib/thirdparty/audio_codec/liblhdcv5-dec
obj-y += $(LHDC_LIB_NAME).a

ccflags-y += -lc -O3 -Wchar-subscripts $(DEF)

ccflags-y +=  -DLHDC_V5_DEC_MODEL

subdir-ccflags-y += \
    -Iservices/tws/inc \
    -Iutils/heap \
    -Iutils/cqueue \
    -Iapps/audioplayers \
    -Iservices/bt_app \
    -Iplatform \
    -Iplatform/hal \
    -Ithirdparty/audio_codec_lib/liblhdcv5-dec/dec_src \
    -Ithirdparty/audio_codec_lib/liblhdcv5-dec \
    -Iservices/ble_stack/ke/api \
    -Iservices/ble_stack/ble_ip \
    $(BT_PROFILES_INCLUDES) \
    -Iservices/nvrecord \
    -Iservices/bt_if/inc \
    -Iservices/bt_if_enhanced/inc \
    -Iconfig \
    -Iinclude/cmsis_dsp \
    -Iservices/ble_stack/hl/api

subdir-ccflags-y += -Ithirdparty/audio_codec_lib/liblhdcv5-dec/dec_src/lhdc_v5_dec
subdir-ccflags-y += -Ithirdparty/audio_codec_lib/liblhdcv5-dec/inc
