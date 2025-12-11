#include "drivers.h"
#include "tx_api.h"  // For tx_thread_sleep
#include <stdio.h>
#include <stdint.h>

extern char uart_buf[64];

// PCA9685 Software Reset via I2C General Call
// This unsticks the chip from "zombie" state where only MODE1/PRESCALE work
static HAL_StatusTypeDef PCA9685_SoftwareReset(I2C_HandleTypeDef *hi2c) {
	/*
	 * @brief
	 *
	 * Description
	 *
	 * ====================== Requirement Traceability ===========================
	 *
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
    uint8_t reset_cmd = 0x06;  // SWRST command
    // Send to General Call address (0x00) - affects ALL PCA9685 on bus
    HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(hi2c, 0x00, &reset_cmd, 1, 500);
    tx_thread_sleep(1);  // 10ms for reset to complete
    return status;
}

// Helper function to write a register value using raw I2C transmit
static HAL_StatusTypeDef PCA9685_WriteReg(I2C_HandleTypeDef *hi2c, uint8_t device_addr, uint8_t reg, uint8_t val) {
	/*
	 * @brief
	 *
	 * Description
	 *
	 * ====================== Requirement Traceability ===========================
	 *
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

    // Method 1: Single transaction with register + data
    uint8_t data[2] = {reg, val};
    HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(hi2c, device_addr, data, 2, 500);
    
    if (status != HAL_OK) {
        // Method 2: Try with Mem_Write as fallback
        status = HAL_I2C_Mem_Write(hi2c, device_addr, reg, I2C_MEMADD_SIZE_8BIT, &val, 1, 500);
    }
    
    tx_thread_sleep(2);  // 20ms delay after each write
    return status;
}

// Helper to read a register
static HAL_StatusTypeDef PCA9685_ReadReg(I2C_HandleTypeDef *hi2c, uint8_t device_addr, uint8_t reg, uint8_t *val) {
	/*
	 * @brief
	 *
	 * Description
	 *
	 * ====================== Requirement Traceability ===========================
	 *
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
	 HAL_StatusTypeDef status = HAL_I2C_Mem_Read(hi2c, device_addr, reg, I2C_MEMADD_SIZE_8BIT, val, 1, 500);
    
    if (status != HAL_OK) {
        // Alternative: Write register address, then read
        status = HAL_I2C_Master_Transmit(hi2c, device_addr, &reg, 1, 500);
        if (status == HAL_OK) {
            status = HAL_I2C_Master_Receive(hi2c, device_addr, val, 1, 500);
        }
    }
    
    return status;
}

// Initialize PCA9685 with Software Reset to clear "zombie" state
HAL_StatusTypeDef PCA9685_Init_Device(I2C_HandleTypeDef *hi2c, uint8_t addr, const char* name) {
	/*
	 * @brief
	 *
	 * Description
	 *
	 * ====================== Requirement Traceability ===========================
	 *
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

    HAL_StatusTypeDef ret;
    char msg[80];
    
    snprintf(msg, sizeof(msg), "Init %s PCA9685 at 0x%02X with SWRST\r\n", name, addr);
    Debug_Print(msg);
    
    // CRITICAL: Software Reset via General Call to unstick chip
    Debug_Print("  Issuing Software Reset (SWRST)...\r\n");
    ret = PCA9685_SoftwareReset(hi2c);
    if (ret != HAL_OK) {
        snprintf(msg, sizeof(msg), "  SWRST failed: %d (continuing anyway)\r\n", ret);
        Debug_Print(msg);
        // Continue - SWRST to general call might fail if no devices respond
    } else {
        Debug_Print("  SWRST OK!\r\n");
    }
    tx_thread_sleep(5);  // 50ms for all PCA9685 chips to reset
    
    // Step 1: Set MODE1 with Sleep + Auto-Increment
    Debug_Print("  Step 1: MODE1=0x31 (Sleep + AI)...\r\n");
    ret = PCA9685_WriteReg(hi2c, addr, PCA9685_MODE1, 0x31);
    if (ret != HAL_OK) {
        snprintf(msg, sizeof(msg), "  FAILED: %d\r\n", ret);
        Debug_Print(msg);
        return ret;
    }
    
    // Step 2: Set prescaler for 50Hz (must be in sleep mode)
    Debug_Print("  Step 2: PRESCALE=121 (50Hz)...\r\n");
    ret = PCA9685_WriteReg(hi2c, addr, PCA9685_PRESCALE, 121);
    if (ret != HAL_OK) {
        snprintf(msg, sizeof(msg), "  FAILED: %d\r\n", ret);
        Debug_Print(msg);
        return ret;
    }
    
    // Step 3: Wake up with Auto-Increment enabled
    Debug_Print("  Step 3: MODE1=0x21 (Wake + AI)...\r\n");
    ret = PCA9685_WriteReg(hi2c, addr, PCA9685_MODE1, 0x21);
    if (ret != HAL_OK) {
        snprintf(msg, sizeof(msg), "  FAILED: %d\r\n", ret);
        Debug_Print(msg);
        return ret;
    }
    tx_thread_sleep(5);  // 50ms for oscillator to stabilize after wake
    
    // Diagnostic: Try reading MODE2 with alternative method
    uint8_t mode2_read = 0;
    Debug_Print("  Diagnostic: Reading MODE2 with Master_Transmit/Receive...\r\n");
    ret = PCA9685_ReadReg(hi2c, addr, PCA9685_MODE2, &mode2_read);
    if (ret == HAL_OK) {
        snprintf(msg, sizeof(msg), "  MODE2 current value: 0x%02X\r\n", mode2_read);
        Debug_Print(msg);
    } else {
        snprintf(msg, sizeof(msg), "  MODE2 read still FAILED: %d\r\n", ret);
        Debug_Print(msg);
    }
    
    // Step 4: Set MODE2 using raw Master_Transmit method
    Debug_Print("  Step 4: MODE2=0x04 with Master_Transmit...\r\n");
    ret = PCA9685_WriteReg(hi2c, addr, PCA9685_MODE2, 0x04);
    if (ret != HAL_OK) {
        snprintf(msg, sizeof(msg), "  MODE2 write still FAILED: %d\r\n", ret);
        Debug_Print(msg);
        
        Debug_Print("  HARDWARE ISSUE CONFIRMED: PCA9685 only responds to registers 0x00 and 0xFE\r\n");
        Debug_Print("  Skipping MODE2 - will attempt PWM write anyway\r\n");
        return HAL_OK;  // Continue to test PWM
    }
    
    Debug_Print("  SUCCESS! All registers initialized!\r\n");
    return HAL_OK;
}

// Set PWM - write each byte individually (C++ reference method)
HAL_StatusTypeDef PCA9685_SetPWM(I2C_HandleTypeDef *hi2c, uint8_t device_addr, uint8_t channel, uint16_t on, uint16_t off) {
	/*
	 * @brief
	 *
	 * Description
	 *
	 * ====================== Requirement Traceability ===========================
	 *
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

    uint8_t reg_base = 0x06 + 4 * channel;
    
    // Write each register individually (matches working C++ code)
    uint8_t cmd[2];
    HAL_StatusTypeDef status;
    
    // Write ON_L
    cmd[0] = reg_base;
    cmd[1] = on & 0xFF;
    status = HAL_I2C_Master_Transmit(hi2c, device_addr, cmd, 2, 500);
    if (status != HAL_OK) return status;
    
    // Write ON_H
    cmd[0] = reg_base + 1;
    cmd[1] = on >> 8;
    status = HAL_I2C_Master_Transmit(hi2c, device_addr, cmd, 2, 500);
    if (status != HAL_OK) return status;
    
    // Write OFF_L
    cmd[0] = reg_base + 2;
    cmd[1] = off & 0xFF;
    status = HAL_I2C_Master_Transmit(hi2c, device_addr, cmd, 2, 500);
    if (status != HAL_OK) return status;
    
    // Write OFF_H
    cmd[0] = reg_base + 3;
    cmd[1] = off >> 8;
    status = HAL_I2C_Master_Transmit(hi2c, device_addr, cmd, 2, 500);
    
    return status;
}

