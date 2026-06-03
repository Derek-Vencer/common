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
#include "stdint.h"
#include "stdbool.h"
#include "plat_types.h"
#include "export_fn_rom.h"
#include "hal_trace.h"
#include "hal_cmu.h"
#include "plat_addr_map.h"
#include "pmu.h"
#include "string.h"
#include "stdio.h"

#include "tool_msg.h"
#include "cmsis_nvic.h"
#include "norflash_cfg.h"
#include "hal_norflash.h"
#include "norflash_api.h"
#include "hal_timer.h"
#include "hal_sysfreq.h"

//#include "user_secure_boot.h"

#define USER_SECURE_DEBUG_ENABLE

#ifndef SECURE_BOOT_CHAIN_FLASH_ID
#define SECURE_BOOT_CHAIN_FLASH_ID  (HAL_FLASH_ID_0)
#endif

#ifdef USER_SECURE_DEBUG_ENABLE
#define USER_SECURE_ASSERT ASSERT
#else
#define USER_SECURE_ASSERT(s,...)
#endif

#define SCRATCH_BUF_LEN ((2 * 1024) / 4)

typedef void (*FLASH_ENTRY)(void);
typedef void (*funcptr_void) (void) ;

static uint32_t scratch_buf[SCRATCH_BUF_LEN];
static uint32_t vendor_key[(MAX_KEY_LEN + 3) / 4];


//static user_secure_boot_struct *user_secure_boot_p;
static bool user_secure_boot_valid = false;

static const struct boot_struct_t * flash_boot_struct;
//POSSIBLY_UNUSED static FLASH_ENTRY normal_flash_entry;
#ifdef SECURE_BOOT
static union SECURITY_VALUE_T security_value;
static const struct code_sig_struct_t * flash_code_sig;
static const struct norflash_cfg_struct_t * norflash_cfg;
static const struct code_ver_struct_t * flash_code_ver;
static FLASH_ENTRY secure_flash_entry;

extern uint32_t hal_norflash_get_nc_base(enum HAL_FLASH_ID_T id);
#endif

static int security_get_efuse_setting(void)
{
    int ret = 0;
    union SECURITY_VALUE_T security;
#ifndef FLASH1_CTRL_BASE
    const
#endif
    ret = pmu_get_security_value(&security);
#ifdef SECURE_BOOT
    if (ret) {
        USER_SECURE_ASSERT(0,"%s error ret = %d line %d",__func__,ret,__LINE__);
        return ret;
    }
    security_value.reg = security.reg;
    USER_SECURE_BOOT_TRACE(0,"%s, Efus_Value = 0x%04X",__func__, security_value.reg);
#endif

    return ret;
}

static void security_get_flash_jump_entry_info(uint32_t jump_entry)
{
    flash_boot_struct = (struct boot_struct_t *)jump_entry;
//    normal_flash_entry = (FLASH_ENTRY)((uint32_t)(&flash_boot_struct->hdr + 1) | 1);
#ifdef SECURE_BOOT
    flash_code_sig = (struct code_sig_struct_t *)(flash_boot_struct + 1);
    norflash_cfg = (struct norflash_cfg_struct_t *)(flash_code_sig + 1);
    flash_code_ver = (struct code_ver_struct_t *)((uint32_t)(flash_code_sig + 1) + sizeof(struct norflash_cfg_struct_t));
    secure_flash_entry = (FLASH_ENTRY)((uint32_t)(flash_code_ver + 1) | 1);
#endif
}

static int security_boot_struct_jump_entry_check(void)
{
#ifdef SECURE_BOOT
    struct boot_hdr_t hdr;
    struct boot_struct_t *boot;
//    unsigned short ver;

    boot = (struct boot_struct_t *)flash_boot_struct;

    memcpy(&hdr, &boot->hdr, sizeof(hdr));

    if (hdr.magic != BOOT_MAGIC_NUMBER) {
        return -1;
    }
    if ((hdr.flag & BOOT_STRUCT_FLAG_SECURE_BOOT) == 0) {
        return -2;
    }

    if (flash_code_sig->code_size < MIN_SIGNED_CODE_SIZE) {
        return -3;
    }

    if (flash_code_sig->code_size < sizeof(*norflash_cfg) + MIN_SIGNED_CODE_SIZE) {
        return -4;
    }

#if 0
    if (sec_check_ver) 
    {
        get_code_version_info(&ver);
        if (flash_code_ver->code_version < ver) {
            return -5;
        }
    }
#endif
#endif

    return 0;
}

