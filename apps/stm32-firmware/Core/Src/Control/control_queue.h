/**
 * @file control_queue.h
 * @brief Control queue API for passing commands to the Control thread
 *
 * This module provides a small ThreadX-backed queue used to pass
 * `VehicleCommand_t` messages from producers (e.g., Communication thread)
 * to the Control thread. Producers should use `ControlQueue_TrySend` when
 * called from interrupt or time-critical contexts (non-blocking). Tasks
 * that can wait may call `ControlQueue_Send`.
 *
 * Requirement traceability:
 * [impl->arch~control-queue~1]
 */

#ifndef CONTROL_QUEUE_H
#define CONTROL_QUEUE_H

#include "tx_api.h"
#include "../Sensors/sensors.h" // for VehicleCommand_t
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Maximum number of commands the control queue can hold */
#define CONTROL_QUEUE_SIZE 32 /* Increased temporarily for debugging/backpressure testing */

/**
 * @brief Initialize control queue
 *
 * Must be called before any send/receive operations. Resets internal
 * drop counters and creates the underlying ThreadX queue.
 *
 * @return UINT TX_SUCCESS on success, error code otherwise
 *
 * Requirement traceability:
 * [impl->arch~control-queue-init~1]
 */
UINT ControlQueue_Init(void);

/**
 * @brief Try to send a command without blocking
 *
 * Intended for use in interrupt or high-priority contexts. If the queue is
 * full the message is dropped and the drop counter is incremented.
 *
 * @param cmd Pointer to `VehicleCommand_t` to enqueue
 *
 * Requirement traceability:
 * [impl->arch~control-queue-send-nonblocking~1]
 *
 * @return int 1 on success, 0 on drop/failure
 */
int ControlQueue_TrySend(const VehicleCommand_t *cmd);

/**
 * @brief Send a command with optional waiting
 *
 * Used from task context when waiting for queue space is acceptable.
 * On failure the drop counter is incremented.
 *
 * @param cmd Pointer to `VehicleCommand_t` to enqueue
 * @param wait Number of ticks to wait (use TX_WAIT_FOREVER to wait indefinitely)
 *
 * Requirement traceability:
 * [impl->arch~control-queue-send-blocking~1]
 *
 * @return int 1 on success, 0 on failure
 */
int ControlQueue_Send(const VehicleCommand_t *cmd, ULONG wait);

/**
 * @brief Receive a command from the queue
 *
 * Blocks for up to `wait` ticks waiting for a message.
 *
 * @param cmd Pointer to receive the dequeued `VehicleCommand_t`
 * @param wait Number of ticks to wait (TX_WAIT_FOREVER for indefinite)
 *
 * Requirement traceability:
 * [impl->arch~control-queue-receive~1]
 *
 * @return UINT TX_SUCCESS on success
 */
UINT ControlQueue_Receive(VehicleCommand_t *cmd, ULONG wait);

/**
 * @brief Get number of dropped messages
 *
 * Returns the number of times enqueue operations failed due to the queue
 * being full (useful for monitoring and tuning).
 *
 * Requirement traceability:
 * [impl->arch~control-queue-drops~1]
 *
 * @return uint32_t Count of dropped messages since last init
 */
uint32_t ControlQueue_GetDrops(void);

/**
 * @brief Get number of messages currently queued (occupancy)
 *
 * @param count Pointer to UINT to receive current message count
 * @return UINT TX_SUCCESS on success
 */
UINT ControlQueue_GetOccupancy(UINT *count);

#ifdef __cplusplus
}
#endif

#endif /* CONTROL_QUEUE_H */
