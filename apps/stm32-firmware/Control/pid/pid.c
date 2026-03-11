#include "pid.h"

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