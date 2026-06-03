/***************************************************************************
 *
 * Copyright (c) 2015-2025 BES Technic
 *
 * Authored by BES CD team (Blueelf Prj).
 *
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
#ifndef __CO_JOB_H__
#define __CO_JOB_H__

/**
 * MACRO DEFINES
 *
 *
 */
#define CO_JOB_ARRAY_EMPTY              (0)
#define CO_JOB_ENABLE_DEBUG             (0)

#if (CO_JOB_ENABLE_DEBUG)
#define CO_JOB_INFO(...)                DEBUG_INFO(0, ##__VA_ARGS__)
#else
#define CO_JOB_INFO(...)
#endif

#define CO_JOB_WARN(...)                DEBUG_WARNING(0, ##__VA_ARGS__)

#define CO_JOB_LOG_WARN(str, ...)       CO_JOB_WARN("[W][JOB][%d]"str, __LINE__, ##__VA_ARGS__)
#define CO_JOB_LOG_DEBUG(str, ...)      CO_JOB_INFO("[D][JOB][%d]"str, __LINE__, ##__VA_ARGS__)

#define CO_JOB_TRIGGER_HANDLER()        do { osapi_notify_evm(); } while (0)

#define CO_JOB_INT_DISABLE()            do { uint32_t lock = int_lock()
#define CO_JOB_INT_RESTORE()            int_unlock(lock); } while (0)

#define CO_JOB_SEND(prio, func, params_ptr, params_size) \
                                        do {\
                                            if (func != NULL) {\
                                                co_job_t *p_job = co_job_alloc(params_size);\
                                                if (p_job != NULL) {\
                                                    p_job->runner = (runner_func)func;\
                                                    if (params_ptr != NULL && params_size != 0) {\
                                                        memcpy(p_job->params, params_ptr, params_size); }\
                                                    if (prio < CO_JOB_QUEUE_PRIO_NUM_MAX) {\
                                                    co_job_push(prio, p_job); }\
                                                }\
                                            } \
                                        } while (0)

#define CO_JOB_NO_PARAM_SEND(prio, func) \
                                        do {\
                                            if (func != NULL) {\
                                                co_job_t *p_job = co_job_alloc(0);\
                                                if (p_job != NULL) {\
                                                    p_job->runner = (runner_func)func;\
                                                    if (prio < CO_JOB_QUEUE_PRIO_NUM_MAX) {\
                                                    co_job_push(prio, p_job); }\
                                                }\
                                            }\
                                        } while (0)

#define CO_JOB_LOW_SEND(func, params_ptr, params_size) \
                                        CO_JOB_SEND(CO_JOB_QUEUE_PRIO_LOW, func, params_ptr, params_size)
#define CO_JOB_HIGH_SEND(func, params_ptr, params_size) \
                                        CO_JOB_SEND(CO_JOB_QUEUE_PRIO_HIGH, func, params_ptr, params_size)
#define CO_JOB_HCI_SEND(func, params_ptr, params_size) \
                                        CO_JOB_SEND(CO_JOB_QUEUE_PRIO_HCI, func, params_ptr, params_size)

#define CO_JOB_NO_PARAM_LOW_SEND(func)  CO_JOB_NO_PARAM_SEND(CO_JOB_QUEUE_PRIO_LOW, func)
#define CO_JOB_NO_PARAM_HIGH_SEND(func) CO_JOB_NO_PARAM_SEND(CO_JOB_QUEUE_PRIO_HIGH, func)
#define CO_JOB_NO_PARAM_HCI_SEND(func)  CO_JOB_NO_PARAM_SEND(CO_JOB_QUEUE_PRIO_HCI, func)

/**
 * DNUMERATIONS
 *
 *
 */
enum co_job_queue_priority
{
    CO_JOB_QUEUE_PRIO_NUM_MIN = 0,
    // Priority for ISR prio job: Realtime handler in BT thread
    CO_JOB_QUEUE_PRIO_ISR = CO_JOB_QUEUE_PRIO_NUM_MIN,
    // Priority for HCI job: HCI RX ISR, HCI TX JOB
    CO_JOB_QUEUE_PRIO_HCI,
    // Priority for high prio job: High Priority function call
    CO_JOB_QUEUE_PRIO_HIGH,
    // Priority for low prio job: Low Proirity function call
    CO_JOB_QUEUE_PRIO_LOW,
    // Priority queue max number
    CO_JOB_QUEUE_PRIO_NUM_MAX,
    // Priority that can not be used
    CO_JOB_QUEUE_PRIO_UNREACHABLE = CO_JOB_QUEUE_PRIO_NUM_MAX,
};

/**
 * TYPEDEFINES
 *
 *
 */
typedef int (*runner_func)(uint32_t *params);
typedef void (*callback_func)(runner_func runner, int ret_val, const uint32_t *params);

typedef struct co_job
{
    /// Job runner
    runner_func runner;
    /// Job callback return with @see runner and @see params
    callback_func callback;
    /// Job runner params
    uint32_t params[CO_JOB_ARRAY_EMPTY];
} co_job_t;

typedef void (*runner)(uint32_t *params);

/**
 * FUNCTIONS DECLARATIONS
 *
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

void co_job_schedule_run(uint32_t param);
void co_job_dump_all_queue(void);
int co_job_init(uint32_t param);
int co_job_deinit(uint32_t param);
co_job_t *co_job_alloc_with_ca(uint16_t param_size, uint32_t caller, uint32_t line);
int co_job_free_with_ca(co_job_t *p_job, uint32_t caller, uint32_t line);
int co_job_push_with_ca(uint32_t prio, co_job_t *p_job, uint32_t caller, uint32_t line);

#define co_job_alloc(param_size)        co_job_alloc_with_ca(param_size, (uint32_t)(uintptr_t)__builtin_return_address(0), __LINE__)
#define co_job_free(p_job)              co_job_free_with_ca(p_job, (uint32_t)(uintptr_t)__builtin_return_address(0), __LINE__)
#define co_job_push(prio, p_job)        co_job_push_with_ca(prio, p_job, (uint32_t)(uintptr_t)__builtin_return_address(0), __LINE__)

#ifdef __cplusplus
}
#endif

#endif /// __CO_JOB_H__