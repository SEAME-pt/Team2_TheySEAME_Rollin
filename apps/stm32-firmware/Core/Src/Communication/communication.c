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
#include "../Sensors/sensors_queue.h"
#include <stdio.h>
#include <string.h>

#ifdef COMM_DEBUG
extern void Debug_Print(const char *msg);
#else
#define Debug_Print(msg) ((void)0)
#endif
char comm_uart_buf[128];
char rx_uart_buf[128];
extern UART_HandleTypeDef huart1;

/* Always-on print for RX messages (bypasses COMM_DEBUG gate) */
static inline void RX_Print(const char *msg) {
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), 100);
}

/* Configuration constants */
enum {
    COMM_LOOP_SLEEP_TICKS = 1, /* 10ms per loop */
    STATUS_TX_INTERVAL_LOOPS = 100, /* 100 * 10ms = 1s */
    STATUS_LONG_INTERVAL_LOOPS = 1000, /* 10s */
    DETAILED_STATUS_INTERVAL_LOOPS = 5000, /* 50s */
};

/* CAN message IDs */
enum CAN_IDs {
    CAN_ID_CONTROLLER_COMBINED = 0x100,
    CAN_ID_THROTTLE            = 0x100,
    CAN_ID_GEAR                = 0x101,
    CAN_ID_GEAR_DBC            = 0x105,
    CAN_ID_STEERING            = 0x102,
    CAN_ID_BRAKE               = 0x103,
    CAN_ID_BRAKE_DBC           = 0x106,
    CAN_ID_DRIVING_MODE        = 0x104,
    CAN_ID_TX_SPEED            = 0x200,
    CAN_ID_TX_BATTERY          = 0x201,
    CAN_ID_CRUISE_CONTROL      = 0x212,
};

static const uint32_t HEARTBEAT_MS = 1000; /* resend every 1s if no command seen */

/* Bus-off recovery counter (reset on successful TX) */
static uint32_t busoff_count = 0;

static uint8_t clamp_u8(uint16_t value, uint8_t max_value) {
    return (uint8_t)((value > max_value) ? max_value : value);
}

static int8_t clamp_i8(int16_t value, int8_t min_value, int8_t max_value) {
    if (value < min_value) return min_value;
    if (value > max_value) return max_value;
    return (int8_t)value;
}

/* Helper: safely snapshot global vehicle data with mutex protection */
static int snapshot_vehicle_data(VehicleData_t *out) {
    if (tx_mutex_get(&g_vehicle_data_mutex, TX_WAIT_FOREVER) == TX_SUCCESS) {
        *out = g_vehicle_data;
        tx_mutex_put(&g_vehicle_data_mutex);
        return 1;  // Success
    }
    Debug_Print("[COMM] Failed to acquire vehicle data mutex\r\n");
    return 0;  // Failure
}

/* Send battery and speed messages over CAN and print a concise summary */
static void send_battery_and_speed(const VehicleData_t *snapshot) {
    // Add validation check
    if (!snapshot->data_valid) {
        Debug_Print("[COMM] Warning: Sending invalid/stale vehicle data\r\n");
    }
    
    uint8_t battery = (uint8_t)snapshot->battery_percentage;
    HAL_StatusTypeDef bat_status = MCP2515_SendBattery(battery);
    HAL_StatusTypeDef speed_status = MCP2515_SendSpeed(snapshot->vehicle_speed);

    uint16_t speed_hmh = (uint16_t)(snapshot->vehicle_speed * 36.0f);
    snprintf(comm_uart_buf, sizeof(comm_uart_buf),
            "[TX] 0x200 Speed=%u hm/h (raw=%.2f m/s) (%s) | 0x201 Bat=%u%% (%s)\r\n",
            speed_hmh,
            snapshot->vehicle_speed,  // Add raw value for debugging
            (speed_status == HAL_OK) ? "OK" : (speed_status == HAL_BUSY) ? "BUSY" : "FAIL",
            battery,
            (bat_status == HAL_OK) ? "OK" : (bat_status == HAL_BUSY) ? "BUSY" : "FAIL");
    Debug_Print(comm_uart_buf);

    /* Try to recover from repeated bus-off by reinitializing MCP2515 */
    if (bat_status == HAL_TIMEOUT || speed_status == HAL_TIMEOUT) {
        busoff_count++;
        if (busoff_count >= 3) {
            Debug_Print("[COMM] Resetting MCP2515 due to bus-off...\r\n");
            MCP2515_Init(CAN_SPEED_500KBPS);
            busoff_count = 0;
        }
    } else {
        busoff_count = 0; /* Reset on successful transmission */
    }
}

