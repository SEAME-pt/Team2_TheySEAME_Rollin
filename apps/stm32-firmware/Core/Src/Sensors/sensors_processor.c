#include "sensors_queue.h"
#include "sensors.h"
#include "tx_api.h"
#include <stdio.h>

void SensorsProcessor_Thread_Entry(ULONG thread_input) {
    (void)thread_input;
    SensorSample_t samp;
    char buf[128];

    while (1) {
        if (SensorsQueue_Receive(&samp, TX_WAIT_FOREVER) == TX_SUCCESS) {
            switch (samp.sensor_id) {
                case SENSOR_ID_SPEED:
                    if (tx_mutex_get(&g_vehicle_data_mutex, TX_WAIT_FOREVER) == TX_SUCCESS) {
                        g_vehicle_data.vehicle_speed = samp.value;
                        g_vehicle_data.data_valid = 1;
                        tx_mutex_put(&g_vehicle_data_mutex);
                    }
                    // snprintf(buf, sizeof(buf), "[SENSOR PROC] Speed updated: %.2f m/s\r\n", samp.value);
                    // Debug_Print(buf);
                    break;
                case SENSOR_ID_BATTERY:
                    if (tx_mutex_get(&g_vehicle_data_mutex, TX_WAIT_FOREVER) == TX_SUCCESS) {
                        g_vehicle_data.battery_percentage = samp.value;
                        g_vehicle_data.data_valid = 1;
                        tx_mutex_put(&g_vehicle_data_mutex);
                    }
                    // snprintf(buf, sizeof(buf), "[SENSOR PROC] Battery updated: %.2f%%\r\n", samp.value);
                    // Debug_Print(buf);
                    break;
                default:
                    Debug_Print("[SENSOR PROC] Unknown sensor id\r\n");
                    break;
            }
        }
    }
}
