#ifndef SENSORS_H
#define SENSORS_H

#include "main.h"
#include "tx_api.h"

#define THREAD_SLEEP_TICKS 10
#define PULSES_PER_REV 20

/**
 * @brief Vehicle command received from CAN/RPi
 *
 * Encodes the control inputs coming from remote/system controller.
 *
 * Requirement traceability:
 */
typedef struct {
    uint8_t driving_mode;       /**< Driving mode: 0=MANUAL, 1=AI_ASSIST */
    uint8_t gear;               /**< Gear: 0=P, 1=N, 2=R, 3=D */
    uint8_t throttle;           /**< Throttle value 0-100 */
    int8_t steering_angle;      /**< Steering -100..+100 representing -1.0..+1.0 */
    uint8_t command_valid;      /**< Flag: 1 if command received, 0 otherwise */
} VehicleCommand_t;

/**
 * @brief Global vehicle command received via CAN
 *
 * Protected by `g_vehicle_command_mutex`.
 *
 * Requirement traceability:
 */
extern VehicleCommand_t g_vehicle_command;

/**
 * @brief Mutex protecting `g_vehicle_command`
 */
extern TX_MUTEX g_vehicle_command_mutex;

/**
 * @brief Calculate RPM from timer delta ticks
 *
 * Converts the measured timer delta between pulses into RPM applying
 * noise filtering and timer calibration.
 *
 * @param delta_ticks Time delta in timer ticks
 *
 * Requirement traceability:
 *
 * @return uint32_t Calculated RPM (0 if below noise threshold)
 */
uint32_t Speed_CalculateRPM(uint32_t delta_ticks);

/**
 * @brief Convert RPM to linear speed (m/s)
 *
 * Uses configured wheel circumference to convert rotational speed to
 * linear velocity.
 *
 * @param rpm Rotational speed in RPM
 *
 * Requirement traceability:
 *
 * @return float Linear speed in meters per second
 */
float Speed_RPMToMetersPerSecond(uint32_t rpm);

/**
 * @brief Process a delta reading and update running averages
 *
 * Adds the incoming RPM sample to the running average and emits a report
 * every N samples (N=5). On emit, outputs calculated speed.
 *
 * @param delta_ticks Time delta in timer ticks
 * @param average Pointer to running average accumulator (modified)
 * @param counter Pointer to reading counter (modified)
 * @param out_speed_ms Output pointer for calculated speed in m/s
 *
 * Requirement traceability:
 *
 * @return int 1 if output was generated (N readings), 0 otherwise
 */
int Speed_ProcessDelta(uint32_t delta_ticks, uint32_t *average, int *counter, float *out_speed_ms);

/**
 * @brief Speed calculation thread entry point
 *
 * Reads captured pulse deltas, computes RPM and linear speed, and updates
 * the global vehicle data structure. Runs periodically under ThreadX.
 *
 * @param thread_input Thread parameter passed by ThreadX scheduler (unused)
 *
 * Requirement traceability:
 *
 * @return void
 */
extern void Speed_Thread_Entry(ULONG thread_input);
#endif
