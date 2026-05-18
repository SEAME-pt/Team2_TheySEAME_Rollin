#include "sensors.h"
#include <stdio.h>
#include "main.h"
#include "../Control/control.h"
#include "../Control/control_queue.h"
#include "sensors_queue.h"


void snapshot_vehicle_command(VehicleCommand_t *out) {
    if (tx_mutex_get(&g_vehicle_command_mutex, TX_WAIT_FOREVER) == TX_SUCCESS) {
        *out = g_vehicle_command;
        tx_mutex_put(&g_vehicle_command_mutex);
    } else {
        Debug_Print("[DISTANCE] Failed to acquire mutex for command snapshot\r\n");
    }
}

void snapshot_vehicle_data(VehicleData_t *out) {
    if (tx_mutex_get(&g_vehicle_data_mutex, TX_WAIT_FOREVER) == TX_SUCCESS) {
        *out = g_vehicle_data;
        tx_mutex_put(&g_vehicle_data_mutex);
    } else {
        Debug_Print("[DISTANCE] Failed to acquire mutex for data snapshot\r\n");
    }
}

void send_cmd(bool brake, VehicleCommand_t cmd)
{
    VehicleCommand_t local_cmd;
    if (tx_mutex_get(&g_vehicle_command_mutex, TX_WAIT_FOREVER) == TX_SUCCESS) {
        g_vehicle_command.brake = brake;
        g_vehicle_command.command_valid = 1;
        local_cmd = g_vehicle_command;
        tx_mutex_put(&g_vehicle_command_mutex);
    } else {
        Debug_Print("[COMM] Failed to acquire mutex for command snapshot\r\n");
        return;
    }
    if (!ControlQueue_TrySend(&local_cmd)) {
        Debug_Print("[COMM] Control queue full\r\n");
    }
}

void Automatic_Brake_Assist(uint16_t distance_cm) {
    VehicleData_t recvs;
    VehicleCommand_t Vcmd;

    snapshot_vehicle_command(&Vcmd);
    snapshot_vehicle_data(&recvs);

    const float DECEL_MS2    = 6.0f;
    const float REACTION_S   = 0.05f;
    const uint16_t OBSTACLE_THRESHOLD_CM = 10;
    const uint16_t RELEASE_HYSTERESIS_CM = 5; /* avoid chatter when releasing brake */

    float current_speed = recvs.vehicle_speed;
    float distance_m = distance_cm / 100.0f;
    float d_stop     = (current_speed * current_speed) / (2.0f * DECEL_MS2);
    float d_reaction = current_speed * REACTION_S;
    float d_required = d_stop + d_reaction + 0.15f;
    
    /* If vehicle is stationary, ensure brake is released and exit */
    if (current_speed <= 0.1f) {
        if (Vcmd.brake)
            send_cmd(false, Vcmd);
        return;
    }

    /* If not in drive (gear 3), ensure brake is not asserted by AEB */
    if (Vcmd.gear != 3) {
        if (Vcmd.brake)
            send_cmd(false, Vcmd);
        return;
    }

    /* protect against division by zero or extremely small required distance */
    if (d_required <= 1e-4f) {
        return;
    }

    if (distance_m <= d_required) {
        float urgency = 1.0f - (distance_m / d_required);
        if (urgency < 0.0f) urgency = 0.0f;
        else if (urgency > 1.0f) urgency = 1.0f;

        char info_buf[64];
        snprintf(info_buf, sizeof(info_buf), "[ABA] Distance=%.2fm, Required=%.2fm, Urgency=%.2f%%\r\n",
                 distance_m, d_required, urgency * 100.0f);
        Debug_Print(info_buf);
        if (urgency > 0.4f || distance_m < 0.20f) {
            send_cmd(true, Vcmd);
        }
        return;
    }

    /* release brake only when obstacle is sufficiently far (with hysteresis) */
    if (distance_cm > (OBSTACLE_THRESHOLD_CM + RELEASE_HYSTERESIS_CM))
        send_cmd(false, Vcmd);
}

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
    
        tx_thread_sleep(20); 
        
        if(HAL_I2C_Mem_Read(&hi2c2, 0xE0, 0x00, 1, buffer, 4, 100) == HAL_OK)
        {
            light = buffer[0]; 
            distance = 200;
            distance = (buffer[2] << 8) | buffer[3];
            sprintf(msg, "[DIST] %d cm | [LIGHT] %d\r\n", distance, light);
            Automatic_Brake_Assist(distance);
            // Debug_Print(msg); 
        }
        else 
        {
            Debug_Print("[Distance THREAD] Error!\r\n");
        }

    }
}
