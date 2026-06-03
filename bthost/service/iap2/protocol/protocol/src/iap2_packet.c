#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string.h>
#include <unistd.h>

#include "iap2_buffer.h"
#include "iap2_packet.h"
#include "iap2_link.h"

#include "bt_common_define.h"

void iAP2PacketResetPacket (iAP2Packet_t* pck);
bool iAP2PacketCheckDetect (iAP2Packet_t* pck);
uint8_t iAP2PacketCalcChecksum (const uint8_t*  buffer,
                                uint32_t        bufferLen);

/*
****************************************************************
**
**  iAP2PacketResetPacket
**
**  Input:
**      pck:                packet to reset
**      bFreePrivateData    free private data buffer as well if bFreeBuffers is TRUE
**
**  Output:
**      None
**
**  Return:
**      None
**
****************************************************************
*/
void iAP2PacketResetPacket (iAP2Packet_t* pck)
{
    DEBUG_INFO(0, "iAP2PacketResetPacket");
    pck->payloadLen     = 0;
    pck->payloadoffset  = NULL;
    pck->dataCurLen     = 0;
    pck->dataChecksum   = 0;
    pck->bufferLen      = 0;
    pck->packetLen      = 0;
    pck->timeStamp      = 0;
    pck->seqPlus        = 0;
    pck->state          = kiAP2PacketParseStateSOP1;
    /* clear the packet header information */
    memset (pck->pckData, 0, kIAP2PacketHeaderLen*sizeof(uint8_t));
    pck->cbContext      = NULL;
    pck->callbackOnSend = NULL;
    pck->recvEAK        = FALSE;
    pck->retransmitCount = 0;
}

/*
****************************************************************
**
**  iAP2PacketCheckDetect
**
**  Input:
**      pck:    packet to delete
**
**  Output:
**      None
**
**  Return:
**      BOOL    return TRUE if iAP 1.0/2.0 Detect packet detected,
**              else returne FALSE
**
****************************************************************
*/
bool iAP2PacketCheckDetect (iAP2Packet_t* pck)
{
    return (kIAP2PacketDetectLEN  == pck->packetLen &&
            kIAP2PacketDetectCTRL == pck->pckData->ctl &&
            kIAP2PacketDetectSEQ  == pck->pckData->seq);
}


bool iAP2PacketCheckDetectNack (iAP2Packet_t* pck)
{
    return (kIAP2PacketDetectLEN  == pck->packetLen &&
            kIAP1PacketDetectNackCTRL == pck->pckData->ctl &&
            kIAP1PacketDetectNACKSEQ  == pck->pckData->seq);
}


/*
****************************************************************
**
**  iAP2PacketCalcChecksum
**
**  Input:
**      buffer:     Input buffer containing data to Checksum.
**      bufferLen:  Size of the Input buffer data.
**
**  Output:
**      None
**
**  Return:
**      uint8_t     Calculated checksum for provide buffer
**
****************************************************************
*/
uint8_t iAP2PacketCalcChecksum (const uint8_t*  buffer,
                                uint32_t        bufferLen)
{
    uint8_t checksum = 0;
    uint32_t count = 0;

    if (buffer != NULL && bufferLen > 0)
    {
        const uint8_t* bufferEnd = buffer + bufferLen;
        //roav_info("check start bufferLen = %d", bufferLen);
        while (buffer != bufferEnd)
        {
            checksum += *buffer; /* add to checksum */
            //printf("0x%02X ", *buffer);
            buffer++;
            count++;
        } 
        checksum = (uint8_t)(0x100 - checksum); /* 2's complement negative checksum */
    }
    else
    {
        DEBUG_INFO(0, "%s:%d NULL buffer(%p) pointer or nothing to copy (len=%u)!",
                     __FILE__, __LINE__, buffer, bufferLen);
    }
    DEBUG_INFO(0, "%s:%d checksum 0x%x!", __FILE__, __LINE__, checksum);

    return checksum;
}

