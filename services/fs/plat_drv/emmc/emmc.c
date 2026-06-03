/***************************************************************************
 * Copyright 2015-2024 BES.
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
 ***************************************************************************/
#include "plat_addr_map.h"
#include <stdio.h>
#ifdef EMMC0_BASE

#include "card.h"
#include "fs/fs.h"
#include "string.h"
#include "hal_dma.h"
#include "hal_cache.h"
#include "hal_gpio.h"
#include "hal_timer.h"
#include "hal_iomux.h"
#include "hal_trace.h"
#include "emmc_error.h"
#include "emmc_config.h"
#include "fs_cache.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif

#define EMMC_WR_DMA_MODE
#define EMMC_HOST_MAX_NUM 1

#define DEV_RST_PIN     HAL_IOMUX_PIN_P3_6
#define DEV_3V3_PIN     HAL_IOMUX_PIN_PMU_P2_3
#define DEV_1V8_PIN     HAL_IOMUX_PIN_PMU_P2_1
#define PWR_ON_LEVEL    1

#ifdef RTOS
osSemaphoreDef(device_emmc_wait_done);
static osSemaphoreId emmc_wait_done_id;
#endif

#define MAX_EMMC_PART_NUM 4

#ifndef AOS_EMMC_PART_NUM
#define AOS_EMMC_PART_NUM MAX_EMMC_PART_NUM
#endif

#if AOS_EMMC_PART_NUM > MAX_EMMC_PART_NUM
#  error MAX_EMMC_PART_NUM is 4
#endif

#ifndef AOS_EMMCPART1_BLOCK_NUM
#define AOS_EMMCPART1_BLOCK_NUM 102400
#endif

#ifndef AOS_EMMCPART2_BLOCK_NUM
#define AOS_EMMCPART2_BLOCK_NUM 102400
#endif

#ifndef AOS_EMMCPART3_BLOCK_NUM
#define AOS_EMMCPART3_BLOCK_NUM 102400
#endif

#ifndef AOS_EMMCPART4_BLOCK_NUM
#define AOS_EMMCPART4_BLOCK_NUM 102400
#endif

static uint32_t g_emmc_partition[MAX_EMMC_PART_NUM] = {AOS_EMMCPART1_BLOCK_NUM, AOS_EMMCPART2_BLOCK_NUM,
                                                       AOS_EMMCPART3_BLOCK_NUM, AOS_EMMCPART4_BLOCK_NUM
                                                      };

struct fs_emmc_conf {
    int opened;
    struct mmc_host emmc_gloable_op;
    struct mmc_config *emmc_cfg;
    uint8_t id;
    uint32_t ref;
    void *mutex;
#ifdef RTOS
    osSemaphoreId emmc_wait_done_id;
#endif
#ifdef FS_EMMC_CACHE_ENABLE
    struct fs_cache_seg_conf cachecfg;
    struct fs_cache_manager cache;
#endif
};

static struct fs_emmc_conf *g_emmc_priv[EMMC_HOST_MAX_NUM];
ssize_t emmc_card_write_entry(void* priv, const unsigned char *buffer, blkcnt_t start_sector, unsigned int nsectors);
ssize_t emmc_card_read_entry(void* priv, unsigned char *buffer, blkcnt_t start_sector, unsigned int nsectors);

static void fs_emmc_host_error(enum EMMC_HOST_ERR error)
{
    AOS_FS_TRACE(0, FS_ERR_TAG"ret: 0x%d", __FS_ERR_WHERE__, error);
}

#ifdef RTOS
static void fs_emmc_host_dma_done(void)
{
    if (emmc_wait_done_id)
        osSemaphoreRelease(emmc_wait_done_id);
}
#endif

static void fs_emmc_delay(uint32_t ms)
{
    osDelay(ms);
}

