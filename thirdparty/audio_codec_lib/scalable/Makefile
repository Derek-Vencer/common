
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

libscalable-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

LIB_BIN_DIR := lib/thirdparty/audio_codec/scalable
ifeq ($(A2DP_DECODER_CROSS_CORE_USE_M55),1)
obj-y := libscalable_24bit_20211210_CM55.a
else
obj-y := libscalable.a
# obj-y := libscalable_CM33.a
endif

ccflags-y :=
