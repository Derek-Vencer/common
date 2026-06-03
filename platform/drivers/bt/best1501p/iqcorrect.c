#ifndef NOSTD
#if defined (RX_IQ_CAL) || defined(TX_PULLING_CAL) || defined(TX_IQ_CAL)

#include <math.h>
#include <stdio.h>

#include "hal_dma.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "heap_api.h"

#include "string.h"
#include "besbt_string.h"
#include "bt_drv.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "hal_btdump.h"
#include "iqcorrect.h"
#include "hal_chipid.h"

#define LOG_MODULE                          HAL_TRACE_MODULE_APP

#define NTbl (1024)
#define fftSize 1024

#define IQ_CALIB
#define DC_CALIB
//#define IQ_DEBUG
//#define DC_DEBUG
//#define PA_BIAS_DEPUG
//#define PULLING_DEBUG
#define DC_ENERGY_THD 200
#define DC_RANGE_MIN -200
#define DC_RANGE_MAX 200
#define IQ_ENERGY_THD 300
#define IQ_RANGE_MIN -200
#define IQ_RANGE_MAX 200
#define SWEEP_STEP 50

#define bitshift 10
#define pi (3.1415926535898)
#define min(a,b)    (((a) < (b)) ? (a) : (b))

#define READ_REG(b,a) *(volatile uint32_t *)(uintptr_t)((uint32_t)(b)+(a))
#define WRITE_REG(v,b,a) *(volatile uint32_t *)(uintptr_t)((uint32_t)(b)+(a)) = v

#define   BUF_SIZE   (1024)

#define MAX_COUNT 5
volatile int iqimb_dma_status = 0;
//short M0data[BUF_SIZE];
#define MED_MEM_POOL_SIZE (88*1024)
static uint8_t *g_medMemPool = NULL;

const uint16_t win[512] = {
    #include "conj_win_1024_half.txt"
};

extern void *rt_malloc(unsigned int size);
extern void rt_free(void *rmem);

typedef struct ComplexInt_
{
    int re;
    int im;
} ComplexInt;
typedef struct Complexflt_
{
    float re;
    float im;
} ComplexFlt;
typedef struct ComplexShort_
{
    short re;
    short im;
} ComplexShort;

#ifdef DCCalib
static void Tblgen(ComplexShort*w0, ComplexShort*w1, ComplexShort*w2,int len)
{
    for (int i=0; i<len; i++)
    {

        w0[i].re = (short)(cos(2*pi*i/(2048.0/32))*32767);
        w0[i].im = (short)(sin(2*pi*i/(2048.0/32))*32767);
        w1[i].re = (short)(cos(2*pi*i/(2048.0/31))*32767);
        w1[i].im = (short)(sin(2*pi*i/(2048.0/31))*32767);
        w2[i].re = (short)(cos(2*pi*i/(2048.0/33))*32767);
        w2[i].im = (short)(sin(2*pi*i/(2048.0/33))*32767);
    }
}
#endif
#if 1
//static void Tblgen_iq(ComplexShort*w0, ComplexShort*w1, ComplexShort*w2,int len)
static void Tblgen_iq_1st(ComplexFlt*w0,int len)
{
    for (int i=0; i<len; i++)
    {
        w0[i].re = (float)(cos(2*pi*i/(1024.0/99)));
        w0[i].im = (float)(sin(2*pi*i/(1024.0/99)));
    }
}

static void Tblgen_iq_2nd(ComplexFlt*w0,int len)
{
    for (int i=0; i<len; i++)
    {
        w0[i].re = (float)(cos(2*pi*i/(1024.0/58)));
        w0[i].im = (float)(sin(2*pi*i/(1024.0/58)));
    }
}
#endif

typedef struct IQMismatchPreprocessState_
{
    short *M0data;
    //int *data_i;
    //int *data_q;
    ComplexFlt *Table0;
    //ComplexShort *Table1;
   // ComplexShort *Table2;
} IQMismatchPreprocessState;

IQMismatchPreprocessState *IQMismatchPreprocessState_init(int fft_size)
{
    DRIVERS_TRACE(0,"malloc ini");
    IQMismatchPreprocessState *st = (IQMismatchPreprocessState *)med_calloc(1, sizeof(IQMismatchPreprocessState));
    if (st){
        st->M0data = (short*)med_calloc(fft_size*2, sizeof(short));
        //st->data_i= (int*)med_calloc(fft_size, sizeof(int));
        //st->data_q= (int*)med_calloc(fft_size, sizeof(int));

#if 1
        st->Table0 = (ComplexFlt*)med_calloc(fft_size, sizeof(ComplexFlt));
        //st->Table1 = (ComplexShort*)med_calloc(fft_size, sizeof(ComplexShort));
        //st->Table2 = (ComplexShort*)med_calloc(fft_size, sizeof(ComplexShort));
#endif
        //DRIVERS_TRACE(5,"st:%p, st->M0data:%p, st->Table0:%p, st->Table1:%p, st->Table2:%p", st, st->M0data, st->Table0, st->Table1, st->Table2);
        DRIVERS_TRACE(0,"malloc ok");
    }
    return st;
}

int32_t IQMismatchPreprocessState_destroy(IQMismatchPreprocessState *st)
{
    //DRIVERS_TRACE(5,"st:%p, st->M0data:%p, st->Table0:%p, st->Table1:%p, st->Table2:%p", st, st->M0data, st->Table0, st->Table1, st->Table2);
    med_free(st->M0data);
    med_free(st->Table0);
    //med_free(st->Table1);
    //med_free(st->Table2);
    med_free(st);
    return 0;
}

//int IQMismatchParameterCalc_ex(const short *M0data,
//                               short Cohcnt, ComplexShort *Table0, ComplexShort *Table1, ComplexShort *Table2,int fftsize)
int IQMismatchParameterCalc_ex(const short *M0data,
                               short Cohcnt, ComplexFlt *Table0,int fftsize)
{
    int i,j,k;
    float M0 = 0;
    ComplexFlt tmp0;
    for (j = 0; j < Cohcnt; j++) {
        tmp0.re = 0.0f;
        tmp0.im = 0.0f;
        for (i=0; i<fftsize; i++) {
            if (i < (fftsize / 2)) {
                k = i;
            } else {
                k = fftsize - i - 1;
            }
            //DRIVERS_TRACE(2,"data_i[%d]=%d,data_q[%d]=%d,win[%d]=%d",i,(int)(M0data[2 * i]),i,(int)(M0data[2 * i + 1]),i,win[i]);
            tmp0.re = tmp0.re + ((float)M0data[2 * i] * Table0[i].re - (float)M0data[2 * i + 1] * Table0[i].im) * (float)(win[k] / 32768.0);
            tmp0.im = tmp0.im + ((float)M0data[2 * i] * Table0[i].im + (float)M0data[2 * i + 1] * Table0[i].re) * (float)(win[k] / 32768.0);
        }
        //DRIVERS_TRACE(2,"tmp0.re=%d, tmp0.im=%d",(int)(tmp0.re),(int)(tmp0.im));
        tmp0.re = (tmp0.re/ 1024.0);// >> bitshift;
        tmp0.im = (tmp0.im/ 1024.0);// >>bitshift;
        M0 = M0 + tmp0.re*tmp0.re + tmp0.im*tmp0.im;
    }
    //DRIVERS_TRACE(1,"M0*1000=%d",(int)(M0*1000));
    return (int)(M0/Cohcnt * 100);
}


void caculate_energy_main_test(IQMismatchPreprocessState *st,int* Energy,int* Energy1,int fftsize)
{
    short Cohcnt = 1;
    //*Energy1 = IQMismatchParameterCalc_ex(st->M0data, Cohcnt, st->Table0, st->Table1, st->Table2,fftsize);
    *Energy1 = IQMismatchParameterCalc_ex(st->M0data, Cohcnt, st->Table0, fftsize);
}

static struct HAL_DMA_DESC_T iqimb_dma_desc[1];
static uint8_t g_dma_channel = HAL_DMA_CHAN_NONE;

