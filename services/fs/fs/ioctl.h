/****************************************************************************
 * fs/fs/ioctl.h
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

#ifndef __INCLUDE_AOS_FS_IOCTL_H
#define __INCLUDE_AOS_FS_IOCTL_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <sys/types.h>
#include "conf/fs_config.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* General ioctl definitions ************************************************/

/* Each Aos ioctl commands are uint16_t's consisting of an 8-bit type
 * identifier and an 8-bit command number.  All command type identifiers are
 * defined below:
 */

#define _TIOCBASE       (0x0100) /* Terminal I/O ioctl commands */
#define _WDIOCBASE      (0x0200) /* Watchdog driver ioctl commands */
#define _FIOCBASE       (0x0300) /* File system ioctl commands */
#define _DIOCBASE       (0x0400) /* Character driver ioctl commands */
#define _BIOCBASE       (0x0500) /* Block driver ioctl commands */
#define _MTDIOCBASE     (0x0600) /* MTD ioctl commands */
#define _SIOCBASE       (0x0700) /* Socket ioctl commands */
#define _ARPIOCBASE     (0x0800) /* ARP ioctl commands */
#define _TSIOCBASE      (0x0900) /* Touchscreen ioctl commands */
#define _SNIOCBASE      (0x0a00) /* Sensor ioctl commands */
#define _ANIOCBASE      (0x0b00) /* Analog (DAC/ADC) ioctl commands */
#define _PWMIOCBASE     (0x0c00) /* PWM ioctl commands */
#define _CAIOCBASE      (0x0d00) /* CDC/ACM ioctl commands */
#define _BATIOCBASE     (0x0e00) /* Battery driver ioctl commands */
#define _QEIOCBASE      (0x0f00) /* Quadrature encoder ioctl commands */
#define _AUDIOIOCBASE   (0x1000) /* Audio ioctl commands */
#define _LCDIOCBASE     (0x1100) /* LCD character driver ioctl commands */
#define _SLCDIOCBASE    (0x1200) /* Segment LCD ioctl commands */
#define _CAPIOCBASE     (0x1300) /* Capture ioctl commands */
#define _WLCIOCBASE     (0x1400) /* Wireless modules ioctl character driver commands */
#define _CFGDIOCBASE    (0x1500) /* Config Data device (app config) ioctl commands */
#define _TCIOCBASE      (0x1600) /* Timer ioctl commands */
#define _JOYBASE        (0x1700) /* Joystick ioctl commands */
#define _PIPEBASE       (0x1800) /* FIFO/pipe ioctl commands */
#define _RTCBASE        (0x1900) /* RTC ioctl commands */
#define _RELAYBASE      (0x1a00) /* Relay devices ioctl commands */
#define _CANBASE        (0x1b00) /* CAN ioctl commands */
#define _BTNBASE        (0x1c00) /* Button ioctl commands */
#define _ULEDBASE       (0x1d00) /* User LED ioctl commands */
#define _ZCBASE         (0x1e00) /* Zero Cross ioctl commands */
#define _LOOPBASE       (0x1f00) /* Loop device commands */
#define _MODEMBASE      (0x2000) /* Modem ioctl commands */
#define _I2CBASE        (0x2100) /* I2C driver commands */
#define _SPIBASE        (0x2200) /* SPI driver commands */
#define _GPIOBASE       (0x2300) /* GPIO driver commands */
#define _CLIOCBASE      (0x2400) /* Contactless modules ioctl commands */
#define _USBCBASE       (0x2500) /* USB-C controller ioctl commands */
#define _MAC802154BASE  (0x2600) /* 802.15.4 MAC ioctl commands */
#define _PWRBASE        (0x2700) /* Power-related ioctl commands */
#define _FBIOCBASE      (0x2800) /* Frame buffer character driver ioctl commands */
#define _NXTERMBASE     (0x2900) /* NxTerm character driver ioctl commands */
#define _RFIOCBASE      (0x2a00) /* RF devices ioctl commands */
#define _RPTUNBASE      (0x2b00) /* Remote processor tunnel ioctl commands */
#define _NOTECTLBASE    (0x2c00) /* Note filter control ioctl commands*/
#define _NOTERAMBASE    (0x2d00) /* Noteram device ioctl commands*/
#define _RCIOCBASE      (0x2e00) /* Remote Control device ioctl commands */
#define _HIMEMBASE      (0x2f00) /* Himem device ioctl commands */
#define _EFUSEBASE      (0x3000) /* Efuse device ioctl commands */
#define _MTRIOBASE      (0x3100) /* Motor device ioctl commands */
#define _MATHIOBASE     (0x3200) /* MATH device ioctl commands */
#define _MMCSDIOBASE    (0x3300) /* MMCSD device ioctl commands */
#define _BLUETOOTHBASE  (0x3400) /* Bluetooth ioctl commands */
#define _PKTRADIOBASE   (0x3500) /* Packet radio ioctl commands */
#define _LTEBASE        (0x3600) /* LTE device ioctl commands */
#define _VIDIOCBASE     (0x3700) /* Video device ioctl commands */
#define _CELLIOCBASE    (0x3800) /* Cellular device ioctl commands */
#define _MIPIDSIBASE    (0x3900) /* Mipidsi device ioctl commands */
#define _BINDERBASE     (0x3a00) /* Binder device ioctl commands */
#define _I3CBASE        (0x3b00) /* I3C driver ioctl commands */
#define _SYSLOGBASE     (0x3c00) /* Syslog device ioctl commands */
#define _WLIOCBASE      (0x8b00) /* Wireless modules ioctl network commands */