static void emmc_pwr_ctrl_init(void)
{
#if defined(DEV_RST_PIN) || defined(DEV_3V3_PIN) || defined(DEV_1V8_PIN)
    struct HAL_IOMUX_PIN_FUNCTION_MAP iomux[] = {
        {HAL_IOMUX_PIN_NUM, HAL_IOMUX_FUNC_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
    };
#endif

#ifdef DEV_RST_PIN
    //emmc reset pin
    if ((enum HAL_IOMUX_PIN_T)(DEV_RST_PIN) < HAL_IOMUX_PIN_NUM) {
        iomux[0].pin = (enum HAL_IOMUX_PIN_T)(DEV_RST_PIN);
        hal_iomux_init(iomux, ARRAY_SIZE(iomux));
    }
    //hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)(DEV_RST_PIN), HAL_GPIO_DIR_OUT, 1);
#endif

#ifdef DEV_3V3_PIN
    //emmc 3.3v power contrl pin
    if ((enum HAL_IOMUX_PIN_T)(DEV_3V3_PIN) < HAL_IOMUX_PIN_NUM) {
        iomux[0].pin = (enum HAL_IOMUX_PIN_T)(DEV_3V3_PIN);
        hal_iomux_init(iomux, ARRAY_SIZE(iomux));
    }
    hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)(DEV_3V3_PIN), HAL_GPIO_DIR_OUT, 0);
#endif

#ifdef DEV_1V8_PIN
    //emmc 1.8v power contrl pin
    if ((enum HAL_IOMUX_PIN_T)(DEV_1V8_PIN) < HAL_IOMUX_PIN_NUM) {
        iomux[0].pin = (enum HAL_IOMUX_PIN_T)(DEV_1V8_PIN);
        hal_iomux_init(iomux, ARRAY_SIZE(iomux));
    }
    hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)(DEV_1V8_PIN), HAL_GPIO_DIR_OUT, 0);
#endif
}

static void emmc_pwr_on_1v8(void)
{
#ifdef DEV_1V8_PIN
    //emmc 1.8v power contrl pin
    if (PWR_ON_LEVEL) {
        hal_gpio_pin_set((enum HAL_GPIO_PIN_T)(DEV_1V8_PIN));
    } else {
        hal_gpio_pin_clr((enum HAL_GPIO_PIN_T)(DEV_1V8_PIN));
    }
    TRACE(0, "%s: done", __FUNCTION__);
#endif
}

static void emmc_pwr_on_3v3(void)
{
#ifdef DEV_3V3_PIN
    //emmc 3.3v power contrl pin
    if (PWR_ON_LEVEL) {
        hal_gpio_pin_set((enum HAL_GPIO_PIN_T)(DEV_3V3_PIN));
    } else {
        hal_gpio_pin_clr((enum HAL_GPIO_PIN_T)(DEV_3V3_PIN));
    }
    TRACE(0, "%s: done", __FUNCTION__);
#endif
}

static void emmc_pwr_off_1v8(void)
{
#ifdef DEV_1V8_PIN
    //emmc 1.8v power contrl pin
    if (PWR_ON_LEVEL) {
        hal_gpio_pin_clr((enum HAL_GPIO_PIN_T)(DEV_1V8_PIN));
    } else {
        hal_gpio_pin_set((enum HAL_GPIO_PIN_T)(DEV_1V8_PIN));
    }
    TRACE(0, "%s: done", __FUNCTION__);
#endif
}

static void emmc_pwr_off_3v3(void)
{
#ifdef DEV_3V3_PIN
    //emmc 3.3v power contrl pin
    if (PWR_ON_LEVEL) {
        hal_gpio_pin_clr((enum HAL_GPIO_PIN_T)(DEV_3V3_PIN));
    } else {
        hal_gpio_pin_set((enum HAL_GPIO_PIN_T)(DEV_3V3_PIN));
    }
    TRACE(0, "%s: done", __FUNCTION__);
#endif
}

static void emmc_pwr_off_other(void)
{
    /*  If the RST pin is enabled, it needs to be pulled low at this time,
        otherwise the power consumption will increase
    */
#ifdef DEV_RST_PIN
    hal_gpio_pin_clr((enum HAL_GPIO_PIN_T)(DEV_RST_PIN));
    TRACE(0, "%s: done", __FUNCTION__);
#endif
}

static struct mmc_config emmc_cfg = {
    /* host configuration */
    .id = MMC_HOST_ID_0,
    .type = DEV_TYPE_EMMC,
    .dma_mode = MM4_SDMA,

    .desc_type = ADMA2_DESC_FIXED_SZ,
    .desc_sz = ADMA2_DESC_MAX_LEN,
    .desc_nr = 1,

