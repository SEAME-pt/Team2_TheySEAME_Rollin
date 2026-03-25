#include "imu.h"

void IMU_Thread_Entry(ULONG thread_input) {
    BSP_MOTION_SENSOR_Axes_t axes;  // Changed from pointer to actual struct
    char comm_uart_buff_acc[128];
    char comm_uart_buff_gyr[128];
    uint32_t instance = 0;  // Instance 0 for ISM330DLC (accelerometer/gyroscope)
    
    // Initialize the motion sensor with accelerometer function
    int32_t result = BSP_MOTION_SENSOR_Init(instance, MOTION_ACCELERO);
    if (result != BSP_ERROR_NONE) {
        snprintf(comm_uart_buff_acc, sizeof(comm_uart_buff_acc), "[ IMU Thread ] Failed to initialize Accelerometer\n\r");
        Debug_Print(comm_uart_buff_acc);
        return;
    }

    result = BSP_MOTION_SENSOR_Init(instance, MOTION_GYRO);  
    if (result != BSP_ERROR_NONE) {
        snprintf(comm_uart_buff_acc, sizeof(comm_uart_buff_acc), "[ IMU Thread ] Failed to initialize GYRO sensor\n\r");
        Debug_Print(comm_uart_buff_acc);
        return;
    }

    BSP_MOTION_SENSOR_Enable(instance, MOTION_ACCELERO);
    BSP_MOTION_SENSOR_Enable(instance, MOTION_GYRO);

    while (1) {
        BSP_MOTION_SENSOR_GetAxes(instance, MOTION_ACCELERO, &axes);  // Pass address of struct
        snprintf(comm_uart_buff_acc, sizeof(comm_uart_buff_acc), "[ IMU Thread ] a_x = %.2f m/s2, a_y = %.2f m/s2, a_z = %.2f m/s2\n\r", (float)axes.xval/100, (float)axes.yval/100, (float)axes.zval/100);
        Debug_Print(comm_uart_buff_acc);


        BSP_MOTION_SENSOR_GetAxes(instance, MOTION_GYRO, &axes);  // Pass address of struct
        snprintf(comm_uart_buff_gyr, sizeof(comm_uart_buff_gyr), "[ IMU Thread ] theta_x = %.2fdegrees, theta_y = %.2fdegrees, theta_z = %.2fdegrees\n\r", (float)axes.xval/100, (float)axes.yval/100, (float)axes.zval/100);
        Debug_Print(comm_uart_buff_gyr);
        tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND);  // 1s delay
    }
}