
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

ifeq ($(VPU_NAME),LSM6DSV16BX)
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)/lsm6dsv16bx/*.c))
CFLAGS_vpu_cfg.o += -DVPU_NAME=LSM6DSV16BX
else
$(error Need to set VPU_NAME)
endif

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

ifeq ($(VPU_CFG_ON_SENSOR_HUB),1)
CFLAGS_vpu_cfg.o += -DVPU_CFG_ON_SENSOR_HUB
endif

ccflags-y := \
	-Iservices/audio_dump/include