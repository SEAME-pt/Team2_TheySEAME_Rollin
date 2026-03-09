#ifndef PID_H
#define PID_H
#include <stdbool.h>
#include <stdio.h>

#define PID_KP 9.0f
#define PID_KI 0.02f
#define PID_KD 0.05f

typedef struct {
    float error;
    float prev_error;
    float integral;
    float derivative;
    float controller_output;
} PIDController;

float PID(float target_speed, float current_speed, float dt, PIDController *pid);
void PID_Reset(PIDController *pid);

#endif