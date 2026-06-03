#pragma once

#include <stdint.h>

#ifdef __cplusplus
#define EXTERNC	extern "C"
#else
#define EXTERNC
#endif

#define PLC_RET_SUCCESS 0
#define PLC_RET_INPUT_ERROR -1
#define PLC_RET_FAIL -2


enum PlcSbcCodecType_e {
	PlcSbcCodecTypeSbc = 0,
	PlcSbcCodecTypeAac,
	PlcSbcCodecTypeLc3,
	PlcSbcCodecTypeCvsd,
	PlcSbcCodecTypeAmrNb,
	PlcSbcCodecTypeAmrWb,
	PlcSbcCodecTypeMax,
};


enum PlcMode_e {
	PlcModeMusicPlc = 0,
	PlcModeSbcPlc,
	PlcModeOther,
	PlcModeMax,
};

enum PlcSampleAlignMode_e {
	PlcSampleHighByteAlign = 0,
	PlcSampleLowByteAlign,
};

typedef struct {
	int16_t validBits;
	int16_t storedBits;
	PlcSampleAlignMode_e alignMode;
}SampleStoreFormat;

typedef struct {
	//common
	int32_t fsHz;
	int16_t channels;
	int16_t frameSamples;
	SampleStoreFormat sampleFormat;
	enum PlcMode_e mode;
	union {
		//music plc
		struct {
			int32_t overlapMs;
			int32_t decayTimeMs;
		}MusicPlcParam;
		//sbcplc
		struct  {
			union {
				enum PlcSbcCodecType_e codecType;
				uint32_t searchSize;
				uint32_t matchSize;
				uint32_t olal;
				uint32_t recover;
			};
		}SbcPlcParam;
	};
	void* other;
}PlcParam;

typedef int32_t PlcRet;



EXTERNC int32_t GetPlcStateSize(PlcParam* param);
EXTERNC PlcRet PlcInit(void* plcStateIn, PlcParam* param);
EXTERNC PlcRet PlcRun(void* plcStateIn, int8_t* in, int32_t inLen, int32_t* inUsed, int8_t* out, int32_t* pOutLen, bool isLost);



