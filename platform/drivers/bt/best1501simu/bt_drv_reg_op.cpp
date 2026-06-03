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
#include "cmsis.h"
#include "bt_drv_reg_op.h"
#include "bt_drv_internal.h"
#include "bt_drv_interface.h"
#include "bt_drv.h"
#include "hal_sysfreq.h"
#include "hal_chipid.h"
#include "hal_trace.h"
#include "hal_iomux.h"
#include <string.h>
#include "bt_drv_1501simu_internal.h"

uint8_t  sleep_param[] =
{
#ifdef __TWS__
   0,
#else
   1,    // sleep_en;
#endif
   1,    // exwakeup_en;
   0xc8,0,    //  lpo_calib_interval;   lpo calibration interval
   0xa,0,0,0,    // lpo_calib_time;  lpo count lpc times
};

uint8_t bt_drv_reg_op_force_get_lc_state(uint16_t conhdl)
{
    return 0;
}

int bt_drv_reg_op_currentfreeaclbuf_get(void)
{
    return 0;
}

uint8_t bt_drv_reg_op_get_controller_tx_free_buffer(void)
{
    return 0;
}

uint8_t bt_drv_reg_op_get_controller_ble_tx_free_buffer(void)
{
    return 0;
}

void bt_drv_reg_op_lsto_hack(uint16_t hciHandle, uint16_t lsto)
{
}

void bt_drv_reg_op_bt_info_checker(void)
{
}

void bt_drv_reg_op_ibrt_env_reset(void)
{
}

struct ke_timer
{
    /// next ke timer
    struct ke_timer *next;
    /// message identifier
    uint16_t     id;
    /// task identifier
    uint16_t    task;
    /// time value
    uint32_t    time;
};

struct co_list_hdr
{
    /// Pointer to next co_list_hdr
    struct co_list_hdr *next;
};

/// structure of a list
struct co_list_con
{
    /// pointer to first element of the list
    struct co_list_hdr *first;
    /// pointer to the last element
    struct co_list_hdr *last;

    /// number of element in the list
    uint32_t cnt;
    /// max number of element in the list
    uint32_t maxcnt;
    /// min number of element in the list
    uint32_t mincnt;
};

struct mblock_free
{
    /// Next free block pointer
    struct mblock_free* next;
    /// Previous free block pointer
    struct mblock_free* previous;
    /// Size of the current free block (including delimiter)
    uint16_t free_size;
    /// Used to check if memory block has been corrupted or not
    uint16_t corrupt_check;
};
bool bt_drv_reg_op_get_dbg_state(void)
{
    return 0;
}

void bt_drv_reg_op_controller_state_checker(void)
{
}

void bt_drv_reg_op_crash_dump(void)
{
}

#define __REBOOT_PAIRING_MODE_ADJUST_GAIN__

void bt_drv_reg_op_set_swagc_mode(uint8_t mode)
{
}

void bt_drv_reg_op_force_retrans(bool enable)
{
    return;

    BT_DRV_REG_OP_ENTER();

    if (enable)
    {
        BTDIGITAL_REG_SET_FIELD(0xd0220468,3,23,3);
    }
    else
    {
        BTDIGITAL_REG_SET_FIELD(0xd0220468,3,23,0);
    }
    BT_DRV_REG_OP_EXIT();
}

struct rx_monitor
{
    int8_t rssi;
    uint8_t rxgain;
};

bool bt_drv_reg_op_read_rssi_in_dbm(uint16_t connHandle,rx_agc_t* rx_val)
{
    return false;
}

void bt_drv_reg_op_save_fast_ack_sync_bit(void)
{
    /*
     *  save ibrt master's fast ack sync bit
     * 1. snoop connected(ibrt master)
     * 2. or tws switch complete(new ibrt master)
     */

}

void bt_drv_reg_op_ibrt_retx_att_nb_set(uint8_t retx_nb)
{
}


