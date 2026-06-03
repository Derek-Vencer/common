/***************************************************************************
 *
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
 *
 ****************************************************************************/
#ifndef __OPUS_API_H__
#define __OPUS_API_H__
#include <stddef.h>

#ifndef EXTERNC
#ifdef __cplusplus
#define EXTERNC extern "C" 
#else
#define EXTERNC
#endif
#endif


#define OPUS_API_RET_SUCCESS		(0)
#define OPUS_API_RET_FAIL			(-1)
#define OPUS_API_RET_NOT_SUPPORT	(-2)

typedef int OpusApiRet_t;

typedef struct {
	void* (*malloc_cb)(int size);
	void* (*realloc_cb)(void* rmem, int newsize);
	void(*free_cb)(void* buf);
	void(*print_cb)(const char* fmt, ...);
}OpusApi_BasePort_t;

typedef enum {
	OPUS_API_ENC_CHOOSE_NORMAL = 0,
	OPUS_API_ENC_CHOOSE_INROM,
	OPUS_API_ENC_CHOOSE_WALKIE_TALKIE,
	OPUS_API_ENC_CHOOSE_MAX,
}OpusApi_EncChoose_c;

typedef enum {
	OPUS_API_DEC_CHOOSE_NORMAL = 0,
	OPUS_API_DEC_CHOOSE_WALKIE_TALKIE,
	OPUS_API_DEC_CHOOSE_MAX,
}OpusApi_DecChoose_c;

typedef enum {
	//OPUS_API_ENC_GET_NO_ONE = 0,
	OPUS_API_ENC_GET_MAX,
}OpusApi_EncGetChhoose_e;


typedef enum {
	OPUS_API_ENC_SET_MODE_AUTO = -1000,
	OPUS_API_ENC_SET_MODE_SILK_ONLY = 1000,
	OPUS_API_ENC_SET_MODE_HYBRID,
	OPUS_API_ENC_SET_MODE_CELT_ONLY,
}OpusApi_EncSetMode_e;

typedef enum {
	OPUS_API_ENC_SET_BIT_RATE = 0,
	OPUS_API_ENC_SET_FRAME_DURATION_0P1MS,
	OPUS_API_ENC_SET_USE_VBR,
	OPUS_API_ENC_SET_COMPLEXITY,
	OPUS_API_ENC_SET_MODE,
	OPUS_API_ENC_SET_OUT_CH,
	OPUS_API_ENC_SET_APPLICATION,
}OpusApi_EncSetChhoose_e;

typedef enum {
	//OPUS_API_DEC_GET_NO_ONE= 0,
	OPUS_API_DEC_GET_MAX,
}OpusApi_DecGetChhoose_e;

typedef enum {
	//OPUS_API_DEC_SET_NO_ONE = 0,
	OPUS_API_DEC_SET_MAX,
}OpusApi_DecSetChhoose_e;


//enc
EXTERNC	OpusApiRet_t opus_api_create_encoder(void** pHd, OpusApi_BasePort_t* basePort, int fs, int channels, bool isWithHead, OpusApi_EncChoose_c choose);
EXTERNC	OpusApiRet_t opus_api_destory_encoder(void* hd);
EXTERNC	OpusApiRet_t opus_api_encoder_set(void* hd, OpusApi_EncSetChhoose_e choose, void* val);
EXTERNC	OpusApiRet_t opus_api_encoder_get(void* hd, OpusApi_EncGetChhoose_e choose, void* val);
EXTERNC	OpusApiRet_t opus_api_encoder_run(void* hd, short* in, int inSample, unsigned char* out, int* outByte);

//dec
EXTERNC	OpusApiRet_t opus_api_create_decoder(void** pHd, OpusApi_BasePort_t* basePort, int fs, int channels, OpusApi_DecChoose_c choose);
EXTERNC	OpusApiRet_t opus_api_destory_decoder(void* hd);
EXTERNC	OpusApiRet_t opus_api_decoder_set(void* hd, OpusApi_DecSetChhoose_e choose, void* val);
EXTERNC	OpusApiRet_t opus_api_decoder_get(void* hd, OpusApi_DecGetChhoose_e choose, void* val);
EXTERNC	OpusApiRet_t opus_api_decoder_run(void* hd, unsigned char* in, int inByte, short* out, int* outSample, bool isPlc);


#endif