    /*host mode*/
    .auto_cmd_type = MM4_AUTO_CMD12, // SDMA does not support AutoCMD23
    // .clk_gating_en = 1,
    .resp_polling_mode = 1,

    /* data transfer */
    .blksz = CARD_BLOCK_LENGTH,

#ifdef EMMC_DDR_MODE
#if defined(EMMC_USE_PLL)
    .timing = HS400,
    .khz = F_HS400,
#else
    .timing = DDR52,
    .khz = F_DDR52,
#endif
#else
#if defined(EMMC_USE_PLL)
    .timing = HS200,
    .khz = F_HS200,
#elif defined(EMMC_HS_MODE)
    .timing = SDR52,
    .khz = F_SDR52,
#else
    .timing = DS,
    .khz = F_DS,
#endif
#endif

    .width = MMC_8BIT_WIDTH,
    /*delay*/
    .ms_delay = fs_emmc_delay,
    /*callback*/
    .emmc_host_error = fs_emmc_host_error,
#ifdef RTOS
    .emmc_host_dma_done = fs_emmc_host_dma_done,
#else
    .emmc_host_dma_done = NULL,
#endif

    /* device power control */
    .dev_pwr_on_time = 10,
    .dev_reset = NULL,
    .dev_pwr_on_1v8 = emmc_pwr_on_1v8,
    .dev_pwr_on_3v3 = emmc_pwr_on_3v3,
    .dev_pwr_off_1v8 = emmc_pwr_off_1v8,
    .dev_pwr_off_3v3 = emmc_pwr_off_3v3,
    .dev_pwr_off_other = emmc_pwr_off_other,
    .dev_pwr_ctrl_init = emmc_pwr_ctrl_init,
};

static int emmc_card_geometry(struct inode *inode, struct geometry *geometry)
{
    int ret;
    struct mmc_card_info card_info;
    struct fs_emmc_conf *conf = (struct fs_emmc_conf *)(inode->i_private);
    ret = get_card_info(&conf->emmc_gloable_op, &card_info);
    if (ret != MMC_NO_ERR)
        return -1;

    geometry->geo_available     = true;
    geometry->geo_mediachanged  = false;
    geometry->geo_writeenabled  = true;
    geometry->geo_nsectors      = card_info.blocknbr;
    geometry->geo_sectorsize    = card_info.blocksize;
    return 0;
}

int emmc_card_open(struct inode *inode)
{
    struct fs_emmc_conf *conf = (struct fs_emmc_conf *)(inode->i_private);
    int ret = MMC_NO_ERR;

    fs_mutex_lock(conf->mutex);
    if (conf->opened == 1) {
        fs_mutex_unlock(conf->mutex);
        return 0;
    }

    struct mmc_host *host = &conf->emmc_gloable_op;
    struct mmc_config *cfg = (struct mmc_config *)conf->emmc_cfg;

    ret = open_card(host, cfg);

    conf->opened = 1;

#ifdef FS_EMMC_CACHE_ENABLE
    struct mmc_card_info card_info;
    ret = get_card_info(&conf->emmc_gloable_op, &card_info);
    ASSERT(ret == MMC_NO_ERR, FS_ERR_TAG"get_card_info failed", __FS_ERR_WHERE__);

    blkcnt_t cache_seg[CONFIG_FS_EMMC_CACHE_SEG_NUM];
    device_read_f readfp = emmc_card_read_entry;
    device_write_f writefp = emmc_card_write_entry;

    CONFIG_FS_EMMC_CACHE_CODE(cache_seg);
    fs_cache_conf_init(&conf->cachecfg, CONFIG_FS_EMMC_CACHE_LEN, CONFIG_FS_EMMC_CACHE_SEG_NUM, cache_seg, readfp, writefp, conf);
    ret = fs_cache_init(&conf->cachecfg, &conf->cache, card_info.blocksize);
    ASSERT(ret == 0, FS_ERR_TAG"fs_cache_init failed", __FS_ERR_WHERE__);
#endif

    AOS_FS_TRACE(1, FS_DEBUG_TAG"emmc %d opened", __FS_ERR_WHERE__, conf->id);
    fs_mutex_unlock(conf->mutex);
    return ret;
}

