cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c := 

obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))

obj-y := $(obj_c:.c=.o)

subdir-ccflags-y += \
    -Ibthost/service/iap2/protocol/adapter \
    -Ibthost/service/iap2/protocol/protocol \
    $(BLUETOOTH_ADAPTER_INCLUDES) \
    -Ibthost/adapter/inc/bt/common \
    -Ibthost/service/iap2/transport


