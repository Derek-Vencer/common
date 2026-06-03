

#include "cmsis_os.h"
#include "string.h"
#include "apple_cert.h"
#include "hal_uart.h"
#include "hal_i2c.h"
#include "hal_gpio.h"
#include "hal_trace.h"
//#include "bes_hal_board.h"
#include "bt_common_define.h"
#include "simulate_i2c.h"
#include "hal_timer.h"
#include "../mfi/common_debug.h"
#include "../mfi/mfi_dev.h"

#define TITLE_STR(x)        #x
#define AppleCertLog(num,title,value,...)   do{DEBUG_INFO(num, title ":" value,##__VA_ARGS__);}while(0)
#define AppleCertLogI(num,x,...)    AppleCertLog(num,TITLE_STR(AppleCertLogI),x,##__VA_ARGS__ )
#define AppleCertLogD(num,x,...)    AppleCertLog(num,TITLE_STR(AppleCertLogD),x,##__VA_ARGS__ )
#define AppleCertLogW(num,x,...)    AppleCertLog(num,TITLE_STR(AppleCertLogW),x,##__VA_ARGS__ )
#define AppleCertLogE(num,x,...)    AppleCertLog(num,TITLE_STR(AppleCertLogE),x,##__VA_ARGS__ )

#define kMFiControlValueGenerateChallangeResponse (0x1)

int read_apple_version(void){

#ifdef BOARD_HW_GPIO_I2C_MODULE
    int ret;
    u8 version=0;
    ret = i2c_read_reg_data(COPROCESSOR_ADDR,DEVICE_VERSION_REG,(u8 *)&version,1);
    if(ret<0){
        AppleCertLogE(0,"read DEVICE_VERSION fail \r\n");
    }else{
        AppleCertLogI(1," DEVICE_VERSION = %d \r\n",version);
    }
    return ret;
#else
    unsigned int mfi_chip_ver = 0;
    int ret = mfi_get_auth_chip_version(&mfi_chip_ver);
    CHECK_RET_RETURN(ret < 0, -1, "MFi Version error.");
    return ret;
#endif
    
}

int read_accessory_certificate_serial_number(u8 *buf,u16 buf_len){
#ifdef BOARD_HW_GPIO_I2C_MODULE
    if(buf_len<31 || buf == NULL){
        AppleCertLogE(0,"buf is small \r\n");
        return -1;
    }

    int ret;
    ret = i2c_read_reg_data(COPROCESSOR_ADDR,CERTIFICATE_SERIAL_NUMBER,buf,31);
    if(ret<0){
        AppleCertLogE(0,"read CERTIFICATE_SERIAL_NUMBER fail \r\n");
        return -1;
    }
    return 32;
#else
    CHECK_RET_RETURN(buf == NULL, -1, "Parameter is invalid.");
    CHECK_RET_RETURN(buf_len == 0, -1, "Parameter is invalid.");
    int ret = mfi_get_device_certificate_serial_number(buf, buf_len);
    CHECK_RET_RETURN(ret < 0, -1, "mfi_get_device_certificate_serial_number");
    return 0;
#endif
}


int read_apple_cert_data(u8 *buf,u16 buf_len){
    
#ifdef BOARD_HW_GPIO_I2C_MODULE
    u8 ret = -1;
    u8 temp_buf[2]={0};
    int cert_len;
    int i = 0;
    
    if(buf == NULL)return -1;
    ret = i2c_read_reg_data(COPROCESSOR_ADDR,ACCESSORY_CERTIFICATE_DATA_LEN_REG,(u8 *)&temp_buf,sizeof(temp_buf));
    if(ret<0){
        AppleCertLogE(0,"read cert len fail \r\n");
        return ret;
    }
    cert_len = temp_buf[1]|(temp_buf[0]<<8);
    AppleCertLogI(1,"cert len = %d \r\n",cert_len);
    if(buf_len<cert_len){
        AppleCertLogE(0,"buf is small\r\n");
        return -1;
    }
    for(i=0;i< (cert_len/128);i++){
        ret = i2c_read_reg_data(COPROCESSOR_ADDR,ACCESSORY_CERTIFICATE_DATA_REG+i,(u8 *)&buf[i*128],128);
        if(ret<0){
            AppleCertLogE(1,"read ACCESSORY_CERTIFICATE_DATA_REG %d fail \r\n",i);
            return -1;
        }
    }
    if(cert_len%128){
        int pos = cert_len - cert_len%128;
        ret = i2c_read_reg_data(COPROCESSOR_ADDR,ACCESSORY_CERTIFICATE_DATA_REG+i,(u8 *)&buf[pos],cert_len%128);
        if(ret<0){
            AppleCertLogE(0,"read ACCESSORY_CERTIFICATE_DATA_REG fail \r\n");
            return -1;
        }
    }
    return cert_len;
#else
    CHECK_RET_RETURN(buf == NULL, -1, "Parameter is invalid.");
    u32 CertificateLength = 0;
    u8 *CertificatePtr = buf;
    int ret = 0;
    do {
         // Get cert length;
         ret = mfi_get_certificate_length(&CertificateLength);
         CHECK_RET_BREAK(ret < 0, "mfi_get_certificate_length");
         CHECK_RET_BREAK(CertificateLength == 0, "mfi_get_certificate_length");
         CHECK_RET_BREAK(buf_len < CertificateLength, "mfi_get_certificate_length");
         AppleCertLogI(0,"CertificateLength:%d",CertificateLength);
         // Get certificate data.
         ret = mfi_get_certificate_data(CertificatePtr, CertificateLength);
         CHECK_RET_BREAK(ret < 0, "mfi_get_certificate_data");
         DUMP8("%02x ",buf,16);
         return CertificateLength;
     } while(0);
     return -1;
#endif
    
}