ssize_t emmc_card_write_entry(void* priv, const unsigned char *buffer, blkcnt_t start_sector, unsigned int nsectors)
{
    struct fs_emmc_conf *conf = (struct fs_emmc_conf *)(priv);
    struct mmc_card_info card_info;
    int ret;

    ret = get_card_info(&conf->emmc_gloable_op, &card_info);
    if (ret != MMC_NO_ERR) {
        return -1;
    }

#if defined(EMMC_WR_DMA_MODE) && (PSRAM_SIZE > 0)
    if ((uintptr_t)buffer >= PSRAM_BASE && (uintptr_t)buffer < (PSRAM_BASE + PSRAM_SIZE)) {
        hal_dma_sync_for_device((const void *)ADDR_ALIGN_TO_CACHE((uintptr_t)buffer),
                            LEN_ALIGN_TO_CACHE(nsectors * card_info.blocksize, (uintptr_t)buffer));
    }
    ret = write_card_blks_dma_mode(&conf->emmc_gloable_op, start_sector, (void *)buffer, nsectors);
    if (ret == 0) {
#ifdef RTOS
        osSemaphoreWait(conf->emmc_wait_done_id, osWaitForever);
#endif
    } else {
        AOS_FS_TRACE(0, FS_ERR_TAG"ret: 0x%d", __FS_ERR_WHERE__, ret);
    }
#else
    ret = write_card_blks_blocking_mode(&conf->emmc_gloable_op, start_sector, (void *)buffer, nsectors);
#endif

    return (ret == MMC_NO_ERR) ? nsectors : -1;
}

ssize_t emmc_card_write(struct inode *inode,
                        const unsigned char *buffer,
                        blkcnt_t start_sector, unsigned int nsectors)
{
    struct fs_emmc_conf *conf = (struct fs_emmc_conf *)(inode->i_private);
    int ret;

    fs_mutex_lock(conf->mutex);
#ifdef FS_EMMC_CACHE_ENABLE
    ret = fs_cache_write(&conf->cache, buffer, start_sector, nsectors);
#else
    ret = emmc_card_write_entry(conf, buffer, start_sector, nsectors);
#endif
    fs_mutex_unlock(conf->mutex);
    return ret;
}

ssize_t emmc_card_read_entry(void* priv, FAR unsigned char *buffer,
                       blkcnt_t start_sector, unsigned int nsectors)
{
    struct fs_emmc_conf *conf = (struct fs_emmc_conf *)(priv);
    struct mmc_card_info card_info;
    int ret;

    ret = get_card_info(&conf->emmc_gloable_op, &card_info);
    if (ret != MMC_NO_ERR) {
        return -1;
    }

#if defined(EMMC_WR_DMA_MODE) && (PSRAM_SIZE > 0)
    if ((uintptr_t)buffer >= PSRAM_BASE && (uintptr_t)buffer < (PSRAM_BASE + PSRAM_SIZE)) {
        hal_dma_sync_for_cpu((const void *)ADDR_ALIGN_TO_CACHE((uintptr_t)buffer),
                            LEN_ALIGN_TO_CACHE(nsectors * card_info.blocksize, (uintptr_t)buffer));
    }
    ret = read_card_blks_dma_mode(&conf->emmc_gloable_op, start_sector, buffer, nsectors);
    if (ret == 0) {
#ifdef RTOS
        osSemaphoreWait(conf->emmc_wait_done_id, osWaitForever);
#endif
    } else {
        AOS_FS_TRACE(0, FS_ERR_TAG"ret: 0x%d", __FS_ERR_WHERE__, ret);
    }
#else
    ret = read_card_blks_blocking_mode(&conf->emmc_gloable_op, start_sector, buffer, nsectors);
#endif

    return (ret == MMC_NO_ERR) ? nsectors : -1;
}

ssize_t emmc_card_read(struct inode *inode, FAR unsigned char *buffer,
                       blkcnt_t start_sector, unsigned int nsectors)
{
    struct fs_emmc_conf *conf = (struct fs_emmc_conf *)(inode->i_private);
    ssize_t ret;

    fs_mutex_lock(conf->mutex);
#ifdef FS_EMMC_CACHE_ENABLE
    ret = fs_cache_read(&conf->cache, buffer, start_sector, nsectors);
#else
    ret = emmc_card_read_entry(conf, buffer, start_sector, nsectors);
#endif
    fs_mutex_unlock(conf->mutex);
    return ret;
}

