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
 * Note: For short-lived debugging you can disable the automatic safety stop
 * by defining `DISABLE_CONTROL_SAFETY` (compile-time only). Do NOT enable in
 * production builds.
 *
 * For convenience, in debug or test builds this macro is defined automatically
 * so you don't need to rebuild with extra flags. Remove this convenience for
 * production builds or CI by undefining `DEBUG`/`TEST_MODE` or explicitly
 * undefining `DISABLE_CONTROL_SAFETY`.
 */

#if defined(DEBUG) || defined(TEST_MODE)
#ifndef DISABLE_CONTROL_SAFETY
#define DISABLE_CONTROL_SAFETY
#endif
#endif

/**
 * @brief Control thread entry and main loop
 *
 * Runs the control subsystem responsible for actuator commands and safety.
 *
 * @param thread_input RTOS thread input parameter (unused)
 *
 * Requirement traceability:
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
 *
 * @return void
 */
void Control_SetSteering(float steering_normalized);  // -1.0 to +1.0

/**
 * @brief Set throttle command
 *
 * Converts desired velocity (m/s) to throttle percentage, then to PWM.
 * Commands the throttle PCA9685 outputs. Passing 0.0 will stop motors.
 * Motor direction is controlled by gear (2=Reverse swaps DIR pins).
 *
 * @param desired_velocity_ms Desired velocity in m/s (0.0 to MAX_VELOCITY_MS)
 * @param gear Gear selection (0=P, 1=N, 2=R, 3=D)
 *
 * Requirement traceability:
 *
 * @return void
 */
void Control_SetThrottle(float desired_velocity_ms, uint8_t gear);

/**
 * @brief Emergency stop — stop all motors immediately
 *
 * Sets all motor PWM outputs to zero to ensure no motion.
 *
 * Requirement traceability:
 *
 * @return void
 */
void Control_StopMotors(void);

#endif
