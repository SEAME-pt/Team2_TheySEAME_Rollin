/**
 ******************************************************************************
 * @file    communication.c
 * @brief   CAN Communication Thread - Reads sensor queue and transmits
 * @note    This module reads sensor samples directly from the sensors queue
 ******************************************************************************
 */

#include "comm.h"
#include "mcp2515.h"
#include "../Sensors/sensors.h"
#include "../Sensors/sensors_queue.h"
#include "main.h"
#include "../Control/control_queue.h"
#include <stdio.h>

#ifndef COMM_DEBUG
#define Debug_Print(msg) ((void)0)
#endif
char comm_uart_buf[128];
char rx_uart_buf[128];
extern UART_HandleTypeDef huart1;

/* Always-on print for RX messages (bypasses COMM_DEBUG gate) */
static inline void RX_Print(const char *msg) {
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), 100);
}

/* Maximum velocity in m/s (used for throttle % to velocity conversion) */
#define MAX_VELOCITY_MS 2.0f

/**
 * @brief Convert throttle percentage (0-100) to desired velocity in m/s
 * @param throttle_percent Throttle value 0-100%
 * @return float Desired velocity in m/s (0.0 to MAX_VELOCITY_MS)
 */
static inline float Throttle_ToVelocity(uint8_t throttle_percent) {
    return (throttle_percent / 100.0f) * MAX_VELOCITY_MS;
}

