cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y :=
subdir-ccflags-y :=

ifeq ($(BECO),1)
obj-y += beco/
endif

ifeq ($(NN_LIB),1)
obj-y += cmsis-nn/
endif

ifeq ($(ETHOSU_ARCH), u55)
obj-y += core-driver/ core-platform/ core-software/ tflite-micro/
subdir-ccflags-y += -Wno-double-promotion -Wno-unused-variable -Wno-unused-function -Wno-shadow -Wno-float-conversion
subdir-ccflags-y += -DETHOSU -DETHOSU55 -DETHOSU_ARCH=$(ETHOSU_ARCH)
endif

ifneq ($(ETHOSU_LOG_ENABLE),)
subdir-ccflags-y += -DETHOSU_LOG_ENABLE=$(ETHOSU_LOG_ENABLE)
endif

ifneq ($(ETHOSU_LOG_SEVERITY),)
subdir-ccflags-y += -DETHOSU_LOG_SEVERITY=$(ETHOSU_LOG_SEVERITY)
endif

ifneq ($(NN_DATA_RAM_LOC),)
subdir-ccflags-y += -DNN_DATA_RAM_LOC=$(NN_DATA_RAM_LOC)
endif

ifneq ($(NN_MODEL_RAM_LOC),)
subdir-ccflags-y += -DNN_MODEL_RAM_LOC=$(NN_MODEL_RAM_LOC)
endif

ifeq ($(NN_INCLUDES),)
export NN_INCLUDES := -Inn/cmsis-nn/Include \
                      -Iinclude/cmsis_dsp \
                      -Inn/beco/Include
endif