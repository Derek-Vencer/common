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
#ifndef __CO_RPC_H__
#define __CO_RPC_H__

/**
 * INCLUDES
 *
 */

#include "bt_callback_func.h"
#include "bt_common_define.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * TYPEDEFINS
 *
 */

#define CO_RPC_FEILD_MASK(bytes)            ((1ULL << (bytes * 8)) - 1)
#define CO_RPC_GET_ARG(a)                   (sizeof(a) == sizeof(uint32_t) ? ((uint32_t)a) : (((uint32_t)a) & CO_RPC_FEILD_MASK(sizeof(a))))

#if 1 //always assume that a function has a return value, It is harmless.

#define CO_RPC_HAVE_RETURN(ret_type)        (1)

#else

#define CO_RPC_HAVE_RETURN(ret_type)        ({\
        uint32_t __r = 1; \
        char* pc_ret_type = __STRING(ret_type); \
        if (strcmp(pc_ret_type, "void")==0) {__r = 0;} \
        __r; })
#endif

#if ! defined ( __GNUC__ )
#error "Not support on non gnu c"
#endif

/**
 * @brief   CO RPC with no arguments.
 *
 * @param[in]   ret_type    the return type of func(next argument)
 * @param[in]   func        rpc function to be called
 * @return   value depends on ret_type, return nothing when ret_type=void
 */
#define __CO_RPC_ARG0_SYNC_WITH_RET(ret_type, func)    ({\
        ASSERT(sizeof(ret_type)<=sizeof(uint32_t), "return type is too large!"); \
        uint32_t _ret; \
        __typeof__(ret_type) (*pfn_arg0)(void)  = func; \
        uint32_t rpc_ext_args[2]; \
        *(rpc_ext_args+0) = (uint32_t) pfn_arg0; \
        *(rpc_ext_args+1) = (uint32_t)(rpc_ext_args+1); \
        _ret = __co_rpc_send(rpc_ext_args, ARRAY_SIZE(rpc_ext_args), true, false); \
        ASSERT(_ret == BT_STS_SUCCESS, "rpc send error, ret = %d", _ret); \
        _ret = *(rpc_ext_args+1); \
        (__typeof__(ret_type))_ret; })

/**
 * @brief   CO RPC with 1 argument.
 *
 * @param[in]   ret_type    the return type of func(next argument)
 * @param[in]   func        rpc function to be called
 * @param[in]   a1          argument of the rpc
 *
 * @return   value depends on ret_type, return nothing when ret_type=void
 */
#define __CO_RPC_ARG1_SYNC_WITH_RET(ret_type, func, a1)    ({\
        ASSERT(sizeof(ret_type)<=sizeof(uint32_t), "return type is too large!"); \
        ASSERT(sizeof(a1)<=sizeof(uint32_t), " argument a1 is too large!"); \
        uint32_t _ret; \
        __typeof__(ret_type) (*pfn_arg1)(__typeof__(a1))  = func; \
        uint32_t rpc_ext_args[3]; \
        *(rpc_ext_args+0) = (uint32_t) pfn_arg1; \
        *(rpc_ext_args+1) = (uint32_t)(rpc_ext_args+1); \
        *(rpc_ext_args+2) = (uint32_t) CO_RPC_GET_ARG(a1); \
        _ret = __co_rpc_send(rpc_ext_args, ARRAY_SIZE(rpc_ext_args), true, false); \
        ASSERT(_ret == BT_STS_SUCCESS, "rpc send error, ret = %d", _ret); \
        _ret = *(rpc_ext_args+1); \
        (__typeof__(ret_type))_ret; })

/**
 * @brief CO RPC with 2 arguments.
 *
 * @param[in]    ret_type    the return type of func(next argument)
 * @param[in]    func        rpc function to be called
 * @param[in]    a1          argument of the rpc function
 * @param[in]    a2          argument of the rpc
 *
 * @return     value depends on ret_type, return nothing when ret_type=void
 */
#define __CO_RPC_ARG2_SYNC_WITH_RET(ret_type, func, a1, a2)    ({\
        ASSERT(sizeof(ret_type)<=sizeof(uint32_t), "return type is too large!"); \
        ASSERT(sizeof(a1)<=sizeof(uint32_t), " argument a1 is too large!"); \
        ASSERT(sizeof(a2)<=sizeof(uint32_t), " argument a2 is too large!"); \
        uint32_t _ret; \
        __typeof__(ret_type) (*pfn_arg2)(__typeof__(a1), __typeof__(a2))  = func; \
        uint32_t rpc_ext_args[4]; \
        *(rpc_ext_args+0) = (uint32_t) pfn_arg2; \
        *(rpc_ext_args+1) = (uint32_t)(rpc_ext_args+1); \
        *(rpc_ext_args+2) = (uint32_t) CO_RPC_GET_ARG(a1); \
        *(rpc_ext_args+3) = (uint32_t) CO_RPC_GET_ARG(a2); \
        _ret = __co_rpc_send(rpc_ext_args, ARRAY_SIZE(rpc_ext_args), true, false); \
        ASSERT(_ret == BT_STS_SUCCESS, "rpc send error, ret = %d", _ret); \
        _ret = *(rpc_ext_args+1); \
        (__typeof__(ret_type))_ret; })

/**
 * @brief CO RPC with 3 arguments.
 *
 * @param[in]    ret_type    the return type of func(next argument)
 * @param[in]    func        rpc function to be called
 * @param[in]    a1          argument of the rpc function
 * @param[in]    a2          argument of the rpc function
 * @param[in]    a3          argument of the rpc
 *
 * @return     value depends on ret_type, return nothing when ret_type=void
 */
#define __CO_RPC_ARG3_SYNC_WITH_RET(ret_type, func, a1, a2, a3)    ({\
        ASSERT(sizeof(ret_type)<=sizeof(uint32_t), "return type is too large!"); \
        ASSERT(sizeof(a1)<=sizeof(uint32_t), " argument a1 is too large!"); \
        ASSERT(sizeof(a2)<=sizeof(uint32_t), " argument a2 is too large!"); \
        ASSERT(sizeof(a3)<=sizeof(uint32_t), " argument a3 is too large!"); \
        uint32_t _ret; \
        __typeof__(ret_type) (*pfn_arg3)(__typeof__(a1), __typeof__(a2), __typeof__(a3))  = func; \
        uint32_t rpc_ext_args[5]; \
        *(rpc_ext_args+0) = (uint32_t) pfn_arg3; \
        *(rpc_ext_args+1) = (uint32_t)(rpc_ext_args+1); \
        *(rpc_ext_args+2) = (uint32_t) CO_RPC_GET_ARG(a1); \
        *(rpc_ext_args+3) = (uint32_t) CO_RPC_GET_ARG(a2); \
        *(rpc_ext_args+4) = (uint32_t) CO_RPC_GET_ARG(a3); \
        _ret = __co_rpc_send(rpc_ext_args, ARRAY_SIZE(rpc_ext_args), true, false); \
        ASSERT(_ret == BT_STS_SUCCESS, "rpc send error, ret = %d", _ret); \
        _ret = *(rpc_ext_args+1); \
        (__typeof__(ret_type))_ret; })

/**
 * @brief CO RPC with 4 arguments.
 *
 * @param[in]    ret_type    the return type of func(next argument)
 * @param[in]    func        rpc function to be called
 * @param[in]    a1          argument of the rpc function
 * @param[in]    a2          argument of the rpc function
 * @param[in]    a3          argument of the rpc function
 * @param[in]    a4          argument of the rpc
 *
 * @return     value depends on ret_type, return nothing when ret_type=void
 */
#define __CO_RPC_ARG4_SYNC_WITH_RET(ret_type, func, a1, a2, a3, a4)    ({\
        ASSERT(sizeof(ret_type)<=sizeof(uint32_t), "return type is too large!"); \
        ASSERT(sizeof(a1)<=sizeof(uint32_t), " argument a1 is too large!"); \
        ASSERT(sizeof(a2)<=sizeof(uint32_t), " argument a2 is too large!"); \
        ASSERT(sizeof(a3)<=sizeof(uint32_t), " argument a3 is too large!"); \
        ASSERT(sizeof(a4)<=sizeof(uint32_t), " argument a4 is too large!"); \
        uint32_t _ret; \
        __typeof__(ret_type) (*pfn_arg4)(__typeof__(a1), __typeof__(a2), __typeof__(a3), __typeof__(a4))  = func; \
        uint32_t rpc_ext_args[6]; \
        *(rpc_ext_args+0) = (uint32_t) pfn_arg4; \
        *(rpc_ext_args+1) = (uint32_t)(rpc_ext_args+1); \
        *(rpc_ext_args+2) = (uint32_t) CO_RPC_GET_ARG(a1); \
        *(rpc_ext_args+3) = (uint32_t) CO_RPC_GET_ARG(a2); \
        *(rpc_ext_args+4) = (uint32_t) CO_RPC_GET_ARG(a3); \
        *(rpc_ext_args+5) = (uint32_t) CO_RPC_GET_ARG(a4); \
        _ret = __co_rpc_send(rpc_ext_args, ARRAY_SIZE(rpc_ext_args), true, false); \
        ASSERT(_ret == BT_STS_SUCCESS, "rpc send error, ret = %d", _ret); \
        _ret = *(rpc_ext_args+1); \
        (__typeof__(ret_type))_ret; })

