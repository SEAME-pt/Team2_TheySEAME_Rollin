#include "control.h"
#include "../Drivers/pca9685.h"
#include "../Sensors/sensors.h"
#include "control_queue.h"
#include "../Sensors/sensors_queue.h"
#include <stdio.h>

extern I2C_HandleTypeDef hi2c1;
char control_uart_buf[128];

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
    // Convert normalized steering (-1.0 to +1.0) to servo angle (-30 to +30 degrees)
    float angle = steering_normalized * 30.0f;
    PCA9685_SetServoAngle(0, angle);  // Channel 0 for steering servo
}

void Control_SetThrottle(uint8_t throttle_percent) {
    // Clamp throttle to 0-100%
    if (throttle_percent > 100) throttle_percent = 100;
    
    // Convert percentage to PWM value (0-4095)
    uint16_t speed_pwm = (uint16_t)((throttle_percent * 4095) / 100);
    uint16_t dir_high = 4095;
    uint16_t dir_low = 0;
    
    if (throttle_percent > 0) {
        // Motor 1 forward (channels 0,1,2,3)
        PCA9685_SetPWM(&hi2c1, PCA9685_ADDR_THROTTLE, 0, 0, speed_pwm);  // M1 speed
        PCA9685_SetPWM(&hi2c1, PCA9685_ADDR_THROTTLE, 1, 0, dir_high);   // M1 DIR1 (forward)
        PCA9685_SetPWM(&hi2c1, PCA9685_ADDR_THROTTLE, 2, 0, dir_low);    // M1 DIR2 (reverse)
        PCA9685_SetPWM(&hi2c1, PCA9685_ADDR_THROTTLE, 3, 0, 0);          // Unused
        
        // Motor 2 forward (channels 4,5,6,7)
        PCA9685_SetPWM(&hi2c1, PCA9685_ADDR_THROTTLE, 4, 0, speed_pwm);  // M2 speed
        PCA9685_SetPWM(&hi2c1, PCA9685_ADDR_THROTTLE, 5, 0, dir_low);    // M2 DIR2 (reverse)
        PCA9685_SetPWM(&hi2c1, PCA9685_ADDR_THROTTLE, 6, 0, dir_high);   // M2 DIR1 (forward)
        PCA9685_SetPWM(&hi2c1, PCA9685_ADDR_THROTTLE, 7, 0, speed_pwm);  // M2 speed
    } else {
        Control_StopMotors();
    }
}

void Control_StopMotors(void) {
    // Set all motor channels to 0
    for (uint8_t ch = 0; ch < 8; ch++) {
        PCA9685_SetPWM(&hi2c1, PCA9685_ADDR_THROTTLE, ch, 0, 0);
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
    uint8_t last_mode = 0xFF;
    uint8_t last_throttle = 0xFF;
    int8_t last_steering = 0x7F;

    const ULONG cmd_wait_ticks = 10; // 100ms wait for command before timeout handling
    ULONG no_cmd_ticks = 0;
    const ULONG no_cmd_threshold = 10; // 10 * 100ms = 1s without command -> stop motors

    /* Rate limit safety prints so operator can read them (ms) */
    const uint32_t SAFETY_PRINT_MS = 5000; // 5 seconds
    uint32_t last_safety_print_ts = 0;

    while(1) {
        VehicleCommand_t recv;
        UINT r = ControlQueue_Receive(&recv, cmd_wait_ticks);
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
                    Debug_Print(occ_buf);
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
                    local_cmd.throttle != last_throttle || 
                    local_cmd.steering_angle != last_steering) {

                    // Convert steering to normalized float
                    float steering_normalized = (float)local_cmd.steering_angle / 100.0f;

                    // Apply commands
                    Control_SetSteering(steering_normalized);
                    Control_SetThrottle(local_cmd.throttle);

                    // Print status
                    int steering_int = (int)(steering_normalized * 1000);
                    snprintf(control_uart_buf, sizeof(control_uart_buf),
                            "[CONTROL] Mode=%d | Throttle=%d%% | Steering=%d.%03d\r\n",
                            local_cmd.driving_mode, local_cmd.throttle, 
                            steering_int/1000, abs(steering_int%1000));
                    Debug_Print(control_uart_buf);

                    // Update last values
                    last_mode = local_cmd.driving_mode;
                    last_throttle = local_cmd.throttle;
                    last_steering = local_cmd.steering_angle;
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
