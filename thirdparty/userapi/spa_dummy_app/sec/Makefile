
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

ccflags-y := \
	-Iplatform/cmsis/inc \
	-Iplatform/hal \
	-Iplatform/drivers/ana \
	-Iutils/heap \
	-Iutils/crc \
	-Iservices/norflash_api

ifeq ($(SPA_AUDIO_SEC),1)
obj-y += ../../../../utils/mbedtls/

ccflags-y += -Iutils/heap
ccflags-y += -Iutils/mbedtls
ccflags-y += -Iutils/mbedtls/configs
ccflags-y += -Iutils/mbedtls/library
ccflags-y += -Iutils/mbedtls/include
ccflags-y += -Iutils/mbedtls/include/mbedtls
ccflags-y += -Iutils/mbedtls/include/psa
ifeq ($(TZ_ROM_UTILS_IF),1)
ccflags-y += -Iutils/rom_utils

ccflags-y += $(BES_MULTIMEDIA_INCLUDES)
endif

ifneq ($(MBEDTLS_CONFIG_FILE),)
subdir-ccflags-y += -DMBEDTLS_CONFIG_FILE='"$(MBEDTLS_CONFIG_FILE)"'
else
subdir-ccflags-y += -DMBEDTLS_CONFIG_FILE='<config-rsa.h>'
endif

endif

ifeq ($(ARM_CMNS),1)
EXCLUDE_OBJS := 
EXCLUDE_OBJS += tz_audio_process.o tz_audio_process_se.o
obj-y := $(filter-out $(EXCLUDE_OBJS),$(obj-y))
endif