/**
 * @brief CO RPC with 5 arguments.
 *
 * @param[in]    ret_type    the return type of func(next argument)
 * @param[in]    func        rpc function to be called
 * @param[in]    a1          argument of the rpc function
 * @param[in]    a2          argument of the rpc function
 * @param[in]    a3          argument of the rpc function
 * @param[in]    a4          argument of the rpc function
 * @param[in]    a5          argument of the rpc
 *
 * @return     value depends on ret_type, return nothing when ret_type=void
 */
#define __CO_RPC_ARG5_SYNC_WITH_RET(ret_type, func, a1, a2, a3, a4, a5)    ({\
        ASSERT(sizeof(ret_type)<=sizeof(uint32_t), "return type is too large!"); \
        ASSERT(sizeof(a1)<=sizeof(uint32_t), " argument a1 is too large!"); \
        ASSERT(sizeof(a2)<=sizeof(uint32_t), " argument a2 is too large!"); \
        ASSERT(sizeof(a3)<=sizeof(uint32_t), " argument a3 is too large!"); \
        ASSERT(sizeof(a4)<=sizeof(uint32_t), " argument a4 is too large!"); \
        ASSERT(sizeof(a5)<=sizeof(uint32_t), " argument a5 is too large!"); \
        uint32_t _ret; \
        __typeof__(ret_type) (*pfn_arg5)(__typeof__(a1), __typeof__(a2), __typeof__(a3), __typeof__(a4), \
                                         __typeof__(a5))  = func; \
        uint32_t rpc_ext_args[7]; \
        *(rpc_ext_args+0) = (uint32_t) pfn_arg5; \
        *(rpc_ext_args+1) = (uint32_t)(rpc_ext_args+1); \
        *(rpc_ext_args+2) = (uint32_t) CO_RPC_GET_ARG(a1); \
        *(rpc_ext_args+3) = (uint32_t) CO_RPC_GET_ARG(a2); \
        *(rpc_ext_args+4) = (uint32_t) CO_RPC_GET_ARG(a3); \
        *(rpc_ext_args+5) = (uint32_t) CO_RPC_GET_ARG(a4); \
        *(rpc_ext_args+6) = (uint32_t) CO_RPC_GET_ARG(a5); \
        _ret = __co_rpc_send(rpc_ext_args, ARRAY_SIZE(rpc_ext_args), true, false); \
        ASSERT(_ret == BT_STS_SUCCESS, "rpc send error, ret = %d", _ret); \
        _ret = *(rpc_ext_args+1); \
        (__typeof__(ret_type))_ret; })

/**
 * @brief CO RPC with 6 arguments.
 *
 * @param[in]    ret_type    the return type of func(next argument)
 * @param[in]    func        rpc function to be called
 * @param[in]    a1          argument of the rpc function
 * @param[in]    a2          argument of the rpc function
 * @param[in]    a3          argument of the rpc function
 * @param[in]    a4          argument of the rpc function
 * @param[in]    a5          argument of the rpc function
 * @param[in]    a6          argument of the rpc function
 *
 * @return     value depends on ret_type, return nothing when ret_type=void
 */
#define __CO_RPC_ARG6_SYNC_WITH_RET(ret_type, func, a1, a2, a3, a4, a5, a6)    ({\
        ASSERT(sizeof(ret_type)<=sizeof(uint32_t), "return type is too large!"); \
        ASSERT(sizeof(a1)<=sizeof(uint32_t), " argument a1 is too large!"); \
        ASSERT(sizeof(a2)<=sizeof(uint32_t), " argument a2 is too large!"); \
        ASSERT(sizeof(a3)<=sizeof(uint32_t), " argument a3 is too large!"); \
        ASSERT(sizeof(a4)<=sizeof(uint32_t), " argument a4 is too large!"); \
        ASSERT(sizeof(a5)<=sizeof(uint32_t), " argument a5 is too large!"); \
        ASSERT(sizeof(a6)<=sizeof(uint32_t), " argument a6 is too large!"); \
        uint32_t _ret; \
        __typeof__(ret_type) (*pfn_arg6)(__typeof__(a1), __typeof__(a2), __typeof__(a3), __typeof__(a4), \
                                         __typeof__(a5), __typeof__(a6))  = func; \
        uint32_t rpc_ext_args[8]; \
        *(rpc_ext_args+0) = (uint32_t) pfn_arg6; \
        *(rpc_ext_args+1) = (uint32_t)(rpc_ext_args+1); \
        *(rpc_ext_args+2) = (uint32_t) CO_RPC_GET_ARG(a1); \
        *(rpc_ext_args+3) = (uint32_t) CO_RPC_GET_ARG(a2); \
        *(rpc_ext_args+4) = (uint32_t) CO_RPC_GET_ARG(a3); \
        *(rpc_ext_args+5) = (uint32_t) CO_RPC_GET_ARG(a4); \
        *(rpc_ext_args+6) = (uint32_t) CO_RPC_GET_ARG(a5); \
        *(rpc_ext_args+7) = (uint32_t) CO_RPC_GET_ARG(a6); \
        _ret = __co_rpc_send(rpc_ext_args, ARRAY_SIZE(rpc_ext_args), true, false); \
        ASSERT(_ret == BT_STS_SUCCESS, "rpc send error, ret = %d", _ret); \
        _ret = *(rpc_ext_args+1); \
        (__typeof__(ret_type))_ret; })

/**
 * @brief   CO RPC with no arguments.
 *
 * @param[in]   func        rpc function to be called
 * @return   value depends on ret_type, return nothing when ret_type=void
 */
#define __CO_RPC_ARG0_SYNC_RET_VOID(func)    ({\
        void (*pfn_arg0)(void)  = func; \
        uint32_t rpc_ext_args[2]; \
        *(rpc_ext_args+0) = (uint32_t) pfn_arg0; \
        *(rpc_ext_args+1) = (uint32_t)(rpc_ext_args+1); \
        uint32_t _ret = __co_rpc_send(rpc_ext_args, ARRAY_SIZE(rpc_ext_args), false, false); \
        ASSERT(_ret == BT_STS_SUCCESS, "rpc send error, ret = %d", _ret); })

/**
 * @brief   CO RPC with 1 argument.
 *
 * @param[in]   func        rpc function to be called
 * @param[in]   a1          argument of the rpc
 *
 * @return   value depends on ret_type, return nothing when ret_type=void
 */
#define __CO_RPC_ARG1_SYNC_RET_VOID(func, a1)    ({\
        ASSERT(sizeof(a1)<=sizeof(uint32_t), " argument a1 is too large!"); \
        void (*pfn_arg1)(__typeof__(a1))  = func; \
        uint32_t rpc_ext_args[3]; \
        *(rpc_ext_args+0) = (uint32_t) pfn_arg1; \
        *(rpc_ext_args+1) = (uint32_t)(rpc_ext_args+1); \
        *(rpc_ext_args+2) = (uint32_t) CO_RPC_GET_ARG(a1); \
        uint32_t _ret = __co_rpc_send(rpc_ext_args, ARRAY_SIZE(rpc_ext_args), false, false); \
        ASSERT(_ret == BT_STS_SUCCESS, "rpc send error, ret = %d", _ret); })

/**
 * @brief CO RPC with 2 arguments.
 *
 * @param[in]    func        rpc function to be called
 * @param[in]    a1          argument of the rpc function
 * @param[in]    a2          argument of the rpc
 *
 * @return     value depends on ret_type, return nothing when ret_type=void
 */
