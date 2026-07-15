cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

src_obj := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

GEN_LIB_NAME := libcapsensor_algorithm

ifeq ($(CAPSENSOR_WEAR_USE_ONE_PAD), 1)
GEN_LIB_NAME := $(GEN_LIB_NAME)_one_pad
endif

root_dir := $(abspath $(dir $(realpath $(firstword $(MAKEFILE_LIST))))/..)
sinclude $(root_dir)/config/lib.mk
$(GEN_LIB_NAME)-y := $(src_obj)
obj-y := $(GEN_LIB_NAME).a

ccflags-y +=  \
    -Iconfig/sensor_hub/ \
    -Iplatform/drivers/capsensor/

ifeq ($(CAPSENSOR_TRACE_DEBUG),1)
KBUILD_CFLAGS += -DCAPSENSOR_TRACE_DEBUG
endif
