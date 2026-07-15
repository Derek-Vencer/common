cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c := 

obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)src/*.c))

obj-y := $(obj_c:.c=.o)

subdir-ccflags-y += \
    -Ibthost/service/iap2/apple_cert/adapter \
    -Ibthost/service/iap2/apple_cert/cert