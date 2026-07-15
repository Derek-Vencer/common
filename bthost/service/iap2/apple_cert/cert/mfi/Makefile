# # obj-c :=
# # obj-cpp :=

# # ifneq ($(MFI_I2C_PORT),)
# # DEFS += -DMFI_I2C_PORT=$(MFI_I2C_PORT)
# # endif

# # DEFS += -DMFI_SERVER_IP=\"121.62.63.73\"
# # DEFS += -DMFI_SERVER_PORT=10636

# # INCLUDES += -I$(AIRPLAY_MFI)
# # INCLUDES += -I$(AIRPLAY_MFI)/i2c
# # INCLUDES += -I$(AIRPLAY_PLATFORM)

# # #SRCS += MFiPlatformRemoteMsg.c
# # #SRCS += stream_sock.c

# # SRCS += i2c/bes_i2c_dev.c
# # SRCS += i2c/mfi_dev.c


# # obj-c += $(SRCS)
# # obj-y := $(obj-c:.c=.o)
# # obj-y += $(obj-cpp:.cpp=.o)

# # ccflags-y += $(DEFS) $(INCLUDES)



cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c := 

obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))

obj-y := $(obj_c:.c=.o)

subdir-ccflags-y += \
    -Iplatform/hal/
