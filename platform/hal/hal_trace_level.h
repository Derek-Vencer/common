#define ACTIVITY_MANAGER_TRACE_ENABLE
#define AI_VOICE_TRACE_ENABLE
#define ANC_SPP_TOOL_TRACE_ENABLE
#define ANC_TRACE_ENABLE
#define ANC_USB_TRACE_ENABLE
#define API_TRACE_ENABLE
#define APP_AI_TRACE_ENABLE
#define APP_RPC_TRACE_ENABLE
#define APP_SPP_TRACE_ENABLE
#define APP_TEST_TRACE_ENABLE
#define AUDIOFLINGER_TRACE_ENABLE
//#define AUDIOPLAYERS_TRACE_ENABLE
#define AUDIO_BT_TRACE_ENABLE
#define AUDIO_CODEC_LIB_TRACE_ENABLE
#define AUDIO_DUMP_TRACE_ENABLE
#define AUDIO_MANAGER_TRACE_ENABLE
#define AUDIO_PROCESS_TRACE_ENABLE
#define AUTO_TEST_TRACE_ENABLE
//#define BATTERY_TRACE_ENABLE
#define BECO_TRACE_ENABLE
#define BESAIR_PLATFORM_TRACE_ENABLE
#define BESAIR_TRACE_ENABLE
#define BINAURAL_RECORD_PROCESS_TRACE_ENABLE
//#define BLUEDROID_TRACE_ENABLE
//#define BONE_SENSOR_TRACE_ENABLE
//#define BTAPP_TRACE_ENABLE
#define BTUSBAUDIO_TRACE_ENABLE
//#define BT_SYNC_TRACE_ENABLE
#define CAPSENSOR_TRACE_ENABLE
#define CHIP_FUNCION_TEST_TRACE_ENABLE
#define CMD_TRACE_ENABLE
#define CMSIS_TRACE_ENABLE
#define CODEC_CALIB_TRACE_ENABLE
#define CODEC_VAD_TRACE_ENABLE
#define COMMON_TRACE_ENABLE
#define COMMUNICATION_TRACE_ENABLE
#define CP_SUBSYS_TRACE_ENABLE
#define CQUEUE_TRACE_ENABLE
#define DRIVERS_TRACE_ENABLE
#define DSP_M55_TRACE_ENABLE
#define EARBUDS_TRACE_ENABLE
#define EAUDIO_TRACE_ENABLE
#define ECOMM_TRACE_ENABLE
#define EPLAYER_TRACE_ENABLE
#define ESHELL_TRACE_ENABLE
#define EXAMPLE_TRACE_ENABLE
#define FACTORY_SUITE_TRACE_ENABLE
#define FACTORY_TRACE_ENABLE
#define FASTNV_TRACE_ENABLE
#define FPGA_IBRT_OTA_TRACE_ENABLE
#define FREERTOS_TRACE_ENABLE
#define FS_TRACE_ENABLE
#define GFPS_TRACE_ENABLE
//#define HAL_TRACE_ENABLE
#define HEADSET_TRACE_ENABLE
#define HEAD_TRACK_3DOF_TRACE_ENABLE
#define HEAD_TRACK_TRACE_ENABLE
#define HEAP_TRACE_ENABLE
#define HEARING_DETEC_TRACE_ENABLE
#define HSM_TRACE_ENABLE
#define HWTEST_TRACE_ENABLE
#define HWTIMER_LIST_TRACE_ENABLE
#define HW_DSP_TRACE_ENABLE
#define INTERCONNECTION_TRACE_ENABLE
#define INTERSYSHCI_TRACE_ENABLE
#define KARAOKE_TRACE_ENABLE
#define KEY_TRACE_ENABLE
#define KFIFO_TRACE_ENABLE
#define KNOWLES_UART_LIB_TRACE_ENABLE
#define LEA_PLAYER_TRACE_ENABLE
#define LIGHTNING_TEST_TRACE_ENABLE
#define LIST_TRACE_ENABLE
#define LITEOS_TRACE_ENABLE
#define LZMA_TRACE_ENABLE
#define MAIN_TRACE_ENABLE
#define MBEDTLS_TRACE_ENABLE
#define MCPP_TRACE_ENABLE
#define MEM_TEST_TRACE_ENABLE
#define MIC_TRACE_ENABLE
#define MINIMA_TEST_TRACE_ENABLE
#define NOISE_TRACKER_LIB_TRACE_ENABLE
#define NORFLASH_API_TRACE_ENABLE
#define NUTTX_TRACE_ENABLE
#define NV_SECTION_TRACE_ENABLE
#define OHOS_WALKIE_TALKIE_STREAM_TRACE_ENABLE
#define OSIF_TRACE_ENABLE
#define OTA_BOOT_REMAP_TRACE_ENABLE
#define OTA_BOOT_TRACE_ENABLE
#define OTA_TRACE_ENABLE
#define OVERLAY_TRACE_ENABLE
#define PRESSURE_TRACE_ENABLE
#define PROGRAMMER_EXT_TRACE_ENABLE
#define PROGRAMMER_TRACE_ENABLE
#define PSRAM_CALIB_TRACE_ENABLE
#define PWL_TRACE_ENABLE
#define RAMRUN_TEST_TRACE_ENABLE
#define RATE_TEST_DEMO_TRACE_ENABLE
#define RELIABILITY_TEST_TRACE_ENABLE
#define RETENTION_RAM_TRACE_ENABLE
#define ROM_TRACE_ENABLE
#define ROM_UTILS_TRACE_ENABLE
#define RPC_TRACE_ENABLE
#define RTX5_TRACE_ENABLE
#define RTX_TRACE_ENABLE
#define RT_THREAD_TRACE_ENABLE
#define SDIO_DOWNLOAD_TEST_TRACE_ENABLE
#define SDMMC_MSD_TRACE_ENABLE
#define SDMMC_TRACE_ENABLE
#define SECURE_BOOT_TRACE_ENABLE
#define SENSHUB_LIB_TRACE_ENABLE
#define SENSORHUB_TRACE_ENABLE
#define SENSOR_HUB_TRACE_ENABLE
#define SENSOR_TEST_TRACE_ENABLE
#define SIGNAL_GENERATOR_TRACE_ENABLE
#define SIMU_TEST_TRACE_ENABLE
#define SOUNDPLUS_LIB_TRACE_ENABLE
#define SOUND_TRACE_ENABLE
#define SPEECH_TEST_TRACE_ENABLE
#define SPP_TEST_TRACE_ENABLE
#define STREAM_MCPS_TRACE_ENABLE
#define THROUGH_PUT_TRACE_ENABLE
#define TILE_TRACE_ENABLE
#define TOTA_TRACE_ENABLE
#define TOTA_V2_TRACE_ENABLE
#define UARTHCI_TRACE_ENABLE
#define USBAUDIO_TRACE_ENABLE
#define USBHOSTSHELL_TRACE_ENABLE
#define USERAPI_TRACE_ENABLE
#define USER_SECURE_BOOT_TRACE_ENABLE
#define VOICEPATH_TRACE_ENABLE
#define VOICE_ASSIST_TRACE_ENABLE
#define VOICE_DETECTOR_TRACE_ENABLE
#define VOICE_DEV_TRACE_ENABLE
#define WALKIE_TALKIE_TRACE_ENABLE
#define XIAOMI_ENC_TRACE_ENABLE
#define FINDMY_TRACE_ENABLE
#define SW_IIR_TRACE_ENABLE

#if defined(BESUI_TWS_EN) || defined(BESUI_STEREO_EN)
#define BESUI_TRACE_ENABLE
#endif

#ifdef ACTIVITY_MANAGER_TRACE_ENABLE
#define ACTIVITY_MANAGER_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define ACTIVITY_MANAGER_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define ACTIVITY_MANAGER_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define ACTIVITY_MANAGER_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define ACTIVITY_MANAGER_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define ACTIVITY_MANAGER_TRACE(attr, str, ...)
#define ACTIVITY_MANAGER_TRACE_IMM(attr, str, ...)
#define ACTIVITY_MANAGER_DUMP8(str, buf, cnt)
#define ACTIVITY_MANAGER_DUMP16(str, buf, cnt)
#define ACTIVITY_MANAGER_DUMP32(str, buf, cnt)
#endif


#ifdef AI_VOICE_TRACE_ENABLE
#define AI_VOICE_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define AI_VOICE_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define AI_VOICE_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define AI_VOICE_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define AI_VOICE_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define AI_VOICE_TRACE(attr, str, ...)
#define AI_VOICE_TRACE_IMM(attr, str, ...)
#define AI_VOICE_DUMP8(str, buf, cnt)
#define AI_VOICE_DUMP16(str, buf, cnt)
#define AI_VOICE_DUMP32(str, buf, cnt)
#endif


#ifdef ANC_SPP_TOOL_TRACE_ENABLE
#define ANC_SPP_TOOL_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define ANC_SPP_TOOL_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define ANC_SPP_TOOL_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define ANC_SPP_TOOL_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define ANC_SPP_TOOL_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define ANC_SPP_TOOL_TRACE(attr, str, ...)
#define ANC_SPP_TOOL_TRACE_IMM(attr, str, ...)
#define ANC_SPP_TOOL_DUMP8(str, buf, cnt)
#define ANC_SPP_TOOL_DUMP16(str, buf, cnt)
#define ANC_SPP_TOOL_DUMP32(str, buf, cnt)
#endif


#ifdef ANC_TRACE_ENABLE
#define ANC_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define ANC_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define ANC_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define ANC_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define ANC_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define ANC_TRACE(attr, str, ...)
#define ANC_TRACE_IMM(attr, str, ...)
#define ANC_DUMP8(str, buf, cnt)
#define ANC_DUMP16(str, buf, cnt)
#define ANC_DUMP32(str, buf, cnt)
#endif


#ifdef ANC_USB_TRACE_ENABLE
#define ANC_USB_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define ANC_USB_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define ANC_USB_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define ANC_USB_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define ANC_USB_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define ANC_USB_TRACE(attr, str, ...)
#define ANC_USB_TRACE_IMM(attr, str, ...)
#define ANC_USB_DUMP8(str, buf, cnt)
#define ANC_USB_DUMP16(str, buf, cnt)
#define ANC_USB_DUMP32(str, buf, cnt)
#endif


#ifdef API_TRACE_ENABLE
#define API_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define API_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define API_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define API_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define API_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define API_TRACE(attr, str, ...)
#define API_TRACE_IMM(attr, str, ...)
#define API_DUMP8(str, buf, cnt)
#define API_DUMP16(str, buf, cnt)
#define API_DUMP32(str, buf, cnt)
#endif


