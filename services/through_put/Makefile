cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))
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
                    -Iservices/ai_voice/audio \
                    -Iservices/ai_voice/transport \
                    -Iservices/ai_voice/manager \
                    -Iservices/ai_voice/protocol/ama/ama_stream \
                    -Iservices/app_ai/inc \
                    $(BLUETOOTH_ADAPTER_INCLUDES) \
                    $(BT_SERVICE_UX_INCLUDES) \
                	-Imultimedia/inc \
                    -Imultimedia/inc/speech/inc \
                    -Iservices/cp_accel \
                    -Ithirdparty/userapi \
                    -Iutils/cqueue \
                    -Iservices/audio_manager \
                    -Iutils/heap
