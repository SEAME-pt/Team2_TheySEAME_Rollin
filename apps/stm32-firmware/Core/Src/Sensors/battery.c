#include "sensors.h"
#include "../Drivers/ina219.h"

void Battery_Thread_Entry(ULONG thread_input)
{
	INA219_t ina219;
	(void)thread_input;
	VehicleData_t local_data;

	while (!INA219_Init(&ina219, &hi2c1, INA219_ADDRESS))
	{
	}

	while (1)
	{
		local_data.battery_voltage = INA219_ReadBusVoltage(&ina219);
		local_data.battery_percentage = INA219_GetBatteryLife(&ina219, 12600, 9800);
		local_data.data_valid = 1;

		if (tx_mutex_get(&g_vehicle_data_mutex, TX_WAIT_FOREVER) == TX_SUCCESS)
		{
			g_vehicle_data.battery_voltage = local_data.battery_voltage;
			g_vehicle_data.battery_percentage = local_data.battery_percentage;
			g_vehicle_data.data_valid = local_data.data_valid;
			tx_mutex_put(&g_vehicle_data_mutex);
		}
		// TX_TIMER_TICKS_PER_SECOND is defined as 100 ticks/second, so 200 ticks = 2s
		tx_thread_sleep(200);
	}
}
