/**
 ******************************************************************************
 * @file    communication.c
 * @brief   CAN Communication Thread - Reads global vehicle data and transmits
 * @note    This module is independent from sensors, reads from global variables
 ******************************************************************************
 */

#include "comm.h"
#include "mcp2515.h"
#include "../Sensors/sensors.h"
#include "main.h"
#include "../Control/control_queue.h"
#include <stdio.h>

#ifndef COMM_DEBUG
#define Debug_Print(msg) ((void)0)
#endif
char comm_uart_buf[128];

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
    Debug_Print("[COMM] Reading from global vehicle data\r\n\r\n");
    
    uint32_t count = 0;
    VehicleData_t local_data;

    /* Heartbeat state: re-send last command if no new command seen for HEARTBEAT_MS */
    const uint32_t HEARTBEAT_MS = 1000; /* 1s heartbeat to avoid log flooding */
    /* Default safe command: throttle=0, steering=0, command_valid=1
     * This prevents repeated safety stop messages until a real command is received.
     */
    VehicleCommand_t last_cmd = { .driving_mode = 0, .throttle = 0, .steering_angle = 0, .command_valid = 1 };
    uint32_t last_cmd_ts = HAL_GetTick();
    int have_last_cmd = 1; /* start with default command enabled */

    Debug_Print("[COMM] Heartbeat initialized with default SAFE command\r\n");

    while(1) {
        // Send status every 200ms (every 20 loops) for faster response
        if (count % 20 == 0) {
            // Read global vehicle data with mutex protection
            if (tx_mutex_get(&g_vehicle_data_mutex, TX_WAIT_FOREVER) == TX_SUCCESS) {
                // Copy data locally to minimize mutex hold time
                local_data = g_vehicle_data;
                tx_mutex_put(&g_vehicle_data_mutex);
            }
            
            // Check if data is valid before sending
            if (1) {
                // Send battery percentage over CAN (ID: 0x4D)
                uint8_t battery = (uint8_t)local_data.battery_percentage;
                HAL_StatusTypeDef status = MCP2515_SendBattery(battery);
                // HAL_StatusTypeDef status = MCP2515_SendBattery(90);
            
                const char* status_str = (status == HAL_OK) ? "OK" : 
                                         (status == HAL_BUSY) ? "BUSY" : 
                                         (status == HAL_TIMEOUT) ? "TIMEOUT" : "ERROR";
                
                int voltage_int = (int)local_data.battery_voltage;
                int voltage_frac = (int)((local_data.battery_voltage - voltage_int) * 100);
                
                snprintf(comm_uart_buf, sizeof(comm_uart_buf), 
                        "[CAN_TX] Battery: %d.%02dV (%u%%) | %s\r\n",
                        voltage_int, voltage_frac, battery, status_str);
                Debug_Print(comm_uart_buf);
                
                // Send speed over CAN (ID: 66/0x42 - expected by cluster)
                HAL_StatusTypeDef speed_status = MCP2515_SendSpeed(local_data.vehicle_speed);
                const char* speed_status_str = (speed_status == HAL_OK) ? "OK" : 
                                               (speed_status == HAL_BUSY) ? "BUSY" : 
                                               (speed_status == HAL_TIMEOUT) ? "TIMEOUT" : "ERROR";
                
                snprintf(comm_uart_buf, sizeof(comm_uart_buf), 
                        "[CAN_TX] Speed: %.2f m/s (%.1f dm/s) | %s\r\n",
                        local_data.vehicle_speed, local_data.vehicle_speed * 10.0f, speed_status_str);
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
        uint16_t rx_can_id;
        uint8_t rx_data[8];
        uint8_t rx_length;
        
        while (MCP2515_ReceiveMessage(&rx_can_id, rx_data, &rx_length)) {
            Debug_Print("\r\n[CAN_RX] *** MESSAGE RECEIVED! ***\r\n");
            
            // Show raw data
            snprintf(comm_uart_buf, sizeof(comm_uart_buf), 
                    "[RX] ID=0x%03X, DLC=%d, Data: ", rx_can_id, rx_length);
            Debug_Print(comm_uart_buf);
            for (int i = 0; i < rx_length; i++) {
                snprintf(comm_uart_buf, sizeof(comm_uart_buf), "0x%02X ", rx_data[i]);
                Debug_Print(comm_uart_buf);
            }
            Debug_Print("\r\n");
            
            // Process command message (accept both 0x100 and 0x200)
            if ((rx_can_id == 0x100 || rx_can_id == 0x200) && rx_length >= 3) {
                // Parse command: byte0=always 0, byte1=throttle (0-100), byte2=steering (-1, 0, 1)
                uint8_t mode = rx_data[0];
                uint8_t throttle = rx_data[1];
                int8_t steering_discrete = (int8_t)rx_data[2];  // -1, 0, or 1

                // Convert discrete steering to -100 to +100 range
                int8_t steering_angle = steering_discrete * 100;

                // Create command message
                VehicleCommand_t cmd;
                cmd.driving_mode = mode;
                cmd.throttle = throttle;
                cmd.steering_angle = steering_angle;
                cmd.command_valid = 1;

                // Enqueue to control queue (non-blocking)
                if (!ControlQueue_TrySend(&cmd)) {
                    Debug_Print("[COMM] Control queue full - command dropped\r\n");
                } else {
                    Debug_Print("[COMM] Command enqueued to Control queue\r\n");
                }

                // Keep global copy for diagnostics/backwards compatibility
                if (tx_mutex_get(&g_vehicle_command_mutex, TX_WAIT_FOREVER) == TX_SUCCESS) {
                    g_vehicle_command = cmd;
                    tx_mutex_put(&g_vehicle_command_mutex);
                }

                // Update last command for heartbeat
                last_cmd = cmd;
                last_cmd_ts = HAL_GetTick();
                have_last_cmd = 1;

                // Convert steering to float for display
                float steering_float = (float)steering_angle / 100.0f;
                int steering_int = (int)(steering_float * 1000);  // For display: -1000 to +1000

                snprintf(comm_uart_buf, sizeof(comm_uart_buf), 
                        "[CMD] Mode=%d | Throttle=%d%% | Steering=%d.%03d\r\n",
                        mode, throttle, steering_int/1000, abs(steering_int%1000));
                Debug_Print(comm_uart_buf);
            } else {
                // Unknown message ID!
                Debug_Print("[RX] Unknown message ID (expecting 0x100 or 0x200)\r\n");
            }
        }
        
        // Periodic status: every 5s print queue drops and other info, plus occupancy
        if (count % 500 == 0) { /* 500 * 10ms = 5s */
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
