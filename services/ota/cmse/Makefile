cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.S))

ifneq ($(SPA_AUDIO_SEC),1)
obj-y += ../../../utils/mbedtls/
endif

obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.S=.o)

ccflags-y +=-Iutils/mbedtls/ \
			-Iutils/crc/ \
			-Iutils/heap \
			-Iutils/mbedtls/include \
			-Iutils/user_secure_boot \
			-Iservices/norflash_api/cmse \

