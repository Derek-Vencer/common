obj-y := 

ifeq ($(AI_OTA),1)
obj-y += ai_ota/
endif

ifeq ($(BES_OTA),1)
obj-y += bes_ota/
endif

ifeq ($(OTA_ENABLE),1)
obj-y += ota_basic.o
endif

ccflags-y += \
    $(BLUETOOTH_ADAPTER_INCLUDES) \
    $(BT_SERVICE_UX_INCLUDES) \
    $(BLE_APP_INCLUDES) \
    -Iservices/audio_bt \
    -Iplatform/drivers/bt \
    -Iservices/tota_v2 \
    -Iservices/ota/bes_ota/inc \
    -Iapps/key \
    -Iutils/cqueue \
    -Iservices/audio_manager \
    -Iservices/bt_app \
    -Iapps/key \
    -Iservices/bt_app/a2dp_codecs/include

ifeq ($(BESUI_TWS_EN),1)
subdir-ccflags-y += -Iapps/besui \
                    -Iapps/besui/tws_ui
endif
ifeq ($(BESUI_STEREO_EN),1)
subdir-ccflags-y += -Iapps/besui \
		  	        -Iapps/besui/stereo_ui
endif

