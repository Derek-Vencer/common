cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_s := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.S))
obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))

obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

subdir-ccflags-y += \
	-Iservices/overlay \
	-Iservices/nvrecord \
	-Iservices/resources \
	-Iservices/multimedia/audio/process/resample/include \
	-Imultimedia/inc \
	-Imultimedia/inc/speech/inc \
	-Imultimedia/inc/audio/process/integer_resampling/include \
	-Imultimedia/inc/audio/process/anc/include \
	-Imultimedia/inc/audio/process/psap/include \
	-Iservices/multimedia/audio/process/integer_resampling/include \
    -Iservices/multimedia/audio/process/anc/include \
	-Iservices/multimedia/audio/process/psap/include \
    -Iservices/multimedia/speech/inc \
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
	-Iapps/earbuds/conn \
	-Iutils/heap \
	-Iapps/voice_assist/inc

ifeq ($(ENABLE_CALCU_CPU_FREQ_LOG),1)
CFLAGS_app_anc_assist.o += -DENABLE_CALCU_CPU_FREQ_LOG
endif

ifeq ($(VOICE_ASSIST_WD_ENABLED),1)
CFLAGS_app_anc_assist.o += -DVOICE_ASSIST_WD_ENABLED
endif

ifeq ($(ANC_ASSIST_USE_INT_CODEC),1)
CFLAGS_app_anc_assist.o += -DANC_ASSIST_USE_INT_CODEC
endif

ifeq ($(ANC_ASSIST_PILOT_TONE_ALWAYS_ON),1)
CFLAGS_app_anc_assist.o += -DANC_ASSIST_PILOT_TONE_ALWAYS_ON
endif


ifeq ($(CHIP_HAS_ANC_HW_GAIN_SMOOTHING),1)
CFLAGS_anc_assist_anc.o += -DANC_HW_GAIN_SMOOTHING
endif

ifeq ($(ANC_ASSIST_PROCESS_THREAD),1)
CFLAGS_app_anc_assist.o += -DANC_ASSIST_PROCESS_THREAD
CFLAGS_anc_assist_thread.o += -DANC_ASSIST_PROCESS_THREAD
endif

ifeq ($(ASSIST_FIR_ANC_OPEN_AGAIN_DETECT),1)
CFLAGS_app_anc_assist.o += -DASSIST_FIR_ANC_OPEN_AGAIN_DETECT
endif