/* Enqueue command to Control queue and log failure if full */
static void enqueue_command_or_log(const VehicleCommand_t *cmd) {
    if (!ControlQueue_TrySend(cmd)) {
        Debug_Print("[COMM] Control queue full\r\n");
    }
}

/* Convert raw RX frame into updates to g_vehicle_command and return whether updated */
static int handle_rx_frame(uint32_t can_id, const uint8_t *data, uint8_t dlc) {
    int updated = 0;
    /* Handle CAN ID 0x100: Controller combined (DLC=3) or Throttle-only (DLC=1) */
    // if (can_id == CAN_ID_CONTROLLER_COMBINED) {
    //     if (dlc == 3) {
    //         /* Controller combined: [mode, throttle, steering(-1/0/1)] */
    //         uint8_t mode = data[0] > 0 ? 1 : 0;
    //         uint8_t throttle = data[1] > 100 ? 100 : data[1];
    //         int8_t steering = ((int8_t)data[2]) * 100;

    //         if (tx_mutex_get(&g_vehicle_command_mutex, TX_WAIT_FOREVER) == TX_SUCCESS) {
    //             g_vehicle_command.driving_mode = mode;
    //             g_vehicle_command.gear = 3; /* default to Drive when controller omits gear */
    //             g_vehicle_command.throttle = throttle;
    //             g_vehicle_command.steering_angle = steering;
    //             g_vehicle_command.command_valid = 1;
    //             g_vehicle_command.cruise_control_enabled = false;
    //             tx_mutex_put(&g_vehicle_command_mutex);
    //         }

    //         snprintf(comm_uart_buf, sizeof(comm_uart_buf),
    //                 "[CMD] Controller: Mode=%s Throttle=%d%% Steering=%d\r\n",
    //                 mode ? "AI" : "MAN", throttle, steering);
    //         Debug_Print(comm_uart_buf);
    //         updated = 1;
    //     } else if (dlc >= 1) {
    //         /* Throttle-only message: supports 1-byte or 16-bit little-endian payload */
    //         uint16_t throttle_raw = (dlc >= 2)
    //             ? (uint16_t)data[0] | ((uint16_t)data[1] << 8)
    //             : (uint16_t)data[0];
    //         uint8_t throttle = clamp_u8(throttle_raw, 100);
    //         if (tx_mutex_get(&g_vehicle_command_mutex, TX_WAIT_FOREVER) == TX_SUCCESS) {
    //             g_vehicle_command.throttle = throttle;
    //             g_vehicle_command.command_valid = 1;
    //             g_vehicle_command.cruise_control_enabled = false;
    //             tx_mutex_put(&g_vehicle_command_mutex);
    //         }
    //         snprintf(comm_uart_buf, sizeof(comm_uart_buf), "[CMD] Throttle=%d%%\r\n", throttle);
    //         Debug_Print(comm_uart_buf);
    //         updated = 1;
    //     }
    //     return updated;
    // }

    /* Kuksa-style / per-message handling */
    switch (can_id) {
        case CAN_ID_THROTTLE: /* Legacy Throttle ID (0x100) */
            if (dlc >= 1) {
                uint8_t throttle = data[0] > 100 ? 100 : data[0];
                if (tx_mutex_get(&g_vehicle_command_mutex, TX_WAIT_FOREVER) == TX_SUCCESS) {
                    g_vehicle_command.throttle = throttle;
                    g_vehicle_command.command_valid = 1;
                    if (g_vehicle_command.cruise_control_enabled) {
                        g_vehicle_command.cruise_control_enabled = false; /* Disable cruise control on manual throttle input */
                    }
                    tx_mutex_put(&g_vehicle_command_mutex);
                }
                snprintf(comm_uart_buf, sizeof(comm_uart_buf), "[CMD] Throttle=%d%%\r\n", throttle);
                Debug_Print(comm_uart_buf);
                updated = 1;
            }
            break;
        case CAN_ID_GEAR:     /* Legacy Gear ID (0x101) */
        case CAN_ID_GEAR_DBC: /* DBC Gear ID (0x105) */
            if (dlc >= 1) {
                uint8_t gear = data[0];
                if (tx_mutex_get(&g_vehicle_command_mutex, TX_WAIT_FOREVER) == TX_SUCCESS) {
                    g_vehicle_command.gear = gear;
                    g_vehicle_command.command_valid = 1;
                    tx_mutex_put(&g_vehicle_command_mutex);
                }
                const char* gear_names[] = {"P","N","R","D"};
                snprintf(comm_uart_buf, sizeof(comm_uart_buf), "[CMD] Gear=%s\r\n", gear <= 3 ? gear_names[gear] : "?");
                Debug_Print(comm_uart_buf);
                updated = 1;
            }
            break;

        case CAN_ID_STEERING: /* Steering: supports controller and DBC payload styles */
            if (dlc == 1) {
                int8_t raw = (int8_t)data[0];
                int8_t steering = raw;
                if (tx_mutex_get(&g_vehicle_command_mutex, TX_WAIT_FOREVER) == TX_SUCCESS) {
                    g_vehicle_command.steering_angle = steering;
                    g_vehicle_command.command_valid = 1;
                    tx_mutex_put(&g_vehicle_command_mutex);
                }
                snprintf(comm_uart_buf, sizeof(comm_uart_buf), "[CMD] Steering=%d (raw=%d)\r\n", steering, raw);
                Debug_Print(comm_uart_buf);
                updated = 1;
            } else if (dlc >= 2) {
                int16_t raw16 = (int16_t)((uint16_t)data[0] | ((uint16_t)data[1] << 8));
                int16_t steering_scaled = raw16;

                /* DBC angle uses [-30..30] degrees; map to control scale [-100..100]. */
                if (raw16 >= -30 && raw16 <= 30) {
                    steering_scaled = (raw16 * 100) / 30;
                }

                int8_t steering = clamp_i8(steering_scaled, -100, 100);
                if (tx_mutex_get(&g_vehicle_command_mutex, TX_WAIT_FOREVER) == TX_SUCCESS) {
                    g_vehicle_command.steering_angle = steering;
                    g_vehicle_command.command_valid = 1;
                    tx_mutex_put(&g_vehicle_command_mutex);
                }
                snprintf(comm_uart_buf, sizeof(comm_uart_buf),
                         "[CMD] Steering=%d%% (raw16=%d)\r\n", steering, raw16);
                Debug_Print(comm_uart_buf);
                updated = 1;
            }
            break;

        case CAN_ID_BRAKE:     /* Legacy Brake ID (0x103) */
            if (dlc >= 1) {
                uint8_t brake_raw = data[0];
                bool brake = brake_raw > 0;
                if (tx_mutex_get(&g_vehicle_command_mutex, TX_WAIT_FOREVER) == TX_SUCCESS) {
                    g_vehicle_command.brake = brake;
                    g_vehicle_command.command_valid = 1;
                    if (brake) {
                        g_vehicle_command.cruise_control_enabled = false; /* Disable cruise control on brake */
                    }
                    tx_mutex_put(&g_vehicle_command_mutex);
                }
                snprintf(comm_uart_buf, sizeof(comm_uart_buf), "[CMD] Brake=%s\r\n", brake ? "ON" : "OFF");
                Debug_Print(comm_uart_buf);
                updated = 1;
            }
            break;

        case CAN_ID_DRIVING_MODE: /* Driving mode */
            if (dlc >= 1) {
                uint8_t mode = data[0] > 0 ? 1 : 0;
                if (tx_mutex_get(&g_vehicle_command_mutex, TX_WAIT_FOREVER) == TX_SUCCESS) {
                    g_vehicle_command.driving_mode = mode;
                    g_vehicle_command.command_valid = 1;
                    tx_mutex_put(&g_vehicle_command_mutex);
                }
                snprintf(comm_uart_buf, sizeof(comm_uart_buf), "[CMD] Mode=%s\r\n", mode ? "AI" : "MAN");
                Debug_Print(comm_uart_buf);
                updated = 1;
            }
            break;
            
        case CAN_ID_CRUISE_CONTROL: /* Cruise control: byte0=enabled(0/1), byte1=target speed in hm/h */
            if (dlc >= 2) {
                uint8_t enabled = data[0] > 0 ? 1 : 0;
                uint8_t target_speed = data[1];
                if (tx_mutex_get(&g_vehicle_command_mutex, TX_WAIT_FOREVER) == TX_SUCCESS) {
                    if (enabled) {
                        g_vehicle_command.cruise_control_enabled = true; /* Force AI_ASSIST mode when cruise control is enabled */
                    }
                    g_vehicle_command.cruise_control_target_speed = target_speed;
                    g_vehicle_command.command_valid = 1;
                    tx_mutex_put(&g_vehicle_command_mutex);
                }
                snprintf(comm_uart_buf, sizeof(comm_uart_buf), "[CMD] Cruise Control: %s, Target=%u hm/h\r\n",
                         enabled ? "ENABLED" : "DISABLED", target_speed);
                Debug_Print(comm_uart_buf);
                updated = 1;
            }
            break;
        default:
            /* Unknown CAN ID - ignore */
            break;
    }

    return updated;
}

