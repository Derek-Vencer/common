cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.s))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))
obj_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)smartvoice_manager/*.cpp))
obj_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)smartvoice_bt/*.cpp))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)smartvoice_ble/*.c))

ifeq ($(SMART_VOICE_KWS_AQE),1)
obj_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)smartvoice_kws/*.cpp))
endif

src_obj := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

GEN_LIB_NAME := lib_smartvoice
root_dir := $(abspath $(dir $(realpath $(firstword $(MAKEFILE_LIST))))/..)
sinclude $(root_dir)/config/lib.mk

$(GEN_LIB_NAME)-y := $(src_obj)

obj-y :=  $(GEN_LIB_NAME).a

SMARTVOICE_BLE_INCLUDES += \
              -Iservices/ai_voice/protocol/smartvoice/smartvoice_gatt_v2

ccflags-y +=  -Iapps/main \
              -Iapps/app_anker/device_drivers/bes_hal \
              -Iapps/audioplayers \
              -Iapps/key \
              -Iplatform/hal \
              -Iplatform/drivers/bt \
              -Iservices/ai_voice/algorithm \
              -Iservices/ai_voice/protocol/smartvoice/smartvoice_ble \
              -Iservices/ai_voice/protocol/smartvoice/smartvoice_bt \
              $(SMARTVOICE_BLE_INCLUDES) \
              -Iservices/ai_voice/protocol/smartvoice/smartvoice_manager \
              -Iservices/ai_voice/audio \
              -Iservices/ai_voice/codec/compression \
              -Iservices/ai_voice/codec/opus121/include \
              -Iservices/ai_voice/transport \
              -Iservices/ai_voice/manager \
              -Iservices/ai_voice/codec/sbc \
              -Iservices/app_ai/inc \
              -Iservices/bt_app \
              -Iservices/bt_app/a2dp_codecs/include \
              -Iservices/resources \
              -Iservices/nv_section/factory_section \
              -Ithirdparty/userapi \
              -Iutils/cqueue \
              -Iutils/crc \
              -Iutils/heap
