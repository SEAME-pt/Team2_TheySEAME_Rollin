/**
 ******************************************************************************
 * @file    mcp2515.c
 * @brief   MCP2515 CAN Controller Driver Implementation (Software SPI)
 * @note    Uses GPIO pins: PD15(CS), PF13(SCK), PD8(MOSI), PD9(MISO)
 ******************************************************************************
 */

#include "mcp2515.h"
#include "main.h"
#include <string.h>
#include <stdio.h>

/* GPIO Pin Definitions (from STM32CubeMX generated main.h) */
#define MCP2515_CS_LOW()    HAL_GPIO_WritePin(MCP2515_CS_GPIO_Port, MCP2515_CS_Pin, GPIO_PIN_RESET)
#define MCP2515_CS_HIGH()   HAL_GPIO_WritePin(MCP2515_CS_GPIO_Port, MCP2515_CS_Pin, GPIO_PIN_SET)
#define MCP2515_SCK_LOW()   HAL_GPIO_WritePin(MCP2515_SCK_GPIO_Port, MCP2515_SCK_Pin, GPIO_PIN_RESET)
#define MCP2515_SCK_HIGH()  HAL_GPIO_WritePin(MCP2515_SCK_GPIO_Port, MCP2515_SCK_Pin, GPIO_PIN_SET)
#define MCP2515_MOSI_LOW()  HAL_GPIO_WritePin(MCP2515_MOSI_GPIO_Port, MCP2515_MOSI_Pin, GPIO_PIN_RESET)
#define MCP2515_MOSI_HIGH() HAL_GPIO_WritePin(MCP2515_MOSI_GPIO_Port, MCP2515_MOSI_Pin, GPIO_PIN_SET)
#define MCP2515_MISO_READ() HAL_GPIO_ReadPin(MCP2515_MISO_GPIO_Port, MCP2515_MISO_Pin)

/* Delay for SPI timing (adjust based on clock speed) */
#define SPI_DELAY() do { \
    for(volatile int i = 0; i < 10; i++) __NOP(); \
} while(0)

/*
 * @brief
 *
 * Description
 *
 * ====================== Requirement Traceability ===========================
 * impl->dsn~
 * ==========================================================================
 *
 * @param name         Function
 *
 * @return HAL_StatusTypeDef
 *         - HAL_OK     : Write successful
 *         - HAL_ERROR  : Transmission failed
 *         - HAL_BUSY   : I2C peripheral is busy
 *         - HAL_TIMEOUT: Communication timeout
 *
 */
static uint8_t MCP2515_SPI_Transfer(uint8_t data) {
    uint8_t received = 0;
    
    for (int i = 7; i >= 0; i--) {
        // Set MOSI
        if (data & (1 << i)) {
            MCP2515_MOSI_HIGH();
        } else {
            MCP2515_MOSI_LOW();
        }
        SPI_DELAY();
        
        // Clock pulse
        MCP2515_SCK_HIGH();
        SPI_DELAY();
        
        // Read MISO
        if (MCP2515_MISO_READ() == GPIO_PIN_SET) {
            received |= (1 << i);
        }
        
        MCP2515_SCK_LOW();
        SPI_DELAY();
    }
    
    return received;
}

/**
 * @brief Reset MCP2515 via SPI command
 */
void MCP2515_Reset(void) {
    MCP2515_CS_LOW();
    MCP2515_SPI_Transfer(MCP2515_CMD_RESET);
    MCP2515_CS_HIGH();
    HAL_Delay(10);  // Wait for reset to complete
}

/**
 * @brief Read a single register from MCP2515
 * @param address Register address
 * @return Register value
 */
uint8_t MCP2515_ReadRegister(uint8_t address) {
    uint8_t value;
    
    MCP2515_CS_LOW();
    MCP2515_SPI_Transfer(MCP2515_CMD_READ);
    MCP2515_SPI_Transfer(address);
    value = MCP2515_SPI_Transfer(0x00);
    MCP2515_CS_HIGH();
    
    return value;
}

/**
 * @brief Write a single register to MCP2515
 * @param address Register address
 * @param value Value to write
 */
void MCP2515_WriteRegister(uint8_t address, uint8_t value) {
    MCP2515_CS_LOW();
    MCP2515_SPI_Transfer(MCP2515_CMD_WRITE);
    MCP2515_SPI_Transfer(address);
    MCP2515_SPI_Transfer(value);
    MCP2515_CS_HIGH();
}

/**
 * @brief Modify specific bits in a register
 * @param address Register address
 * @param mask Bit mask
 * @param value New value for masked bits
 */
void MCP2515_BitModify(uint8_t address, uint8_t mask, uint8_t value) {
    MCP2515_CS_LOW();
    MCP2515_SPI_Transfer(MCP2515_CMD_BIT_MODIFY);
    MCP2515_SPI_Transfer(address);
    MCP2515_SPI_Transfer(mask);
    MCP2515_SPI_Transfer(value);
    MCP2515_CS_HIGH();
}

/**
 * @brief Set MCP2515 operating mode
 * @param mode Operating mode (CONFIG, NORMAL, etc.)
 * @return HAL_OK if mode set successfully
 */
