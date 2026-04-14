#include "sensors.h"
#include "sensors_queue.h"

uint32_t delta_ticks = 0;

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	static uint32_t prev_capture = 0;
	uint32_t curr_capture;

	if (htim->Instance == TIM1 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3)
	{
		curr_capture = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_3);

		if (curr_capture >= prev_capture)
			delta_ticks = curr_capture - prev_capture;
		else
			delta_ticks = (0xFFFF - prev_capture + 1) + curr_capture;

		prev_capture = curr_capture;
	}
}

static uint32_t Speed_ReadDeltaTicks(void)
{
	uint32_t local_delta;
	__disable_irq();
	local_delta = delta_ticks;
	delta_ticks = 0;
	__enable_irq();
	return local_delta;
}

uint32_t Speed_CalculateRPM(uint32_t delta_ticks)
{
	// Ignore zero or unrealistically small deltas (debounce/glitches)
	if (delta_ticks < 20)
		return 0;

	const uint32_t timer_freq_hz = 160000000U / (7999U + 1U);
	return (60U * timer_freq_hz) / (delta_ticks * PULSES_PER_REV);
}

float Speed_RPMToMetersPerSecond(uint32_t rpm)
{
	return (float)rpm * 0.21f / 60.0f;
}

int Speed_ProcessDelta(uint32_t delta_ticks, uint32_t *average, int *counter, float *out_speed_ms)
{
	uint32_t rpm = Speed_CalculateRPM(delta_ticks);

	if (rpm == 0)
		return 0;

	*average += rpm;
	(*counter)++;

	if (*counter == 5)
	{
		*average = *average / 5;
		float speed_ms = Speed_RPMToMetersPerSecond(*average);

		if (out_speed_ms)
			*out_speed_ms = speed_ms;

		*counter = 0;
		*average = 0;
		return 1;
	}

	return 0;
}

void Speed_Thread_Entry(ULONG thread_input)
{
	(void)thread_input;

	uint32_t average = 0;
	int counter = 0;
	int no_pulse_counter = 0;

	while (1)
	{
		uint32_t local_delta = Speed_ReadDeltaTicks();

		if (local_delta >= 20)
		{
			no_pulse_counter = 0;
			float speed_ms;
			if (Speed_ProcessDelta(local_delta, &average, &counter, &speed_ms))
			{
				SensorSample_t samp = { .sensor_id = SENSOR_ID_SPEED, .value = speed_ms, .ts = HAL_GetTick() };
				SensorsQueue_TrySend(&samp);
			}
		}
		else
		{
			// No valid pulse detected in this iteration
			no_pulse_counter++;

			// Stop after ~0.2s without pulses (THREAD_SLEEP_TICKS is 10 ticks = 0.1s)
			if (no_pulse_counter == 2)
			{
				SensorSample_t samp = { .sensor_id = SENSOR_ID_SPEED, .value = 0.0f, .ts = HAL_GetTick() };
				SensorsQueue_TrySend(&samp);
				counter = 0;
				average = 0;
			}

			if (no_pulse_counter > 255)
				no_pulse_counter = 0;  // Keep incrementing to stay past threshold, cap to prevent overflow
		}
		// TX_TIMER_TICKS_PER_SECOND is defined as 100 ticks/second, so 10 ticks = 0.1s
		tx_thread_sleep(THREAD_SLEEP_TICKS);
	}
}
