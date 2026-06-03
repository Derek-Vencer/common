/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "hal_trace.h"
#include "hal_i2c.h"
#include "hal_timer.h"
#include "bes_i2c_dev.h"
#include "cmsis_os.h"
#include "common_debug.h"
#define MFI_V3_SLAVE_ADDR                   (0x10)
#define MFI_DEFAULT_SLAVE_ADDR              (0x11)

#ifndef MFI_I2C_PORT
#define MFI_I2C_PORT                        (0)
#endif

#define MFI_I2C_SPEED 300000

static struct i2c_device mfi_i2c_dev = {
    .name = "MFi_3.0",
    .port = MFI_I2C_PORT,
    .bus_addr = MFI_V3_SLAVE_ADDR,
    .cfg = {
        .speed = MFI_I2C_SPEED,
        .mode = HAL_I2C_API_MODE_SIMPLE,
        .use_dma = 0,
        .use_sync = 1,
        .as_master = 1,
    }
};

void mfi_set_i2c_id(int id)
{
    if (id >= 0) {
        mfi_i2c_dev.port = id;
    }
}

#define kMFiAuthRegisterProtocolMajorVersion 0x02
#define kMFiAuthRegisterControl 0x10
#define kMFiAuthRegisterChallengeResponseDataLength 0x11
#define kMFiAuthRegisterChallengeResponseData 0x12
#define kMFiAuthRegisterChallengeDataLength 0x20
#define kMFiAuthRegisterChallengeData 0x21
#define kMFiAuthRegisterAccessoryCertificateDataLength 0x30
#define kMFiAuthRegisterAccessoryCertificateData 0x31
#define kMFiAuthRegisterDeviceCertificateSerialNumber 0x4E

static const unsigned char mfi_slave_addr_array [] = {
    MFI_V3_SLAVE_ADDR,
    MFI_DEFAULT_SLAVE_ADDR
};

int mfi_is_exist_by_check_version(void)
{
    if(mfi_i2c_dev.bus_addr != 0) {
        TR_INFO(0,"Found MFI(%#x)\n", mfi_i2c_dev.bus_addr);
        return 1;
    }

    unsigned char buf[1] = {0};
    struct i2c_device temp_dev;
    mfi_i2c_dev.cfg.speed = MFI_I2C_SPEED;
    memcpy(&temp_dev, &mfi_i2c_dev, sizeof(struct i2c_device));
    int mfiCount = sizeof(mfi_slave_addr_array) / sizeof(unsigned char);
    int i = 0;
    int j = 0;
    int ret = 0;
    for(j = 0; j < 3; j++) {        // retry 3 times.
        for(i = 0; i < mfiCount; i++) {
            temp_dev.bus_addr = mfi_slave_addr_array[i];
            ret = do_i2c(&temp_dev, kMFiAuthRegisterProtocolMajorVersion, NULL, 0, buf, 1);
            if(ret == 0) {
                mfi_i2c_dev.bus_addr = temp_dev.bus_addr;
                TR_INFO(0,"Found MFI(%#x)\n", mfi_i2c_dev.bus_addr);
                return 1;
            }
        }
    }

    TR_INFO(0,"Not Found MFi !!!\n");
    return 0;
}

int mfi_detect(unsigned int *authProtoMajorVer)
{
    unsigned char buf[1] = {0};
    struct i2c_device temp_dev;
    mfi_i2c_dev.cfg.speed = MFI_I2C_SPEED;
    memcpy(&temp_dev, &mfi_i2c_dev, sizeof(struct i2c_device));
    int mfiCount = sizeof(mfi_slave_addr_array) / sizeof(unsigned char);
    int i = 0;
    int j = 0;
    int ret = 0;
    for(j = 0; j < 3; j++) {        // retry 3 times.
        for(i = 0; i < mfiCount; i++) {
            temp_dev.bus_addr = mfi_slave_addr_array[i];
            ret = do_i2c(&temp_dev, kMFiAuthRegisterProtocolMajorVersion, NULL, 0, buf, 1);
            if(ret == 0) {
                mfi_i2c_dev.bus_addr = temp_dev.bus_addr;
                *authProtoMajorVer = (unsigned int)buf[0];
                TR_INFO(0,"Found MFI(%#x) Auth Proto Ver:%d.0\n", mfi_i2c_dev.bus_addr, *authProtoMajorVer);
                return 0;
            }
        }
    }
    return -1;
}

int mfi_get_auth_chip_version(unsigned int *version)
{
    CHECK_RET_RETURN((version == NULL), -1, "parameter error");
    int ret = 0;
    unsigned char buf[1] = {0};
    ret = do_i2c(&mfi_i2c_dev, kMFiAuthRegisterProtocolMajorVersion, NULL, 0, buf, 1);
    if(ret == 0) {
        *version = (int)buf[0];
    }
    return ret;
}

