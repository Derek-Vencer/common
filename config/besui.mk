#######################################################################
#user config start

export EVB_DEMO_EN ?= 0
ifeq ($(EVB_DEMO_EN),1)
KBUILD_CPPFLAGS += -DEVB_DEMO_EN
export BOX_COMM_UNUSE_EN  				?= 1
endif

export CALIB_SLOW_TIMER  				?= 1
export MUTE_FRAME_DETECT 				?= 1

#----------------------------------------------------------------------
#besui app connect use spp/ble
export BESUI_APP_EN 					?= 0
ifeq ($(BESUI_APP_EN),1)
KBUILD_CPPFLAGS += -DBESUI_APP_EN
endif

export AAC_BECO_FFT ?= 0
ifeq ($(AAC_BECO_FFT),1)
export BECO := 1
KBUILD_CPPFLAGS += -DBECO
KBUILD_CPPFLAGS += -DAAC_BECO_FFT
endif

export AUDIO_STREAMING_DETECT_EN ?= 1
ifeq ($(AUDIO_STREAMING_DETECT_EN),1)
KBUILD_CPPFLAGS += -DAUDIO_STREAMING_DETECT_EN
endif

export CHARGE_SHIPMODE_NORST_CURRENT := 0
ifeq ($(CHARGE_SHIPMODE_NORST_CURRENT),1)
KBUILD_CPPFLAGS += -DCHARGE_SHIPMODE_NORST_CURRENT
endif

ifeq ($(AUDIO_STREAMING_DETECT_EN),1)
export USER_COMPUTER_STREAMING_DYNAMIC ?= 1
ifeq ($(USER_COMPUTER_STREAMING_DYNAMIC),1)
KBUILD_CPPFLAGS += -DUSER_COMPUTER_STREAMING_DYNAMIC
endif
endif

export RSSI_SWITCH_ROLE_EN 				:= 0
ifeq ($(RSSI_SWITCH_ROLE_EN),1)
KBUILD_CPPFLAGS += -DRSSI_SWITCH_ROLE_EN
endif

export BATTERY_SWITCH_ROLE_EN 			?= 0
ifeq ($(BATTERY_SWITCH_ROLE_EN),1)
KBUILD_CPPFLAGS += -DBATTERY_SWITCH_ROLE_EN
endif

ifeq ($(HW_DAC_IIR_EQ_PROCESS),1)
KBUILD_CPPFLAGS += -D__HW_DAC_IIR_EQ_PROCESS__
endif


#----------------------------------------------------------------------------------
#----------------------------------------------------------------------------------
export EQ_SET_CUSTOMER_EN 				?= 0
ifeq ($(BESUI_APP_EN),1)
export EQ_SET_CUSTOMER_EN 				:= 1
endif
ifeq ($(EQ_SET_CUSTOMER_EN),1)
KBUILD_CPPFLAGS += -DEQ_SET_CUSTOMER_EN
endif

export TWS_SPEECH_ROLE_SWITCH 			?= 0
ifeq ($(TWS_SPEECH_ROLE_SWITCH),1)
KBUILD_CPPFLAGS += -DTWS_SPEECH_ROLE_SWITCH
endif

#----------------------------------------------------------------------------------
# project function define start
#----------------------------------------------------------------------------------
export BES_TWSPRO_EN        			?= 0
export BES_NOTWS_EN        				?= 0

export OTA_BOOT_COPY_EN     			?= 0
export CRASH_FAST_RST_EN    			?= 0
export BESSPA_ONOFF_EN      			?= 0
export ALGO_INFO_SYNC_EN    			?= 1
export OTA_BOOT_SYNC_EN     			?= 0
export BESUI_COMM_EN        			?= 1
export USER_SPEECH_DUMP_EN  			?= 0
export USER_ANC_DUMP_EN     			?= 0
export UUID_TOTA_USE_OTA_EN 			?= 1
export USER_APP_BLE_DIS_EN  			?= 1
export USER_TOTA_SPP_SYNC_KEY_EN 		?= 1

export BESUI_STEREO_EN       			?= 0
export USER_QUICK_SWITCH_EN  			?= 0

