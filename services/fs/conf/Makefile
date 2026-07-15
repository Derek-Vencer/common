cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))
obj_c   := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c))
obj_cpp := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.cpp))
obj-y := $(obj_c:.c=.o) $(obj_s:.S=.o) $(obj_cpp:.cpp=.o)

ifeq ($(FS_MM_HEAP_CACHE_ALIGNED), 1)
subdir-ccflags-y += -DFS_MM_HEAP_CACHE_ALIGNED
endif

ifneq ($(FS_MM_HEAP_SIZE),)
subdir-ccflags-y += -DFS_MM_HEAP_SIZE=$(FS_MM_HEAP_SIZE)
endif

ccflags-y += \
        -Iservices/fs \
        -Iutils/heap \
