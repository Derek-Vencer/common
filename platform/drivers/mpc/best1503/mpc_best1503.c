/***************************************************************************
 *
 * Copyright 2015-2023 BES.
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
#include "plat_addr_map.h"
#include "hal_sec.h"
#include "hal_trace.h"
#include "hal_chipid.h"

#include "mpc_sie200_drv.h"

/* Ranges controlled by this SRAM0_MPC */
static const struct mpc_sie200_memory_range_t MPC_SRAM0_RANGE_S = {
    .base         = RAM0_BASE,
    .limit        = RAM0_BASE + RAM_S_SIZE + RAM_NSC_SIZE - 1,
    .range_offset = 0,
    .attr         = MPC_SIE200_SEC_ATTR_SECURE,
};

static const struct mpc_sie200_memory_range_t MPC_SRAM0_RANGE_NS = {
    .base         = RAM0_BASE + RAM_S_SIZE,
    .limit        = RAM1_BASE - 1,
    .range_offset = RAM_S_SIZE,
    .attr         = MPC_SIE200_SEC_ATTR_NONSECURE,
};

#define MPC_SRAM0_RANGE_LIST_LEN  2u
static const struct mpc_sie200_memory_range_t* MPC_SRAM0_RANGE_LIST[MPC_SRAM0_RANGE_LIST_LEN] = {
    &MPC_SRAM0_RANGE_S,
    &MPC_SRAM0_RANGE_NS
};

static const struct mpc_sie200_dev_cfg_t MPC_SRAM0_DEV_CFG = {
    .base = MPC_SRAM0_BASE,
};
static struct mpc_sie200_dev_data_t MPC_SRAM0_DEV_DATA = {
    .range_list = 0,
    .nbr_of_ranges = 0,
    .state = 0,
    .reserved = 0,
};

struct mpc_sie200_dev_t MPC_SRAM0_DEV = {
    &(MPC_SRAM0_DEV_CFG),
    &(MPC_SRAM0_DEV_DATA),
};

/* Ranges controlled by this SRAM1_MPC */
static const struct mpc_sie200_memory_range_t MPC_SRAM1_RANGE_NS = {
    .base         = RAM1_BASE,
    .limit        = RAM2_BASE - 1,
    .range_offset = 0,
    .attr         = MPC_SIE200_SEC_ATTR_NONSECURE,
};

#define MPC_SRAM1_RANGE_LIST_LEN  1u
static const struct mpc_sie200_memory_range_t* MPC_SRAM1_RANGE_LIST[MPC_SRAM1_RANGE_LIST_LEN] = {
    &MPC_SRAM1_RANGE_NS
};

static const struct mpc_sie200_dev_cfg_t MPC_SRAM1_DEV_CFG = {
    .base = MPC_SRAM1_BASE,
};
static struct mpc_sie200_dev_data_t MPC_SRAM1_DEV_DATA = {
    .range_list = 0,
    .nbr_of_ranges = 0,
    .state = 0,
    .reserved = 0,
};

struct mpc_sie200_dev_t MPC_SRAM1_DEV = {
    &(MPC_SRAM1_DEV_CFG),
    &(MPC_SRAM1_DEV_DATA),
};

/* Ranges controlled by this SRAM2_MPC */
static const struct mpc_sie200_memory_range_t MPC_SRAM2_RANGE_NS = {
    .base         = RAM2_BASE,
    .limit        = RAM3_BASE - 1,
    .range_offset = 0,
    .attr         = MPC_SIE200_SEC_ATTR_NONSECURE,
};

#define MPC_SRAM2_RANGE_LIST_LEN  1u
static const struct mpc_sie200_memory_range_t* MPC_SRAM2_RANGE_LIST[MPC_SRAM2_RANGE_LIST_LEN] = {
    &MPC_SRAM2_RANGE_NS
};

static const struct mpc_sie200_dev_cfg_t MPC_SRAM2_DEV_CFG = {
    .base = MPC_SRAM2_BASE,
};
static struct mpc_sie200_dev_data_t MPC_SRAM2_DEV_DATA = {
    .range_list = 0,
    .nbr_of_ranges = 0,
    .state = 0,
    .reserved = 0,
};

struct mpc_sie200_dev_t MPC_SRAM2_DEV = {
    &(MPC_SRAM2_DEV_CFG),
    &(MPC_SRAM2_DEV_DATA),
};