static void iqimb_dma_dout_handler(uint32_t remains, uint32_t error, struct HAL_DMA_DESC_T *lli)
{
    if(g_dma_channel != HAL_DMA_CHAN_NONE)
    {
        hal_audma_free_chan(g_dma_channel);
    }
    hal_btdump_disable();
    iqimb_dma_status = 0;

    return;
}

int bt_iqimb_dma_enable ( short * dma_dst_data, uint16_t size)
{
    int sRet = 0;
    struct HAL_DMA_CH_CFG_T iqimb_dma_cfg;
    // DRIVERS_TRACE(0,"bt_iqimb_dma_enable :");

    iqimb_dma_status = 1;

    sRet = memset_s(&iqimb_dma_cfg, sizeof(iqimb_dma_cfg), 0, sizeof(iqimb_dma_cfg));
    if (sRet){
        DRIVERS_TRACE(1, "%s line:%d sRet:%d", __func__, __LINE__, sRet);
    }

    //iqimb_dma_cfg.ch = hal_audma_get_chan(HAL_AUDMA_DSD_RX,HAL_DMA_HIGH_PRIO);

    iqimb_dma_cfg.ch = hal_audma_get_chan(HAL_AUDMA_BTDUMP,HAL_DMA_HIGH_PRIO);
    ASSERT((HAL_DMA_CHAN_NONE != iqimb_dma_cfg.ch), "hal_audma_get_chan failed.");
    g_dma_channel = iqimb_dma_cfg.ch;
    iqimb_dma_cfg.dst_bsize = HAL_DMA_BSIZE_16;
    iqimb_dma_cfg.dst_periph = 0; //useless
    iqimb_dma_cfg.dst_width = HAL_DMA_WIDTH_WORD;
    iqimb_dma_cfg.handler = (HAL_DMA_IRQ_HANDLER_T)iqimb_dma_dout_handler;
    iqimb_dma_cfg.src = 0; // useless
    iqimb_dma_cfg.src_bsize = HAL_DMA_BSIZE_4;

    //iqimb_dma_cfg.src_periph = HAL_AUDMA_DSD_RX;
    iqimb_dma_cfg.src_periph = HAL_AUDMA_BTDUMP;
    iqimb_dma_cfg.src_tsize = size;//1600; //1600*2/26=123us
    iqimb_dma_cfg.src_width = HAL_DMA_WIDTH_WORD;
    //iqimb_dma_cfg.src_width = HAL_DMA_WIDTH_HALFWORD;
    iqimb_dma_cfg.try_burst = 1;
    iqimb_dma_cfg.type = HAL_DMA_FLOW_P2M_DMA;
    iqimb_dma_cfg.dst = (uintptr_t)(dma_dst_data);

    hal_audma_init_desc(&iqimb_dma_desc[0], &iqimb_dma_cfg, 0, 1);

    hal_audma_sg_start(&iqimb_dma_desc[0], &iqimb_dma_cfg);

    //configed after mismatch parameter done, or apb clock muxed.

    //wait
    for(volatile int i=0; i<5000; i++);
    hal_btdump_enable();

    return 1;

}

void check_mem_data(void* data, int len)
{
    short* share_mem = (short*)data;
    DRIVERS_TRACE(3,"check_mem_data :share_mem= %p, 0x%x, 0x%x",share_mem,share_mem[0],share_mem[1]);

    int32_t i =0;
    int32_t remain = len;

    while(remain > 0)
    {
        for(i=0; i<32; i++)//output two line
        {
            if (remain >16)
            {
                DRIVERS_DUMP16("%04X ",share_mem,16);
                share_mem +=16;
                remain -= 16;
            }
            else
            {
                DRIVERS_DUMP16("%04X ",share_mem,remain);
                remain =0;
                return;
            }
        }
        //  DRIVERS_TRACE(0,"\n");
        //DRIVERS_TRACE(1,"addr :0x%08x\n",share_mem);
        hal_sys_timer_delay(MS_TO_TICKS(100));
    }
}

int bt_Txdc_cal_set(int ch_num, int dc_add)
{
    uint32_t val;
    uint32_t tmp = (uint32_t)dc_add;
    //sel apb clock
    val = READ_REG(0xd0350348,0x0);
    if(ch_num==0)
    {
        val &= 0xfffffc00; //bit31 1 int_en_mismatch
        val |= tmp & 0x3ff;
    }
    else
    {
        // DRIVERS_TRACE(1,"bt_Txdc_cal_set, dcadd : 0x%08x",tmp);
        val &= 0xfc00ffff; //bit31 1 int_en_mismatch
        val |= (tmp & 0x3ff) <<16;
    }
    WRITE_REG(val,0xd0350348,0x0);


    // DRIVERS_TRACE(1,"bt_Txdc_cal_set, 0xd0350348 : 0x%08x",val);

    return 1;
}

//g/p mismatch base addr 0xd0310000
//dc_i: 0xd0350348 9:0
//dc_q: 0xd0350348 25:16
int bt_iqimb_add_mismatch(int ch_num, int gain_mis, int phase_mis, int dc_i, int dc_q, uint32_t addr)
{
    uint32_t val;
    //sel apb clock
    val = READ_REG(0xd0350348,0x0);
    val &= 0x7fffffff; //bit31 1 int_en_mismatch
    val |= (0x0<<31);
    WRITE_REG(val,0xd0350348,0x0);

    val = (phase_mis << 16) | (gain_mis & 0x0000ffff);
    WRITE_REG(val,addr,ch_num*4);
    //tval = READ_REG(addr,0x0);
    //DRIVERS_TRACE(1,"bt_iqimb_add_mismatch, iq : 0x%08x",tval);

    WRITE_REG(0x400000,0xd0350220,0);
    WRITE_REG(0x400000,0xd0350224,0);
    // bt_Txdc_cal_set(0,dc_i);
    //bt_Txdc_cal_set(1,dc_q);

    /*
        val = READ_REG(0xd0350348,0x0);
        val &= 0xfffffc00; //bit9:0
        val |= dc_i;
        val &= 0xfc00ffff; //bit25:16
        val |= (dc_q<<16);
        WRITE_REG(val,0xd0350348,0x0);
    */
    //sel 26m clock
    val = READ_REG(0xd0350348,0x0);
    val &= 0x7fffffff; //bit31 1 int_en_mismatch
    val |= (0x1<<31);
    WRITE_REG(val,0xd0350348,0x0);

    return 1;
}

//g/p mismatch base addr 0x002E
int bt_rfimb_add_mismatch(int phase_mis)
{
    uint16_t phase_mis_high = phase_mis << 9;
    uint16_t val;
    val = phase_mis_high | phase_mis;
    btdrv_write_rf_reg(0x002E,val);

    return 1;
}