static int security_get_flash_otp_key_base_addr(uint32_t * addr,uint32_t * lenth)
{
#ifdef SECURE_BOOT
    uint32_t reg_base;
    uint32_t reg_size;
    uint32_t reg_offset;
    uint32_t boot_struct_len;
    uint32_t key_len;

    if (security_value.otp.skip_romkey) {
#if (SECURE_BOOT_VER >= 3) && !defined(SECURE_BOOT_ED25519_DISABLE)
        if (security_value.otp.sig_type == SECURE_BOOT_SIG_ED25519) {
            key_len = ED25519_KEY_LEN;
        } else
#endif
        {
            key_len = RSA_KEY_LEN;
        }
        boot_struct_len = OFFSETOF(struct boot_struct_t, key) + key_len;
    } else {
        boot_struct_len = sizeof(struct boot_struct_t);
    }

    if (security_value.otp.reg_base == SEC_REG_BASE_0X1000) {
        reg_base = 0x1000;
    } else if (security_value.otp.reg_base == SEC_REG_BASE_0X0000) {
        reg_base = 0;
    } else if (security_value.otp.reg_base == SEC_REG_BASE_0X2000) {
        reg_base = 0x2000;
    } else {
        return 1;
    }
    reg_base += hal_norflash_get_nc_base(SECURE_BOOT_CHAIN_FLASH_ID);

    if (security_value.otp.reg_size == SEC_REG_SIZE_1024) {
        reg_size = 1024;
    } else if (security_value.otp.reg_size == SEC_REG_SIZE_512) {
        reg_size = 512;
    } else if (security_value.otp.reg_size == SEC_REG_SIZE_256) {
        reg_size = 256;
    } else if (security_value.otp.reg_size == SEC_REG_SIZE_4096) {
        reg_size = 4096;
    } else {
        return 2;
    }

    if (security_value.otp.reg_offset == SEC_REG_OFFSET_0X1000) {
        reg_offset = 0x1000;
    } else if (security_value.otp.reg_offset == SEC_REG_OFFSET_0X0100) {
        reg_offset = 0x0100;
    } else {
        return 3;
    }

    if (reg_size > reg_offset) {
        return 4;
    }

    *addr = reg_base + reg_offset;
    *lenth = boot_struct_len;

#endif

    return 0;
}

static int security_boot_jump_entry_check(void)
{
#ifdef SECURE_BOOT
    USER_SECURE_BOOT_TRACE(0,"%s enter mode : %d %d",__func__,security_value.root.security_en,security_value.root.mode);
    if ((security_value.root.security_en) &&
    (security_value.root.mode == SECURE_BOOT_MODE_OTP_KEY)){
        uint32_t addr;
        uint32_t len;

        int ret;
        POSSIBLY_UNUSED uint32_t time[2];

        ret = security_get_flash_otp_key_base_addr(&addr,&len);
        if(ret != 0){
            USER_SECURE_ASSERT(0,"%s error ret = %d line %d",__func__,ret,__LINE__);
        }

        addr = 0;//suggestion : secure-boot using key locate in otp offset 0 is better

        hal_norflash_security_register_read(SECURE_BOOT_CHAIN_FLASH_ID,addr,(uint8_t*)&scratch_buf[0],len);

        memcpy(&vendor_key[0], &((struct boot_struct_t *)scratch_buf)->key[0], MAX_KEY_LEN);

        USER_SECURE_BOOT_TRACE(0,"start verify max_key_len = %d code_size %d boot_strcut_len %d",MAX_KEY_LEN,flash_code_sig->code_size,len);
#ifdef CHIP_BEST1306
        USER_SECURE_BOOT_TRACE(0,"sig_type %d verify_func 0x%02x sig_addr 0x%02x verify_addr 0x%02x",security_value.root.sig_type,(uint32_t)export_fn_rom->verify_signature,(uint32_t)&flash_code_sig->sig[0],(uint32_t)(flash_code_sig + 1));
#else
        USER_SECURE_BOOT_TRACE(0,"sig_type %d verify_func 0x%02x sig_addr 0x%02x verify_addr 0x%02x",security_value.root.sig_type,(uint32_t)__export_fn_rom.verify_signature,(uint32_t)&flash_code_sig->sig[0],(uint32_t)(flash_code_sig + 1));
#endif
        time[0] = hal_sys_timer_get();

        USER_SECURE_BOOT_TRACE(0,"type %d len %d",security_value.root.sig_type, flash_code_sig->code_size);
#if (SECURE_BOOT_VER == 1) || (SECURE_BOOT_VER == 2)
        ret = ((__VERIFY_SIGNATURE_V1)__export_fn_rom.verify_signature)((uint8_t *)&vendor_key[0], &flash_code_sig->sig[0], (const uint8_t *)(flash_code_sig + 1), flash_code_sig->code_size);
#else
#ifdef CHIP_BEST1306
        ret = export_fn_rom->verify_signature(0, security_value.root.sig_type, (uint8_t *)&vendor_key[0], &flash_code_sig->sig[0], (const uint8_t *)(flash_code_sig + 1), flash_code_sig->code_size);
#else
        ret = __export_fn_rom.verify_signature(0, security_value.root.sig_type, (uint8_t *)&vendor_key[0], &flash_code_sig->sig[0], (const uint8_t *)(flash_code_sig + 1), flash_code_sig->code_size);
#endif
#endif

        if (ret) {
            // USER_SECURE_ASSERT(0,"%s error ret = %d line %d",__func__,ret,__LINE__);
            return -1;
        }

        time[1] = hal_sys_timer_get();

        USER_SECURE_BOOT_TRACE(1,"C-V: %u ms\r\n",
            TICKS_TO_MS(time[1] - time[0]));
        }else{
            return -1;
        }
#endif

    return 0;
}

