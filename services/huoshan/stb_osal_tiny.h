
/* Copyright 2024-2026 VE Starburst. All rights reserved.
 *
 * NOTICE：All information contained herein is, and remains the property of VE
 * Starburst. The intellectual and technical concepts contained herein are
 * proprietary to VE Starburst. and may be covered by patents, patents in process,
 * and are protected by trade secret or copyright law. Dissemination of this
 * information or reproduction of this material is strictly forbidden unless prior
 * written permission is obtained from VE Starburst.
 */

#ifndef _STB_OSAL_H
#define _STB_OSAL_H

// #include "rt_thread.h"
// #include "stb_config.h"

#include "plat_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup stb_kernel 内核
 * 提供starburst内核功能的基础API.
 *
 * @{
 */

/**
 * @addtogroup stb_kernel_sysctrl
 * 提供starburst系统控制功能的基础API.
 *
 * @{
 */

/* Define the starburst' Version */
#define SYSINFO_KERNEL_VERSION "STARBURST-R-3.3.1" /**< starburst内核版本信息 */

/* Defined for API with delay time */
#define STB_WAIT_FOREVER 0xffffffffu /**< 阻塞性等待，即一直等待，直到事件发生或资源获得才返回 */
#define STB_NO_WAIT      0x0         /**< 非阻塞性等待，即若无事件发生或无资源可获得，则返回 */

/* Define default starburst task priority*/
#ifndef STB_DEFAULT_APP_PRI
#define STB_DEFAULT_APP_PRI 32       /**< 任务默认优先级，当调用stb_task_new()创建任务时被内核用来设置任务优先级 */
#endif

/* Define default starburst task priority*/
#ifndef STB_LANXUN_APP_PRI
#define STB_LANXUN_APP_PRI 25       /**< 任务优先级 LanXun RTT 默认25, Lanxun: 优先级只能选25，介于main和btstack之间*/
#endif

/** @} */

/**
 * @addtogroup stb_kernel_event
 * 提供starburst系统内核事件功能的基础API.
 *
 * @{
 */
#define STB_EVENT_AND              0x02u /**< 期望事件标志位均为1时，即等待所有事件均发生时，任务解除阻塞 */
#define STB_EVENT_AND_CLEAR        0x03u /**< 期望事件标志位均为1时，即等待所有事件均发生时，任务解除阻塞且将事件标志位清零 */
#define STB_EVENT_OR               0x00u /**< 期望任意事件标志位为1时，即等待的任意事件发生，任务解除阻塞 */
#define STB_EVENT_OR_CLEAR         0x01u /**< 期望任意事件标志位为1时，即等待的任意事件发生，任务解除阻塞且将事件标志位清零 */
/** @} */

/**
 * @addtogroup stb_kernel_task
 * 提供starburst系统内核任务管理功能的基础API.
 *
 * @{
 */
#define STB_TASK_NONE              0x0u  /**< 表示不指定任意选项，当调用stb_task_create()创建任务时，用来指定option参数 */
#define STB_TASK_AUTORUN           0x01u /**< 表示任务创建后即可被调度执行，当调用stb_task_create()创建任务时，用来指定option参数*/
/** @} */

/**
 * @addtogroup stb_kernel_timer
 * 提供starburst系统内核定时器功能的基础API.
 *
 * @{
 */
#define STB_TIMER_NONE             0x0u  /**< 表示不指定任意选项，当调用stb_timer_create()创建定时器时，用来指定option参数 */
#define STB_TIMER_AUTORUN          0x01u /**< 表示定时器创建后即启动，当调用stb_timer_create()创建定时器时，用来指定option参数*/
#define STB_TIMER_REPEAT           0x02u /**< 表示定时器是周期性的，当调用stb_timer_create()创建定时器时，用来指定option参数 */
/** @} */

/**
 * @addtogroup stb_kernel_sysctrl
 * 提供starburst系统控制功能的基础API.
 *
 * @{
 */

/* Define the handle for all starburst module     */
typedef void *stb_hdl_t;    /**< STARBURST内核对象句柄通用类型 */
/** @} */


/**
 * @addtogroup stb_kernel_sysctrl
 * 提供starburst系统控制功能的基础API.
 *
 * @{
 */
/* Define the data type for function return */
typedef int32_t stb_status_t; /**< STARBURST返回值状态类型 */
/** @} */



#define STB_SUCCESS 0
#define STB_ERROR -1
#define STB_FAIL -1



/**
 * Alloc memory.
 *
 * @param[in]  size  size of the mem to malloc.
 *
 * @return  NULL: error.
 */
void *stb_malloc(size_t size);



/**
 * Free memory.
 *
 * @param[in]  ptr  address point of the mem.
 *
 * @return  none.
 */
void stb_free(void *mem);

/** @} */



/**
 * Get current time in milliseconds.
 *
 * @return  elapsed time in milliseconds from system starting.
 */
uint32_t stb_now_ms(void);

/**
 * Initialize the random number generator.
 */
void stb_rand_init(void);


/**
 * rand function.
 *
 * @return  random value.
 */
uint32_t stb_rand(void);


/**
 * msleep.
 *
 * @param[in]  ms  sleep time in milliseconds.
 */
void stb_msleep(uint32_t ms);

#ifdef __cplusplus
}
#endif

#endif /* STB_OSAL_H */