/* Ranges controlled by this SRAM3_MPC */
static const struct mpc_sie200_memory_range_t MPC_SRAM3_RANGE_NS = {
    .base         = RAM3_BASE,
    .limit        = RAM4_BASE - 1,
    .range_offset = 0,
    .attr         = MPC_SIE200_SEC_ATTR_NONSECURE,
};

#define MPC_SRAM3_RANGE_LIST_LEN  1u
static const struct mpc_sie200_memory_range_t* MPC_SRAM3_RANGE_LIST[MPC_SRAM3_RANGE_LIST_LEN] = {
    &MPC_SRAM3_RANGE_NS
};

static const struct mpc_sie200_dev_cfg_t MPC_SRAM3_DEV_CFG = {
    .base = MPC_SRAM3_BASE,
};
static struct mpc_sie200_dev_data_t MPC_SRAM3_DEV_DATA = {
    .range_list = 0,
    .nbr_of_ranges = 0,
    .state = 0,
    .reserved = 0,
};

struct mpc_sie200_dev_t MPC_SRAM3_DEV = {
    &(MPC_SRAM3_DEV_CFG),
    &(MPC_SRAM3_DEV_DATA),
};

/* Ranges controlled by this SRAM4_MPC */
static const struct mpc_sie200_memory_range_t MPC_SRAM4_RANGE_NS = {
    .base         = RAM4_BASE,
    .limit        = RAM5_BASE - 1,
    .range_offset = 0,
    .attr         = MPC_SIE200_SEC_ATTR_NONSECURE,
};

#define MPC_SRAM4_RANGE_LIST_LEN  1u
static const struct mpc_sie200_memory_range_t* MPC_SRAM4_RANGE_LIST[MPC_SRAM4_RANGE_LIST_LEN] = {
    &MPC_SRAM4_RANGE_NS
};

static const struct mpc_sie200_dev_cfg_t MPC_SRAM4_DEV_CFG = {
    .base = MPC_SRAM4_BASE,
};
static struct mpc_sie200_dev_data_t MPC_SRAM4_DEV_DATA = {
    .range_list = 0,
    .nbr_of_ranges = 0,
    .state = 0,
    .reserved = 0,
};

struct mpc_sie200_dev_t MPC_SRAM4_DEV = {
    &(MPC_SRAM4_DEV_CFG),
    &(MPC_SRAM4_DEV_DATA),
};

/* Ranges controlled by this SRAM5_MPC */
static const struct mpc_sie200_memory_range_t MPC_SRAM5_RANGE_NS = {
    .base         = RAM5_BASE,
    .limit        = RAM6_BASE - 1,
    .range_offset = 0,
    .attr         = MPC_SIE200_SEC_ATTR_NONSECURE,
};

#define MPC_SRAM5_RANGE_LIST_LEN  1u
static const struct mpc_sie200_memory_range_t* MPC_SRAM5_RANGE_LIST[MPC_SRAM5_RANGE_LIST_LEN] = {
    &MPC_SRAM5_RANGE_NS
};

static const struct mpc_sie200_dev_cfg_t MPC_SRAM5_DEV_CFG = {
    .base = MPC_SRAM5_BASE,
};
static struct mpc_sie200_dev_data_t MPC_SRAM5_DEV_DATA = {
    .range_list = 0,
    .nbr_of_ranges = 0,
    .state = 0,
    .reserved = 0,
};

struct mpc_sie200_dev_t MPC_SRAM5_DEV = {
    &(MPC_SRAM5_DEV_CFG),
    &(MPC_SRAM5_DEV_DATA),
};

/* Ranges controlled by this SRAM6_MPC */
static const struct mpc_sie200_memory_range_t MPC_SRAM6_RANGE_NS = {
    .base         = RAM6_BASE,
    .limit        = RAM6_BASE + RAM6_SIZE - 1,
    .range_offset = 0,
    .attr         = MPC_SIE200_SEC_ATTR_NONSECURE,
};

#define MPC_SRAM6_RANGE_LIST_LEN  1u
static const struct mpc_sie200_memory_range_t* MPC_SRAM6_RANGE_LIST[MPC_SRAM6_RANGE_LIST_LEN] = {
    &MPC_SRAM6_RANGE_NS
};

static const struct mpc_sie200_dev_cfg_t MPC_SRAM6_DEV_CFG = {
    .base = MPC_SRAM6_BASE,
};
static struct mpc_sie200_dev_data_t MPC_SRAM6_DEV_DATA = {
    .range_list = 0,
    .nbr_of_ranges = 0,
    .state = 0,
    .reserved = 0,
};

