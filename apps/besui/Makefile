cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.s))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)


ifeq ($(BESUI_TWS_EN),1)
obj-y += tws_ui/
endif

ifeq ($(BESUI_STEREO_EN),1)
obj-y += stereo_ui/
endif

ccflags-y += \
    $(BLUETOOTH_ADAPTER_INCLUDES) \
    $(EPLAYER_INCLUDES) \
    $(ECOMM_INCLUDES) \
	-Iapps/apptester \
	-Iapps/audioplayers \
    -Iapps/audioplayers/a2dp_decoder \
	-Iapps/anc/inc \
	-Iapps/voice_assist/inc \
	-Iapps/common \
	-Iapps/sdmmc \
	-Iapps/sdmmc_msd \
	-Iapps/main \
	-Iapps/cmd \
	-Iapps/key \
	-Iapps/pwl \
	-Iapps/battery \
	-Iutils/list \
	-Iutils/heap \
	-Iservices/voicepath \
	-Iservices/audio_dump/include \
    -Iservices/tota_v2 \
	-Iservices/audio_manager \
	-Iservices/multimedia/audio/process/filters/include \
    -Iservices/btservice/base/inc \
    -Iservices/btservice/module/bt/inc \
    -Iservices/btservice/module/bt/if \
    -Iservices/btservice/module/tws/if \
    -Iservices/btservice/module/common/if \
    -Ithirdparty/userapi \
    
ifeq ($(BESUI_STEREO_EN),1)
ccflags-y += -Iapps/besui/stereo_ui
endif

ifeq ($(CAPSENSOR_ENABLE),1)
subdir-ccflags-y += -Iservices/nv_section/factory_section \
                    -Iapps/capsensor \
                    -Iplatform/drivers/capsensor/best1306p \
					-Iapps/battery
endif

#########################################################################
ifeq ($(ROM_UTILS_ON)-$(CHIP),1-best1306)
ccflags-y += \
	-Iservices/multimedia/audio/codec/sbc_rom_adapter/inc
else
ifeq ($(SBC_REDUCE_SIZE),1)
ccflags-y += \
	-Iservices/multimedia/audio/codec/sbc_reduce/inc \
	-Iservices/multimedia/audio/codec/sbc_reduce/src/inc
else
ccflags-y += \
	-Iservices/multimedia/audio/codec/sbc/inc \
	-Iservices/multimedia/audio/codec/sbc/src/inc
endif
endif

ccflags-y +=  \
    $(BLE_STACK_INCLUDES) \
    $(BLUETOOTH_ADAPTER_INCLUDES) \
    $(EPLAYER_INCLUDES) \
    $(ECOMM_INCLUDES) \
    -Iservices/audio_bt \
    -Iservices/audioflinger \
    -Iservices/norflash_api \
    -Iservices/nv_section/factory_section \
    -Iservices/nv_section/log_section \
    -Iapps/nv_section/factory_section \
    -Iservices/overlay \
    -Iservices/resources \
    -Iservices/audio_process \
    -Iutils/intersyshci \
    -Iapps/apptester \
    -Iapps/factory \
    -Iapps/voice_detector \
    -Iutils/crc32 \
    -Iutils/list \
    -Iplatform/drivers/bt \
    -Iplatform/drivers/ana \
    -Iapps/audioplayers/rbplay \
    -Itests/anc_usb \
    -Iapps/anc/inc \
    -Iapps/btspeaker \
    -Ithirdparty/userapi \
    -Iapps/communication \
    -Iutils/cqueue \
    -Iservices/ai_voice/audio \
    -Iservices/ai_voice/manager \
    -Iservices/ai_voice/transport \
    -Iservices/app_ai/inc \
    -Iservices/interconnection/red \
    -Iservices/interconnection/green \
    -Iservices/bridge/ \
    -Iservices/bt_app/a2dp_codecs/include \
    -Iservices/aob_app/inc \
    -Iservices/aob_bes_ux/inc \
    -Iservices/aob_custom_ux \
    -Ithirdparty/tile \
    -Iservices/ux/common/ibrt/ibrt_middleware/inc  \
    -Iservices/rpc/inc \
    -Ithirdparty/tile/tile_target \
    -Iinclude/ \
    -Iapps/btwatch \
    -Iapps/sensorhub \
    -Iapps/dsp_m55 \
    -Iplatform/drivers/cp_accel \
    -Iservices/audio_manager \
    -Iservices/audio_manager/audio_policy/bredr_policy/inc \
    -Iplatform/main/cmse

ifeq ($(DOLBY_AUDIO_ENABLE),1)
ccflags-y += \
    -Ithirdparty/userapi/spa_dummy_app/inc\
    -Ithirdparty/userapi/spa_dummy_app/sec\
	-Ithirdparty/dolby_lib \
    -Ithirdparty/dolby_lib/inc
endif

ifeq ($(BESUI_APP_EN),1)
subdir-ccflags-y += -Iapps/tota_v2 
endif

ifeq ($(BESUI_TWS_EN),1)
ccflags-y += -Iapps/besui \
             -Iapps/besui/tws_ui
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
endif


ifeq ($(APP_TEST_AUDIO),1)
CFLAGS_apps_tester.o += -DAPP_TEST_AUDIO
endif

ifeq ($(OTA_ENABLE),1)
ccflags-y += -Iservices/ota
endif

ifeq ($(BES_OTA),1)
ccflags-y += -Iservices/ota/bes_ota/inc
endif

ifeq ($(AI_OTA),1)
ccflags-y += -Iservices/ota/ai_ota
endif

ifeq ($(GFPS_ENABLE),1)
ccflags-y += \
   -Iservices/ble_app/app_gfps \
   -Iservices/gfps/inc \
   -Ibthost/service/ble_app/inc \
   -I$(BLUETOOTH_BTH_DIR_PATH)/service/ble_app_new/inc
endif

ifeq ($(BLE),1)
ccflags-y += -I$(BLUETOOTH_BTH_DIR_PATH)/service/ble_app_new/inc
endif

ifeq ($(MOTION_ENGINE_EN),1)
ccflags-y += -Iapps/motionhub
endif

ifeq ($(BESUI_COMM_EN),1)
subdir-ccflags-y += -Wno-date-time
endif

ifeq ($(BESUI_CAPSENSOR_FACTORY_EN),1)
ccflags-y += -Iservices/capsensor/ \
    		 -Iplatform/drivers/capsensor/ \
			 -Iplatform/drivers/capsensor/best1306 \
			 -Iplatform/drivers/capsensor/best1306p
endif
#########################################################################
