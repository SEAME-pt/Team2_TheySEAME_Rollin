/*********************************************************************
*                    SEGGER Microcontroller GmbH                     *
*                        The Embedded Experts                        *
**********************************************************************
*                                                                    *
*            (c) 1995 - 2024 SEGGER Microcontroller GmbH             *
*                                                                    *
*       www.segger.com     Support: support@segger.com               *
*                                                                    *
**********************************************************************

File    : SEGGER_SYSVIEW_Conf.h
Purpose : SEGGER SystemView configuration for STM32U585 with ThreadX
*/

#ifndef SEGGER_SYSVIEW_CONF_H
#define SEGGER_SYSVIEW_CONF_H

#include "stm32u5xx.h"  // For __get_IPSR() and CMSIS core functions

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/

// SystemView is single-core, so we only need CPU 0
#define SEGGER_SYSVIEW_GET_TIMESTAMP()      SEGGER_SYSVIEW_X_GetTimestamp()  // Retrieve timestamp for System View events
#define SEGGER_SYSVIEW_GET_INTERRUPT_ID()   ((U32)(__get_IPSR()))           // Get the currently active interrupt Id from IPSR

// Use fixed RTT channel 1 for SystemView data (channel 0 = terminal)
#define SEGGER_SYSVIEW_RTT_CHANNEL          1

// Post-mortem mode: target writes continuously to a large circular buffer.
// We halt + dump whenever we want to analyze. No data loss from slow RTT reads.
#define SEGGER_SYSVIEW_POST_MORTEM_MODE     1

// Large RTT buffer for post-mortem (16 KB out of ~256 KB SRAM available)
#define SEGGER_SYSVIEW_RTT_BUFFER_SIZE      (16 * 1024)

// Core clock frequency in Hz
#define SEGGER_SYSVIEW_CPU_FREQ             (160000000)                     // STM32U585 running at 160 MHz

// Frequency of the timestamp counter
#define SEGGER_SYSVIEW_TIMESTAMP_FREQ       (SEGGER_SYSVIEW_CPU_FREQ)       // Use CPU freq for timestamp

// System Frequency. SystemCoreClock is used in most CMSIS compatible projects.
#define SEGGER_SYSVIEW_SYSDESC0             "I#15=SysTick"                  // IRQ 15 is SysTick

// Define number of valid bits low-order delivered by clock source (1-32)
#define SEGGER_SYSVIEW_TIMESTAMP_BITS       (32)                            // Full 32-bit DWT counter

// API functions to read system information
void SEGGER_SYSVIEW_Conf(void);
U32 SEGGER_SYSVIEW_X_GetTimestamp(void);
U32 SEGGER_SYSVIEW_X_GetInterruptId(void);

#endif  // SEGGER_SYSVIEW_CONF_H
