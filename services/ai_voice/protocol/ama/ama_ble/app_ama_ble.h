#ifndef APP_AMA_BLE_H_
#define APP_AMA_BLE_H_

/**
 ****************************************************************************************
 * @addtogroup APP
 * @ingroup RICOW
 *
 * @brief Smart Voice Application entry point.
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include <stdint.h>          // Standard Integer Definition

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */
#ifdef __cplusplus
extern "C" {
#endif

/**
 ****************************************************************************************
 * @brief Add a DataPath Server instance in the DB
 ****************************************************************************************
 */
void app_ai_add_ama(void);
void app_ama_gatt_svc_event_report(bes_ble_ai_event_param_t *param);
bool app_ama_ble_send_via_notification(uint8_t* ptrData, uint32_t length, uint8_t ai_index, uint8_t conidx);
void app_ama_ble_send_via_indication(uint8_t* ptrData, uint32_t length, uint8_t ai_index, uint8_t conidx);


#ifdef __cplusplus
	}
#endif


#endif // APP_AMA_VOICE_H_

