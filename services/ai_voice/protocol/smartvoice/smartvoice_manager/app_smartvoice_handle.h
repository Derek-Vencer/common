#ifndef __APP_SMARTVOICE_HANDLE_H
#define __APP_SMARTVOICE_HANDLE_H

#undef  UUID_128_ENABLE
#define IS_ENABLE_START_DATA_XFER_STEP			0

#define IS_USE_ENCODED_DATA_SEQN_PROTECTION    	1
#define IS_SMARTVOICE_DEBUG_ON					1
#define IS_FORCE_STOP_VOICE_STREAM				0

#define APP_SMARTVOICE_PATH_TYPE_SPP	(1 << 0)
#define APP_SMARTVOICE_PATH_TYPE_BLE	(1 << 1)
#define APP_SMARTVOICE_PATH_TYPE_HFP	(1 << 2)

#if IS_USE_ENCODED_DATA_SEQN_PROTECTION
#define SMARTVOICE_ENCODED_DATA_SEQN_SIZE	    sizeof(uint8_t)
#else
#define SMARTVOICE_ENCODED_DATA_SEQN_SIZE	    0
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum {
    APP_SMARTVOICE_STATE_IDLE,
    APP_SMARTVOICE_STATE_MIC_STARTING,
    APP_SMARTVOICE_STATE_MIC_STARTED,
    APP_SMARTVOICE_STATE_SPEAKER_STARTING,
    APP_SMARTVOICE_STATE_SPEAKER_STARTED,
    APP_SMARTVOICE_STATE_MAX
};

typedef enum
{
	CLOUD_MUSIC_CONTROL_NEXT = 0,
	CLOUD_MUSIC_CONTROL_PRE,
	CLOUD_MUSIC_CONTROL_PAUSE,
	CLOUD_MUSIC_CONTROL_RESUME
} CLOUD_MUSIC_CONTROL_REQ_E;

typedef enum
{
    CLOUD_PLATFORM_IDLE,
    CLOUD_PLATFORM_BAIDU,
    CLOUD_PLATFORM_ALEXA,
} CLOUD_PLATFORM_E;

#ifdef HUOSHAN_ENABLED
typedef void (*huoshan_connected_callback)(uint8_t connHdl);
typedef void (*huoshan_disconnected_callback)(uint8_t connHdl);
typedef void (*huoshan_rx_callback)(uint8_t connHdl, uint8_t *data, uint16_t len);

typedef struct {
    huoshan_connected_callback connect_handler;
    huoshan_disconnected_callback disconnect_handler;
    huoshan_rx_callback rx_handler;
} huoshan_callbacks_t;

void app_sv_huoshan_register_callback(huoshan_callbacks_t *cbs);
void app_sv_huoshan_rec_handler(uint8_t connHdl, uint8_t *data, uint32_t len);
bool app_sv_huoshan_send_handler(uint8_t connHdl, uint8_t *data, uint32_t dataLen);
uint16_t app_sv_huoshan_get_mtu_handler(uint8_t connHdl);
void app_sv_huoshan_update_mtu(uint8_t connHdl, uint16_t mtu);
uint8_t app_sv_huoshan_is_connected(void);
void app_sv_huoshan_upate_conn_param(uint8_t connHdl, uint32_t minInterval, uint32_t maxInterval,
    uint32_t supervisionTimeout, uint8_t slaveLatency);
uint16_t app_sv_huoshan_get_conn_interval(uint8_t connHdl);
#endif

extern const struct ai_handler_func_table smartvoice_handler_function_table;

void app_sv_start_voice_stream_via_hfp(uint8_t device_id);
uint32_t app_sv_stop_voice_stream(void *param1, uint32_t param2);
void app_sv_start_voice_stream_via_ble(uint8_t device_t);
void app_sv_control_cloud_music_playing(CLOUD_MUSIC_CONTROL_REQ_E req, uint8_t device_id);

void app_smartvoice_switch_hfp_stream_status(uint8_t device_id);
void app_smartvoice_switch_ble_stream_status(uint8_t device_id);
void app_smartvoice_switch_spp_stream_status(uint8_t device_id);

void app_sv_switch_cloud_platform(CLOUD_PLATFORM_E platform, uint8_t device_id);
void app_sv_start_voice_stream(uint8_t device_id, uint32_t readOffset);
void app_sv_stop_stream(uint8_t device_id);

#ifdef __cplusplus
}
#endif

#endif

