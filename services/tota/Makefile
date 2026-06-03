cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

src_obj := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

GEN_LIB_NAME := libtota

root_dir := $(abspath $(dir $(realpath $(firstword $(MAKEFILE_LIST))))/..)
sinclude $(root_dir)/config/lib.mk
$(GEN_LIB_NAME)-y := $(src_obj)

obj-y += $(GEN_LIB_NAME).a

CFLAGS_app_tota.o += -DBESLIB_INFO=$(BESLIB_INFO)

subdir-ccflags-y += -Iplatform/drivers/ana \
                    -Iplatform/hal \
                    -Iservices/bluedroid/adapter/src \
                    -Iapps/key \
                    -Iservices/audio_bt \
                    -Iservices/audio_eq \
                    -Iservices/audio_manager \
                    $(BLUETOOTH_ADAPTER_INCLUDES) \
                    $(BT_SERVICE_UX_INCLUDES) \
                    -Iservices/overlay \
                    -Iservices/resources \
                    -Iplatform/drivers/uarthci \
                    -Iplatform/drivers/ana \
                    -Iplatform/drivers/bt \
                    -Iutils/cqueue \
                    -Iutils/heap \
                    -Iutils/retention_ram \
                    -Iservices/audioflinger \
                    -Iutils/lockcqueue \
                    -Iutils/intersyshci \
                    -Iutils/boot_struct \
                    -Iapps/main \
                    -Iapps/common \
                    -Iapps/audioplayers \
                    -Iapps/factory \
                    -Iservices/bridge/ \
                    -Iservices/bt_app \
                    -Imultimedia/speech/codec/codecs/adpcm \
                    -Iservices/anc_spp_tool \
                    -Iservices/tws/inc \
                    -Iservices/ibrt_core/inc \
                    -Iutils/crc \
                    -Ithirdparty/userapi \
                    -Iapps/battery \
                    -Iservices/nv_section/factory_section \
                    -Iservices/nv_section/log_section
