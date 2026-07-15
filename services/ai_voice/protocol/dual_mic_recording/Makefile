cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.s))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)voice_ble/*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)voice_bt/*.cpp))
obj_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)voice_manager/*.cpp))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

ifeq ($(VOC_ENCODE_SCALABLE), 1)
obj-y +=../../../../thirdparty/binaural_lib/
endif
RECORDING_LIB_NAME := lib_recording
$(RECORDING_LIB_NAME)-y := $(obj-y)
obj-y :=  $(RECORDING_LIB_NAME).a

ccflags-y += \
    -Iapps/main \
    -Iapps/app_anker/device_drivers/bes_hal \
    -Iapps/audioplayers \
    -Iapps/battery \
    -Iapps/key \
    -Iapps/bt_sync \
    -Iapps/earbuds/conn \
    -Iplatform/hal \
    -Iplatform/drivers/ana \
    -Iplatform/drivers/bt \
    -Iservices/ai_voice/algorithm \
    -Iservices/ai_voice/audio \
    -Iservices/ai_voice/codec/compression \
    -Iservices/ai_voice/codec/sbc \
    -Iservices/ai_voice/transport \
    -Iservices/ai_voice/manager \
    -Iservices/ai_voice/protocol/dual_mic_recording/voice_gatt_v2 \
    -Iservices/ai_voice/protocol/dual_mic_recording/voice_manager \
    -Iservices/ai_voice/protocol/dual_mic_recording/voice_ble \
    -Iservices/ai_voice/protocol/dual_mic_recording/voice_bt \
    -Iservices/ai_voice/protocol/common_api/voice_bt \
    -Iservices/ai_voice/protocol/common_api/voice_ble \
    -Iservices/ai_voice/protocol/common_api/voice_gatt \
    -Iservices/app_ai/inc \
    -Iservices/app_ibrt/inc/  \
    -Iservices/bt_app \
    -Iservices/bt_app/a2dp_codecs/include \
    $(BLUETOOTH_ADAPTER_INCLUDES) \
    $(BT_SERVICE_UX_INCLUDES) \
    -Iservices/ai_voice/codec/opus121/include \
    -Iservices/resources \
    -Iservices/ibrt_core/inc \
    -Iutils/cqueue \
    -Iutils/heap \
    -Iutils/crc \
    -Iservices/app_ai/inc \
    -Iservices/nv_section/factory_section \
    -Ithirdparty/binaural_lib \
    -Iservices/ibrt/app_ibrt/inc \
    -Iservices/ibrt/ibrt_ui/inc