#ifdef APP_AI_TRACE_ENABLE
#define APP_AI_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define APP_AI_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define APP_AI_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define APP_AI_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define APP_AI_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define APP_AI_TRACE(attr, str, ...)
#define APP_AI_TRACE_IMM(attr, str, ...)
#define APP_AI_DUMP8(str, buf, cnt)
#define APP_AI_DUMP16(str, buf, cnt)
#define APP_AI_DUMP32(str, buf, cnt)
#endif


#ifdef APP_RPC_TRACE_ENABLE
#define APP_RPC_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define APP_RPC_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define APP_RPC_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define APP_RPC_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define APP_RPC_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define APP_RPC_TRACE(attr, str, ...)
#define APP_RPC_TRACE_IMM(attr, str, ...)
#define APP_RPC_DUMP8(str, buf, cnt)
#define APP_RPC_DUMP16(str, buf, cnt)
#define APP_RPC_DUMP32(str, buf, cnt)
#endif


#ifdef APP_SPP_TRACE_ENABLE
#define APP_SPP_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define APP_SPP_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define APP_SPP_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define APP_SPP_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define APP_SPP_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define APP_SPP_TRACE(attr, str, ...)
#define APP_SPP_TRACE_IMM(attr, str, ...)
#define APP_SPP_DUMP8(str, buf, cnt)
#define APP_SPP_DUMP16(str, buf, cnt)
#define APP_SPP_DUMP32(str, buf, cnt)
#endif


#ifdef APP_TEST_TRACE_ENABLE
#define APP_TEST_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define APP_TEST_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define APP_TEST_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define APP_TEST_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define APP_TEST_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define APP_TEST_TRACE(attr, str, ...)
#define APP_TEST_TRACE_IMM(attr, str, ...)
#define APP_TEST_DUMP8(str, buf, cnt)
#define APP_TEST_DUMP16(str, buf, cnt)
#define APP_TEST_DUMP32(str, buf, cnt)
#endif


#ifdef AUDIOFLINGER_TRACE_ENABLE
#define AUDIOFLINGER_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define AUDIOFLINGER_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define AUDIOFLINGER_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define AUDIOFLINGER_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define AUDIOFLINGER_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define AUDIOFLINGER_TRACE(attr, str, ...)
#define AUDIOFLINGER_TRACE_IMM(attr, str, ...)
#define AUDIOFLINGER_DUMP8(str, buf, cnt)
#define AUDIOFLINGER_DUMP16(str, buf, cnt)
#define AUDIOFLINGER_DUMP32(str, buf, cnt)
#endif


#ifdef AUDIOPLAYERS_TRACE_ENABLE
#define AUDIOPLAYERS_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define AUDIOPLAYERS_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define AUDIOPLAYERS_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define AUDIOPLAYERS_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define AUDIOPLAYERS_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define AUDIOPLAYERS_TRACE(attr, str, ...)
#define AUDIOPLAYERS_TRACE_IMM(attr, str, ...)
#define AUDIOPLAYERS_DUMP8(str, buf, cnt)
#define AUDIOPLAYERS_DUMP16(str, buf, cnt)
#define AUDIOPLAYERS_DUMP32(str, buf, cnt)
#endif


#ifdef AUDIO_BT_TRACE_ENABLE
#define AUDIO_BT_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define AUDIO_BT_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define AUDIO_BT_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define AUDIO_BT_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define AUDIO_BT_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define AUDIO_BT_TRACE(attr, str, ...)
#define AUDIO_BT_TRACE_IMM(attr, str, ...)
#define AUDIO_BT_DUMP8(str, buf, cnt)
#define AUDIO_BT_DUMP16(str, buf, cnt)
#define AUDIO_BT_DUMP32(str, buf, cnt)
#endif


#ifdef AUDIO_CODEC_LIB_TRACE_ENABLE
#define AUDIO_CODEC_LIB_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define AUDIO_CODEC_LIB_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define AUDIO_CODEC_LIB_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define AUDIO_CODEC_LIB_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define AUDIO_CODEC_LIB_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define AUDIO_CODEC_LIB_TRACE(attr, str, ...)
#define AUDIO_CODEC_LIB_TRACE_IMM(attr, str, ...)
#define AUDIO_CODEC_LIB_DUMP8(str, buf, cnt)
#define AUDIO_CODEC_LIB_DUMP16(str, buf, cnt)
#define AUDIO_CODEC_LIB_DUMP32(str, buf, cnt)
#endif


#ifdef AUDIO_DUMP_TRACE_ENABLE
#define AUDIO_DUMP_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define AUDIO_DUMP_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define AUDIO_DUMP_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define AUDIO_DUMP_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define AUDIO_DUMP_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define AUDIO_DUMP_TRACE(attr, str, ...)
#define AUDIO_DUMP_TRACE_IMM(attr, str, ...)
#define AUDIO_DUMP_DUMP8(str, buf, cnt)
#define AUDIO_DUMP_DUMP16(str, buf, cnt)
#define AUDIO_DUMP_DUMP32(str, buf, cnt)
#endif


#ifdef AUDIO_MANAGER_TRACE_ENABLE
#define AUDIO_MANAGER_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define AUDIO_MANAGER_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define AUDIO_MANAGER_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define AUDIO_MANAGER_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define AUDIO_MANAGER_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define AUDIO_MANAGER_TRACE(attr, str, ...)
#define AUDIO_MANAGER_TRACE_IMM(attr, str, ...)
#define AUDIO_MANAGER_DUMP8(str, buf, cnt)
#define AUDIO_MANAGER_DUMP16(str, buf, cnt)
#define AUDIO_MANAGER_DUMP32(str, buf, cnt)
#endif


#ifdef AUDIO_PROCESS_TRACE_ENABLE
#define AUDIO_PROCESS_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define AUDIO_PROCESS_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define AUDIO_PROCESS_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define AUDIO_PROCESS_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define AUDIO_PROCESS_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define AUDIO_PROCESS_TRACE(attr, str, ...)
#define AUDIO_PROCESS_TRACE_IMM(attr, str, ...)
#define AUDIO_PROCESS_DUMP8(str, buf, cnt)
#define AUDIO_PROCESS_DUMP16(str, buf, cnt)
#define AUDIO_PROCESS_DUMP32(str, buf, cnt)
#endif


#ifdef AUTO_TEST_TRACE_ENABLE
#define AUTO_TEST_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define AUTO_TEST_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define AUTO_TEST_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define AUTO_TEST_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define AUTO_TEST_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define AUTO_TEST_TRACE(attr, str, ...)
#define AUTO_TEST_TRACE_IMM(attr, str, ...)
#define AUTO_TEST_DUMP8(str, buf, cnt)
#define AUTO_TEST_DUMP16(str, buf, cnt)
#define AUTO_TEST_DUMP32(str, buf, cnt)
#endif


#ifdef BATTERY_TRACE_ENABLE
#define BATTERY_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define BATTERY_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define BATTERY_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define BATTERY_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define BATTERY_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define BATTERY_TRACE(attr, str, ...)
#define BATTERY_TRACE_IMM(attr, str, ...)
#define BATTERY_DUMP8(str, buf, cnt)
#define BATTERY_DUMP16(str, buf, cnt)
#define BATTERY_DUMP32(str, buf, cnt)
#endif


#ifdef BECO_TRACE_ENABLE
#define BECO_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define BECO_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define BECO_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define BECO_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define BECO_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define BECO_TRACE(attr, str, ...)
#define BECO_TRACE_IMM(attr, str, ...)
#define BECO_DUMP8(str, buf, cnt)
#define BECO_DUMP16(str, buf, cnt)
#define BECO_DUMP32(str, buf, cnt)
#endif


#ifdef BESAIR_PLATFORM_TRACE_ENABLE
#define BESAIR_PLATFORM_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define BESAIR_PLATFORM_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define BESAIR_PLATFORM_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define BESAIR_PLATFORM_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define BESAIR_PLATFORM_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define BESAIR_PLATFORM_TRACE(attr, str, ...)
#define BESAIR_PLATFORM_TRACE_IMM(attr, str, ...)
#define BESAIR_PLATFORM_DUMP8(str, buf, cnt)
#define BESAIR_PLATFORM_DUMP16(str, buf, cnt)
#define BESAIR_PLATFORM_DUMP32(str, buf, cnt)
#endif


#ifdef BESAIR_TRACE_ENABLE
#define BESAIR_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define BESAIR_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define BESAIR_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define BESAIR_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define BESAIR_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define BESAIR_TRACE(attr, str, ...)
#define BESAIR_TRACE_IMM(attr, str, ...)
#define BESAIR_DUMP8(str, buf, cnt)
#define BESAIR_DUMP16(str, buf, cnt)
#define BESAIR_DUMP32(str, buf, cnt)
#endif


#ifdef BINAURAL_RECORD_PROCESS_TRACE_ENABLE
#define BINAURAL_RECORD_PROCESS_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define BINAURAL_RECORD_PROCESS_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define BINAURAL_RECORD_PROCESS_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define BINAURAL_RECORD_PROCESS_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define BINAURAL_RECORD_PROCESS_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define BINAURAL_RECORD_PROCESS_TRACE(attr, str, ...)
#define BINAURAL_RECORD_PROCESS_TRACE_IMM(attr, str, ...)
#define BINAURAL_RECORD_PROCESS_DUMP8(str, buf, cnt)
#define BINAURAL_RECORD_PROCESS_DUMP16(str, buf, cnt)
#define BINAURAL_RECORD_PROCESS_DUMP32(str, buf, cnt)
#endif


#ifdef BLUEDROID_TRACE_ENABLE
#define BLUEDROID_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define BLUEDROID_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define BLUEDROID_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define BLUEDROID_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define BLUEDROID_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define BLUEDROID_TRACE(attr, str, ...)
#define BLUEDROID_TRACE_IMM(attr, str, ...)
#define BLUEDROID_DUMP8(str, buf, cnt)
#define BLUEDROID_DUMP16(str, buf, cnt)
#define BLUEDROID_DUMP32(str, buf, cnt)
#endif


