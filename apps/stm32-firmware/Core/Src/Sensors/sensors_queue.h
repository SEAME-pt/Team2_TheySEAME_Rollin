/**
 * @file sensors_queue.h
 * @brief Sensors queue API and types for sensor samples
 *
 * Defines the `SensorId_t` identifiers and `SensorSample_t` payload used to
 * pass sensor samples from producers (ISR or sampling threads) to the
 * Sensors Processor thread. Producers should use `SensorsQueue_TrySend`
 * when called from time-critical contexts; the sensor processor calls
 * `SensorsQueue_Receive` to drain and process samples.
 *
 * Requirement traceability:
 * [impl->arch~sensors-queue~1]
 */

#ifndef SENSORS_QUEUE_H
#define SENSORS_QUEUE_H

#include "tx_api.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @enum SensorId_t
 * @brief Identifiers for supported sensor types
 */
typedef enum {
    SENSOR_ID_SPEED = 1,   /**< Wheel speed sensor (m/s) */
    SENSOR_ID_BATTERY = 2, /**< Battery percentage/voltage */
    SENSOR_ID_INA219 = 3,  /**< INA219 current sensor */
} SensorId_t;

/**
 * @struct SensorSample_t
 * @brief Single sensor sample payload
 *
 * @param sensor_id Type of sensor (SensorId_t)
 * @param value Floating point payload (interpretation depends on sensor)
 * @param ts Timestamp in ms (from HAL_GetTick)
 */
typedef struct {
    SensorId_t sensor_id;
    float value;      /**< simple payload: single float value */
    uint32_t ts;      /**< timestamp (HAL_GetTick) */
} SensorSample_t;

/** Maximum queued samples */
#define SENSORS_QUEUE_SIZE 64

/**
 * @brief Initialize sensors queue
 *
 * Creates the underlying ThreadX queue and resets internal drop counter.
 *
 * Requirement traceability:
 * [impl->arch~sensors-queue-init~1]
 *
 * @return UINT TX_SUCCESS on success
 */
UINT SensorsQueue_Init(void);

/**
 * @brief Try to enqueue a sensor sample without waiting
 *
 * Intended for use in ISRs or high-priority producers. If the queue is full
 * the sample is dropped and the drop counter is incremented.
 *
 * @param samp Pointer to sample to enqueue
 *
 * Requirement traceability:
 * [impl->arch~sensors-queue-send-nonblocking~1]
 *
 * @return int 1 on success, 0 on drop/failure
 */
int SensorsQueue_TrySend(const SensorSample_t *samp);

/**
 * @brief Enqueue a sensor sample with optional wait
 *
 * Task-context send with a wait option. On failure the drop counter is
 * incremented.
 *
 * @param samp Pointer to sample to enqueue
 * @param wait Number of ticks to wait
 *
 * Requirement traceability:
 * [impl->arch~sensors-queue-send-blocking~1]
 *
 * @return int 1 on success, 0 on failure
 */
int SensorsQueue_Send(const SensorSample_t *samp, ULONG wait);

/**
 * @brief Receive a sample from the sensors queue
 *
 * Blocks for up to `wait` ticks waiting for a sample.
 *
 * @param samp Pointer to receive the sample
 * @param wait Number of ticks to wait (TX_WAIT_FOREVER for indefinite)
 *
 * Requirement traceability:
 * [impl->arch~sensors-queue-receive~1]
 *
 * @return UINT TX_SUCCESS on success
 */
UINT SensorsQueue_Receive(SensorSample_t *samp, ULONG wait);

/**
 * @brief Get number of dropped samples
 *
 * Returns the number of times enqueue operations failed due to the queue
 * being full (useful for monitoring and tuning).
 *
 * Requirement traceability:
 * [impl->arch~sensors-queue-drops~1]
 *
 * @return uint32_t Count of dropped messages since last init
 */
uint32_t SensorsQueue_GetDrops(void);

/**
 * @brief Get number of messages currently queued (occupancy)
 *
 * @param count Pointer to UINT to receive current message count
 * @return UINT TX_SUCCESS on success
 */
UINT SensorsQueue_GetOccupancy(UINT *count);

#ifdef __cplusplus
}
#endif

#endif /* SENSORS_QUEUE_H */