cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))

ifeq ($(VOC_ENCODE_OPUS),1)
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)opus121/*.c))
endif

obj_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)sbc/*.cpp))
obj_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)compression/*.cpp))

src_obj := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

GEN_LIB_NAME := lib_codec
root_dir := $(abspath $(dir $(realpath $(firstword $(MAKEFILE_LIST))))/..)
sinclude $(root_dir)/config/lib.mk

$(GEN_LIB_NAME)-y := $(src_obj)

obj-y += $(GEN_LIB_NAME).a

subdir-ccflags-y +=	\
    -Iservices/audioflinger \
    -Iapps/audioplayers \
    -Iapps/earbuds/conn \
    -Iservices/ai_voice/codec/sbc \
    -Iservices/ai_voice/codec/opus121 \
	-Imultimedia/inc \
    -Imultimedia/inc/speech/inc \
    -Iservices/bt_app \
    $(BLUETOOTH_ADAPTER_INCLUDES) \
    $(BES_MULTIMEDIA_INCLUDES) \
    -Iutils/cqueue \
    -Iutils/heap \
    -Iutils/lockcqueue \
    -Iutils/retention_ram \
    -Ithirdparty/binaural_lib \
    -Iservices/app_ibrt/inc \
    -Iservices/ibrt_core/inc
