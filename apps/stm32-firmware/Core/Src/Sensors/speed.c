#include "sensors.h"

uint32_t IC_Val1 = 0;
uint32_t IC_Val2 = 0;
uint32_t Difference = 0;
int Is_First_Captured = 0;

/* Measure Frequency */
float frequency = 0;

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    static uint32_t prev_capture = 0;
    uint32_t curr_capture;
//            snprintf(uart_buf, sizeof(uart_buf), "TIM CAPTURE CALLBACK ENTERED\n");
//            Debug_Print(uart_buf);
    if(htim->Instance == TIM1 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3)
    {
        curr_capture = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_3);

        // Calculate delta_time in timer ticks
        if(curr_capture >= prev_capture)
            delta_time_us = curr_capture - prev_capture;
        else
            delta_time_us = (0xFFFF - prev_capture + 1) + curr_capture; // handle overflow

        prev_capture = curr_capture;
    }
}

/*
 * @brief 
 *
 * Description
 *
 * ====================== Requirement Traceability ===========================
 * impl->dsn~rpm-latency~1
 * ==========================================================================
 *
 * @param name         Function
 *
 * @return
 *
 */

void Speed_Thread_Entry(ULONG thread_input)
{
    uint32_t local_delta;
    uint32_t rpm;

    while(1)
    {
        // Read delta_time safely
        __disable_irq();
        local_delta = delta_time_us;
        __enable_irq();         // Re-enable interrupts

        if (local_delta > 0)
        {
            rpm = 60.0f * 1000000.0f / (local_delta * PULSES_PER_REV);
            local_delta = 0;
        }

        // Sleep to reduce CPU usage
        // TX_TIMER_TICKS_PER_SECOND is defined as 100 ticks/second, so 50 ticks = 0.5s
        tx_thread_sleep(THREAD_SLEEP_TICKS);
    }
}