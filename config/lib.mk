ifeq ($(USE_TRACE_ID), 1)
GEN_LIB_NAME := $(GEN_LIB_NAME)_log_compressed
endif

ifeq ($(BLE), 1)
GEN_LIB_NAME := $(GEN_LIB_NAME)_ble
endif

ifeq ($(GFPS_ENABLE), 1)
GEN_LIB_NAME := $(GEN_LIB_NAME)_gfps
endif

ifeq ($(SASS_ENABLE), 1)
GEN_LIB_NAME := $(GEN_LIB_NAME)_sass
endif

ifeq ($(GATT_OVER_BR_EDR), 1)
GEN_LIB_NAME := $(GEN_LIB_NAME)_gatt
endif

ifeq ($(ANC_APP), 1)
GEN_LIB_NAME := $(GEN_LIB_NAME)_anc
endif

ifeq ($(FREEMAN_ENABLED_STERO), 1)
GEN_LIB_NAME := $(GEN_LIB_NAME)_stero
endif
