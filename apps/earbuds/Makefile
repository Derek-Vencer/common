
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)conn/*.S))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)conn/*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)conn/*.cpp))

obj_s += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)app_ibrt/src/*.s))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)app_ibrt/src/*.c))
obj_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)app_ibrt/src/*.cpp))
obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

subdir-ccflags-y += \
    $(BLUETOOTH_ADAPTER_INCLUDES) \
    $(BT_SERVICE_UX_INCLUDES) \
    -Iservices/osif \
    -Iservices/nv_section/factory_section \
    -Iservices/app_ai/inc \
    -Iservices/ai_voice/transport \
    -Iservices/ai_voice/manager \
    -Iplatform/drivers/bt \
    -Iplatform/drivers/sensor_hub \
    -Iutils/cqueue \
    -Iutils/heap \
    -Iservices/audioflinger \
    -Iutils/lockcqueue \
    -Iutils/intersyshci \
    -Iutils/list \
    -Iapps \
    -Iapps/key \
    -Iapps/main \
    -Iapps/common \
    -Iapps/audioplayers \
    -Iapps/audioplayers/a2dp_decoder \
    -Iapps/factory \
    -Iservices/nv_section/log_section \
    -Iservices/audio_bt/ \
    -Iservices/ai_voice/protocol/gma/gma_manager \
    -Iservices/ai_voice/protocol/gma/gma_crypto \
    -Iservices/ai_voice/protocol/dual_mic_recording/voice_manager \
    -Iservices/ai_voice/protocol/bixbyvoice \
    -Iservices/ai_voice/protocol/bixbyvoice/bixbyvoice_manager \
    -Iservices/ai_voice/protocol/dual_mic_recording/voice_manager \
    -Iservices/ai_voice/codec/compression \
    -Iservices/ai_voice/codec/opus121/include \
    -Iservices/norflash_api \
    -Ibthost/service/bt_app/inc \
    -Iservices/audio_manager \
    -Iservices/audio_manager/test \
    -Iapps/bt_sync \
    -Iutils/hsm \
    -Iapps/earbuds/conn/ \
    -Iservices/gfps/inc \
    -Ibthost/service/ble_app_new/inc \
    -Iservices/audio_manager/audio_policy/bredr_policy/inc \
    -Iapps/earbuds/conn/

ifeq ($(BLE_AUDIO_ENABLED),1)
subdir-ccflags-y += \
    $(BLE_STACK_INCLUDES)
endif

ifeq ($(BES_OTA),1)
subdir-ccflags-y += \
    -Iservices/ota/bes_ota/inc
endif

# ifeq ($(A2DP_AUDIO_STEREO_MIX_CTRL),1)
# CFLAGS_app_ibrt_customif_ui.o += -DA2DP_AUDIO_STEREO_MIX_CTRL
# endif

ifeq ($(SPA_AUDIO_ENABLE),1)
subdir-ccflags-y += -Ithirdparty/userapi/spa_dummy_app/inc
ifeq ($(SPA_AUDIO_SEC),1)
ccflags-y += -Ithirdparty/userapi/spa_dummy_app/sec
endif
endif

ifeq ($(BISTO_ENABLE),1)
subdir-ccflags-y += \
    -Iservices/ai_voice/protocol/gva/gsound_custom/inc \
    -Iservices/ai_voice/protocol/gva/gsound_target_api_read_only
endif

ifeq ($(FINDMY_ENABLE),1)
subdir-ccflags-y += \
    -Ithirdparty/ios-find-my/inc \
    -Iutils/kfifo
endif

ifeq ($(BESUI_APP_EN),1)
subdir-ccflags-y += -Iservices/tota_v2 
endif

ifeq ($(BESUI_TWS_EN),1)
ccflags-y += -Iapps/besui/tws_ui
subdir-ccflags-y += -Iplatform/drivers/capsensor/best1306 \
                    -Iplatform/hal/best1306 \
                    -Iplatform/drivers/ana/best1501p
endif

ifeq ($(BESUI_COMM_EN),1)
ccflags-y += -Iapps/besui
endif

