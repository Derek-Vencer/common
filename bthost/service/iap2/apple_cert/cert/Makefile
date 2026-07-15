cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c := 
obj_c += mfi/
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)src/*.c))

obj-y := $(obj_c:.c=.o)

subdir-ccflags-y += \
    -Iservices/MFi/apple_cert/adapter \
    -Iservices/MFi/apple_cert/cert/ \
    -Iservices/MFi/apple_cert/cert/mfi \
    -Iservices/MFi/apple_cert/cert/src \
    -Iplatform/hal/