#ifdef BONE_SENSOR_TRACE_ENABLE
#define BONE_SENSOR_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define BONE_SENSOR_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define BONE_SENSOR_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define BONE_SENSOR_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define BONE_SENSOR_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define BONE_SENSOR_TRACE(attr, str, ...)
#define BONE_SENSOR_TRACE_IMM(attr, str, ...)
#define BONE_SENSOR_DUMP8(str, buf, cnt)
#define BONE_SENSOR_DUMP16(str, buf, cnt)
#define BONE_SENSOR_DUMP32(str, buf, cnt)
#endif


#ifdef BTAPP_TRACE_ENABLE
#define BTAPP_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define BTAPP_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define BTAPP_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define BTAPP_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define BTAPP_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define BTAPP_TRACE(attr, str, ...)
#define BTAPP_TRACE_IMM(attr, str, ...)
#define BTAPP_DUMP8(str, buf, cnt)
#define BTAPP_DUMP16(str, buf, cnt)
#define BTAPP_DUMP32(str, buf, cnt)
#endif


#ifdef BTUSBAUDIO_TRACE_ENABLE
#define BTUSBAUDIO_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define BTUSBAUDIO_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define BTUSBAUDIO_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define BTUSBAUDIO_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define BTUSBAUDIO_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define BTUSBAUDIO_TRACE(attr, str, ...)
#define BTUSBAUDIO_TRACE_IMM(attr, str, ...)
#define BTUSBAUDIO_DUMP8(str, buf, cnt)
#define BTUSBAUDIO_DUMP16(str, buf, cnt)
#define BTUSBAUDIO_DUMP32(str, buf, cnt)
#endif


#ifdef BT_SYNC_TRACE_ENABLE
#define BT_SYNC_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define BT_SYNC_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define BT_SYNC_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define BT_SYNC_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define BT_SYNC_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define BT_SYNC_TRACE(attr, str, ...)
#define BT_SYNC_TRACE_IMM(attr, str, ...)
#define BT_SYNC_DUMP8(str, buf, cnt)
#define BT_SYNC_DUMP16(str, buf, cnt)
#define BT_SYNC_DUMP32(str, buf, cnt)
#endif


#ifdef CAPSENSOR_TRACE_ENABLE
#define CAPSENSOR_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define CAPSENSOR_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define CAPSENSOR_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define CAPSENSOR_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define CAPSENSOR_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define CAPSENSOR_TRACE(attr, str, ...)
#define CAPSENSOR_TRACE_IMM(attr, str, ...)
#define CAPSENSOR_DUMP8(str, buf, cnt)
#define CAPSENSOR_DUMP16(str, buf, cnt)
#define CAPSENSOR_DUMP32(str, buf, cnt)
#endif


#ifdef CHIP_FUNCION_TEST_TRACE_ENABLE
#define CHIP_FUNCION_TEST_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define CHIP_FUNCION_TEST_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define CHIP_FUNCION_TEST_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define CHIP_FUNCION_TEST_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define CHIP_FUNCION_TEST_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define CHIP_FUNCION_TEST_TRACE(attr, str, ...)
#define CHIP_FUNCION_TEST_TRACE_IMM(attr, str, ...)
#define CHIP_FUNCION_TEST_DUMP8(str, buf, cnt)
#define CHIP_FUNCION_TEST_DUMP16(str, buf, cnt)
#define CHIP_FUNCION_TEST_DUMP32(str, buf, cnt)
#endif


#ifdef CMD_TRACE_ENABLE
#define CMD_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define CMD_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define CMD_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define CMD_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define CMD_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define CMD_TRACE(attr, str, ...)
#define CMD_TRACE_IMM(attr, str, ...)
#define CMD_DUMP8(str, buf, cnt)
#define CMD_DUMP16(str, buf, cnt)
#define CMD_DUMP32(str, buf, cnt)
#endif


#ifdef CMSIS_TRACE_ENABLE
#define CMSIS_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define CMSIS_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define CMSIS_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define CMSIS_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define CMSIS_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define CMSIS_TRACE(attr, str, ...)
#define CMSIS_TRACE_IMM(attr, str, ...)
#define CMSIS_DUMP8(str, buf, cnt)
#define CMSIS_DUMP16(str, buf, cnt)
#define CMSIS_DUMP32(str, buf, cnt)
#endif


#ifdef CODEC_CALIB_TRACE_ENABLE
#define CODEC_CALIB_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define CODEC_CALIB_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define CODEC_CALIB_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define CODEC_CALIB_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define CODEC_CALIB_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define CODEC_CALIB_TRACE(attr, str, ...)
#define CODEC_CALIB_TRACE_IMM(attr, str, ...)
#define CODEC_CALIB_DUMP8(str, buf, cnt)
#define CODEC_CALIB_DUMP16(str, buf, cnt)
#define CODEC_CALIB_DUMP32(str, buf, cnt)
#endif


#ifdef CODEC_VAD_TRACE_ENABLE
#define CODEC_VAD_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define CODEC_VAD_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define CODEC_VAD_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define CODEC_VAD_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define CODEC_VAD_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define CODEC_VAD_TRACE(attr, str, ...)
#define CODEC_VAD_TRACE_IMM(attr, str, ...)
#define CODEC_VAD_DUMP8(str, buf, cnt)
#define CODEC_VAD_DUMP16(str, buf, cnt)
#define CODEC_VAD_DUMP32(str, buf, cnt)
#endif


#ifdef COMMON_TRACE_ENABLE
#define COMMON_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define COMMON_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define COMMON_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define COMMON_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define COMMON_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define COMMON_TRACE(attr, str, ...)
#define COMMON_TRACE_IMM(attr, str, ...)
#define COMMON_DUMP8(str, buf, cnt)
#define COMMON_DUMP16(str, buf, cnt)
#define COMMON_DUMP32(str, buf, cnt)
#endif


#ifdef COMMUNICATION_TRACE_ENABLE
#define COMMUNICATION_TRACE(attr, str, ...)   //TR_INFO(attr, str, ##__VA_ARGS__)
#define COMMUNICATION_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define COMMUNICATION_DUMP8(str, buf, cnt)     //DUMP8(str, buf, cnt)
#define COMMUNICATION_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define COMMUNICATION_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define COMMUNICATION_TRACE(attr, str, ...)
#define COMMUNICATION_TRACE_IMM(attr, str, ...)
#define COMMUNICATION_DUMP8(str, buf, cnt)
#define COMMUNICATION_DUMP16(str, buf, cnt)
#define COMMUNICATION_DUMP32(str, buf, cnt)
#endif


#ifdef CP_SUBSYS_TRACE_ENABLE
#define CP_SUBSYS_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define CP_SUBSYS_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define CP_SUBSYS_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define CP_SUBSYS_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define CP_SUBSYS_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define CP_SUBSYS_TRACE(attr, str, ...)
#define CP_SUBSYS_TRACE_IMM(attr, str, ...)
#define CP_SUBSYS_DUMP8(str, buf, cnt)
#define CP_SUBSYS_DUMP16(str, buf, cnt)
#define CP_SUBSYS_DUMP32(str, buf, cnt)
#endif


#ifdef CQUEUE_TRACE_ENABLE
#define CQUEUE_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define CQUEUE_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define CQUEUE_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define CQUEUE_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define CQUEUE_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define CQUEUE_TRACE(attr, str, ...)
#define CQUEUE_TRACE_IMM(attr, str, ...)
#define CQUEUE_DUMP8(str, buf, cnt)
#define CQUEUE_DUMP16(str, buf, cnt)
#define CQUEUE_DUMP32(str, buf, cnt)
#endif


#ifdef DRIVERS_TRACE_ENABLE
#define DRIVERS_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define DRIVERS_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define DRIVERS_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define DRIVERS_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define DRIVERS_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define DRIVERS_TRACE(attr, str, ...)
#define DRIVERS_TRACE_IMM(attr, str, ...)
#define DRIVERS_DUMP8(str, buf, cnt)
#define DRIVERS_DUMP16(str, buf, cnt)
#define DRIVERS_DUMP32(str, buf, cnt)
#endif


#ifdef DSP_M55_TRACE_ENABLE
#define DSP_M55_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define DSP_M55_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define DSP_M55_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define DSP_M55_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define DSP_M55_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define DSP_M55_TRACE(attr, str, ...)
#define DSP_M55_TRACE_IMM(attr, str, ...)
#define DSP_M55_DUMP8(str, buf, cnt)
#define DSP_M55_DUMP16(str, buf, cnt)
#define DSP_M55_DUMP32(str, buf, cnt)
#endif


#ifdef EARBUDS_TRACE_ENABLE
#define EARBUDS_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define EARBUDS_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define EARBUDS_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define EARBUDS_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define EARBUDS_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define EARBUDS_TRACE(attr, str, ...)
#define EARBUDS_TRACE_IMM(attr, str, ...)
#define EARBUDS_DUMP8(str, buf, cnt)
#define EARBUDS_DUMP16(str, buf, cnt)
#define EARBUDS_DUMP32(str, buf, cnt)
#endif


#ifdef EAUDIO_TRACE_ENABLE
#define EAUDIO_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define EAUDIO_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define EAUDIO_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define EAUDIO_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define EAUDIO_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define EAUDIO_TRACE(attr, str, ...)
#define EAUDIO_TRACE_IMM(attr, str, ...)
#define EAUDIO_DUMP8(str, buf, cnt)
#define EAUDIO_DUMP16(str, buf, cnt)
#define EAUDIO_DUMP32(str, buf, cnt)
#endif


#ifdef ECOMM_TRACE_ENABLE
#define ECOMM_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define ECOMM_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define ECOMM_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define ECOMM_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define ECOMM_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define ECOMM_TRACE(attr, str, ...)
#define ECOMM_TRACE_IMM(attr, str, ...)
#define ECOMM_DUMP8(str, buf, cnt)
#define ECOMM_DUMP16(str, buf, cnt)
#define ECOMM_DUMP32(str, buf, cnt)
#endif


#ifdef EPLAYER_TRACE_ENABLE
#define EPLAYER_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define EPLAYER_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define EPLAYER_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define EPLAYER_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define EPLAYER_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define EPLAYER_TRACE(attr, str, ...)
#define EPLAYER_TRACE_IMM(attr, str, ...)
#define EPLAYER_DUMP8(str, buf, cnt)
#define EPLAYER_DUMP16(str, buf, cnt)
#define EPLAYER_DUMP32(str, buf, cnt)
#endif


#ifdef ESHELL_TRACE_ENABLE
#define ESHELL_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define ESHELL_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define ESHELL_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define ESHELL_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define ESHELL_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define ESHELL_TRACE(attr, str, ...)
#define ESHELL_TRACE_IMM(attr, str, ...)
#define ESHELL_DUMP8(str, buf, cnt)
#define ESHELL_DUMP16(str, buf, cnt)
#define ESHELL_DUMP32(str, buf, cnt)
#endif


