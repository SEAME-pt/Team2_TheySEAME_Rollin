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

File    : SEGGER_SYSVIEW_Config_ThreadX.c
Purpose : Real-time recording configuration file for ThreadX on STM32U585
*/

#include "SEGGER_SYSVIEW.h"
#include "SEGGER_SYSVIEW_Conf.h"
#include "stm32u5xx.h"

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

// SystemView is single-core, ID is always 0 for single-core
#define SYSVIEW_DEVICE_NAME     "Cortex-M33"
#define SYSVIEW_RAM_BASE        (0x20000000)

/*********************************************************************
*
*       _cbSendSystemDesc()
*
*  Function description
*    Sends SystemView description strings.
*/
static void _cbSendSystemDesc(void) {
  SEGGER_SYSVIEW_SendSysDesc("N="SYSVIEW_DEVICE_NAME",D=STM32U585,O=ThreadX");
  SEGGER_SYSVIEW_SendSysDesc("I#15=SysTick");
}

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/

/*********************************************************************
*
*       SEGGER_SYSVIEW_Conf()
*
*  Function description
*    Configure and initialize SystemView and register it with RTOS.
*
*  Additional information
*    Call this function once during initialization.
*/
void SEGGER_SYSVIEW_Conf(void) {
  // Enable DWT cycle counter for precise timestamps
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;  // Enable trace
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;             // Enable cycle counter
  
  SEGGER_SYSVIEW_Init(SEGGER_SYSVIEW_TIMESTAMP_FREQ, 
                      SEGGER_SYSVIEW_CPU_FREQ,
                      0,  // Not used for Cortex-M
                      _cbSendSystemDesc);
  
  SEGGER_SYSVIEW_SetRAMBase(SYSVIEW_RAM_BASE);
}

/*********************************************************************
*
*       SEGGER_SYSVIEW_X_GetTimestamp()
*
*  Function description
*    Returns the current timestamp in CPU cycles.
*
*  Return value
*    Current timestamp (CPU cycles).
*/
U32 SEGGER_SYSVIEW_X_GetTimestamp(void) {
  return DWT->CYCCNT;
}

/*********************************************************************
*
*       SEGGER_SYSVIEW_X_GetInterruptId()
*
*  Function description
*    Returns the currently active interrupt.
*
*  Return value
*    Interrupt ID.
*/
U32 SEGGER_SYSVIEW_X_GetInterruptId(void) {
  return ((*(U32*)(0xE000ED04)) & 0x1FF);  // Read ICSR[8:0] = active vector
}

/*************************** End of file ****************************/