HAL_StatusTypeDef MCP2515_SetMode(uint8_t mode) {
    MCP2515_BitModify(MCP2515_REG_CANCTRL, 0xE0, mode);
    HAL_Delay(10);
    
    // Verify mode change
    uint8_t canstat = MCP2515_ReadRegister(MCP2515_REG_CANSTAT);
    if ((canstat & 0xE0) == mode) {
        return HAL_OK;
    }
    
    return HAL_ERROR;
}

/**
 * @brief Initialize MCP2515 CAN controller
 * @param speed CAN bus speed (500kbps, 250kbps, etc.)
 * @return HAL_OK if initialization successful
 */
HAL_StatusTypeDef MCP2515_Init(MCP2515_Speed_t speed) {
    // Initialize CS high
    MCP2515_CS_HIGH();
    MCP2515_SCK_LOW();
    HAL_Delay(10);
    
    // Reset MCP2515
    MCP2515_Reset();
    HAL_Delay(10);
    
    // Set configuration mode
    if (MCP2515_SetMode(MCP2515_MODE_CONFIG) != HAL_OK) {
        return HAL_ERROR;
    }
    
    // Configure CAN bit timing for 8MHz crystal
    // Speed: 500kbps = 8MHz / (2*(BRP+1)*(1+PROP+PS1+PS2))
    // 500kbps needs 8 TQ total: 8MHz / (2*1*8) = 500kHz
    switch (speed) {
        case CAN_SPEED_500KBPS:
            // FOR 8MHz CRYSTAL (most common on HW-184)
            MCP2515_WriteRegister(MCP2515_REG_CNF1, 0x00);  // BRP=0, SJW=1 (2TQ)
            MCP2515_WriteRegister(MCP2515_REG_CNF2, 0x90);  // BTLMODE=1, SAM=0, PS1=2, PRSEG=1
            MCP2515_WriteRegister(MCP2515_REG_CNF3, 0x02);  // WAKFIL=0, PS2=3
            
            // IF YOUR MODULE HAS 16MHz CRYSTAL, UNCOMMENT THESE INSTEAD:
            // MCP2515_WriteRegister(MCP2515_REG_CNF1, 0x00);  // BRP=0
            // MCP2515_WriteRegister(MCP2515_REG_CNF2, 0xF0);  // BTLMODE=1, SAM=0, PS1=7, PRSEG=1
            // MCP2515_WriteRegister(MCP2515_REG_CNF3, 0x86);  // PS2=7
            break;
            
        case CAN_SPEED_250KBPS:
            MCP2515_WriteRegister(MCP2515_REG_CNF1, 0x01);  // BRP=1
            MCP2515_WriteRegister(MCP2515_REG_CNF2, 0xF0);
            MCP2515_WriteRegister(MCP2515_REG_CNF3, 0x86);
            break;
            
        case CAN_SPEED_125KBPS:
            MCP2515_WriteRegister(MCP2515_REG_CNF1, 0x03);  // BRP=3
            MCP2515_WriteRegister(MCP2515_REG_CNF2, 0xF0);
            MCP2515_WriteRegister(MCP2515_REG_CNF3, 0x86);
            break;
            
        case CAN_SPEED_100KBPS:
            MCP2515_WriteRegister(MCP2515_REG_CNF1, 0x04);  // BRP=4
            MCP2515_WriteRegister(MCP2515_REG_CNF2, 0xF0);
            MCP2515_WriteRegister(MCP2515_REG_CNF3, 0x86);
            break;
            
        default:
            return HAL_ERROR;
    }
    
    // Disable interrupts (polling mode)
    MCP2515_WriteRegister(MCP2515_REG_CANINTE, 0x00);
    
    // Clear interrupt flags
    MCP2515_WriteRegister(MCP2515_REG_CANINTF, 0x00);
    
    // Configure RX buffers to accept ALL messages (disable filtering)
    // RXB0CTRL: Turn off mask/filters (RXM[1:0] = 11 = receive any message)
    MCP2515_WriteRegister(MCP2515_REG_RXB0CTRL, 0x60);  // RXM1=1, RXM0=1 (accept all)
    MCP2515_WriteRegister(MCP2515_REG_RXB1CTRL, 0x60);  // RXM1=1, RXM0=1 (accept all)
    
    // Set to normal mode
    if (MCP2515_SetMode(MCP2515_MODE_NORMAL) != HAL_OK) {
        return HAL_ERROR;
    }
    
    return HAL_OK;
}

/**
 * @brief Send a generic CAN message
 * @param can_id CAN identifier (11-bit standard ID)
 * @param data Pointer to data buffer
 * @param length Data length (0-8 bytes)
 * @return HAL_OK if message sent successfully, HAL_ERROR if invalid params, HAL_BUSY if buffer busy, HAL_TIMEOUT if timeout
 */
