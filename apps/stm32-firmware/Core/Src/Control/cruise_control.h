#ifndef CRUISE_CONTROL_H
#define CRUISE_CONTROL_H

#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include "../Drivers/pca9685.h"
#include "../Sensors/sensors.h"
/*----------------------------------------CRUISE_CONTROL --------------------------------------------*/



/*----------------------------------------PID --------------------------------------------*/
#define PID_KP 15.0f
#define PID_KI 3.5f
#define PID_KD 0.3f

#define PID_OUTPUT_MAX 100.0f
#define PID_OUTPUT_MIN 7.0f
#define PID_INTEGRAL_MAX 40.0f
#define PID_INTEGRAL_MIN -40.0f
#define FEED_FORWARD_GAIN 18.0f

float PID(float target_speed, float current_speed, float dt);
float clamp(float value);
float cruise_control(uint8_t target_speed, float current_speed, float dt);
/* test helpers */
void PID_Reset(void);
float PID_GetIntegral(void);

/*----------------------------------------   --------------------------------------------*/

#endif /* CRUISE_CONTROL_H */