#define __CO_RPC_ARG2_SYNC_RET_VOID(func, a1, a2)    ({\
        ASSERT(sizeof(a1)<=sizeof(uint32_t), " argument a1 is too large!"); \
        ASSERT(sizeof(a2)<=sizeof(uint32_t), " argument a2 is too large!"); \
        void (*pfn_arg2)(__typeof__(a1), __typeof__(a2))  = func; \
        uint32_t rpc_ext_args[4]; \
        *(rpc_ext_args+0) = (uint32_t) pfn_arg2; \
        *(rpc_ext_args+1) = (uint32_t)(rpc_ext_args+1); \
        *(rpc_ext_args+2) = (uint32_t) CO_RPC_GET_ARG(a1); \
        *(rpc_ext_args+3) = (uint32_t) CO_RPC_GET_ARG(a2); \
        uint32_t _ret = __co_rpc_send(rpc_ext_args, ARRAY_SIZE(rpc_ext_args), false, false); \
        ASSERT(_ret == BT_STS_SUCCESS, "rpc send error, ret = %d", _ret); })

/**
 * @brief CO RPC with 3 arguments.
 *
 * @param[in]    func        rpc function to be called
 * @param[in]    a1          argument of the rpc function
 * @param[in]    a2          argument of the rpc function
 * @param[in]    a3          argument of the rpc
 *
 * @return     value depends on ret_type, return nothing when ret_type=void
 */
#define __CO_RPC_ARG3_SYNC_RET_VOID(func, a1, a2, a3)    ({\
        ASSERT(sizeof(a1)<=sizeof(uint32_t), " argument a1 is too large!"); \
        ASSERT(sizeof(a2)<=sizeof(uint32_t), " argument a2 is too large!"); \
        ASSERT(sizeof(a3)<=sizeof(uint32_t), " argument a3 is too large!"); \
        void (*pfn_arg3)(__typeof__(a1), __typeof__(a2), __typeof__(a3))  = func; \
        uint32_t rpc_ext_args[5]; \
        *(rpc_ext_args+0) = (uint32_t) pfn_arg3; \
        *(rpc_ext_args+1) = (uint32_t)(rpc_ext_args+1); \
        *(rpc_ext_args+2) = (uint32_t) CO_RPC_GET_ARG(a1); \
        *(rpc_ext_args+3) = (uint32_t) CO_RPC_GET_ARG(a2); \
        *(rpc_ext_args+4) = (uint32_t) CO_RPC_GET_ARG(a3); \
        uint32_t _ret = __co_rpc_send(rpc_ext_args, ARRAY_SIZE(rpc_ext_args), false, false); \
        ASSERT(_ret == BT_STS_SUCCESS, "rpc send error, ret = %d", _ret); })

/**
 * @brief CO RPC with 4 arguments.
 *
 * @param[in]    func        rpc function to be called
 * @param[in]    a1          argument of the rpc function
 * @param[in]    a2          argument of the rpc function
 * @param[in]    a3          argument of the rpc function
 * @param[in]    a4          argument of the rpc
 *
 * @return     value depends on ret_type, return nothing when ret_type=void
 */
#define __CO_RPC_ARG4_SYNC_RET_VOID(func, a1, a2, a3, a4)    ({\
        ASSERT(sizeof(a1)<=sizeof(uint32_t), " argument a1 is too large!"); \
        ASSERT(sizeof(a2)<=sizeof(uint32_t), " argument a2 is too large!"); \
        ASSERT(sizeof(a3)<=sizeof(uint32_t), " argument a3 is too large!"); \
        ASSERT(sizeof(a4)<=sizeof(uint32_t), " argument a4 is too large!"); \
        void (*pfn_arg4)(__typeof__(a1), __typeof__(a2), __typeof__(a3), __typeof__(a4))  = func; \
        uint32_t rpc_ext_args[6]; \
        *(rpc_ext_args+0) = (uint32_t) pfn_arg4; \
        *(rpc_ext_args+1) = (uint32_t)(rpc_ext_args+1); \
        *(rpc_ext_args+2) = (uint32_t) CO_RPC_GET_ARG(a1); \
        *(rpc_ext_args+3) = (uint32_t) CO_RPC_GET_ARG(a2); \
        *(rpc_ext_args+4) = (uint32_t) CO_RPC_GET_ARG(a3); \
        *(rpc_ext_args+5) = (uint32_t) CO_RPC_GET_ARG(a4); \
        uint32_t _ret = __co_rpc_send(rpc_ext_args, ARRAY_SIZE(rpc_ext_args), false, false); \
        ASSERT(_ret == BT_STS_SUCCESS, "rpc send error, ret = %d", _ret); })

/**
 * @brief CO RPC with 5 arguments.
 *
 * @param[in]    func        rpc function to be called
 * @param[in]    a1          argument of the rpc function
 * @param[in]    a2          argument of the rpc function
 * @param[in]    a3          argument of the rpc function
 * @param[in]    a4          argument of the rpc function
 * @param[in]    a5          argument of the rpc
 *
 * @return     value depends on ret_type, return nothing when ret_type=void
 */
#define __CO_RPC_ARG5_SYNC_RET_VOID(func, a1, a2, a3, a4, a5)    ({\
        ASSERT(sizeof(a1)<=sizeof(uint32_t), " argument a1 is too large!"); \
        ASSERT(sizeof(a2)<=sizeof(uint32_t), " argument a2 is too large!"); \
        ASSERT(sizeof(a3)<=sizeof(uint32_t), " argument a3 is too large!"); \
        ASSERT(sizeof(a4)<=sizeof(uint32_t), " argument a4 is too large!"); \
        ASSERT(sizeof(a5)<=sizeof(uint32_t), " argument a5 is too large!"); \
        void (*pfn_arg5)(__typeof__(a1), __typeof__(a2), __typeof__(a3), __typeof__(a4), \
                                         __typeof__(a5))  = func; \
        uint32_t rpc_ext_args[7]; \
        *(rpc_ext_args+0) = (uint32_t) pfn_arg5; \
        *(rpc_ext_args+1) = (uint32_t)(rpc_ext_args+1); \
        *(rpc_ext_args+2) = (uint32_t) CO_RPC_GET_ARG(a1); \
        *(rpc_ext_args+3) = (uint32_t) CO_RPC_GET_ARG(a2); \
        *(rpc_ext_args+4) = (uint32_t) CO_RPC_GET_ARG(a3); \
        *(rpc_ext_args+5) = (uint32_t) CO_RPC_GET_ARG(a4); \
        *(rpc_ext_args+6) = (uint32_t) CO_RPC_GET_ARG(a5); \
        uint32_t _ret = __co_rpc_send(rpc_ext_args, ARRAY_SIZE(rpc_ext_args), false, false); \
        ASSERT(_ret == BT_STS_SUCCESS, "rpc send error, ret = %d", _ret); })

/**
 * @brief CO RPC with 6 arguments.
 *
 * @param[in]    func        rpc function to be called
 * @param[in]    a1          argument of the rpc function
 * @param[in]    a2          argument of the rpc function
 * @param[in]    a3          argument of the rpc function
 * @param[in]    a4          argument of the rpc function
 * @param[in]    a5          argument of the rpc function
 * @param[in]    a6          argument of the rpc
 *
 * @return     value depends on ret_type, return nothing when ret_type=void
 */
#define __CO_RPC_ARG6_SYNC_RET_VOID(func, a1, a2, a3, a4, a5, a6)    ({\
        ASSERT(sizeof(a1)<=sizeof(uint32_t), " argument a1 is too large!"); \
        ASSERT(sizeof(a2)<=sizeof(uint32_t), " argument a2 is too large!"); \
        ASSERT(sizeof(a3)<=sizeof(uint32_t), " argument a3 is too large!"); \
        ASSERT(sizeof(a4)<=sizeof(uint32_t), " argument a4 is too large!"); \
        ASSERT(sizeof(a5)<=sizeof(uint32_t), " argument a5 is too large!"); \
        ASSERT(sizeof(a6)<=sizeof(uint32_t), " argument a6 is too large!"); \
        void (*pfn_arg6)(__typeof__(a1), __typeof__(a2), __typeof__(a3), __typeof__(a4), \
                                         __typeof__(a5), __typeof__(a6))  = func; \
        uint32_t rpc_ext_args[8]; \
        *(rpc_ext_args+0) = (uint32_t) pfn_arg6; \
        *(rpc_ext_args+1) = (uint32_t)(rpc_ext_args+1); \
        *(rpc_ext_args+2) = (uint32_t) CO_RPC_GET_ARG(a1); \
        *(rpc_ext_args+3) = (uint32_t) CO_RPC_GET_ARG(a2); \
        *(rpc_ext_args+4) = (uint32_t) CO_RPC_GET_ARG(a3); \
        *(rpc_ext_args+5) = (uint32_t) CO_RPC_GET_ARG(a4); \
        *(rpc_ext_args+6) = (uint32_t) CO_RPC_GET_ARG(a5); \
        *(rpc_ext_args+7) = (uint32_t) CO_RPC_GET_ARG(a6); \
        uint32_t _ret = __co_rpc_send(rpc_ext_args, ARRAY_SIZE(rpc_ext_args), false, false); \
        ASSERT(_ret == BT_STS_SUCCESS, "rpc send error, ret = %d", _ret); })