HAL_StatusTypeDef MCP2515_SendMessage(uint16_t can_id, uint8_t *data, uint8_t length) {
    extern UART_HandleTypeDef huart1;
    char buffer[200];
    uint8_t tx_ctrl;
    uint8_t sidh_readback;
    uint8_t sidl_readback;
    uint8_t dlc_readback;
    uint8_t canstat_pre;
    uint8_t canctrl_pre;
    uint32_t start_tick;
    uint32_t timeout;
    int poll_count;
    uint32_t elapsed;
    uint8_t eflg;
    uint8_t canintf;
    uint8_t i;
    
    // Validate parameters
    if (can_id > 0x7FF || length > 8 || data == NULL) {
        return HAL_ERROR;
    }
    
    // Check TX buffer 0 status
    tx_ctrl = MCP2515_ReadRegister(MCP2515_REG_TXB0CTRL);
    if (tx_ctrl & 0x08) {
        // TX buffer busy - abort previous transmission
        snprintf(buffer, sizeof(buffer), "[DEBUG] TX buffer busy (TXCTRL=0x%02X), aborting...\r\n", tx_ctrl);
        HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 100);
        MCP2515_BitModify(MCP2515_REG_TXB0CTRL, 0x08, 0x00);  // Clear TXREQ
        HAL_Delay(1);
        return HAL_BUSY;
    }
    
    // Set standard ID (11-bit)
    MCP2515_WriteRegister(MCP2515_REG_TXB0SIDH, (uint8_t)(can_id >> 3));
    MCP2515_WriteRegister(MCP2515_REG_TXB0SIDL, (uint8_t)(can_id << 5));
    
    // Set data length code
    MCP2515_WriteRegister(MCP2515_REG_TXB0DLC, length);
    
    // Set data bytes
    for (i = 0; i < length; i++) {
        MCP2515_WriteRegister(MCP2515_REG_TXB0DATA + i, data[i]);
    }
    
    // Verify what was written
    sidh_readback = MCP2515_ReadRegister(MCP2515_REG_TXB0SIDH);
    sidl_readback = MCP2515_ReadRegister(MCP2515_REG_TXB0SIDL);
    dlc_readback = MCP2515_ReadRegister(MCP2515_REG_TXB0DLC);
    
    snprintf(buffer, sizeof(buffer), "[DEBUG] TX Buffer verify: SIDH=0x%02X SIDL=0x%02X DLC=0x%02X\r\n",
        sidh_readback, sidl_readback, dlc_readback);
    HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 100);
    
    snprintf(buffer, sizeof(buffer), "[DEBUG] Sending CAN ID 0x%03X, %d bytes\r\n", can_id, length);
    HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 100);
    
    // Check mode before transmission
    canstat_pre = MCP2515_ReadRegister(MCP2515_REG_CANSTAT);
    canctrl_pre = MCP2515_ReadRegister(MCP2515_REG_CANCTRL);
    
    snprintf(buffer, sizeof(buffer), "[DEBUG] Pre-TX: CANSTAT=0x%02X CANCTRL=0x%02X\r\n", canstat_pre, canctrl_pre);
    HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 100);
    
    // Request transmission
    MCP2515_BitModify(MCP2515_REG_TXB0CTRL, 0x08, 0x08);
    
    // Wait for transmission to complete (timeout: 100ms)
    start_tick = HAL_GetTick();
    timeout = start_tick + 100;
    poll_count = 0;
    
    while (HAL_GetTick() < timeout) {
        tx_ctrl = MCP2515_ReadRegister(MCP2515_REG_TXB0CTRL);
        poll_count++;
        
        if (!(tx_ctrl & 0x08)) {
            // Transmission complete
            elapsed = HAL_GetTick() - start_tick;
            eflg = MCP2515_ReadRegister(MCP2515_REG_EFLG);
            canintf = MCP2515_ReadRegister(MCP2515_REG_CANINTF);
            
            snprintf(buffer, sizeof(buffer), 
                "[DEBUG] TX complete in %lums (%d polls), EFLG=0x%02X, CANINTF=0x%02X\r\n",
                elapsed, poll_count, eflg, canintf);
            HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 100);
            
            if (eflg != 0) {
                MCP2515_WriteRegister(MCP2515_REG_EFLG, 0x00);
            }
            return HAL_OK;
        }
        HAL_Delay(1);
    }
    
    // Timeout - detailed error reporting
    uint8_t tec;
    uint8_t rec;
    eflg = MCP2515_ReadRegister(MCP2515_REG_EFLG);
    canintf = MCP2515_ReadRegister(MCP2515_REG_CANINTF);
    tec = MCP2515_ReadRegister(MCP2515_REG_TEC);
    rec = MCP2515_ReadRegister(MCP2515_REG_REC);
    
    snprintf(buffer, sizeof(buffer), 
        "[ERROR] TX timeout after 100ms (%d polls), TXCTRL=0x%02X, EFLG=0x%02X, CANINTF=0x%02X, TEC=%d, REC=%d\r\n",
        poll_count, tx_ctrl, eflg, canintf, tec, rec);
    HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 100);
    
    return HAL_TIMEOUT;
}

/** * @brief Send speed data over CAN
 * @param speed_ms Vehicle speed in meters per second
 * @return HAL_OK if message sent successfully
 * @note CAN Message Format (4 bytes - float):
 *       Bytes 0-3: Speed as IEEE 754 float (m/s)
 */