int mfi_get_certificate_length(unsigned int *length)
{
    CHECK_RET_RETURN((length == NULL), -1, "parameter error");
    int ret = 0;
    unsigned char buf[2] = {0};
    ret = do_i2c(&mfi_i2c_dev, kMFiAuthRegisterAccessoryCertificateDataLength, NULL, 0, buf, 2);
    if(ret == 0) {
        unsigned short certLen = (buf[0] << 8) | buf[1];
        *length = certLen;
    }
    return ret;
}

int mfi_get_certificate_data(unsigned char *certData, const unsigned int certDataLength)
{
    CHECK_RET_RETURN((certData == NULL || certDataLength == 0), -1, "parameter error");
    int ret = 0;
    ret = do_i2c(&mfi_i2c_dev, kMFiAuthRegisterAccessoryCertificateData, NULL, 0, certData, certDataLength);
    return ret;
}

int mfi_set_challenge_data(const unsigned char* challengeData, const unsigned int challengeDataLength, const unsigned int authChipVersion)
{
    CHECK_RET_RETURN((challengeData == NULL || challengeDataLength == 0), -1, "parameter error");
    int ret = 0;
    if(authChipVersion == 3) {
        ret = do_i2c(&mfi_i2c_dev, kMFiAuthRegisterChallengeData, challengeData, challengeDataLength, NULL, 0);
    } else {
        unsigned int length = challengeDataLength + 2;
        unsigned char* ptr = (unsigned char*)malloc(length);
        CHECK_RET_RETURN((ptr == NULL), -1, "malloc error");
        memset(ptr, 0, length);
        ptr[0] = (unsigned char)((challengeDataLength >> 8) & 0xFF);
        ptr[1] = (unsigned char)(challengeDataLength & 0xFF);
        memcpy(ptr + 2, challengeData, challengeDataLength);
        ret = do_i2c(&mfi_i2c_dev, kMFiAuthRegisterChallengeDataLength, ptr, length, NULL, 0);
        free(ptr);
    }
    return ret;
}

int mfi_write_control_status(const unsigned char status)
{
    int ret = 0;
    ret = do_i2c(&mfi_i2c_dev, kMFiAuthRegisterControl, &status, 1, NULL, 0);
    return ret;
}

int mif_get_response_challenge_data_length(unsigned int *length)
{
    CHECK_RET_RETURN((length == NULL), -1, "parameter error");
    int ret = 0;
    // int retry = 0;
    unsigned char buf[2] = {0};
    int i = 0;
    osDelay(390);  // MFi 3.0 take time range[399 - 425] ms.
    for(i = 0; i < 10; i++) {       // MFi3.0 loop 5 times, MFi2.0 loop 3 times.
        ret = do_i2c(&mfi_i2c_dev, kMFiAuthRegisterChallengeResponseDataLength, NULL, 0, buf, 2);
        if(ret == 0) {
            unsigned short responseLength = (buf[0] << 8) | buf[1];
            *length = responseLength;
            break;
        }
    }
    return ret;
}

int mfi_get_response_challenge_data(unsigned char *challengeData, const unsigned int challengeDataLength)
{
    CHECK_RET_RETURN((challengeData == NULL || challengeDataLength == 0), -1, "parameter error");
    int ret = 0;
    ret = do_i2c(&mfi_i2c_dev, kMFiAuthRegisterChallengeResponseData, NULL, 0, challengeData, challengeDataLength);
    return ret;
}

int mfi_get_device_certificate_serial_number(unsigned char* serialNumber,  const unsigned int serialNumberLen)
{
    CHECK_RET_RETURN((serialNumber == NULL || serialNumberLen == 0), -1, "parameter error");
    int ret = 0;
    ret = do_i2c(&mfi_i2c_dev, kMFiAuthRegisterDeviceCertificateSerialNumber, NULL, 0, serialNumber, serialNumberLen);
    return ret;
}



#if 1 || MFI_UNIT_TEST

//extern void hexdump(const char* tag, const unsigned char* mem, const unsigned int size);

