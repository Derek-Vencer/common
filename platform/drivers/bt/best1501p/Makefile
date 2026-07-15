cur_dir := $(dir $(lastword $(MAKEFILE_LIST)))

obj-y := $(patsubst $(cur_dir)%,%,$(wildcard $(cur_dir)*.c $(cur_dir)*.cpp $(cur_dir)*.S))
obj-y := $(obj-y:.c=.o)
obj-y := $(obj-y:.cpp=.o)
obj-y := $(obj-y:.S=.o)

BTC_NO_UART_BRIDGE?= 0
ifeq ($(BTC_NO_UART_BRIDGE),1)
obj-y := $(filter-out bt_drv_uart_bridge_intsys.o,$(obj-y))
else
CFLAGS_bt_drv_uart_bridge_intsys.o += $(BT_DRV_UART_BRIDGE_FLAGS)
endif

CFLAGS_bt_drv.o += $(BT_DRV_FLAGS)

CFLAGS_bt_drv_patch.o += $(BT_DRV_PATCH_FLAGS)

CFLAGS_bt_drv_config.o += $(BT_DRV_CONFIG_FLAGS)

TX_IQ_CAL ?= 0
ifeq ($(TX_IQ_CAL),1)
subdir-ccflags-y += -DTX_IQ_CAL
endif

TX_PULLING_CAL ?= 0
ifeq ($(TX_PULLING_CAL),1)
subdir-ccflags-y += -DTX_PULLING_CAL
endif

RX_IQ_CAL ?= 0
ifeq ($(RX_IQ_CAL),1)
subdir-ccflags-y += -DRX_IQ_CAL
endif

subdir-ccflags-y += -Iutils/crash_catcher/include
subdir-ccflags-y += -Iservices/nv_section/userdata_section
ccflags-y += $(BLUETOOTH_ADAPTER_INCLUDES)


BLE_EXT_ADV_TX_PWR_INDEPEND ?= 0
ifeq ($(BLE_EXT_ADV_TX_PWR_INDEPEND),1)
ccflags-y += -DBLE_EXT_ADV_TX_PWR_INDEPEND
endif


BT_WATCH_PATCH ?= 0
ifeq ($(BT_WATCH_PATCH),1)
ccflags-y += -DBT_WATCH_PATCH
endif

ifeq ($(BT_DSB_3M_TX),1)
ccflags-y += -D__BT_DSB_3M_TX__
endif

ifeq ($(BT_RF_PREFER), 3M)
ifeq ($(NEW_SWAGC_MODE),1)
ifeq ($(BT_SYSTEM_52M),0)
$(error BT SYSTEM 24M do not support $(BT_RF_PREFER))
endif
endif
endif