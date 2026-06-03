cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c :=
obj_cpp :=
obj_s :=

obj_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)tencent_manager/*.cpp))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)tencent_manager/*.c))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)tencent_ble/*.c))
obj_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)tencent_bt/*.cpp))

obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)tencent_gatt_v2/*.c))

TENCENT_BLE_INCLUDES += \
              -Iservices/ai_voice/protocol/tencent/tencent_gatt_v2

src_obj := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

GEN_LIB_NAME := lib_tencent
root_dir := $(abspath $(dir $(realpath $(firstword $(MAKEFILE_LIST))))/..)
sinclude $(root_dir)/config/lib.mk

$(GEN_LIB_NAME)-y := $(src_obj)

obj-y := $(GEN_LIB_NAME).a

ccflags-y +=  -Iapps/key \
			  -Iapps/main \
			  -Iapps/audioplayers \
              -Iapps/battery \
			  -Iplatform/drivers/bt \
			  -Iplatform/hal \
			  -Iservices/ai_voice/protocol/tencent/tencent_ble \
			  -Iservices/ai_voice/protocol/tencent/tencent_bt \
			  $(TENCENT_BLE_INCLUDES) \
			  $(BT_SERVICE_UX_INCLUDES) \
			  -Iservices/ai_voice/protocol/tencent/tencent_manager \
			  -Iservices/ai_voice/algorithm \
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
			  -Itests/rom_cust \
			  -Iutils/crc \
			  -Iutils/cqueue \
			  -Iutils/heap \
			  -Iutils/kfifo