/**
 * @brief CO RPC async with no arguments.
 *
 * @param[in]    res_cb      result callback(see @ref pfunc_tprc_cb_t), can be NULL if you don't care
 * @param[in]    ret_type    the return type of func(next argument)
 * @param[in]    func        rpc function to be called
 */
#define __CO_RPC_ARG0_ASYNC_WITH_RET(res_cb, ret_type, func)    do{\
        ASSERT(sizeof(ret_type)<=sizeof(uint32_t), "return type is too large!"); \
        __typeof__(ret_type) (*pfn_arg0)(void)  = func; \
        uint32_t rpc_ext_args[2]; \
        *(rpc_ext_args+0) = (uint32_t) pfn_arg0; \
        *(rpc_ext_args+1) = (uint32_t) res_cb; \
        uint32_t _ret = __co_rpc_send(rpc_ext_args, ARRAY_SIZE(rpc_ext_args), true, true); \
        ASSERT(_ret == BT_STS_SUCCESS, "rpc send error, ret = %d", _ret); \
    }while(0)

/**
 * @brief CO RPC async with 1 argument.
 *
 * @param[in]    res_cb      result callback(see @ref pfunc_tprc_cb_t), can be NULL if you don't care
 * @param[in]    ret_type    the return type of func(next argument)
 * @param[in]    func        rpc function to be called
 * @param[in]    a1          argument of the rpc
 *
 */
#define __CO_RPC_ARG1_ASYNC_WITH_RET(res_cb, ret_type, func, a1)    do{\
        ASSERT(sizeof(ret_type)<=sizeof(uint32_t), "return type is too large!"); \
        ASSERT(sizeof(a1)<=sizeof(uint32_t), " argument a1 is too large!"); \
        __typeof__(ret_type) (*pfn_arg1)(__typeof__(a1))  = func; \
        uint32_t rpc_ext_args[3]; \
        *(rpc_ext_args+0) = (uint32_t) pfn_arg1; \
        *(rpc_ext_args+1) = (uint32_t) res_cb; \
        *(rpc_ext_args+2) = (uint32_t) CO_RPC_GET_ARG(a1); \
        uint32_t _ret = __co_rpc_send(rpc_ext_args, ARRAY_SIZE(rpc_ext_args), true, true); \
        ASSERT(_ret == BT_STS_SUCCESS, "rpc send error, ret = %d", _ret); \
    }while(0)

/**
 * @brief CO RPC async with 2 arguments.
 *
 * @param[in]    res_cb      result callback(see @ref pfunc_tprc_cb_t), can be NULL if you don't care
 * @param[in]    ret_type    the return type of func(next argument)
 * @param[in]    func        rpc function to be called
 * @param[in]    a1          argument of the rpc function
 * @param[in]    a2          argument of the rpc
 *
 */
#define __CO_RPC_ARG2_ASYNC_WITH_RET(res_cb, ret_type, func, a1, a2)    do{\
        ASSERT(sizeof(ret_type)<=sizeof(uint32_t), "return type is too large!"); \
        ASSERT(sizeof(a1)<=sizeof(uint32_t), " argument a1 is too large!"); \
        ASSERT(sizeof(a2)<=sizeof(uint32_t), " argument a2 is too large!"); \
        __typeof__(ret_type) (*pfn_arg2)(__typeof__(a1), __typeof__(a2))  = func; \
        uint32_t rpc_ext_args[4]; \
        *(rpc_ext_args+0) = (uint32_t) pfn_arg2; \
        *(rpc_ext_args+1) = (uint32_t) res_cb; \
        *(rpc_ext_args+2) = (uint32_t) CO_RPC_GET_ARG(a1); \
        *(rpc_ext_args+3) = (uint32_t) CO_RPC_GET_ARG(a2); \
        uint32_t _ret = __co_rpc_send(rpc_ext_args, ARRAY_SIZE(rpc_ext_args), true, true); \
        ASSERT(_ret == BT_STS_SUCCESS, "rpc send error, ret = %d", _ret); \
    }while(0)

/**
 * @brief CO RPC async with 3 arguments.
 *
 * @param[in]    res_cb      result callback(see @ref pfunc_tprc_cb_t), can be NULL if you don't care
 * @param[in]    ret_type    the return type of func(next argument)
 * @param[in]    func        rpc function to be called
 * @param[in]    a1          argument of the rpc function
 * @param[in]    a2          argument of the rpc function
 * @param[in]    a3          argument of the rpc
 *
 */
#define __CO_RPC_ARG3_ASYNC_WITH_RET(res_cb, ret_type, func, a1, a2, a3)    do{\
        ASSERT(sizeof(ret_type)<=sizeof(uint32_t), "return type is too large!"); \
        ASSERT(sizeof(a1)<=sizeof(uint32_t), " argument a1 is too large!"); \
        ASSERT(sizeof(a2)<=sizeof(uint32_t), " argument a2 is too large!"); \
        ASSERT(sizeof(a3)<=sizeof(uint32_t), " argument a3 is too large!"); \
        __typeof__(ret_type) (*pfn_arg3)(__typeof__(a1), __typeof__(a2), __typeof__(a3))  = func; \
        uint32_t rpc_ext_args[5]; \
        *(rpc_ext_args+0) = (uint32_t) pfn_arg3; \
        *(rpc_ext_args+1) = (uint32_t) res_cb; \
        *(rpc_ext_args+2) = (uint32_t) CO_RPC_GET_ARG(a1); \
        *(rpc_ext_args+3) = (uint32_t) CO_RPC_GET_ARG(a2); \
        *(rpc_ext_args+4) = (uint32_t) CO_RPC_GET_ARG(a3); \
        uint32_t _ret = __co_rpc_send(rpc_ext_args, ARRAY_SIZE(rpc_ext_args), true, true); \
        ASSERT(_ret == BT_STS_SUCCESS, "rpc send error, ret = %d", _ret); \
    }while(0)

/**
 * @brief CO RPC async with 4 arguments.
 *
 * @param[in]    res_cb      result callback(see @ref pfunc_tprc_cb_t), can be NULL if you don't care
 * @param[in]    ret_type    the return type of func(next argument)
 * @param[in]    func        rpc function to be called
 * @param[in]    a1          argument of the rpc function
 * @param[in]    a2          argument of the rpc function
 * @param[in]    a3          argument of the rpc function
 * @param[in]    a4          argument of the rpc
 *
 */
#define __CO_RPC_ARG4_ASYNC_WITH_RET(res_cb, ret_type, func, a1, a2, a3, a4)    do{\
        ASSERT(sizeof(ret_type)<=sizeof(uint32_t), "return type is too large!"); \
        ASSERT(sizeof(a1)<=sizeof(uint32_t), " argument a1 is too large!"); \
        ASSERT(sizeof(a2)<=sizeof(uint32_t), " argument a2 is too large!"); \
        ASSERT(sizeof(a3)<=sizeof(uint32_t), " argument a3 is too large!"); \
        ASSERT(sizeof(a4)<=sizeof(uint32_t), " argument a4 is too large!"); \
        __typeof__(ret_type) (*pfn_arg4)(__typeof__(a1), __typeof__(a2), __typeof__(a3), __typeof__(a4))  = func; \
        uint32_t rpc_ext_args[6]; \
        *(rpc_ext_args+0) = (uint32_t) pfn_arg4; \
        *(rpc_ext_args+1) = (uint32_t) res_cb; \
        *(rpc_ext_args+2) = (uint32_t) CO_RPC_GET_ARG(a1); \
        *(rpc_ext_args+3) = (uint32_t) CO_RPC_GET_ARG(a2); \
        *(rpc_ext_args+4) = (uint32_t) CO_RPC_GET_ARG(a3); \
        *(rpc_ext_args+5) = (uint32_t) CO_RPC_GET_ARG(a4); \
        uint32_t _ret = __co_rpc_send(rpc_ext_args, ARRAY_SIZE(rpc_ext_args), true, true); \
        ASSERT(_ret == BT_STS_SUCCESS, "rpc send error, ret = %d", _ret); \
    }while(0)

/**
 * @brief CO RPC async with 5 arguments.
 *
 * @param[in]    res_cb      result callback(see @ref pfunc_tprc_cb_t), can be NULL if you don't care
 * @param[in]    ret_type    the return type of func(next argument)
 * @param[in]    func        rpc function to be called
 * @param[in]    a1          argument of the rpc function
 * @param[in]    a2          argument of the rpc function
 * @param[in]    a3          argument of the rpc function
 * @param[in]    a4          argument of the rpc function
 * @param[in]    a5          argument of the rpc
 *
 */