/* Drain all pending CAN messages and process them */
static void drain_and_process_can_messages(VehicleCommand_t *out_last_cmd, uint32_t *out_last_ts, int *out_have_last) {
    uint32_t rx_can_id;
    uint8_t rx_data[8];
    uint8_t rx_length;

    while (MCP2515_ReceiveMessage(&rx_can_id, rx_data, &rx_length)) {
        /* Print raw frame */
        snprintf(rx_uart_buf, sizeof(rx_uart_buf), "[RX] ID=0x%08lX DLC=%d Data=", (unsigned long)rx_can_id, rx_length);
        RX_Print(rx_uart_buf);
        for (int i = 0; i < rx_length; i++) {
            snprintf(rx_uart_buf, sizeof(rx_uart_buf), "%02X ", rx_data[i]);
            RX_Print(rx_uart_buf);
        }
        RX_Print("\r\n");

        int updated = handle_rx_frame(rx_can_id, rx_data, rx_length);
        if (!updated) continue;

        VehicleCommand_t snapshot_cmd = {0};
        if (tx_mutex_get(&g_vehicle_command_mutex, TX_WAIT_FOREVER) == TX_SUCCESS) {
            snapshot_cmd = g_vehicle_command;
            tx_mutex_put(&g_vehicle_command_mutex);
        } else {
            Debug_Print("[COMM] Failed to acquire mutex for command snapshot\r\n");
            continue;
        }
        enqueue_command_or_log(&snapshot_cmd);

        *out_last_cmd = snapshot_cmd;
        *out_last_ts = HAL_GetTick();
        *out_have_last = 1;
    }
}

