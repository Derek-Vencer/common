/*******************************************************************************
    Copyright 2016-2017 Sony Corporation
*******************************************************************************/
#ifndef LOG_H__
#define LOG_H__

#if defined __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "string.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include "hal_trace.h"
#include "hal_timer.h"
#include "cmsis.h"
#include "stdlib.h"
#include "ldac_pthread_undefine.h"
#include "hal_trace.h"

struct time_ldac_spec{
    uint32_t tv_sec;    //time_t tv_sec	整秒数（合法值 >= 0）
    uint32_t tv_nsec;   //long tv_nsec  纳秒数（合法值为 [0, 999999999] ）
};

#define malloc(p)  ldac_bco_malloc(p);
#define free(s)    ldac_bco_free(s)
//#include <stdint.h>
extern int syspool_get_buff(uint8_t **buff, uint32_t size);
static POSSIBLY_UNUSED void *ldac_bco_malloc(size_t size)
{
	void *ptr = NULL;

	AUDIO_CODEC_LIB_TRACE(0,"[%s]", __FUNCTION__);
	syspool_get_buff((uint8_t **)&ptr, size);

	return ptr;
}

static POSSIBLY_UNUSED void *ldac_bco_calloc(size_t nmemb, size_t size)
{
	void *ptr = ldac_bco_malloc(nmemb * size);
	if (ptr != NULL)
		memset(ptr, 0, nmemb * size);
	return ptr;
}

static POSSIBLY_UNUSED void ldac_bco_free(void *ptr)
{
    AUDIO_CODEC_LIB_TRACE(0,"[%s]", __FUNCTION__);
	/*
	syspool_free_size();
	*/
}


#if defined __cplusplus
}
#endif /* __cplusplus */

#endif /* LOG_H__ */