void DC_correction(IQMismatchPreprocessState *st,int *dc_i_r,int *dc_q_r,int fftsize)
{
    uint8_t k;
    int Energy,Energy1,tmp;
    int dc_iters = 2;
    int dc_i;
    int dc_q;
    int dc_step = 4;
    int dc_i_base;
    int dc_q_base;
    int P0,PIplus,PIneg,PQplus,PQneg,PIPQ;
    int CI,CQ,tmp_D;
    dc_i_base =0;
    dc_q_base = 0;
    for(k=0; k<dc_iters; k++)
    {
        dc_i = dc_i_base;
        dc_q = dc_q_base;
        bt_Txdc_cal_set(0,dc_i);
        bt_Txdc_cal_set(1,dc_q);
        bt_iqimb_dma_enable(st->M0data,(BUF_SIZE/2));
        while(1)
        {
            if(iqimb_dma_status==0)
                break;
        }
        caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
        P0 = Energy1;
        dc_i = dc_i_base + dc_step;
        dc_q = dc_q_base;
        bt_Txdc_cal_set(0,dc_i);
        bt_Txdc_cal_set(1,dc_q);
        bt_iqimb_dma_enable(st->M0data,(BUF_SIZE/2));
        while(1)
        {
            if(iqimb_dma_status==0)
                break;
        }
        caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
        PIplus = Energy1;
        dc_i = dc_i_base - dc_step;
        dc_q = dc_q_base;
        bt_Txdc_cal_set(0,dc_i);
        bt_Txdc_cal_set(1,dc_q);
        bt_iqimb_dma_enable(st->M0data,(BUF_SIZE/2));
        while(1)
        {
            if(iqimb_dma_status==0)
                break;
        }
        caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
        PIneg = Energy1;
        dc_i = dc_i_base;
        dc_q = dc_q_base + dc_step;
        bt_Txdc_cal_set(0,dc_i);
        bt_Txdc_cal_set(1,dc_q);
        bt_iqimb_dma_enable(st->M0data,(BUF_SIZE/2));
        while(1)
        {
            if(iqimb_dma_status==0)
                break;
        }
        caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
        PQplus = Energy1;
        dc_i = dc_i_base;
        dc_q = dc_q_base - dc_step;
        bt_Txdc_cal_set(0,dc_i);
        bt_Txdc_cal_set(1,dc_q);
        bt_iqimb_dma_enable(st->M0data,(BUF_SIZE/2));
        while(1)
        {
            if(iqimb_dma_status==0)
                break;
        }
        caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
        PQneg = Energy1;
        dc_i = dc_i_base + dc_step;
        dc_q = dc_q_base + dc_step;
        bt_Txdc_cal_set(0,dc_i);
        bt_Txdc_cal_set(1,dc_q);
        bt_iqimb_dma_enable(st->M0data,(BUF_SIZE/2));
        while(1)
        {
            if(iqimb_dma_status==0)
                break;
        }
        caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
        PIPQ = Energy1;
        tmp = 2*PIplus*PIplus + 2*PQplus*PQplus + 2*PIPQ*PIPQ;
        tmp = tmp - 6*P0*P0;
        tmp = tmp + 4*P0*(PIneg+PQneg+PIPQ);
        tmp = tmp + 2*(PIplus*(PQplus - PQneg)-PIneg*(PQplus+PQneg));
        tmp_D = tmp - 4*PIPQ*(PIplus+PQplus);
        tmp = P0*(2*(PIplus - PIneg)+PQplus -PQneg);
        tmp = tmp - PIPQ*(PQneg-PQplus)+PIneg*(PQplus+PQneg);
        tmp = tmp + PQplus*(PQneg-PQplus-2*PIplus);
        CI = -dc_step*tmp;
        tmp = P0*(2*(PQplus - PQneg)+PIplus-PIneg);
        tmp = tmp - PIPQ*(PIneg-PIplus)+PQneg*(PIplus+PIneg);
        tmp = tmp + PIplus*(PIneg-PIplus-2*PQplus);
        CQ = -dc_step*tmp;
        dc_i_base = dc_i_base + CI/tmp_D;
        dc_q_base = dc_q_base + CQ/tmp_D;
        //dc_step = dc_step/2;
    }
    *dc_i_r = dc_i_base;
    *dc_q_r = dc_q_base;
}
int IQ_GAIN_Mismatch_Correction(IQMismatchPreprocessState *st,int phase_mis_base,int fftsize,uint32_t addr)
{
    uint8_t k = 0;
    int phase_mis_tmp =0;
    int gain_mis_tmp =0;
    int tmp = 0;
    int energy_ret0_last = 1048576;
    int energy_ret0 = 0;
    int energy_ret1 = 0;
    int energy_ret2 = 0;
    int Energy,Energy1 ;
    int iters = 4;
    int gainstep =8;
    int gain_mis_base = 0;

    phase_mis_tmp = phase_mis_base;
    energy_ret0_last = 1048576;
    for(k=0; k<iters+2; k++)
    {
        gain_mis_tmp = gain_mis_base;
        bt_iqimb_add_mismatch(0,gain_mis_tmp,phase_mis_tmp,0,0,addr);
        bt_iqimb_dma_enable(st->M0data,fftsize);
        while(1)
        {
            if(iqimb_dma_status==0)
                break;
        }
        caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
        energy_ret0 = Energy1;
        if(energy_ret0<50)
            break;
        if(k>0)
        {
            if(energy_ret0_last < energy_ret0)
            {
                gain_mis_base = gain_mis_base + tmp;
                gain_mis_tmp = gain_mis_base;
                bt_iqimb_add_mismatch(0,gain_mis_tmp,phase_mis_tmp,0,0,addr);
                bt_iqimb_dma_enable(st->M0data,fftsize);
                while(1)
                {
                    if(iqimb_dma_status==0)
                        break;
                }
                caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
                energy_ret0 = Energy1;
            }
        }
        //DRIVERS_TRACE(1,"IQ gain correct energy_ret0 = %d",energy_ret0);
        gainstep = 4;///////////////////
        gain_mis_tmp = gain_mis_base + gainstep;
        bt_iqimb_add_mismatch(0,gain_mis_tmp,phase_mis_tmp,0,0,addr);
        bt_iqimb_dma_enable(st->M0data,fftsize);
        while(1)
        {
            if(iqimb_dma_status==0)
                break;
        }
        caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
        energy_ret2 = Energy1;
        gain_mis_tmp = gain_mis_base - gainstep;
        bt_iqimb_add_mismatch(0,gain_mis_tmp,phase_mis_tmp,0,0,addr);
        bt_iqimb_dma_enable(st->M0data,fftsize);
        while(1)
        {
            if(iqimb_dma_status==0)
                break;
        }
        caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
        energy_ret1 = Energy1;
        tmp = energy_ret2 -2*energy_ret0 + energy_ret1;
        //DRIVERS_TRACE(1,"IQ gain correct energy_ret2 -2*energy_ret0 + energy_ret1 = %d",tmp);
        if(tmp>0)
        {
            tmp = (energy_ret2-energy_ret1)*gainstep/tmp/2;
            tmp = min(tmp,4*gainstep);
            gainstep = gainstep/2;
        }
        else
        {
            tmp = 0;
            iters = iters+1;
        }
        if(iters>8)
            break;
        if((gain_mis_base - tmp>60)||(gain_mis_base - tmp<-60))
            tmp = 0;
        //DRIVERS_TRACE(2,"IQ gain correct gain_mis = %d ,gain_adj = %d",gain_mis_base,tmp);
        gain_mis_base = gain_mis_base - tmp;
        energy_ret0_last = energy_ret0;
    }
    return gain_mis_base;
}
int IQ_Phase_Mismatch_Correction(IQMismatchPreprocessState *st,int gain_mis_base,int fftsize,uint32_t addr)
{
    uint8_t k = 0;
    int phase_mis_tmp =0;
    int gain_mis_tmp =0;
    int tmp = 0;
    int energy_ret0_last = 1048576;
    int energy_ret0 = 0;
    int energy_ret1 = 0;
    int energy_ret2 = 0;
    int Energy,Energy1 ;
    int iters = 4;
    int phasestep =4;
    int phase_mis_base = 0;
    tmp = 0;
    energy_ret0_last = 1048576;
    gain_mis_tmp = gain_mis_base;
    for(k=0; k<iters; k++)
    {
        phase_mis_tmp = phase_mis_base;
        bt_iqimb_add_mismatch(0,gain_mis_tmp,phase_mis_tmp,0,0,addr); //no mismatch
        bt_iqimb_dma_enable(st->M0data,fftsize);
        while(1)
        {
            if(iqimb_dma_status==0)
                break;
        }
        caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
        energy_ret0 = Energy1;
        if(k>0)
        {
            if(energy_ret0_last < energy_ret0)
            {
                phase_mis_base = phase_mis_base + tmp;
                phase_mis_tmp = phase_mis_base;
                bt_iqimb_add_mismatch(0,gain_mis_tmp,phase_mis_tmp,0,0,addr); //no mismatch
                bt_iqimb_dma_enable(st->M0data,fftsize);
                while(1)
                {
                    if(iqimb_dma_status==0)
                        break;
                }
                caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
                energy_ret0 = Energy1;
            }
        }
        if(energy_ret0<50)
            break;
        // DRIVERS_TRACE(1,"IQ phase correct energy_ret0 = %d",energy_ret0);
        phasestep = 4;//////////////////////////
        phase_mis_tmp = phase_mis_base + phasestep;
        bt_iqimb_add_mismatch(0,gain_mis_tmp,phase_mis_tmp,0,0,addr); //no mismatch
        bt_iqimb_dma_enable(st->M0data,fftsize);
        while(1)
        {
            if(iqimb_dma_status==0)
                break;
        }
        caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
        energy_ret2 = Energy1;
        // DRIVERS_TRACE(1,"IQ phase correct energy_ret2 = %d",energy_ret2);
        phase_mis_tmp = phase_mis_base - phasestep;
        bt_iqimb_add_mismatch(0,gain_mis_tmp,phase_mis_tmp,0,0,addr); //no mismatch
        bt_iqimb_dma_enable(st->M0data,fftsize);
        while(1)
        {
            if(iqimb_dma_status==0)
                break;
        }
        caculate_energy_main_test(st,&Energy,&Energy1,fftsize);
        energy_ret1 = Energy1;
        //DRIVERS_TRACE(1,"IQ phase correct energy_ret1 = %d",energy_ret1);
        tmp = energy_ret2 -2*energy_ret0 + energy_ret1;
        // DRIVERS_TRACE(1,"IQ phase correct energy_ret2 -2*energy_ret0 + energy_ret1 = %d",tmp);
        if(tmp>0)
        {
            tmp = (energy_ret2-energy_ret1)*phasestep/tmp/2;
            tmp = min(tmp,4*phasestep);
            phasestep = phasestep/2;
        }
        else
        {
            tmp = 0;
            iters = iters + 1;
        }
        if(iters>8)
            break;
        if((phase_mis_base - tmp>60)||(phase_mis_base - tmp<-60))
            tmp = 0;
        // DRIVERS_TRACE(2,"IQ phase correct phase_mis = %d ,phase_adj = %d",phase_mis_base,tmp);
        phase_mis_base = phase_mis_base - tmp;
        energy_ret0_last = energy_ret0;

    }
    return phase_mis_base;
}