void Communication_Thread_Entry(ULONG thread_input) {
    (void) thread_input;

    Debug_Print("\r\n\r\n=== COMMUNICATION THREAD STARTED ===\r\n");
    Debug_Print("[COMM] Initializing MCP2515 CAN controller...\r\n");

    tx_thread_sleep(50); /* 500ms: let other threads initialize */

    MCP2515_TestConnection();

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
    int have_last_cmd = 1;

    Debug_Print("[COMM] Heartbeat initialized with default SAFE command\r\n");

   while (1) {
        if ((loop_counter % STATUS_TX_INTERVAL_LOOPS) == 0) {
            if (snapshot_vehicle_data(&snapshot)) {
                send_battery_and_speed(&snapshot);
                MCP2515_SendMessage(531, (uint8_t*)&snapshot.cruise_control_active, sizeof(uint8_t));
            } else {
                Debug_Print("[COMM] Skipping TX - no valid data\r\n");
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
                Debug_Print("[COMM] Skipping TX - no valid data\r\n");
            }
        }

            if (rx_can_id == 0x100 && rx_length == 3) {
                // Legacy Controller combined frame: [mode, throttle, steering]
                // NOTE: Deprecated - use individual messages (0x104, 0x212, 0x102)
                uint8_t mode = rx_data[0] > 0 ? 1 : 0;
                uint8_t throttle = rx_data[1] > 100 ? 100 : rx_data[1];
                // steering: RPi sends integer division result (-1, 0, 1), scale to -100..100
                int8_t steering_raw = (int8_t)rx_data[2];
                int8_t steering = steering_raw * 100; // -1→-100, 0→0, 1→100

                // Approximate velocity from throttle percentage (legacy fallback)
                float approx_velocity = (throttle / 100.0f) * 2.0f; // Max 2.0 m/s

                if (tx_mutex_get(&g_vehicle_command_mutex, TX_WAIT_FOREVER) == TX_SUCCESS) {
                    g_vehicle_command.driving_mode = mode;
                    g_vehicle_command.gear = 3;  // Controller doesn't send gear, default to Drive
                    g_vehicle_command.desired_velocity = approx_velocity;
                    g_vehicle_command.steering_angle = steering;
                    g_vehicle_command.command_valid = 1;
                    tx_mutex_put(&g_vehicle_command_mutex);
                }
                cmd_updated = 1;

                snprintf(comm_uart_buf, sizeof(comm_uart_buf),
                        "[CMD] Legacy: Mode=%s Throttle=%d%% (~%.2fm/s) Steering=%d\r\n",
                        mode ? "AI" : "MAN", throttle, approx_velocity, steering);
                Debug_Print(comm_uart_buf);
            }
            else if (rx_can_id == 0x100 && rx_length >= 1) {
                // ThrottleMsg (0x100): Legacy throttle percentage
                // NOTE: Prefer using CruiseControl/ACC messages for velocity control
                uint8_t throttle = rx_data[0] > 100 ? 100 : rx_data[0];
                float approx_velocity = (throttle / 100.0f) * 2.0f; // Max 2.0 m/s
                
                if (tx_mutex_get(&g_vehicle_command_mutex, TX_WAIT_FOREVER) == TX_SUCCESS) {
                    g_vehicle_command.desired_velocity = approx_velocity;
                    g_vehicle_command.command_valid = 1;
                    tx_mutex_put(&g_vehicle_command_mutex);
                }
                cmd_updated = 1;

                snprintf(comm_uart_buf, sizeof(comm_uart_buf),
                        "[CMD] Throttle=%d%% (~%.2fm/s)\r\n", throttle, approx_velocity);
                Debug_Print(comm_uart_buf);
            }
            else if (rx_can_id == 0x101 && rx_length >= 1) {
                // BrakeMsg (0x101): 0=OFF, 1=ON (logged only, no struct field)
                uint8_t brake = rx_data[0];
                
                snprintf(comm_uart_buf, sizeof(comm_uart_buf),
                        "[CMD] Brake=%s\r\n", brake ? "ON" : "OFF");
                Debug_Print(comm_uart_buf);
            }
            else if (rx_can_id == 0x102 && rx_length >= 1) {
                // AngleMsg (0x102): Steering angle -30 to 30 degrees
                int8_t angle_deg = (int8_t)rx_data[0]; // -30 to +30
                
                // Clamp to valid range
                if (angle_deg < -30) angle_deg = -30;
                if (angle_deg > 30) angle_deg = 30;
                
                // Convert to normalized range: -30deg → -100, 0deg → 0, +30deg → +100
                int8_t steering = (int8_t)((angle_deg * 100) / 30);
                
                if (tx_mutex_get(&g_vehicle_command_mutex, TX_WAIT_FOREVER) == TX_SUCCESS) {
                    g_vehicle_command.steering_angle = steering;
                    g_vehicle_command.command_valid = 1;
                    tx_mutex_put(&g_vehicle_command_mutex);
                }
                cmd_updated = 1;

                snprintf(comm_uart_buf, sizeof(comm_uart_buf),
                        "[CMD] Steering=%d deg (norm=%d)\r\n", angle_deg, steering);
                Debug_Print(comm_uart_buf);
            }
            else if (rx_can_id == 0x104 && rx_length >= 1) {
                // DrivingModeMsg (0x104): 0=MANUAL, 1=AI_ASSIST, 2=AUTONOMOUS
                uint8_t mode = rx_data[0];
                if (mode > 2) mode = 0; // Clamp to valid range
                
                if (tx_mutex_get(&g_vehicle_command_mutex, TX_WAIT_FOREVER) == TX_SUCCESS) {
                    g_vehicle_command.driving_mode = mode;
                    g_vehicle_command.command_valid = 1;
                    tx_mutex_put(&g_vehicle_command_mutex);
                }
                cmd_updated = 1;

                const char* mode_names[] = {"MANUAL", "AI_ASSIST", "AUTONOMOUS"};
                snprintf(comm_uart_buf, sizeof(comm_uart_buf),
                        "[CMD] Mode=%s\r\n", mode_names[mode]);
                Debug_Print(comm_uart_buf);
            }
            else if (rx_can_id == 0x105 && rx_length >= 1) {
                // GearMsg (0x105): 0=PARK, 1=NEUTRAL, 2=REVERSE, 3=DRIVE
                uint8_t gear = rx_data[0];
                if (tx_mutex_get(&g_vehicle_command_mutex, TX_WAIT_FOREVER) == TX_SUCCESS) {
                    g_vehicle_command.gear = gear;
                    g_vehicle_command.command_valid = 1;
                    tx_mutex_put(&g_vehicle_command_mutex);
                }
                cmd_updated = 1;

                const char* gear_names[] = {"PARK", "NEUTRAL", "REVERSE", "DRIVE"};
                snprintf(comm_uart_buf, sizeof(comm_uart_buf),
                        "[CMD] Gear=%s\r\n", gear <= 3 ? gear_names[gear] : "?");
                Debug_Print(comm_uart_buf);
            }
            else if (rx_can_id == 0x212 && rx_length >= 2) {
                // CruiseControlCmdMsg (0x212): CC_Enabled (bool), CC_TargetSpeed (0-250 hm/h)
                uint8_t cc_enabled = rx_data[0];
                uint8_t target_speed_hm = rx_data[1]; // 0-250 hm/h
                float target_velocity_ms = HmPerH_ToMetersPerSec(target_speed_hm);
                
                if (tx_mutex_get(&g_vehicle_command_mutex, TX_WAIT_FOREVER) == TX_SUCCESS) {
                    if (cc_enabled) {
                        g_vehicle_command.desired_velocity = target_velocity_ms;
                        g_vehicle_command.command_valid = 1;
                    }
                    tx_mutex_put(&g_vehicle_command_mutex);
                }
                cmd_updated = cc_enabled; // Only update command if CC is enabled

                snprintf(comm_uart_buf, sizeof(comm_uart_buf),
                        "[CMD] CruiseControl: %s, Target=%u hm/h (%.2fm/s)\r\n",
                        cc_enabled ? "ENABLED" : "DISABLED", target_speed_hm, target_velocity_ms);
                Debug_Print(comm_uart_buf);
            }
            else if (rx_can_id == 0x214 && rx_length >= 3) {
                // ACCCmdMsg (0x214): ACC_Enabled (bool), ACC_TargetSpeed (0-250 hm/h), ACC_TimeGap (0-10s)
                uint8_t acc_enabled = rx_data[0];
                uint8_t target_speed_hm = rx_data[1]; // 0-250 hm/h
                uint8_t time_gap = rx_data[2]; // 0-10 seconds
                float target_velocity_ms = HmPerH_ToMetersPerSec(target_speed_hm);
                
                if (tx_mutex_get(&g_vehicle_command_mutex, TX_WAIT_FOREVER) == TX_SUCCESS) {
                    if (acc_enabled) {
                        g_vehicle_command.desired_velocity = target_velocity_ms;
                        g_vehicle_command.command_valid = 1;
                    }
                    tx_mutex_put(&g_vehicle_command_mutex);
                }
                cmd_updated = acc_enabled; // Only update command if ACC is enabled

                snprintf(comm_uart_buf, sizeof(comm_uart_buf),
                        "[CMD] ACC: %s, Target=%u hm/h (%.2fm/s), Gap=%us\r\n",
                        acc_enabled ? "ENABLED" : "DISABLED", target_speed_hm, target_velocity_ms, time_gap);
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
            snprintf(status_buf, sizeof(status_buf), "[COMM] Control drops=%u occ=%u, Sensors drops=%u occ=%u\r\n",
                     ControlQueue_GetDrops(), control_occ, SensorsQueue_GetDrops(), sensors_occ);
            Debug_Print(status_buf);
        }

        /* Heartbeat: resend last command every HEARTBEAT_MS if nothing new */
        if (have_last_cmd) {
            uint32_t now = HAL_GetTick();
            if ((now - last_cmd_ts) > HEARTBEAT_MS) {
                last_cmd.current_velocity = vehicle_speed;  // Update with current speed
                if (ControlQueue_TrySend(&last_cmd)) {
                    Debug_Print("[COMM] Heartbeat: re-sent last command to Control queue\r\n");
                } else {
                    Debug_Print("[COMM] Heartbeat: Control queue full on re-send\r\n");
                }
                last_cmd_ts = now;
            }
        }

        if ((loop_counter % DETAILED_STATUS_INTERVAL_LOOPS) == 0) {
            Debug_Print("\r\n=== CAN Status Report ===\r\n");
            MCP2515_PrintDetailedStatus();
        }

        loop_counter++;
        tx_thread_sleep(COMM_LOOP_SLEEP_TICKS);
    }
}