/**
 ******************************************************************************
 * @file    mcp2515.h
 * @brief   MCP2515 CAN Controller Driver for STM32 (Software SPI)
 * @note    This driver uses GPIO bit-banging for SPI communication
 ******************************************************************************
 */

#ifndef MCP2515_H
#define MCP2515_H

#include "stm32u5xx_hal.h"
#include <stdint.h>

/* MCP2515 SPI Commands */
#define MCP2515_CMD_RESET       0xC0
#define MCP2515_CMD_READ        0x03
#define MCP2515_CMD_WRITE       0x02
#define MCP2515_CMD_RTS         0x80
#define MCP2515_CMD_READ_STATUS 0xA0
#define MCP2515_CMD_BIT_MODIFY  0x05

/* MCP2515 Registers */
#define MCP2515_REG_CANSTAT     0x0E
#define MCP2515_REG_CANCTRL     0x0F
#define MCP2515_REG_TEC         0x1C  // Transmit Error Counter
#define MCP2515_REG_REC         0x1D  // Receive Error Counter
#define MCP2515_REG_CNF3        0x28
#define MCP2515_REG_CNF2        0x29
#define MCP2515_REG_CNF1        0x2A
#define MCP2515_REG_CANINTE     0x2B
#define MCP2515_REG_CANINTF     0x2C
#define MCP2515_REG_EFLG        0x2D  // Error Flags
#define MCP2515_REG_TXB0CTRL    0x30
#define MCP2515_REG_TXB0SIDH    0x31
#define MCP2515_REG_TXB0SIDL    0x32
#define MCP2515_REG_TXB0DLC     0x35
#define MCP2515_REG_TXB0DATA    0x36
#define MCP2515_REG_RXB0CTRL    0x60
#define MCP2515_REG_RXB0SIDH    0x61
#define MCP2515_REG_RXB0SIDL    0x62
#define MCP2515_REG_RXB0EID8    0x63
#define MCP2515_REG_RXB0EID0    0x64
#define MCP2515_REG_RXB0DLC     0x65
#define MCP2515_REG_RXB0DATA    0x66
#define MCP2515_REG_RXB1CTRL    0x70  // RX Buffer 1 Control
#define MCP2515_REG_RXB1SIDH    0x71
#define MCP2515_REG_RXB1SIDL    0x72
#define MCP2515_REG_RXB1EID8    0x73
#define MCP2515_REG_RXB1EID0    0x74
#define MCP2515_REG_RXB1DLC     0x75
#define MCP2515_REG_RXB1DATA    0x76

/* MCP2515 Modes */
#define MCP2515_MODE_NORMAL     0x00
#define MCP2515_MODE_SLEEP      0x20
#define MCP2515_MODE_LOOPBACK   0x40
#define MCP2515_MODE_LISTENONLY 0x60
#define MCP2515_MODE_CONFIG     0x80

/* CAN Speed Configuration (for 8MHz crystal) */
typedef enum {
    CAN_SPEED_500KBPS = 0,
    CAN_SPEED_250KBPS = 1,
    CAN_SPEED_125KBPS = 2,
    CAN_SPEED_100KBPS = 3
} MCP2515_Speed_t;

/* Function Prototypes */

/**
 * @brief Initialize the MCP2515 CAN controller.
 *
 * Configures the MCP2515 registers for the specified CAN bus bitrate, sets
 * operating mode to NORMAL and verifies initialization via CANSTAT register
 * reads.
 *
 * Responsibilities:
 * - Configure CNF registers for the requested bitrate
 * - Set NORMAL mode
 * - Verify proper operation by reading CANSTAT
 *
 * @param speed CAN speed selection (see MCP2515_Speed_t)
 *
 * Requirement traceability:
 * [impl->dsn~can-init~1]
 *
 * @return HAL_StatusTypeDef HAL_OK on success, otherwise HAL_ERROR / other HAL status
 */
HAL_StatusTypeDef MCP2515_Init(MCP2515_Speed_t speed);

/**
 * @brief Send a generic CAN message (transmit).
 *
 * Transmits a CAN frame with the specified CAN identifier and payload. The
 * implementation confirms transmission completion via MCP2515 status polling
 * and follows error handling policies for timeouts and retries.
 *
 * Behavior:
 * - Clamps/validates payload values where applicable
 * - Confirms TX completion and reports errors
 * - Honors configured rate-limiting for telemetry
 *
 * @param can_id 11-bit or 29-bit CAN identifier depending on device configuration
 * @param data Pointer to payload bytes to send (maximum 8 bytes)
 * @param length Number of payload bytes (0..8)
 *
 * @par Requirement traceability:
 * [impl->dsn~can-telemetry-tx~1]
 * [impl->dsn~can-error-handling~1]
 * [impl->dsn~can-rate-limit~1]
 * [impl->dsn~can-logging-policy~1]
 *
 * @return HAL_StatusTypeDef HAL_OK on success, otherwise HAL_ERROR
 */