struct mpc_sie200_dev_t MPC_SRAM6_DEV = {
    &(MPC_SRAM6_DEV_CFG),
    &(MPC_SRAM6_DEV_DATA),
};

/* Ranges controlled by this PSRAM_MPC */
static const struct mpc_sie200_memory_range_t MPC_PSRAM_RANGE_NS = {
    .base         = PSRAM_BASE,
    .limit        = PSRAM_BASE + PSRAM_SIZE - 1,
    .range_offset = 0,
    .attr         = MPC_SIE200_SEC_ATTR_NONSECURE,
};
#define PSRAM_MPC_VALID_ADDR_BIT 26
#define PSRAM_MPC_BLOCK_SIZE 0x40000
#define PSRAM_MPC_ALIGNED_SIZE(addr) (addr&(~(PSRAM_MPC_BLOCK_SIZE-1)))
#define PSRAM_MPC_OFFSET(addr) PSRAM_MPC_ALIGNED_SIZE(addr)&((1<<PSRAM_MPC_VALID_ADDR_BIT)-1)
static const struct mpc_sie200_memory_range_t MPC_PSRAM_RANGE_DCACHE = {
    .base         = PSRAM_MPC_ALIGNED_SIZE(DCACHE_CTRL_BASE),
    .limit        = PSRAM_MPC_ALIGNED_SIZE(DCACHE_CTRL_BASE) + PSRAM_MPC_BLOCK_SIZE - 1,
    .range_offset = PSRAM_MPC_OFFSET(DCACHE_CTRL_BASE),
    .attr         = MPC_SIE200_SEC_ATTR_NONSECURE,
};

#define MPC_PSRAM_RANGE_LIST_LEN  2u
static const struct mpc_sie200_memory_range_t* MPC_PSRAM_RANGE_LIST[MPC_PSRAM_RANGE_LIST_LEN] = {
    &MPC_PSRAM_RANGE_NS,
    &MPC_PSRAM_RANGE_DCACHE,
};

static const struct mpc_sie200_dev_cfg_t MPC_PSRAM_DEV_CFG = {
    .base = MPC_PSRAM0_BASE,
};
static struct mpc_sie200_dev_data_t MPC_PSRAM_DEV_DATA = {
    .range_list = 0,
    .nbr_of_ranges = 0,
    .state = 0,
    .reserved = 0,
};

struct mpc_sie200_dev_t MPC_PSRAM_DEV = {
    &(MPC_PSRAM_DEV_CFG),
    &(MPC_PSRAM_DEV_DATA),
};

/* Ranges controlled by this FLASH0_MPC */
static const struct mpc_sie200_memory_range_t MPC_FLASH_RANGE_S = {
    .base         = FLASH_BASE,
    .limit        = FLASH_BASE + FLASH_S_SIZE - 1,
    .range_offset = 0,
    .attr         = MPC_SIE200_SEC_ATTR_SECURE,
};

static const struct mpc_sie200_memory_range_t MPC_FLASH_RANGE_NS = {
    .base         = FLASH_BASE + FLASH_S_SIZE,
    .limit        = FLASH_BASE + FLASH_SIZE - 1,
    .range_offset = FLASH_S_SIZE,
    .attr         = MPC_SIE200_SEC_ATTR_NONSECURE,
};
#define FLASH_MPC_VALID_ADDR_BIT 26
#define FLASH_MPC_BLOCK_SIZE 0x40000
#define FLASH_MPC_ALIGNED_SIZE(addr) (addr&(~(FLASH_MPC_BLOCK_SIZE-1)))
#define FLASH_MPC_OFFSET(addr) FLASH_MPC_ALIGNED_SIZE(addr)&((1<<FLASH_MPC_VALID_ADDR_BIT)-1)
static const struct mpc_sie200_memory_range_t MPC_FLASH_RANGE_ICACHE = {
    .base         = FLASH_MPC_ALIGNED_SIZE(ICACHE_CTRL_BASE),
    .limit        = FLASH_MPC_ALIGNED_SIZE(ICACHE_CTRL_BASE) + FLASH_MPC_BLOCK_SIZE - 1,
    .range_offset = FLASH_MPC_OFFSET(ICACHE_CTRL_BASE),
    .attr         = MPC_SIE200_SEC_ATTR_NONSECURE,
};

