cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))
obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)

ccflags-y += \
    -Iservices/capsensor/ \
    -Iplatform/drivers/capsensor/ \
    -Iapps/sensorhub/


ifeq ($(CAPSENSOR_AT_SENS),1)
KBUILD_CFLAGS += -DCAPSENSOR_AT_SENS
endif

ifeq ($(CAPSENSOR_AT_MCU),1)
KBUILD_CFLAGS += -DCAPSENSOR_AT_MCU
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

ifeq ($(BESUI_TWS_EN),1)
ccflags-y += \
    -Iapps/factory \
    -Iapps/common_ui \
    -Iapps/tota_v2 \
	-Iplatform/drivers/bt \
    -Iutils/cqueue \
    -Iplatform/drivers/capsensor/$(CHIP)/
endif

ifeq ($(BES_OTA),1)
subdir-ccflags-y += -Iservices/ota/bes_ota/inc
endif

ifeq ($(BESUI_APP_EN),1)
subdir-ccflags-y += -Iapps/tota_v2 
endif

ifeq ($(BESUI_TWS_EN),1)
subdir-ccflags-y += -Iapps/besui \
                    -Iapps/besui/tws_ui
endif
ifeq ($(BESUI_STEREO_EN),1)
subdir-ccflags-y += -Iapps/besui \
		            -Iapps/besui/stereo_ui
endif
