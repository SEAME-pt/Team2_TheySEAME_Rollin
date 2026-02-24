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
#include "Control/control_queue.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "Sensors/sensors.h"
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
TX_THREAD communication_thread;
TX_THREAD battery_thread;
TX_THREAD speed_thread;
TX_THREAD sensors_proc_thread;
TX_THREAD test_thread;
TX_THREAD control_thread;

UCHAR sensors_proc_thread_stack[2048];
UCHAR test_thread_stack[2048];
UCHAR speed_thread_stack[2048];
UCHAR battery_thread_stack[2048];
UCHAR communication_thread_stack[2048];
UCHAR control_thread_stack[2048];

extern void Battery_Thread_Entry(ULONG thread_input);
extern void Communication_Thread_Entry(ULONG thread_input);
extern void Control_Thread_Entry(ULONG thread_input);
extern void Test_Thread_Entry(ULONG thread_input);
extern void Speed_Thread_Entry(ULONG thread_input);
extern void SensorsProcessor_Thread_Entry(ULONG thread_input);


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
    g_vehicle_command.gear = 3;  // Default to Drive
    g_vehicle_command.throttle = 0;
    g_vehicle_command.steering_angle = 0;
    g_vehicle_command.command_valid = 0;
    
    // Initialize global vehicle data structure
    g_vehicle_data.battery_voltage = 0;
    g_vehicle_data.battery_percentage = 0.0f;
    g_vehicle_data.battery_current = 0.0f;
    g_vehicle_data.vehicle_speed = 0.0f;
    g_vehicle_data.data_valid = 0;
    
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

  /* Initialize control queue used for passing commands to the Control thread */
  ControlQueue_Init();

  /* Initialize sensors queue (sensor producers -> sensor processor) */
  SensorsQueue_Init();

  /* Disable test thread by default in production builds. Set TEST_MODE=1 for local test runs. */
  #ifndef TEST_MODE
  #define TEST_MODE 0
  #endif

  #if TEST_MODE
  status = tx_thread_create(&test_thread, "Test Routine Thread",
                                  Test_Thread_Entry, 0,
                                  test_thread_stack, sizeof(test_thread_stack),
                                  10, 10, TX_NO_TIME_SLICE, TX_AUTO_START);
    if (status != TX_SUCCESS) {
      return TX_THREAD_ERROR;
    }
  #endif
  
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
                            8, 8, TX_NO_TIME_SLICE, TX_AUTO_START);
  if (status != TX_SUCCESS) {
      return TX_THREAD_ERROR;
  }

  // Create sensors processor thread (aggregates samples and updates g_vehicle_data)
  status = tx_thread_create(&sensors_proc_thread, "Sensors Proc Thread",
                            SensorsProcessor_Thread_Entry, 0,
                            sensors_proc_thread_stack, sizeof(sensors_proc_thread_stack),
                            12, 12, TX_NO_TIME_SLICE, TX_AUTO_START);
  if (status != TX_SUCCESS) {
      return TX_THREAD_ERROR;
  }

  status = tx_thread_create(&speed_thread, "Speed Thread",
                                  Speed_Thread_Entry, 0,
                                  speed_thread_stack, sizeof(speed_thread_stack),
                                  14, 14, TX_NO_TIME_SLICE, TX_AUTO_START);
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
