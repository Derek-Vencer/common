
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

obj-y += ../../../utils/mbedtls/
obj-y += ../../../utils/heap/
ccflags-y += -Iutils/heap
ccflags-y += -Iutils/mbedtls
ccflags-y += -Iutils/mbedtls/configs
ccflags-y += -Iutils/mbedtls/library
ccflags-y += -Iutils/mbedtls/include
ccflags-y += -Iutils/mbedtls/include/mbedtls
ccflags-y += -Iutils/mbedtls/include/psa

ifneq ($(MBEDTLS_CONFIG_FILE),)
ccflags-y += -DMBEDTLS_CONFIG_FILE='"$(MBEDTLS_CONFIG_FILE)"'
endif

CFLAGS_mbedtls_test.o += -DMBEDTLS_TEST
