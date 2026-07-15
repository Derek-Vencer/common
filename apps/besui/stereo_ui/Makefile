cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.s))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)



ccflags-y += \
    $(BLUETOOTH_ADAPTER_INCLUDES) \
    $(EPLAYER_INCLUDES) \
    $(ECOMM_INCLUDES) \
	-Iinclude/watchdog \
	-Iplatform/hal \
	-Iapps/factory \
	-Iservices/audio_bt \
	-Iservices/osif \
	-Iservices/bt_app \
	-Iservices/tota_v2 \
	-Iservices/multimedia/audio/codec/sbc/inc \
    -Iservices/multimedia/audio/codec/sbc/src/inc \
	-Iservices/ibrt_middleware/inc \
	-Iservices/communication \
    -Iservices/nv_section/userdata_section \
    -Iservices/nv_section/factory_section \
    -Iservices/audio_manager/audio_policy/bredr_policy/inc \
	-Iplatform/drivers/bt \
    -Iplatform/drivers/ana \
    -Iplatform/drivers/ana/best1306 \
	-Iutils/cqueue \
	-Iutils/hsm

ifeq ($(BES_OTA),1)
ccflags-y += -Iservices/ota/bes_ota/inc
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

ifeq ($(GFPS_ENABLE),1)
ccflags-y += \
   -Iservices/ble_app/app_gfps \
   -Iservices/gfps/inc
endif

ifeq ($(BESUI_CAPSENSOR_FACTORY_EN),1)
ccflags-y += -Iservices/capsensor/ \
    		 -Iplatform/drivers/capsensor/ \
			 -Iplatform/drivers/capsensor/best1306
endif

ifeq ($(DOLBY_AUDIO_ENABLE),1)
ccflags-y += \
    -Ithirdparty/userapi/spa_dummy_app/inc\
    -Ithirdparty/userapi/spa_dummy_app/sec\
	-Ithirdparty/dolby_lib \
    -Ithirdparty/dolby_lib/inc
endif
ifeq ($(BESUI_APP_EN),1)
subdir-ccflags-y += -Iapps/nv_section/factory_section \
					-Iapps/battery
endif
