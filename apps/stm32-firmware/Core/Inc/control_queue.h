#ifndef CONTROL_QUEUE_H
#define CONTROL_QUEUE_H

#include "tx_api.h"
#include "Sensors/sensors.h" // for VehicleCommand_t
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CONTROL_QUEUE_SIZE 8

/**
 * Initialize control queue. Must be called before any send/receive operations.
 * Returns TX_SUCCESS on success.
 */
UINT ControlQueue_Init(void);

/**
 * Try to send a command to the control queue without waiting (ISR-safe usage pattern).
 * Returns 1 on success, 0 on drop/failure.
 */
int ControlQueue_TrySend(const VehicleCommand_t *cmd);

/**
 * Send with wait (task context). Returns 1 on success, 0 on failure.
 */
int ControlQueue_Send(const VehicleCommand_t *cmd, ULONG wait);

/**
 * Receive a command from the queue. Returns TX_SUCCESS on success.
 */
UINT ControlQueue_Receive(VehicleCommand_t *cmd, ULONG wait);

/**
 * Query number of dropped messages (enqueue failures).
 */
uint32_t ControlQueue_GetDrops(void);

#ifdef __cplusplus
}
#endif

#endif /* CONTROL_QUEUE_H */