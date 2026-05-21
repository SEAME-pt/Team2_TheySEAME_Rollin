#include "control.h"
#include "../Drivers/pca9685.h"
#include "../Sensors/sensors.h"
#include "control_queue.h"
#include "../Sensors/sensors_queue.h"
#include "PID.h"
#include <stdio.h>

extern I2C_HandleTypeDef hi2c1;
char control_uart_buf[128];

typedef enum {
    TRAFFIC_SIGN_UNKNOWN = 0,
    TRAFFIC_SIGN_STOP = 1,
    TRAFFIC_SIGN_SPEED_LIMIT_30 = 2,
    TRAFFIC_SIGN_SPEED_LIMIT_50 = 3,
    TRAFFIC_SIGN_SPEED_LIMIT_100 = 4,
    TRAFFIC_SIGN_SPEED_LIMIT_80 = 5,
    TRAFFIC_SIGN_SPEED_LIMIT_120 = 6,
    TRAFFIC_SIGN_YIELD = 7,
    TRAFFIC_SIGN_NO_ENTRY = 8,
    TRAFFIC_SIGN_TURN_LEFT = 9,
    TRAFFIC_SIGN_TURN_RIGHT = 10,
    TRAFFIC_SIGN_PEDESTRIAN = 11,
    TRAFFIC_SIGN_TRAFFIC_LIGHT = 12,
    TRAFFIC_SIGN_ONE_WAY = 13,
    TRAFFIC_SIGN_NO_PARKING = 14,
    TRAFFIC_SIGN_NO_OVERTAKING = 15
} TrafficSignType;