HAL_StatusTypeDef MCP2515_SendSpeed(float speed_ms) {
    extern UART_HandleTypeDef huart1;
    char buffer[200];
    
    // CAN ID: 0x42 (66 decimal - Speed status message expected by cluster)
    uint16_t can_id = 0x42;
    
    // Check TX buffer 0 status
    uint8_t tx_ctrl = MCP2515_ReadRegister(MCP2515_REG_TXB0CTRL);
    if (tx_ctrl & 0x08) {
        // TX buffer busy - abort previous transmission
        snprintf(buffer, sizeof(buffer), "[DEBUG] TX buffer busy (TXCTRL=0x%02X), aborting...\r\n", tx_ctrl);
        HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 100);
        MCP2515_BitModify(MCP2515_REG_TXB0CTRL, 0x08, 0x00);  // Clear TXREQ
        HAL_Delay(1);
        return HAL_BUSY;
    }
    
    // Set standard ID (11-bit)
    MCP2515_WriteRegister(MCP2515_REG_TXB0SIDH, (uint8_t)(can_id >> 3));
    MCP2515_WriteRegister(MCP2515_REG_TXB0SIDL, (uint8_t)(can_id << 5));
    
    // Set data length code (1 byte - cluster expects single byte)
    MCP2515_WriteRegister(MCP2515_REG_TXB0DLC, 0x01);
    
    // Convert m/s to dm/s (decimeters per second): 1 m/s = 10 dm/s
    float speed_dms = speed_ms * 10.0f;
    
    // Send speed in dm/s as single byte (0-255), clamp the value
    uint8_t speed_byte = (uint8_t)(speed_dms > 255.0f ? 255 : (speed_dms < 0 ? 0 : speed_dms));
    
    // Write speed data (1 byte only)
    MCP2515_WriteRegister(MCP2515_REG_TXB0DATA, speed_byte);
    
    // Verify what was written
    uint8_t sidh_readback = MCP2515_ReadRegister(MCP2515_REG_TXB0SIDH);
    uint8_t sidl_readback = MCP2515_ReadRegister(MCP2515_REG_TXB0SIDL);
    uint8_t dlc_readback = MCP2515_ReadRegister(MCP2515_REG_TXB0DLC);
    
    snprintf(buffer, sizeof(buffer), "[DEBUG] TX Buffer verify: SIDH=0x%02X SIDL=0x%02X DLC=0x%02X\r\n",
        sidh_readback, sidl_readback, dlc_readback);
    HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 100);
    
    snprintf(buffer, sizeof(buffer), "[DEBUG] Sending CAN ID 0x%03X: Speed=%.2f m/s (%.1f dm/s, byte=%d)\r\n", 
            can_id, speed_ms, speed_dms, speed_byte);
    HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 100);
    
    // Check mode before transmission
    uint8_t canstat_pre = MCP2515_ReadRegister(MCP2515_REG_CANSTAT);
    uint8_t canctrl_pre = MCP2515_ReadRegister(MCP2515_REG_CANCTRL);
    snprintf(buffer, sizeof(buffer), "[DEBUG] Pre-TX: CANSTAT=0x%02X CANCTRL=0x%02X\r\n", canstat_pre, canctrl_pre);
    HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 100);
    
    // Request transmission
    MCP2515_BitModify(MCP2515_REG_TXB0CTRL, 0x08, 0x08);
    
    // Wait for transmission to complete (with timeout)
    uint32_t start_tick = HAL_GetTick();
    int poll_count = 0;
    while (HAL_GetTick() - start_tick < 100) {
        tx_ctrl = MCP2515_ReadRegister(MCP2515_REG_TXB0CTRL);
        poll_count++;
        
        if (!(tx_ctrl & 0x08)) {
            // Transmission complete
            uint32_t elapsed = HAL_GetTick() - start_tick;
            uint8_t eflg = MCP2515_ReadRegister(MCP2515_REG_EFLG);
            uint8_t canintf = MCP2515_ReadRegister(MCP2515_REG_CANINTF);
            
            snprintf(buffer, sizeof(buffer), 
                "[DEBUG] TX complete in %lums (%d polls), EFLG=0x%02X, CANINTF=0x%02X\r\n",
                elapsed, poll_count, eflg, canintf);
            HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 100);
            
            if (eflg != 0) {
                MCP2515_WriteRegister(MCP2515_REG_EFLG, 0x00);
            }
            return HAL_OK;
        }
        HAL_Delay(1);
    }
    
    // Timeout - detailed error reporting
    uint8_t eflg = MCP2515_ReadRegister(MCP2515_REG_EFLG);
    uint8_t canintf = MCP2515_ReadRegister(MCP2515_REG_CANINTF);
    uint8_t tec = MCP2515_ReadRegister(MCP2515_REG_TEC);
    uint8_t rec = MCP2515_ReadRegister(MCP2515_REG_REC);
    
    snprintf(buffer, sizeof(buffer), 
        "[ERROR] Speed TX timeout after 100ms (%d polls), TXCTRL=0x%02X, EFLG=0x%02X, CANINTF=0x%02X, TEC=%d, REC=%d\r\n",
        poll_count, tx_ctrl, eflg, canintf, tec, rec);
    HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 100);
    
    // Clear error and interrupt flags
    MCP2515_WriteRegister(MCP2515_REG_EFLG, 0x00);
    MCP2515_WriteRegister(MCP2515_REG_CANINTF, 0x00);
    
    // Abort the stuck transmission
    MCP2515_BitModify(MCP2515_REG_TXB0CTRL, 0x08, 0x00);  // Clear TXREQ
    
    return HAL_TIMEOUT;
}

