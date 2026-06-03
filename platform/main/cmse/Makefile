cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.S))

obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.S=.o)

ifeq ($(FLASH_SECURITY_REGISTER),1)
CFLAGS_tz_otp_handler_demo.o += -DFLASH_SECURITY_REGISTER
endif

ifneq ($(TZ_PROCESS_STACK_SLOTS2),)
CFLAGS_tz_context.o += -DTZ_PROCESS_STACK_SLOTS2=$(TZ_PROCESS_STACK_SLOTS2)
endif

ifneq ($(TZ_PROCESS_STACK_SIZE1),)
CFLAGS_tz_context.o += -DTZ_PROCESS_STACK_SIZE1=$(TZ_PROCESS_STACK_SIZE1)
endif

ifneq ($(TZ_PROCESS_STACK_SIZE2),)
CFLAGS_tz_context.o += -DTZ_PROCESS_STACK_SIZE2=$(TZ_PROCESS_STACK_SIZE2)
endif

ifeq ($(DLB_SEC_LOAD_DEMO),1)
KBUILD_CFLAGS += -DDLB_SEC_LOAD_DEMO
obj-y := ../../thirdparty/thirdparty_lib/lib/dlb_sec_boot.a
obj-y += tz_context.o tz_interface.o tz_trace.o tz_trace_ns.o tz_otp_handler_demo.o
endif

