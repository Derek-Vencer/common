
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj_c := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))
src_obj := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

GEN_LIB_NAME := libcryption
root_dir := $(abspath $(dir $(realpath $(firstword $(MAKEFILE_LIST))))/..)
sinclude $(root_dir)/config/lib.mk
$(GEN_LIB_NAME)-y := $(src_obj)
obj-y += $(GEN_LIB_NAME).a

ccflags-y := \
    -Iservices/bt_profiles/inc/ \
    -Iservices/bt_if/inc/ \
    -Iservices/bt_app/ \
    -Iapps/common/ \
    -Iutils/cqueue