static uint8_t Control_ApplyTrafficSignThrottle(uint8_t throttle_percent, int traffic_sign) {
    if (traffic_sign == TRAFFIC_SIGN_SPEED_LIMIT_50) {
        return (uint8_t)(((uint16_t)throttle_percent * 75U) / 100U);
    }
    return throttle_percent;
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

void PCA9685_SetServoAngle(uint8_t channel, float angle) {
    // Implementation note: API documented in `control.h` (Doxygen comments live in header)

    // Clamp angle to safe range
    if (angle < -30.0f) angle = -30.0f;
    if (angle > 30.0f) angle = 30.0f;

    // Convert angle to pulse width (1ms to 2ms)
    float pulse_center = 307.0f;
    float counts_per_degree = 2.27f;

    uint16_t pulse_width = (uint16_t)(pulse_center + (angle * counts_per_degree));

    // Set PWM: ON at 0, OFF at calculated pulse width
    PCA9685_SetPWM(&hi2c1, PCA9685_ADDR_STEERING, channel, 0, pulse_width);
}

void Control_SetSteering(float steering_normalized) {
    float angle = steering_normalized;
    PCA9685_SetServoAngle(0, angle);  // Channel 0 for steering servo
}

void Control_SetThrottle(uint8_t throttle_percent, uint8_t gear, bool brake) {
    
    // Clamp throttle to 0-100%
    if (throttle_percent > 100) throttle_percent = 100;
    
    // Convert percentage to PWM value (0-MAX_THROTTLE_PWM)
    uint16_t speed_pwm = (uint16_t)((throttle_percent * MAX_THROTTLE_PWM) / 100);
    uint16_t dir_high = 0;      // Swapped base values to match motor wiring
    uint16_t dir_low = MAX_THROTTLE_PWM;
    
    // Reverse direction if gear is 2 (R=Reverse)
    if (gear == 2) {
        uint16_t temp = dir_high;
        dir_high = dir_low;
        dir_low = temp;
    }
    if (brake) {
        Control_StopMotors();
        return;
    }
    char speed_pwm_buf[64];
    snprintf(speed_pwm_buf, sizeof(speed_pwm_buf), "[CONTROL] Throttle PWM=%u (for %u%% throttle) \r\n", speed_pwm, throttle_percent);
    // Debug_Print(speed_pwm_buf); 
    if (throttle_percent > 0 && gear != 0 && gear != 1) { // Don't move if P or N
        // Motor 1 (channels 0,1,2,3)
        PCA9685_SetPWM(&hi2c1, PCA9685_ADDR_THROTTLE, 0, 0, speed_pwm);  // M1 speed
        PCA9685_SetPWM(&hi2c1, PCA9685_ADDR_THROTTLE, 1, 0, dir_high);   // M1 DIR1
        PCA9685_SetPWM(&hi2c1, PCA9685_ADDR_THROTTLE, 2, 0, dir_low);    // M1 DIR2
        PCA9685_SetPWM(&hi2c1, PCA9685_ADDR_THROTTLE, 3, 0, 0);          // Unused

        // Motor 2 (channels 4,5,6,7)
        PCA9685_SetPWM(&hi2c1, PCA9685_ADDR_THROTTLE, 4, 0, speed_pwm);  // M2 speed
        PCA9685_SetPWM(&hi2c1, PCA9685_ADDR_THROTTLE, 5, 0, dir_low);    // M2 DIR2
        PCA9685_SetPWM(&hi2c1, PCA9685_ADDR_THROTTLE, 6, 0, dir_high);   // M2 DIR1
        PCA9685_SetPWM(&hi2c1, PCA9685_ADDR_THROTTLE, 7, 0, speed_pwm);  // M2 speed
    } else {
        Control_StopMotors();
    }
}

void Control_StopMotors(void) {
    // Set all motor channels to 0
    for (uint8_t ch = 0; ch < 8; ch++) {
        PCA9685_SetPWM(&hi2c1, PCA9685_ADDR_THROTTLE, ch, 0, MAX_THROTTLE_PWM);
    }
}



void Control_Thread_Entry(ULONG thread_input) {
    (void)thread_input;
    
    Debug_Print("[CONTROL] Initializing both PCA9685 devices...\r\n");
    
    // Initialize both PCA9685 devices with single software reset
    HAL_StatusTypeDef pca_status = PCA9685_Init_Multiple(&hi2c1, 
                                                        PCA9685_ADDR_STEERING, "Steering",
                                                        PCA9685_ADDR_THROTTLE, "Throttle");
    if (pca_status != HAL_OK) {
        snprintf(control_uart_buf, sizeof(control_uart_buf), "[CONTROL] PCA9685 Multiple Init FAILED: %d\r\n", pca_status);
        Debug_Print(control_uart_buf);
    } else {
        Debug_Print("[CONTROL] Both PCA9685 devices initialized successfully\r\n");
    }
    
    // Wait for initialization
    tx_thread_sleep(100);  // 1 second
    
    VehicleCommand_t local_cmd;
    memset(&local_cmd, 0, sizeof(local_cmd));
    uint8_t last_mode = 0xFF;
    uint8_t last_brake = 0xFF;
    uint8_t last_gear = 0xFF;
    uint8_t last_throttle = 0xFF;
    int last_traffic_sign = -1;
    int8_t last_steering = 0x7F;
    uint8_t last_aeb_enabled = 0xFF;

    const ULONG cmd_wait_ticks = 10; // 100ms wait for command before timeout handling
    ULONG no_cmd_ticks = 0;
    const ULONG no_cmd_threshold = 10; // 10 * 100ms = 1s without command -> stop motors
    const float tx_tick_s = 1.0f / (float)TX_TIMER_TICKS_PER_SECOND;
    ULONG last_cc_tick = 0;

    /* Rate limit safety prints so operator can read them (ms) */
    const uint32_t SAFETY_PRINT_MS = 5000; // 5 seconds
    uint32_t last_safety_print_ts = 0;

    while(1) {
        VehicleCommand_t recv;
        VehicleData_t recvs;
        UINT r = ControlQueue_Receive(&recv, cmd_wait_ticks);
        float current_speed = 0.0f;
        bool active_cruise_control = false;
        ULONG now_cc_tick = tx_time_get();
        ULONG dt_ticks = (last_cc_tick == 0U) ? cmd_wait_ticks : (now_cc_tick - last_cc_tick);
        float cc_dt = (float)dt_ticks * tx_tick_s;
        last_cc_tick = now_cc_tick;
        if (snapshot_vehicle_data(&recvs))
            current_speed = recvs.vehicle_speed;
        if (r == TX_SUCCESS) {
            local_cmd = recv;

            // Update global copy for diagnostics/backwards compatibility
            if (tx_mutex_get(&g_vehicle_command_mutex, TX_WAIT_FOREVER) == TX_SUCCESS) {
                g_vehicle_command = recv;
                tx_mutex_put(&g_vehicle_command_mutex);
            }
        }
        if (local_cmd.cruise_control_enabled == true && local_cmd.brake == false) {
            active_cruise_control = cruise_control(local_cmd.cruise_control_target_speed,
                                                   current_speed,
                                                   local_cmd.cruise_control_enabled,
                                                   cc_dt);
            if (tx_mutex_get(&g_vehicle_data_mutex, TX_WAIT_FOREVER) == TX_SUCCESS) {
                g_vehicle_data.cruise_control_active = active_cruise_control;
                tx_mutex_put(&g_vehicle_data_mutex);
            }
        }
        else {
            if (tx_mutex_get(&g_vehicle_data_mutex, TX_WAIT_FOREVER) == TX_SUCCESS) {
                g_vehicle_data.cruise_control_active = false;
                tx_mutex_put(&g_vehicle_data_mutex);
            }
        }
        if (r == TX_SUCCESS) {
            // Got a command - reset timeout counter
            no_cmd_ticks = 0;

            /* Occasionally print occupancy so we can see how full the queue is */
            static int occupancy_report_cnt = 0;
            occupancy_report_cnt++;
            if ((occupancy_report_cnt % 10) == 0) { /* every ~1s */
                UINT occ = 0;
                if (ControlQueue_GetOccupancy(&occ) == TX_SUCCESS) {
                    char occ_buf[64];
                    snprintf(occ_buf, sizeof(occ_buf), "[CONTROL] Queue occupancy=%u\r\n", occ);
                    // Debug_Print(occ_buf);
                }
            }

            // Update global copy for diagnostics/backwards compatibility
            if (tx_mutex_get(&g_vehicle_command_mutex, TX_WAIT_FOREVER) == TX_SUCCESS) {
                g_vehicle_command = recv;
                tx_mutex_put(&g_vehicle_command_mutex);
            }

            local_cmd = recv;
            if (local_cmd.command_valid) {
                // Check if command changed
                if (local_cmd.driving_mode != last_mode || 
                    local_cmd.gear != last_gear ||
                    local_cmd.throttle != last_throttle || 
                    local_cmd.steering_angle != last_steering ||
                    local_cmd.brake != last_brake ||
                    local_cmd.traffic_sign != last_traffic_sign ||
                    local_cmd.aeb_enabled != last_aeb_enabled) {

                    // Convert steering to normalized float
                    float steering_normalized = (float)local_cmd.steering_angle / 100.0f;
                    uint8_t applied_throttle = Control_ApplyTrafficSignThrottle(local_cmd.throttle,
                                                                               local_cmd.traffic_sign);

                    Control_SetSteering(steering_normalized);
                    Control_SetThrottle(applied_throttle, local_cmd.gear, local_cmd.brake);

                    snprintf(control_uart_buf, sizeof(control_uart_buf),
                             "[CONTROL] Sign=%d Throttle=%u%% -> %u%%\r\n",
                             local_cmd.traffic_sign,
                             applied_throttle);
                    Debug_Print(control_uart_buf);

                    // Print status
                    // const char* gear_names[] = {"P", "N", "R", "D"};
                    // int steering_int = (int)(steering_normalized * 1000);
                    // snprintf(control_uart_buf, sizeof(control_uart_buf),
                    //         "[CONTROL] Mode=%d Gear=%s | Throttle=%d%% | Steering=%d.%03d | brake=%d\r\n",
                    //         local_cmd.driving_mode, 
                    //         local_cmd.gear <= 3 ? gear_names[local_cmd.gear] : "?",
                    //         local_cmd.throttle, 
                    //         steering_int/1000, abs(steering_int%1000),
                    //         local_cmd.brake);
                    // Debug_Print(control_uart_buf);

                    // Update last values
                    last_mode = local_cmd.driving_mode;
                    last_gear = local_cmd.gear;
                    last_throttle = local_cmd.throttle;
                    last_traffic_sign = local_cmd.traffic_sign;
                    last_steering = local_cmd.steering_angle;
                    last_brake = local_cmd.brake;
                }
            }
        } else {
            // No command received within timeout
            no_cmd_ticks++;
            if (no_cmd_ticks >= no_cmd_threshold) {
                uint32_t now = HAL_GetTick();
                if ((now - last_safety_print_ts) >= SAFETY_PRINT_MS) {
#ifdef DISABLE_CONTROL_SAFETY
                    Debug_Print("[CONTROL] SAFETY: No recent command - motors stopped (DISABLED)\r\n");
#else
                    Debug_Print("[CONTROL] SAFETY: No recent command - motors stopped\r\n");
#endif
                    last_safety_print_ts = now;
                }
#ifndef DISABLE_CONTROL_SAFETY
                Control_StopMotors();
#endif
                no_cmd_ticks = 0; // report once per threshold
            }
        }
    }
}
