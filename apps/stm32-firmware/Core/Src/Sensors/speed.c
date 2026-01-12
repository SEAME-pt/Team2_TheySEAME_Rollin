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
 * [impl->dsn~speed-counter-overflow~1]
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
 * @brief Atomically read and reset delta_ticks global variable
 *
 * This function safely reads the delta_ticks value and resets it to zero
 * in an atomic operation to prevent race conditions with the interrupt handler.
 *
 * @return The delta_ticks value before reset
 */
static uint32_t Speed_ReadDeltaTicks(void)
{
    uint32_t local_delta;
    __disable_irq();
    local_delta = delta_ticks;
    delta_ticks = 0;
    __enable_irq();
    return local_delta;
}

/*
 * @brief Calculate RPM from timer delta ticks
 *
 * Calculates motor RPM based on the time delta between pulses.
 * Returns 0 if delta is below the noise filtering threshold.
 *
 * Timer Configuration:
 * - Timer frequency: 160MHz / (PSC+1) = 160MHz / 8000 = 20kHz (50μs per tick)
 * - Minimum valid period: 20 ticks (1ms) for noise filtering
 *
 * @param delta_ticks Time delta in timer ticks
 * @return Calculated RPM, or 0 if delta is too small (noise filtered)
 */
uint32_t Speed_CalculateRPM(uint32_t delta_ticks)
{
    // Ignore zero or unrealistically small deltas (debounce/glitches)
    // Timer tick frequency = 160 MHz / (PSC+1). With PSC=7999, tick = 50us => 20kHz.
    // Minimum valid period threshold: e.g., 20 ticks (1ms) to filter noise.
    if (delta_ticks < 20)
    {
        return 0;
    }

    const uint32_t timer_freq_hz = 160000000U / (7999U + 1U); // 20,000 Hz
    return (60U * timer_freq_hz) / (delta_ticks * PULSES_PER_REV);
}

/*
 * @brief Convert RPM to linear speed in m/s
 *
 * Converts rotational speed (RPM) to linear velocity using wheel circumference.
 * Wheel circumference: 0.21 meters
 *
 * @param rpm Rotational speed in RPM
 * @return Linear speed in meters per second
 */
float Speed_RPMToMetersPerSecond(uint32_t rpm)
{
    return (float)rpm * 0.21f / 60.0f;
}

/*
 * @brief Process one iteration of speed calculation
 *
 * This function processes one delta reading, updates the running average,
 * and outputs results when 5 readings have been accumulated. This is the
 * core logic extracted from the thread loop for testability.
 *
  * ====================== Requirement Traceability ===========================
 * [impl->dsn~rpm-latency~1]
 * [impl->dsn~rpm-sensing~1]
 * ==========================================================================

 * @param delta_ticks Time delta in timer ticks
 * @param average Pointer to running average accumulator (modified)
 * @param counter Pointer to reading counter (modified)
 * @return 1 if output was generated (5 readings reached), 0 otherwise
 */
int Speed_ProcessDelta(uint32_t delta_ticks, uint32_t *average, int *counter)
{
    uint32_t rpm = Speed_CalculateRPM(delta_ticks);
    
    if (rpm == 0)
    {
        return 0; // Noise filtered, no processing
    }

    *average += rpm;
    (*counter)++;

    if (*counter == 5)
    {
        *average = *average / 5;
        float speed_ms = Speed_RPMToMetersPerSecond(*average);
        char uart_buf[128];
        snprintf(uart_buf, sizeof(uart_buf), "[SPEED THREAD] RPM = %lu, Speed (m/s) = %.2f\r\n", *average, speed_ms);
        Debug_Print(uart_buf);
        
        if (tx_mutex_get(&g_vehicle_command_mutex, TX_WAIT_FOREVER) == TX_SUCCESS)
        {
            g_vehicle_data.vehicle_speed = speed_ms;
            tx_mutex_put(&g_vehicle_command_mutex);
        }
        
        *counter = 0;
        *average = 0;
        return 1; // Output generated
    }

    return 0; // Still accumulating
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
 * ==========================================================================
 *
 * @param thread_input Thread parameter passed by ThreadX scheduler (unused in this implementation)
 *
 * @return void
 *
 */
void Speed_Thread_Entry(ULONG thread_input)
{
    (void)thread_input; // Unused parameter
    
    uint32_t average = 0;
    int counter = 0;

    while(1)
    {
        uint32_t local_delta = Speed_ReadDeltaTicks();
        Speed_ProcessDelta(local_delta, &average, &counter);
        
        // TX_TIMER_TICKS_PER_SECOND is defined as 100 ticks/second, so 10 ticks = 0.1s
        tx_thread_sleep(THREAD_SLEEP_TICKS);
    }
}