/* boardctl() commands share the same number space */

#define _BOARDBASE      (0xff00) /* boardctl commands */

/* Macros used to manage ioctl commands.  IOCTL commands are divided into
 * groups of 256 commands for major, broad functional areas.  That makes
 * them a limited resource.
 */

#define _IOC_MASK       (0x00ff)
#define _IOC_TYPE(cmd)  ((cmd) & ~_IOC_MASK)
#define _IOC_NR(cmd)    ((cmd) & _IOC_MASK)

#define _IOC(type,nr)   ((type)|(nr))

/* Terminal I/O ioctl commands **********************************************/

#define _TIOCVALID(c)   (_IOC_TYPE(c)==_TIOCBASE)
#define _TIOC(nr)       _IOC(_TIOCBASE,nr)

/* Terminal I/O IOCTL definitions are retained in tioctl.h */

/****************************************************************************
 * Included Files
 ****************************************************************************/

/* Watchdog driver ioctl commands *******************************************/

#define _WDIOCVALID(c)  (_IOC_TYPE(c)==_WDIOCBASE)
#define _WDIOC(nr)      _IOC(_WDIOCBASE,nr)

/* Aos file system ioctl definitions **************************************/

#define _FIOCVALID(c)   (_IOC_TYPE(c)==_FIOCBASE)
#define _FIOC(nr)       _IOC(_FIOCBASE,nr)

#define FIOC_REFORMAT   _FIOC(0x0001)     /* IN:  None
                                           * OUT: None
                                           */
#define FIOC_OPTIMIZE   _FIOC(0x0002)     /* IN:  The number of bytes to recover
                                           *      (ignored on most file systems)
                                           * OUT: None
                                           */
#define FIOC_FILEPATH   _FIOC(0x0003)     /* IN:  FAR char *(length >= PATH_MAX)
                                           * OUT: The full file path
                                           */
#define FIOC_INTEGRITY  _FIOC(0x0004)     /* Run a consistency check on the
                                           *      file system media.
                                           * IN:  None
                                           * OUT: None
                                           */
#define FIOC_DUMP       _FIOC(0x0005)     /* Dump logical content of media.
                                           * IN:  None
                                           * OUT: None
                                           */
#define FIONREAD        _FIOC(0x0006)     /* IN:  Location to return value (int *)
                                           * OUT: Bytes readable from this fd
                                           */
#define FIONWRITE       _FIOC(0x0007)     /* IN:  Location to return value (int *)
                                           * OUT: Number bytes in send queue
                                           */
#define FIONSPACE       _FIOC(0x0008)     /* IN:  Location to return value (int *)
                                           * OUT: Free space in send queue.
                                           */
#define FIONUSERFS      _FIOC(0x0009)     /* IN:  Pointer to struct usefs_config_s
                                           *      holding userfs configuration.
                                           * OUT: Instance number is returned on
                                           *      success.
                                           */
#define FIONBIO         _FIOC(0x000a)     /* IN:  Boolean option takes an
                                           *      int value.
                                           * OUT: Origin option.
                                           */
#define FIOCLEX         _FIOC(0x000b)     /* IN:  None
                                           * OUT: None
                                           */
#define FIONCLEX        _FIOC(0x000c)     /* IN:  None
                                           * OUT: None
                                           */
#define FIOC_NOTIFY     _FIOC(0x000d)     /* IN:  Pointer to struct automount_notify_s
                                           *      holding automount notification
                                           *      configuration
                                           * OUT: None
                                           */

#ifdef CONFIG_FDSAN
#define FIOC_SETTAG     _FIOC(0x000e)     /* IN:  FAR uint64_t *
                                           * Pointer to file tag
                                           * OUT: None
                                           */

#define FIOC_GETTAG     _FIOC(0x000f)     /* IN:  FAR uint64_t *
                                           * Pointer to file tag
                                           * OUT: None
                                           */
#endif

/* Aos file system ioctl definitions **************************************/

#define _DIOCVALID(c)   (_IOC_TYPE(c)==_DIOCBASE)
#define _DIOC(nr)       _IOC(_DIOCBASE,nr)

#define DIOC_GETPRIV    _DIOC(0x0001)     /* IN:  Location to return handle (void **)
                                           * OUT: Reference to internal data
                                           *      structure.  May have a reference
                                           *      incremented.
                                           */
#define DIOC_RELPRIV    _DIOC(0x0003)     /* IN:  None
                                           * OUT: None, reference obtained by
                                           *      FIOC_GETPRIV released.
                                           */