ifeq ($(BES_TWSPRO_EN),1)
KBUILD_CPPFLAGS += -DBES_TWSPRO_EN
export CRASH_FAST_RST_EN        		:= 0
export A2DP_VIRTUAL_SURROUND            := 1
export BESSPA_ONOFF_EN                  := 1
export CRASH_REBOOT        				:= 1
export OTA_BOOT_COPY_EN         		:= 1
export EQ_CUSTOM_APP_EN         		:= 1
export WEAR_DETECT_PROMPT_EN    		:= 0
export BESUI_PROMPT_ISSUE_EN 			:= 1
export AUDIO_OUTPUT_SW_GAIN     		:= 1
export AUDIO_OUTPUT_SW_GAIN_BEFORE_DRC	:= 1

SPEECH_TX_2MIC_NS8  					:= 1
SPEECH_TX_EQ        					:= 1
SPEECH_TX_DC_FILTER 					:= 1
SPEECH_RX_NS2FLOAT  					:= 1
SPEECH_RX_EQ        					:= 0
endif


ifeq ($(BES_NOTWS_EN),1)
KBUILD_CPPFLAGS += -DBES_NOTWS_EN
export CRASH_FAST_RST_EN        		:= 0
export A2DP_VIRTUAL_SURROUND            := 1
export BESSPA_ONOFF_EN                  := 1
export CRASH_REBOOT        				:= 1
export OTA_BOOT_COPY_EN         		:= 1
export EQ_CUSTOM_APP_EN         		:= 1
export WEAR_DETECT_PROMPT_EN    		:= 0
export AUDIO_OUTPUT_SW_GAIN     		:= 1
export AUDIO_OUTPUT_SW_GAIN_BEFORE_DRC	:= 1

SPEECH_TX_2MIC_NS8  					:= 1
SPEECH_TX_EQ        					:= 1
SPEECH_TX_DC_FILTER 					:= 1
SPEECH_RX_NS2FLOAT  					:= 1
SPEECH_RX_EQ        					:= 0
endif

#----------------------------------------------------------------------------------
# project function define end
#----------------------------------------------------------------------------------


#----------------------------------------------------------------------------------
# stereo ui start
#----------------------------------------------------------------------------------
ifeq ($(BESUI_STEREO_EN),1)
KBUILD_CPPFLAGS += -DBESUI_STEREO_EN

# FACTORY_MODE := 0
# ENGINEER_MODE := 0

export STEREO_HALL_EN					?= 1
ifeq ($(STEREO_HALL_EN),1)
KBUILD_CPPFLAGS += -DSTEREO_HALL_EN
endif

export DUT_TEST_EN 						?= 0
ifeq ($(DUT_TEST_EN),1)
KBUILD_CPPFLAGS += -DDUT_TEST_EN
endif

export DC_DETECT_EN 					?= 0
ifeq ($(DC_DETECT_EN),1)
KBUILD_CPPFLAGS += -DDC_DETECT_EN
endif

export AUX_DETECT_EN 					?= 0
ifeq ($(AUX_DETECT_EN),1)
KBUILD_CPPFLAGS += -DAUX_DETECT_EN
endif

endif
#----------------------------------------------------------------------------------
# stereo ui end
#----------------------------------------------------------------------------------


ifeq ($(USER_QUICK_SWITCH_EN),1)
KBUILD_CPPFLAGS += -DUSER_QUICK_SWITCH_EN
endif

ifeq ($(OTA_BOOT_COPY_EN),1)
KBUILD_CPPFLAGS += -DOTA_BOOT_COPY_EN
endif

ifeq ($(CRASH_FAST_RST_EN),1)
KBUILD_CPPFLAGS += -DCRASH_FAST_RST_EN
endif

ifeq ($(CAP_LR_SET_EN),1)
KBUILD_CPPFLAGS += -DCAP_LR_SET_EN
endif

ifeq ($(BESSPA_ONOFF_EN),1)
KBUILD_CPPFLAGS += -DBESSPA_ONOFF_EN
endif

ifeq ($(GFPS_ENABLE),1)
export BESUI_GFPS_ID_EN 				?= 1
ifeq ($(BESUI_GFPS_ID_EN),1)
KBUILD_CPPFLAGS += -DBESUI_GFPS_ID_EN
endif
export GFPS_FIND_VOICE_LOOP_EN 			?= 1
ifeq ($(GFPS_FIND_VOICE_LOOP_EN),1)
KBUILD_CPPFLAGS += -DGFPS_FIND_VOICE_LOOP_EN
endif
endif

