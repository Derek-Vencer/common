/***************************************************************************
 *
 * Copyright 2015-2019 BES.
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a database or retrieval system, without prior written
 * permission of BES.
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary information of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/

#include "tota_stream_data_transfer.h"
#include "app_tota_cmd_code.h"
#include "bluetooth_bt_api.h"
#include "app_bt_stream.h"
#include "app_audio.h"
#include "nvrecord_bt.h"
#include "nvrecord_env.h"
#include "app_tota.h"
#include "app_tota_cmd_handler.h"
#include "apps.h"
#include "cmsis_os.h"
// #include "app_ibrt_nvrecord.h"
#include "app_tota_common.h"
#include "app_ibrt_customif_cmd.h"
#include "bts_core_if.h"
#include "app_tws_ibrt_cmd_handler.h"

/**/
static void _custom_cmd_handle(APP_TOTA_CMD_CODE_E funcCode, uint8_t* ptrParam, uint32_t paramLen);

/*-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------*/


static void _custom_cmd_handle(APP_TOTA_CMD_CODE_E funcCode, uint8_t* ptrParam, uint32_t paramLen)
{
    TOTA_V2_TRACE(3,"[%s]: opCode:0x%x, paramLen:%d", __func__, funcCode, paramLen);

    if(funcCode != OP_TOTA_OTA)
    {
        app_tota_send_rsp(funcCode,TOTA_NO_ERROR,NULL,0);
    }
    switch (funcCode)
    {
        case OP_TOTA_FACTORY_RESET:
            TOTA_V2_TRACE(1,"##### custom: factory reset  connect path %d",tota_get_connect_path());
#ifdef BESUI_APP_EN
            if(APP_TOTA_VIA_NOTIFICATION == tota_get_connect_path()){
                ///>send to slave
                tws_ctrl_send_cmd(APP_TWS_CMD_SYNC_TOTA_FACTORY_RESET, NULL, 0);
            }
            nv_record_rebuild(NV_REBUILD_CUSTOMER_ONLY);
#endif
            app_reset();

            break;
        case OP_TOTA_CLEAR_PAIRING_INFO:
            TOTA_V2_TRACE(0,"##### custom: clear pairing info");
            //app_ibrt_nvrecord_delete_all_mobile_record();
            break;
        case OP_TOTA_SHUTDOWM:
            TOTA_V2_TRACE(0,"##### custom: shutdown");
            app_shutdown();
            break;
        case OP_TOTA_REBOOT:
            TOTA_V2_TRACE(0,"##### custom: reboot");
            // TODO:
            break;
#if defined(OTA_OVER_TOTA_ENABLED)
        case OP_TOTA_OTA:
            TOTA_V2_TRACE(0,"##### custom: ota");
            TOTA_V2_TRACE(2,"[OTA]: ota_opCode:0x%x, paramLen:%d", *ptrParam, paramLen);
            app_ota_over_tota_receive_data(ptrParam, paramLen);
            break;
#endif
        default:
            ;
    }
}

#ifdef BESUI_APP_EN
#include "app_tota_general.h"
void app_tota_sync_factory_reset_handle(uint8_t *key, uint16_t length)
{
    TOTA_V2_TRACE(0, "app_tota_sync_factory_reset_handle");
#if defined(BESUI_TWS_EN) || defined(BESUI_STEREO_EN)
    besui_app_clear_all_nvrecord();
#endif
}
#endif

TOTA_COMMAND_TO_ADD(OP_TOTA_FACTORY_RESET, _custom_cmd_handle, false, 0, NULL );
TOTA_COMMAND_TO_ADD(OP_TOTA_CLEAR_PAIRING_INFO, _custom_cmd_handle, false, 0, NULL );
TOTA_COMMAND_TO_ADD(OP_TOTA_SHUTDOWM, _custom_cmd_handle, false, 0, NULL );
TOTA_COMMAND_TO_ADD(OP_TOTA_REBOOT, _custom_cmd_handle, false, 0, NULL );
#if defined(OTA_OVER_TOTA_ENABLED)
TOTA_COMMAND_TO_ADD(OP_TOTA_OTA, _custom_cmd_handle, false, 0, NULL );
#endif