HAL_StatusTypeDef MCP2515_SendMessage(uint16_t can_id, uint8_t *data, uint8_t length);  // Generic CAN send

/**
 * @brief Send battery percentage over CAN using a predefined message format.
 *
 * Convenience wrapper that constructs a battery status message and sends it.
 * The message format and CAN ID follow project telemetry specification.
 *
 * @param percentage Battery charge percentage (0..100)
 *
 * Requirement traceability:
 * [impl->dsn~can-telemetry-tx~1]
 *
 * @return HAL_StatusTypeDef HAL_OK on success, otherwise HAL_ERROR
 */
HAL_StatusTypeDef MCP2515_SendBattery(uint8_t percentage);  // Convenience wrapper for battery data

/**
 * @brief Send speed data over CAN
 *
 * Convenience wrapper that converts a float speed (m/s) to the single-byte
 * telemetry format (decimeters/second) and transmits using the MCP2515.
 *
 * Requirement traceability:
 * [impl->dsn~can-telemetry-tx~1]
 *
 * @param speed_ms Vehicle speed in meters per second
 * @return HAL_StatusTypeDef HAL_OK on success, otherwise HAL_ERROR
 */
HAL_StatusTypeDef MCP2515_SendSpeed(float speed_ms);  // Convenience wrapper for speed data

/**
 * @brief Set MCP2515 operating mode.
 *
 * Switches between normal, sleep, loopback, listen-only and config modes.
 * This is used during initialization and in diagnostic/test flows.
 *
 * @param mode One of MCP2515_MODE_* values
 *
 * Requirement traceability:
 * [impl->dsn~can-init~1]
 *
 * @return HAL_StatusTypeDef HAL_OK on success, otherwise HAL_ERROR
 */
HAL_StatusTypeDef MCP2515_SetMode(uint8_t mode);

/**
 * @brief Try to receive and parse a CAN message from the MCP2515.
 *
 * Reads a pending message from the device if available and parses supported
 * telemetry IDs (e.g., speed, battery) updating the in-memory state.
 *
 * @param can_id Pointer to variable to receive the CAN identifier
 * @param data Pointer to buffer that receives payload bytes (should be at least 8 bytes)
 * @param length Pointer to variable that will receive the payload length
 *
 * Requirement traceability:
 * [impl->dsn~can-telemetry-rx~1]
 *
 * @return int Returns 1 if a message was received, 0 if no message is available, negative on error
 */
int MCP2515_ReceiveMessage(uint32_t *can_id, uint8_t *data, uint8_t *length);  // Returns 1 if message received, 0 otherwise

/**
 * @brief Poll and handle received messages.
 *
 * Convenience routine called periodically to check for new messages, parse
 * them and dispatch updates to the rest of the system.
 *
 * Requirement traceability:
 * [impl->dsn~can-telemetry-rx~1]
 * [impl->dsn~can-error-handling~1]
 */
void MCP2515_CheckForMessages(void);  // Check and print any received messages

/**
 * @brief Print basic MCP2515 status to debug console.
 *
 * Controlled by the communications logging policy macros; disabled in
 * production builds unless explicitly enabled.
 *
 * Requirement traceability:
 * [impl->dsn~can-logging-policy~1]
 */
void MCP2515_PrintStatus(void);  // Diagnostic function

/**
 * @brief Print detailed MCP2515 status including RX buffer contents.
 *
 * Controlled by the communications logging policy macros; disabled in
 * production builds unless explicitly enabled.
 *
 * Requirement traceability:
 * [impl->dsn~can-logging-policy~1]
 */
void MCP2515_PrintDetailedStatus(void);  // Verbose diagnostic with RX buffer check

/**
 * @brief Run self-test to verify SPI connection and basic functionality.
 *
 * Useful for manufacturing checks and diagnostics; typically executed in
 * dedicated test flows.
 *
 * Requirement traceability:
 * [impl->dsn~can-init~1]
 */
void MCP2515_TestConnection(void);  // Test SPI connection

/* Internal Functions (exposed for debugging) */

/**
 * @brief Reset MCP2515 via the reset SPI command.
 *
 * Used during initialization and error recovery to force device reset.
 *
 * Requirement traceability:
 * [impl->dsn~can-init~1]
 */
void MCP2515_Reset(void);

/**
 * @brief Read a register from the MCP2515.
 *
 * @param address Register address to read
 * @return uint8_t Register value
 */
uint8_t MCP2515_ReadRegister(uint8_t address);

/**
 * @brief Write a value to a MCP2515 register.
 *
 * @param address Register address to write
 * @param value Value to write into the register
 */
void MCP2515_WriteRegister(uint8_t address, uint8_t value);

/**
 * @brief Modify bits in a MCP2515 register using the bit modify command.
 *
 * @param address Register address
 * @param mask Bit mask specifying bits to modify
 * @param value New bits to write where mask bits are set
 */
void MCP2515_BitModify(uint8_t address, uint8_t mask, uint8_t value);

#endif /* MCP2515_H */