void Communication_Thread_Entry(ULONG thread_input) {
    (void) thread_input;
    
    Debug_Print("\r\n\r\n=== COMMUNICATION THREAD STARTED ===\r\n");
    Debug_Print("[COMM] Initializing MCP2515 CAN controller...\r\n");
    
    // Wait for other threads to initialize
    tx_thread_sleep(50);  // 500ms delay
    
    // Test MCP2515 connection
    MCP2515_TestConnection();
    
    // Initialize MCP2515
    if (MCP2515_Init(CAN_SPEED_500KBPS) == HAL_OK) {
        Debug_Print("[COMM] MCP2515 initialized (500 kbps) - NORMAL MODE\r\n");
        MCP2515_PrintDetailedStatus();
    } else {
        Debug_Print("[COMM] MCP2515 initialization FAILED!\r\n");
        MCP2515_PrintDetailedStatus();
    }
    
    Debug_Print("[COMM] Starting CAN loop (TX every 200ms, RX continuous)\r\n");
    Debug_Print("[COMM] Reading sensor samples directly from sensors queue\r\n\r\n");
    
    uint32_t count = 0;
    
    /* Local sensor state - updated from sensors queue */
    float battery_percentage = 0.0f;
    float vehicle_speed = 0.0f;
    int sensors_valid = 0;

    /* Heartbeat state: re-send last command if no new command seen for HEARTBEAT_MS */
    const uint32_t HEARTBEAT_MS = 1000; /* 1s heartbeat to avoid log flooding */
    /* Default safe command: desired_velocity=0, steering=0, command_valid=1
     * This prevents repeated safety stop messages until a real command is received.
     */
    VehicleCommand_t last_cmd = { .driving_mode = 0, .desired_velocity = 0.0f, .steering_angle = 0, .current_velocity = 0.0f, .command_valid = 1 };
    uint32_t last_cmd_ts = HAL_GetTick();
    int have_last_cmd = 1; /* start with default command enabled */

    Debug_Print("[COMM] Heartbeat initialized with default SAFE command\r\n");

    while(1) {
        // Drain sensor queue to get latest samples (non-blocking)
        SensorSample_t samp;
        while (SensorsQueue_Receive(&samp, TX_NO_WAIT) == TX_SUCCESS) {
            switch (samp.sensor_id) {
                case SENSOR_ID_SPEED:
                    vehicle_speed = samp.value;
                    sensors_valid = 1;
                    break;
                case SENSOR_ID_BATTERY:
                    battery_percentage = samp.value;
                    sensors_valid = 1;
                    break;
                default:
                    break;
            }
        }
        
        // Send status every 1s (every 100 loops at 10ms) to reduce log spam
        if (count % 100 == 0) {
            // Check if data is valid before sending
            if (sensors_valid) {
                // Send battery percentage over CAN (ID: 0x201)
                uint8_t battery = (uint8_t)battery_percentage;
                HAL_StatusTypeDef status = MCP2515_SendBattery(battery);
                // HAL_StatusTypeDef status = MCP2515_SendBattery(90);
            
                // Send speed over CAN (ID: 0x200 - SpeedMsg)
                HAL_StatusTypeDef speed_status = MCP2515_SendSpeed(vehicle_speed);

                // Single summary line for TX
                uint16_t speed_hmh = (uint16_t)(vehicle_speed * 36.0f);
                snprintf(comm_uart_buf, sizeof(comm_uart_buf),
                        "[TX] 0x200 Speed=%u hm/h (%s) | 0x201 Bat=%u%% (%s)\r\n",
                        speed_hmh,
                        (speed_status == HAL_OK) ? "OK" : (speed_status == HAL_BUSY) ? "BUSY" : "FAIL",
                        battery,
                        (status == HAL_OK) ? "OK" : (status == HAL_BUSY) ? "BUSY" : "FAIL");
                Debug_Print(comm_uart_buf);
                
                // If bus-off detected, try to recover
                if (status == HAL_TIMEOUT || speed_status == HAL_TIMEOUT) {
                    static uint32_t busoff_count = 0;
                    busoff_count++;
                    if (busoff_count >= 3) {
                        Debug_Print("[COMM] Resetting MCP2515 due to bus-off...\r\n");
                        MCP2515_Init(CAN_SPEED_500KBPS);
                        busoff_count = 0;
                    }
                }
                
            } else {
                Debug_Print("[CAN_TX] Waiting for valid sensor data...\r\n");
            }
        }
        
        // Check for received CAN messages (commands from controller)
        // Process ALL pending messages in buffer to prevent backlog
        uint32_t rx_can_id;
        uint8_t rx_data[8];
        uint8_t rx_length;
        
        while (MCP2515_ReceiveMessage(&rx_can_id, rx_data, &rx_length)) {
            // Always print received frame: ID + raw bytes
            snprintf(rx_uart_buf, sizeof(rx_uart_buf),
                    "[RX] ID=0x%08lX DLC=%d Data=", (unsigned long)rx_can_id, rx_length);
            RX_Print(rx_uart_buf);
            for (int i = 0; i < rx_length; i++) {
                snprintf(rx_uart_buf, sizeof(rx_uart_buf), "%02X ", rx_data[i]);
                RX_Print(rx_uart_buf);
            }
            RX_Print("\r\n");
            
            // Process received CAN messages
            // Controller (RPi): 0x100 DLC=3 → [mode, throttle, steering] combined format
            // Kuksa: Individual IDs per README spec
            int cmd_updated = 0;

            if (rx_can_id == 0x100 && rx_length == 3) {
                // Controller combined frame: [mode, throttle, steering]
                uint8_t mode = rx_data[0] > 0 ? 1 : 0;
                uint8_t throttle = rx_data[1] > 100 ? 100 : rx_data[1];
                // steering: RPi sends integer division result (-1, 0, 1), scale to -100..100
                int8_t steering_raw = (int8_t)rx_data[2];
                int8_t steering = steering_raw * 100; // -1→-100, 0→0, 1→100

                if (tx_mutex_get(&g_vehicle_command_mutex, TX_WAIT_FOREVER) == TX_SUCCESS) {
                    g_vehicle_command.driving_mode = mode;
                    g_vehicle_command.gear = 3;  // Controller doesn't send gear, default to Drive
                    g_vehicle_command.desired_velocity = desired_vel;
                    g_vehicle_command.steering_angle = steering;
                    g_vehicle_command.command_valid = 1;
                    tx_mutex_put(&g_vehicle_command_mutex);
                }
                cmd_updated = 1;

                snprintf(comm_uart_buf, sizeof(comm_uart_buf),
                        "[CMD] Controller: Mode=%s Throttle=%d%% (%.2fm/s) Steering=%d\r\n",
                        mode ? "AI" : "MAN", throttle, desired_vel, steering);
                Debug_Print(comm_uart_buf);
            }
            else if (rx_can_id == 0x100 && rx_length >= 1) {
                // Kuksa ThrottleMsg: DLC=8, first byte is throttle 0-100%
                uint8_t throttle = rx_data[0] > 100 ? 100 : rx_data[0];
                float desired_vel = Throttle_ToVelocity(throttle);
                if (tx_mutex_get(&g_vehicle_command_mutex, TX_WAIT_FOREVER) == TX_SUCCESS) {
                    g_vehicle_command.desired_velocity = desired_vel;
                    g_vehicle_command.command_valid = 1;
                    tx_mutex_put(&g_vehicle_command_mutex);
                }
                cmd_updated = 1;

                snprintf(comm_uart_buf, sizeof(comm_uart_buf),
                        "[CMD] Throttle=%d%% (%.2fm/s)\r\n", throttle, desired_vel);
                Debug_Print(comm_uart_buf);
            }
            else if (rx_can_id == 0x101 && rx_length >= 1) {
                // Gear: 0=P, 1=N, 2=R, 3=D
                uint8_t gear = rx_data[0];
                if (tx_mutex_get(&g_vehicle_command_mutex, TX_WAIT_FOREVER) == TX_SUCCESS) {
                    g_vehicle_command.gear = gear;
                    g_vehicle_command.command_valid = 1;
                    tx_mutex_put(&g_vehicle_command_mutex);
                }
                cmd_updated = 1;

                const char* gear_names[] = {"P", "N", "R", "D"};
                snprintf(comm_uart_buf, sizeof(comm_uart_buf),
                        "[CMD] Gear=%s\r\n", gear <= 3 ? gear_names[gear] : "?");
                Debug_Print(comm_uart_buf);
            }
            else if (rx_can_id == 0x102 && rx_length == 1) {
                // Controller AngleMsg: DLC=1, value is -1/0/1 from integer division, scale ×100
                int8_t steering_raw = (int8_t)rx_data[0];
                int8_t steering = steering_raw * 100; // -1→-100, 0→0, 1→100
                if (tx_mutex_get(&g_vehicle_command_mutex, TX_WAIT_FOREVER) == TX_SUCCESS) {
                    g_vehicle_command.steering_angle = steering;
                    g_vehicle_command.command_valid = 1;
                    tx_mutex_put(&g_vehicle_command_mutex);
                }
                cmd_updated = 1;

                snprintf(comm_uart_buf, sizeof(comm_uart_buf),
                        "[CMD] Steering=%d (raw=%d)\r\n", steering, steering_raw);
                Debug_Print(comm_uart_buf);
            }
            else if (rx_can_id == 0x102 && rx_length >= 8) {
                // Kuksa AngleMsg: DLC=8, first byte is 0-100 percentage, use as-is
                int8_t steering = (int8_t)rx_data[0];
                if (tx_mutex_get(&g_vehicle_command_mutex, TX_WAIT_FOREVER) == TX_SUCCESS) {
                    g_vehicle_command.steering_angle = steering;
                    g_vehicle_command.command_valid = 1;
                    tx_mutex_put(&g_vehicle_command_mutex);
                }
                cmd_updated = 1;

                snprintf(comm_uart_buf, sizeof(comm_uart_buf),
                        "[CMD] Steering=%d%%\r\n", steering);
                Debug_Print(comm_uart_buf);
            }
            else if (rx_can_id == 0x103 && rx_length >= 1) {
                // Break: 0=DRIVE, 1=BREAK (logged only, no struct field)
                snprintf(comm_uart_buf, sizeof(comm_uart_buf),
                        "[CMD] Brake=%d\r\n", rx_data[0]);
                Debug_Print(comm_uart_buf);
            }
            else if (rx_can_id == 0x104 && rx_length >= 1) {
                // DrivingModeMsg: 0=MANUAL, 1=AI_ASSIST
                uint8_t mode = rx_data[0] > 0 ? 1 : 0;
                if (tx_mutex_get(&g_vehicle_command_mutex, TX_WAIT_FOREVER) == TX_SUCCESS) {
                    g_vehicle_command.driving_mode = mode;
                    g_vehicle_command.command_valid = 1;
                    tx_mutex_put(&g_vehicle_command_mutex);
                }
                cmd_updated = 1;

                snprintf(comm_uart_buf, sizeof(comm_uart_buf),
                        "[CMD] Mode=%s\r\n", mode ? "AI" : "MAN");
                Debug_Print(comm_uart_buf);
            }

            if (cmd_updated) {
                VehicleCommand_t cmd;
                if (tx_mutex_get(&g_vehicle_command_mutex, TX_WAIT_FOREVER) == TX_SUCCESS) {
                    cmd = g_vehicle_command;
                    cmd.current_velocity = vehicle_speed;  // Add current speed from sensor
                    tx_mutex_put(&g_vehicle_command_mutex);
                }
                if (!ControlQueue_TrySend(&cmd)) {
                    Debug_Print("[COMM] Control queue full\r\n");
                }

                last_cmd = cmd;
                last_cmd_ts = HAL_GetTick();
                have_last_cmd = 1;
            }
        }
        
        // Periodic status: every 10s print queue drops and other info, plus occupancy
        if (count % 1000 == 0) { /* 1000 * 10ms = 10s */
            char status_buf[128];
            UINT control_occ = 0;
            UINT sensors_occ = 0;
            ControlQueue_GetOccupancy(&control_occ);
            SensorsQueue_GetOccupancy(&sensors_occ);
            snprintf(status_buf, sizeof(status_buf), "[COMM] Control drops=%u occ=%u, Sensors drops=%u occ=%u\r\n", ControlQueue_GetDrops(), control_occ, SensorsQueue_GetDrops(), sensors_occ);
            Debug_Print(status_buf);
        }

        // Heartbeat: re-send last command if we haven't seen a command recently
        if (have_last_cmd) {
            uint32_t now = HAL_GetTick();
            if ((now - last_cmd_ts) > HEARTBEAT_MS) {
                last_cmd.current_velocity = vehicle_speed;  // Update with current speed
                if (ControlQueue_TrySend(&last_cmd)) {
                    /* Log heartbeat at most once per resend (HEARTBEAT_MS) to avoid flooding */
                    Debug_Print("[COMM] Heartbeat: re-sent last command to Control queue\r\n");
                } else {
                    Debug_Print("[COMM] Heartbeat: Control queue full on re-send\r\n");
                }
                last_cmd_ts = now; // update to avoid flooding
            }
        }

        // Print detailed status every 5000 iterations (50 seconds at 10ms loop)
        if (count % 5000 == 0) {
            Debug_Print("\r\n=== CAN Status Report ===\r\n");
            MCP2515_PrintDetailedStatus();
        }
        
        count++;
        tx_thread_sleep(1);  // 10ms - responsive control loop (100Hz)
    }
}
