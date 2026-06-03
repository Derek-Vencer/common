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
#undef MOUDLE
#define MOUDLE APP_BT

#include "bluetooth.h"
#include "bes_me_api.h"

#define VERSION_4_2             0x08
#define VERSION_5_0             0x09
#define VERSION_5_1             0x0A
#define VERSION_5_2             0x0B
#define VERSION_5_3             0x0C
#define VERSION_5_4             0x0D
#define VERSION_6_0             0x0E

#define COMPANY_ID_Intel_Corp   0x0002
#define COMPANY_ID_Qualcomm     0x001D
#define COMPANY_ID_HiSilicon    0x010F

#define BTM_CUSTOM_BONDING_UNSPECIFIED              0xFF
#define BTM_CUSTOM_AUTH_MITM_PROTECT_UNSPECIFIED    0xFF

struct bt_custom_pairing_auth
{
    bt_remver_t remote_version;
    uint8_t bonding_mode;
    uint8_t mitm_protection_required;
};

// * Requirement for add more custom items:
// *    - Just add a new item at the following table.
// * Note that, remote version here does not refer to the version of the remote host but that of the remote controller.
// * This custom configs may be unable to some compatible problems, `cause no way to get the actual host version of certain remote device.
// * It may also influence other devices with the absolutely same controller version, though this case has low probability of occurance.
struct bt_custom_pairing_auth g_bt_custom_pairing_auth_table[] =
{
    // * Description of each item is strongly recommended.
    // * ====== ====== comment description format ====== ======
    // ? [bug_id]: xxxxxx
    // ? [reason]: ... ...
    // * ====== ====== comment description format ====== ======

    // ? [bug_id]: 4271362204
    // ? [reason]: Pop-up Window on PC need to be disabled according to requirement of Harman.
    {
        .remote_version = { .vers = VERSION_4_2, .compid = COMPANY_ID_Intel_Corp, .subvers = 0x0100 },
        .bonding_mode = BTM_GENERAL_BONDING,
        .mitm_protection_required = BTM_AUTH_MITM_PROTECT_NOT_REQUIRED,
    },
    // ? [bug_id]: 5045454266
    // ? [reason]: Only search HFP by SDP when both bonding modes are "no bonding" when Galaxy S22 initiates auth.
    {
        .remote_version = { .vers = VERSION_5_2, .compid = COMPANY_ID_Qualcomm, .subvers = 0x5152 },
        .bonding_mode = BTM_GENERAL_BONDING,
        .mitm_protection_required = BTM_AUTH_MITM_PROTECT_NOT_REQUIRED,
    },
    // ? [bug_id]: none
    // ? [reason]: for test
    {
        .remote_version = { .vers = VERSION_5_1, .compid = COMPANY_ID_HiSilicon, .subvers = 0x1317 },
        .bonding_mode = BTM_CUSTOM_BONDING_UNSPECIFIED,
        .mitm_protection_required = BTM_AUTH_MITM_PROTECT_NOT_REQUIRED,
    },
};

bool app_bt_get_io_capability_for_special_devices(uint16_t connhdl, bt_iocap_requirement_t *p_iocap)
{
    bool is_overrided = false;
    bt_remver_t remver = bes_bt_me_acl_get_remote_version(connhdl);
    struct bt_custom_pairing_auth *p_auth = NULL;
    uint8_t bonding_mode = p_iocap->bonding_mode;
    uint8_t mitm_protection_required = p_iocap->mitm_protection_required;

    for (int i = 0; i < ARRAY_SIZE(g_bt_custom_pairing_auth_table); i++)
    {
        p_auth = &g_bt_custom_pairing_auth_table[i];
        if (!memcmp(&remver, &p_auth->remote_version, sizeof(bt_remver_t)))
        {
            bonding_mode = (p_auth->bonding_mode == BTM_CUSTOM_BONDING_UNSPECIFIED)
                        ? bonding_mode : p_auth->bonding_mode;
            mitm_protection_required =  (p_auth->mitm_protection_required == BTM_CUSTOM_AUTH_MITM_PROTECT_UNSPECIFIED)
                        ? mitm_protection_required : p_auth->mitm_protection_required;
            DEBUG_INFO(0, "%s: bonding(%d=>%d), mitm(%d=>%d)", __func__, p_iocap->bonding_mode, bonding_mode,
                        p_iocap->mitm_protection_required, mitm_protection_required);
            p_iocap->bonding_mode = bonding_mode;
            p_iocap->mitm_protection_required = mitm_protection_required;
            is_overrided = true;
        }
    }

    return is_overrided;
}