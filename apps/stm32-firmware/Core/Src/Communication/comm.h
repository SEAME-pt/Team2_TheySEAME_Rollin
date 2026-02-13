/**
 ******************************************************************************
 * @file    comm.h
 * @brief   Communication module header
 * @note    Independent CAN communication thread
 ******************************************************************************
 */

#ifndef COMM_H
#define COMM_H

#include "main.h"

/**
 * @brief Communication thread entry and main loop
 *
 * Runs the communications subsystem loop responsible for CAN interactions:
 * - Initialize and monitor MCP2515 controller
 * - Transmit telemetry messages (speed, battery) with rate-limiting
 * - Receive and parse incoming CAN frames and update in-memory telemetry
 * - Handle CAN-level errors, timeouts and diagnostics
 *
 * Responsibilities and behavior:
 * - Ensures initialization is verified via CANSTAT reads
 * - Enforces transmission rate limits (e.g., speed updates not faster than 200ms)
 * - Updates shared state for other modules to consume parsed telemetry
 * - Observes logging policy for diagnostic prints
 *
 * @param thread_input Thread input parameter provided by the RTOS (typically a pointer or id). Can be NULL if unused.
 *
 * Requirement traceability:
 * [impl->dsn~can-init~1]
 * [impl->dsn~can-telemetry-tx~1]
 * [impl->dsn~can-telemetry-rx~1]
 * [impl->dsn~can-error-handling~1]
 * [impl->dsn~can-rate-limit~1]
 *
 * @return void
 */
void Communication_Thread_Entry(ULONG thread_input);

#endif /* COMM_H */
