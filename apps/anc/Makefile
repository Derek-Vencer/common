  
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)src/*.S))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)src/*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)src/*.cpp))

ifeq ($(ANC_APP),1)
CFLAGS_app_anc.o += -DANC_APP
endif

ifeq ($(ANC_FF_ENABLED),1)
CFLAGS_app_anc.o += -DANC_FF_ENABLED
CFLAGS_app_anc_fade.o += -DANC_FF_ENABLED
endif

ifeq ($(ANC_FB_ENABLED),1)
CFLAGS_app_anc.o += -DANC_FB_ENABLED
CFLAGS_app_anc_fade.o += -DANC_FB_ENABLED
endif

ifeq ($(PSAP_APP),1)
CFLAGS_app_anc.o += -DPSAP_APP
endif

ifeq ($(AUDIO_RESAMPLE),1)
CFLAGS_app_anc.o += -D__AUDIO_RESAMPLE__
endif
ifeq ($(SW_PLAYBACK_RESAMPLE),1)
CFLAGS_app_anc.o += -DSW_PLAYBACK_RESAMPLE
endif
ifeq ($(SW_CAPTURE_RESAMPLE),1)
CFLAGS_app_anc.o += -DSW_CAPTURE_RESAMPLE
endif
ifeq ($(AUDIO_SECTION_SUPPT),1)
CFLAGS_app_anc.o += -D__AUDIO_SECTION_SUPPT__
endif

ifeq ($(ANC_CALIB_WITH_GAIN),1)
CFLAGS_app_anc.o += -DANC_CALIB_WITH_GAIN
endif

ifneq ($(filter 1, $(PC_CMD_UART) $(TOTA_EQ_TUNING) $(USB_EQ_TUNING)),)
CFLAGS_app_anc.o += -DAUDIO_EQ_TUNING
endif

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)


subdir-ccflags-y += \
	-Iservices/overlay \
	-Iservices/nvrecord \
	-Iservices/resources \
	-Imultimedia/inc/audio/process/resample/include \
	-Imultimedia/inc/audio/process/integer_resampling/include \
    -Imultimedia/inc/audio/process/anc/include \
	-Imultimedia/inc/audio/process/psap/include \
	-Imultimedia/inc \
    -Imultimedia/inc/speech/inc \
	-Iplatform/drivers/uarthci \
	-Iplatform/drivers/ana \
	-Iplatform/drivers/bt \
	-Iutils/cqueue \
	-Iservices/audio_bt \
	-Iservices/audioflinger \
	-Iutils/lockcqueue \
	-Iutils/intersyshci \
	-Iapps/anc/inc \
	-Iapps/key \
	-Iapps/main \
	-Iapps/common \
	-Iapps/audioplayers \
	-Iapps/factory \
	-Iapps/voice_detector \
	-Iapps/anc/src/assist \
	-Iservices/anc/inc \
    -Iservices/nv_section/aud_section \
    -Iservices/nv_section/include   \
    -Iutils/hwtimer_list \
	-Iservices/tota \
	$(BLUETOOTH_ADAPTER_INCLUDES) \
	$(BLE_STACK_INCLUDES) \
	$(BT_SERVICE_UX_INCLUDES) \
	-Iinclude/cmsis_dsp \
	-Iservices/audio_dump/include/ \
	-Iservices/audio_manager \
	-Iservices/audio_manager/audio_policy/bredr_policy/inc \
	-Iapps/earbuds/conn