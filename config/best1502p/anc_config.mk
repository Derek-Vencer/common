#### ANC DEFINE START ######
export ANC_APP ?= 1

#### ANC CONFIG ######
export ANC_FB_CHECK         ?= 0
export ANC_FF_CHECK         ?= 0
export ANC_TT_CHECK         ?= 0
export ANC_FF_ENABLED	    ?= 1
export ANC_FB_ENABLED	    ?= 1
export AUDIO_ANC_FB_MC      ?= 0
export AUDIO_ANC_FB_MC_HW   ?= 1
export AUDIO_ANC_FB_ADJ_MC  ?= 0
export AUDIO_SECTION_SUPPT  ?= 1
export AUDIO_ANC_SPKCALIB_HW ?= 0
export AUDIO_ANC_FIR_HW     ?= 0
export AUDIO_ANC_TT_HW      ?= 0
##### ANC DEFINE END ######

export AUDIO_ADJ_EQ_REV ?= 0
export AUDIO_ADAPTIVE_IIR_EQ  ?= 0
export AUDIO_ADAPTIVE_FIR_EQ  ?= 0

export PSAP_APP  ?= 0

export AUDIO_HEARING_COMPSATN ?= 0
ifeq ($(AUDIO_HEARING_COMPSATN),1)
AUDIO_HW_LIMITER := 1
export PSAP_APP_ONLY_MUSIC := 1
endif

AUDIO_HW_LIMITER ?= 0
ifeq ($(AUDIO_HW_LIMITER),1)
export PSAP_APP := 1
endif

ifeq ($(AUDIO_ANC_TT_HW),1)
export AUDIO_ANC_FB_MC_HW := 1
endif

export AUDIO_OUTPUT_DAC2 ?= 0

APP_ANC_TEST ?= 1
ifeq ($(APP_ANC_TEST),1)
export TOTA_v2 := 1
endif

ifeq ($(ANC_ASSIST_UNUSED_ON_PHONE_CALL),1)
KBUILD_CPPFLAGS += -DANC_ASSIST_UNUSED_ON_PHONE_CALL
endif

ifeq ($(ANC_APP),1)
KBUILD_CPPFLAGS += \
    -DANC_APP \
    -D__BT_ANC_KEY__ \
    -D__APP_KEY_FN_STYLE_A__
endif
