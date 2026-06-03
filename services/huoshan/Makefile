cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))
obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)

subdir-ccflags-y += \
    -Iapps/common \
    -Iapps/key \
    -Iapps/main \
    -Iapps/voice_assist/inc \
    -Iapps/audioplayers \
    -Iapps/factory \
    -Iplatform/drivers/ana \
    -Iplatform/drivers/bt \
    -Iplatform/hal \
    -Irtos/rtx/TARGET_CORTEX_M \
    $(BLUETOOTH_ADAPTER_INCLUDES) \
    $(BLE_APP_INCLUDES) \
    $(BLE_STACK_INCLUDES) \
    $(BT_SERVICE_UX_INCLUDES) \
    -Iinclude/cmsis_dsp \
    -Iservices/overlay \
    -Iservices/resources \
    -Ithirdparty/userapi \
    -Iutils/cqueue \
    -Iutils/heap \
    -Iutils/kfifo \
    -Iutils/lockcqueue \
    -Iutils/retention_ram \
    -Iservices/huoshan \
    -Iservices/norflash_api \
    -Iservices/ai_voice/protocol/smartvoice/smartvoice_manager