#ifdef EXAMPLE_TRACE_ENABLE
#define EXAMPLE_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define EXAMPLE_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define EXAMPLE_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define EXAMPLE_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define EXAMPLE_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define EXAMPLE_TRACE(attr, str, ...)
#define EXAMPLE_TRACE_IMM(attr, str, ...)
#define EXAMPLE_DUMP8(str, buf, cnt)
#define EXAMPLE_DUMP16(str, buf, cnt)
#define EXAMPLE_DUMP32(str, buf, cnt)
#endif


#ifdef FACTORY_SUITE_TRACE_ENABLE
#define FACTORY_SUITE_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define FACTORY_SUITE_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define FACTORY_SUITE_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define FACTORY_SUITE_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define FACTORY_SUITE_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define FACTORY_SUITE_TRACE(attr, str, ...)
#define FACTORY_SUITE_TRACE_IMM(attr, str, ...)
#define FACTORY_SUITE_DUMP8(str, buf, cnt)
#define FACTORY_SUITE_DUMP16(str, buf, cnt)
#define FACTORY_SUITE_DUMP32(str, buf, cnt)
#endif


#ifdef FACTORY_TRACE_ENABLE
#define FACTORY_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define FACTORY_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define FACTORY_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define FACTORY_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define FACTORY_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define FACTORY_TRACE(attr, str, ...)
#define FACTORY_TRACE_IMM(attr, str, ...)
#define FACTORY_DUMP8(str, buf, cnt)
#define FACTORY_DUMP16(str, buf, cnt)
#define FACTORY_DUMP32(str, buf, cnt)
#endif


#ifdef FASTNV_TRACE_ENABLE
#define FASTNV_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define FASTNV_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define FASTNV_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define FASTNV_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define FASTNV_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define FASTNV_TRACE(attr, str, ...)
#define FASTNV_TRACE_IMM(attr, str, ...)
#define FASTNV_DUMP8(str, buf, cnt)
#define FASTNV_DUMP16(str, buf, cnt)
#define FASTNV_DUMP32(str, buf, cnt)
#endif


#ifdef FPGA_IBRT_OTA_TRACE_ENABLE
#define FPGA_IBRT_OTA_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define FPGA_IBRT_OTA_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define FPGA_IBRT_OTA_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define FPGA_IBRT_OTA_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define FPGA_IBRT_OTA_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define FPGA_IBRT_OTA_TRACE(attr, str, ...)
#define FPGA_IBRT_OTA_TRACE_IMM(attr, str, ...)
#define FPGA_IBRT_OTA_DUMP8(str, buf, cnt)
#define FPGA_IBRT_OTA_DUMP16(str, buf, cnt)
#define FPGA_IBRT_OTA_DUMP32(str, buf, cnt)
#endif


#ifdef FREERTOS_TRACE_ENABLE
#define FREERTOS_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define FREERTOS_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define FREERTOS_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define FREERTOS_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define FREERTOS_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define FREERTOS_TRACE(attr, str, ...)
#define FREERTOS_TRACE_IMM(attr, str, ...)
#define FREERTOS_DUMP8(str, buf, cnt)
#define FREERTOS_DUMP16(str, buf, cnt)
#define FREERTOS_DUMP32(str, buf, cnt)
#endif


#ifdef FS_TRACE_ENABLE
#define FS_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define FS_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define FS_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define FS_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define FS_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define FS_TRACE(attr, str, ...)
#define FS_TRACE_IMM(attr, str, ...)
#define FS_DUMP8(str, buf, cnt)
#define FS_DUMP16(str, buf, cnt)
#define FS_DUMP32(str, buf, cnt)
#endif


#ifdef GFPS_TRACE_ENABLE
#define GFPS_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define GFPS_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define GFPS_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define GFPS_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define GFPS_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define GFPS_TRACE(attr, str, ...)
#define GFPS_TRACE_IMM(attr, str, ...)
#define GFPS_DUMP8(str, buf, cnt)
#define GFPS_DUMP16(str, buf, cnt)
#define GFPS_DUMP32(str, buf, cnt)
#endif


#ifdef HAL_TRACE_ENABLE
#define HAL_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define HAL_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define HAL_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define HAL_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define HAL_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define HAL_TRACE(attr, str, ...)
#define HAL_TRACE_IMM(attr, str, ...)
#define HAL_DUMP8(str, buf, cnt)
#define HAL_DUMP16(str, buf, cnt)
#define HAL_DUMP32(str, buf, cnt)
#endif


#ifdef HEADSET_TRACE_ENABLE
#define HEADSET_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define HEADSET_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define HEADSET_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define HEADSET_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define HEADSET_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define HEADSET_TRACE(attr, str, ...)
#define HEADSET_TRACE_IMM(attr, str, ...)
#define HEADSET_DUMP8(str, buf, cnt)
#define HEADSET_DUMP16(str, buf, cnt)
#define HEADSET_DUMP32(str, buf, cnt)
#endif


#ifdef HEAD_TRACK_3DOF_TRACE_ENABLE
#define HEAD_TRACK_3DOF_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define HEAD_TRACK_3DOF_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define HEAD_TRACK_3DOF_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define HEAD_TRACK_3DOF_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define HEAD_TRACK_3DOF_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define HEAD_TRACK_3DOF_TRACE(attr, str, ...)
#define HEAD_TRACK_3DOF_TRACE_IMM(attr, str, ...)
#define HEAD_TRACK_3DOF_DUMP8(str, buf, cnt)
#define HEAD_TRACK_3DOF_DUMP16(str, buf, cnt)
#define HEAD_TRACK_3DOF_DUMP32(str, buf, cnt)
#endif


#ifdef HEAD_TRACK_TRACE_ENABLE
#define HEAD_TRACK_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define HEAD_TRACK_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define HEAD_TRACK_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define HEAD_TRACK_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define HEAD_TRACK_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define HEAD_TRACK_TRACE(attr, str, ...)
#define HEAD_TRACK_TRACE_IMM(attr, str, ...)
#define HEAD_TRACK_DUMP8(str, buf, cnt)
#define HEAD_TRACK_DUMP16(str, buf, cnt)
#define HEAD_TRACK_DUMP32(str, buf, cnt)
#endif


#ifdef HEAP_TRACE_ENABLE
#define HEAP_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define HEAP_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define HEAP_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define HEAP_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define HEAP_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define HEAP_TRACE(attr, str, ...)
#define HEAP_TRACE_IMM(attr, str, ...)
#define HEAP_DUMP8(str, buf, cnt)
#define HEAP_DUMP16(str, buf, cnt)
#define HEAP_DUMP32(str, buf, cnt)
#endif


#ifdef HEARING_DETEC_TRACE_ENABLE
#define HEARING_DETEC_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define HEARING_DETEC_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define HEARING_DETEC_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define HEARING_DETEC_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define HEARING_DETEC_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define HEARING_DETEC_TRACE(attr, str, ...)
#define HEARING_DETEC_TRACE_IMM(attr, str, ...)
#define HEARING_DETEC_DUMP8(str, buf, cnt)
#define HEARING_DETEC_DUMP16(str, buf, cnt)
#define HEARING_DETEC_DUMP32(str, buf, cnt)
#endif


#ifdef HSM_TRACE_ENABLE
#define HSM_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define HSM_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define HSM_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define HSM_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define HSM_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define HSM_TRACE(attr, str, ...)
#define HSM_TRACE_IMM(attr, str, ...)
#define HSM_DUMP8(str, buf, cnt)
#define HSM_DUMP16(str, buf, cnt)
#define HSM_DUMP32(str, buf, cnt)
#endif


#ifdef HWTEST_TRACE_ENABLE
#define HWTEST_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define HWTEST_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define HWTEST_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define HWTEST_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define HWTEST_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define HWTEST_TRACE(attr, str, ...)
#define HWTEST_TRACE_IMM(attr, str, ...)
#define HWTEST_DUMP8(str, buf, cnt)
#define HWTEST_DUMP16(str, buf, cnt)
#define HWTEST_DUMP32(str, buf, cnt)
#endif


#ifdef HWTIMER_LIST_TRACE_ENABLE
#define HWTIMER_LIST_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define HWTIMER_LIST_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define HWTIMER_LIST_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define HWTIMER_LIST_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define HWTIMER_LIST_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define HWTIMER_LIST_TRACE(attr, str, ...)
#define HWTIMER_LIST_TRACE_IMM(attr, str, ...)
#define HWTIMER_LIST_DUMP8(str, buf, cnt)
#define HWTIMER_LIST_DUMP16(str, buf, cnt)
#define HWTIMER_LIST_DUMP32(str, buf, cnt)
#endif


#ifdef HW_DSP_TRACE_ENABLE
#define HW_DSP_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define HW_DSP_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define HW_DSP_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define HW_DSP_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define HW_DSP_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define HW_DSP_TRACE(attr, str, ...)
#define HW_DSP_TRACE_IMM(attr, str, ...)
#define HW_DSP_DUMP8(str, buf, cnt)
#define HW_DSP_DUMP16(str, buf, cnt)
#define HW_DSP_DUMP32(str, buf, cnt)
#endif


#ifdef INTERCONNECTION_TRACE_ENABLE
#define INTERCONNECTION_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define INTERCONNECTION_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define INTERCONNECTION_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define INTERCONNECTION_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define INTERCONNECTION_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define INTERCONNECTION_TRACE(attr, str, ...)
#define INTERCONNECTION_TRACE_IMM(attr, str, ...)
#define INTERCONNECTION_DUMP8(str, buf, cnt)
#define INTERCONNECTION_DUMP16(str, buf, cnt)
#define INTERCONNECTION_DUMP32(str, buf, cnt)
#endif


#ifdef INTERSYSHCI_TRACE_ENABLE
#define INTERSYSHCI_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define INTERSYSHCI_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define INTERSYSHCI_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define INTERSYSHCI_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define INTERSYSHCI_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define INTERSYSHCI_TRACE(attr, str, ...)
#define INTERSYSHCI_TRACE_IMM(attr, str, ...)
#define INTERSYSHCI_DUMP8(str, buf, cnt)
#define INTERSYSHCI_DUMP16(str, buf, cnt)
#define INTERSYSHCI_DUMP32(str, buf, cnt)
#endif


