#ifndef DRIVERS_H
#define DRIVERS_H

#include "main.h"

// Register addresses
#define PCA9685_MODE1      0x00
#define PCA9685_MODE2      0x01
#define PCA9685_PRESCALE   0xFE
#define PCA9685_LED0_ON_L  0x06
#define PCA9685_ALLLED_ON_L  0xFA
#define PCA9685_ALLLED_ON_H  0xFB
#define PCA9685_ALLLED_OFF_L 0xFC
#define PCA9685_ALLLED_OFF_H 0xFD
#define PCA9685_LED0_ON_L      0x06

// PCA9685 Addresses (8-bit format: 7-bit address << 1)
// Scanner finds these at 7-bit 0x40 and 0x60
#define PCA9685_ADDR_STEERING  (0x40 << 1)  // = 0x80
#define PCA9685_ADDR_THROTTLE  (0x60 << 1)  // = 0xC0

/**
 * @brief Initialize a PCA9685 device with a software reset and setup for 50Hz PWM
 *
 * Performs software reset (General Call), applies sleep + auto-increment, sets
 * prescaler for 50Hz and wakes the device. Intended for single-device initialization.
 *
 * @param hi2c Pointer to I2C handle used to talk to PCA9685
 * @param device_addr 8-bit I2C device address (7-bit << 1 format)
 * @param device_name Human-readable device name used in debug prints
 *
 * Requirement traceability:
 *
 * @return HAL_StatusTypeDef HAL_OK on success, otherwise HAL_ERROR / other HAL status
 */
HAL_StatusTypeDef PCA9685_Init_Device(I2C_HandleTypeDef *hi2c, uint8_t device_addr, const char* device_name);

/**
 * @brief Initialize two PCA9685 devices using a single software reset
 *
 * Issues a global software reset (General Call), then initializes two devices
 * sequentially without additional resets. Useful when two PCA9685 chips share
 * the same I2C bus and a single reset is desired.
 *
 * @param hi2c Pointer to I2C handle
 * @param addr1 8-bit address of first PCA9685 device
 * @param name1 Human-readable name for first device
 * @param addr2 8-bit address of second PCA9685 device
 * @param name2 Human-readable name for second device
 *
 * Requirement traceability:
 *
 * @return HAL_StatusTypeDef HAL_OK on success, otherwise HAL_ERROR / other HAL status
 */
HAL_StatusTypeDef PCA9685_Init_Multiple(I2C_HandleTypeDef *hi2c, uint8_t addr1, const char* name1, uint8_t addr2, const char* name2);

/**
 * @brief Set PWM (on/off) values for a PCA9685 channel
 *
 * Writes the four registers (ON_L/ON_H/OFF_L/OFF_H) for the requested channel.
 * This is a low-level primitive used by higher-level servo/throttle code.
 *
 * @param hi2c Pointer to I2C handle
 * @param device_addr 8-bit I2C device address
 * @param channel Channel number (0..15)
 * @param on 12-bit ON time
 * @param off 12-bit OFF time
 *
 * Requirement traceability:
 *
 * @return HAL_StatusTypeDef HAL_OK on success, otherwise HAL_ERROR / other HAL status
 */
HAL_StatusTypeDef PCA9685_SetPWM(I2C_HandleTypeDef *hi2c, uint8_t device_addr, uint8_t channel, uint16_t on, uint16_t off);


#endif
