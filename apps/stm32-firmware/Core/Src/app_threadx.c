/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_threadx.c
  * @author  MCD Application Team
  * @brief   ThreadX applicative file
  ******************************************************************************
    * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "app_threadx.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "Sensors/sensors.h"
#include "Communication/comm.h"
#include "Control/control.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
TX_THREAD battery_thread;
UCHAR battery_thread_stack[2048];
extern void Battery_Thread_Entry(ULONG thread_input);

TX_THREAD communication_thread;
UCHAR communication_thread_stack[2048];
extern void Communication_Thread_Entry(ULONG thread_input);

TX_THREAD control_thread;
UCHAR control_thread_stack[2048];
extern void Control_Thread_Entry(ULONG thread_input);

/* Global vehicle data and mutex */
VehicleData_t g_vehicle_data;
TX_MUTEX g_vehicle_data_mutex;

/* Global vehicle command and mutex */
VehicleCommand_t g_vehicle_command;
TX_MUTEX g_vehicle_command_mutex;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/**
  * @brief  Application ThreadX Initialization.
  * @param memory_ptr: memory pointer
  * @retval int
  */
UINT App_ThreadX_Init(VOID *memory_ptr)
{
  UINT ret = TX_SUCCESS;

  /* USER CODE BEGIN App_ThreadX_MEM_POOL */

  /* USER CODE END App_ThreadX_MEM_POOL */

  /* USER CODE BEGIN App_ThreadX_Init */
  // Initialize global command structure
  g_vehicle_command.driving_mode = 0;
  g_vehicle_command.throttle = 0;
  g_vehicle_command.steering_angle = 0;
  g_vehicle_command.command_valid = 0;
  
  // Create mutex for protecting global vehicle data
  UINT status = tx_mutex_create(&g_vehicle_data_mutex, "VehicleData Mutex", TX_NO_INHERIT);
  if (status != TX_SUCCESS) {
      return TX_MUTEX_ERROR;
  }
  
  // Create mutex for protecting global vehicle command
  status = tx_mutex_create(&g_vehicle_command_mutex, "VehicleCommand Mutex", TX_NO_INHERIT);
  if (status != TX_SUCCESS) {
      return TX_MUTEX_ERROR;
  }

  // Create the battery monitoring thread
  status = tx_thread_create(&battery_thread, "Battery Thread",
                            Battery_Thread_Entry, 0,
                            battery_thread_stack, sizeof(battery_thread_stack),
                            10, 10, TX_NO_TIME_SLICE, TX_AUTO_START);
  if (status != TX_SUCCESS) {
      return TX_THREAD_ERROR;
  }

  // Create the communication thread
  status = tx_thread_create(&communication_thread, "Communication Thread",
                            Communication_Thread_Entry, 0,
                            communication_thread_stack, sizeof(communication_thread_stack),
                            10, 10, TX_NO_TIME_SLICE, TX_AUTO_START);
  if (status != TX_SUCCESS) {
      return TX_THREAD_ERROR;
  }

  // Create the control thread
  status = tx_thread_create(&control_thread, "Control Thread",
                            Control_Thread_Entry, 0,
                            control_thread_stack, sizeof(control_thread_stack),
                            10, 10, TX_NO_TIME_SLICE, TX_AUTO_START);
  if (status != TX_SUCCESS) {
      return TX_THREAD_ERROR;
  }
  /* USER CODE END App_ThreadX_Init */

  return ret;
}

  /**
  * @brief  Function that implements the kernel's initialization.
  * @param  None
  * @retval None
  */
void MX_ThreadX_Init(void)
{
  /* USER CODE BEGIN Before_Kernel_Start */

  /* USER CODE END Before_Kernel_Start */

  tx_kernel_enter();

  /* USER CODE BEGIN Kernel_Start_Error */

  /* USER CODE END Kernel_Start_Error */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
