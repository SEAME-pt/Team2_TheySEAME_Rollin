// SEGGER SystemView Configuration for STM32U585 + ThreadX
// Copyright (c) 2024 SEGGER Microcontroller GmbH

#include "SEGGER_SYSVIEW.h"
#include "SEGGER_SYSVIEW_Conf.h"
#include "stm32u5xx.h"

#define SYSVIEW_DEVICE_NAME     "Cortex-M33"
#define SYSVIEW_RAM_BASE        (0x20000000)

/* Provided by SEGGER_SYSVIEW_ThreadX.c */
extern void sysview_send_task_list(void);

static void _cbSendSystemDesc(void) {
  SEGGER_SYSVIEW_SendSysDesc("N="SYSVIEW_DEVICE_NAME",D=STM32U585,O=ThreadX");
  SEGGER_SYSVIEW_SendSysDesc("I#15=SysTick");
  /* Re-send thread list when SystemView (re)connects */
  sysview_send_task_list();
}

// Initialize SystemView - call once during startu p
void SEGGER_SYSVIEW_Conf(void) {
  // Enable DWT cycle counter for precise timestamps
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
  
  SEGGER_SYSVIEW_Init(SEGGER_SYSVIEW_TIMESTAMP_FREQ, 
                      SEGGER_SYSVIEW_CPU_FREQ,
                      0,
                      _cbSendSystemDesc);
  
  SEGGER_SYSVIEW_SetRAMBase(SYSVIEW_RAM_BASE);
  
  // Auto-start recording so data streams immediately via RTT.
  // Without this, SystemView waits for a "start" command from the host
  // which only works over J-Link, not over the OpenOCD RTT bridge.
  SEGGER_SYSVIEW_Start();
}

// Return current timestamp in CPU cycles
U32 SEGGER_SYSVIEW_X_GetTimestamp(void) {
  return DWT->CYCCNT;
}

// Return currently active interrupt ID
U32 SEGGER_SYSVIEW_X_GetInterruptId(void) {
  return ((*(U32*)(0xE000ED04)) & 0x1FF);
}