#define MPC_FLASH_RANGE_LIST_LEN  3u
static const struct mpc_sie200_memory_range_t* MPC_FLASH_RANGE_LIST[MPC_FLASH_RANGE_LIST_LEN] = {
    &MPC_FLASH_RANGE_S,
    &MPC_FLASH_RANGE_NS,
    &MPC_FLASH_RANGE_ICACHE,
};

static const struct mpc_sie200_dev_cfg_t MPC_FLASH0_DEV_CFG = {
    .base = MPC_FLASH0_BASE,
};
static struct mpc_sie200_dev_data_t MPC_FLASH0_DEV_DATA = {
    .range_list = 0,
    .nbr_of_ranges = 0,
    .state = 0,
    .reserved = 0,
};

struct mpc_sie200_dev_t MPC_FLASH0_DEV = {
    &(MPC_FLASH0_DEV_CFG),
    &(MPC_FLASH0_DEV_DATA),
};

int mpc_init(void)
{
    enum mpc_sie200_error_t ret;

    // init SRAM0
    ret = mpc_sie200_init(&MPC_SRAM0_DEV,
                          MPC_SRAM0_RANGE_LIST,
                          MPC_SRAM0_RANGE_LIST_LEN);
    if (ret != MPC_SIE200_ERR_NONE) {
        DRIVERS_TRACE(0, "%s:%d, sie200 init fail. ret:%d", __FUNCTION__, __LINE__, ret);
        return ret;
    }
    mpc_sie200_set_ctrl(&MPC_SRAM0_DEV, 0x10);

    if((RAM0_BASE + RAM_S_SIZE) < RAM1_BASE) {
        ret = mpc_sie200_config_region(&MPC_SRAM0_DEV,
            RAM0_BASE+RAM_S_SIZE+RAM_NSC_SIZE, RAM1_BASE-1, MPC_SIE200_SEC_ATTR_NONSECURE);
        if (ret != MPC_SIE200_ERR_NONE) {
            DRIVERS_TRACE(0, "%s:%d, sie200 config fail. ret:%d", __FUNCTION__, __LINE__, ret);
            return ret;
        }
    }
    mpc_sie200_irq_disable(&MPC_SRAM0_DEV);
    mpc_sie200_lock_down(&MPC_SRAM0_DEV);
    //hal_sec_cfg_nonsec_bypass(SPY_SRAM0_BASE, false);

    // init SRAM1
    ret = mpc_sie200_init(&MPC_SRAM1_DEV,
                          MPC_SRAM1_RANGE_LIST,
                          MPC_SRAM1_RANGE_LIST_LEN);
    if (ret != MPC_SIE200_ERR_NONE) {
        DRIVERS_TRACE(0, "%s:%d, sie200 init fail. ret:%d", __FUNCTION__, __LINE__, ret);
        return ret;
    }
    mpc_sie200_set_ctrl(&MPC_SRAM1_DEV, 0x10);

    ret = mpc_sie200_config_region(&MPC_SRAM1_DEV,
        RAM1_BASE, RAM2_BASE - 1, MPC_SIE200_SEC_ATTR_NONSECURE);
    if (ret != MPC_SIE200_ERR_NONE) {
        DRIVERS_TRACE(0, "%s:%d, sie200 config fail. ret:%d", __FUNCTION__, __LINE__, ret);
        return ret;
    }
    mpc_sie200_irq_disable(&MPC_SRAM1_DEV);
    mpc_sie200_lock_down(&MPC_SRAM1_DEV);
    //hal_sec_cfg_nonsec_bypass(SPY_SRAM1_BASE, false);

    // init SRAM2
    ret = mpc_sie200_init(&MPC_SRAM2_DEV,
                          MPC_SRAM2_RANGE_LIST,
                          MPC_SRAM2_RANGE_LIST_LEN);
     if (ret != MPC_SIE200_ERR_NONE) {
        DRIVERS_TRACE(0, "%s:%d, sie200 init fail. ret:%d", __FUNCTION__, __LINE__, ret);
        return ret;
    }
    mpc_sie200_set_ctrl(&MPC_SRAM2_DEV, 0x10);

    ret = mpc_sie200_config_region(&MPC_SRAM2_DEV,
        RAM2_BASE, RAM3_BASE - 1, MPC_SIE200_SEC_ATTR_NONSECURE);
    if (ret != MPC_SIE200_ERR_NONE) {
        DRIVERS_TRACE(0, "%s:%d, sie200 config fail. ret:%d", __FUNCTION__, __LINE__, ret);
        return ret;
    }
    mpc_sie200_irq_disable(&MPC_SRAM2_DEV);
    mpc_sie200_lock_down(&MPC_SRAM2_DEV);
    //hal_sec_cfg_nonsec_bypass(SPY_SRAM2_BASE, false);

    // init SRAM3
    ret = mpc_sie200_init(&MPC_SRAM3_DEV,
                          MPC_SRAM3_RANGE_LIST,
                          MPC_SRAM3_RANGE_LIST_LEN);
    if (ret != MPC_SIE200_ERR_NONE) {
        DRIVERS_TRACE(0, "%s:%d, sie200 init fail. ret:%d", __FUNCTION__, __LINE__, ret);
        return ret;
    }
    mpc_sie200_set_ctrl(&MPC_SRAM3_DEV, 0x10);

    ret = mpc_sie200_config_region(&MPC_SRAM3_DEV,
        RAM3_BASE, RAM4_BASE - 1, MPC_SIE200_SEC_ATTR_NONSECURE);
    if (ret != MPC_SIE200_ERR_NONE) {
        DRIVERS_TRACE(0, "%s:%d, sie200 config fail. ret:%d", __FUNCTION__, __LINE__, ret);
        return ret;
    }
    mpc_sie200_irq_disable(&MPC_SRAM3_DEV);
    mpc_sie200_lock_down(&MPC_SRAM3_DEV);
    //hal_sec_cfg_nonsec_bypass(SPY_SRAM3_BASE, false);

    // init SRAM4
    ret = mpc_sie200_init(&MPC_SRAM4_DEV,
                          MPC_SRAM4_RANGE_LIST,
                          MPC_SRAM4_RANGE_LIST_LEN);
    if (ret != MPC_SIE200_ERR_NONE) {
        DRIVERS_TRACE(0, "%s:%d, sie200 init fail. ret:%d", __FUNCTION__, __LINE__, ret);
        return ret;
    }
    mpc_sie200_set_ctrl(&MPC_SRAM4_DEV, 0x10);

    ret = mpc_sie200_config_region(&MPC_SRAM4_DEV,
        RAM4_BASE, RAM5_BASE - 1, MPC_SIE200_SEC_ATTR_NONSECURE);
    if (ret != MPC_SIE200_ERR_NONE) {
        DRIVERS_TRACE(0, "%s:%d, sie200 config fail. ret:%d", __FUNCTION__, __LINE__, ret);
        return ret;
    }
    mpc_sie200_irq_disable(&MPC_SRAM4_DEV);
    mpc_sie200_lock_down(&MPC_SRAM4_DEV);
    //hal_sec_cfg_nonsec_bypass(SPY_SRAM4_BASE, false);

    // init SRAM5
    ret = mpc_sie200_init(&MPC_SRAM5_DEV,
                          MPC_SRAM5_RANGE_LIST,
                          MPC_SRAM5_RANGE_LIST_LEN);
    if (ret != MPC_SIE200_ERR_NONE) {
        DRIVERS_TRACE(0, "%s:%d, sie200 init fail. ret:%d", __FUNCTION__, __LINE__, ret);
        return ret;
    }
    mpc_sie200_set_ctrl(&MPC_SRAM5_DEV, 0x10);

    ret = mpc_sie200_config_region(&MPC_SRAM5_DEV,
        RAM5_BASE, RAM6_BASE - 1, MPC_SIE200_SEC_ATTR_NONSECURE);
    if (ret != MPC_SIE200_ERR_NONE) {
        DRIVERS_TRACE(0, "%s:%d, sie200 config fail. ret:%d", __FUNCTION__, __LINE__, ret);
        return ret;
    }
    mpc_sie200_irq_disable(&MPC_SRAM5_DEV);
    mpc_sie200_lock_down(&MPC_SRAM5_DEV);
    //hal_sec_cfg_nonsec_bypass(SPY_SRAM4_BASE, false);

    // init SRAM6
    ret = mpc_sie200_init(&MPC_SRAM6_DEV,
                          MPC_SRAM6_RANGE_LIST,
                          MPC_SRAM6_RANGE_LIST_LEN);
    if (ret != MPC_SIE200_ERR_NONE) {
        DRIVERS_TRACE(0, "%s:%d, sie200 init fail. ret:%d", __FUNCTION__, __LINE__, ret);
        return ret;
    }
    mpc_sie200_set_ctrl(&MPC_SRAM6_DEV, 0x10);

    ret = mpc_sie200_config_region(&MPC_SRAM6_DEV,
        RAM6_BASE, RAM6_BASE + RAM6_SIZE - 1, MPC_SIE200_SEC_ATTR_NONSECURE);
    if (ret != MPC_SIE200_ERR_NONE) {
        DRIVERS_TRACE(0, "%s:%d, sie200 config fail. ret:%d", __FUNCTION__, __LINE__, ret);
        return ret;
    }
    mpc_sie200_irq_disable(&MPC_SRAM6_DEV);
    mpc_sie200_lock_down(&MPC_SRAM6_DEV);
    //hal_sec_cfg_nonsec_bypass(SPY_SRAM4_BASE, false);

    // init PSRAM
    ret = mpc_sie200_init(&MPC_PSRAM_DEV,
                          MPC_PSRAM_RANGE_LIST,
                          MPC_PSRAM_RANGE_LIST_LEN);
    if (ret != MPC_SIE200_ERR_NONE) {
        DRIVERS_TRACE(0, "%s:%d, sie200 init fail. ret:%d", __FUNCTION__, __LINE__, ret);
        return ret;
    }
    mpc_sie200_set_ctrl(&MPC_PSRAM_DEV, 0x10);
    ret = mpc_sie200_config_region(&MPC_PSRAM_DEV,
        PSRAM_BASE,
        PSRAM_BASE + PSRAM_SIZE - 1, MPC_SIE200_SEC_ATTR_NONSECURE);
    if (ret != MPC_SIE200_ERR_NONE) {
        DRIVERS_TRACE(0, "%s:%d, sie200 config fail. ret:%d", __FUNCTION__, __LINE__, ret);
        return ret;
    }

    ret = mpc_sie200_config_region(&MPC_PSRAM_DEV,
        PSRAM_MPC_ALIGNED_SIZE(DCACHE_CTRL_BASE), PSRAM_MPC_ALIGNED_SIZE(DCACHE_CTRL_BASE) + PSRAM_MPC_BLOCK_SIZE - 1,
        MPC_SIE200_SEC_ATTR_NONSECURE);
    if (ret != MPC_SIE200_ERR_NONE) {
        DRIVERS_TRACE(0, "%s:%d, sie200 config fail. ret:%d", __FUNCTION__, __LINE__, ret);
        return ret;
    }
    mpc_sie200_irq_disable(&MPC_PSRAM_DEV);
    mpc_sie200_lock_down(&MPC_PSRAM_DEV);
    //hal_sec_cfg_nonsec_bypass(SPY_PSRAM0_BASE, false);

    // init FLASH0
    ret = mpc_sie200_init(&MPC_FLASH0_DEV,
                          MPC_FLASH_RANGE_LIST,
                          MPC_FLASH_RANGE_LIST_LEN);
    if (ret != MPC_SIE200_ERR_NONE) {
        DRIVERS_TRACE(0, "%s:%d, sie200 init fail. ret:%d", __FUNCTION__, __LINE__, ret);
        return ret;
    }
    mpc_sie200_set_ctrl(&MPC_FLASH0_DEV, 0x10);

    ret = mpc_sie200_config_region(&MPC_FLASH0_DEV,
        FLASH_BASE + FLASH_S_SIZE,
        FLASH_BASE + FLASH_SIZE - 1, MPC_SIE200_SEC_ATTR_NONSECURE);
    if (ret != MPC_SIE200_ERR_NONE) {
        DRIVERS_TRACE(0, "%s:%d, sie200 config fail. ret:%d", __FUNCTION__, __LINE__, ret);
        return ret;
    }
    ret = mpc_sie200_config_region(&MPC_FLASH0_DEV,
        FLASH_MPC_ALIGNED_SIZE(ICACHE_CTRL_BASE), FLASH_MPC_ALIGNED_SIZE(ICACHE_CTRL_BASE) + FLASH_MPC_BLOCK_SIZE - 1,
        MPC_SIE200_SEC_ATTR_NONSECURE);
    if (ret != MPC_SIE200_ERR_NONE) {
        DRIVERS_TRACE(0, "%s:%d, sie200 config fail. ret:%d", __FUNCTION__, __LINE__, ret);
        return ret;
    }
    mpc_sie200_irq_disable(&MPC_FLASH0_DEV);
    mpc_sie200_lock_down(&MPC_FLASH0_DEV);
    hal_sec_cfg_nonsec_bypass(false);

    DRIVERS_TRACE(0, "MPC init successfully");
    return 0;
}
