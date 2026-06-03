obj-y += inode/ mount/ driver/ conf/ fs/
obj-y += fatfs/ littlefs/ vfs/ plat_drv/ nofs/
obj-y += partition/

ifeq ($(CONFIG_FS_YAFFS), 1)
obj-y += yaffs/
subdir-ccflags-y += -DCONFIG_FS_YAFFS
endif

ifeq ($(OPENAMP_ENABLE), 1)
obj-y += rpmsgfs/
subdir-ccflags-y += -Iinclude/open-amp
ifeq ($(RPMSGFS_CLIENT), 1)
subdir-ccflags-y += -DBES_AOS_RPMSGFS_CLIENT
endif

ifeq ($(RPMSGFS_SERVER), 1)
subdir-ccflags-y += -DBES_AOS_RPMSGFS_SERVER
endif
endif

ifeq ($(AOS_FS_POSIX), 1)
subdir-ccflags-y += -DAOS_FS_POSIX
endif

ccflags-y += \
        -Iservices/fs \
        -Iservices/fs/public/include