int emmc_card_erase()
{
    return 0;
}

int emmc_card_close(struct inode *inode)
{
    struct fs_emmc_conf *conf = (struct fs_emmc_conf *)(inode->i_private);

    fs_mutex_lock(conf->mutex);
    AOS_FS_TRACE(1, FS_DEBUG_TAG"priv=%p priv->ref=%d inode=%p", __FS_ERR_WHERE__, conf, conf->ref, inode);
    conf->ref--;
    if (conf->ref == 0) {
        AOS_FS_TRACE(1, FS_DEBUG_TAG"emmc %d closed", __FS_ERR_WHERE__, conf->id);
        struct mmc_host *host = &conf->emmc_gloable_op;
        close_card(host);
        conf->opened = 0;
#ifdef FS_EMMC_CACHE_ENABLE
        fs_cache_delinit(&conf->cache);
#endif
    }
    fs_mutex_unlock(conf->mutex);
    return 0;
}

int emmc_card_ioctl(struct inode *inode, int cmd, unsigned long arg)
{
#ifdef FS_EMMC_CACHE_ENABLE
    struct fs_emmc_conf *conf = (struct fs_emmc_conf *)(inode->i_private);
#endif

    switch (cmd) {
        case BIOC_FLUSH:
#ifdef FS_EMMC_CACHE_ENABLE
            fs_cache_sync(&conf->cache);
#endif
            return 0;
        default:
            break;
    }
    AOS_FS_TRACE(0, FS_ERR_TAG"unknown cmd, cmd: %d", __FS_ERR_WHERE__, cmd);
    return -1;
}

int emmc_card_priv_conf_init(int id, struct fs_emmc_conf **ppriv)
{
    struct fs_emmc_conf *priv = NULL;
    struct mmc_host *host = NULL;

    if (g_emmc_priv[id] != NULL) {
        priv = g_emmc_priv[id];
        fs_mutex_lock(priv->mutex);
        priv->ref++;
        fs_mutex_unlock(priv->mutex);
        *ppriv = priv;
        AOS_FS_TRACE(1, FS_DEBUG_TAG"priv=%p priv->ref=%d", __FS_ERR_WHERE__, priv, priv->ref);
        return 0;
    }

    priv = fs_mm_zalloc(sizeof(struct fs_emmc_conf));
    if (priv == NULL) {
        return -ENOMEM;
    }

    g_emmc_priv[id] = priv;
    priv->mutex = fs_mutex_create(NULL);
    fs_mutex_lock(priv->mutex);
    priv->id = id;
    priv->ref++;
    priv->emmc_cfg = &emmc_cfg;
#ifdef RTOS
    emmc_wait_done_id = osSemaphoreCreate(osSemaphore(device_emmc_wait_done), 0);
    priv->emmc_wait_done_id = emmc_wait_done_id;
#endif
    host = &priv->emmc_gloable_op;
    host->cfg = priv->emmc_cfg;

    fs_mutex_unlock(priv->mutex);
    *ppriv = priv;
    AOS_FS_TRACE(1, FS_DEBUG_TAG"priv=%p priv->ref=%d", __FS_ERR_WHERE__, priv, priv->ref);
    return 0;
}

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const struct block_operations g_emmc_bops = {
    emmc_card_open,     /* open     */
    emmc_card_close,    /* close    */
    emmc_card_read,     /* read     */
    emmc_card_write,    /* write    */
    emmc_card_geometry, /* geometry */
    emmc_card_ioctl     /* ioctl    */
};

#if defined(CONFIG_GPT_PARTITION)
void bes_emmc_partition_handler(struct partition_s *part, void *path)
{
    char dev[PATH_MAX];
    int ret;
    snprintf(dev, sizeof(dev), "/%s", part->name);
    ret = register_blockpartition(dev, 0, path, part->firstblock, part->nblocks);
    if (ret < 0) {
        AOS_FS_TRACE(0, FS_ERR_TAG"Failed to register block device: %s %d", __FS_ERR_WHERE__, part->name, ret);
    }
    AOS_FS_TRACE(1, FS_DEBUG_TAG"Register block device:%s, first:%d, nblocks:%d, blocksize:%d",
                 __FS_DEBUG_WHERE__,
                 part->name, part->firstblock, part->nblocks, part->blocksize);
}
#endif