static POSSIBLY_UNUSED void btIqCalibration(void)
{
}

void bt_iq_calibration_setup(void)
{
}

const uint16_t rfreg_store[][1] =
{
    {0xde},
    {0xdf},
    {0x80},
    {0x7f},
    {0x20},
};

#define TX_PULLING_CAL_CNT                 8
#define FFTSIZE 1024

void bt_tx_pulling_write_max_txpwr(const int cnt, const uint16_t phase_low, const uint16_t phase_high)
{
    uint16_t RF_REG1 = 0x164;
    uint16_t RF_REG2 = 0x17c;

    RF_REG1 = 0x164 + cnt;
    BTRF_REG_SET_FIELD(RF_REG1, 0x1FF, 0, phase_low);

    RF_REG2 = 0x17c + (cnt/2);
    if (cnt % 2 == 1) {
        BTRF_REG_SET_FIELD(RF_REG2, 0x7F, 7, phase_high);
    } else {
        BTRF_REG_SET_FIELD(RF_REG2, 0x7F, 0, phase_high);
    }

    DRIVERS_TRACE(5, "%s 0x%x=0x%x , 0x%x=0x%x", __func__, RF_REG1, phase_low, RF_REG2, phase_high);
}

void bt_tx_pulling_write_second_txpwr(const int cnt, const uint16_t phase_low, const uint16_t phase_high)
{
    uint16_t RF_REG1 = 0x16C;
    uint16_t RF_REG2 = 0x180;

    RF_REG1 = 0x16C + cnt;
    BTRF_REG_SET_FIELD(RF_REG1, 0x1FF, 0, phase_low);

    RF_REG2 = 0x180 + (cnt/2);
    if (cnt % 2 == 1) {
        BTRF_REG_SET_FIELD(RF_REG2, 0x7F, 7, phase_high);
    } else {
        BTRF_REG_SET_FIELD(RF_REG2, 0x7F, 0, phase_high);
    }

    DRIVERS_TRACE(5, "%s 0x%x=0x%x , 0x%x=0x%x", __func__, RF_REG1, phase_low, RF_REG2, phase_high);
}
#define TX_PULLING_CAL_CNT                 8

void tx_pulling_rf_dig_set(const uint16_t bbpll_sdm_freqword, const uint32_t rc_step, const uint32_t test_channel)
{
    DRIVERS_TRACE(4, "%s rf_de=0x%x, d0350248=0x%x, d0220c00=0x%x", __func__, bbpll_sdm_freqword, rc_step, test_channel);
    btdrv_write_rf_reg(0xDE, bbpll_sdm_freqword);

    btdrv_write_rf_reg(0xdf, 0x0001);
    hal_sys_timer_delay(MS_TO_TICKS(2));
    btdrv_write_rf_reg(0xdf, 0x0003);

    BTDIGITAL_REG_WR(0xd0350248, rc_step);
    BTDIGITAL_REG_WR(0xd0220c00, test_channel);
    // reset tx farrow filter
    BTDIGITAL_REG_WR(0xD0330058, 0x410000);
    BTDIGITAL_REG_WR(0xD0330060, 0x410000);
    BTDIGITAL_REG_WR(0xD0330064, 0x410000);
    hal_sys_timer_delay(MS_TO_TICKS(2));
}

uint8_t txPullingPhaseCnt_get(void)
{
    uint16_t rf_c4, rf_2d, rf_2a;
    uint8_t log_cal_value;

    btdrv_read_rf_reg(0xC4, &rf_c4);
    btdrv_read_rf_reg(0x2D, &rf_2d);
    btdrv_read_rf_reg(0x2A, &rf_2a);

    btdrv_write_rf_reg(0xC4, 0x7C89);
    btdrv_write_rf_reg(0x2D, 0x0322);
    BTRF_REG_SET_FIELD(0x2A, 1, 9, 0);
    hal_sys_timer_delay(MS_TO_TICKS(1));
    BTRF_REG_SET_FIELD(0x2A, 1, 9, 1);

    BTDIGITAL_REG_WR(0xD0220C00, 0xA004E);
    hal_sys_timer_delay(MS_TO_TICKS(1));

    BTRF_REG_GET_FIELD(0x58, 0xFF, 0, log_cal_value);
    if (log_cal_value > 235) {
        log_cal_value = 235;
    }

    DRIVERS_TRACE(2, "%s log_cal_value=%d.\n",__func__, log_cal_value);

    //reset
    BTDIGITAL_REG_WR(0xD0220C00, 0);
    btdrv_write_rf_reg(0xC4, rf_c4);
    btdrv_write_rf_reg(0x2D, rf_2d);
    btdrv_write_rf_reg(0x2A, rf_2a);

    return log_cal_value;
}

// pa bias scan range
const uint16_t pa_calib_rf_0xca[] = {
                0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
          0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
    0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
    0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
    0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
    0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
    0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
    0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
    0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF
};

static void bt_tx_pa_calib(void)
{
    const int fftsize = 1024;
    uint16_t rf_local = 0;
    uint32_t dig1, dig2 = 0;
    IQMismatchPreprocessState *st = NULL;
    uint16_t pa_calib_rf_0xca_size = 0;

    syspool_init_specific_size(MED_MEM_POOL_SIZE);
    syspool_get_buff(&g_medMemPool, MED_MEM_POOL_SIZE);
    med_heap_init(&g_medMemPool[0], MED_MEM_POOL_SIZE);

    st = IQMismatchPreprocessState_init(FFTSIZE);
    Tblgen_iq_2nd(st->Table0,FFTSIZE);
    //add by xulicheng
    int tmp_energy, tmp_energy1, min_energy = 1000000000;
    rf_local = 0x9640 + 3*0xA0;
    dig2     = 0x800a0003 + 3*0xA;
    dig1 = 0x80bea3f9;
    tx_pulling_rf_dig_set(rf_local, dig1, dig2);
    btdrv_write_rf_reg(0x2E, 0x40A0);
    //only for debug, do not open
    // BTDIGITAL_REG_WR(0x40086050, 0xfd012);
    // BTDIGITAL_REG_WR(0x40086008, 0xffffffff);
    // while (1)
    // {
    //     bt_rfimb_add_mismatch(0); //no mismatch
    //     bt_iqimb_dma_enable(st->M0data,fftsize);
    //     while(1) {
    //         if(iqimb_dma_status==0)
    //             break;
    //     }
    //     caculate_energy_main_test(st,&tmp_energy,&tmp_energy1,fftsize);
    //     DRIVERS_TRACE(1,"tmp_energy: %d ",tmp_energy1);
    //     hal_sys_timer_delay(MS_TO_TICKS(5000));
    // }
    int min_num = 0;
    pa_calib_rf_0xca_size = ARRAY_SIZE(pa_calib_rf_0xca);

    for(int ii=0; ii<pa_calib_rf_0xca_size; ii++)
    {
        BTRF_REG_SET_FIELD(0xCA, 0xFF, 8, pa_calib_rf_0xca[ii]);
        bt_rfimb_add_mismatch(160);     //fix log delay during pabias calib
        bt_iqimb_dma_enable(st->M0data,FFTSIZE);
        while(1) {
            if(iqimb_dma_status==0)
                break;
        }
        caculate_energy_main_test(st,&tmp_energy,&tmp_energy1,fftsize);
        if(min_energy>tmp_energy1) {
            min_num = ii;
            min_energy = tmp_energy1;
        }
#ifdef PA_BIAS_DEPUG
        DRIVERS_TRACE(0,"0xca:0x%x, energy:%d", pa_calib_rf_0xca[ii], tmp_energy1);
#endif
    }

    BTDIGITAL_REG_WR(0xd0220c00, 0x0);      //turn of tx on

    BTRF_REG_SET_FIELD(0xCA, 0xFF, 8, pa_calib_rf_0xca[min_num]);
    DRIVERS_TRACE(0,"%s, min_energy:0x%x, ca:0x%x", __func__, min_energy, pa_calib_rf_0xca[min_num]);
}

