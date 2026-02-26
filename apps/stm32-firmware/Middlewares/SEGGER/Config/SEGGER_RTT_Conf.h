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

File    : SEGGER_RTT_Conf.h
Purpose : Implementation-dependent configuration of SEGGER RTT
*/

#ifndef SEGGER_RTT_CONF_H
#define SEGGER_RTT_CONF_H

#ifdef __IAR_SYSTEMS_ICC__
  #include <intrinsics.h>
#endif

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/

#define SEGGER_RTT_MAX_NUM_UP_BUFFERS    (3)     // Max. number of up-buffers (T->H) available on this target
#define SEGGER_RTT_MAX_NUM_DOWN_BUFFERS  (3)     // Max. number of down-buffers (H->T) available on this target

#define BUFFER_SIZE_UP                   (4096)  // Size of the buffer for terminal output of target, up to host
#define BUFFER_SIZE_DOWN                 (16)    // Size of the buffer for terminal input to target from host (Usually keyboard input)

#define SEGGER_RTT_PRINTF_BUFFER_SIZE    (64u)   // Size of buffer for RTT printf to bulk-send chars via RTT

//
// Target is not allowed to perform other RTT operations while string still has not been stored completely.
// Otherwise we would probably end up with a mixed string in the buffer.
// If using  RTT from within interrupts, multiple tasks or multi processors, define the SEGGER_RTT_LOCK() and SEGGER_RTT_UNLOCK() function here.
//
#define SEGGER_RTT_LOCK()     { unsigned int _lockState; __asm volatile("MRS %0, PRIMASK\n CPSID i" : "=r"(_lockState) :: "memory");
#define SEGGER_RTT_UNLOCK()     __asm volatile("MSR PRIMASK, %0" :: "r"(_lockState) : "memory"); }

#endif