#ifdef KARAOKE_TRACE_ENABLE
#define KARAOKE_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define KARAOKE_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define KARAOKE_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define KARAOKE_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define KARAOKE_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define KARAOKE_TRACE(attr, str, ...)
#define KARAOKE_TRACE_IMM(attr, str, ...)
#define KARAOKE_DUMP8(str, buf, cnt)
#define KARAOKE_DUMP16(str, buf, cnt)
#define KARAOKE_DUMP32(str, buf, cnt)
#endif


#ifdef KEY_TRACE_ENABLE
#define KEY_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define KEY_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define KEY_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define KEY_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define KEY_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define KEY_TRACE(attr, str, ...)
#define KEY_TRACE_IMM(attr, str, ...)
#define KEY_DUMP8(str, buf, cnt)
#define KEY_DUMP16(str, buf, cnt)
#define KEY_DUMP32(str, buf, cnt)
#endif


#ifdef KFIFO_TRACE_ENABLE
#define KFIFO_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define KFIFO_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define KFIFO_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define KFIFO_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define KFIFO_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define KFIFO_TRACE(attr, str, ...)
#define KFIFO_TRACE_IMM(attr, str, ...)
#define KFIFO_DUMP8(str, buf, cnt)
#define KFIFO_DUMP16(str, buf, cnt)
#define KFIFO_DUMP32(str, buf, cnt)
#endif


#ifdef KNOWLES_UART_LIB_TRACE_ENABLE
#define KNOWLES_UART_LIB_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define KNOWLES_UART_LIB_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define KNOWLES_UART_LIB_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define KNOWLES_UART_LIB_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define KNOWLES_UART_LIB_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define KNOWLES_UART_LIB_TRACE(attr, str, ...)
#define KNOWLES_UART_LIB_TRACE_IMM(attr, str, ...)
#define KNOWLES_UART_LIB_DUMP8(str, buf, cnt)
#define KNOWLES_UART_LIB_DUMP16(str, buf, cnt)
#define KNOWLES_UART_LIB_DUMP32(str, buf, cnt)
#endif


#ifdef LEA_PLAYER_TRACE_ENABLE
#define LEA_PLAYER_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define LEA_PLAYER_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define LEA_PLAYER_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define LEA_PLAYER_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define LEA_PLAYER_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define LEA_PLAYER_TRACE(attr, str, ...)
#define LEA_PLAYER_TRACE_IMM(attr, str, ...)
#define LEA_PLAYER_DUMP8(str, buf, cnt)
#define LEA_PLAYER_DUMP16(str, buf, cnt)
#define LEA_PLAYER_DUMP32(str, buf, cnt)
#endif


#ifdef LIGHTNING_TEST_TRACE_ENABLE
#define LIGHTNING_TEST_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define LIGHTNING_TEST_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define LIGHTNING_TEST_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define LIGHTNING_TEST_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define LIGHTNING_TEST_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define LIGHTNING_TEST_TRACE(attr, str, ...)
#define LIGHTNING_TEST_TRACE_IMM(attr, str, ...)
#define LIGHTNING_TEST_DUMP8(str, buf, cnt)
#define LIGHTNING_TEST_DUMP16(str, buf, cnt)
#define LIGHTNING_TEST_DUMP32(str, buf, cnt)
#endif


#ifdef LIST_TRACE_ENABLE
#define LIST_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define LIST_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define LIST_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define LIST_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define LIST_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define LIST_TRACE(attr, str, ...)
#define LIST_TRACE_IMM(attr, str, ...)
#define LIST_DUMP8(str, buf, cnt)
#define LIST_DUMP16(str, buf, cnt)
#define LIST_DUMP32(str, buf, cnt)
#endif


#ifdef LITEOS_TRACE_ENABLE
#define LITEOS_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define LITEOS_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define LITEOS_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define LITEOS_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define LITEOS_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define LITEOS_TRACE(attr, str, ...)
#define LITEOS_TRACE_IMM(attr, str, ...)
#define LITEOS_DUMP8(str, buf, cnt)
#define LITEOS_DUMP16(str, buf, cnt)
#define LITEOS_DUMP32(str, buf, cnt)
#endif


#ifdef LZMA_TRACE_ENABLE
#define LZMA_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define LZMA_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define LZMA_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define LZMA_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define LZMA_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define LZMA_TRACE(attr, str, ...)
#define LZMA_TRACE_IMM(attr, str, ...)
#define LZMA_DUMP8(str, buf, cnt)
#define LZMA_DUMP16(str, buf, cnt)
#define LZMA_DUMP32(str, buf, cnt)
#endif


#ifdef MAIN_TRACE_ENABLE
#define MAIN_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define MAIN_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define MAIN_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define MAIN_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define MAIN_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define MAIN_TRACE(attr, str, ...)
#define MAIN_TRACE_IMM(attr, str, ...)
#define MAIN_DUMP8(str, buf, cnt)
#define MAIN_DUMP16(str, buf, cnt)
#define MAIN_DUMP32(str, buf, cnt)
#endif


#ifdef MBEDTLS_TRACE_ENABLE
#define MBEDTLS_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define MBEDTLS_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define MBEDTLS_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define MBEDTLS_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define MBEDTLS_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define MBEDTLS_TRACE(attr, str, ...)
#define MBEDTLS_TRACE_IMM(attr, str, ...)
#define MBEDTLS_DUMP8(str, buf, cnt)
#define MBEDTLS_DUMP16(str, buf, cnt)
#define MBEDTLS_DUMP32(str, buf, cnt)
#endif


#ifdef MCPP_TRACE_ENABLE
#define MCPP_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define MCPP_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define MCPP_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define MCPP_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define MCPP_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define MCPP_TRACE(attr, str, ...)
#define MCPP_TRACE_IMM(attr, str, ...)
#define MCPP_DUMP8(str, buf, cnt)
#define MCPP_DUMP16(str, buf, cnt)
#define MCPP_DUMP32(str, buf, cnt)
#endif


#ifdef MEM_TEST_TRACE_ENABLE
#define MEM_TEST_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define MEM_TEST_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define MEM_TEST_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define MEM_TEST_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define MEM_TEST_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define MEM_TEST_TRACE(attr, str, ...)
#define MEM_TEST_TRACE_IMM(attr, str, ...)
#define MEM_TEST_DUMP8(str, buf, cnt)
#define MEM_TEST_DUMP16(str, buf, cnt)
#define MEM_TEST_DUMP32(str, buf, cnt)
#endif


#ifdef MIC_TRACE_ENABLE
#define MIC_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define MIC_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define MIC_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define MIC_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define MIC_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define MIC_TRACE(attr, str, ...)
#define MIC_TRACE_IMM(attr, str, ...)
#define MIC_DUMP8(str, buf, cnt)
#define MIC_DUMP16(str, buf, cnt)
#define MIC_DUMP32(str, buf, cnt)
#endif


#ifdef MINIMA_TEST_TRACE_ENABLE
#define MINIMA_TEST_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define MINIMA_TEST_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define MINIMA_TEST_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define MINIMA_TEST_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define MINIMA_TEST_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define MINIMA_TEST_TRACE(attr, str, ...)
#define MINIMA_TEST_TRACE_IMM(attr, str, ...)
#define MINIMA_TEST_DUMP8(str, buf, cnt)
#define MINIMA_TEST_DUMP16(str, buf, cnt)
#define MINIMA_TEST_DUMP32(str, buf, cnt)
#endif


#ifdef NOISE_TRACKER_LIB_TRACE_ENABLE
#define NOISE_TRACKER_LIB_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define NOISE_TRACKER_LIB_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define NOISE_TRACKER_LIB_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define NOISE_TRACKER_LIB_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define NOISE_TRACKER_LIB_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define NOISE_TRACKER_LIB_TRACE(attr, str, ...)
#define NOISE_TRACKER_LIB_TRACE_IMM(attr, str, ...)
#define NOISE_TRACKER_LIB_DUMP8(str, buf, cnt)
#define NOISE_TRACKER_LIB_DUMP16(str, buf, cnt)
#define NOISE_TRACKER_LIB_DUMP32(str, buf, cnt)
#endif


#ifdef NORFLASH_API_TRACE_ENABLE
#define NORFLASH_API_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define NORFLASH_API_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define NORFLASH_API_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define NORFLASH_API_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define NORFLASH_API_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define NORFLASH_API_TRACE(attr, str, ...)
#define NORFLASH_API_TRACE_IMM(attr, str, ...)
#define NORFLASH_API_DUMP8(str, buf, cnt)
#define NORFLASH_API_DUMP16(str, buf, cnt)
#define NORFLASH_API_DUMP32(str, buf, cnt)
#endif


#ifdef NUTTX_TRACE_ENABLE
#define NUTTX_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define NUTTX_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define NUTTX_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define NUTTX_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define NUTTX_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define NUTTX_TRACE(attr, str, ...)
#define NUTTX_TRACE_IMM(attr, str, ...)
#define NUTTX_DUMP8(str, buf, cnt)
#define NUTTX_DUMP16(str, buf, cnt)
#define NUTTX_DUMP32(str, buf, cnt)
#endif


#ifdef NV_SECTION_TRACE_ENABLE
#define NV_SECTION_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define NV_SECTION_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define NV_SECTION_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define NV_SECTION_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define NV_SECTION_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define NV_SECTION_TRACE(attr, str, ...)
#define NV_SECTION_TRACE_IMM(attr, str, ...)
#define NV_SECTION_DUMP8(str, buf, cnt)
#define NV_SECTION_DUMP16(str, buf, cnt)
#define NV_SECTION_DUMP32(str, buf, cnt)
#endif


#ifdef OHOS_WALKIE_TALKIE_STREAM_TRACE_ENABLE
#define OHOS_WALKIE_TALKIE_STREAM_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define OHOS_WALKIE_TALKIE_STREAM_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define OHOS_WALKIE_TALKIE_STREAM_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define OHOS_WALKIE_TALKIE_STREAM_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define OHOS_WALKIE_TALKIE_STREAM_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define OHOS_WALKIE_TALKIE_STREAM_TRACE(attr, str, ...)
#define OHOS_WALKIE_TALKIE_STREAM_TRACE_IMM(attr, str, ...)
#define OHOS_WALKIE_TALKIE_STREAM_DUMP8(str, buf, cnt)
#define OHOS_WALKIE_TALKIE_STREAM_DUMP16(str, buf, cnt)
#define OHOS_WALKIE_TALKIE_STREAM_DUMP32(str, buf, cnt)
#endif