uint16_t bt_tx_pulling_get_phase_step(uint16_t phase_tmp)
{
    uint16_t phase_step = 0;

    if (phase_tmp < 64) {
        phase_step = 16;    //0~79
    } else if (phase_tmp < 88) {
        phase_step = 8;     //80~95
    } else if (phase_tmp < 124) {
        phase_step = 4;     //96~127
    } else if (phase_tmp < 200) {
        phase_step = 8;
    } else {
        phase_step = 4;
    }

    return phase_step;
}
#define TX_PULLING_CAL_IDX_5        4
#define TX_PULLING_CAL_IDX_4        3

static void bt_tx_pulling_algorithm1(uint8_t calib_tx_idx)
{
    uint8_t phase_count = 176;
    uint16_t rf_de_value = 0;
    uint32_t rc_step_val, test_chnl = 0;
    uint16_t phase_step = 0;

    IQMismatchPreprocessState *st1 = NULL;
    st1 = IQMismatchPreprocessState_init(FFTSIZE);
    Tblgen_iq_1st(st1->Table0,FFTSIZE);

    IQMismatchPreprocessState *st2 = NULL;
    st2 = IQMismatchPreprocessState_init(FFTSIZE);
    Tblgen_iq_2nd(st2->Table0,FFTSIZE);

    phase_count = txPullingPhaseCnt_get() + 16;
    if (phase_count > 234) {
        phase_count = 234;
    }

    for (int k = 0; k < TX_PULLING_CAL_CNT; k++) {
        rf_de_value = 0x9640 + k * 0xA0;
        test_chnl = 0x800A0003 + k * 0xA;

        if(k<=2) {
            rc_step_val = 0x80bb80d6 + k*0x10BB6;
        } else if((k>2) && (k<5)) {
            rc_step_val = 0x80bea3f9 + (k-3)*0x10BB6;
        } else {
            rc_step_val = 0x80c0bb66 + (k-5)*0x10BB6;
        }

        tx_pulling_rf_dig_set(rf_de_value, rc_step_val, test_chnl);

        int phase_tmp = 0 ;
        int phase_min_val = 0;
        int cur_energy = 1000000000;
        int Energy,Energy1,Energy2 = 0;

        //Get phase_min_val
        for (phase_tmp = 0; phase_tmp <= phase_count; phase_tmp += phase_step) {
            bt_rfimb_add_mismatch(phase_tmp);

            bt_iqimb_dma_enable(st1->M0data,FFTSIZE);
            while(1) {
                if(iqimb_dma_status==0)
                    break;
            }
            //check_mem_data((uint8_t *)st1->M0data, 3*1024);
            caculate_energy_main_test(st1,&Energy,&Energy1,FFTSIZE);

            bt_iqimb_dma_enable(st2->M0data,FFTSIZE);
            while(1) {
                if(iqimb_dma_status==0)
                    break;
            }
            //check_mem_data((uint8_t *)st2->M0data, 3*1024);
            caculate_energy_main_test(st2,&Energy,&Energy2,FFTSIZE);

            if (cur_energy > Energy1 + Energy2) {
                phase_min_val = phase_tmp;
                cur_energy = Energy1 + Energy2;
            }
            phase_step = bt_tx_pulling_get_phase_step(phase_tmp);
#ifdef PULLING_DEBUG
            DRIVERS_TRACE(0,"%s, phase_tmp:%d, energy1:%d, energy2:%d energy_num:%d", __func__,
            phase_tmp, Energy1, Energy2, Energy1 + Energy2);
#endif
        }

        BTDIGITAL_REG_WR(0xd0220c00, 0x0);      //turn of tx on

        DRIVERS_TRACE(3, "%s phase_min_val=0x%x, cur_energy=%d.", __func__, phase_min_val, cur_energy);

        if (calib_tx_idx == TX_PULLING_CAL_IDX_5) {
            bt_tx_pulling_write_max_txpwr(k, (phase_min_val & 0x1FF), (phase_min_val & 0x7F));      //calib max tx pwr idx4
        } else if (calib_tx_idx == TX_PULLING_CAL_IDX_4) {
            bt_tx_pulling_write_second_txpwr(k, (phase_min_val & 0x1FF), (phase_min_val & 0x7F));   //calib second tx pwr idx3
        }
    }
}

void bt_iqimb_test_ex(int mismatch_type)
{
    POSSIBLY_UNUSED uint32_t time_start = hal_sys_timer_get();
    uint8_t calib_tx_idx;

    //dr tx gain
    BTRF_REG_SET_FIELD(0x24, 0x1, 4, 0x1);      // rf reg 0x24 resetore in func btdrv_tx_pulling_cal

    calib_tx_idx = TX_PULLING_CAL_IDX_5;
    BTRF_REG_SET_FIELD(0x24, 0x7, 5, 0x3);      //dr tx gain idx3
    bt_tx_pa_calib();
    bt_tx_pulling_algorithm1(calib_tx_idx);


    DRIVERS_TRACE(1,"use time: %d ms", __TICKS_TO_MS(hal_sys_timer_get()-time_start));
}

static float get_DC_energy(uint32_t addr, short dc_i, short dc_q,int num)
{
    float E_ave = 0;
    uint8_t calib_time = 0;
calib_again:
    for(int cc = 0; cc < num; cc++) {
        int32_t i_data, q_data = 0;

        BTDIGITAL_REG_SET_FIELD(addr, 0x3FF, 0,  (dc_i & 0x3FF));
        BTDIGITAL_REG_SET_FIELD(addr, 0x3FF, 16, (dc_q & 0x3FF));

        hal_sys_timer_delay(US_TO_TICKS(1));

        BTDIGITAL_REG_SET_FIELD(0xD0350334, 0x1, 30, 0);    //reset start mix
        BTDIGITAL_REG_SET_FIELD(0xD0350334, 0x1, 30, 1);    //start mix

        hal_sys_timer_delay(US_TO_TICKS(400));
        i_data = BTDIGITAL_REG(0xD0350338) & 0x3FFFFF;
        q_data = BTDIGITAL_REG(0xD035033C) & 0x3FFFFF;

        if(0x200000 <= i_data)
            i_data -= 0x400000;
        if(0x200000 <= q_data)
            q_data -= 0x400000;
        i_data = i_data/8;
        q_data = q_data/8;
        float E = sqrtf(i_data*i_data + q_data*q_data);
        //DRIVERS_TRACE(3,"geti=%d,q=%d,e_tmp=%d",dc_i,dc_q,(int)E);
        E_ave += E;
    }

    if (calib_time == 3)
        goto exit;

    if (E_ave == 0) {
        BTRF_REG_SET_FIELD(0x2C, 0x7, 1, 0x0);  // reduce adc gain
        calib_time++;
        goto calib_again;
    }
exit:
    E_ave = E_ave / num;
    return E_ave;
}