#define DIOC_SETKEY     _DIOC(0X0004)     /* IN:  Encryption key
                                           * OUT: None
                                           */

/* Aos block driver ioctl definitions *************************************/

#define _BIOCVALID(c)   (_IOC_TYPE(c)==_BIOCBASE)
#define _BIOC(nr)       _IOC(_BIOCBASE,nr)

#define BIOC_XIPBASE    _BIOC(0x0001)     /* Perform mapping to random access memory.
                                           * IN:  Pointer to pointer to void in
                                           *      which to received the XIP base.
                                           * OUT: If media is directly accessible,
                                           *      return (void *) base address
                                           *      of device memory */
#define BIOC_PROBE      _BIOC(0x0002)     /* Re-probe and interface; check for media
                                           * in the slot
                                           * IN:  None
                                           * OUT: None (ioctl return value provides
                                           *      success/failure indication). */
#define BIOC_EJECT      _BIOC(0x0003)     /* Eject/disable media in the slot
                                           * IN:  None
                                           * OUT: None (ioctl return value provides
                                           *      success/failure indication). */
#define BIOC_LLFORMAT   _BIOC(0x0004)     /* Low-Level Format on SMART flash devices
                                           * IN:  None
                                           * OUT: None (ioctl return value provides
                                           *      success/failure indication). */
#define BIOC_GETFORMAT  _BIOC(0x0005)     /* Returns SMART flash format information
                                           * such as format status, logical sector
                                           * size, total sectors, free sectors, etc.
                                           * IN:  None
                                           * OUT: Pointer to the format information. */
#define BIOC_ALLOCSECT  _BIOC(0x0006)     /* Allocate a logical sector from the block
                                           * device.
                                           * IN:  None
                                           * OUT: Logical sector number allocated. */
#define BIOC_FREESECT   _BIOC(0x0007)     /* Allocate a logical sector from the block
                                           * device.
                                           * IN:  None
                                           * OUT: Logical sector number allocated. */
#define BIOC_READSECT   _BIOC(0x0008)     /* Read a logical sector from the block
                                           * device.
                                           * IN:  Pointer to sector read data (the
                                           *      logical sector number, count and
                                           *      read buffer address
                                           * OUT: Number of bytes read or error */
#define BIOC_WRITESECT  _BIOC(0x0009)     /* Write to data to a logical sector
                                           * IN:  Pointer to sector write data (the
                                           *      logical sector number and write
                                           *      buffer address
                                           * OUT: None (ioctl return value provides
                                           *      success/failure indication). */
#define BIOC_GETPROCFSD _BIOC(0x000a)     /* Get ProcFS data specific to the
                                           * block device.
                                           * IN:  Pointer to a struct defined for
                                           *      the block to load with it's
                                           *      ProcFS data.
                                           * OUT: None (ioctl return value provides
                                           *      success/failure indication). */
#define BIOC_DEBUGCMD   _BIOC(0x000b)     /* Send driver specific debug command /
                                           * data to the block device.
                                           * IN:  Pointer to a struct defined for
                                           *      the block with specific debug
                                           *      command and data.
                                           * OUT: None.  */
#define BIOC_GEOMETRY   _BIOC(0x000c)     /* Used only by BCH to return the
                                           * geometry of the contained block
                                           * driver.
                                           * IN:  Pointer to writable instance
                                           *      of struct geometry in which
                                           *      to return geometry.
                                           * OUT: Data return in user-provided
                                           *      buffer. */
#define BIOC_FLUSH      _BIOC(0x000d)     /* Flush the block device write buffer
                                           * IN:  None
                                           * OUT: None (ioctl return value provides
                                           *      success/failure indication). */
#define BIOC_PARTINFO   _BIOC(0x000e)     /* Retrieve partition information from the
                                           * block device.
                                           * IN:  Pointer to writable struct
                                           *      partition_info_s in which to
                                           *      receive partition information data
                                           * OUT: Partition information structure
                                           *      populated with data from the block
                                           *      device partition */
#define BIOC_BLKSSZGET  _BIOC(0x000f)     /* Get block device sector size.
                                           * IN:  Pointer to writable instance
                                           *      of sector size in which
                                           *      to return sector size.
                                           * OUT: Data return in user-provided
                                           *      buffer. */
#define BIOC_BLKGETSIZE _BIOC(0x0010)     /* Get block device sector numbers.
                                           * IN:  Pointer to writable instance
                                           *      of sector numbers in which
                                           *      to return sector numbers.
                                           * OUT: Data return in user-provided
                                           *      buffer. */

/* Aos MTD driver ioctl definitions ***************************************/

#define _MTDIOCVALID(c)   (_IOC_TYPE(c)==_MTDIOCBASE)
#define _MTDIOC(nr)       _IOC(_MTDIOCBASE,nr)

/****************************************************************************
 * Public Data
 ****************************************************************************/

#ifdef __cplusplus
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

#undef EXTERN
#ifdef __cplusplus
}
#endif

#endif /* __INCLUDE_AOS_FS_IOCTL_H */
