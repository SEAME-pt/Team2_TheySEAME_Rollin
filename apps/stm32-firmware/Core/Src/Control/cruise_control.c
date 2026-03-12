#include "cruise_control.h"

static float integral = 0.0f;
static float prev_error = 0.0f;

void PID_Reset(void)
{
    integral = 0.0f;
    prev_error = 0.0f;
}

float PID_GetIntegral(void)
{
    return integral;
}

float PID(float set_point, float current_value, float dt)
{
    float error = set_point - current_value;
    float derivative = 0.0f;
    float output_unsat = 0.0f;

    if (dt <= 0.0f) {
        return 0.0f;
    }

    derivative = (error - prev_error) / dt;

    output_unsat = PID_KP * error + PID_KI * integral + PID_KD * derivative;

    if (!((output_unsat >= PID_OUTPUT_MAX && error > 0.f) ||
          (output_unsat <= PID_OUTPUT_MIN && error < 0.f)))
    {
        integral += error * dt;
        output_unsat = PID_KP * error + PID_KI * integral + PID_KD * derivative;
    }

    prev_error = error;

    return clamp(output_unsat);
}

float clamp(float value)
{
    if (value > PID_OUTPUT_MAX)
        return PID_OUTPUT_MAX;
    else if (value < PID_OUTPUT_MIN)
        return PID_OUTPUT_MIN;
    else
        return value;
}

bool cruise_control(float target_speed, float current_speed, float dt, VehicleCommand_t command_data)
{
    float throttle = PID(target_speed, current_speed, dt);
    uint16_t speed_pwm = (uint16_t)((throttle * 4095) / 100);
    uint16_t dir_high = 0;      // Swapped base values to match motor wiring
    uint16_t dir_low = 4095;
    
    char buf[128];
    snprintf(buf, sizeof(buf), "%d.%02d%%\r\n", (int)current_speed, (int)((current_speed - (int)current_speed) * 100));
    Debug_Print("[CRUISE CONTROL](current_speed): ");
    Debug_Print(buf);
    snprintf(buf, sizeof(buf), "%d.%02d%%\r\n", (int)target_speed, (int)((target_speed - (int)target_speed) * 100));
    Debug_Print("[CRUISE CONTROL](target_speed): ");
    Debug_Print(buf);
    snprintf(buf, sizeof(buf), "%d.%02d%%\r\n", (int)throttle, (int)((throttle - (int)throttle) * 100));
    Debug_Print("[CRUISE CONTROL](throttle): ");
    Debug_Print(buf);
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
    
    return true;
}