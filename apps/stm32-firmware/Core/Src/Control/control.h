#ifndef CONTROL_H
#define CONTROL_H

#include "main.h"
#include "tx_api.h"

/* Control functions */
void Control_Thread_Entry(ULONG thread_input);
void PCA9685_SetServoAngle(uint8_t channel, float angle);
void Control_SetSteering(float steering_normalized);  // -1.0 to +1.0
void Control_SetThrottle(uint8_t throttle_percent);   // 0-100%
void Control_StopMotors(void);

#endif
