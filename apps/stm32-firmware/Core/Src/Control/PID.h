#ifndef CRUISE_CONTROL_H
#define CRUISE_CONTROL_H

#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include "../Drivers/pca9685.h"
#include "../Sensors/sensors.h"

#define PID_KP 50.0f
#define PID_KI 12.3f
#define PID_KD 0.002f

#define PID_OUTPUT_MAX 100.0f
#define PID_OUTPUT_MIN 0.0f

#define PID_INTEGRAL_MAX 100.0f
#define PID_INTEGRAL_MIN -100.0f

#define FEED_FORWARD_GAIN 33.0f

typedef struct {
    float kp;
    float ki;
    float kd;
} PID_Gains_t;

typedef enum {
    PID_MODE_CRUISE,
    PID_MODE_STEERING
} PID_Mode_t;

float PID(float set_point, float current_value, float dt, PID_Mode_t mode);
float clamp(float value);
/* test helpers */
void PID_Reset(void);
float PID_GetIntegral(void);

bool cruise_control(uint8_t target_speed, float current_speed, bool enabled, float dt);
/*----------------------------------------   --------------------------------------------*/

#endif /* CRUISE_CONTROL_H */
