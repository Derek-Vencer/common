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

#ifndef __COMMON_DEBUG_H__
#define __COMMON_DEBUG_H__

void app_dbg_hex_dump(const char* tag, const unsigned char* mem, const unsigned int size);

#define eprt(fmt, args...) TR_INFO(0, "Err:" fmt, ##args);
#define wprt(fmt, args...) TR_INFO(0, "Wrn:" fmt, ##args);
#define iprt(fmt, args...) TR_INFO(0, fmt, ##args);
#define dprt(fmt, args...) TR_INFO(0, fmt, ##args);
#define CHECK_RET_BREAK(cond, fmt, args...) if(cond){ eprt(fmt, ##args); break;}
#define CHECK_RET_BREAK_I(cond, fmt, args...) if(cond){ iprt(fmt, ##args); break;}
#define CHECK_RET_RETURN(cond, ret, fmt, args...) if(cond){ eprt(fmt, ##args); return ret;}
#define CHECK_RET_RETURN_NULL(cond, fmt, args...) if(cond){ eprt(fmt, ##args); return;}



#endif /* __COMMON_DEBUG_H__ */