static void set_gain_phase(uint32_t addr, short gain, short phase)
{
    BTDIGITAL_REG_SET_FIELD(0xD0350250, 0x1, 31, 0);

    BTDIGITAL_REG_SET_FIELD(addr, 0xFFF, 0,  (gain & 0xFFF));
    BTDIGITAL_REG_SET_FIELD(addr, 0xFFF, 16, (phase & 0xFFF));
}

static float get_iq_energy(uint32_t addr, short gain,short phase,int num)
{
    float E_ave = 0;
    uint8_t calib_time = 0;
calib_again:
    for(int cc = 0; cc < num; cc++) {
        int32_t i_data, q_data = 0;

        BTDIGITAL_REG_SET_FIELD(0xD0350250, 0x1, 31, 0);

        BTDIGITAL_REG_SET_FIELD(addr, 0xFFF, 0,  (gain & 0xFFF));
        BTDIGITAL_REG_SET_FIELD(addr, 0xFFF, 16, (phase & 0xFFF));
        hal_sys_timer_delay(US_TO_TICKS(1));

        BTDIGITAL_REG_WR(0xD0330058, 0x00800000);
        BTDIGITAL_REG_WR(0xD0330060, 0xFFFFFFFF);       //rst
        BTDIGITAL_REG_WR(0xD0330064, 0xFFFFFFFF);       //clr rst
        BTDIGITAL_REG_SET_FIELD(0xD0350250, 0x1, 31, 1);

        BTDIGITAL_REG_SET_FIELD(0xD0350334, 0x1, 30, 0);//reset start mix
        BTDIGITAL_REG_SET_FIELD(0xD0350334, 0x1, 30, 1);//start mix

        hal_sys_timer_delay(US_TO_TICKS(400));
        i_data = BTDIGITAL_REG(0xD0350338) & 0x3FFFFF;
        q_data = BTDIGITAL_REG(0xD035033C) & 0x3FFFFF;
        //DRIVERS_TRACE(2,"i=0x%x,q=0x%x",i_data,q_data);
        if(0x200000 <= i_data)
            i_data -= 0x400000;
        if(0x200000 <= q_data)
            q_data -= 0x400000;
        i_data = i_data/8;
        q_data = q_data/8;
        float E = sqrtf(i_data*i_data + q_data*q_data);
        //DRIVERS_TRACE(2,"i=%d,q=%d",i_data,q_data);
#ifdef IQ_DEBUG
        //DRIVERS_TRACE(3,"n=%d,g=%d,p=%d,e=%d",cc,gain,phase,(int)E);
#endif
        E_ave += E;
    }

    if (calib_time == 3)
        goto exit;

    if (E_ave == 0) {
        BTRF_REG_SET_FIELD(0x2C, 0x7, 1, 0x0);  // reduce adc gain
        calib_time++;
        goto calib_again;
    }
exit:
    E_ave = E_ave / num;
    return E_ave;
}


static int get_best_i(int min_i,float min_energy,int base_q,int step,int* im, float* em)
{
    float energy_left = 0;
    float energy_right = 0;
    float energy_middle = 0;
    uint32_t e_addr = 0xD0350250;
    int i_left = 0;
    int i_right = 0;
    int i_middle = 0;
    int base_i = 0;
    i_left = min_i - step;
    energy_left = get_DC_energy(e_addr, i_left, base_q, 1);
    i_right = min_i + step;
    energy_right = get_DC_energy(e_addr, i_right, base_q, 1);
    while(1) {
        i_middle = (i_left + i_right) / 2;
        if(ABS(i_left - i_right) < 17) {
            energy_middle = get_DC_energy(e_addr, i_middle, base_q, 3);

        } else {
            energy_middle = get_DC_energy(e_addr, i_middle, base_q, 2);
        }
#ifdef DC_DEBUG
        DRIVERS_TRACE(4,"Mi=%d,q=%d,e=%d,DC_Emin=%d",i_middle,base_q,(int)energy_middle,(int)*em);
        DRIVERS_TRACE(4,"Li=%d,ri=%d,le=%d,re=%d",i_left,i_right,(int)energy_left,(int)energy_right);
#endif
        if(energy_left > energy_right) {
            i_left = i_middle;
            energy_left = energy_middle;
            min_energy = energy_right;
            min_i = i_right;
        } else {
            i_right = i_middle;
            energy_right = energy_middle;
            min_energy = energy_left;
            min_i = i_left;
        }
        if(min_energy < *em) {
            *im = min_i;
            *em = min_energy;
        }
#ifdef DC_DEBUG
        //DRIVERS_TRACE(4,"DICHOTOMY_best_i=%d,best_q=%d,energy_min=%d,DC_Emin=%d",min_i,base_q,(int)min_energy,(int)*em);
#endif
        if(ABS(i_left - i_right) < 4)
            break;
    }

    if(energy_left > energy_right) {
        min_energy = energy_right;
        min_i = i_right;
    } else {
        min_energy = energy_left;
        min_i = i_left;
    }
    if(min_energy < *em) {
        *im = min_i;
        *em = min_energy;
    }
#ifdef DC_DEBUG
    DRIVERS_TRACE(4,"LAST_i=%d,q=%d,e=%d,DC_Emin=%d",min_i,base_q,(int)min_energy,(int)*em);
#endif
    if(min_i != *im) {
        *em = get_DC_energy(e_addr, *im, base_q, 3);
        if(*em > min_energy) {
            *im = min_i;
            *em = min_energy;
        }
    }
    base_i = *im;
    return base_i;
}

static int get_best_q(int min_q,float min_energy,int base_i,int step,int* qm, float* em)
{
    float energy_left = 0;
    float energy_right = 0;
    float energy_middle = 0;
    uint32_t e_addr = 0xD0350250;
    int q_left = 0;
    int q_right = 0;
    int q_middle = 0;
    int base_q = 0;
    q_left = min_q - step;
    energy_left = get_DC_energy(e_addr, base_i, q_left, 1);
    q_right = min_q + step;
    energy_right = get_DC_energy(e_addr, base_i, q_right, 1);

    while(1) {
        q_middle = (q_left+q_right) / 2;
        if(ABS(q_left - q_right) < 17) {
            energy_middle = get_DC_energy(e_addr, base_i, q_middle, 3);
        } else {
            energy_middle = get_DC_energy(e_addr, base_i, q_middle, 2);
        }
#ifdef DC_DEBUG
        DRIVERS_TRACE(4,"Mi=%d,q=%d,e=%d,DC_Emin=%d",base_i,q_middle,(int)energy_middle,(int)*em);
#endif
#ifdef DC_DEBUG
        DRIVERS_TRACE(4,"Lq=%d,rq=%d,le=%d,re=%d",q_left,q_right,(int)energy_left,(int)energy_right);
#endif
        if(energy_left > energy_right) {
            q_left = q_middle;
            energy_left = energy_middle;
            min_energy = energy_right;
            min_q = q_right;
        } else {
            q_right = q_middle;
            energy_right = energy_middle;
            min_energy = energy_left;
            min_q = q_left;
        }
        if(min_energy < *em) {
            *qm = min_q;
            *em = min_energy;
        }
#ifdef DC_DEBUG
        //DRIVERS_TRACE(4,"DICHOTOMY_best_i=%d,best_q=%d,energy_min=%d,DC_Emin=%d",base_i,min_q,(int)min_energy,(int)*em);
#endif
        if(ABS(q_left - q_right) < 4)
            break;
    }

    if(energy_left > energy_right) {
        min_energy = energy_right;
        min_q = q_right;
    } else {
        min_energy = energy_left;
        min_q = q_left;
    }
    if(min_energy < *em) {
        *qm = min_q;
        *em = min_energy;
    }
#ifdef DC_DEBUG
    DRIVERS_TRACE(4,"LAST_i=%d,q=%d,e=%d,DC_Emin=%d",base_i,min_q,(int)min_energy,(int)*em);
#endif
    if(min_q != *qm) {
        *em = get_DC_energy(e_addr, base_i, *qm, 3);
        if(*em > min_energy) {
            *qm = min_q;
            *em = min_energy;
        }
    }
    base_q = *qm;
    return base_q;
}

