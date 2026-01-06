#include "sensors.h"
#include "../Drivers/ina219.h"


void Battery_Thread_Entry(ULONG thread_input) {
    INA219_t ina219;
    uint16_t power;
    // float   battery_life;
    // uint16_t voltage;
    char buffer_debug[125];
    VehicleData_t local_data;

    while(!INA219_Init(&ina219, &hi2c1, INA219_ADDRESS))
    {
    }
    while (1) {
        power = INA219_ReadPower(&ina219);
        local_data.battery_voltage = INA219_ReadBusVoltage(&ina219);
        local_data.battery_percentage = INA219_GetBatteryLife(&ina219, 12600, 9800);
        local_data.data_valid = 1;
        snprintf(buffer_debug, sizeof(buffer_debug), "Power: %umW, Voltage: %humV, Battery Life: %f%%\r\n", power, local_data.battery_voltage, local_data.battery_percentage);
        Debug_Print(buffer_debug);
        if (tx_mutex_get(&g_vehicle_data_mutex, TX_WAIT_FOREVER) == TX_SUCCESS) {
            // Write local sensor data to global structure
            g_vehicle_data.battery_voltage = local_data.battery_voltage;
            g_vehicle_data.battery_percentage = local_data.battery_percentage;
            g_vehicle_data.data_valid = local_data.data_valid;
            tx_mutex_put(&g_vehicle_data_mutex);
        }
        tx_thread_sleep(200);
    }
}
