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
#define MCP2515_REG_RXB0DLC     0x65
#define MCP2515_REG_RXB0DATA    0x66
#define MCP2515_REG_RXB1CTRL    0x70  // RX Buffer 1 Control
#define MCP2515_REG_RXB1SIDH    0x71
#define MCP2515_REG_RXB1SIDL    0x72
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
HAL_StatusTypeDef MCP2515_Init(MCP2515_Speed_t speed);
HAL_StatusTypeDef MCP2515_SendMessage(uint16_t can_id, uint8_t *data, uint8_t length);  // Generic CAN send
HAL_StatusTypeDef MCP2515_SendBattery(uint8_t percentage);  // Convenience wrapper for battery data
HAL_StatusTypeDef MCP2515_SetMode(uint8_t mode);
int MCP2515_ReceiveMessage(uint16_t *can_id, uint8_t *data, uint8_t *length);  // Returns 1 if message received, 0 otherwise
void MCP2515_CheckForMessages(void);  // Check and print any received messages
void MCP2515_PrintStatus(void);  // Diagnostic function
void MCP2515_PrintDetailedStatus(void);  // Verbose diagnostic with RX buffer check
void MCP2515_TestConnection(void);  // Test SPI connection

/* Internal Functions (exposed for debugging) */
void MCP2515_Reset(void);
uint8_t MCP2515_ReadRegister(uint8_t address);
void MCP2515_WriteRegister(uint8_t address, uint8_t value);
void MCP2515_BitModify(uint8_t address, uint8_t mask, uint8_t value);

#endif /* MCP2515_H */
