cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

mmlib_obj :=

ifneq ($(wildcard $(cur_dir)/algorithm/fft/src/Makefile),)
mmlib_obj += algorithm/fft/src/
endif

ifneq ($(wildcard $(cur_dir)/audio/process/adp/src/Makefile),)
mmlib_obj += audio/process/adp/src/
endif

ifneq ($(wildcard $(cur_dir)/audio/process/anc/src/Makefile),)
mmlib_obj += audio/process/anc/src/
endif

ifneq ($(wildcard $(cur_dir)/audio/process/psap/src/Makefile),)
mmlib_obj += audio/process/psap/src/
endif

ifneq ($(wildcard $(cur_dir)/audio/process/fir2iir/src/Makefile),)
mmlib_obj += audio/process/fir2iir/src/
endif

ifneq ($(wildcard $(cur_dir)/audio/process/common/src/Makefile),)
mmlib_obj += audio/process/common/src/
endif

ifneq ($(wildcard $(cur_dir)/audio/process/drc/src/Makefile),)
mmlib_obj += audio/process/drc/src/
endif

ifneq ($(wildcard $(cur_dir)/audio/process/filters/src/Makefile),)
mmlib_obj += audio/process/filters/src/
endif

ifneq ($(wildcard $(cur_dir)/audio/process/limiter/src/Makefile),)
mmlib_obj += audio/process/limiter/src/
endif

ifneq ($(wildcard $(cur_dir)/audio/process/resample/src/Makefile),)
mmlib_obj += audio/process/resample/src/
endif

ifneq ($(wildcard $(cur_dir)/audio/process/integer_resampling/src/Makefile),)
mmlib_obj += audio/process/integer_resampling/src/
endif

ifneq ($(wildcard $(cur_dir)/audio/process/bes_fir_filter/src/Makefile),)
mmlib_obj += audio/process/bes_fir_filter/src/
endif

ifneq ($(wildcard $(cur_dir)/audio/process/floatlimiter/src/Makefile),)
mmlib_obj += audio/process/floatlimiter/src/
endif

ifneq ($(wildcard $(cur_dir)/audio/process/adj_mc/src/Makefile),)
mmlib_obj += audio/process/adj_mc/src/
endif

ifneq ($(wildcard $(cur_dir)/audio/process/reverb/src/Makefile),)
mmlib_obj += audio/process/reverb/src/
endif

ifneq ($(wildcard $(cur_dir)/audio/process/sidetone/src/Makefile),)
mmlib_obj += audio/process/sidetone/src/
endif

ifneq ($(wildcard $(cur_dir)/audio/process/sbcplc/src/Makefile),)
mmlib_obj += audio/process/sbcplc/src/
endif

ifneq ($(wildcard $(cur_dir)/audio/process/dynamic_boost/src/Makefile),)
mmlib_obj += audio/process/dynamic_boost/src/
endif

ifneq ($(wildcard $(cur_dir)/audio/process/dynamic_eq/src/Makefile),)
mmlib_obj += audio/process/dynamic_eq/src/
endif

ifneq ($(wildcard $(cur_dir)/audio/process/adaptive_volume/src/Makefile),)
mmlib_obj += audio/process/adaptive_volume/src/
endif

ifneq ($(wildcard $(cur_dir)/audio/process/bass_enhancer/src/Makefile),)
mmlib_obj += audio/process/bass_enhancer/src/
endif


ifneq ($(wildcard $(cur_dir)/audio/process/virtual_surround_process/src/Makefile),)
mmlib_obj += audio/process/virtual_surround_process/src/
endif

ifneq ($(wildcard $(cur_dir)/audio/process/musicplc/Makefile),)
mmlib_obj += audio/process/musicplc/
endif

ifneq ($(wildcard $(cur_dir)/audio/process/head_track/Makefile),)
mmlib_obj += audio/process/head_track/
endif

export MULTIMEDIA_DEBUG ?=1
ifeq ($(MULTIMEDIA_DEBUG),1)
KBUILD_CPPFLAGS += -DMULTIMEDIA_DEBUG
else
KBUILD_CPPFLAGS += -DNDEBUG
endif

ifeq ($(FM_ENABLE),1)
ifneq ($(wildcard $(cur_dir)/fm/src/Makefile),)
mmlib_obj += fm/
endif
endif

ifneq ($(NO_MULTIMEDIA_CODEC),1)
mmlib_obj += ../multimedia/libs/
endif

#mmlib_obj += rbcodec/src/

ifeq ($(SPEECH_LIB),1)
ifneq ($(SPEECH_ARMCC_LIB),1)
ifneq ($(wildcard $(cur_dir)/speech/src/Makefile),)
mmlib_obj += speech/src/
endif
endif
endif

GEN_LIB_NAME := $(CHIP)_libmultimedia

ifeq ($(ROM_UTILS_ON),1)
GEN_LIB_NAME := $(GEN_LIB_NAME)_romaac
endif

ifeq ($(A2DP_CP_ACCEL),1)
GEN_LIB_NAME := $(GEN_LIB_NAME)_cp
endif

ifeq ($(LC3PLUS_ENABLE),1)
GEN_LIB_NAME := $(GEN_LIB_NAME)_lc3plus
endif

ifeq ($(AUDIO_ANC_FIR_HW),1)
GEN_LIB_NAME := $(GEN_LIB_NAME)_fir_anc
endif

ifeq ($(AUDIO_SECTION_ENABLE), 1)
GEN_LIB_NAME := $(GEN_LIB_NAME)_eq
endif

ifeq ($(CHIP_SUBSYS),sensor_hub)
GEN_LIB_NAME := $(GEN_LIB_NAME)_$(CHIP_SUBSYS)
endif

ifeq ($(HEAD_TRACK_ENABLE), 1)
GEN_LIB_NAME := $(GEN_LIB_NAME)_ht
endif

root_dir := $(abspath $(dir $(realpath $(firstword $(MAKEFILE_LIST))))/..)
sinclude $(root_dir)/config/lib.mk
$(GEN_LIB_NAME)-y := $(mmlib_obj)

rel_src_obj :=
rel_src_obj += inc/audio/process/anc/cfg/
rel_src_obj += inc/audio/process/filters/cfg/
rel_src_obj += inc/audio/process/resample/coef/
rel_src_obj += speech/armcc/

obj-y := $(GEN_LIB_NAME).a $(rel_src_obj)

ifeq ($(SPEECH_ARMCC_LIB),1)
ifeq ($(FREEMAN_ENABLED_STERO),1)
obj-y += lib_speech_${CPU}_stero.a
else
obj-y += lib_speech_${CPU}.a
endif
endif

subdir-ccflags-y += \
	-Iutils/heap \
	-Iinclude/cmsis_dsp \
	$(NN_INCLUDE) \
	-Imultimedia/inc/audio/process/filters/include \
	-I$(APPS_DIR)/common \
	-Imultimedia/inc/

ifneq ($(LTO_CFLAGS),)
KBUILD_CFLAGS := $(filter-out $(LTO_CFLAGS),$(KBUILD_CFLAGS))
KBUILD_CPPFLAGS := $(filter-out $(LTO_CPPFLAGS),$(KBUILD_CPPFLAGS))
endif
