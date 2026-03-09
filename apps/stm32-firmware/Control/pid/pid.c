#include "pid.h"

float PID(float set_point, float current_value, float dt, PIDController *pid)
{
    pid->error = set_point - current_value;

    pid->integral += pid->error * dt;

    pid->derivative = 0.0f;
    pid->derivative = (pid->error - pid->prev_error) / dt;

    pid->prev_error = pid->error;

    pid->controller_output = PID_KP * pid->error + PID_KI * pid->integral + PID_KD * pid->derivative;
    // printf("Error: %.4f, Integral: %.4f, Derivative: %.4f\n", pid->error, pid->integral, derivative);
    return pid->controller_output;
}

void PID_Reset(PIDController *pid)
{
    pid->prev_error = 0.0f;
    pid->integral = 0.0f;
    pid->derivative = 0.0f;
    pid->controller_output = 0.0f;
}