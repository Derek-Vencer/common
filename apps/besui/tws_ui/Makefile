cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.s))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

ccflags-y += \
    $(BLE_STACK_INCLUDES) \
    $(BLUETOOTH_ADAPTER_INCLUDES) \
    $(EPLAYER_INCLUDES) \
    $(ECOMM_INCLUDES) \
	-Iplatform/hal \
	-Iservices/bt_app \
	-Iutils/cqueue \
    -Iservices/multimedia/audio/codec/sbc/inc \
    -Iservices/multimedia/audio/codec/sbc/src/inc \
	-Iservices/ibrt_middleware/inc \
    -Iservices/audio_bt \
	-Iservices/communication \
    -Iservices/nv_section/factory_section \
    -Iapps/nv_section/factory_section \
    -Iapps/nv_section/userdata_section \
    -Iservices/ota/bes_ota/inc/ \
    -Iservices/audio_manager \
    -Iservices/audio_manager/audio_policy/bredr_policy/inc \
    -Iservices/ai_voice/manager \
    -Iservices/ai_voice/transport \
    -Iservices/ai_voice/protocol/gma/gma_manager \
    -Iservices/ai_voice/protocol/gma/gma_crypto \
    -Iservices/ai_voice/protocol/dual_mic_recording/voice_manager \
    -Iservices/ai_voice/protocol/bixbyvoice \
    -Iservices/ai_voice/protocol/bixbyvoice/bixbyvoice_manager \
    -Iservices/ai_voice/protocol/dual_mic_recording/voice_manager \
    -Iservices/ai_voice/protocol/smartvoice/smartvoice_manager \
    -Iservices/ai_voice/codec/compression \
    -Iservices/ai_voice/codec/opus121/include \
	-Iplatform/drivers/bt \
    -Iplatform/drivers/ana \
    -Iplatform/drivers/ana/best1306 \
    -Iplatform/drivers/ana/best1306p \
    -Iplatform/drivers/ana/best1503 \
    -Iplatform/drivers/capsensor/best1501p \
	-Iapps/factory \
	-Iapps/communication \
	-Iinclude/watchdog	\
    -Ithirdparty/userapi \
    -Ibthost/service/bt_app/inc \
    -Iutils/hsm

ifeq ($(ANC_APP),1)
ccflags-y += -Iapps/anc/src
endif

ifeq ($(GFPS_ENABLE),1)
ccflags-y += \
   -Iservices/ble_app/app_gfps \
   -Iservices/gfps/inc \
   -Ibthost/service/ble_app/inc \
   -I$(BLUETOOTH_BTH_DIR_PATH)/service/ble_app_new/inc  
endif

ifeq ($(BESUI_APP_EN),1)
subdir-ccflags-y += -Iservices/tota_v2 
endif

ifeq ($(IBRT),1)
ifeq ($(IBRT_UI),1)
ccflags-y += \
    -Iservices/ux/common/ibrt/app_ibrt/inc \
    -Iservices/ux/common/ibrt/ibrt_core/inc  \
    -Iservices/ux/common/ibrt/custom_api \
    -Iservices/ux/common/ibrt/ibrt_ui/inc \
    -Iservices/ux/product/earbud_tws/inc \
    -Iapps/earbuds/conn
endif
ccflags-y += \
    -Iservices/ux/common/ibrt/ibrt_middleware/inc 
endif

# ifeq ($(BESUI_CAPSENSOR_FACTORY_EN),1)
ccflags-y += -Iservices/capsensor/ \
    		 -Iplatform/drivers/capsensor/ \
             -Iplatform/drivers/ana/best1307/ \
             -Iplatform/drivers/ana/best1307s/ \
             -Iplatform/drivers/ana/best1307p/     
# endif
ifeq ($(CAPSENSOR_ENABLE),1)
subdir-ccflags-y += -Iservices/nv_section/factory_section \
                    -Iservices/capsensor/capsensor_algorithm \
                    -Iapps/capsensor \
					-Iapps/battery
endif