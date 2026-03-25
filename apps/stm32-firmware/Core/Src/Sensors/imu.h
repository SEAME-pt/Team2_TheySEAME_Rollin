#ifndef IMU_H
#define IMU_H

#include "main.h"
#include "b_u585i_iot02a_motion_sensors.h"
#include "tx_api.h" // For ThreadX TX_TIMER_TICKS_PER_SECOND

// Forward declaration of the IMU thread entry function
void IMU_Thread_Entry(ULONG thread_input);

#endif