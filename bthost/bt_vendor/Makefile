cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj-y := $(obj_cpp:.cpp=.o)

ccflags-y += \
	$(BLUETOOTH_ADAPTER_INCLUDES)/inc \
 	-Iplatform/drivers/bt \
	-Ibthost/stack/common
