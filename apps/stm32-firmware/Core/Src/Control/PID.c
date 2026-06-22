#include "PID.h"

static float integral_cruise = 0.0f;
static float prev_error_cruise = 0.0f;

void PID_Reset(void)
{
    integral_cruise = 0.0f;
    prev_error_cruise = 0.0f;
}

float PID_GetIntegral(void)
{
    return integral_cruise;
}

float PID(float set_point, float current_value, float dt)
{
    float error = set_point - current_value;
    float output = 0.0f;
    float ff = 0.0f;
        
    if (dt <= 0.0f) 
        return 0.0f;
    float derivative = (error - prev_error_cruise) / dt;

    ff = set_point * FEED_FORWARD_GAIN;

    float u_unsat = ff + PID_KP * error +
                    PID_KI * integral_cruise +
                    PID_KD * derivative;

    output = clamp(u_unsat);

    if (!((output >= PID_OUTPUT_MAX && error > 0.0f) ||
        (output <= PID_OUTPUT_MIN && error < 0.0f)))
    {
        integral_cruise += error * dt;

        if (integral_cruise > PID_INTEGRAL_MAX) integral_cruise = PID_INTEGRAL_MAX;
        if (integral_cruise < PID_INTEGRAL_MIN) integral_cruise = PID_INTEGRAL_MIN;
    }

    prev_error_cruise = error;
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

bool cruise_control(uint8_t target_speed, float current_speed, bool enabled, float dt)
{
    float set_point = (float)target_speed / 36.0f; // Convert hm/h to m/s
    bool active = true;

    // Guard dt so PID remains stable if scheduler jitter or delayed loop occurs.
    if (dt < 0.01f) dt = 0.01f;
    if (dt > 0.20f) dt = 0.20f;

    float throttle = 0.0f;
    if (enabled && (set_point > 0.416f && set_point < 3.61f))
        throttle = PID(set_point, current_speed, dt);
    else
    {
        active = false;
        PID_Reset();
        throttle = 0.0f;
    }
    throttle = clamp(throttle);
    char buf[128];
    snprintf(buf, sizeof(buf), "[CC] current=%.2f target=%.2f throttle=%.2f%% dt=%.2f\r\n",
            current_speed, set_point, throttle, dt);
    Debug_Print(buf);
    Control_SetThrottle(throttle, 3, false); // Ensure manual throttle is off when cruise control is active
    return active;
}