uint16_t bt_drv_reg_op_bitoff_getf(int elt_idx)
{
    uint32_t evt_ptr = 0;
    uint32_t acl_par_ptr = 0;
    uint16_t bitoff_val = 0;

    uint32_t value_store_flag = 0;
    uint32_t ld_acl_env_store = 0;
    value_store_flag = *(uint32_t *)(DEBUG_ADDR_VALUE_STORE);
    if(value_store_flag == 0xffffffff)
    {
        ld_acl_env_store = *(uint32_t *)(DEBUG_ADDR_VALUE_STORE+4);
    }

    evt_ptr = *(uint32_t *)(ld_acl_env_store + elt_idx*4);//ld_acl_env
    if(evt_ptr)
    {
        acl_par_ptr = evt_ptr;
        //last_sync_bit_off = acl_evt_ptr + 0xba;
        bitoff = *(uint16_t *)(acl_evt_ptr + 0xba);
        DRIVERS_TRACE(1,"[%s] acl_par: acl_par_ptr=0x%x, bitoff_val=%d",__func__,acl_par_ptr,bitoff_val);
    }
    return bitoff_val;
}

void em_bt_bitoff_setf(int elt_idx, uint16_t bitoff)
{
}

void bt_drv_reg_op_cs_monitor(void)
{
}

uint8_t  bt_drv_reg_op_get_role(uint8_t linkid)
{
    return 0xff;
}
extern "C" void bt_drv_reg_op_data_tx_flow(uint8_t linkid, uint8_t on_off)
{
}
void bt_drv_reg_op_set_tpoll(uint8_t linkid,uint16_t poll_interval)
{
}

void bt_drv_reg_op_set_music_link(uint8_t link_id)
{
}

void bt_drv_reg_op_music_link_config(uint16_t active_link,uint8_t active_role,uint16_t inactive_link,uint8_t inactive_role)
{
}

void bt_drv_reg_op_pcm_set(uint8_t en)
{
    return;
}

uint8_t bt_drv_reg_op_pcm_get()
{
    return 0;
}

bool bt_drv_reg_op_check_bt_controller_state(void)
{
    return 1;
}

uint16_t em_bt_clkoff1_getf(int elt_idx)
{
	return 0;
}
uint16_t em_bt_clkoff0_getf(int elt_idx)
{
    return 0;
}

void bt_drv_reg_op_piconet_clk_offset_get(uint16_t connHandle, int32_t *clock_offset, uint16_t *bit_offset)
{
    uint8_t index = 0;
    uint32_t clock_offset_raw = 0;

    BT_DRV_REG_OP_ENTER();
    if(hal_get_chip_metal_id()>=HAL_CHIP_METAL_ID_0)
    {
        if (connHandle)
        {
            index = btdrv_conhdl_to_linkid(connHandle);
            *bit_offset = bt_drv_reg_op_bitoff_getf(index);
            clock_offset_raw = (em_bt_clkoff1_getf(index) << 16) | em_bt_clkoff0_getf(index);
            *clock_offset = clock_offset_raw;
            *clock_offset = (*clock_offset << 4) >> 4;
        }
        else
        {
            *bit_offset = 0;
            *clock_offset = 0;
        }
    }
    BT_DRV_REG_OP_EXIT();

}

void bt_drv_reg_op_sw_seq_filter(uint16_t existConnHandle)
{
}

void bt_drv_reg_op_sco_txfifo_reset(uint16_t codec_id)
{
}

void bt_drv_reg_op_dma_tc_clkcnt_get(uint32_t *btclk, uint16_t *btcnt)
{
    BT_DRV_REG_OP_ENTER();
    *btclk = *((volatile uint32_t*)0xd0220e34);
    *btcnt = (*((volatile uint32_t*)0xd0220e38) & 0x3ff);
    DRIVERS_TRACE(0, "dma tc get clk,cnt %d,%d", *btclk, *btcnt);
    BT_DRV_REG_OP_EXIT();
}

#if defined(IBRT)
void em_bt_bt_ext1_tx_silence_setf(int elt_idx, uint8_t txsilence)
{
}