void mfi_unit_test()
{
    TR_INFO(0,"####mfi_unit_test");
    int ret = -1;
    TR_INFO(0,"%s new API v3\n", __FUNCTION__);
    mfi_set_i2c_id(0);
#if 0//!I2C_SWITCH_ENABLE
    ret = hal_i2c_open(I2C_PORT, &i2c_cfg);
    if(ret != 0) {
        printf("open i2c interface error[%d]\n", ret);
        return ;
    }
#endif
    mfi_is_exist_by_check_version();

    /*Test auth chip version api*/
    unsigned int mfi_version = 0;
    ret = mfi_get_auth_chip_version(&mfi_version);
    if(ret != 0) {
        TR_INFO(0,"%d:authChipVersion test failed[%d]\n", __LINE__, ret);
    } else {
        TR_INFO(0,"Mfi version :%d\n", mfi_version);
    }

    /*Test cert length api*/
    unsigned int certLength = 0;
    ret = mfi_get_certificate_length(&certLength);
    if(ret != 0) {
        TR_INFO(0,"%d:Get cert length test failed[%d]\n", __LINE__, ret);
    } else {
        if(certLength != 908) {
            TR_INFO(0,"Warnnig:Mfi cert length maybe error!!!\n");
        }
        TR_INFO(0,"Mfi cert length:%d\n", certLength);
    }

    /*Test cert data api*/
    unsigned char *certData = NULL;
    if(certLength == 608) {
        certData = (unsigned char*)malloc(certLength);
    }
    ret = mfi_get_certificate_data(certData, certLength);
    if(ret != 0) {
        TR_INFO(0,"%d:Get cert data test failed[%d]\n",__LINE__, ret);
    } else {
        DUMP8("%02x ",certData,certLength);
        TR_INFO(0,"Mfi cert data ok\n");
        //free(certData);
    }

#if 0
    /*Test use SHA algorithm api*/
    unsigned char digestbuf[20] = {0};
    unsigned char *outSHABuf = NULL;
    unsigned int outSHABufLength = 0;
    ret = mif_challenge_data_use_SHA(mfi_version, digestbuf, sizeof(digestbuf), &outSHABuf, &outSHABufLength);
    if(ret != 0) {
        printf("%d:SHA error[%d]\n", __LINE__, ret);
    } else {
        printf("SHA ok\n");
    }
#else
    unsigned char SHABuf[] = {
        0x67,0x68,0x03,0x3E,0x21,0x64,0x68,0x24,0x7B,0xD0,
        0x31,0xA0,0xA2,0xD9,0x87,0x6D,0x79,0x81,0x8F,0x8F
        }; /*SHA1 authchip version = 2.*/
    unsigned int outSHABufLength = sizeof(SHABuf);
    unsigned char *outSHABuf = (unsigned char*)malloc(outSHABufLength);
    if(outSHABuf != NULL) {
        memcpy(outSHABuf, SHABuf, outSHABufLength);
        //HEX_DUMP("SHAData", outSHABuf, outSHABufLength);
        DUMP8("%02x ",outSHABuf,outSHABufLength);
    }

#endif
    /*Test write challenge data api*/
    unsigned char *challengeData = NULL;
    unsigned int challengeDataLength = 0;
    if(outSHABuf != NULL && outSHABufLength != 0) {
        challengeData = (unsigned char *)malloc(outSHABufLength);
        if(challengeData != NULL) {
            memcpy(challengeData, outSHABuf, outSHABufLength);
            challengeDataLength = outSHABufLength;
        }
        //free(outSHABuf);
        outSHABuf = NULL;
    }

    ret = mfi_set_challenge_data(challengeData, challengeDataLength, mfi_version);
    if(ret != 0) {
        TR_INFO(0,"%d:Mfi set challenge data failed[%d]\n", __LINE__, ret);
    } else {
        TR_INFO(0,"Mfi set challenge data ok\n");
    }

    if(challengeData) {
        //free(challengeData);
        challengeData = NULL;
    }

    /*Test write mfi status*/
    unsigned char status = 1;
    ret = mfi_write_control_status(status);
    if(ret != 0) {
        TR_INFO(0,"%d:Mfi set status failed[%d]\n", __LINE__, ret);
    } else {
        TR_INFO(0,"Mfi write status ok\n");
    }

    /*Test get challenge data length*/
    unsigned int respchallengeDataLength = 0;
    ret = mif_get_response_challenge_data_length(&respchallengeDataLength);
    if(ret != 0) {
        TR_INFO(0,"%d:Get challenge length test failed[%d]\n", __LINE__, ret);
    } else {
        if(respchallengeDataLength != 128) {
            TR_INFO(0,"Warnnig:Mfi challenge length maybe error!!!\n");
        }
        TR_INFO(0,"Mfi challenge length:%d\n", respchallengeDataLength);
    }

    /*Test get response challenge data*/
    unsigned char *respchallengeData = NULL;
    if(respchallengeDataLength == 128) {
        respchallengeData = (unsigned char*)malloc(respchallengeDataLength);
    }

    ret = mfi_get_response_challenge_data(respchallengeData, respchallengeDataLength);
    if(ret != 0) {
        TR_INFO(0,"%d:Get response challenge data test failed[%d]\n",__LINE__, ret);
    } else {
        TR_INFO(0,"Mfi response challenge data ok\n");
        //HEX_DUMP("challenge data", respchallengeData, respchallengeDataLength);
        //free(respchallengeData);
    }
    memset(respchallengeData, 0, respchallengeDataLength);
    ret = mfi_get_response_challenge_data(respchallengeData, respchallengeDataLength);
    if(ret != 0) {
        TR_INFO(0,"%d:Get response challenge data test failed[%d]\n",__LINE__, ret);
    } else {
        TR_INFO(0,"Mfi response challenge data ok\n");
        //HEX_DUMP("challenge data", respchallengeData, respchallengeDataLength);
        //free(resphallengeData);
    }
#if 0//!I2C_SWITCH_ENABLE
    hal_i2c_close(I2C_PORT);
#endif
}


