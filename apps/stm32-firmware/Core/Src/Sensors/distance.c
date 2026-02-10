#include "sensors.h"
#include <stdio.h>

void Distance_Thread_Entry(ULONG thread_input)
{
    uint16_t distance = 0;
    uint8_t light = 0;
    uint8_t buffer[4];
    char msg[64];

    while (1)
    {
        uint8_t cmd = 0x51;
        HAL_I2C_Mem_Write(&hi2c2, 0xE0, 0x00, 1, &cmd, 1, 100);
    
        tx_thread_sleep(70); 

        if(HAL_I2C_Mem_Read(&hi2c2, 0xE0, 0x00, 1, buffer, 4, 100) == HAL_OK)
        {
            light = buffer[0]; 
            distance = (buffer[2] << 8) | buffer[3];

            sprintf(msg, "[DIST] %d cm | [LIGHT] %d\r\n", distance, light);
            
            Debug_Print(msg); 
        }
        else 
        {
            Debug_Print("[Distance THREAD] Error!\r\n");
        }

        tx_thread_sleep(200);
    }
}