#define __CO_RPC_ARG5_ASYNC_WITH_RET(res_cb, ret_type, func, a1, a2, a3, a4, a5)    do{\
        ASSERT(sizeof(ret_type)<=sizeof(uint32_t), "return type is too large!"); \
        ASSERT(sizeof(a1)<=sizeof(uint32_t), " argument a1 is too large!"); \
        ASSERT(sizeof(a2)<=sizeof(uint32_t), " argument a2 is too large!"); \
        ASSERT(sizeof(a3)<=sizeof(uint32_t), " argument a3 is too large!"); \
        ASSERT(sizeof(a4)<=sizeof(uint32_t), " argument a4 is too large!"); \
        ASSERT(sizeof(a5)<=sizeof(uint32_t), " argument a5 is too large!"); \
        __typeof__(ret_type) (*pfn_arg5)(__typeof__(a1), __typeof__(a2), __typeof__(a3), __typeof__(a4), \
                                         __typeof__(a5))  = func; \
        uint32_t rpc_ext_args[7]; \
        *(rpc_ext_args+0) = (uint32_t) pfn_arg5; \
        *(rpc_ext_args+1) = (uint32_t) res_cb; \
        *(rpc_ext_args+2) = (uint32_t) CO_RPC_GET_ARG(a1); \
        *(rpc_ext_args+3) = (uint32_t) CO_RPC_GET_ARG(a2); \
        *(rpc_ext_args+4) = (uint32_t) CO_RPC_GET_ARG(a3); \
        *(rpc_ext_args+5) = (uint32_t) CO_RPC_GET_ARG(a4); \
        *(rpc_ext_args+6) = (uint32_t) CO_RPC_GET_ARG(a5); \
        uint32_t _ret = __co_rpc_send(rpc_ext_args, ARRAY_SIZE(rpc_ext_args), true, true); \
        ASSERT(_ret == BT_STS_SUCCESS, "rpc send error, ret = %d", _ret); \
    }while(0)

/**
 * @brief CO RPC async with 6 arguments.
 *
 * @param[in]    res_cb      result callback(see @ref pfunc_tprc_cb_t), can be NULL if you don't care
 * @param[in]    ret_type    the return type of func(next argument)
 * @param[in]    func        rpc function to be called
 * @param[in]    a1          argument of the rpc function
 * @param[in]    a2          argument of the rpc function
 * @param[in]    a3          argument of the rpc function
 * @param[in]    a4          argument of the rpc function
 * @param[in]    a5          argument of the rpc function
 * @param[in]    a6          argument of the rpc
 *
 */
#define __CO_RPC_ARG6_ASYNC_WITH_RET(res_cb, ret_type, func, a1, a2, a3, a4, a5, a6)    do{\
        ASSERT(sizeof(ret_type)<=sizeof(uint32_t), "return type is too large!"); \
        ASSERT(sizeof(a1)<=sizeof(uint32_t), " argument a1 is too large!"); \
        ASSERT(sizeof(a2)<=sizeof(uint32_t), " argument a2 is too large!"); \
        ASSERT(sizeof(a3)<=sizeof(uint32_t), " argument a3 is too large!"); \
        ASSERT(sizeof(a4)<=sizeof(uint32_t), " argument a4 is too large!"); \
        ASSERT(sizeof(a5)<=sizeof(uint32_t), " argument a5 is too large!"); \
        ASSERT(sizeof(a6)<=sizeof(uint32_t), " argument a6 is too large!"); \
        __typeof__(ret_type) (*pfn_arg6)(__typeof__(a1), __typeof__(a2), __typeof__(a3), __typeof__(a4), \
                                         __typeof__(a5), __typeof__(a6))  = func; \
        uint32_t rpc_ext_args[8]; \
        *(rpc_ext_args+0) = (uint32_t) pfn_arg6; \
        *(rpc_ext_args+1) = (uint32_t) res_cb; \
        *(rpc_ext_args+2) = (uint32_t) CO_RPC_GET_ARG(a1); \
        *(rpc_ext_args+3) = (uint32_t) CO_RPC_GET_ARG(a2); \
        *(rpc_ext_args+4) = (uint32_t) CO_RPC_GET_ARG(a3); \
        *(rpc_ext_args+5) = (uint32_t) CO_RPC_GET_ARG(a4); \
        *(rpc_ext_args+6) = (uint32_t) CO_RPC_GET_ARG(a5); \
        *(rpc_ext_args+7) = (uint32_t) CO_RPC_GET_ARG(a6); \
        uint32_t _ret = __co_rpc_send(rpc_ext_args, ARRAY_SIZE(rpc_ext_args), true, true); \
        ASSERT(_ret == BT_STS_SUCCESS, "rpc send error, ret = %d", _ret); \
    }while(0)

/**
 * @brief CO RPC async with no arguments.
 *
 * @param[in]    res_cb      result callback(see @ref pfunc_tprc_cb_t), can be NULL if you don't care
 * @param[in]    func        rpc function to be called
 */
#define __CO_RPC_ARG0_ASYNC_RET_VOID(res_cb, func)    do{\
        __typeof__(void) (*pfn_arg0)(void)  = func; \
        uint32_t rpc_ext_args[2]; \
        *(rpc_ext_args+0) = (uint32_t) pfn_arg0; \
        *(rpc_ext_args+1) = (uint32_t) res_cb; \
        uint32_t _ret = __co_rpc_send(rpc_ext_args, ARRAY_SIZE(rpc_ext_args), false, true); \
        ASSERT(_ret == BT_STS_SUCCESS, "rpc send error, ret = %d", _ret); \
    }while(0)

/**
 * @brief CO RPC async with 1 argument.
 *
 * @param[in]    res_cb      result callback(see @ref pfunc_tprc_cb_t), can be NULL if you don't care
 * @param[in]    func        rpc function to be called
 * @param[in]    a1          argument of the rpc
 *
 */
#define __CO_RPC_ARG1_ASYNC_RET_VOID(res_cb, func, a1)    do{\
        ASSERT(sizeof(a1)<=sizeof(uint32_t), " argument a1 is too large!"); \
        __typeof__(void) (*pfn_arg1)(__typeof__(a1))  = func; \
        uint32_t rpc_ext_args[3]; \
        *(rpc_ext_args+0) = (uint32_t) pfn_arg1; \
        *(rpc_ext_args+1) = (uint32_t) res_cb; \
        *(rpc_ext_args+2) = (uint32_t) CO_RPC_GET_ARG(a1); \
        uint32_t _ret = __co_rpc_send(rpc_ext_args, ARRAY_SIZE(rpc_ext_args), false, true); \
        ASSERT(_ret == BT_STS_SUCCESS, "rpc send error, ret = %d", _ret); \
    }while(0)

/**
 * @brief CO RPC async with 2 arguments.
 *
 * @param[in]    res_cb      result callback(see @ref pfunc_tprc_cb_t), can be NULL if you don't care
 * @param[in]    func        rpc function to be called
 * @param[in]    a1          argument of the rpc function
 * @param[in]    a2          argument of the rpc
 *
 */
#define __CO_RPC_ARG2_ASYNC_RET_VOID(res_cb, func, a1, a2)    do{\
        ASSERT(sizeof(a1)<=sizeof(uint32_t), " argument a1 is too large!"); \
        ASSERT(sizeof(a2)<=sizeof(uint32_t), " argument a2 is too large!"); \
        __typeof__(void) (*pfn_arg2)(__typeof__(a1), __typeof__(a2))  = func; \
        uint32_t rpc_ext_args[4]; \
        *(rpc_ext_args+0) = (uint32_t) pfn_arg2; \
        *(rpc_ext_args+1) = (uint32_t) res_cb; \
        *(rpc_ext_args+2) = (uint32_t) CO_RPC_GET_ARG(a1); \
        *(rpc_ext_args+3) = (uint32_t) CO_RPC_GET_ARG(a2); \
        uint32_t _ret = __co_rpc_send(rpc_ext_args, ARRAY_SIZE(rpc_ext_args), false, true); \
        ASSERT(_ret == BT_STS_SUCCESS, "rpc send error, ret = %d", _ret); \
    }while(0)

/**
 * @brief CO RPC async with 3 arguments.
 *
 * @param[in]    res_cb      result callback(see @ref pfunc_tprc_cb_t), can be NULL if you don't care
 * @param[in]    func        rpc function to be called
 * @param[in]    a1          argument of the rpc function
 * @param[in]    a2          argument of the rpc function
 * @param[in]    a3          argument of the rpc
 *
 */
