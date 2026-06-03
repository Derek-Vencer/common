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

#ifndef __MFI_DEV_H__
#define __MFI_DEV_H__

/* Return -1: failed, 0 : success */
int mfi_detect(unsigned int *authProtoMajorVer);

int mfi_get_auth_chip_version(unsigned int *version);

int mfi_get_certificate_length(unsigned int *length);

int mfi_get_certificate_data(unsigned char *certData, const unsigned int certDataLength);

int mfi_set_challenge_data(const unsigned char* challengeData, const unsigned int challengeDataLength, const unsigned int authChipVersion);

int mfi_write_control_status(const unsigned char status);

int mif_get_response_challenge_data_length(unsigned int *length);

int mfi_get_response_challenge_data(unsigned char *challengeData, const unsigned int challengeDataLength);

void mfi_set_i2c_id(int id);

int mfi_get_device_certificate_serial_number(unsigned char* serialNumber,  const unsigned int serialNumberLen);


#endif /* __MFI_DEV_H__ */
