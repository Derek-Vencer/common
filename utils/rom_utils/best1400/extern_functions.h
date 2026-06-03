#ifndef _EXTERN_FUNCTIONS_
#define _EXTERN_FUNCTIONS_

#include "plat_types.h"
#include "hal_trace.h"
#include "hal_dma.h"
#include "hal_timer.h"
#include "hal_cmu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "cmsis_os.h"
#include "hal_uart.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_chipid.h"
#include "hal_trace.h"
#include "hal_sleep.h"

// fft
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdbool.h>

typedef struct
{
    unsigned long long int quot; /* quotient */
    unsigned long long int rem; /* remainder */
} ulldiv_t;

// var related
#if defined(__cplusplus)
extern "C" {
#endif
void rom1_entry(struct __extern_functions__ *efp);
void set__impure_ptr(void *p);
ulldiv_t __aeabi_uldivmod(unsigned long long n, unsigned long long d);
#if defined(__cplusplus)
}
#endif


#if 0
typedef int (*T_hwfft_create)(struct hwfft_t *fft_inst, int mode, int order,int sym_fft);
typedef int (*T_hwfft_destroy)(struct hwfft_t *fft_inst);
typedef int (*T_hwfft_fft_process)(struct hwfft_t *fft_inst, short *in, short *out);
#endif

typedef int (*T_main)(int argc, char *argv[]);

typedef uint32_t (*T_hal_sys_timer_get)(void);

typedef void (*T_OS_MemSet)(unsigned char *dest, unsigned char byte, unsigned int len);
typedef uint16_t (*T_OS_Rand)(void);
typedef int (*T_hal_trace_crash_dump_register)(HAL_TRACE_CRASH_DUMP_CB_T cb);
typedef void NORETURN (*T_hal_trace_assert_dump_fix_arg)(const char *str);
typedef int (*T_hal_trace_printf_without_crlf_fix_arg)(const char *str);
typedef int (*T_hal_trace_printf_imm_fix_arg)(const char *str);
typedef int (*T_vsprintf)(char *buf, const char *fmt, va_list ap);
typedef void * (*T_memcpy)(void *dst,const void *src, size_t length);
typedef void * (*T_memmove)(void *dest, const void *src, size_t n);
typedef int (*T_hal_trace_output)(const unsigned char *buf, unsigned int buf_len);
typedef int (*T_hal_trace_dump)(const char *fmt, unsigned int size,  unsigned int count, const void *buffer);
typedef void (*T_speex_echo_get_residual)(void *st, void *residual_echo, int len);
typedef double (*T_pow)(double x, double y);
typedef double (*T_sqrt)(double x);
typedef void (*T_exit)(int status);
typedef void *(*T_memset)(void *s, int c, size_t n);
typedef double (*T__aeabi_i2d)(int Value);
typedef float (*T__aeabi_d2f)(double Value);
typedef double (*T__aeabi_f2d)(float Value);
typedef int (*T__aeabi_d2iz)(double);
typedef double (*T__aeabi_dmul)(double, double);
typedef unsigned int (*T__aeabi_d2uiz)(double Value);
typedef lldiv_t (*T__aeabi_ldivmod)(long long n, long long d);
typedef ulldiv_t (*T__aeabi_uldivmod)(unsigned long long n, unsigned long long d);
typedef double (*T__aeabi_dadd)(double n, double d);
typedef double (*T_exp)(double x);
typedef double (*T_log)(double x);
typedef double (*T_floor)(double x);
typedef float (*T_powf)(float x, float y);
typedef int (*T_log10_20fix)(int in);
typedef short (*T_sqrt_fix)(int x);
typedef void *(*T_malloc)(size_t size);
typedef void (*T_free)(void *ptr);
typedef double (*T_cos)(double x);
typedef double (*T_sin)(double x);
typedef int (*T_rand)(void);
typedef int (*T_abs)(int j);

typedef int (*T_exp_fix)(short x);
typedef int (*T_ExpIntfix)(short in);

typedef int (*T_hal_cpu_wake_unlock)(enum HAL_CPU_WAKE_LOCK_USER_T user);
typedef int (*T_hal_cpu_wake_lock)(enum HAL_CPU_WAKE_LOCK_USER_T user);



struct __extern_functions__
{
    T_hal_trace_assert_dump_fix_arg hal_trace_assert_dump_fix_arg;
    T_vsprintf vsprintf;
    T_hal_trace_crash_dump_register hal_trace_crash_dump_register;
    T_hal_trace_printf_without_crlf_fix_arg hal_trace_printf_without_crlf_fix_arg;
    T_hal_trace_printf_imm_fix_arg hal_trace_printf_imm_fix_arg;
    T_hal_trace_output hal_trace_output;
    T_hal_trace_dump hal_trace_dump;

    T_hal_sys_timer_get hal_sys_timer_get;
    T_hal_cpu_wake_lock hal_cpu_wake_lock;
    T_hal_cpu_wake_unlock hal_cpu_wake_unlock;

    T_OS_MemSet OS_MemSet;
#if 0
    T_OS_Rand OS_Rand;
#endif
    T_memset memset;
    T_memcpy memcpy;
#if 0
    T_memmove memmove;
#endif

#if 0
    T__aeabi_i2d __aeabi_i2d;
    T__aeabi_d2f __aeabi_d2f;
    T__aeabi_f2d __aeabi_f2d;
    T__aeabi_d2iz __aeabi_d2iz;
    T__aeabi_dmul __aeabi_dmul;
#endif
#if 0
    T__aeabi_ldivmod __aeabi_ldivmod;
#endif
    T__aeabi_uldivmod __aeabi_uldivmod;
#if 0
    T__aeabi_dadd __aeabi_dadd;
    T__aeabi_d2uiz __aeabi_d2uiz;
#endif

#if 0
    T_exp exp;
    T_log log;
    T_floor floor;
    T_pow pow;
    T_sqrt sqrt;
    T_exit exit;
    T_powf powf;
#endif
};


#endif /* _EXTERN_FUNCTIONS_ */