export PAIRMODE_AUTO_POWEROFF_EN 		?= 1
ifeq ($(PAIRMODE_AUTO_POWEROFF_EN),1)
KBUILD_CPPFLAGS += -DPAIRMODE_AUTO_POWEROFF_EN
endif

ifeq ($(ALGO_INFO_SYNC_EN),1)
KBUILD_CPPFLAGS += -DALGO_INFO_SYNC_EN
endif

ifeq ($(OTA_BOOT_SYNC_EN),1)
KBUILD_CPPFLAGS += -DOTA_BOOT_SYNC_EN
endif

export USE_ALGO_EN 						?= 1
ifeq ($(USE_ALGO_EN),1)
KBUILD_CPPFLAGS += -DUSE_ALGO_EN
endif

ifeq ($(BESUI_COMM_EN),1)
KBUILD_CPPFLAGS += -DBESUI_COMM_EN
endif

ifeq ($(CAPSENSOR_ENABLE),1)
export CAPSENSOR_FAC_CALCULATE  		?= 1

KBUILD_CPPFLAGS += -DCAP_TRACE_10S_EN

ifeq ($(CAPSENSOR_WEAR),1)
KBUILD_CFLAGS += -DCAPSENSOR_WEAR
endif

endif

ifeq ($(CAPSENSOR_FAC_CALCULATE),1)
export BESUI_CAPSENSOR_FACTORY_EN 		?= 1
ifeq ($(BESUI_CAPSENSOR_FACTORY_EN),1)
KBUILD_CPPFLAGS += -DBESUI_CAPSENSOR_FACTORY_EN
endif
endif

# ifeq ($(BT_DEVICE_NUM),2)
export APP_MULTIPOINT_ONOFF_EN 			?= 1
ifeq ($(APP_MULTIPOINT_ONOFF_EN),1)
KBUILD_CPPFLAGS += -DAPP_MULTIPOINT_ONOFF_EN
endif
# endif

ifeq ($(UUID_TOTA_USE_OTA_EN),1)
KBUILD_CPPFLAGS += -DUUID_TOTA_USE_OTA_EN
export IS_BLE_TOTA_ENABLED := 0
endif

ifeq ($(USER_SPEECH_DUMP_EN),1)
KBUILD_CPPFLAGS += -DUSER_SPEECH_DUMP_EN
endif

ifeq ($(USER_ANC_DUMP_EN),1)
KBUILD_CPPFLAGS += -DUSER_ANC_DUMP_EN
endif

ifeq ($(USER_APP_BLE_DIS_EN),1)
KBUILD_CPPFLAGS += -DUSER_APP_BLE_DIS_EN
endif

ifeq ($(USER_TOTA_SPP_SYNC_KEY_EN),1)
KBUILD_CPPFLAGS += -DUSER_TOTA_SPP_SYNC_KEY_EN
endif

ifeq ($(USER_LED_BREATH_EN),1)
KBUILD_CPPFLAGS += -DUSER_LED_BREATH_EN
endif

ifeq ($(USER_LED_CASE_OPEN_EN),1)
KBUILD_CPPFLAGS += -DUSER_LED_CASE_OPEN_EN
endif

ifeq ($(WEAR_DETECT_PROMPT_EN),1)
KBUILD_CPPFLAGS += -DWEAR_DETECT_PROMPT_EN
endif

ifeq ($(EQ_CUSTOM_APP_EN),1)
KBUILD_CPPFLAGS += -DEQ_CUSTOM_APP_EN

export EQBAND_NUM       				?= 7
KBUILD_CPPFLAGS += -DEQBAND_NUM=$(EQBAND_NUM)
export EQBAND_RANGE_DB  				?= 10
KBUILD_CPPFLAGS += -DEQBAND_RANGE_DB=$(EQBAND_RANGE_DB)
endif

export USER_DAC_REDUCE_EN 				?= 0
ifeq ($(USER_DAC_REDUCE_EN),1)
KBUILD_CPPFLAGS += -DUSER_DAC_REDUCE_EN
export DAC_REDUCE_DB	 				?= 0
KBUILD_CPPFLAGS += -DDAC_REDUCE_DB=$(DAC_REDUCE_DB)
endif

