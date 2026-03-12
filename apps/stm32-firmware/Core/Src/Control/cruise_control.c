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
    Debug_Print("[CRUISE CONTROL] PID output (throttle %): ");
    char buf[64];
    snprintf(buf, sizeof(buf), "%d.%02d%%\r\n", (int)throttle, (int)((throttle - (int)throttle) * 100));
    PCA9685_SetPWM(&hi2c1, PCA9685_ADDR_THROTTLE, 0, 0, speed_pwm);
    PCA9685_SetPWM(&hi2c1, PCA9685_ADDR_THROTTLE, 4, 0, speed_pwm);
    return true;
}