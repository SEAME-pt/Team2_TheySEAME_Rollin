#ifndef CONTROL_H
#define CONTROL_H

#include "main.h"
#include "tx_api.h"

/**
 * @brief Control thread entry and main loop
 *
 * Runs the control subsystem responsible for actuator commands and safety:
 * - Initialize and verify both PCA9685 devices (steering and throttle)
 * - Read global vehicle command structure and apply steering/throttle
 * - Enforce safety (stop motors when commands invalid/missing)
 * - Log applied commands for diagnostics
 *
 * Responsibilities:
 * - Ensure PCA9685 devices are initialized and operational
 * - Convert and clamp control inputs into safe actuator commands
 * - Stop motors on invalid state or failsafe conditions
 *
 * @param thread_input RTOS thread input parameter (unused)
 *
 * Requirement traceability:
 * [impl->arch~control-actuation-flow~1]
 * [impl->dsg~control-throttle-command~1]
 * [impl->dsg~control-user-interface~1]
 * [impl->dsg~control-safety-limits~1]
 * [impl->feat~rtos-control~1]
 *
 * @return void
 */
void Control_Thread_Entry(ULONG thread_input);

/**
 * @brief Set PCA9685 servo angle on a channel
 *
 * Clamps the requested angle to the safe range and programs the PCA9685
 * PWM output accordingly.
 *
 * @param channel PCA9685 channel number
 * @param angle Target servo angle in degrees (-30..+30)
 *
 * Requirement traceability:
 * [impl->arch~control-actuation-flow~1]
 *
 * @return void
 */
void PCA9685_SetServoAngle(uint8_t channel, float angle);

/**
 * @brief Set steering command
 *
 * Converts a normalized steering value (-1.0..+1.0) into a servo angle
 * and forwards it to the steering driver.
 *
 * @param steering_normalized Normalized steering value in range -1.0..+1.0
 *
 * Requirement traceability:
 * [impl->dsg~control-user-interface~1]
 * [impl->arch~control-actuation-flow~1]
 *
 * @return void
 */
void Control_SetSteering(float steering_normalized);  // -1.0 to +1.0

/**
 * @brief Set throttle command
 *
 * Clamps the throttle percentage (0..100), converts it to PWM and
 * commands the throttle PCA9685 outputs. Passing 0 will stop motors.
 *
 * @param throttle_percent Throttle percentage (0..100)
 *
 * Requirement traceability:
 * [impl->dsg~control-throttle-command~1]
 * [impl->dsg~control-safety-limits~1]
 *
 * @return void
 */
void Control_SetThrottle(uint8_t throttle_percent);   // 0-100%

/**
 * @brief Emergency stop — stop all motors immediately
 *
 * Sets all motor PWM outputs to zero to ensure no motion.
 *
 * Requirement traceability:
 * [impl->dsg~control-safety-limits~1]
 *
 * @return void
 */
void Control_StopMotors(void);

#endif
