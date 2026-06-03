cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)/*/*.c $(cur_dir)/*/*.cpp))
obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
