cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))
obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)

ccflags-y += \
    -Iplatform/drivers/ana/ \
    -Iservices/capsensor/ \
    -Iplatform/drivers/capsensor/ \
    -Iplatform/drivers/ana/best1503/

ifeq ($(CAPSENSOR_FP_MODE),1)
KBUILD_CFLAGS  += -DCAPSENSOR_FP_MODE
endif
ifeq ($(CAPSENSOR_PRESS_INT),1)
KBUILD_CFLAGS  += -DCAPSENSOR_PRESS_INT
endif
