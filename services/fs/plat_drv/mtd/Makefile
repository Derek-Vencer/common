cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))
obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))

ifeq ($(NANDFLASH_SUPPORT), 1)
ccflags-y += -DNANDFLASH_SUPPORT

ifeq ($(NANDFLASH_MTD_ENABLE), 1)
ccflags-y += -DNANDFLASH_MTD_ENABLE
endif

ifeq ($(NAND_NFTL_CLEAR), 1)
ccflags-y += -DNAND_NFTL_CLEAR
endif

obj_c += ../../../../thirdparty/dhara_lib/
obj_c += ../../../../utils/cqueue/
endif

ifeq ($(NORFLASH_PARTITION_ENABLE),1)
ccflags-y += -DNORFLASH_PARTITION_ENABLE
endif

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

ccflags-y  += \
    -Iutils/boot_struct \
    -Iservices/fs \
    -Iutils \
    -Iservices/norflash_api

ifeq ($(NANDFLASH_SUPPORT), 1)
ccflags-y  += -Ithirdparty/dhara_lib \
              -Iplatform/drivers/nandflash
endif