static int get_best_gain(uint32_t addr,int min_gain,float min_energy,int base_phase,int step,int* gm, float* em)
{
    float energy_left = 0;
    float energy_right = 0;
    float energy_middle = 0;
    uint32_t en_addr = addr;
    int gain_left = 0;
    int gain_right = 0;
    int gain_middle = 0;
    int base_gain = 0;
    gain_left = min_gain - step;
    energy_left = get_iq_energy(en_addr, gain_left, base_phase, 2);
    gain_right = min_gain + step;
    energy_right = get_iq_energy(en_addr, gain_right, base_phase, 2);
    while(1) {
        gain_middle = (gain_left+gain_right) / 2;
        if(ABS(gain_left - gain_right) < 8) {
            energy_middle = get_iq_energy(en_addr, gain_middle, base_phase, 4);
        } else {
            energy_middle = get_iq_energy(en_addr, gain_middle, base_phase, 2);
        }
#ifdef IQ_DEBUG
        DRIVERS_TRACE(4,"Mg=%d,p=%d,e=%d,IQ_Emin=%d",gain_middle,base_phase,(int)energy_middle,(int)*em);
        DRIVERS_TRACE(4,"Lg=%d,rg=%d,le=%d,re=%d",gain_left,gain_right,(int)energy_left,(int)energy_right);
#endif
        if(energy_left > energy_right) {
            gain_left = gain_middle;
            energy_left = energy_middle;
            min_energy = energy_right;
            min_gain = gain_right;
        } else {
            gain_right = gain_middle;
            energy_right = energy_middle;
            min_energy = energy_left;
            min_gain = gain_left;
        }
        if(min_energy < *em) {
            *gm = min_gain;
            *em = min_energy;
        }

#ifdef IQ_DEBUG
        //DRIVERS_TRACE(4,"DICHOTOMY_best_gain=%d,best_phase=%d,energy_min=%d,IQ_Emin=%d",min_gain,base_phase,(int)min_energy,(int)*em);
#endif
        if(ABS(gain_left-gain_right) < 2)
            break;
    }

    if(energy_left > energy_right) {
        min_energy = energy_right;
        min_gain = gain_right;
    } else {
        min_energy = energy_left;
        min_gain = gain_left;
    }
    if(min_energy < *em) {
        *gm = min_gain;
        *em = min_energy;
    }
#ifdef IQ_DEBUG
    //DRIVERS_TRACE(4,"LAST_DICHOTOMY_best_gain=%d,best_phase=%d,energy_min=%d,IQ_Emin=%d",min_gain,base_phase,(int)min_energy,(int)*em);
#endif
    if(min_gain != *gm) {
        *em = get_DC_energy(en_addr, *gm, base_phase, 3);
        if(*em > min_energy) {
            *gm = min_gain;
            *em = min_energy;
        }
    }
    base_gain = *gm;
    return base_gain;
}

static int get_best_phase(uint32_t addr,int min_phase,float min_energy,int base_gain,int step,int* pm, float* em)
{
    float energy_left = 0;
    float energy_right = 0;
    float energy_middle = 0;
    uint32_t en_addr = addr;
    int phase_left = 0;
    int phase_right = 0;
    int phase_middle = 0;
    int base_phase = 0;
    phase_left = min_phase - step;
    energy_left = get_iq_energy(en_addr, base_gain, phase_left, 2);
    phase_right = min_phase + step;
    energy_right = get_iq_energy(en_addr, base_gain, phase_right, 2);
    while(1) {
        phase_middle = (phase_left + phase_right) / 2;
        if(ABS(phase_left - phase_right) < 8) {
            energy_middle = get_iq_energy(en_addr,base_gain,phase_middle,4);
        } else {
            energy_middle = get_iq_energy(en_addr,base_gain,phase_middle,2);
        }
#ifdef IQ_DEBUG
        DRIVERS_TRACE(4,"Mg=%d,p=%d,e=%d,IQ_Emin=%d",base_gain,phase_middle,(int)energy_middle,(int)*em);
        DRIVERS_TRACE(4,"Lp=%d,rp=%d,le=%d,re=%d",phase_left,phase_right,(int)energy_left,(int)energy_right);
#endif
        if(energy_left > energy_right) {
            phase_left = phase_middle;
            energy_left = energy_middle;
            min_energy = energy_right;
            min_phase = phase_right;
        } else {
            phase_right = phase_middle;
            energy_right = energy_middle;
            min_energy = energy_left;
            min_phase = phase_left;
        }
        if(min_energy < *em) {
            *pm = min_phase;
            *em = min_energy;
        }
#ifdef IQ_DEBUG
        //DRIVERS_TRACE(4,"DICHOTOMY_best_gain=%d,best_phase=%d,energy_min=%d,IQ_Emin=%d",base_gain,min_phase,(int)min_energy,(int)*em);
#endif
        if(ABS(phase_left - phase_right) < 2)
            break;
    }

    if(energy_left > energy_right) {
        min_energy = energy_right;
        min_phase = phase_right;
    } else {
        min_energy = energy_left;
        min_phase = phase_left;
    }
    if(min_energy < *em) {
        *pm = min_phase;
        *em = min_energy;
    }
#ifdef IQ_DEBUG
    //DRIVERS_TRACE(4,"LAST_DICHOTOMY_best_gain=%d,best_phase=%d,energy_min=%d,IQ_Emin=%d",base_gain,min_phase,(int)min_energy,(int)*em);
#endif
    if(min_phase != *pm) {
        *em = get_DC_energy(en_addr, base_gain, *pm, 3);
        if(*em > min_energy) {
            *pm = min_phase;
            *em = min_energy;
        }
    }
    base_phase = *pm;
    return base_phase;
}

#define ENERGY_LIMIT (10)

int dc_energy_limit(int *dc_iq)
{
    int dc_iq_loc;

    dc_iq_loc = *dc_iq;
    DRIVERS_TRACE(0, "dc_energy_limit dc_iq:%08x->%d\n", *dc_iq, dc_iq_loc);

    if (dc_iq_loc > ENERGY_LIMIT){
        dc_iq_loc = 0;
    }else if (dc_iq_loc < -ENERGY_LIMIT){
        dc_iq_loc = 0;
    }

    DRIVERS_TRACE(0, "dc_energy_limit out dc_iq:%08x->%d\n", dc_iq_loc & 0x3ff, dc_iq_loc);

    return (dc_iq_loc & 0x3ff);
}

