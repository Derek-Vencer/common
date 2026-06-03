#ifndef __TWSUI_SPP_H__
#define __TWSUI_SPP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

#ifdef USER_SPP_CMD_EN
//spp common cmd                        L/R  renyi
#define SPP_OPEN_MASTER_MIC_ONLY                0x00
//                            header     L/R     CMD     NC     END
//open master mic only        F5 01      00      00      00    01 F5
//reply                       F5 01      00      00      01    01 F5
#define SPP_OPEN_SLAVE_MIC_ONLY                 0x01
//open slave  mic only        F5 01      00/01   01      00    01 F5 
//replay                      F5 01      00      01      01    01 F5
#define SPP_OPEN_MASTER_SLAVE_MIC               0x02
//open slave  mic only        F5 01      00      02      00    01 F5 
//replay                      F5 01      00      02      01    01 F5
#define SPP_GET_SOFTWARE_VERSION                0x03
//open slave  mic only        F5 01      00      03      00    01 F5 
//replay                      F5 01      00      03      3byte    01 F5

#define SPP_GET_BTADDR                          0x04
//open slave  mic only        F5 01      00      04      00    01 F5 
//replay                      F5 01      00      04      6byte    01 F5 //gaowei dao diwei

#define SPP_GET_BTNAME                          0x05
//open slave  mic only        F5 01      00      05      00    01 F5 
//replay                      F5 01      00      05      nbyte    01 F5

#define SPP_GET_LRSIDE                          0x06
//open slave  mic only        F5 01      00      06      00    01 F5 
//replay                      F5 01      00      06      nbyte    01 F5


#define MAX_SPP_LEN                             20
#define SPP_DATA_LEN                            7

void app_spp_msg_modual_init(void);
void app_spp_msg_post_msg(uint8_t *buffer_data, uint8_t data_len); 
#endif //#ifdef USER_SPP_CMD_EN


#ifdef __cplusplus
}
#endif

#endif //__TWSUI_SPP_H__