/** * @brief Send battery data over CAN bus
 * @param percentage Battery percentage (0-100)
 * @return HAL_OK if message sent successfully
 * @note CAN Message Format (1 byte):
 *       Byte 0: Percentage (0-100)
 */
HAL_StatusTypeDef MCP2515_SendBattery(uint8_t percentage) {
    extern UART_HandleTypeDef huart1;
    char buffer[200];
    
    // CAN ID: 0x4D (Battery status message)
    uint16_t can_id = 0x4d;
    
    // Check TX buffer 0 status
    uint8_t tx_ctrl = MCP2515_ReadRegister(MCP2515_REG_TXB0CTRL);
    if (tx_ctrl & 0x08) {
        // TX buffer busy - abort previous transmission
        snprintf(buffer, sizeof(buffer), "[DEBUG] TX buffer busy (TXCTRL=0x%02X), aborting...\r\n", tx_ctrl);
        HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 100);
        MCP2515_BitModify(MCP2515_REG_TXB0CTRL, 0x08, 0x00);  // Clear TXREQ
        HAL_Delay(1);
        return HAL_BUSY;
    }
    
    // Set standard ID (11-bit)
    MCP2515_WriteRegister(MCP2515_REG_TXB0SIDH, (uint8_t)(can_id >> 3));
    MCP2515_WriteRegister(MCP2515_REG_TXB0SIDL, (uint8_t)(can_id << 5));
    
    // Set data length code (1 byte)
    MCP2515_WriteRegister(MCP2515_REG_TXB0DLC, 0x01);
    
    // Set battery percentage data (single byte)
    MCP2515_WriteRegister(MCP2515_REG_TXB0DATA, percentage);
    
    // Verify what was written
    uint8_t sidh_readback = MCP2515_ReadRegister(MCP2515_REG_TXB0SIDH);
    uint8_t sidl_readback = MCP2515_ReadRegister(MCP2515_REG_TXB0SIDL);
    uint8_t dlc_readback = MCP2515_ReadRegister(MCP2515_REG_TXB0DLC);
    
    snprintf(buffer, sizeof(buffer), "[DEBUG] TX Buffer verify: SIDH=0x%02X SIDL=0x%02X DLC=0x%02X\r\n",
        sidh_readback, sidl_readback, dlc_readback);
    HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 100);
    
    snprintf(buffer, sizeof(buffer), "[DEBUG] Sending CAN ID 0x%03X: Battery=%d%%\r\n", 
            can_id, percentage);
    HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 100);
    
    // Check mode before transmission
    uint8_t canstat_pre = MCP2515_ReadRegister(MCP2515_REG_CANSTAT);
    uint8_t canctrl_pre = MCP2515_ReadRegister(MCP2515_REG_CANCTRL);
    snprintf(buffer, sizeof(buffer), "[DEBUG] Pre-TX: CANSTAT=0x%02X CANCTRL=0x%02X\r\n", canstat_pre, canctrl_pre);
    HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 100);
    
    // Request transmission
    MCP2515_BitModify(MCP2515_REG_TXB0CTRL, 0x08, 0x08);
    
    // Wait for transmission to complete (with timeout)
    uint32_t start_tick = HAL_GetTick();
    int poll_count = 0;
    while (HAL_GetTick() - start_tick < 100) {
        tx_ctrl = MCP2515_ReadRegister(MCP2515_REG_TXB0CTRL);
        poll_count++;
        
        if (!(tx_ctrl & 0x08)) {
            // Transmission complete
            uint32_t elapsed = HAL_GetTick() - start_tick;
            uint8_t eflg = MCP2515_ReadRegister(MCP2515_REG_EFLG);
            uint8_t canintf = MCP2515_ReadRegister(MCP2515_REG_CANINTF);
            
            snprintf(buffer, sizeof(buffer), 
                "[DEBUG] TX complete in %lums (%d polls), EFLG=0x%02X, CANINTF=0x%02X\r\n",
                elapsed, poll_count, eflg, canintf);
            HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 100);
            
            if (eflg != 0) {
                MCP2515_WriteRegister(MCP2515_REG_EFLG, 0x00);
            }
            return HAL_OK;
        }
        HAL_Delay(1);
    }
    
    // Timeout - detailed error reporting
    uint8_t eflg = MCP2515_ReadRegister(MCP2515_REG_EFLG);
    uint8_t canintf = MCP2515_ReadRegister(MCP2515_REG_CANINTF);
    uint8_t tec = MCP2515_ReadRegister(MCP2515_REG_TEC);
    uint8_t rec = MCP2515_ReadRegister(MCP2515_REG_REC);
    
    snprintf(buffer, sizeof(buffer), 
        "[ERROR] TX timeout after 100ms (%d polls), TXCTRL=0x%02X, EFLG=0x%02X, CANINTF=0x%02X, TEC=%d, REC=%d\r\n",
        poll_count, tx_ctrl, eflg, canintf, tec, rec);
    HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 100);
    
    // Clear error and interrupt flags
    MCP2515_WriteRegister(MCP2515_REG_EFLG, 0x00);
    MCP2515_WriteRegister(MCP2515_REG_CANINTF, 0x00);
    
    // Abort the stuck transmission
    MCP2515_BitModify(MCP2515_REG_TXB0CTRL, 0x08, 0x00);  // Clear TXREQ
    
    return HAL_TIMEOUT;
}

