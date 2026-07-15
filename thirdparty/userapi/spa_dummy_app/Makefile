cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))


obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)src/*.c))
obj_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)src/*.cpp))
obj_s += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)src/*.S))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

ifeq ($(SPA_AUDIO_SEC),1)
obj-y += sec/
endif

subdir-ccflags-y +=  \
    $(BT_SERVICE_UX_INCLUDES) \
    -Ithirdparty/userapi/spa_dummy_app/inc \
    -Iapps/sensorhub/ \
    -Iservices/bt_if_enhanced/inc \
    -Iapps/dsp_m55/ \
    -Iapps/audioplayers \
    -Iapps/common \
    -Iapps/earbuds/conn \
    -Iutils/heap \
    -Iutils/kfifo \
    -Iutils/cqueue 