#ifdef OSIF_TRACE_ENABLE
#define OSIF_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define OSIF_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define OSIF_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define OSIF_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define OSIF_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define OSIF_TRACE(attr, str, ...)
#define OSIF_TRACE_IMM(attr, str, ...)
#define OSIF_DUMP8(str, buf, cnt)
#define OSIF_DUMP16(str, buf, cnt)
#define OSIF_DUMP32(str, buf, cnt)
#endif


#ifdef OTA_BOOT_REMAP_TRACE_ENABLE
#define OTA_BOOT_REMAP_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define OTA_BOOT_REMAP_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define OTA_BOOT_REMAP_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define OTA_BOOT_REMAP_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define OTA_BOOT_REMAP_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define OTA_BOOT_REMAP_TRACE(attr, str, ...)
#define OTA_BOOT_REMAP_TRACE_IMM(attr, str, ...)
#define OTA_BOOT_REMAP_DUMP8(str, buf, cnt)
#define OTA_BOOT_REMAP_DUMP16(str, buf, cnt)
#define OTA_BOOT_REMAP_DUMP32(str, buf, cnt)
#endif


#ifdef OTA_BOOT_TRACE_ENABLE
#define OTA_BOOT_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define OTA_BOOT_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define OTA_BOOT_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define OTA_BOOT_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define OTA_BOOT_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define OTA_BOOT_TRACE(attr, str, ...)
#define OTA_BOOT_TRACE_IMM(attr, str, ...)
#define OTA_BOOT_DUMP8(str, buf, cnt)
#define OTA_BOOT_DUMP16(str, buf, cnt)
#define OTA_BOOT_DUMP32(str, buf, cnt)
#endif


#ifdef OTA_TRACE_ENABLE
#define OTA_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define OTA_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define OTA_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define OTA_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define OTA_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define OTA_TRACE(attr, str, ...)
#define OTA_TRACE_IMM(attr, str, ...)
#define OTA_DUMP8(str, buf, cnt)
#define OTA_DUMP16(str, buf, cnt)
#define OTA_DUMP32(str, buf, cnt)
#endif


#ifdef OVERLAY_TRACE_ENABLE
#define OVERLAY_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define OVERLAY_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define OVERLAY_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define OVERLAY_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define OVERLAY_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define OVERLAY_TRACE(attr, str, ...)
#define OVERLAY_TRACE_IMM(attr, str, ...)
#define OVERLAY_DUMP8(str, buf, cnt)
#define OVERLAY_DUMP16(str, buf, cnt)
#define OVERLAY_DUMP32(str, buf, cnt)
#endif


#ifdef PRESSURE_TRACE_ENABLE
#define PRESSURE_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define PRESSURE_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define PRESSURE_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define PRESSURE_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define PRESSURE_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define PRESSURE_TRACE(attr, str, ...)
#define PRESSURE_TRACE_IMM(attr, str, ...)
#define PRESSURE_DUMP8(str, buf, cnt)
#define PRESSURE_DUMP16(str, buf, cnt)
#define PRESSURE_DUMP32(str, buf, cnt)
#endif


#ifdef PROGRAMMER_EXT_TRACE_ENABLE
#define PROGRAMMER_EXT_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define PROGRAMMER_EXT_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define PROGRAMMER_EXT_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define PROGRAMMER_EXT_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define PROGRAMMER_EXT_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define PROGRAMMER_EXT_TRACE(attr, str, ...)
#define PROGRAMMER_EXT_TRACE_IMM(attr, str, ...)
#define PROGRAMMER_EXT_DUMP8(str, buf, cnt)
#define PROGRAMMER_EXT_DUMP16(str, buf, cnt)
#define PROGRAMMER_EXT_DUMP32(str, buf, cnt)
#endif


#ifdef PROGRAMMER_TRACE_ENABLE
#define PROGRAMMER_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define PROGRAMMER_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define PROGRAMMER_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define PROGRAMMER_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define PROGRAMMER_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define PROGRAMMER_TRACE(attr, str, ...)
#define PROGRAMMER_TRACE_IMM(attr, str, ...)
#define PROGRAMMER_DUMP8(str, buf, cnt)
#define PROGRAMMER_DUMP16(str, buf, cnt)
#define PROGRAMMER_DUMP32(str, buf, cnt)
#endif


#ifdef PSRAM_CALIB_TRACE_ENABLE
#define PSRAM_CALIB_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define PSRAM_CALIB_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define PSRAM_CALIB_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define PSRAM_CALIB_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define PSRAM_CALIB_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define PSRAM_CALIB_TRACE(attr, str, ...)
#define PSRAM_CALIB_TRACE_IMM(attr, str, ...)
#define PSRAM_CALIB_DUMP8(str, buf, cnt)
#define PSRAM_CALIB_DUMP16(str, buf, cnt)
#define PSRAM_CALIB_DUMP32(str, buf, cnt)
#endif


#ifdef PWL_TRACE_ENABLE
#define PWL_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define PWL_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define PWL_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define PWL_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define PWL_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define PWL_TRACE(attr, str, ...)
#define PWL_TRACE_IMM(attr, str, ...)
#define PWL_DUMP8(str, buf, cnt)
#define PWL_DUMP16(str, buf, cnt)
#define PWL_DUMP32(str, buf, cnt)
#endif


#ifdef RAMRUN_TEST_TRACE_ENABLE
#define RAMRUN_TEST_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define RAMRUN_TEST_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define RAMRUN_TEST_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define RAMRUN_TEST_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define RAMRUN_TEST_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define RAMRUN_TEST_TRACE(attr, str, ...)
#define RAMRUN_TEST_TRACE_IMM(attr, str, ...)
#define RAMRUN_TEST_DUMP8(str, buf, cnt)
#define RAMRUN_TEST_DUMP16(str, buf, cnt)
#define RAMRUN_TEST_DUMP32(str, buf, cnt)
#endif


#ifdef RATE_TEST_DEMO_TRACE_ENABLE
#define RATE_TEST_DEMO_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define RATE_TEST_DEMO_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define RATE_TEST_DEMO_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define RATE_TEST_DEMO_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define RATE_TEST_DEMO_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define RATE_TEST_DEMO_TRACE(attr, str, ...)
#define RATE_TEST_DEMO_TRACE_IMM(attr, str, ...)
#define RATE_TEST_DEMO_DUMP8(str, buf, cnt)
#define RATE_TEST_DEMO_DUMP16(str, buf, cnt)
#define RATE_TEST_DEMO_DUMP32(str, buf, cnt)
#endif


#ifdef RELIABILITY_TEST_TRACE_ENABLE
#define RELIABILITY_TEST_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define RELIABILITY_TEST_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define RELIABILITY_TEST_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define RELIABILITY_TEST_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define RELIABILITY_TEST_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define RELIABILITY_TEST_TRACE(attr, str, ...)
#define RELIABILITY_TEST_TRACE_IMM(attr, str, ...)
#define RELIABILITY_TEST_DUMP8(str, buf, cnt)
#define RELIABILITY_TEST_DUMP16(str, buf, cnt)
#define RELIABILITY_TEST_DUMP32(str, buf, cnt)
#endif


#ifdef RETENTION_RAM_TRACE_ENABLE
#define RETENTION_RAM_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define RETENTION_RAM_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define RETENTION_RAM_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define RETENTION_RAM_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define RETENTION_RAM_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define RETENTION_RAM_TRACE(attr, str, ...)
#define RETENTION_RAM_TRACE_IMM(attr, str, ...)
#define RETENTION_RAM_DUMP8(str, buf, cnt)
#define RETENTION_RAM_DUMP16(str, buf, cnt)
#define RETENTION_RAM_DUMP32(str, buf, cnt)
#endif


#ifdef ROM_TRACE_ENABLE
#define ROM_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define ROM_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define ROM_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define ROM_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define ROM_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define ROM_TRACE(attr, str, ...)
#define ROM_TRACE_IMM(attr, str, ...)
#define ROM_DUMP8(str, buf, cnt)
#define ROM_DUMP16(str, buf, cnt)
#define ROM_DUMP32(str, buf, cnt)
#endif


#ifdef ROM_UTILS_TRACE_ENABLE
#define ROM_UTILS_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define ROM_UTILS_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define ROM_UTILS_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define ROM_UTILS_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define ROM_UTILS_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define ROM_UTILS_TRACE(attr, str, ...)
#define ROM_UTILS_TRACE_IMM(attr, str, ...)
#define ROM_UTILS_DUMP8(str, buf, cnt)
#define ROM_UTILS_DUMP16(str, buf, cnt)
#define ROM_UTILS_DUMP32(str, buf, cnt)
#endif


#ifdef RPC_TRACE_ENABLE
#define RPC_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define RPC_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define RPC_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define RPC_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define RPC_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define RPC_TRACE(attr, str, ...)
#define RPC_TRACE_IMM(attr, str, ...)
#define RPC_DUMP8(str, buf, cnt)
#define RPC_DUMP16(str, buf, cnt)
#define RPC_DUMP32(str, buf, cnt)
#endif


#ifdef RTX5_TRACE_ENABLE
#define RTX5_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define RTX5_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define RTX5_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define RTX5_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define RTX5_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define RTX5_TRACE(attr, str, ...)
#define RTX5_TRACE_IMM(attr, str, ...)
#define RTX5_DUMP8(str, buf, cnt)
#define RTX5_DUMP16(str, buf, cnt)
#define RTX5_DUMP32(str, buf, cnt)
#endif


#ifdef RTX_TRACE_ENABLE
#define RTX_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define RTX_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define RTX_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define RTX_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define RTX_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define RTX_TRACE(attr, str, ...)
#define RTX_TRACE_IMM(attr, str, ...)
#define RTX_DUMP8(str, buf, cnt)
#define RTX_DUMP16(str, buf, cnt)
#define RTX_DUMP32(str, buf, cnt)
#endif


#ifdef RT_THREAD_TRACE_ENABLE
#define RT_THREAD_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define RT_THREAD_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define RT_THREAD_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define RT_THREAD_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define RT_THREAD_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define RT_THREAD_TRACE(attr, str, ...)
#define RT_THREAD_TRACE_IMM(attr, str, ...)
#define RT_THREAD_DUMP8(str, buf, cnt)
#define RT_THREAD_DUMP16(str, buf, cnt)
#define RT_THREAD_DUMP32(str, buf, cnt)
#endif