void dc_iq_calib_1501p(void)
{
    enum HAL_CHIP_METAL_ID_T metal_id;
    metal_id = hal_get_chip_metal_id();
    POSSIBLY_UNUSED uint32_t time_start = hal_sys_timer_get();
    BTDIGITAL_REG_SET_FIELD(0xD0350334, 0x1, 23, 0);
    if(metal_id == HAL_CHIP_METAL_ID_0)
    {
        BTDIGITAL_REG_WR(0xD0350028, 0x001F0012);   //one tone
    }
    else
    {
        BTDIGITAL_REG_WR(0xD0350028, 0x007F0012);   //one tone
    }
    BTDIGITAL_REG_WR(0xD035002C, 0x7);          //320K

#ifdef DC_CALIB
    int DC_i_min = 0;
    int DC_q_min = 0;
    uint32_t e_addr = 0xD0350250;
    float DC_energy_min = 1000000;

    int base_i = 0;
    int base_q = 0;
    int min_i = 0;
    int min_q = 0;
    int step = SWEEP_STEP;
    float min_energy = 1000000;

    for(int q1=DC_RANGE_MIN; q1<=DC_RANGE_MAX; q1+=step) {
        float e_tmp=get_DC_energy(e_addr,base_i,q1,1);
        if(min_energy > e_tmp) {
            min_q = q1;
            min_energy = e_tmp;
            DC_q_min = min_q;
            DC_energy_min = min_energy;
        }
#ifdef DC_DEBUG
    //DRIVERS_TRACE(4,"i=%d,q=%d,e_tmp=%d,DC_Emin=%d",base_i,q1,(int)e_tmp,(int)DC_energy_min);
#endif
    }
#ifdef DC_DEBUG
    DRIVERS_TRACE(4,"--------------SWEEP q=%d,e_tmp=%d--------------",min_q,(int)min_energy);
#endif

    base_q=get_best_q(min_q,min_energy,base_i,step,&DC_q_min,&DC_energy_min);

#ifdef DC_DEBUG
    DRIVERS_TRACE(2,"Step1:best_q=%d,energy_min=%d",base_q,(int)DC_energy_min);
#endif

    if(DC_ENERGY_THD < DC_energy_min) {
        min_energy = 1000000;
        for(int i1=DC_RANGE_MIN; i1<=DC_RANGE_MAX; i1+=step){
            float e_tmp=get_DC_energy(e_addr,i1,base_q,1);
            if(min_energy > e_tmp) {
                min_i = i1;
                min_energy = e_tmp;
                DC_i_min = min_i;
                DC_energy_min = min_energy;
            }
#ifdef DC_DEBUG
            //DRIVERS_TRACE(4,"i=%d,q=%d,e_tmp=%d,DC_Emin=%d",i1,base_q,(int)e_tmp,(int)DC_energy_min);
#endif
        }
#ifdef DC_DEBUG
        DRIVERS_TRACE(4,"--------------SWEEP i=%d,e_tmp=%d--------------",min_i,(int)min_energy);
#endif
        base_i=get_best_i(min_i,min_energy,base_q,step,&DC_i_min,&DC_energy_min);
#ifdef DC_DEBUG
        DRIVERS_TRACE(3,"Step2:abest_i=%d,best_q=%d,energy_min=%d",base_i,base_q,(int)DC_energy_min);
#endif
        if(DC_ENERGY_THD < DC_energy_min) {
            base_q=get_best_q(min_q,min_energy,base_i,step,&DC_q_min,&DC_energy_min);
#ifdef DC_DEBUG
            DRIVERS_TRACE(3,"Step3:abest_i=%d,best_q=%d,energy_min=%d",base_i,base_q,(int)DC_energy_min);
#endif
        }
    }
#endif

    DRIVERS_TRACE(3,"FINAL DC best_i=%d,best_q=%d", base_i,base_q);
    base_i = dc_energy_limit(&base_i);
    base_q = dc_energy_limit(&base_q);
    POSSIBLY_UNUSED float teste=get_DC_energy(e_addr,base_i,base_q,1);
    DRIVERS_TRACE(3,"FINAL limit DC best_i=%d,best_q=%d,e=%d,energy_min=%d", base_i,base_q,(int)teste,(int)DC_energy_min);

    //IQ
    BTDIGITAL_REG_SET_FIELD(0xD0350334, 0x1, 23, 0);
    BTDIGITAL_REG_WR(0xD035002C, 0x3);          //160K
#ifdef IQ_CALIB
    uint32_t en_addr_base = 0xD0310000;
    uint32_t en_addr = 0x0;
    for (int k = 0; k<3; k++) {
        uint32_t freq_add = 0x80020000;
        if(k==0) {
            freq_add += 13;
            BTDIGITAL_REG_WR(0xD0220C00, freq_add);
            en_addr = en_addr_base + 13 * 4;
        } else if(k==1) {
            freq_add += 39;
            BTDIGITAL_REG_WR(0xD0220C00, freq_add);
            en_addr = en_addr_base + 39 * 4;
        } else {
            freq_add += 65;
            BTDIGITAL_REG_WR(0xD0220C00, freq_add);
            en_addr = en_addr_base + 65 * 4;
        }
        // reset tx farrow filter
        BTDIGITAL_REG_WR(0xD0330058, 0x410000);
        BTDIGITAL_REG_WR(0xD0330060, 0x410000);
        BTDIGITAL_REG_WR(0xD0330064, 0x410000);
        int iq_gain_min = 0;
        int iq_phase_min = 0;
        float iq_energy_min = 1000000;
        int base_phase = 0;
        int base_gain = 0;
        int min_phase = 0;
        int min_gain = 0;
        step = SWEEP_STEP;
        min_energy = 1000000;
        for(int gain1=IQ_RANGE_MIN; gain1<=IQ_RANGE_MAX; gain1+=step) {
            float e_tmp=get_iq_energy(en_addr,gain1,base_phase,1);
            if(min_energy > e_tmp) {
                min_gain = gain1;
                min_energy = e_tmp;
                iq_gain_min = min_gain;
                iq_energy_min = min_energy;
            }
#ifdef IQ_DEBUG
        //DRIVERS_TRACE(4,"gain=%d,phase=%d,e_tmp=%d,IQ_Emin=%d",gain1,base_phase,(int)e_tmp,(int)iq_energy_min);
#endif
        }
#ifdef IQ_DEBUG
        DRIVERS_TRACE(4,"--------------SWEEP gain=%d,e_tmp=%d--------------",min_gain,(int)min_energy);
#endif
        base_gain=get_best_gain(en_addr,min_gain,min_energy,base_phase,step,&iq_gain_min,&iq_energy_min);
#ifdef IQ_DEBUG
        DRIVERS_TRACE(2,"Step1:best_gain=%d,energy_min=%d",base_gain,(int)iq_energy_min);
#endif

        if(IQ_ENERGY_THD < iq_energy_min) {
            min_energy = 1000000;
            for(int phase1=IQ_RANGE_MIN; phase1<=IQ_RANGE_MAX; phase1+=step) {
                float e_tmp=get_iq_energy(en_addr,base_gain,phase1,1);
                if(min_energy > e_tmp) {
                    min_phase = phase1;
                    min_energy = e_tmp;
                    iq_phase_min = min_phase;
                    iq_energy_min = min_energy;
                }
#ifdef IQ_DEBUG
            //DRIVERS_TRACE(4,"gain=%d,phase=%d,e_tmp=%d,IQ_Emin=%d",base_gain,phase1,(int)e_tmp,(int)iq_energy_min);
#endif
            }
#ifdef IQ_DEBUG
            DRIVERS_TRACE(4,"--------------SWEEP phase=%d,e_tmp=%d--------------",min_phase,(int)min_energy);
#endif
            base_phase=get_best_phase(en_addr,min_phase,min_energy,base_gain,step,&iq_phase_min,&iq_energy_min);
#ifdef IQ_DEBUG
            DRIVERS_TRACE(3,"Step2:best_gain=%d,best_phase=%d,energy_min=%d",base_gain,base_phase,(int)iq_energy_min);
#endif

            if(IQ_ENERGY_THD < iq_energy_min) {
                base_gain=get_best_gain(en_addr,min_gain,min_energy,base_phase,step,&iq_gain_min,&iq_energy_min);
#ifdef IQ_DEBUG
                DRIVERS_TRACE(3,"Step3:best_gain=%d,best_phase=%d,energy_min=%d",base_gain,base_phase,(int)iq_energy_min);
#endif
            }
        }

        POSSIBLY_UNUSED float testen=get_iq_energy(en_addr,base_gain,base_phase,1);
        DRIVERS_TRACE(3,"FINAL %d,best_g=%d,best_p=%d,e=%d,energy_min=%d", k,base_gain,base_phase,(int)testen,(int)iq_energy_min);

        int ch1 = 0;
        int ch2 = 0;
        if(k==0) {
            ch1= 0;
            ch2= 25;
        } else if(k==1) {
            ch1= 26;
            ch2= 51;
        } else {
            ch1= 52;
            ch2= 78;
        }
        for (int ch = ch1; ch <= ch2; ch++) {
            uint32_t addre=en_addr_base+ch*4;
            set_gain_phase(addre,base_gain,base_phase);
        }
#ifdef IQ_DEBUG
        DRIVERS_TRACE(3,"aaaaaaa--FINAL %d,best_g=%d,best_p=%d--aaaaaaa", k,base_gain,base_phase);
#endif
    }
#endif
    //common RF register init
    if(metal_id == HAL_CHIP_METAL_ID_0)
    {
        BTDIGITAL_REG_SET_FIELD(0xD0350250, 0x1, 31, 0);
    }
    else
    {
        BTDIGITAL_REG_SET_FIELD(0xD0350250, 0x1, 31, 1);
    }

    DRIVERS_TRACE(1,"use time: %d ms", __TICKS_TO_MS(hal_sys_timer_get()-time_start));
}

#endif
#endif