#if 0
int user_get_cert(unsigned char** outCertData, unsigned int *outCertDataLength)
{
    unsigned short certLen = 0 ;
    unsigned char *pCertData = NULL;
    int ret = 0;
    unsigned char buf[2] = {0};
    ret = do_i2c(&mfi_i2c_dev, kMFiAuthRegisterAccessoryCertificateDataLength, NULL, 0, buf, 2);
    if(ret == 0) {
        certLen = (buf[0] << 8) | buf[1];
        if(pCertData == NULL) {
            pCertData = (unsigned char*)malloc((certLen));
            if(pCertData) {
                ret = do_i2c(&mfi_i2c_dev, kMFiAuthRegisterAccessoryCertificateData, NULL, 0, pCertData, certLen);
                if(ret == 0) {
                    *outCertData = pCertData;
                    *outCertDataLength = certLen;
                    return 0;
                }
            }
        }
    }
    if(pCertData) {
        free(pCertData);
    }
    return -1;
}

int user_update_sha(const unsigned char *shaData, const unsigned int shaDataLength)
{
    int ret = 0;
    printf("Default Test MFI v2\n");
    unsigned int length = shaDataLength + 2;
    unsigned char* ptr = (unsigned char*)malloc(length);
    if(ptr == NULL) {
        printf("malloc error\n");
        return -1;
    }
    memset(ptr, 0, length);
    ptr[0] = (unsigned char)((shaDataLength >> 8) & 0xFF);
    ptr[1] = (unsigned char)(shaDataLength & 0xFF);
    memcpy(ptr + 2, shaData, shaDataLength);
    ret = do_i2c(&mfi_i2c_dev, kMFiAuthRegisterChallengeDataLength, ptr, length, NULL, 0);
    free(ptr);
    return ret;
}

int user_enable_control()
{
    int ret = 0;
    unsigned char status = 1;
    ret = do_i2c(&mfi_i2c_dev, kMFiAuthRegisterControl, &status, 1, NULL, 0);
    return ret;
}

int user_get_response_signature(unsigned char** outSignatureData, unsigned int *outSignatureDataLength)
{
    unsigned short signatureDataLen = 0;
    unsigned char *signatureData = NULL;
    int ret = 0;
    unsigned char buf[2] = {0};
    ret = do_i2c(&mfi_i2c_dev, kMFiAuthRegisterChallengeResponseDataLength, NULL, 0, buf, 2);
    if(ret == 0) {
        signatureDataLen = (buf[0] << 8) | buf[1];
        signatureData = (unsigned char *)malloc(signatureDataLen);
        if(signatureData != NULL) {
            ret = do_i2c(&mfi_i2c_dev, kMFiAuthRegisterChallengeResponseData, NULL, 0, signatureData, signatureDataLen);
            if(ret == 0) {
                *outSignatureData = signatureData;
                *outSignatureDataLength = signatureDataLen;
                return 0;
            }
        }
    }
    if(signatureData) {
        free(signatureData);
    }
    return -1;
}

void get_signature_test()
{
    printf("==%s==\n", __FUNCTION__);
    unsigned char *data = NULL;
    unsigned int length = 0;
    int ret = user_get_response_signature(&data, &length);
    if(ret) {
        printf("user_get_response_signature failed\n");
    } else {
        printf("user_get_response_signature success\n");
        HEX_DUMP("signature", data, length);
    }
    if(data) {
        free(data);
    }
}

void set_control_status_test()
{
    printf("==%s==\n", __FUNCTION__);
    int ret = user_enable_control();
    if(ret) {
        printf("user_enable_control failed\n");
    } else {
        printf("user_enable_control success\n");
    }
}

static uint8_t shaDataArray[3][20] = {{0},{0xFF},
        {0x67,0x68,0x03,0x3E,0x21,0x64,0x68,0x24,0x7B,0xD0, 0x31,0xA0,0xA2,0xD9,0x87,0x6D,0x79,0x81,0x8F,0x8F}
    };

void update_sha_data_test(int index)
{
    printf("==%s==\n", __FUNCTION__);
    if(index >= 3) {
        return ;
    }
    int ret = user_update_sha(shaDataArray[index], 20);
    if(ret) {
        printf("user_update_sha failed\n");
    } else {
        printf("user_update_sha success\n");
    }
}
#endif

#endif