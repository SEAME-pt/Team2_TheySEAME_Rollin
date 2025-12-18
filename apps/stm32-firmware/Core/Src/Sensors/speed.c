#include "sensors.h"

uint32_t delta_ticks = 0;

/*
 * @brief Timer input capture callback for speed sensor pulse detection
 *
 * This interrupt callback is triggered on each rising edge detected on TIM1 Channel 3.
 * It calculates the time delta between consecutive pulses by reading the timer capture
 * register and handling potential timer overflow conditions. The calculated delta is
 * stored globally for processing by the Speed_Thread_Entry function.
 *
 * ====================== Requirement Traceability ===========================
 * impl->dsn~speed-counter-overflow~1
 * impl->dsn~rpm-sensing~1
 * ==========================================================================
 *
 * @param htim Pointer to TIM_HandleTypeDef structure containing timer configuration
 *             and state information. Expected to be TIM1 with active Channel 3.
 *
 * @return void
 *
 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    static uint32_t prev_capture = 0;
    uint32_t curr_capture;
    if(htim->Instance == TIM1 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3)
    {
        curr_capture = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_3);

        if(curr_capture >= prev_capture)
            delta_ticks = curr_capture - prev_capture;
        else
            delta_ticks = (0xFFFF - prev_capture + 1) + curr_capture; // handle overflow
        prev_capture = curr_capture;
    }
    else {
        Debug_Print("not entered\r\n");
    }
}

/*
 * @brief Main speed calculation thread for RPM and velocity computation
 *
 * This thread continuously processes pulse timing data captured by the timer interrupt
 * to calculate motor RPM and linear speed. It implements noise filtering to ignore
 * invalid short pulses, averages multiple RPM readings for stability, and converts
 * rotational speed to linear velocity using wheel circumference. Results are output
 * via UART debug interface every 0.5 seconds (5 readings average).
 *
 * Timer Configuration:
 * - TIM1 Channel 3 set to Alternate Mode with Input Capture Direct Mode
 * - Timer frequency: 160MHz / (PSC+1) = 160MHz / 8000 = 20kHz (50μs per tick)
 * - Minimum valid period: 20 ticks (1ms) for noise filtering
 * - Thread sleep: 0.1s between iterations (based on ThreadX timer)
 *
 * ====================== Requirement Traceability ===========================
 * impl->dsn~rpm-latency~1
 * ==========================================================================
 *
 * @param thread_input Thread parameter passed by ThreadX scheduler (unused in this implementation)
 *
 * @return void
 *
 */
void Speed_Thread_Entry(ULONG thread_input)
{
    uint32_t local_delta;
    uint32_t rpm;
    uint32_t average = 0;
    float speed_ms = 0;
    int counter = 0;

    while(1)
    {
        __disable_irq();
        local_delta = delta_ticks;
        delta_ticks = 0;  
        __enable_irq();

        // Ignore zero or unrealistically small deltas (debounce/glitches)
        // Timer tick frequency = 160 MHz / (PSC+1). With PSC=7999, tick = 50us => 20kHz.
        // Minimum valid period threshold: e.g., 20 ticks (1ms) to filter noise.
        if (local_delta >= 20)
        {
            const uint32_t timer_freq_hz = 160000000U / (7999U + 1U); // 20,000 Hz
            rpm = (60U * timer_freq_hz) / (local_delta * PULSES_PER_REV);
            local_delta = 0;
            average += rpm;
            counter++;
            if (counter == 5) {
                average = average / 5;
                speed_ms = (float)average * 0.21f / 60.0f;
                snprintf(uart_buf, sizeof(uart_buf), "[SPEED THREAD] RPM = %lu, Speed (m/s) = %.2f\r\n", average, speed_ms);
                Debug_Print(uart_buf);
                counter = 0;
                average = 0;
            }
        }
        // TX_TIMER_TICKS_PER_SECOND is defined as 100 ticks/second, so 10 ticks = 0.1s
        tx_thread_sleep(THREAD_SLEEP_TICKS);
    }
}
