cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := adapter/
obj-y += protocol/

subdir-ccflags-y += \
    -Ibthost/service/iap2/protocol/adapter \
    -Ibthost/service/iap2/protocol/src \
    -Ibthost/service/iap2/protocol \
    $(BLUETOOTH_ADAPTER_INCLUDES)

