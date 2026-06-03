/****************************************************************************
 * include/sys/stat.h
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
 ****************************************************************************/

#ifndef __INCLUDE_SYS_STAT_H
#define __INCLUDE_SYS_STAT_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include "conf/fs_config.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
#define S_IFMTD     (030000)
#define S_ISMTD(m)  (((m) & S_IFMT) == S_IFMTD)

#define S_IFMQ      (050000)
#define S_ISMQ(m)   (((m) & S_IFMT) == S_IFMQ)

/****************************************************************************
 * Type Definitions
 ****************************************************************************/

#undef EXTERN
#if defined(__cplusplus)
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

#define UTIME_NOW  0x3fffffff
#define UTIME_OMIT 0x3ffffffe

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

int mkdir(FAR const char *pathname, mode_t mode);
int stat(FAR const char *path, FAR struct stat *buf);
int lstat(FAR const char *path, FAR struct stat *buf);
int fstat(int fd, FAR struct stat *buf);
int utimens(FAR const char *path, const struct timespec times[2]);

#undef EXTERN
#if defined(__cplusplus)
}
#endif

#endif /* __INCLUDE_SYS_STAT_H */
