cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))
obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))

obj_c += mtd/ cache/

ifeq ($(AOS_FS_EMMC_CACHE_ENABLE),1)
subdir-ccflags-y += -DFS_EMMC_CACHE_ENABLE
endif

ifeq ($(AOS_FS_SDMMC_CACHE_ENABLE),1)
subdir-ccflags-y += -DFS_SDMMC_CACHE_ENABLE
endif

ifeq ($(EMMC_ENABLE), 1)
obj_c += emmc/
subdir-ccflags-y += -DEMMC_ENABLE
ifeq ($(EMMC_PARTITION_ENABLE), 1)
subdir-ccflags-y += -DEMMC_PARTITION_ENABLE
ifneq ($(AOS_EMMC_PART_NUM),)
subdir-ccflags-y += -DAOS_EMMC_PART_NUM=$(AOS_EMMC_PART_NUM)
endif
endif
endif

ifeq ($(SDMMC_ENABLE), 1)
obj_c += sdemmc/
subdir-ccflags-y += -DSDMMC_ENABLE
ifeq ($(SDMMC_PARTITION_ENABLE), 1)
subdir-ccflags-y += -DSDEMMC_PARTITION_ENABLE
ifneq ($(AOS_SDMMC_PART_NUM),)
subdir-ccflags-y += -DAOS_SDMMC_PART_NUM=$(AOS_SDMMC_PART_NUM)
endif
endif
endif

ifeq ($(USBHOST_MASS_ENABLE), 1)
subdir-ccflags-y += -DUSBHOST_MASS_ENABLE
obj_c += usbhost_mass/
endif

ifeq ($(NANDFLASH_SUPPORT), 1)
subdir-ccflags-y += -DNANDFLASH_SUPPORT

#nandflash yaffs begin
ifeq ($(NANDFLASH_MTD_YAFFS_ENABLE), 1)
subdir-ccflags-y += -DNANDFLASH_MTD_YAFFS_ENABLE
endif

ifeq ($(NANDFLASH_MTD_YAFFS_PARTITION_ENABLE),1)
subdir-ccflags-y += -DNANDFLASH_MTD_YAFFS_PARTITION_ENABLE
ifneq ($(AOS_NAND_PART_NUM),)
subdir-ccflags-y += -DAOS_NAND_PART_NUM=$(AOS_NAND_PART_NUM)
endif
endif #NANDFLASH_MTD_YAFFS_PARTITION_ENABLE
#nandflash yaffs end

#nandflash dhara-cache begin
ifeq ($(NANDFLASH_MTD_ENABLE), 1)
subdir-ccflags-y += -DNANDFLASH_MTD_ENABLE
endif
#nandflash dhara-cache end

#nandflash dhara begin
ifeq ($(NANDFLASH_PARTITION_ENABLE),1)
subdir-ccflags-y += -DNANDFLASH_PARTITION_ENABLE
ifneq ($(AOS_NAND_PART_NUM),)
subdir-ccflags-y += -DAOS_NAND_PART_NUM=$(AOS_NAND_PART_NUM)
endif
endif #NANDFLASH_PARTITION_ENABLE
#nandflash dhara end

endif #NANDFLASH_SUPPORT

ifeq ($(call value_zero,$(NORFLASH_FS_SIZE)),n)
subdir-ccflags-y += -DNORFLASH_FS_ADDR=$(NORFLASH_FS_ADDR) -DNORFLASH_FS_SIZE=$(NORFLASH_FS_SIZE)
endif

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

ccflags-y += \
        -Iservices/fs/ \
        -Iservices/fs/plat_drv

subdir-ccflags-y += \
        -Iservices/fs/plat_drv/cache/
