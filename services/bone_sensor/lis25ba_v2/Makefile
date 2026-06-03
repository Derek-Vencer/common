
BONE_SENSOR_COMP_HEAP_SRC ?= 0

cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

ifeq ($(BONE_SENSOR_COMP_HEAP_SRC),1)
obj-y += ../../../utils/heap/
endif

ifeq ($(BONE_SENSOR_SW_RESAMPLE),1)
export NO_MULTIMEDIA_CODEC ?= 1
obj-y += ../../../multimedia/
endif

ifneq ($(PLAY_TDM_SLOT_IDX),)
ifneq ($(filter-out 0 1 2,$(PLAY_TDM_SLOT_IDX)),)
$(error invalid PLAY_TDM_SLOT_IDX=$(PLAY_TDM_SLOT_IDX), should be {0,1,2})
else
PLAY_TDM_SLOT_IDX ?= -DPLAY_TDM_SLOT_IDX
endif
endif

ccflags-y := \
	-Iservices/audioflinger \
	-Iservices/audio_dump/include \
	-Iutils/hwtimer_list \
	-Iutils/heap \

ifeq ($(BONE_SENSOR_SW_RESAMPLE),1)
ccflags-y += -DBONE_SENSOR_SW_RESAMPLE \
	-Imultimedia/inc/audio/process/resample/include \

endif

ifeq ($(BONE_SENSOR_PLAY_TEST),1)
ccflags-y += -DBONE_SENSOR_PLAY_TEST
endif
