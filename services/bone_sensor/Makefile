
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))

obj-y := $(obj_c:.c=.o)

ifeq ($(VPU_NAME),LIS25BA)
ifeq ($(SPEECH_BONE_SENSOR_I2S),1)
obj-y += lis25ba/
else ifeq ($(SPEECH_BONE_SENSOR_TDM),1)
obj-y += lis25ba_v2/
else
$(error Enable SPEECH_BONE_SENSOR_I2S or SPEECH_BONE_SENSOR_TDM)
endif
else
ifeq ($(VPU_CFG_ON_SENSOR_HUB),1)
CFLAGS_speech_bone_sensor.o += -DVPU_CFG_ON_SENSOR_HUB
else
obj-y += vpu_cfg/
endif
obj-y += vpu_stream/
CFLAGS_speech_bone_sensor.o += -DVPU_STREAM
endif

ifeq ($(SPEECH_BONE_SENSOR_I2S),1)
CFLAGS_speech_bone_sensor.o += -DSPEECH_BONE_SENSOR_I2S
else ifeq ($(SPEECH_BONE_SENSOR_TDM),1)
CFLAGS_speech_bone_sensor.o += -DSPEECH_BONE_SENSOR_TDM
else
$(error Enable SPEECH_BONE_SENSOR_I2S or SPEECH_BONE_SENSOR_TDM)
endif

ifeq ($(VPU_DEBUG_CODEC_I2S_SYNC),1)
CFLAGS_speech_bone_sensor.o += -DVPU_DEBUG_CODEC_I2S_SYNC
endif

ccflags-y := \
    -Iservices/audio_dump/include
