cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)accessory_authen/src/*.c))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)accessory_ident/src/*.c))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)app_launch/src/*.c))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)external_accessory_protocol/src/*.c))

obj-y := $(obj_c:.c=.o)

subdir-ccflags-y += \
    -Ibthost/service/iap2/features \
    -Ibthost/service/iap2/features/accessory_authen/src \
    -Ibthost/service/iap2/features/accessory_authen \
    -Ibthost/service/iap2/features/accessory_ident/src \
    -Ibthost/service/iap2/features/accessory_ident \
    -Ibthost/service/iap2/features/app_launch/src \
    -Ibthost/service/iap2/features/app_launch \
    -Ibthost/service/iap2/features/external_accessory_protocol/src \
    -Ibthost/service/iap2/features/external_accessory_protocol \
    -Ibthost/service/iap2/apple_cert/adapter \
    -Ibthost/service/iap2/protocol \
    -Ibthost/service/iap2/protocol/adapter \
    -Ibthost/service/iap2/transport \
    -Ibthost/adapter/inc/bt/common