int read_challenge_response_data(u8 *challenge_buf,u16 challenge_len,u8 *challenge_response_buf,u16 response_buf_len){
#ifdef BOARD_HW_GPIO_I2C_MODULE
    int ret = 0;
    int status=0;
    u8 temp_buf[2]={0};
    int response_len = 0;
    
    temp_buf[0] = challenge_len>>8;
    temp_buf[1] = challenge_len&0xff;
    AppleCertLogI(1,"challenge_len %d",challenge_len);
    AppleCertLogI(0,"CHALLENGE_DATA_LEN_REG");
    ret = i2c_write_reg_data(COPROCESSOR_ADDR,CHALLENGE_DATA_LEN_REG,(u8 *)&temp_buf,sizeof(temp_buf));
    if(ret<0){
        AppleCertLogE(0,"write challenge len fail\r\n");
        return ret;
    }
    AppleCertLogI(0,"CHALLENGE_DATA_REG");
    ret = i2c_write_reg_data(COPROCESSOR_ADDR,CHALLENGE_DATA_REG,challenge_buf,challenge_len);
    if(ret<0){
        AppleCertLogE(0,"write challenge data fail\r\n");
        return ret;
    }
    u8 reg = 0x01;
    AppleCertLogI(0,"WIRTE CONTROL_STATUS_REG");
    ret = i2c_write_reg_data(COPROCESSOR_ADDR,CONTROL_STATUS_REG,&reg,1);
    if(ret<0){
        AppleCertLogE(0,"write control status fail\r\n");
        return ret;
    }
    osDelay(10);
    AppleCertLogI(0,"READ CONTROL_STATUS_REG");
    ret = i2c_read_reg_data(COPROCESSOR_ADDR,CONTROL_STATUS_REG,(u8 *)&status,1);
    if(ret<0){
        AppleCertLogE(0,"read control status fail\r\n");
            return ret;
    }
    AppleCertLogI(2,"status %d ret = %d",status,ret);
    if((status&0x70)!= 0x10){
        AppleCertLogE(1,"response generated fail %d \r\n",status&0x80);
        return -1;
    }
    memset(temp_buf,0,sizeof(temp_buf));
    osDelay(420);
    AppleCertLogI(0,"READ CHALLENGE_RESPONSE_LEN_REG");
    ret = i2c_read_reg_data(COPROCESSOR_ADDR,CHALLENGE_RESPONSE_LEN_REG,(u8 *)&temp_buf,sizeof(temp_buf));
    if(ret<0){
        AppleCertLogE(0,"read response len fail\r\n");
        return ret;
    }
    response_len = (temp_buf[0]<<8) | temp_buf[1];
    AppleCertLogI(1,"CHALLENGE_RESPONSE_LEN %d",response_len);
    if((response_buf_len<response_len)||(challenge_response_buf == NULL)){
        AppleCertLogE(0,"response_buf_len is small \r\n");
        return -1;
    }
    
    ret = i2c_read_reg_data(COPROCESSOR_ADDR,CHALLENGE_RESPONSE_DATA_REG,challenge_response_buf,response_len);
    if(ret<0){
        AppleCertLogE(0,"read response len fail\r\n");
        return ret;
    }
    return response_len;
#else 
    CHECK_RET_RETURN(challenge_buf == NULL, -1, "Parameter is invalid.");
    CHECK_RET_RETURN(challenge_response_buf == NULL, -1, "Parameter is invalid.");
    CHECK_RET_RETURN(challenge_len == 0, -1, "Parameter is invalid.");
    CHECK_RET_RETURN(response_buf_len == 0, -1, "Parameter is invalid.");
    int ret = -1;
    u8 status = kMFiControlValueGenerateChallangeResponse;
    u32 signature_length = 0;
    do {
        // set challenge
        ret = mfi_set_challenge_data(challenge_buf, challenge_len, 3);
        CHECK_RET_BREAK(ret < 0, "mfi_set_challenge_data");

        // enable signature process.
        ret = mfi_write_control_status(status);
        CHECK_RET_BREAK(ret < 0, "mfi_write_control_status");

        // get signature length.
        ret = mif_get_response_challenge_data_length(&signature_length);
        CHECK_RET_BREAK(ret < 0, "mif_get_response_challenge_data_length");
        CHECK_RET_BREAK(response_buf_len < signature_length, "mif_get_response_challenge_data_length");

        // get signature data.
        ret = mfi_get_response_challenge_data(challenge_response_buf, signature_length);
        CHECK_RET_BREAK(ret < 0, "mif_get_response_challenge_data_length");

        return signature_length;
    } while(0);
    return -1;
#endif
}
