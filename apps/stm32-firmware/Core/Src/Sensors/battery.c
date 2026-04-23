#include "sensors.h"
#include "../Drivers/ina219.h"

void Battery_Thread_Entry(ULONG thread_input)
{
	INA219_t ina219_1;
	INA219_t ina219_2;
	(void)thread_input;
	VehicleData_t local_data;

	while (!INA219_Init(&ina219_1, &hi2c1, INA219_ADDRESS))
	{
	}
	while (!INA219_Init(&ina219_2, &hi2c1, INA219_ADDRESS_2))
	{
	}
	while (1)
	{
		local_data.battery_voltage = INA219_ReadBusVoltage(&ina219_1);
		local_data.battery_percentage = INA219_GetBatteryLife(&ina219_1, 12600, 9800);
		local_data.rasp_battery_voltage = INA219_ReadBusVoltage(&ina219_2);
		local_data.rasp_battery_percentage = INA219_GetBatteryLife(&ina219_2, 12600, 9800);
		local_data.data_valid = 1;
		char voltage_buf[100];
		snprintf(voltage_buf, sizeof(voltage_buf), "[BATTERY] Voltage: %u mV, Percentage: %.2f%% | Rasp Voltage: %u mV, Rasp Percentage: %.2f%%\r\n",
				 local_data.battery_voltage, local_data.battery_percentage, local_data.rasp_battery_voltage, local_data.rasp_battery_percentage);
		Debug_Print(voltage_buf);
		if (tx_mutex_get(&g_vehicle_data_mutex, TX_WAIT_FOREVER) == TX_SUCCESS)
		{
			g_vehicle_data.battery_voltage = local_data.battery_voltage;
			g_vehicle_data.battery_percentage = local_data.battery_percentage;
			g_vehicle_data.rasp_battery_voltage = local_data.rasp_battery_voltage;
			g_vehicle_data.rasp_battery_percentage = local_data.rasp_battery_percentage;
			g_vehicle_data.data_valid = local_data.data_valid;
			tx_mutex_put(&g_vehicle_data_mutex);
		}
		// TX_TIMER_TICKS_PER_SECOND is defined as 100 ticks/second, so 200 ticks = 2s
		tx_thread_sleep(200);
	}
}
