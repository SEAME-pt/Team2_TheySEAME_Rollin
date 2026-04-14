#ifndef UTILS_H
#define UTILS_H

#include "main.h"

/**
 * @brief Print a debug message over UART
 *
 * Sends a NUL-terminated debug string to the configured UART device. Output
 * respects the project's logging policy and should not be used for high-volume
 * telemetry in production builds unless explicitly enabled.
 *
 * @param msg NUL-terminated message string to transmit
 *
 * Requirement traceability:
 *
 * @return void
 */
void Debug_Print(const char *msg);

#endif
