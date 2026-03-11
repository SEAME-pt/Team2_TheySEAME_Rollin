#ifndef CRUISE_CONTROL_H
#define CRUISE_CONTROL_H

#include <stdbool.h>
#include <stdio.h>

/*----------------------------------------CRUISE_CONTROL --------------------------------------------*/



/*----------------------------------------PID --------------------------------------------*/
#define PID_KP 9.0f
#define PID_KI 0.02f
#define PID_KD 0.05f

#define PID_OUTPUT_MAX 100.0f
#define PID_OUTPUT_MIN 0.0f

float PID(float target_speed, float current_speed, float dt);
float clamp(float value);

/* test helpers */
void PID_Reset(void);
float PID_GetIntegral(void);

/*----------------------------------------   --------------------------------------------*/

#endif /* CRUISE_CONTROL_H */