export USER_BURN_ADDR_PAIR_EN 			?= 0
ifeq ($(USER_BURN_ADDR_PAIR_EN),1)
KBUILD_CPPFLAGS += -DUSER_BURN_ADDR_PAIR_EN
export POWER_ON_ENTER_TWS_PAIRING_ENABLED := 1
endif

ifeq ($(USER_FACTORY_TRACE_RX_EN),1)
KBUILD_CPPFLAGS += -DUSER_FACTORY_TRACE_RX_EN
export APP_TRACE_RX_ENABLE 				:= 1 
export APP_RX_API_ENABLE   				:= 1
endif

export USER_OTA_FIX_DEVNAME_EN  		:= 1
ifeq ($(USER_OTA_FIX_DEVNAME_EN),1)
KBUILD_CPPFLAGS += -DUSER_OTA_FIX_DEVNAME_EN
endif

ifeq ($(ALGO_DELAY_ONOFF_EN),1)
KBUILD_CPPFLAGS += -DALGO_DELAY_ONOFF_EN
endif

export TRIPLE_DUT_EN 					?= 0
ifeq ($(TRIPLE_DUT_EN),1)
KBUILD_CPPFLAGS += -DTRIPLE_DUT_EN
endif

export ANC_SPEAKTHRU_EN 				?= 0
ifeq ($(ANC_SPEAKTHRU_EN),1)
KBUILD_CPPFLAGS += -DANC_SPEAKTHRU_EN
endif

export CHARGE_FULL_POWEROFF_EN 			?= 0
ifeq ($(CHARGE_FULL_POWEROFF_EN),1)
KBUILD_CPPFLAGS += -DCHARGE_FULL_POWEROFF_EN
endif

export BESSPA_EQ_EN 					?= 0
ifeq ($(BESSPA_EQ_EN),1)
KBUILD_CPPFLAGS += -DBESSPA_EQ_EN
endif

ifeq ($(A2DP_LDAC_ON),1)
export BT_DONT_PLAY_MUTE_WHEN_A2DP_STUCK_PATCH ?= 0
endif

ifeq ($(BESUI_APP_EN),1)
export CODEC_TYPE_APP_EN 				?= 1
endif

ifeq ($(CODEC_TYPE_APP_EN),1)
KBUILD_CPPFLAGS += -DCODEC_TYPE_APP_EN
endif

export BESUI_CALL_WEAR_SWITCH			?= 0
ifeq ($(BESUI_CALL_WEAR_SWITCH),1)
KBUILD_CPPFLAGS += -DBESUI_CALL_WEAR_SWITCH
endif

export APP_SYNC_GAME_EN 				?= 0
ifeq ($(APP_SYNC_GAME_EN),1)
KBUILD_CPPFLAGS += -DAPP_SYNC_GAME_EN
endif

export APP_SYNC_VOLUME_EN 				?= 0
ifeq ($(APP_SYNC_VOLUME_EN),1)
KBUILD_CPPFLAGS += -DAPP_SYNC_VOLUME_EN
endif

export BESUI_PROMPT_ISSUE_EN 			?= 0
ifeq ($(BESUI_PROMPT_ISSUE_EN),1)
KBUILD_CPPFLAGS += -DBESUI_PROMPT_ISSUE_EN
endif

export APP_TOUCH_ONOFF_EN 				?= 0
ifeq ($(APP_TOUCH_ONOFF_EN),1)
KBUILD_CPPFLAGS += -DAPP_TOUCH_ONOFF_EN
endif

export USER_NOISE_ADAPTIVE_ANC_EN 		?= 0
ifeq ($(USER_NOISE_ADAPTIVE_ANC_EN),1)
KBUILD_CPPFLAGS += -DUSER_NOISE_ADAPTIVE_ANC_EN
endif

export USER_EXT_VOLTAGE_DET_EN 			?= 0
ifeq ($(USER_EXT_VOLTAGE_DET_EN),1)
KBUILD_CPPFLAGS += -DUSER_EXT_VOLTAGE_DET_EN
endif

