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

void send_cmd(bool brake)
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

#define DT_SECONDS          0.02f
#define TTC_THRESHOLD_MS    350.0f
#define BRAKE_THRESHOLD_CM  20
#define CLEAR_DIST_CM       100

static uint16_t dist_old     = 80;
static float    velocity_cms = 0.0f;
static float    ttc_ms       = 9999.0f;

void Automatic_Brake_Assist(uint16_t distance_cm)
{
    VehicleData_t recvs;
    VehicleCommand_t Vcmd;
    snapshot_vehicle_command(&Vcmd);
    snapshot_vehicle_data(&recvs);

    if (Vcmd.aeb_enabled == false) {
         if (Vcmd.brake) send_cmd(false);
         dist_old = distance_cm;
         return;
    }    
    
    if (Vcmd.brake && distance_cm <= BRAKE_THRESHOLD_CM && distance_cm > 0 && Vcmd.gear != 2) {
        send_cmd(true);
        dist_old = distance_cm;
        return;
    }

    if (distance_cm > CLEAR_DIST_CM || distance_cm == 0) {
        dist_old = distance_cm;
        if (Vcmd.brake) send_cmd(false);
        return;
    }

    if (Vcmd.gear == 2) {
        if (Vcmd.brake) send_cmd(false);
        dist_old = distance_cm;
        return;
    }


    float current_speed = recvs.vehicle_speed;
    float current_speed_cms = current_speed * 100.0f;

    int16_t delta = (int16_t)dist_old - (int16_t)distance_cm;
    if (abs(delta) > 1)
        velocity_cms = (float)delta / DT_SECONDS;
    else
        velocity_cms = 0.0f;

    float closing_cms = (velocity_cms > current_speed_cms) ? velocity_cms : current_speed_cms;

    if (closing_cms > 10.0f)
        ttc_ms = ((float)distance_cm / closing_cms) * 1000.0f;
    else
        ttc_ms = 9999.0f;

    if (current_speed <= 0.1f) {
        if (Vcmd.brake) send_cmd(false);
        dist_old = distance_cm;
        return;
    }


    if (ttc_ms < TTC_THRESHOLD_MS || distance_cm <= BRAKE_THRESHOLD_CM) {
        send_cmd(true);
    } else {
        if (Vcmd.brake) send_cmd(false);
    }

    dist_old = distance_cm;
}

void Distance_Thread_Entry(ULONG thread_input)
{
    uint16_t distance = 0;
    uint8_t  light    = 0;
    uint8_t  buffer[4];
    char     msg[64];

    uint8_t range_reg = 35;
    uint8_t gain_reg  = 5;
    HAL_I2C_Mem_Write(&hi2c2, 0xE0, 0x02, 1, &range_reg, 1, 100);
    HAL_I2C_Mem_Write(&hi2c2, 0xE0, 0x01, 1, &gain_reg,  1, 100);
    tx_thread_sleep(10);

    while (1)
    {
        uint8_t cmd = 0x51;
        HAL_I2C_Mem_Write(&hi2c2, 0xE0, 0x00, 1, &cmd, 1, 100);

        uint8_t status = 0xFF;
        uint32_t timeout = 20;
        while (status == 0xFF && timeout > 0)
        {
            tx_thread_sleep(2);
            timeout -= 2;
            HAL_I2C_Mem_Read(&hi2c2, 0xE0, 0x00, 1, &status, 1, 100);
        }

        if (timeout == 0)
        {
            Debug_Print("[DIST] Sensor timeout!\r\n");
            continue;
        }

        if (HAL_I2C_Mem_Read(&hi2c2, 0xE0, 0x00, 1, buffer, 4, 100) == HAL_OK)
        {
            light    = buffer[1];
            distance = (buffer[2] << 8) | buffer[3];

            snprintf(msg, sizeof(msg), "[DIST] %d cm | [LIGHT] %d\r\n", distance, light);
            // Debug_Print(msg);

            Automatic_Brake_Assist(distance);
        }
        else
        {
            Debug_Print("[Distance THREAD] Error!\r\n");
        }
    }
}
