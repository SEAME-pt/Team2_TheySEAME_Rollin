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
    float output = 0.0f;

    if (dt <= 0.0f) {
        return 0.0f;
    }

    derivative = (error - prev_error) / dt;

    output = PID_KP * error + PID_KI * integral + PID_KD * derivative;

    if (!((output >= PID_OUTPUT_MAX && error > 0.f) ||
          (output <= PID_OUTPUT_MIN && error < 0.f)))
    {
        integral += error * dt;
        if (integral > PID_INTEGRAL_MAX) integral = PID_INTEGRAL_MAX;
        if (integral < PID_INTEGRAL_MIN) integral = PID_INTEGRAL_MIN;
        output = PID_KP * error + PID_KI * integral + PID_KD * derivative;
    }
    prev_error = error;

    return output;
}

float clamp(float value)
{
    if (value < PID_OUTPUT_MIN) {
        value = PID_OUTPUT_MIN;
    } else if (value > PID_OUTPUT_MAX) {
        value = PID_OUTPUT_MAX;
    }
    return value;
}

float cruise_control(uint8_t target_speed, float current_speed, float dt)
{
    float set_point = (float)target_speed / 36.0f; // Convert hm/h to m/s
    float throttle = PID(set_point, current_speed, dt);
    
    char buf[128];
    snprintf(buf, sizeof(buf), "%d.%02d\r\n", (int)current_speed, (int)((current_speed - (int)current_speed) * 100));
    Debug_Print("[CRUISE CONTROL](current_speed): ");
    Debug_Print(buf);
    snprintf(buf, sizeof(buf), "%d.%02d\r\n", (int)set_point, (int)((set_point - (int)set_point) * 100));
    Debug_Print("[CRUISE CONTROL](target_speed): ");
    Debug_Print(buf);
    snprintf(buf, sizeof(buf), "%d.%02d\r\n", (int)throttle, (int)((throttle - (int)throttle) * 100));
    Debug_Print("[CRUISE CONTROL](throttle): ");
    Debug_Print(buf);
    
    throttle = clamp(throttle);
    return throttle;
}