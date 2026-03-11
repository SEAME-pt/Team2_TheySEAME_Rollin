#include "sensors.h"
#include "sensors_queue.h"
#include "../Drivers/ina219.h"

void Battery_Thread_Entry(ULONG thread_input)
{
	INA219_t ina219;
	(void)thread_input;

	while (!INA219_Init(&ina219, &hi2c1, INA219_ADDRESS))
	{
	}

	while (1)
	{
		float battery_percentage = INA219_GetBatteryLife(&ina219, 12600, 9800);
		
		// Send battery sample to sensors queue
		SensorSample_t samp;
		samp.sensor_id = SENSOR_ID_BATTERY;
		samp.value = battery_percentage;
		samp.ts = HAL_GetTick();
		
		SensorsQueue_TrySend(&samp);
		
		// TX_TIMER_TICKS_PER_SECOND is defined as 100 ticks/second, so 200 ticks = 2s
		tx_thread_sleep(200);
	}
}
