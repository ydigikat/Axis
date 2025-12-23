/*
  ------------------------------------------------------------------------------
   SynthCoreF4
   Author: ydigikat
  ------------------------------------------------------------------------------
   MIT License
   Copyright (c) 2025 YDigiKat

   Permission to use, copy, modify, and/or distribute this code for any purpose
   with or without fee is hereby granted, provided the above copyright notice and
   this permission notice appear in all copies.
  ------------------------------------------------------------------------------
*/
#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <stdio.h>
#include "stm32f411xe.h"

#ifdef RTT_ENABLED

#include "SEGGER_RTT.h"

#ifdef DEBUG

/** 
 * \brief Assertion macro 
 * \note  Only active when both RTT and DEBUG are enabled
 * 
 *        Example: RTT_ASSERT(buffer != NULL);  
 *
 *        If the check fails, it prints the file, function and line number where it happened
 *        to RTT then hangs the system (infinite loop) since continuing would be dangerous
 */
#define RTT_ASSERT(expr)                                                                                        \
  do                                                                                                            \
  {                                                                                                             \
    if (!(expr))                                                                                                \
    {                                                                                                           \
      SEGGER_RTT_printf(0, "ASSERTION FAILED: %s, function: %s, line: %d\n", __FILE__, __FUNCTION__, __LINE__); \
      while (1)                                                                                                 \
        ;                                                                                                       \
    }                                                                                                           \
  } while (0)
#else
/* set to nothing in Release modes */
#define RTT_ASSERT(expr) ((void)0)
#endif /* DEBUG */

/* Following macros need RTT and work for both DEBUG/RELEASE presets */

/**  
 * \brief Use this when you need to print floating point values
 * \note Regular RTT_LOG can't handle floats directly
 *       This is slow because it uses snprintf to format the string first
 */
#define RTT_LOG_FLOAT(fmt, ...)               \
  do                                          \
  {                                           \
    char buffer[96];                          \
    snprintf(buffer, 96, fmt, ##__VA_ARGS__); \
    SEGGER_RTT_printf(0, "%s", buffer);       \
  } while (0)

/** 
 * \brief The main debug print function 
 * \note Use this for most debug messages when you don't need floating point
 */
#define RTT_LOG(fmt, ...) SEGGER_RTT_printf(0, fmt, ##__VA_ARGS__)

/** 
 * \brief DWT cycle counter setup - used for timing measurements
 * \note This initializes the ARM Cortex debug hardware for cycle counting
 *       Put this at the beginning of a function where you want to measure performance
 */
#define DWT_INIT()                                \
  uint32_t dwt_start, dwt_end, dwt_cycles;        \
  uint32_t dwt_time_us;                           \
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; \
  DWT->CYCCNT = 0;                                \
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

/** 
 * \brief Resets and starts the cycle counter
 * \note Call this right before the code you want to measure
 */
#define DWT_CLEAR() \
  DWT->CYCCNT = 0;  \
  dwt_start = DWT->CYCCNT;

/*
 * Conversion factor from CPU cycles to microseconds (100MHz clock) 
 * The fixed-point scaling gives some precision without floating point
 */
#define DWT_SCALE_FIXED ((1000000LL << 16) / 100000000) 

/** 
 * \brief Outputs the timing measurement to RTT
 * \note Call this right after the code you want to measure
 *       Shows both CPU cycles and approximate time in microseconds
 * 
 */
#define DWT_OUTPUT(msg)                               \
  dwt_end = DWT->CYCCNT;                              \
  dwt_cycles = dwt_end - dwt_start;                   \
  dwt_time_us = (dwt_cycles * DWT_SCALE_FIXED) >> 16; \
  RTT_LOG("%s # %s : %lu cycles (~%lu us)\n", RTT_CTRL_TEXT_BRIGHT_CYAN, msg, dwt_cycles, dwt_time_us);

#else
/*
 * When RTT is disabled, all debug macros become empty
 */
#define RTT_LOG(...)
#define RTT_LOG_FLOAT(fmt, ...)
#define RTT_ASSERT(expr)
#define DWT_INIT()
#define DWT_CLEAR()
#define DWT_OUTPUT(function)

#endif /* RTT_ENABLED */

#endif /* __DEBUG_H__ */