/**
 * @brief Print MCP2515 status registers for debugging
 */
void MCP2515_PrintStatus(void) {
    extern UART_HandleTypeDef huart1;
    char buffer[150];
    
    uint8_t canstat = MCP2515_ReadRegister(MCP2515_REG_CANSTAT);
    uint8_t canctrl = MCP2515_ReadRegister(MCP2515_REG_CANCTRL);
    uint8_t eflg = MCP2515_ReadRegister(MCP2515_REG_EFLG);
    uint8_t tec = MCP2515_ReadRegister(MCP2515_REG_TEC);
    uint8_t rec = MCP2515_ReadRegister(MCP2515_REG_REC);
    uint8_t canintf = MCP2515_ReadRegister(MCP2515_REG_CANINTF);
    
    snprintf(buffer, sizeof(buffer),
        "[MCP2515] CANSTAT=0x%02X CANCTRL=0x%02X EFLG=0x%02X TEC=%d REC=%d CANINTF=0x%02X\r\n",
        canstat, canctrl, eflg, tec, rec, canintf);
    HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 1000);
}

/**
 * @brief Print detailed MCP2515 status with RX buffer check
 */
void MCP2515_PrintDetailedStatus(void) {
    extern UART_HandleTypeDef huart1;
    char buffer[200];
    
    HAL_UART_Transmit(&huart1, (uint8_t*)"\r\n=== DETAILED MCP2515 STATUS ===\r\n", 37, 1000);
    
    // Operating mode
    uint8_t canstat = MCP2515_ReadRegister(MCP2515_REG_CANSTAT);
    uint8_t canctrl = MCP2515_ReadRegister(MCP2515_REG_CANCTRL);
    const char* mode_str = "UNKNOWN";
    switch (canstat & 0xE0) {
        case 0x00: mode_str = "NORMAL"; break;
        case 0x20: mode_str = "SLEEP"; break;
        case 0x40: mode_str = "LOOPBACK"; break;
        case 0x60: mode_str = "LISTEN-ONLY"; break;
        case 0x80: mode_str = "CONFIG"; break;
    }
    snprintf(buffer, sizeof(buffer), "Mode: %s (CANSTAT=0x%02X, CANCTRL=0x%02X)\r\n", 
        mode_str, canstat, canctrl);
    HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 1000);
    
    // Error counters
    uint8_t tec = MCP2515_ReadRegister(MCP2515_REG_TEC);
    uint8_t rec = MCP2515_ReadRegister(MCP2515_REG_REC);
    snprintf(buffer, sizeof(buffer), "Error Counters: TEC=%d, REC=%d\r\n", tec, rec);
    HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 1000);
    
    // Error flags
    uint8_t eflg = MCP2515_ReadRegister(MCP2515_REG_EFLG);
    snprintf(buffer, sizeof(buffer), "Error Flags (EFLG): 0x%02X\r\n", eflg);
    HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 1000);
    if (eflg) {
        if (eflg & 0x80) HAL_UART_Transmit(&huart1, (uint8_t*)"  - RX1OVR\r\n", 14, 1000);
        if (eflg & 0x40) HAL_UART_Transmit(&huart1, (uint8_t*)"  - RX0OVR\r\n", 14, 1000);
        if (eflg & 0x20) HAL_UART_Transmit(&huart1, (uint8_t*)"  - TXBO (Bus-off!)\r\n", 22, 1000);
        if (eflg & 0x10) HAL_UART_Transmit(&huart1, (uint8_t*)"  - TXEP\r\n", 11, 1000);
        if (eflg & 0x08) HAL_UART_Transmit(&huart1, (uint8_t*)"  - RXEP\r\n", 11, 1000);
    }
    
    // Interrupt flags
    uint8_t canintf = MCP2515_ReadRegister(MCP2515_REG_CANINTF);
    snprintf(buffer, sizeof(buffer), "Interrupt Flags (CANINTF): 0x%02X\r\n", canintf);
    HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 1000);
    if (canintf) {
        if (canintf & 0x80) HAL_UART_Transmit(&huart1, (uint8_t*)"  - MERR (Message Error - NO ACK!)\r\n", 38, 1000);
        if (canintf & 0x40) HAL_UART_Transmit(&huart1, (uint8_t*)"  - WAKIF\r\n", 13, 1000);
        if (canintf & 0x20) HAL_UART_Transmit(&huart1, (uint8_t*)"  - ERRIF\r\n", 13, 1000);
        if (canintf & 0x04) HAL_UART_Transmit(&huart1, (uint8_t*)"  - TX0IF\r\n", 13, 1000);
        if (canintf & 0x02) HAL_UART_Transmit(&huart1, (uint8_t*)"  - RX1IF\r\n", 13, 1000);
        if (canintf & 0x01) HAL_UART_Transmit(&huart1, (uint8_t*)"  - RX0IF (Message received!)\r\n", 33, 1000);
    }
    
    // TX buffer status
    uint8_t txb0ctrl = MCP2515_ReadRegister(MCP2515_REG_TXB0CTRL);
    snprintf(buffer, sizeof(buffer), "TX Buffer 0 (TXB0CTRL): 0x%02X %s\r\n", 
        txb0ctrl, (txb0ctrl & 0x08) ? "[PENDING]" : "[EMPTY]");
    HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 1000);
    
    // RX buffer status
    uint8_t rxb0ctrl = MCP2515_ReadRegister(MCP2515_REG_RXB0CTRL);
    snprintf(buffer, sizeof(buffer), "RX Buffer 0 (RXB0CTRL): 0x%02X\r\n", rxb0ctrl);
    HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 1000);
    
    // Check if RX buffer has data
    if (canintf & 0x01) {
        uint8_t sidh = MCP2515_ReadRegister(MCP2515_REG_RXB0SIDH);
        uint8_t sidl = MCP2515_ReadRegister(MCP2515_REG_RXB0SIDL);
        uint8_t dlc = MCP2515_ReadRegister(MCP2515_REG_RXB0DLC);
        uint8_t data = MCP2515_ReadRegister(MCP2515_REG_RXB0DATA);
        uint16_t rx_id = (sidh << 3) | (sidl >> 5);
        
        snprintf(buffer, sizeof(buffer), "RX Message: ID=0x%03X, DLC=%d, Data=0x%02X\r\n", 
            rx_id, dlc & 0x0F, data);
        HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 1000);
    }
    
    // Bit timing configuration
    uint8_t cnf1 = MCP2515_ReadRegister(MCP2515_REG_CNF1);
    uint8_t cnf2 = MCP2515_ReadRegister(MCP2515_REG_CNF2);
    uint8_t cnf3 = MCP2515_ReadRegister(MCP2515_REG_CNF3);
    snprintf(buffer, sizeof(buffer), "Bit Timing: CNF1=0x%02X CNF2=0x%02X CNF3=0x%02X\r\n", 
        cnf1, cnf2, cnf3);
    HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 1000);
    
    HAL_UART_Transmit(&huart1, (uint8_t*)"================================\r\n\r\n", 37, 1000);
}