void bt_drv_reg_op_acl_tx_silence(uint16_t connHandle, uint8_t on)
{
    return;
    uint8_t idx = btdrv_conhdl_to_linkid(connHandle);
    DRIVERS_TRACE(2,"BT_REG_OP:BT ACL tx silence idx=%d,on=%d\n",idx,on);

    if (btdrv_is_link_index_valid(idx))
    {
        if(TX_SILENCE_ON == on)
        {
            em_bt_bt_ext1_tx_silence_setf(idx, TX_SILENCE_ON);
        }
        else if(TX_SILENCE_OFF == on)
        {
            em_bt_bt_ext1_tx_silence_setf(idx, TX_SILENCE_OFF);
        }
    }
}


/*****************************************************************************
 Prototype    : btdrv_set_tws_acl_poll_interval
 Description  : in ibrt mode, set tws acl poll interval
 Input        : uint16_t poll_interval
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History        :
 Date         : 2019/4/19
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/
void btdrv_reg_op_set_private_tws_poll_interval(uint16_t poll_interval, uint16_t poll_interval_in_sco)
{
}

void btdrv_reg_op_set_tws_link_duration(uint8_t slot_num)
{
}

void btdrv_reg_op_enable_private_tws_schedule(bool enable)
{
    return;
}

int8_t  bt_drv_reg_op_rssi_correction(int8_t rssi)
{
    return 0;
}

void bt_drv_reg_op_lm_nb_sync_hacker(uint8_t sco_status)
{
}

#define BT_FASTACK_ADDR   0xD0220468
#define BT_SCO_TRIGGER_BYPASS_ADDR   0xD022046C
void bt_drv_reg_op_fastack_status_checker(uint16_t conhdl)
{
}

void bt_drv_reg_op_clear_ibrt_snoop_status(void)
{
}
#endif

void bt_drv_reg_op_write_private_public_key(uint8_t* private_key,uint8_t* public_key)
{

}

void bt_drv_reg_op_for_test_mode_disable(void)
{
}

uint16_t bt_drv_reg_op_get_ibrt_sco_hdl(uint16_t acl_hdl)
{
    // FIXME
    return acl_hdl|0x100;
}

void bt_drv_reg_op_get_ibrt_address(uint8_t *addr)
{
}


void bt_drv_reg_op_set_tws_link_id(uint8_t link_id)
{
}

void bt_drv_reg_op_hack_max_slot(uint8_t link_id,uint8_t slot)
{
}

void bt_drv_reg_op_fa_set(uint8_t en)
{
}

//////return true means controller had received a data packet
bool bt_drv_reg_op_check_seq_pending_status(void)
{
    return 0;
}


///if the flag is not clear when disconnect happened  call this function
void bt_drv_reg_op_clear_seq_pending_status(void)
{
}

void bt_drv_reg_op_set_link_policy(uint8_t linkid, uint8_t policy)
{
}

uint8_t bt_drv_reg_op_lmp_sco_hdl_get(uint16_t sco_handle)
{
    return 0;
}

bool bt_drv_reg_op_lmp_sco_hdl_set(uint16_t sco_handle, uint8_t lmp_hdl)
{
    return false;
}

//pealse use btdrv_is_link_index_valid() check link index whether valid
uint8_t btdrv_conhdl_to_linkid(uint16_t connect_hdl)
{
    //invalid hci handle,such as link disconnected
    if(connect_hdl < HCI_HANDLE_MIN || connect_hdl > HCI_HANDLE_MAX)
    {
        DRIVERS_TRACE(0, "ERROR Connect Handle=0x%x",connect_hdl);
        return HCI_LINK_INDEX_INVALID;
    }
	else
	{
    	return (connect_hdl - HCI_HANDLE_MIN);
	}
}

void bt_drv_reg_op_force_sco_retrans(bool enable)
{
}

uint8_t bt_drv_reg_op_get_sync_id_op(uint8_t op, uint8_t sync_id)
{
    return  0xff;
}

void bt_drv_reg_op_force_set_sniff_att(uint16_t conhdle)
{
}

void bt_drv_reg_op_afh_follow_en(bool enable, uint8_t be_followed_link_id, uint8_t follow_link_id)
{
}

void bt_drv_reg_op_force_set_lc_state(uint16_t conhdl, uint8_t state)
{
}

void bt_drv_reg_op_set_ibrt_second_sco_decision(uint8_t value)
{
}

uint8_t bt_drv_reg_op_get_esco_nego_airmode(uint8_t sco_link_id)
{
    DRIVERS_TRACE(1,"%s REG_OP: not support", __func__);
    return 0xff;
}

void bt_drv_reg_op_update_dbg_state(void)
{
}
uint32_t bt_drv_reg_op_get_host_ref_clk(void)
{
    return 0;
}
uint8_t bt_drv_reg_op_bt_sync_swagc_en_get(void)
{
    return 0;
}
void bt_drv_reg_op_hci_vender_ibrt_ll_monitor(uint8_t* ptr, uint16_t* p_sum_err,uint16_t* p_rx_total)
{
    const char *monitor_str[35] =
    {
        "TX DM1",
        "TX DH1",
        "TX DM3",
        "TX DH3",
        "TX DM5",
        "TX DH5",
        "TX 2DH1",
        "TX 3DH1",
        "TX 2DH3",
        "TX 3DH3",
        "TX 2DH5",
        "TX 3DH5",

        "RX DM1",
        "RX DH1",
        "RX DM3",
        "RX DH3",
        "RX DM5",
        "RX DH5",
        "RX 2DH1",
        "RX 3DH1",
        "RX 2DH3",
        "RX 3DH3",
        "RX 2DH5",
        "RX 3DH5",
        "hec error",
        "crc error",
        "fec error",
        "gard error",
        "ecc count",

        "radio_count",
        "sleep_duration_count",
        "radio_tx_succss_count",
        "radio_tx_count",
        "softbit_success_count",
        "softbit_count",
    };

    uint8_t *p = ( uint8_t * )ptr;
    uint32_t sum_err = 0;
    uint32_t rx_data_sum = 0;
    uint32_t val;

    DRIVERS_TRACE(0,"ibrt_ui_log:ll_monitor");

    for (uint8_t i = 0; i < 35; i++)
    {
        val = co_read32p(p);
        if (val)
        {
            if(i>= 12 && i<=23)
            {
                rx_data_sum += val;
            }

            if((i > 23) && (i < 29))
            {
                sum_err += val;
            }
            DRIVERS_TRACE(2,"%s %d", monitor_str[i], val);
        }
        p+=4;
    }
    *p_sum_err = sum_err;
    *p_rx_total = rx_data_sum;
}

#if defined(PCM_FAST_MODE) && defined(PCM_PRIVATE_DATA_FLAG)
void bt_drv_reg_op_set_pcm_flag()
{
    //config btpcm slot
    BTDIGITAL_REG_SET_FIELD(0xD0220CBC,0x1,0,1);
    BTDIGITAL_REG_SET_FIELD(0xD0220CBC,0x4,1,7);

    //config private data
    BTDIGITAL_REG_SET_FIELD(0xd0220c88,0xff,8,0x4b);
    BTDIGITAL_REG_SET_FIELD(0xd0220c88,0x1,15,1);
    BTDIGITAL_REG_SET_FIELD(0xd0220c88,0x1,17,1);
//    BTDIGITAL_REG_SET_FIELD(0xd0220648,0x1ff,9,8);
//    BTDIGITAL_REG_SET_FIELD(0xd0220648,0x1,31,1);

//    BTDIGITAL_REG_SET_FIELD(BT_BES_CNTL5_ADDR, 1, 11, 1);
}
#endif

void bt_drv_reg_op_key_gen_after_reset(bool enable)
{
}

uint32_t bt_drv_lp_clk_get()
{
    return 0xFFFFFFFF;
}

void bt_drv_reg_op_trigger_controller_assert(void)
{
    ASSERT(0, "Trigger BTC crash on purpose!")
}

int bt_drv_reg_op_acl_chnmap(uint16_t hciHandle, uint8_t *chnmap, uint8_t chnmap_len)
{
    return -1;
}

void bt_drv_reg_op_set_btpcm_trig_flag(bool flag)
{
    uint32_t value_store_flag = 0;
    value_store_flag = *(uint32_t *)(DEBUG_ADDR_VALUE_STORE);
    if(value_store_flag == 0xffffffff)
    {
        uint32_t pcm_need_start_flag = 0;
        pcm_need_start_flag = *(uint32_t *)(DEBUG_ADDR_VALUE_STORE + 8);
        BTDIGITAL_REG(pcm_need_start_flag) = flag;
        DRIVERS_TRACE(0,"pcm_need_start_flag=0x%x\n",pcm_need_start_flag);
    }
}

uint8_t dma_tc_used_index = 0;
#define DMA_TC_MAX_CNT  4
void bt_drv_reg_op_enable_dma_tc(uint8_t adma_ch, uint32_t dma_base)
{
    uint32_t val = 0;
    uint8_t i = 0;
    uint32_t lock;
    adma_ch = adma_ch&0xF;
    lock = int_lock();
    if(dma_tc_used_index >= DMA_TC_MAX_CNT)
    {
        //  int_unlock(lock);
        DRIVERS_TRACE(0,"enable_dma_tc err more than max ch");
        goto exit;
    }
    val = BTDIGITAL_REG(0xd0220C50);
    for(i = 0; i < DMA_TC_MAX_CNT; i++)
    {
        if((val>>(i*8)&0x1F)==(uint32_t)(8+adma_ch))
        {
            // int_unlock(lock);
            DRIVERS_TRACE(1,"enable_dma_tc err ch already enabled adma ch %d",adma_ch);
            goto exit;
        }
    }
    for(i = 0; i < DMA_TC_MAX_CNT; i++)
    {
        if((val>>(i*8)&0x1F)==0)
            break;
    }
    if(i >= DMA_TC_MAX_CNT)
    {
        //int_unlock(lock);
        DRIVERS_TRACE(0,"enable_dma_tc err cannot find unused ch");
        goto exit;
    }
    dma_tc_used_index++;
    BTDIGITAL_REG_SET_FIELD(0xd0220C50,1,31,1);//reg_trig_enable
    BTDIGITAL_REG_SET_FIELD(0xd0220C50,0x1F,(i*8),(8+adma_ch));//reg_trig_selx
    DRIVERS_TRACE(2,"enable_dma_tc succ adma_ch=%d sel reg 0x%x/%x,used %d",adma_ch,val,BTDIGITAL_REG(0xd0220C50),dma_tc_used_index);
exit:
    int_unlock(lock);
}

void bt_drv_reg_op_disable_dma_tc(uint8_t adma_ch, uint32_t dma_base)
{
    uint32_t val = 0;
    uint8_t i = 0;
    uint32_t lock;
    adma_ch = adma_ch&0xF;
    lock = int_lock();
    if(dma_tc_used_index == 0)
    {
        // int_unlock(lock);
        DRIVERS_TRACE(0,"disable_dma_tc err no ch enabled");
        goto exit;
    }
    val = BTDIGITAL_REG(0xd0220C50);
    for(i = 0; i < DMA_TC_MAX_CNT; i++)
    {
        if((val>>(i*8)&0x1F)==(uint32_t)(8+adma_ch))
            break;
    }
    if(i >= DMA_TC_MAX_CNT)
    {
        // int_unlock(lock);
        DRIVERS_TRACE(0,"disable_dma_tc err cannot find same ch id to disable");
        goto exit;
    }
    dma_tc_used_index--;
    BTDIGITAL_REG_SET_FIELD(0xd0220C50,0x1F,(i*8),0);//reg_trig_selx
    DRIVERS_TRACE(2,"disable_dma_tc succ adma_ch=%d sel reg 0x%x/%x,used %d",adma_ch,val,BTDIGITAL_REG(0xd0220C50),dma_tc_used_index);
exit:
    int_unlock(lock);
}


void bt_drv_reg_op_dma_tc_clkcnt_get_by_ch(uint32_t *btclk, uint16_t *btcnt, uint8_t adma_ch, uint32_t dma_base)
{
    uint32_t val = 0;
    uint8_t i = 0;
    uint32_t cap_reg_base = 0xd0220e34;
    adma_ch = adma_ch&0xF;
    DRIVERS_TRACE(0,"dma_tc_clkcnt_get_by_ch adma_ch=%d\n",adma_ch);
    val = BTDIGITAL_REG(0xd0220C50);
    for(i = 0; i < DMA_TC_MAX_CNT; i++)
    {
        if((val>>(i*8)&0x1F)==(uint32_t)(8+adma_ch))
            break;
    }
    if(i >= DMA_TC_MAX_CNT)
    {
        DRIVERS_TRACE(0,"get_dma_tc err cannot find same ch id, not enabled?");
        return;
    }
    *btclk = BTDIGITAL_REG(cap_reg_base+8*i);
    *btcnt = (BTDIGITAL_REG(cap_reg_base+4+8*i) & 0x3ff);
    DRIVERS_TRACE(0, "dma tc dma_tc_clkcnt_get_by_ch clk,cnt %d,%d", *btclk, *btcnt);
}

#ifdef PCM_FAST_MODE
void btdrv_reg_op_open_pcm_fast_mode_enable(void)
{
    DRIVERS_TRACE(0,"pcm fast mode\n");
    BTDIGITAL_REG_SET_FIELD(0xD0220C88, 1, 15, 1);
    BTDIGITAL_REG_SET_FIELD(0xD0220C88, 0x7f, 8, 0x3b);
    BTDIGITAL_REG_SET_FIELD(0xD0220688, 0x1FF, 0, 8);
    BTDIGITAL_REG_SET_FIELD(0xD0220688, 1, 31, 1);
}
void btdrv_reg_op_open_pcm_fast_mode_disable(void)
{
    //BT_DRV_REG_OP_CLK_ENB();
    //BTDIGITAL_REG_SET_FIELD(BT_BES_PCMCNTL_ADDR, 1, 15, 0);
    //BTDIGITAL_REG_SET_FIELD(BT_PCMPHYSCNTL1_ADDR, 0x1F, 0, 0);
    //BT_DRV_REG_OP_CLK_DIB();
}
#endif

void bt_drv_reg_op_ble_sup_timeout_set(uint16_t ble_conhdl, uint16_t sup_to)
{

}

int32_t bt_drv_reg_op_get_clkoffset(uint16_t linkid)
{
    uint32_t clkoff = 0;
    uint32_t acl_evt_ptr = 0x0;
    int32_t offset;
    uint32_t local_offset;

    uint32_t value_store_flag = 0;
    uint32_t ld_acl_env_store = 0;
    value_store_flag = *(uint32_t *)(DEBUG_ADDR_VALUE_STORE);
    if(value_store_flag == 0xffffffff)
    {
        ld_acl_env_store = *(uint32_t *)(DEBUG_ADDR_VALUE_STORE+4);
    }

    if(ld_acl_env_store)
    {
        acl_evt_ptr = *(uint32_t *)(ld_acl_env_store + linkid*4);//ld_acl_env
    }

    if (acl_evt_ptr != 0)
    {
        //[last_sync_clk_off]offset=0xa4
        clkoff = *(uint32_t *)(acl_evt_ptr + 0xa4);
    }
    else
    {
        DRIVERS_TRACE(1,"BT_REG_OP:ERROR LINK ID FOR RD clkoff %x", linkid);
    }

    local_offset = clkoff & 0x0fffffff;
    offset = local_offset;
    offset = (offset << 4)>>4;
   return offset;
}

uint8_t bt_drv_reg_op_get_max_acl_nb(void)
{
    return MAX_NB_ACTIVE_ACL;
}


bool bt_drv_reg_op_read_ble_rssi_in_dbm(uint16_t connHandle,rx_agc_t* rx_val)
{
    return false;
}