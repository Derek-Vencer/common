cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

src_obj := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

GEN_LIB_NAME := libanc_spp_tool

ifeq ($(AUDIO_SECTION_ENABLE), 1)
GEN_LIB_NAME := $(GEN_LIB_NAME)_eq
endif

ifeq ($(ARM_CMNS), 1)
GEN_LIB_NAME := $(GEN_LIB_NAME)_ARM_CMNS
endif

ifeq ($(ANC_PROD_TEST), 1)
GEN_LIB_NAME := $(GEN_LIB_NAME)_anc_pord
endif

root_dir := $(abspath $(dir $(realpath $(firstword $(MAKEFILE_LIST))))/..)
sinclude $(root_dir)/config/lib.mk
$(GEN_LIB_NAME)-y := $(src_obj)

obj-y += $(GEN_LIB_NAME).a

ccflags-y += -Iutils/hexdump    \
        -Iapps/key  \
		-Iapps/psap/inc \
        -Iservices/bt_app   \
        -Iservices/bt_if/inc  \
        -Iplatform/drivers/ana \
		-Imultimedia/inc/audio/process/anc/include \
		-Imultimedia/inc/audio/process/psap/include \
		-Imultimedia/inc/speech/inc/ \
		-Iservices/audioflinger \
		-Iservices/nv_section/aud_section \
		-Iservices/nv_section/include \
		-Iservices/audio_eq \
		-Iplatform/hal \
		-Iplatform/drivers/norflash \
		-Imultimedia/inc/audio/process/filters/include \
		-Iutils/crc \
		-Iutils/boot_struct \
		-Iutils/hwtimer_list \
		-Iutils/heap \
        -Iservices/tws/inc \
        -Iutils/cqueue \
        -Iapps/audioplayers \
		-Iapps/voice_assist/inc \
		-Iapps/anc/inc \
		-Iapps/anc_assist/inc


ifeq ($(ANC_FF_ENABLED),1)
CFLAGS_anc_usb_app.o += -DANC_FF_ENABLED
endif

ifeq ($(ANC_FB_ENABLED),1)
CFLAGS_anc_usb_app.o += -DANC_FB_ENABLED
endif

ccflags-y += $(BLUETOOTH_ADAPTER_INCLUDES)
ifeq ($(APP_ANC_TEST),1)
ccflags-y += -Iservices/audio_bt
endif

ifeq ($(TEST_OVER_THE_AIR_v2),1)
ccflags-y += -Iservices/tota_v2
endif

ifeq ($(TEST_OVER_THE_AIR),1)
ccflags-y += -Iservices/tota
endif