#ifdef SDIO_DOWNLOAD_TEST_TRACE_ENABLE
#define SDIO_DOWNLOAD_TEST_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define SDIO_DOWNLOAD_TEST_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define SDIO_DOWNLOAD_TEST_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define SDIO_DOWNLOAD_TEST_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define SDIO_DOWNLOAD_TEST_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define SDIO_DOWNLOAD_TEST_TRACE(attr, str, ...)
#define SDIO_DOWNLOAD_TEST_TRACE_IMM(attr, str, ...)
#define SDIO_DOWNLOAD_TEST_DUMP8(str, buf, cnt)
#define SDIO_DOWNLOAD_TEST_DUMP16(str, buf, cnt)
#define SDIO_DOWNLOAD_TEST_DUMP32(str, buf, cnt)
#endif


#ifdef SDMMC_MSD_TRACE_ENABLE
#define SDMMC_MSD_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define SDMMC_MSD_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define SDMMC_MSD_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define SDMMC_MSD_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define SDMMC_MSD_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define SDMMC_MSD_TRACE(attr, str, ...)
#define SDMMC_MSD_TRACE_IMM(attr, str, ...)
#define SDMMC_MSD_DUMP8(str, buf, cnt)
#define SDMMC_MSD_DUMP16(str, buf, cnt)
#define SDMMC_MSD_DUMP32(str, buf, cnt)
#endif


#ifdef SDMMC_TRACE_ENABLE
#define SDMMC_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define SDMMC_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define SDMMC_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define SDMMC_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define SDMMC_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define SDMMC_TRACE(attr, str, ...)
#define SDMMC_TRACE_IMM(attr, str, ...)
#define SDMMC_DUMP8(str, buf, cnt)
#define SDMMC_DUMP16(str, buf, cnt)
#define SDMMC_DUMP32(str, buf, cnt)
#endif


#ifdef SECURE_BOOT_TRACE_ENABLE
#define SECURE_BOOT_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define SECURE_BOOT_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define SECURE_BOOT_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define SECURE_BOOT_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define SECURE_BOOT_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define SECURE_BOOT_TRACE(attr, str, ...)
#define SECURE_BOOT_TRACE_IMM(attr, str, ...)
#define SECURE_BOOT_DUMP8(str, buf, cnt)
#define SECURE_BOOT_DUMP16(str, buf, cnt)
#define SECURE_BOOT_DUMP32(str, buf, cnt)
#endif


#ifdef SENSHUB_LIB_TRACE_ENABLE
#define SENSHUB_LIB_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define SENSHUB_LIB_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define SENSHUB_LIB_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define SENSHUB_LIB_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define SENSHUB_LIB_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define SENSHUB_LIB_TRACE(attr, str, ...)
#define SENSHUB_LIB_TRACE_IMM(attr, str, ...)
#define SENSHUB_LIB_DUMP8(str, buf, cnt)
#define SENSHUB_LIB_DUMP16(str, buf, cnt)
#define SENSHUB_LIB_DUMP32(str, buf, cnt)
#endif


#ifdef SENSORHUB_TRACE_ENABLE
#define SENSORHUB_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define SENSORHUB_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define SENSORHUB_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define SENSORHUB_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define SENSORHUB_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define SENSORHUB_TRACE(attr, str, ...)
#define SENSORHUB_TRACE_IMM(attr, str, ...)
#define SENSORHUB_DUMP8(str, buf, cnt)
#define SENSORHUB_DUMP16(str, buf, cnt)
#define SENSORHUB_DUMP32(str, buf, cnt)
#endif


#ifdef SENSOR_HUB_TRACE_ENABLE
#define SENSOR_HUB_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define SENSOR_HUB_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define SENSOR_HUB_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define SENSOR_HUB_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define SENSOR_HUB_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define SENSOR_HUB_TRACE(attr, str, ...)
#define SENSOR_HUB_TRACE_IMM(attr, str, ...)
#define SENSOR_HUB_DUMP8(str, buf, cnt)
#define SENSOR_HUB_DUMP16(str, buf, cnt)
#define SENSOR_HUB_DUMP32(str, buf, cnt)
#endif


#ifdef SENSOR_TEST_TRACE_ENABLE
#define SENSOR_TEST_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define SENSOR_TEST_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define SENSOR_TEST_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define SENSOR_TEST_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define SENSOR_TEST_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define SENSOR_TEST_TRACE(attr, str, ...)
#define SENSOR_TEST_TRACE_IMM(attr, str, ...)
#define SENSOR_TEST_DUMP8(str, buf, cnt)
#define SENSOR_TEST_DUMP16(str, buf, cnt)
#define SENSOR_TEST_DUMP32(str, buf, cnt)
#endif


#ifdef SIGNAL_GENERATOR_TRACE_ENABLE
#define SIGNAL_GENERATOR_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define SIGNAL_GENERATOR_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define SIGNAL_GENERATOR_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define SIGNAL_GENERATOR_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define SIGNAL_GENERATOR_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define SIGNAL_GENERATOR_TRACE(attr, str, ...)
#define SIGNAL_GENERATOR_TRACE_IMM(attr, str, ...)
#define SIGNAL_GENERATOR_DUMP8(str, buf, cnt)
#define SIGNAL_GENERATOR_DUMP16(str, buf, cnt)
#define SIGNAL_GENERATOR_DUMP32(str, buf, cnt)
#endif


#ifdef SIMU_TEST_TRACE_ENABLE
#define SIMU_TEST_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define SIMU_TEST_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define SIMU_TEST_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define SIMU_TEST_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define SIMU_TEST_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define SIMU_TEST_TRACE(attr, str, ...)
#define SIMU_TEST_TRACE_IMM(attr, str, ...)
#define SIMU_TEST_DUMP8(str, buf, cnt)
#define SIMU_TEST_DUMP16(str, buf, cnt)
#define SIMU_TEST_DUMP32(str, buf, cnt)
#endif


#ifdef SOUNDPLUS_LIB_TRACE_ENABLE
#define SOUNDPLUS_LIB_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define SOUNDPLUS_LIB_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define SOUNDPLUS_LIB_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define SOUNDPLUS_LIB_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define SOUNDPLUS_LIB_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define SOUNDPLUS_LIB_TRACE(attr, str, ...)
#define SOUNDPLUS_LIB_TRACE_IMM(attr, str, ...)
#define SOUNDPLUS_LIB_DUMP8(str, buf, cnt)
#define SOUNDPLUS_LIB_DUMP16(str, buf, cnt)
#define SOUNDPLUS_LIB_DUMP32(str, buf, cnt)
#endif


#ifdef SOUND_TRACE_ENABLE
#define SOUND_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define SOUND_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define SOUND_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define SOUND_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define SOUND_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define SOUND_TRACE(attr, str, ...)
#define SOUND_TRACE_IMM(attr, str, ...)
#define SOUND_DUMP8(str, buf, cnt)
#define SOUND_DUMP16(str, buf, cnt)
#define SOUND_DUMP32(str, buf, cnt)
#endif


#ifdef SPEECH_TEST_TRACE_ENABLE
#define SPEECH_TEST_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define SPEECH_TEST_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define SPEECH_TEST_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define SPEECH_TEST_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define SPEECH_TEST_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define SPEECH_TEST_TRACE(attr, str, ...)
#define SPEECH_TEST_TRACE_IMM(attr, str, ...)
#define SPEECH_TEST_DUMP8(str, buf, cnt)
#define SPEECH_TEST_DUMP16(str, buf, cnt)
#define SPEECH_TEST_DUMP32(str, buf, cnt)
#endif


#ifdef SPP_TEST_TRACE_ENABLE
#define SPP_TEST_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define SPP_TEST_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define SPP_TEST_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define SPP_TEST_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define SPP_TEST_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define SPP_TEST_TRACE(attr, str, ...)
#define SPP_TEST_TRACE_IMM(attr, str, ...)
#define SPP_TEST_DUMP8(str, buf, cnt)
#define SPP_TEST_DUMP16(str, buf, cnt)
#define SPP_TEST_DUMP32(str, buf, cnt)
#endif


#ifdef STREAM_MCPS_TRACE_ENABLE
#define STREAM_MCPS_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define STREAM_MCPS_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define STREAM_MCPS_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define STREAM_MCPS_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define STREAM_MCPS_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define STREAM_MCPS_TRACE(attr, str, ...)
#define STREAM_MCPS_TRACE_IMM(attr, str, ...)
#define STREAM_MCPS_DUMP8(str, buf, cnt)
#define STREAM_MCPS_DUMP16(str, buf, cnt)
#define STREAM_MCPS_DUMP32(str, buf, cnt)
#endif


#ifdef THROUGH_PUT_TRACE_ENABLE
#define THROUGH_PUT_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define THROUGH_PUT_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define THROUGH_PUT_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define THROUGH_PUT_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define THROUGH_PUT_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define THROUGH_PUT_TRACE(attr, str, ...)
#define THROUGH_PUT_TRACE_IMM(attr, str, ...)
#define THROUGH_PUT_DUMP8(str, buf, cnt)
#define THROUGH_PUT_DUMP16(str, buf, cnt)
#define THROUGH_PUT_DUMP32(str, buf, cnt)
#endif


#ifdef TILE_TRACE_ENABLE
#define TILE_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define TILE_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define TILE_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define TILE_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define TILE_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define TILE_TRACE(attr, str, ...)
#define TILE_TRACE_IMM(attr, str, ...)
#define TILE_DUMP8(str, buf, cnt)
#define TILE_DUMP16(str, buf, cnt)
#define TILE_DUMP32(str, buf, cnt)
#endif


#ifdef TOTA_TRACE_ENABLE
#define TOTA_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define TOTA_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define TOTA_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define TOTA_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define TOTA_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define TOTA_TRACE(attr, str, ...)
#define TOTA_TRACE_IMM(attr, str, ...)
#define TOTA_DUMP8(str, buf, cnt)
#define TOTA_DUMP16(str, buf, cnt)
#define TOTA_DUMP32(str, buf, cnt)
#endif


#ifdef TOTA_V2_TRACE_ENABLE
#define TOTA_V2_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define TOTA_V2_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define TOTA_V2_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define TOTA_V2_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define TOTA_V2_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define TOTA_V2_TRACE(attr, str, ...)
#define TOTA_V2_TRACE_IMM(attr, str, ...)
#define TOTA_V2_DUMP8(str, buf, cnt)
#define TOTA_V2_DUMP16(str, buf, cnt)
#define TOTA_V2_DUMP32(str, buf, cnt)
#endif