#define __CO_RPC_ARG3_ASYNC_RET_VOID(res_cb, func, a1, a2, a3)    do{\
        ASSERT(sizeof(a1)<=sizeof(uint32_t), " argument a1 is too large!"); \
        ASSERT(sizeof(a2)<=sizeof(uint32_t), " argument a2 is too large!"); \
        ASSERT(sizeof(a3)<=sizeof(uint32_t), " argument a3 is too large!"); \
        __typeof__(void) (*pfn_arg3)(__typeof__(a1), __typeof__(a2), __typeof__(a3))  = func; \
        uint32_t rpc_ext_args[5]; \
        *(rpc_ext_args+0) = (uint32_t) pfn_arg3; \
        *(rpc_ext_args+1) = (uint32_t) res_cb; \
        *(rpc_ext_args+2) = (uint32_t) CO_RPC_GET_ARG(a1); \
        *(rpc_ext_args+3) = (uint32_t) CO_RPC_GET_ARG(a2); \
        *(rpc_ext_args+4) = (uint32_t) CO_RPC_GET_ARG(a3); \
        uint32_t _ret = __co_rpc_send(rpc_ext_args, ARRAY_SIZE(rpc_ext_args), false, true); \
        ASSERT(_ret == BT_STS_SUCCESS, "rpc send error, ret = %d", _ret); \
    }while(0)

/**
 * @brief CO RPC async with 4 arguments.
 *
 * @param[in]    res_cb      result callback(see @ref pfunc_tprc_cb_t), can be NULL if you don't care
 * @param[in]    ret_type    the return type of func(next argument)
 * @param[in]    func        rpc function to be called
 * @param[in]    a1          argument of the rpc function
 * @param[in]    a2          argument of the rpc function
 * @param[in]    a3          argument of the rpc function
 * @param[in]    a4          argument of the rpc
 *
 */
#define __CO_RPC_ARG4_ASYNC_RET_VOID(res_cb, func, a1, a2, a3, a4)    do{\
        ASSERT(sizeof(a1)<=sizeof(uint32_t), " argument a1 is too large!"); \
        ASSERT(sizeof(a2)<=sizeof(uint32_t), " argument a2 is too large!"); \
        ASSERT(sizeof(a3)<=sizeof(uint32_t), " argument a3 is too large!"); \
        ASSERT(sizeof(a4)<=sizeof(uint32_t), " argument a4 is too large!"); \
        __typeof__(void) (*pfn_arg4)(__typeof__(a1), __typeof__(a2), __typeof__(a3), __typeof__(a4))  = func; \
        uint32_t rpc_ext_args[6]; \
        *(rpc_ext_args+0) = (uint32_t) pfn_arg4; \
        *(rpc_ext_args+1) = (uint32_t) res_cb; \
        *(rpc_ext_args+2) = (uint32_t) CO_RPC_GET_ARG(a1); \
        *(rpc_ext_args+3) = (uint32_t) CO_RPC_GET_ARG(a2); \
        *(rpc_ext_args+4) = (uint32_t) CO_RPC_GET_ARG(a3); \
        *(rpc_ext_args+5) = (uint32_t) CO_RPC_GET_ARG(a4); \
        uint32_t _ret = __co_rpc_send(rpc_ext_args, ARRAY_SIZE(rpc_ext_args), false, true); \
        ASSERT(_ret == BT_STS_SUCCESS, "rpc send error, ret = %d", _ret); \
    }while(0)

/**
 * @brief CO RPC async with 5 arguments.
 *
 * @param[in]    res_cb      result callback(see @ref pfunc_tprc_cb_t), can be NULL if you don't care
 * @param[in]    func        rpc function to be called
 * @param[in]    a1          argument of the rpc function
 * @param[in]    a2          argument of the rpc function
 * @param[in]    a3          argument of the rpc function
 * @param[in]    a4          argument of the rpc function
 * @param[in]    a5          argument of the rpc
 *
 */
#define __CO_RPC_ARG5_ASYNC_RET_VOID(res_cb, func, a1, a2, a3, a4, a5)    do{\
        ASSERT(sizeof(a1)<=sizeof(uint32_t), " argument a1 is too large!"); \
        ASSERT(sizeof(a2)<=sizeof(uint32_t), " argument a2 is too large!"); \
        ASSERT(sizeof(a3)<=sizeof(uint32_t), " argument a3 is too large!"); \
        ASSERT(sizeof(a4)<=sizeof(uint32_t), " argument a4 is too large!"); \
        ASSERT(sizeof(a5)<=sizeof(uint32_t), " argument a5 is too large!"); \
        __typeof__(void) (*pfn_arg5)(__typeof__(a1), __typeof__(a2), __typeof__(a3), __typeof__(a4), \
                                         __typeof__(a5))  = func; \
        uint32_t rpc_ext_args[7]; \
        *(rpc_ext_args+0) = (uint32_t) pfn_arg5; \
        *(rpc_ext_args+1) = (uint32_t) res_cb; \
        *(rpc_ext_args+2) = (uint32_t) CO_RPC_GET_ARG(a1); \
        *(rpc_ext_args+3) = (uint32_t) CO_RPC_GET_ARG(a2); \
        *(rpc_ext_args+4) = (uint32_t) CO_RPC_GET_ARG(a3); \
        *(rpc_ext_args+5) = (uint32_t) CO_RPC_GET_ARG(a4); \
        *(rpc_ext_args+6) = (uint32_t) CO_RPC_GET_ARG(a5); \
        uint32_t _ret = __co_rpc_send(rpc_ext_args, ARRAY_SIZE(rpc_ext_args), false, true); \
        ASSERT(_ret == BT_STS_SUCCESS, "rpc send error, ret = %d", _ret); \
    }while(0)

/**
 * @brief CO RPC async with 6 arguments.
 *
 * @param[in]    res_cb      result callback(see @ref pfunc_tprc_cb_t), can be NULL if you don't care
 * @param[in]    func        rpc function to be called
 * @param[in]    a1          argument of the rpc function
 * @param[in]    a2          argument of the rpc function
 * @param[in]    a3          argument of the rpc function
 * @param[in]    a4          argument of the rpc function
 * @param[in]    a5          argument of the rpc function
 * @param[in]    a6          argument of the rpc
 *
 */
#define __CO_RPC_ARG6_ASYNC_RET_VOID(res_cb, func, a1, a2, a3, a4, a5, a6)    do{\
        ASSERT(sizeof(a1)<=sizeof(uint32_t), " argument a1 is too large!"); \
        ASSERT(sizeof(a2)<=sizeof(uint32_t), " argument a2 is too large!"); \
        ASSERT(sizeof(a3)<=sizeof(uint32_t), " argument a3 is too large!"); \
        ASSERT(sizeof(a4)<=sizeof(uint32_t), " argument a4 is too large!"); \
        ASSERT(sizeof(a5)<=sizeof(uint32_t), " argument a5 is too large!"); \
        ASSERT(sizeof(a6)<=sizeof(uint32_t), " argument a6 is too large!"); \
        __typeof__(void) (*pfn_arg6)(__typeof__(a1), __typeof__(a2), __typeof__(a3), __typeof__(a4), \
                                         __typeof__(a5), __typeof__(a6))  = func; \
        uint32_t rpc_ext_args[8]; \
        *(rpc_ext_args+0) = (uint32_t) pfn_arg6; \
        *(rpc_ext_args+1) = (uint32_t) res_cb; \
        *(rpc_ext_args+2) = (uint32_t) CO_RPC_GET_ARG(a1); \
        *(rpc_ext_args+3) = (uint32_t) CO_RPC_GET_ARG(a2); \
        *(rpc_ext_args+4) = (uint32_t) CO_RPC_GET_ARG(a3); \
        *(rpc_ext_args+5) = (uint32_t) CO_RPC_GET_ARG(a4); \
        *(rpc_ext_args+6) = (uint32_t) CO_RPC_GET_ARG(a5); \
        *(rpc_ext_args+7) = (uint32_t) CO_RPC_GET_ARG(a6); \
        uint32_t _ret = __co_rpc_send(rpc_ext_args, ARRAY_SIZE(rpc_ext_args), false, true); \
        ASSERT(_ret == BT_STS_SUCCESS, "rpc send error, ret = %d", _ret); \
    }while(0)

/***************************************************************************************************************/
/*                                              CO RPC API                                                     */
/***************************************************************************************************************/
/**
 * @brief  RPC call with 0 argument by sync method with no return
 *
 * @param[in] func     rpc function to be called
 *
 */
