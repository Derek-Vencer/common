cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)umm_malloc/*.c))

ifeq ($(INTERACTION),1)
obj_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)green/*.cpp))
endif

ifeq ($(INTERCONNECTION),1)
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)red/*.c))
obj_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)red/*.cpp))
endif

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

subdir-ccflags-y += -Iplatform/drivers/ana \
					-Iplatform/hal \
					-Iservices/bluedroid/adapter/src \
					-Iapps/key \
					-Iservices/audio_process \
					$(BLUETOOTH_ADAPTER_INCLUDES) \
					$(BT_SERVICE_UX_INCLUDES) \
					-Iservices/overlay \
					-Iservices/resources \
					-Iplatform/drivers/uarthci \
					-Iplatform/drivers/ana \
					-Iplatform/drivers/bt \
					-Iutils/cqueue \
					-Iutils/retention_ram \
					-Iservices/audioflinger \
					-Iutils/lockcqueue \
					-Iutils/intersyshci \
					-Iapps/main \
					-Iapps/common \
					-Iapps/audioplayers \
					-Iapps/factory \
					-Iservices/audio_bt \
					-Iservices/bridge/ \
					-Iservices/interconnection/red \
					-Iservices/interconnection/green \
					-Iservices/interconnection/umm_malloc \
					-Iservices/ota \
					-Iservices/norflash_api \
					$(BES_MULTIMEDIA_INCLUDES) \
					-Imultimedia/speech/application/Pcm8kCvsd/include \
					-Imultimedia/speech/codec/codecs/adpcm \
					-Imultimedia/speech/process/lc_mmse_ns/include \
					-Imultimedia/speech/process/ns_and_ec/include \
					-Imultimedia/speech/process/speech_2mic_ns2_denoise/include \
					-Imultimedia/speech/process/dual_mic_denoise/include \
					-Imultimedia/speech/process/far_field_speech_enhancement/include \
					-Imultimedia/speech/process/plc/include \
					-Imultimedia/speech/process/common/include \
					-Iservices/nv_section/factory_section \
					-Iapps/battery   \
					-Ithirdparty/userapi \
					-Iutils/crc \
					-Iutils/sha256	\
					-Iutils/aes256 \
					-Iutils/heap \
					-Iservices/norflash_api \
					-Iservices/audio_manager

ifeq ($(VOICE_PROMPT),1)
ccflags-y += -DMEDIA_PLAYER_SUPPORT
endif
