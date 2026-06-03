/***************************************************************************
 *
 * Copyright 2015-2020 BES.
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
#ifndef CHIP_SUBSYS_SENS
#include "plat_addr_map.h"
#include "hal_cmu.h"
#include "hal_sec.h"

#include CHIP_SPECIFIC_HDR(reg_sec)

//these sec regs can only be accessed in security state
static struct HAL_SEC_T *const sec = (struct HAL_SEC_T *)SEC_CTRL_BASE;

static struct HAL_SEC_T *const aon_sec = (struct HAL_SEC_T *)AON_SEC_CTRL_BASE;

static void hal_sec_set_gdma(enum HAL_SEC_TYPE_T sec_type)
{
    if ((sec_type == HAL_SEC_TYPE_NS))
        sec->REG_00C |= (SEC_CFG_NONSEC_GDMA);
    else
        sec->REG_00C &= ~(SEC_CFG_NONSEC_GDMA);
}

static void hal_sec_set_adma(enum HAL_SEC_TYPE_T sec_type)
{
    if (sec_type == HAL_SEC_TYPE_NS)
        sec->REG_00C |= (SEC_CFG_NONSEC_ADMA);
    else
        sec->REG_00C &= ~(SEC_CFG_NONSEC_ADMA);
}

static void hal_sec_set_bcm(enum HAL_SEC_TYPE_T sec_type)
{
    if (sec_type == HAL_SEC_TYPE_NS)
        sec->REG_00C |= SEC_CFG_NONSEC_BCM;
    else
        sec->REG_00C &= ~SEC_CFG_NONSEC_BCM;
}

static void hal_sec_set_usb(enum HAL_SEC_TYPE_T sec_type)
{
    if (sec_type == HAL_SEC_TYPE_NS)
        sec->REG_00C |= SEC_CFG_NONSEC_USB;
    else
        sec->REG_00C &= ~SEC_CFG_NONSEC_USB;
}

static void hal_sec_set_bt2mcu(enum HAL_SEC_TYPE_T sec_type)
{
    if (sec_type == HAL_SEC_TYPE_NS)
        sec->REG_00C |= SEC_CFG_NONSEC_BT2MCU;
    else
        sec->REG_00C &= ~SEC_CFG_NONSEC_BT2MCU;
}

static void hal_sec_set_sens2mcu(enum HAL_SEC_TYPE_T sec_type)
{
    if (sec_type == HAL_SEC_TYPE_NS)
        sec->REG_00C |= SEC_CFG_NONSEC_SENS2MCU;
    else
        sec->REG_00C &= ~SEC_CFG_NONSEC_SENS2MCU;
}

void hal_sec_cfg_nonsec_bypass(bool bypass)
{
    if (bypass)
        sec->REG_004 |= SEC_CFG_NONSEC_BYPASS_MPC_SPINOR0;
    else
        sec->REG_004 &= ~SEC_CFG_NONSEC_BYPASS_MPC_SPINOR0;
}

static void hal_sec_set_i2c_slv(enum HAL_SEC_TYPE_T sec_type)
{
    if (sec_type == HAL_SEC_TYPE_NS)
        aon_sec->REG_000 |= AON_SEC_CFG_NOSEC_I2C_SLV;
    else
        aon_sec->REG_000 &= ~AON_SEC_CFG_NOSEC_I2C_SLV;
}

static void hal_sec_set_sens2aon(enum HAL_SEC_TYPE_T sec_type)
{
    if (sec_type == HAL_SEC_TYPE_NS)
        aon_sec->REG_000 |= AON_SEC_CFG_NOSEC_SENS;
    else
        aon_sec->REG_000 &= ~AON_SEC_CFG_NOSEC_SENS;
}

void hal_sec_set_sram0(enum HAL_SEC_TYPE_T sec_type)
{
    sec->REG_01C &= ~SEC_CFG_SEC2NSEC;

    sec->REG_004 |= SEC_CFG_AP_PROT_SRAM0;
    if (sec_type == HAL_SEC_TYPE_NS)
        sec->REG_004 |= SEC_CFG_NONSEC_PROT_SRAM0;
    else
        sec->REG_004 &= ~SEC_CFG_NONSEC_PROT_SRAM0;
    sec->REG_004 |= SEC_CFG_SEC_RESP_PROT_SRAM0;
    sec->REG_004 &= ~SEC_CFG_NONSEC_BYPASS_PROT_SRAM0;
}

void hal_sec_set_sram1(enum HAL_SEC_TYPE_T sec_type)
{
    sec->REG_01C &= ~SEC_CFG_SEC2NSEC;

    sec->REG_004 |= SEC_CFG_AP_PROT_SRAM1;
    if (sec_type == HAL_SEC_TYPE_NS)
        sec->REG_004 |= SEC_CFG_NONSEC_PROT_SRAM1;
    else
        sec->REG_004 &= ~SEC_CFG_NONSEC_PROT_SRAM1;
    sec->REG_004 |= SEC_CFG_SEC_RESP_PROT_SRAM1;
    sec->REG_004 &= ~SEC_CFG_NONSEC_BYPASS_PROT_SRAM1;
}

void hal_sec_set_sram2_7(int ram_id, enum HAL_SEC_TYPE_T sec_type)
{
    uint32_t val;
    ram_id = (ram_id-2)*4;

    sec->REG_01C &= ~SEC_CFG_SEC2NSEC;

    val = sec->REG_000;
    val |= (SEC_CFG_AP_PROT_SRAM2>>ram_id);
    if (sec_type == HAL_SEC_TYPE_NS)
        val |= (SEC_CFG_NONSEC_PROT_SRAM2>>ram_id);
    else
        val &= ~(SEC_CFG_NONSEC_PROT_SRAM2>>ram_id);
    val |= (SEC_CFG_SEC_RESP_PROT_SRAM2>>ram_id);
    val &= ~((uint32_t)SEC_CFG_NONSEC_BYPASS_PROT_SRAM2>>ram_id);
    sec->REG_000 = val;
}

void hal_sec_set_sram2(enum HAL_SEC_TYPE_T sec_type)
{
    hal_sec_set_sram2_7(2, sec_type);
}

void hal_sec_set_sram3(enum HAL_SEC_TYPE_T sec_type)
{
    hal_sec_set_sram2_7(3, sec_type);
}

void hal_sec_set_sram4(enum HAL_SEC_TYPE_T sec_type)
{
    hal_sec_set_sram2_7(4, sec_type);
}

void hal_sec_set_sram5(enum HAL_SEC_TYPE_T sec_type)
{
    hal_sec_set_sram2_7(5, sec_type);
}

void hal_sec_set_sram6(enum HAL_SEC_TYPE_T sec_type)
{
    hal_sec_set_sram2_7(6, sec_type);
}

void hal_sec_set_sram7(enum HAL_SEC_TYPE_T sec_type)
{
    hal_sec_set_sram2_7(7, sec_type);
}

void hal_sec_set_sram8(enum HAL_SEC_TYPE_T sec_type)
{
    sec->REG_01C &= ~SEC_CFG_SEC2NSEC;

    sec->REG_000 |= SEC_CFG_AP_PROT_SRAM8;
    if (sec_type == HAL_SEC_TYPE_NS)
        sec->REG_000 |= SEC_CFG_NONSEC_PROT_SRAM8;
    else
        sec->REG_000 &= ~SEC_CFG_NONSEC_PROT_SRAM8;
    sec->REG_000 |= SEC_CFG_SEC_RESP_PROT_SRAM8;
    sec->REG_000 &= ~SEC_CFG_NONSEC_BYPASS_PROT_SRAM8;
}

void hal_sec_set_mcu2sens(enum HAL_SEC_TYPE_T sec_type)
{
    sec->REG_01C &= ~SEC_CFG_SEC2NSEC;

    sec->REG_000 |= SEC_CFG_AP_PROT_MCU2SENS;
    if (sec_type == HAL_SEC_TYPE_NS)
        sec->REG_000 |= SEC_CFG_NONSEC_PROT_MCU2SENS;
    else
        sec->REG_000 &= ~SEC_CFG_NONSEC_PROT_MCU2SENS;
    sec->REG_000 |= SEC_CFG_SEC_RESP_PROT_MCU2SENS;
    sec->REG_000 &= ~SEC_CFG_NONSEC_BYPASS_PROT_MCU2SENS;
}

void hal_sec_idau_enable(bool enable)
{
    if (enable) {
        sec->REG_01C |= SEC_IDAUEN;
    } else {
        sec->REG_01C &= ~SEC_IDAUEN;
    }
}

void hal_sec_init()
{
    hal_cmu_clock_enable(HAL_CMU_MOD_P_TZC);
    hal_cmu_clock_enable(HAL_CMU_AON_A_TZC);
    hal_cmu_reset_clear(HAL_CMU_MOD_P_TZC);
    hal_cmu_reset_clear(HAL_CMU_AON_A_TZC);

    ///TODO: idau doesn't work  as expected now
    //hal_sec_idau_enable(true);

    hal_sec_set_gdma(HAL_SEC_TYPE_NS);
    hal_sec_set_adma(HAL_SEC_TYPE_NS);
    hal_sec_set_bcm(HAL_SEC_TYPE_NS);
    hal_sec_set_usb(HAL_SEC_TYPE_NS);
    hal_sec_set_bt2mcu(HAL_SEC_TYPE_NS);
    hal_sec_set_i2c_slv(HAL_SEC_TYPE_NS);
    hal_sec_set_sens2mcu(HAL_SEC_TYPE_NS);
    hal_sec_set_sens2aon(HAL_SEC_TYPE_NS);
}
#endif