/**
 * @brief Test MCP2515 SPI connection and print diagnostics
 */
void MCP2515_TestConnection(void) {
    extern UART_HandleTypeDef huart1;
    char buffer[200];
    
    HAL_UART_Transmit(&huart1, (uint8_t*)"\r\n=== MCP2515 CONNECTION TEST ===\r\n", 36, 1000);
    
    // Test 1: Read CANSTAT register multiple times (should be consistent)
    HAL_UART_Transmit(&huart1, (uint8_t*)"[TEST 1] Reading CANSTAT register 5 times:\r\n", 45, 1000);
    for (int i = 0; i < 5; i++) {
        uint8_t canstat = MCP2515_ReadRegister(MCP2515_REG_CANSTAT);
        snprintf(buffer, sizeof(buffer), "  Read %d: CANSTAT = 0x%02X\r\n", i+1, canstat);
        HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 1000);
        HAL_Delay(10);
    }
    
    // Test 2: Write and read back a register (CNF1)
    HAL_UART_Transmit(&huart1, (uint8_t*)"\r\n[TEST 2] Write/Read test on CNF1 register:\r\n", 47, 1000);
    
    // Set config mode first
    MCP2515_SetMode(MCP2515_MODE_CONFIG);
    HAL_Delay(10);
    
    uint8_t test_value = 0xAA;
    MCP2515_WriteRegister(MCP2515_REG_CNF1, test_value);
    HAL_Delay(10);
    uint8_t read_back = MCP2515_ReadRegister(MCP2515_REG_CNF1);
    
    snprintf(buffer, sizeof(buffer), "  Wrote: 0x%02X, Read back: 0x%02X %s\r\n", 
        test_value, read_back, (test_value == read_back) ? "[OK]" : "[FAILED]");
    HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 1000);
    
    // Restore original value
    MCP2515_WriteRegister(MCP2515_REG_CNF1, 0x00);
    
    // Test 3: Check if all-zero pattern works
    HAL_UART_Transmit(&huart1, (uint8_t*)"\r\n[TEST 3] Testing all-zero pattern:\r\n", 39, 1000);
    MCP2515_WriteRegister(MCP2515_REG_CNF1, 0x00);
    HAL_Delay(10);
    read_back = MCP2515_ReadRegister(MCP2515_REG_CNF1);
    snprintf(buffer, sizeof(buffer), "  Wrote: 0x00, Read back: 0x%02X %s\r\n", 
        read_back, (read_back == 0x00) ? "[OK]" : "[FAILED]");
    HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 1000);
    
    // Test 4: Check multiple registers
    HAL_UART_Transmit(&huart1, (uint8_t*)"\r\n[TEST 4] Reading multiple registers:\r\n", 41, 1000);
    snprintf(buffer, sizeof(buffer), 
        "  CANSTAT = 0x%02X\r\n"
        "  CANCTRL = 0x%02X\r\n"
        "  CNF1    = 0x%02X\r\n"
        "  CNF2    = 0x%02X\r\n"
        "  CNF3    = 0x%02X\r\n",
        MCP2515_ReadRegister(MCP2515_REG_CANSTAT),
        MCP2515_ReadRegister(MCP2515_REG_CANCTRL),
        MCP2515_ReadRegister(MCP2515_REG_CNF1),
        MCP2515_ReadRegister(MCP2515_REG_CNF2),
        MCP2515_ReadRegister(MCP2515_REG_CNF3));
    HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 1000);
    
    // Summary
    HAL_UART_Transmit(&huart1, (uint8_t*)"\r\n[DIAGNOSIS]\r\n", 15, 1000);
    HAL_UART_Transmit(&huart1, (uint8_t*)"If all reads return 0xFF: Check CS, SCK, MOSI connections and power\r\n", 70, 1000);
    HAL_UART_Transmit(&huart1, (uint8_t*)"If all reads return 0x00: Check MISO connection\r\n", 50, 1000);
    HAL_UART_Transmit(&huart1, (uint8_t*)"If reads are random: Check all SPI connections\r\n", 49, 1000);
    HAL_UART_Transmit(&huart1, (uint8_t*)"If write/read matches: SPI connection is good!\r\n", 49, 1000);
    HAL_UART_Transmit(&huart1, (uint8_t*)"=== TEST COMPLETE ===\r\n\r\n", 26, 1000);
}

