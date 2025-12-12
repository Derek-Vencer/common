cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.s))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

subdir-ccflags-y += -Iapps/common \
                    -Iapps/key \
                    -Iapps/main \
                    -Iplatform/drivers/bt \
                    -Iplatform/hal \
                    $(BLUETOOTH_ADAPTER_INCLUDES) \
                    $(BT_SERVICE_UX_INCLUDES) \
                	-Imultimedia/inc \
                    -Imultimedia/inc/speech/inc \
                    -Iservices/cp_accel \
                    -Ithirdparty/userapi \
                    -Iutils/cqueue \
                    -Iservices/audio_manager \
                    -Iutils/heap \
				    -I$(BLE_STACK_INCLUDES) \
				    -Ibthost/stack/ble_stack/inc/l2cap \
				    -Ibthost/stack/ble_stack/inc/gap \
				    -Ibthost/stack/ble_stack/inc/gatt \
				    -I$(BLE_APP_INCLUDES) \
				    $(BT_SERVICE_UX_INCLUDES) \
				    -I$(BT_APP_DIR_PATH)/inc \
				    -I$(BT_APP_DIR_PATH)/inc/a2dp_codecs \
				    -I$(BT_IF_DIR_PATH)/inc \
				    -I$(BT_PROFILES_DIR_PATH)/inc \
				    -I$(BLUETOOTH_ADAPTER_DIR_PATH)/inc/adapter_service \
				    -I$(BLUETOOTH_ADAPTER_DIR_PATH)/inc/bt/common \
				    -I$(BLUETOOTH_ADAPTER_DIR_PATH)/inc/bt \
				    -I$(BLUETOOTH_ADAPTER_DIR_PATH)/inc/bt/me \
				    -I$(BLUETOOTH_ADAPTER_DIR_PATH)/inc/bt/ibrt \
				    -I$(BLUETOOTH_ADAPTER_DIR_PATH)/inc/bt/l2cap \
				    -I$(BLUETOOTH_ADAPTER_DIR_PATH)/inc/bt/a2dp \
				    -I$(BLUETOOTH_ADAPTER_DIR_PATH)/inc/bt/avrcp \
				    -I$(BLUETOOTH_ADAPTER_DIR_PATH)/inc/bt/hfp \
				    -I$(BLUETOOTH_ADAPTER_DIR_PATH)/inc/bt/dip \
				    -I$(BLUETOOTH_ADAPTER_DIR_PATH)/inc/bt/source \
				    -I$(BLUETOOTH_ADAPTER_DIR_PATH)/inc/ble/common \
				    -I$(BLUETOOTH_ADAPTER_DIR_PATH)/inc/ble \
				    -I$(BLUETOOTH_ADAPTER_DIR_PATH)/inc/ble/gap \
				    -I$(BLUETOOTH_ADAPTER_DIR_PATH)/inc/ble/gatt \
				    -I$(BLUETOOTH_ADAPTER_DIR_PATH)/inc/ble/aob \
				    -I$(BLUETOOTH_ADAPTER_DIR_PATH)/inc/ble/gfps \
				    -I$(BLUETOOTH_ADAPTER_DIR_PATH)/inc/ble/dp \
				    -I$(BLUETOOTH_ADAPTER_DIR_PATH)/inc/ble/ai \
				    -Ibthost/porting \
				    -Iservices/gfps/inc \
				    -Iutils/cqueue \
				    -Iutils/encrypt \
				    $(BES_MULTIMEDIA_INCLUDES) \
				    -Iplatform/drivers/bt \
				    -Iapps/key \
				    -Iapps/main \
				    -Iservices/audio_manager/audio_policy/bredr_policy/inc \
				    -Iapps/earbuds/conn \
				    -Iservices/audio_manager \
				    -Iservices/audio_bt \
				    -Iutils/hwtimer_list \
				    -Iservices/nv_section/customparam_section \
				    -Ibthost/service/ble_app_new/inc  \
				    -Iutils/hsm \
				    -Ibthost/adapter/inc/ble/aob
				    
				    