#endif


/****************************************************************************
 * Name: bes_emmc_slotinitialize
 *
 * Description:
 *   Initialize one slot for operation using the EMMC interface
 *
 * Input Parameters:
 *   minor - The EMMC minor device number.  The EMMC device will be
 *     registered as /emmcN where N is the minor number
 *   dev - And instance of an EMMC interface.  The EMMC hardware should
 *     be initialized and ready to use.
 *
 ****************************************************************************/
int bes_emmc_slotinitialize(int id, int minor)
{
#ifdef EMMC0_BASE
    int ret = -ENOMEM;
    struct fs_emmc_conf *priv = NULL;
    char devname[16];

    if (minor < 0 || minor > 255) {
        AOS_FS_TRACE(0, FS_ERR_TAG"param error, minor: %d", __FS_ERR_WHERE__, minor);
        return -EINVAL;
    }

    ret = emmc_card_priv_conf_init(id, &priv);
    if (ret != 0) {
        return ret;
    }

    snprintf(devname, 16, "/dev/emmc%d", minor);
    ret = register_blockdriver(devname, &g_emmc_bops, 0, priv);
    if (ret < 0) {
        AOS_FS_TRACE(0, FS_ERR_TAG"register_blockdriver error, ret: %d", __FS_ERR_WHERE__, ret);
        return ret;
    }
#endif
    return 0;
}

int bes_emmc_slotinitialize_partition(int id, int minor, int part, off_t firstsector, off_t nsectors)
{
#ifdef EMMC0_BASE
    int ret = -ENOMEM;
    struct fs_emmc_conf *priv = NULL;
    char devname[32];
    char pdevname[32];

    if (minor < 0 || minor > 255) {
        AOS_FS_TRACE(0, FS_ERR_TAG"param error, minor: %d", __FS_ERR_WHERE__, minor);
        return -EINVAL;
    }

    if (part < 0 || minor > 16) {
        AOS_FS_TRACE(0, FS_ERR_TAG"param error, part: %d", __FS_ERR_WHERE__, minor);
        return -EINVAL;
    }

    ret = emmc_card_priv_conf_init(id, &priv);
    if (ret != 0) {
        return ret;
    }

    snprintf(devname, sizeof(devname) - 1, "/dev/emmc%d", minor);
    snprintf(pdevname, sizeof(pdevname) - 1, "/dev/emmc%d_%d", minor, part);
    ret = register_blockdriver(devname, &g_emmc_bops, 0, priv);
    if (ret < 0) {
        if (ret != -EEXIST) {
            AOS_FS_TRACE(0, FS_ERR_TAG"register_blockdriver error, ret: %d", __FS_ERR_WHERE__, ret);
            return ret;
        }
    }
    ret = register_blockpartition(pdevname, 0, devname, firstsector, nsectors);
    if (ret < 0) {
        AOS_FS_TRACE(0, FS_ERR_TAG"register_blockpartition error, ret: %d", __FS_ERR_WHERE__, ret);
        return ret;
    }
#endif
    return 0;
}

int bes_emmc_slotinitialize_partition_offset(int idx)
{
    int offset = 0;
    for (int i = 0; i < idx; i++) {
        offset += g_emmc_partition[i];
    }
    return offset;
}

void bes_emmc_drivers_register(void)
{
    int id = 0;
    int minor = 0;

#ifdef EMMC_PARTITION_ENABLE
    int boffset = 0;
    int bnum = 0;

    for (int i = 0; i < AOS_EMMC_PART_NUM; i++) {
        boffset = bes_emmc_slotinitialize_partition_offset(i);
        bnum = g_emmc_partition[i];
        bes_emmc_slotinitialize_partition(id, minor, i + 1, boffset, bnum); // /dev/emmc0_i+1
    }
#else
    bes_emmc_slotinitialize(id, minor);
#endif
}

