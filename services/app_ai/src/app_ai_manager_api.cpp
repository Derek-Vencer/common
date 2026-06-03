#include "cmsis_os.h"
#include "hal_trace.h"
#include "app_ai_if.h"
#include "app_ai_tws.h"
#include "app_ai_manager_api.h"
#include "ai_manager.h"
#include "ai_thread.h"

void app_ai_manager_key_event_handle(APP_KEY_STATUS *status, void *param)
{
    APP_AI_TRACE(1,"%s enter",__func__);
    ai_manager_key_event_handle(status, param);
}

bool app_ai_manager_is_in_multi_ai_mode(void)
{
    bool ret = false;
#ifdef IS_MULTI_AI_ENABLED
    ret = true;
#endif
    return ret;
}

bool app_ai_manager_voicekey_is_enable(void)
{
    bool ret = ai_voicekey_is_enable();
    return ret;
}

void app_ai_manager_voicekey_save_status(bool state)
{
    ai_voicekey_save_status(state);
}

void app_ai_manager_switch_spec(AI_SPEC_TYPE_E ai_spec)
{
    ai_manager_switch_spec(ai_spec);
}

void app_ai_manager_set_current_spec(AI_SPEC_TYPE_E ai_spec)
{
    ai_manager_set_current_spec(ai_spec);
}

uint8_t app_ai_manager_get_current_spec(void)
{
    uint8_t ret = ai_manager_get_current_spec();
    return ret;
}

bool app_ai_manager_is_need_reboot(void)
{
    bool ret = ai_manager_is_need_reboot();
    return ret;
}

void app_ai_manager_enable(bool isEnabled, AI_SPEC_TYPE_E ai_spec)
{
    ai_manager_enable(isEnabled, ai_spec);
}

void app_ai_manager_set_spec_connected_status(AI_SPEC_TYPE_E ai_spec, uint8_t connected)
{
    ai_manager_set_spec_connected_status(ai_spec, connected);
}

int8_t app_ai_manager_get_spec_connected_status(uint8_t ai_spec)
{
    uint8_t ret = ai_manager_get_spec_connected_status(ai_spec);
    return ret;
}

bool app_ai_manager_spec_get_status_is_in_invalid(void)
{
    bool ret = ai_manager_spec_get_status_is_in_invalid();
    return ret;
}

void app_ai_manager_set_spec_update_flag(uint8_t spec, bool onOff)
{
    ai_manager_set_spec_update_flag(spec, onOff);
}

bool app_ai_manager_get_spec_update_flag(void)
{
    bool ret = ai_manager_get_spec_update_flag();
    return ret;
}

void app_ai_manager_spec_update_start_reboot(void)
{
    ai_manager_spec_update_start_reboot();
}

void app_ai_manager_init(void)
{
    ai_manager_init();
}

#ifdef IS_MULTI_AI_ENABLED
void app_ai_manager_gva_onoff_key(APP_KEY_STATUS *status, void *param)
{
    uint8_t current_ai_spec = app_ai_manager_get_current_spec();

    APP_AI_TRACE(2,"%s current_ai_spec %d", __func__, current_ai_spec);
    if (current_ai_spec == AI_SPEC_INIT)
    {
        app_ai_manager_enable(true, AI_SPEC_GSOUND);
    }
    else if(current_ai_spec == AI_SPEC_GSOUND)
    {
        app_ai_manager_enable(false, AI_SPEC_GSOUND);
    }
    else if(current_ai_spec == AI_SPEC_AMA)
    {
        app_ai_manager_switch_spec(AI_SPEC_GSOUND);
    }
    bes_ble_gap_refresh_adv_state(BLE_ADVERTISING_INTERVAL);
}

void app_ai_manager_ama_onoff_key(APP_KEY_STATUS *status, void *param)
{
    uint8_t current_ai_spec = app_ai_manager_get_current_spec();

    APP_AI_TRACE(2,"%s current_ai_spec %d", __func__, current_ai_spec);
    if (current_ai_spec == AI_SPEC_INIT)
    {
        app_ai_manager_enable(true, AI_SPEC_AMA);
    }
    else if(current_ai_spec == AI_SPEC_AMA)
    {
        app_ai_manager_enable(false, AI_SPEC_AMA);
    }
    else if(current_ai_spec == AI_SPEC_GSOUND)
    {
        app_ai_manager_switch_spec(AI_SPEC_AMA);
    }
    bes_ble_gap_refresh_adv_state(BLE_ADVERTISING_INTERVAL);
}
#endif

