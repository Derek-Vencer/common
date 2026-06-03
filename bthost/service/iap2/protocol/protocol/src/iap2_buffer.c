#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "iap2_buffer.h"

#include "iap2_link.h"

#include "bt_common_define.h"

#define IAP2_REC_BUFFER_POOL_NUM    (10)

IAP2_REC_PCK *iap2_rec_pck = NULL;
IAP2_REC_PCK *iap2_rec_pck_s = NULL;   // start of iap2_rec_pck_data
IAP2_REC_PCK *iap2_rec_pck_e = NULL;   // end of iap2_rec_pck_data
IAP2_REC_PCK *iap2_rec_pck_push = NULL;
IAP2_REC_PCK *iap2_rec_pck_pull = NULL;

IAP2_REC_PCK iap2_rec_pck_data[IAP2_REC_BUFFER_POOL_NUM];

int iap2_receive_buffer_init() {
    int i = 0;
    iap2_rec_pck = (IAP2_REC_PCK *)&iap2_rec_pck_data;
    
    iap2_rec_pck_s      = iap2_rec_pck;
    iap2_rec_pck_e      = iap2_rec_pck + IAP2_REC_BUFFER_POOL_NUM - 1;
    iap2_rec_pck_push   = iap2_rec_pck;
    iap2_rec_pck_pull   = iap2_rec_pck;
    
    for(i = 0; i < IAP2_REC_BUFFER_POOL_NUM; i++)
    {
        (iap2_rec_pck + i)->pckValid  = FALSE;
        (iap2_rec_pck + i)->pckLength = 0;
    }
    DEBUG_INFO(0, "%s",__func__);
    return 0;
}

int iap2_push_data_to_rec_pool(uint8_t *buf, uint16_t len)
{
    int ret = -1;
    int i = 0;
    if ((buf == NULL) || (len > IAP2_TRANS_TOTAL_LEN) || (iap2_rec_pck_push == NULL))
    {
        DEBUG_INFO(0, "buf %p len %d rec_pck %p", buf, len, iap2_rec_pck_push);
        return ret;
    }

    DEBUG_INFO(0, "push iap2_rec_pck_push->pckValid = %d len = %d addr = %p", iap2_rec_pck_push->pckValid, len,iap2_rec_pck_push);

    if (!(iap2_rec_pck_push->pckValid))
    {
        memcpy(iap2_rec_pck_push->pckData, buf, len);
        iap2_rec_pck_push->pckValid = TRUE;
        iap2_rec_pck_push->pckLength = len;
        if (iap2_rec_pck_push != iap2_rec_pck_e)
        {
            iap2_rec_pck_push++;
        }
        else
        {
            iap2_rec_pck_push = iap2_rec_pck_s;
        }
        ret = 0;
        wake_up_recv_process();
        return ret;
    }

    for (i = 0; i < IAP2_REC_BUFFER_POOL_NUM; i++)
    {
        if (iap2_rec_pck_push != iap2_rec_pck_e)
        {
            iap2_rec_pck_push++;
        }
        else
        {
            iap2_rec_pck_push = iap2_rec_pck_s;
        }

        if (!(iap2_rec_pck_push->pckValid))
        {
            memcpy(iap2_rec_pck_push->pckData, buf, len);
            iap2_rec_pck_push->pckValid = TRUE;
            iap2_rec_pck_push->pckLength = len;

            if (iap2_rec_pck_push != iap2_rec_pck_e)
            {
                iap2_rec_pck_push++;
            }
            else
            {
                iap2_rec_pck_push = iap2_rec_pck_s;
            }
            ret = 0;
            wake_up_recv_process();
            return ret;
        }
    }

    DEBUG_INFO(0, "cant fine valid rec_pck");
    return ret;
}

IAP2_REC_PCK * iap2_pull_data_from_rec_pool(void)
{
    IAP2_REC_PCK *pRet = NULL;
    int i = 0;

    if (iap2_rec_pck_pull == NULL)
    {
        DEBUG_INFO(0, "iap2_rec_pck_pull == null");
        return pRet;
    }

    //DEBUG_INFO(0, "begin pull iap2_rec_pck_pull->pckValid = %d len = %d , %p\n", iap2_rec_pck_pull->pckValid, iap2_rec_pck_pull->pckLength,iap2_rec_pck_pull);

    if (iap2_rec_pck_pull->pckValid)
    {
        pRet = iap2_rec_pck_pull;
        if (iap2_rec_pck_pull != iap2_rec_pck_e)
        {
            iap2_rec_pck_pull++;
        }
        else
        {
            iap2_rec_pck_pull = iap2_rec_pck_s;
        }
    }
    else
    {
        for (i = 0; i < IAP2_REC_BUFFER_POOL_NUM; i++)
        {
            if (iap2_rec_pck_pull != iap2_rec_pck_e)
            {
                iap2_rec_pck_pull++;
            }
            else
            {
                iap2_rec_pck_pull = iap2_rec_pck_s;
            }
            if (iap2_rec_pck_pull->pckValid)
            {
                pRet = iap2_rec_pck_pull;
                if (iap2_rec_pck_pull != iap2_rec_pck_e)
                {
                    iap2_rec_pck_pull++;
                }
                else
                {
                    iap2_rec_pck_pull = iap2_rec_pck_s;
                }
                break;
            }
        }
    }

    if (pRet != NULL)
    {
        //DEBUG_INFO(0, "end pull pRet->pckValid = %d len = %d\n", pRet->pckValid, pRet->pckLength);
    }
    else
    {
        DEBUG_INFO(0, "iap2_rec_pck_pull is empty");
    }
    //DEBUG_INFO(0, "end pull iap2_rec_pck_pull->pckValid = %d len = %d\n", iap2_rec_pck_pull->pckValid, iap2_rec_pck_pull->pckLength);

    return pRet;
}

void iap2_set_all_receive_packet_invalid() {
    DEBUG_INFO(0, "%s", __func__);
    iap2_receive_buffer_init();
}


void iap2_free_packet(IAP2_REC_PCK *iap2_rec_pck)
{
    iap2_rec_pck->pckValid = false;
    iap2_rec_pck->pckLength = 0;
}