/**
 * @brief Check for received CAN messages and print them
 */
/**
 * @brief  Receive a CAN message from MCP2515
 * @param  can_id: Pointer to store received CAN ID
 * @param  data: Pointer to buffer for received data (must be at least 8 bytes)
 * @param  length: Pointer to store received data length
 * @retval 1 if message received, 0 if no message available
 */
int MCP2515_ReceiveMessage(uint16_t *can_id, uint8_t *data, uint8_t *length) {
    extern UART_HandleTypeDef huart1;
    char debug_buf[80];
    
    // Check CANINTF register for RX flags
    uint8_t canintf = MCP2515_ReadRegister(MCP2515_REG_CANINTF);
    
    // Debug: Print CANINTF every 50 calls to see if RX flags ever set
    static uint32_t poll_count = 0;
    poll_count++;
    if (poll_count % 50 == 0) {
        snprintf(debug_buf, sizeof(debug_buf), "[MCP2515_RX] Poll #%lu, CANINTF=0x%02X\r\n", poll_count, canintf);
        HAL_UART_Transmit(&huart1, (uint8_t*)debug_buf, strlen(debug_buf), 100);
    }
    
    // Check RX Buffer 0
    if (canintf & 0x01) {  // RX0IF flag
        HAL_UART_Transmit(&huart1, (uint8_t*)"[MCP2515_RX] RX0IF SET!\r\n", 25, 100);
        uint8_t sidh = MCP2515_ReadRegister(MCP2515_REG_RXB0SIDH);
        uint8_t sidl = MCP2515_ReadRegister(MCP2515_REG_RXB0SIDL);
        uint8_t dlc = MCP2515_ReadRegister(MCP2515_REG_RXB0DLC);
        
        *can_id = (sidh << 3) | (sidl >> 5);
        *length = dlc & 0x0F;
        
        // Read data bytes
        for (int i = 0; i < *length; i++) {
            data[i] = MCP2515_ReadRegister(MCP2515_REG_RXB0DATA + i);
        }
        
        // Clear the RX0IF flag
        MCP2515_BitModify(MCP2515_REG_CANINTF, 0x01, 0x00);
        
        return 1;  // Message received
    }
    
    // Check RX Buffer 1
    if (canintf & 0x02) {  // RX1IF flag
        HAL_UART_Transmit(&huart1, (uint8_t*)"[MCP2515_RX] RX1IF SET!\r\n", 25, 100);
        uint8_t sidh = MCP2515_ReadRegister(MCP2515_REG_RXB1SIDH);
        uint8_t sidl = MCP2515_ReadRegister(MCP2515_REG_RXB1SIDL);
        uint8_t dlc = MCP2515_ReadRegister(MCP2515_REG_RXB1DLC);
        
        *can_id = (sidh << 3) | (sidl >> 5);
        *length = dlc & 0x0F;
        
        // Read data bytes
        for (int i = 0; i < *length; i++) {
            data[i] = MCP2515_ReadRegister(MCP2515_REG_RXB1DATA + i);
        }
        
        // Clear the RX1IF flag
        MCP2515_BitModify(MCP2515_REG_CANINTF, 0x02, 0x00);
        
        return 1;  // Message received
    }
    
    return 0;  // No message
}

void MCP2515_CheckForMessages(void) {
    extern UART_HandleTypeDef huart1;
    char buffer[150];
    
    uint16_t can_id;
    uint8_t data[8];
    uint8_t length;
    
    if (MCP2515_ReceiveMessage(&can_id, data, &length)) {
        snprintf(buffer, sizeof(buffer), "[RX] Message received! ID=0x%03X, DLC=%d, Data: ", 
            can_id, length);
        HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 100);
        
        // Print data bytes
        for (int i = 0; i < length; i++) {
            snprintf(buffer, sizeof(buffer), "0x%02X ", data[i]);
            HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 100);
        }
        HAL_UART_Transmit(&huart1, (uint8_t*)"\r\n", 2, 100);
    }
}