void security_boot_jump_to_next_entry(void)
{
    if(user_secure_boot_valid == false){
        return ;
    }

    funcptr_void ResetHandler;
    uint32_t jump_entry = USER_SECURE_BOOT_JUMP_ENTRY_ADDR;

    jump_entry += hal_norflash_get_nc_base(SECURE_BOOT_CHAIN_FLASH_ID);

#ifdef SECURE_BOOT
    ResetHandler = secure_flash_entry;
#else
    ResetHandler = (funcptr_void)((uint32_t)(&((struct boot_struct_t *)jump_entry)->hdr + 1));
#endif

    // Disable all IRQs
    NVIC_DisableAllIRQs();
#if 0
    // Enable debug i2c to debug error while jumping into flash
    if (debug_i2c_en) {
        hal_iomux_set_analog_i2c();
    }
#endif
    // Jump
    ResetHandler();

}

extern     int  ProgrammerInflashEnterApp_retry(void);

int user_secure_boot_check(void)
{
    if(USER_SECURE_BOOT_JUMP_ENTRY_ADDR == 0xffffffff){
        return -2;
    }

    if(user_secure_boot_valid){
        return 0;
    }

#ifdef ARM_CMSE
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_104M);
#else
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_52M);
#endif

    int ret = 0;
    uint32_t jump_entry = USER_SECURE_BOOT_JUMP_ENTRY_ADDR;
    jump_entry += hal_norflash_get_nc_base(SECURE_BOOT_CHAIN_FLASH_ID);

    USER_SECURE_BOOT_TRACE(1,"%s addr = 0x%02x 0x%02x ",__func__,USER_SECURE_BOOT_JUMP_ENTRY_ADDR,jump_entry);

    security_get_flash_jump_entry_info(jump_entry);

    if((ret = security_boot_struct_jump_entry_check()) != 0){
        USER_SECURE_BOOT_TRACE(0,"security_boot_struct_jump_entry_check error");
        goto error;
    }

    if((ret = security_get_efuse_setting()) != 0){
        USER_SECURE_BOOT_TRACE(0,"security_get_efuse_setting error");
        goto error;
    }

    if((ret = security_boot_jump_entry_check()) != 0){
        USER_SECURE_BOOT_TRACE(0,"security_boot_jump_entry_check error");
        goto error;
    }

    hal_trace_flush_buffer();

    user_secure_boot_valid = true;

    return 0;
error:
    USER_SECURE_BOOT_TRACE(0,"%s secure boot check error ret = %d",__func__,ret );
    hal_trace_flush_buffer();
    pmu_shutdown();

    return -1;
}

void user_secure_boot_init(void)
{
    user_secure_boot_check();
}
// void boot_loader_post_init_hook(void) __attribute__((alias("user_secure_boot_init")));

void user_secure_checker(void)
{
    if(USER_SECURE_BOOT_JUMP_ENTRY_ADDR == 0xffffffff){
        return ;
    }

    USER_SECURE_ASSERT(user_secure_boot_valid, "user_secure !!!");
    if (!user_secure_boot_valid){
        pmu_shutdown();
    }
}


int user_secure_boot_ota_check(uint32_t offset)
{
    int ret = 0;

#ifdef SECURE_BOOT
    uint32_t nc_addr = offset + hal_norflash_get_nc_base(SECURE_BOOT_CHAIN_FLASH_ID);

    USER_SECURE_BOOT_TRACE(1,"%s addr = 0x%02x 0x%02x ",__func__, offset, nc_addr);

    security_get_flash_jump_entry_info(nc_addr);
    if((ret = security_get_efuse_setting()) != 0){
        USER_SECURE_BOOT_TRACE(0,"security_get_efuse_setting error");
        goto error;
    }

    if((ret = security_boot_jump_entry_check()) != 0){
        USER_SECURE_BOOT_TRACE(0,"security_boot_jump_entry_check error");
        goto error;
    }

error:
    return ret;
#endif

    return ret;
}

