#include "sensors.h"
#include "../Drivers/ina219.h"


void Battery_Thread_Entry(ULONG thread_input) {
    INA219_t ina219;
    uint16_t power;
    float   battery_life;
    uint16_t voltage;
    char buffer_debug[125];
    while(!INA219_Init(&ina219, &hi2c1, INA219_ADDRESS))
    {
    }
    while (1) {
        power = INA219_ReadPower(&ina219);
        voltage = INA219_ReadBusVoltage(&ina219);
        battery_life = INA219_GetBatteryLife(&ina219, 12600, 9800);
        snprintf(buffer_debug, sizeof(buffer_debug), "Power: %fmW, Voltage: %uV, Battery Life: %f%%\r\n", power, voltage, battery_life);
        Debug_Print(buffer_debug);
        tx_thread_sleep(200);
    }
}