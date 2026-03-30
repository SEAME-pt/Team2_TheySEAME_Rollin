#include "PID.h"

static float integral_cruise = 0.0f;
static float prev_error_cruise = 0.0f;
static float prev_error_steering = 0.0f;

void PID_Reset(void)
{
    integral_cruise = 0.0f;
    prev_error_cruise = 0.0f;
    prev_error_steering = 0.0f;
}

float PID_GetIntegral(void)
{
    return integral_cruise;
}

float PID(float set_point, float current_value, float dt, PID_Mode_t mode)
{
    if (dt <= 0.0f) return 0.0f;

    float error = set_point - current_value;
    float output = 0.0f;
    float ff = 0.0f;

    PID_Gains_t cruise_gains = {50.0f, 12.3f, 0.0f};
    PID_Gains_t steering_gains = {10.0f, 0.0f, 1.0f};

    // CRUISE
    if (mode == PID_MODE_CRUISE)
    {
        float derivative = (error - prev_error_cruise) / dt;

        ff = set_point * FEED_FORWARD_GAIN;

        float u_unsat = ff + cruise_gains.kp * error +
                        cruise_gains.ki * integral_cruise +
                        cruise_gains.kd * derivative;

        output = clamp(u_unsat);

        if (!((output >= PID_OUTPUT_MAX && error > 0.0f) ||
            (output <= PID_OUTPUT_MIN && error < 0.0f)))
        {
            integral_cruise += error * dt;

            if (integral_cruise > PID_INTEGRAL_MAX) integral_cruise = PID_INTEGRAL_MAX;
            if (integral_cruise < PID_INTEGRAL_MIN) integral_cruise = PID_INTEGRAL_MIN;
        }

        prev_error_cruise = error;
    }
    // STEERING
    else if (mode == PID_MODE_STEERING)
    {
        float derivative = (error - prev_error_steering) / dt;

        output = steering_gains.kp * error +
                steering_gains.kd * derivative;

        prev_error_steering = error;
        // char buf[128];
        // snprintf(buf, sizeof(buf), "[PID] set_point=%.2f current=%.2f error=%.2f output=%.2f dt=%.2f\r\n",
        //         set_point, current_value, error, output, dt);
        // Debug_Print(buf);
    }

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
        throttle = PID(set_point, current_speed, dt, PID_MODE_CRUISE);
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
    Control_SetThrottle(throttle, 3, false); // Ensure manual throttle is off when cruise control is active
    return active;
}

void steer_control(int target_pos, float lane_position, float dt)
{
    dt = 0.1;
    float steering_output = PID(target_pos, lane_position, dt, PID_MODE_STEERING);
    char buf[128];
    snprintf(buf, sizeof(buf), "[STEER] target_pos=%d lane_pos=%.2f output=%.2f%% dt=%.2f\r\n",
            target_pos, lane_position, steering_output, dt);
    // Debug_Print(buf);
    Control_SetSteering(steering_output / 100.0f);
}