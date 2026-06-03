/***************************************************************************
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
 ***************************************************************************/
#ifndef __AOS_FS_ERRNO_H__
#define __AOS_FS_ERRNO_H__

#ifndef __LINUX_ERRNO_EXTENSIONS__
#define __LINUX_ERRNO_EXTENSIONS__
#endif

#include <sys/errno.h>

#define set_errno(e) do { errno = (int)(e); } while (0)
#define get_errno() errno

#define ENMFILE             2001  /* Cygwin */
#define ENMFILE_STR         "No more files"
#define EUCLEAN             2002
#define EUCLEAN_STR         "Structure needs cleaning"

#undef EXTERN
#if defined(__cplusplus)
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

#undef EXTERN
#if defined(__cplusplus)
}
#endif

#endif /*__AOS_FS_ERRNO_H__*/