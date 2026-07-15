
cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

# crypt_lib-y := library/
obj-y := library/

ifneq ($(MBEDTLS_CONFIG_FILE),)
subdir-ccflags-y += -DMBEDTLS_CONFIG_FILE='"$(MBEDTLS_CONFIG_FILE)"'
else
subdir-ccflags-y += -DMBEDTLS_CONFIG_FILE='<config-rsa.h>'
endif

# obj-y := crypt_lib.o