#define CO_RPC_SYNC_0ARG_RET_VOID(func)    do {\
        if (bt_callback_is_bt_thread()) {\
            func();\
        } else {\
            __CO_RPC_ARG0_SYNC_RET_VOID(func);\
        }\
    }while(0)

/**
 * @brief  RPC call with 1 argument by sync method with no return
 *
 * @param[in] func     rpc function to be called
 * @param[in] a1       Arguements 1
 *
 */
#define CO_RPC_SYNC_1ARG_RET_VOID(func, a1)    do {\
        if (bt_callback_is_bt_thread()) {\
            func(a1);\
        } else {\
            __CO_RPC_ARG1_SYNC_RET_VOID(func, a1);\
        }\
    }while(0)

/**
 * @brief  RPC call with 2 arguments by sync method with no return
 *
 * @param[in] func     rpc function to be called
 * @param[in] a1       Arguements 1
 * @param[in] a2       Arguements 2
 *
 */
#define CO_RPC_SYNC_2ARG_RET_VOID(func, a1, a2)   do {\
        if (bt_callback_is_bt_thread()) {\
            func(a1, a2);\
        } else {\
            __CO_RPC_ARG2_SYNC_RET_VOID(func, a1, a2);\
        }\
    }while(0)

/**
 * @brief  RPC call with 3 arguments by sync method with no return
 *
 * @param[in] func     rpc function to be called
 * @param[in] a1       Arguements 1
 * @param[in] a2       Arguements 2
 * @param[in] a3       Arguements 3
 *
 */
#define CO_RPC_SYNC_3ARG_RET_VOID(func, a1, a2, a3)   do {\
        if (bt_callback_is_bt_thread()) {\
            func(a1, a2, a3);\
        } else {\
            __CO_RPC_ARG3_SYNC_RET_VOID(func, a1, a2, a3);\
        }\
    }while(0)

/**
 * @brief  RPC call with 4 arguments by sync method with no return
 *
 * @param[in] func     rpc function to be called
 * @param[in] a1       Arguements 1
 * @param[in] a2       Arguements 2
 * @param[in] a3       Arguements 3
 * @param[in] a4       Arguements 4
 *
 */
#define CO_RPC_SYNC_4ARG_RET_VOID(func, a1, a2, a3, a4)   do {\
        if (bt_callback_is_bt_thread()) {\
            func(a1, a2, a3, a4);\
        } else {\
            __CO_RPC_ARG4_SYNC_RET_VOID(func, a1, a2, a3, a4);\
        }\
    }while(0)

/**
 * @brief  RPC call with 5 arguments by sync method with no return
 *
 * @param[in] func     rpc function to be called
 * @param[in] a1       Arguements 1
 * @param[in] a2       Arguements 2
 * @param[in] a3       Arguements 3
 * @param[in] a4       Arguements 4
 * @param[in] a5       Arguements 5
 *
 */
#define CO_RPC_SYNC_5ARG_RET_VOID(func, a1, a2, a3, a4, a5)   do {\
        if (bt_callback_is_bt_thread()) {\
            func(a1, a2, a3, a4, a5);\
        } else {\
            __CO_RPC_ARG5_SYNC_RET_VOID(func, a1, a2, a3, a4, a5);\
        }\
    }while(0)

/**
 * @brief  RPC call with 6 arguments by sync method with no return
 *
 * @param[in] func     rpc function to be called
 * @param[in] a1       Arguements 1
 * @param[in] a2       Arguements 2
 * @param[in] a3       Arguements 3
 * @param[in] a4       Arguements 4
 * @param[in] a5       Arguements 5
 * @param[in] a6       Arguements 6
 *
 */
#define CO_RPC_SYNC_6ARG_RET_VOID(func, a1, a2, a3, a4, a5, a6)   do {\
        if (bt_callback_is_bt_thread()) {\
            func(a1, a2, a3, a4, a5, a6);\
        } else {\
            __CO_RPC_ARG6_SYNC_RET_VOID(func, a1, a2, a3, a4, a5, a6);\
        }\
    }while(0)

/**
 * @brief  RPC call with 0 argument by sync method with return
 *
 * @param[in] ret_type Type of return value
 * @param[in] ret_val  Return value to store the ret
 * @param[in] func     rpc function to be called
 *
 */
#define CO_RPC_SYNC_0ARG(ret_type, ret_val, func)    do {\
        if (bt_callback_is_bt_thread()) {\
            ret_val = func();\
        } else {\
            ret_val = __CO_RPC_ARG0_SYNC_WITH_RET(ret_type, func);\
        }\
    }while(0)

/**
 * @brief  RPC call with 1 argument by sync method with return
 *
 * @param[in] ret_type Type of return value
 * @param[in] ret_val  Return value to store the ret
 * @param[in] func     rpc function to be called
 * @param[in] a1       Arguements 1
 *
 */
#define CO_RPC_SYNC_1ARG(ret_type, ret_val, func, a1)    do {\
        if (bt_callback_is_bt_thread()) {\
            ret_val = func(a1);\
        } else {\
            ret_val = __CO_RPC_ARG1_SYNC_WITH_RET(ret_type, func, a1);\
        }\
    }while(0)

/**
 * @brief  RPC call with 2 arguments by sync method with return
 *
 * @param[in] ret_type Type of return value
 * @param[in] ret_val  Return value to store the ret
 * @param[in] func     rpc function to be called
 * @param[in] a1       Arguements 1
 * @param[in] a2       Arguements 2
 *
 */
#define CO_RPC_SYNC_2ARG(ret_type, ret_val, func, a1, a2)   do {\
        if (bt_callback_is_bt_thread()) {\
            ret_val = func(a1, a2);\
        } else {\
            ret_val = __CO_RPC_ARG2_SYNC_WITH_RET(ret_type, func, a1, a2);\
        }\
    }while(0)

/**
 * @brief  RPC call with 3 arguments by sync method with return
 *
 * @param[in] ret_type Type of return value
 * @param[in] ret_val  Return value to store the ret
 * @param[in] func     rpc function to be called
 * @param[in] a1       Arguements 1
 * @param[in] a2       Arguements 2
 * @param[in] a3       Arguements 3
 *
 */
#define CO_RPC_SYNC_3ARG(ret_type, ret_val, func, a1, a2, a3)   do {\
        if (bt_callback_is_bt_thread()) {\
            ret_val = func(a1, a2, a3);\
        } else {\
            ret_val = __CO_RPC_ARG3_SYNC_WITH_RET(ret_type, func, a1, a2, a3);\
        }\
    }while(0)

/**
 * @brief  RPC call with 4 arguments by sync method with return
 *
 * @param[in] ret_type Type of return value
 * @param[in] ret_val  Return value to store the ret
 * @param[in] func     rpc function to be called
 * @param[in] a1       Arguements 1
 * @param[in] a2       Arguements 2
 * @param[in] a3       Arguements 3
 * @param[in] a4       Arguements 4
 *
 */
#define CO_RPC_SYNC_4ARG(ret_type, ret_val, func, a1, a2, a3, a4)   do {\
        if (bt_callback_is_bt_thread()) {\
            ret_val = func(a1, a2, a3, a4);\
        } else {\
            ret_val = __CO_RPC_ARG4_SYNC_WITH_RET(ret_type, func, a1, a2, a3, a4);\
        }\
    }while(0)

/**
 * @brief  RPC call with 5 arguments by sync method with return
 *
 * @param[in] ret_type Type of return value
 * @param[in] ret_val  Return value to store the ret
 * @param[in] func     rpc function to be called
 * @param[in] a1       Arguements 1
 * @param[in] a2       Arguements 2
 * @param[in] a3       Arguements 3
 * @param[in] a4       Arguements 4
 * @param[in] a5       Arguements 5
 *
 */
#define CO_RPC_SYNC_5ARG(ret_type, ret_val, func, a1, a2, a3, a4, a5)   do {\
        if (bt_callback_is_bt_thread()) {\
            ret_val = func(a1, a2, a3, a4, a5);\
        } else {\
            ret_val = __CO_RPC_ARG5_SYNC_WITH_RET(ret_type, func, a1, a2, a3, a4, a5);\
        }\
    }while(0)

/**
 * @brief  RPC call with 6 arguments by sync method with return
 *
 * @param[in] ret_type Type of return value
 * @param[in] ret_val  Return value to store the ret
 * @param[in] func     rpc function to be called
 * @param[in] a1       Arguements 1
 * @param[in] a2       Arguements 2
 * @param[in] a3       Arguements 3
 * @param[in] a4       Arguements 4
 * @param[in] a5       Arguements 5
 * @param[in] a6       Arguements 6
 *
 */
