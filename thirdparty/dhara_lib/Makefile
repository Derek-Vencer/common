cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

#dhara
obj_s += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)dhara/*.S))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)dhara/*.c))
obj_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)dhara/*.cpp))

#ecc
obj_s += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)ecc/*.S))
obj_c += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)ecc/*.c))
obj_cpp += $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)ecc/*.cpp))

src_obj := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

NFTL_LIB_NAME := libdhara

$(NFTL_LIB_NAME)-y := $(src_obj)

obj-y :=  $(NFTL_LIB_NAME).a
