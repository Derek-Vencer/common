
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
#ifndef __BES_I2C_DEV_H__
#define __BES_I2C_DEV_H__
struct i2c_device {
    char* name;
    unsigned char port;
    unsigned char bus_addr;
    struct HAL_I2C_CONFIG_T cfg;
};
int do_i2c(struct i2c_device *dev, unsigned char reg, const unsigned char* txData, const unsigned int txLength, unsigned char* rxData, unsigned int rxLength);
int do_i2c_tx(struct i2c_device *dev, const unsigned char *txData, const unsigned int txLength);
#endif /* __BES_I2C_DEV_H__ */