#define CO_RPC_SYNC_6ARG(ret_type, ret_val, func, a1, a2, a3, a4, a5, a6)   do {\
        if (bt_callback_is_bt_thread()) {\
            ret_val = func(a1, a2, a3, a4, a5, a6);\
        } else {\
            ret_val = __CO_RPC_ARG6_SYNC_WITH_RET(ret_type, func, a1, a2, a3, a4, a5, a6);\
        }\
    }while(0)

/**
 * @brief  RPC call with 0 arguments by async method with no return
 *
 * @param[in] func     rpc function to be called
 *
 */
#define CO_RPC_ASYNC_0ARG_RET_VOID(func)    do {\
            __CO_RPC_ARG0_ASYNC_RET_VOID(NULL, func);\
    }while(0)

/**
 * @brief  RPC call with 1 arguments by async method with no return
 *
 * @param[in] func     rpc function to be called
 * @param[in] a1       Arguement 1
 *
 */
#define CO_RPC_ASYNC_1ARG_RET_VOID(func, a1)   do {\
            __CO_RPC_ARG1_ASYNC_RET_VOID(NULL, func, a1);\
    }while(0)

/**
 * @brief  RPC call with 2 arguments by async method with no return
 *
 * @param[in] func     rpc function to be called
 * @param[in] a1       Arguements 1
 * @param[in] a2       Arguements 2
 *
 */
#define CO_RPC_ASYNC_2ARG_RET_VOID(func, a1, a2)   do {\
            __CO_RPC_ARG2_ASYNC_RET_VOID(NULL, func, a1, a2);\
    }while(0)

/**
 * @brief  RPC call with 3 arguments by async method with no return
 *
 * @param[in] func     rpc function to be called
 * @param[in] a1       Arguements 1
 * @param[in] a2       Arguements 2
 * @param[in] a3       Arguements 3
 *
 */
#define CO_RPC_ASYNC_3ARG_RET_VOID(func, a1, a2, a3)   do {\
            __CO_RPC_ARG3_ASYNC_RET_VOID(NULL, func, a1, a2, a3);\
    }while(0)

/**
 * @brief  RPC call with 4 arguments by async method with no return
 *
 * @param[in] func     rpc function to be called
 * @param[in] a1       Arguements 1
 * @param[in] a2       Arguements 2
 * @param[in] a3       Arguements 3
 * @param[in] a4       Arguements 4
 *
 */
#define CO_RPC_ASYNC_4ARG_RET_VOID(func, a1, a2, a3, a4)   do {\
            __CO_RPC_ARG4_ASYNC_RET_VOID(NULL, func, a1, a2, a3, a4);\
    }while(0)

/**
 * @brief  RPC call with 5 arguments by async method with no return
 *
 * @param[in] func     rpc function to be called
 * @param[in] a1       Arguements 1
 * @param[in] a2       Arguements 2
 * @param[in] a3       Arguements 3
 * @param[in] a4       Arguements 4
 * @param[in] a5       Arguements 5
 *
 */
#define CO_RPC_ASYNC_5ARG_RET_VOID(func, a1, a2, a3, a4, a5)   do {\
            __CO_RPC_ARG5_ASYNC_RET_VOID(NULL, func, a1, a2, a3, a4, a5);\
    }while(0)

/**
 * @brief  RPC call with 6 arguments by async method with no return
 *
 * @param[in] func     rpc function to be called
 * @param[in] a1       Arguements 1
 * @param[in] a2       Arguements 2
 * @param[in] a3       Arguements 3
 * @param[in] a4       Arguements 4
 * @param[in] a5       Arguements 5
 * @param[in] a6       Arguements 6
 *
 */
#define CO_RPC_ASYNC_6ARG_RET_VOID(func, a1, a2, a3, a4, a5, a6)   do {\
            __CO_RPC_ARG6_ASYNC_RET_VOID(NULL, func, a1, a2, a3, a4, a5, a6);\
    }while(0)

/**
 * @brief  RPC call with 0 argument by async method with return
 *
 * @param[in] ret_type Type of return value
 * @param[in] func     rpc function to be called
 *
 */
#define CO_RPC_ASYNC_0ARG(ret_type, func)   do {\
            __CO_RPC_ARG0_ASYNC_WITH_RET(NULL, ret_type, func);\
    }while (0)

/**
 * @brief  RPC call with 1 argument by async method with return
 *
 * @param[in] ret_type Type of return value
 * @param[in] func     rpc function to be called
 * @param[in] a1       Arguement 1
 *
 */
#define CO_RPC_ASYNC_1ARG(ret_type, func, a1)   do {\
            __CO_RPC_ARG1_ASYNC_WITH_RET(NULL, ret_type, func, a1);\
    }while (0)

/**
 * @brief  RPC call with 2 arguments by async method with return
 *
 * @param[in] ret_type Type of return value
 * @param[in] func     rpc function to be called
 * @param[in] a1       Arguements 1
 * @param[in] a2       Arguements 2
 *
 */
#define CO_RPC_ASYNC_2ARG(ret_type, func, a1, a2)   do {\
            __CO_RPC_ARG2_ASYNC_WITH_RET(NULL, ret_type, func, a1, a2);\
    }while (0)

/**
 * @brief  RPC call with 3 arguments by async method with return
 *
 * @param[in] ret_type Type of return value
 * @param[in] func     rpc function to be called
 * @param[in] a1       Arguements 1
 * @param[in] a2       Arguements 2
 * @param[in] a3       Arguements 3
 *
 */
#define CO_RPC_ASYNC_3ARG(ret_type, func, a1, a2, a3)   do {\
            __CO_RPC_ARG3_ASYNC_WITH_RET(NULL, ret_type, func, a1, a2, a3);\
    }while (0)

/**
 * @brief  RPC call with 4 arguments by async method with return
 *
 * @param[in] ret_type Type of return value
 * @param[in] func     rpc function to be called
 * @param[in] a1       Arguements 1
 * @param[in] a2       Arguements 2
 * @param[in] a3       Arguements 3
 * @param[in] a4       Arguements 4
 *
 */
#define CO_RPC_ASYNC_4ARG(ret_type, func, a1, a2, a3, a4)   do {\
            __CO_RPC_ARG4_ASYNC_WITH_RET(NULL, ret_type, func, a1, a2, a3, a4);\
    }while (0)

/**
 * @brief  RPC call with 5 arguments by async method with return
 *
 * @param[in] ret_type Type of return value
 * @param[in] func     rpc function to be called
 * @param[in] a1       Arguements 1
 * @param[in] a2       Arguements 2
 * @param[in] a3       Arguements 3
 * @param[in] a4       Arguements 4
 * @param[in] a5       Arguements 5
 *
 */
#define CO_RPC_ASYNC_5ARG(ret_type, func, a1, a2, a3, a4, a5)   do {\
            __CO_RPC_ARG5_ASYNC_WITH_RET(NULL, ret_type, func, a1, a2, a3, a4, a5);\
    }while (0)

/**
 * @brief  RPC call with 6 arguments by async method with return
 *
 * @param[in] ret_type Type of return value
 * @param[in] func     rpc function to be called
 * @param[in] a1       Arguements 1
 * @param[in] a2       Arguements 2
 * @param[in] a3       Arguements 3
 * @param[in] a4       Arguements 4
 * @param[in] a5       Arguements 5
 * @param[in] a6       Arguements 6
 *
 */
#define CO_RPC_ASYNC_6ARG(ret_type, func, a1, a2, a3, a4, a5, a6)   do {\
            __CO_RPC_ARG6_ASYNC_WITH_RET(NULL, ret_type, func, a1, a2, a3, a4, a5, a6);\
    }while(0)

/**
 * @brief  CO RPC Send api to start a function using rpc call by sync or async method
 *
 * @param[in] args     Input arguments with max type size <= sizeof(uint32_t) per arg
 * @param[in] arg_num  Number of arguements
 * @param[in] ret_exist
 *                     Return value exist or return void
 * @param[in] async    RPC call by Sync or Async call method
 *
 * @return int         RPC send status, 0 == OK, others Failed
 */
#define __co_rpc_send(args, arg_num, ret_exist, async) \
            __co_rpc_send_with_ca(args, (ret_exist | (async << 8) | (arg_num << 16)),\
                                  (uint32_t)(uintptr_t)__builtin_return_address(0), __LINE__)

int __co_rpc_send_with_ca(uint32_t *args, uint32_t ret_exist_async_arg_num, uint32_t ca, uint32_t ln);

#ifdef __cplusplus
}
#endif

#endif  /* __CO_RPC_H__ */