export BTLE_NAME_FIXED_EN 				?= 0
ifeq ($(BTLE_NAME_FIXED_EN),1)
KBUILD_CPPFLAGS += -DBTLE_NAME_FIXED_EN
endif

export ANC_PARAM_OTA_EN 				?= 0
ifeq ($(ANC_PARAM_OTA_EN),1)
KBUILD_CPPFLAGS += -DANC_PARAM_OTA_EN
endif

export BESUI_FW_LOG_WN 					?= 0
ifeq ($(BESUI_FW_LOG_WN),1)
KBUILD_CPPFLAGS += -DBESUI_FW_LOG_WN
endif

export BESUI_PLAY_ANSWER_PROMPT 		?= 0
ifeq ($(BESUI_PLAY_ANSWER_PROMPT),1)
KBUILD_CPPFLAGS += -DBESUI_PLAY_ANSWER_PROMPT
endif


ifeq ($(BES_TWSPRO_EN),1)
export SOFTWARE_VERSION_INFO 			:= 1
else ifeq ($(BES_NOTWS_EN),1)
export SOFTWARE_VERSION_INFO 			:= 1
else
export SOFTWARE_VERSION_INFO 			:= 1
endif

KBUILD_CPPFLAGS += -DSOFTWARE_VERSION_INFO=$(SOFTWARE_VERSION_INFO)
define FUNC_VERSION
V0.$(shell expr $(1) / 10).$(shell expr $(1) % 10)
endef
export BESUI_VER_STR := $(call FUNC_VERSION,$(SOFTWARE_VERSION_INFO))
KBUILD_CPPFLAGS += -DBESUI_VER_STR=$(BESUI_VER_STR)

#user config end
#######################################################################


#----------------------------------------------------------------------------------
# TWS UI define start
#----------------------------------------------------------------------------------
export BESUI_TWS_EN 					?= 0
ifeq ($(BESUI_TWS_EN),1)
KBUILD_CPPFLAGS += -DBESUI_TWS_EN

export BESUI_LR_IODET_EN        		?= 1
export BESUI_BOX_PUTINOUT_EN    		?= 1
export BESUI_KEY_EN             		?= 1
export BESUI_BTMSG_EN           		?= 1
export BESUI_CHARGE_EN          		?= 1
export BESUI_1WIRE_EN           		?= 1
export USER_SPP_CMD_EN         		 	?= 0

export BOX_COMM_UNUSE_EN        		?= 0
ifeq ($(BOX_COMM_UNUSE_EN),1)
KBUILD_CPPFLAGS += -DBOX_COMM_UNUSE_EN
endif

KBUILD_CPPFLAGS += -DBESUI_GAME_EN

export BESUI_GAME_NV_EN         		?= 1
ifeq ($(BESUI_GAME_NV_EN),1)
KBUILD_CPPFLAGS += -DBESUI_GAME_NV_EN
endif

ifeq ($(USER_SPP_CMD_EN),1)
KBUILD_CPPFLAGS += -DUSER_SPP_CMD_EN
endif

ifeq ($(BESUI_NTC_EN),1)
KBUILD_CPPFLAGS += -DBESUI_NTC_EN
endif

ifeq ($(BESUI_BOX_PUTINOUT_EN),1)
KBUILD_CPPFLAGS += -DBESUI_BOX_PUTINOUT_EN
endif

ifeq ($(BESUI_KEY_EN),1)
KBUILD_CPPFLAGS += -DBESUI_KEY_EN
endif

ifeq ($(BESUI_LR_IODET_EN),1)
KBUILD_CPPFLAGS += -DBESUI_LR_IODET_EN
endif

ifeq ($(BESUI_BTMSG_EN),1)
KBUILD_CPPFLAGS += -DBESUI_BTMSG_EN
endif

ifeq ($(BESUI_CHARGE_EN),1)
KBUILD_CPPFLAGS += -DBESUI_CHARGE_EN
endif

ifeq ($(BESUI_1WIRE_EN),1)
KBUILD_CPPFLAGS += -DBESUI_1WIRE_EN
else
export POWER_ON_ENTER_TWS_PAIRING_ENABLED := 1
endif

export IBRT_RIGHT_MASTER 				:= 0
endif

#----------------------------------------------------------------------------------
# TWS UI define end
#----------------------------------------------------------------------------------