#ifdef UARTHCI_TRACE_ENABLE
#define UARTHCI_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define UARTHCI_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define UARTHCI_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define UARTHCI_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define UARTHCI_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define UARTHCI_TRACE(attr, str, ...)
#define UARTHCI_TRACE_IMM(attr, str, ...)
#define UARTHCI_DUMP8(str, buf, cnt)
#define UARTHCI_DUMP16(str, buf, cnt)
#define UARTHCI_DUMP32(str, buf, cnt)
#endif


#ifdef USBAUDIO_TRACE_ENABLE
#define USBAUDIO_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define USBAUDIO_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define USBAUDIO_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define USBAUDIO_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define USBAUDIO_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define USBAUDIO_TRACE(attr, str, ...)
#define USBAUDIO_TRACE_IMM(attr, str, ...)
#define USBAUDIO_DUMP8(str, buf, cnt)
#define USBAUDIO_DUMP16(str, buf, cnt)
#define USBAUDIO_DUMP32(str, buf, cnt)
#endif


#ifdef USBHOSTSHELL_TRACE_ENABLE
#define USBHOSTSHELL_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define USBHOSTSHELL_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define USBHOSTSHELL_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define USBHOSTSHELL_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define USBHOSTSHELL_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define USBHOSTSHELL_TRACE(attr, str, ...)
#define USBHOSTSHELL_TRACE_IMM(attr, str, ...)
#define USBHOSTSHELL_DUMP8(str, buf, cnt)
#define USBHOSTSHELL_DUMP16(str, buf, cnt)
#define USBHOSTSHELL_DUMP32(str, buf, cnt)
#endif


#ifdef USERAPI_TRACE_ENABLE
#define USERAPI_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define USERAPI_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define USERAPI_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define USERAPI_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define USERAPI_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define USERAPI_TRACE(attr, str, ...)
#define USERAPI_TRACE_IMM(attr, str, ...)
#define USERAPI_DUMP8(str, buf, cnt)
#define USERAPI_DUMP16(str, buf, cnt)
#define USERAPI_DUMP32(str, buf, cnt)
#endif


#ifdef USER_SECURE_BOOT_TRACE_ENABLE
#define USER_SECURE_BOOT_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define USER_SECURE_BOOT_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define USER_SECURE_BOOT_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define USER_SECURE_BOOT_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define USER_SECURE_BOOT_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define USER_SECURE_BOOT_TRACE(attr, str, ...)
#define USER_SECURE_BOOT_TRACE_IMM(attr, str, ...)
#define USER_SECURE_BOOT_DUMP8(str, buf, cnt)
#define USER_SECURE_BOOT_DUMP16(str, buf, cnt)
#define USER_SECURE_BOOT_DUMP32(str, buf, cnt)
#endif


#ifdef VOICEPATH_TRACE_ENABLE
#define VOICEPATH_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define VOICEPATH_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define VOICEPATH_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define VOICEPATH_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define VOICEPATH_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define VOICEPATH_TRACE(attr, str, ...)
#define VOICEPATH_TRACE_IMM(attr, str, ...)
#define VOICEPATH_DUMP8(str, buf, cnt)
#define VOICEPATH_DUMP16(str, buf, cnt)
#define VOICEPATH_DUMP32(str, buf, cnt)
#endif


#ifdef VOICE_ASSIST_TRACE_ENABLE
#define VOICE_ASSIST_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define VOICE_ASSIST_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define VOICE_ASSIST_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define VOICE_ASSIST_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define VOICE_ASSIST_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define VOICE_ASSIST_TRACE(attr, str, ...)
#define VOICE_ASSIST_TRACE_IMM(attr, str, ...)
#define VOICE_ASSIST_DUMP8(str, buf, cnt)
#define VOICE_ASSIST_DUMP16(str, buf, cnt)
#define VOICE_ASSIST_DUMP32(str, buf, cnt)
#endif


#ifdef VOICE_DETECTOR_TRACE_ENABLE
#define VOICE_DETECTOR_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define VOICE_DETECTOR_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define VOICE_DETECTOR_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define VOICE_DETECTOR_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define VOICE_DETECTOR_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define VOICE_DETECTOR_TRACE(attr, str, ...)
#define VOICE_DETECTOR_TRACE_IMM(attr, str, ...)
#define VOICE_DETECTOR_DUMP8(str, buf, cnt)
#define VOICE_DETECTOR_DUMP16(str, buf, cnt)
#define VOICE_DETECTOR_DUMP32(str, buf, cnt)
#endif


#ifdef VOICE_DEV_TRACE_ENABLE
#define VOICE_DEV_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define VOICE_DEV_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define VOICE_DEV_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define VOICE_DEV_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define VOICE_DEV_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define VOICE_DEV_TRACE(attr, str, ...)
#define VOICE_DEV_TRACE_IMM(attr, str, ...)
#define VOICE_DEV_DUMP8(str, buf, cnt)
#define VOICE_DEV_DUMP16(str, buf, cnt)
#define VOICE_DEV_DUMP32(str, buf, cnt)
#endif


#ifdef WALKIE_TALKIE_TRACE_ENABLE
#define WALKIE_TALKIE_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define WALKIE_TALKIE_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define WALKIE_TALKIE_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define WALKIE_TALKIE_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define WALKIE_TALKIE_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define WALKIE_TALKIE_TRACE(attr, str, ...)
#define WALKIE_TALKIE_TRACE_IMM(attr, str, ...)
#define WALKIE_TALKIE_DUMP8(str, buf, cnt)
#define WALKIE_TALKIE_DUMP16(str, buf, cnt)
#define WALKIE_TALKIE_DUMP32(str, buf, cnt)
#endif


#ifdef XIAOMI_ENC_TRACE_ENABLE
#define XIAOMI_ENC_TRACE(attr, str, ...)   TR_INFO(attr, str, ##__VA_ARGS__)
#define XIAOMI_ENC_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define XIAOMI_ENC_DUMP8(str, buf, cnt)    DUMP8(str, buf, cnt) 
#define XIAOMI_ENC_DUMP16(str, buf, cnt)    DUMP16(str, buf, cnt) 
#define XIAOMI_ENC_DUMP32(str, buf, cnt)    DUMP32(str, buf, cnt) 
#else
#define XIAOMI_ENC_TRACE(attr, str, ...)
#define XIAOMI_ENC_TRACE_IMM(attr, str, ...)
#define XIAOMI_ENC_DUMP8(str, buf, cnt)
#define XIAOMI_ENC_DUMP16(str, buf, cnt)
#define XIAOMI_ENC_DUMP32(str, buf, cnt)
#endif

#ifdef FINDMY_TRACE_ENABLE
#define FINDMY_TRACE(attr, str, ...)        TR_INFO(attr, str, ##__VA_ARGS__)
#define FINDMY_TRACE_IMM(attr, str, ...)    TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define FINDMY_DUMP8(str, buf, cnt)         DUMP8(str, buf, cnt)
#define FINDMY_DUMP16(str, buf, cnt)        DUMP16(str, buf, cnt)
#define FINDMY_DUMP32(str, buf, cnt)        DUMP32(str, buf, cnt)
#else
#define FINDMY_TRACE(attr, str, ...)
#define FINDMY_TRACE_IMM(attr, str, ...)
#define FINDMY_DUMP8(str, buf, cnt)
#define FINDMY_DUMP16(str, buf, cnt)
#define FINDMY_DUMP32(str, buf, cnt)
#endif

#ifdef SW_IIR_TRACE_ENABLE
#define SW_IIR_TRACE(attr, str, ...)         TR_INFO(attr, str, ##__VA_ARGS__)
#define SW_IIR_DEBUG(attr, str, ...)         TR_DEBUG(attr, str, ##__VA_ARGS__)
#define SW_IIR_TRACE_IMM(attr, str, ...)     TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define SW_IIR_DUMP8(str, buf, cnt)          DUMP8(str, buf, cnt)
#define SW_IIR_DUMP16(str, buf, cnt)         DUMP16(str, buf, cnt)
#define SW_IIR_DUMP32(str, buf, cnt)         DUMP32(str, buf, cnt)
#else
#define SW_IIR_TRACE(attr, str, ...)
#define SW_IIR_DEBUG(attr, str, ...)
#define SW_IIR_TRACE_IMM(attr, str, ...)
#define SW_IIR_DUMP8(str, buf, cnt)
#define SW_IIR_DUMP16(str, buf, cnt)
#define SW_IIR_DUMP32(str, buf, cnt)
#endif

#if defined(BESUI_TWS_EN) || defined(BESUI_STEREO_EN)
#ifdef BESUI_TRACE_ENABLE
#define BESUI_TRACE(attr, str, ...)         TR_INFO(attr, str, ##__VA_ARGS__)
#define BESUI_DEBUG(attr, str, ...)         TR_DEBUG(attr, str, ##__VA_ARGS__)
#define BESUI_TRACE_IMM(attr, str, ...)     TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define BESUI_DUMP8(str, buf, cnt)          DUMP8(str, buf, cnt)
#define BESUI_DUMP16(str, buf, cnt)         DUMP16(str, buf, cnt)
#define BESUI_DUMP32(str, buf, cnt)         DUMP32(str, buf, cnt)
#else
#define BESUI_TRACE(attr, str, ...)
#define BESUI_DEBUG(attr, str, ...)
#define BESUI_TRACE_IMM(attr, str, ...)
#define BESUI_DUMP8(str, buf, cnt)
#define BESUI_DUMP16(str, buf, cnt)
#define BESUI_DUMP32(str, buf, cnt)
#endif
#endif

#ifdef HUOSHAN_TRACE_ENABLE
#define HUOSHAN_TRACE(attr, str, ...)       TR_INFO(attr, str, ##__VA_ARGS__)
#define HUOSHAN_TRACE_IMM(attr, str, ...)   TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define HUOSHAN_DUMP8(str, buf, cnt)        DUMP8(str, buf, cnt)
#define HUOSHAN_DUMP16(str, buf, cnt)       DUMP16(str, buf, cnt)
#define HUOSHAN_DUMP32(str, buf, cnt)       DUMP32(str, buf, cnt)
#else
#define HUOSHAN_TRACE(attr, str, ...)
#define HUOSHAN_TRACE_IMM(attr, str, ...)
#define HUOSHAN_DUMP8(str, buf, cnt)
#define HUOSHAN_DUMP16(str, buf, cnt)
#define HUOSHAN_DUMP32(str, buf, cnt)
#endif
