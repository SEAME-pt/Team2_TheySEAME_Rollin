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
    float ff;

    if (dt <= 0.0f) {
        return 0.0f;
    }
    
    derivative = (error - prev_error) / dt;
    ff = set_point * FEED_FORWARD_GAIN; // Simple feed-forward term proportional to target speed
    output = ff +PID_KP * error + PID_KI * integral + PID_KD * derivative;

    if (!((output >= PID_OUTPUT_MAX && error > 0.f) ||
          (output <= PID_OUTPUT_MIN && error < 0.f)))
    {
        integral += error * dt;
        if (integral > PID_INTEGRAL_MAX) integral = PID_INTEGRAL_MAX;
        if (integral < PID_INTEGRAL_MIN) integral = PID_INTEGRAL_MIN;
        output = ff + PID_KP * error + PID_KI * integral + PID_KD * derivative;
    }
    prev_error = error;

    char buf[128];
    snprintf(buf, sizeof(buf), "[PID] error=%.2f deriv=%.2f integ=%.2f output=%.2f\r\n",
            error, derivative, integral, output);
    Debug_Print(buf);
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

bool cruise_control(uint8_t target_speed, float current_speed, bool enabled)
{
    float set_point = (float)target_speed / 36.0f; // Convert hm/h to m/s
    static uint32_t last_tick = 0;
    uint32_t now = HAL_GetTick();
    bool active = true;
    float dt = 0.1f;

    if (last_tick != 0) {
        dt = (now - last_tick) / 1000.0f;
    }
    last_tick = now;
    float throttle = 0.0f;
    if (enabled && current_speed > 0.416f && current_speed < 3.61f) // Only enable if target or current speed is above ~3 km/h to prevent trying to maintain 0 speed
        throttle = PID(set_point, current_speed, dt);
    else
    {
        active = false;
        PID_Reset(); // Reset PID state when cruise control is disabled to prevent windup on next enable
        throttle = 0.0f; // No throttle when cruise control is disabled
    }
    throttle = clamp(throttle);
    char buf[128];
    snprintf(buf, sizeof(buf), "[CC] current=%.2f target=%.2f throttle=%.2f%% dt=%.2f\r\n",
            current_speed, set_point, throttle, dt);
    Debug_Print(buf);
    Control_SetThrottle(throttle, 3); // Ensure manual throttle is off when cruise control is active
    return active;
}