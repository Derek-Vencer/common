cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c   := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

ccflags-y := \
    -Iplatform/drivers/emmc/host/inc \
    -Iplatform/drivers/emmc/host/card \
    -Iplatform/drivers/emmc/host/core \
    -Iplatform/drivers/emmc/host/host \
    -Iplatform/drivers/emmc/host/hci \
    -Iservices/fs


