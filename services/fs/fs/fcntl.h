/******************************************************************************
 * include/fcntl.h
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ******************************************************************************/

#ifndef __INCLUDE_FCNTL_H
#define __INCLUDE_FCNTL_H

/******************************************************************************
 * Included Files
 ******************************************************************************/

#include <sys/types.h>
#include <sys/fcntl.h>
#include <stdint.h>
#include "conf/fs_config.h"

/******************************************************************************
 * Pre-processor Definitions
 ******************************************************************************/
#ifndef O_RDOK
#define O_RDOK      (O_RDONLY|O_RDWR)        /* Read access is permitted (non-standard) */
#endif
#ifndef O_WROK
#define O_WROK      (O_WRONLY|O_RDWR)       /* Write access is permitted (non-standard) */
#endif

/* FFCNTL is all the bits that may be set via fcntl. */
#ifndef FFCNTL
#define FFCNTL      (FNONBLOCK | FNDELAY | FAPPEND | FSYNC | FASYNC)
#endif

#define F_GETPATH       15 /* Get the path of the file descriptor(BSD/macOS) */

#if defined(CONFIG_FS_LARGEFILE)
#define F_GETLK64         F_GETLK
#define F_SETLK64         F_SETLK
#define F_SETLKW64        F_SETLKW
#define open64            open
#endif

/******************************************************************************
 * Public Type Definitions
 ******************************************************************************/

/******************************************************************************
 * Public Function Prototypes
 ******************************************************************************/

#undef EXTERN
#if defined(__cplusplus)
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

/******************************************************************************
 * POSIX-like File System Interfaces
 ******************************************************************************/
int open(FAR const char *path, int oflag, ...);
int fcntl(int fd, int cmd, ...);

/******************************************************************************
 * int creat(const char *path, mode_t mode);
 ******************************************************************************/
#define creat(path, mode) open(path, O_WRONLY|O_CREAT|O_TRUNC, mode)

#undef EXTERN
#if defined(__cplusplus)
}
#endif

#endif /* __INCLUDE_FCNTL_H */
