#ifndef __APP_LAUNCH__
#define __APP_LAUNCH__
#include <stdint.h>
#include "adapter_common_type.h"

#ifdef __cplusplus
extern "C" {
#endif

//
// Control session messages (app_launch)
//

#define RequestAppLaunch                        (0xEA02) /* from accessory */

#define RequestAppLaunch_id_AppBundleID         (0)

#define RequestAppLaunch_id_AppLaunchMethod     (1)
#define AppLaunchMethod_id_with_user_alert      (0) // default value
#define AppLaunchMethod_id_without_user_alert   (1)

bool mfi_fea_app_launch(iap2_link_info_t* link_info, uint8_t* app_bundle_id, uint8_t app_bundle_id_len, uint8_t AppLaunchMethod);

#ifdef __cplusplus
}
#endif
#endif // __APP_LAUNCH__