#ifndef PCA9685_H
#define PCA9685_H

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

HAL_StatusTypeDef PCA9685_Init_Device(I2C_HandleTypeDef *hi2c, uint8_t device_addr, const char* device_name);
HAL_StatusTypeDef PCA9685_Init_Multiple(I2C_HandleTypeDef *hi2c, uint8_t addr1, const char* name1, uint8_t addr2, const char* name2);
HAL_StatusTypeDef PCA9685_SetPWM(I2C_HandleTypeDef *hi2c, uint8_t device_addr, uint8_t channel, uint16_t on, uint16_t off);


#endif
