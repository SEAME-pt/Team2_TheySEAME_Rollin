/*
 * INA219.h
 *
 *  Created on: Dec 30, 2020
 *  Updated on: Jan 29, 2022
 *      Author: Piotr Smolen <komuch@gmail.com>
 *     	Updated by: Brandon Thibeaux <github: thibeaux>
 */

#ifndef INA219_H_
#define INA219_H_

#include <stdbool.h>
#include "stm32u5xx_hal.h"
#include "main.h"

#define INA219_ADDRESS 							(0x41)
#define INA219_ADDRESS_2							(0x40)

//
//	Registers
//
#define	INA219_REG_CONFIG						(0x00)
#define	INA219_REG_SHUNTVOLTAGE					(0x01)
#define	INA219_REG_BUSVOLTAGE					(0x02)
#define	INA219_REG_POWER						(0x03)
#define	INA219_REG_CURRENT						(0x04)
#define	INA219_REG_CALIBRATION					(0x05)
//
#define INA219_CONFIG_RESET 					(0x8000)
//
#define INA219_CONFIG_BVOLTAGERANGE_16V			(0x0000) // 0-16V Range
#define INA219_CONFIG_BVOLTAGERANGE_32V			(0x2000) // 0-32V Range

#define	INA219_CONFIG_GAIN_1_40MV				(0x0000)  // Gain 1, 40mV Range
#define	INA219_CONFIG_GAIN_2_80MV				(0x0800)  // Gain 2, 80mV Range
#define	INA219_CONFIG_GAIN_4_160MV				(0x1000) // Gain 4, 160mV Range
#define	INA219_CONFIG_GAIN_8_320MV				(0x1800) // Gain 8, 320mV Range

#define	INA219_CONFIG_BADCRES_9BIT				(0x0000)  // 9-bit bus res = 0..511
#define	INA219_CONFIG_BADCRES_10BIT				(0x0080) // 10-bit bus res = 0..1023
#define	INA219_CONFIG_BADCRES_11BIT				(0x0100) // 11-bit bus res = 0..2047
#define	INA219_CONFIG_BADCRES_12BIT				(0x0180) // 12-bit bus res = 0..4097
#define	INA219_CONFIG_BADCRES_12BIT_2S_1060US 	(0x0480) // 2 x 12-bit bus samples averaged together
#define	INA219_CONFIG_BADCRES_12BIT_4S_2130US	(0x0500) // 4 x 12-bit bus samples averaged together
#define	INA219_CONFIG_BADCRES_12BIT_8S_4260US	(0x0580) // 8 x 12-bit bus samples averaged together
#define	INA219_CONFIG_BADCRES_12BIT_16S_8510US	(0x0600) // 16 x 12-bit bus samples averaged together
#define	INA219_CONFIG_BADCRES_12BIT_32S_17MS	(0x0680) // 32 x 12-bit bus samples averaged together
#define	INA219_CONFIG_BADCRES_12BIT_64S_34MS	(0x0700) // 64 x 12-bit bus samples averaged together
#define	INA219_CONFIG_BADCRES_12BIT_128S_69MS	(0x0780) // 128 x 12-bit bus samples averaged together

#define	INA219_CONFIG_SADCRES_9BIT_1S_84US		(0x0000) // 1 x 9-bit shunt sample
#define	INA219_CONFIG_SADCRES_10BIT_1S_148US	(0x0008) // 1 x 10-bit shunt sample
#define	INA219_CONFIG_SADCRES_11BIT_1S_276US	(0x0010) // 1 x 11-bit shunt sample
#define	INA219_CONFIG_SADCRES_12BIT_1S_532US	(0x0018) // 1 x 12-bit shunt sample
#define	INA219_CONFIG_SADCRES_12BIT_2S_1060US	(0x0048) // 2 x 12-bit shunt samples averaged together
#define	INA219_CONFIG_SADCRES_12BIT_4S_2130US	(0x0050) // 4 x 12-bit shunt samples averaged together
#define	INA219_CONFIG_SADCRES_12BIT_8S_4260US	(0x0058) // 8 x 12-bit shunt samples averaged together
#define	INA219_CONFIG_SADCRES_12BIT_16S_8510US	(0x0060) // 16 x 12-bit shunt samples averaged together
#define	INA219_CONFIG_SADCRES_12BIT_32S_17MS	(0x0068) // 32 x 12-bit shunt samples averaged together
#define	INA219_CONFIG_SADCRES_12BIT_64S_34MS	(0x0070) // 64 x 12-bit shunt samples averaged together
#define	INA219_CONFIG_SADCRES_12BIT_128S_69MS	(0x0078) // 128 x 12-bit shunt samples averaged together
//
#define INA219_CONFIG_MODE_MASK					(0x07)
#define	INA219_CONFIG_MODE_POWERDOWN			0x00 /**< power down */
#define	INA219_CONFIG_MODE_SVOLT_TRIGGERED		0x01 /**< shunt voltage triggered */
#define	INA219_CONFIG_MODE_BVOLT_TRIGGERED		0x02 /**< bus voltage triggered */
#define	INA219_CONFIG_MODE_SANDBVOLT_TRIGGERED	0x03 /**< shunt and bus voltage triggered */
#define	INA219_CONFIG_MODE_ADCOFF				0x04 /**< ADC off */
#define	INA219_CONFIG_MODE_SVOLT_CONTINUOUS		0x05 /**< shunt voltage continuous */
#define	INA219_CONFIG_MODE_BVOLT_CONTINUOUS		0x06 /**< bus voltage continuous */
#define	INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS 0x07

typedef struct
{
    I2C_HandleTypeDef *ina219_i2c;
    uint8_t            Address;
    float              filteredVoltage; 
    bool               voltageFilterInitialized;
    int16_t            currentDivider_mA;
    int16_t            powerMultiplier_mW;
    uint16_t           calibrationValue;
} INA219_t;

enum BatteryState {Battery_START,Battery_OK, Battery_LOW}; // To help health check function sufficiently diagnose problems
extern bool isFirst;

/**
 * @brief Get the elapsed time in milliseconds since last call
 *
 * Returns the delta time between subsequent calls to this helper so power
 * computations can integrate over time.
 *
 * Requirement traceability:
 *
 * @return int Delta time in ms
 */
int INA219_GetDeltaTime_ms();

/**
 * @brief Initialize INA219 sensor instance
 *
 * Verifies the device is present on the I2C bus and initializes internal state.
 *
 * @param ina219 Pointer to INA219 instance to initialize
 * @param i2c Pointer to HAL I2C handle
 * @param Address 7-bit I2C address of the INA219 device
 *
 * Requirement traceability:
 *
 * @return uint8_t 1 on success, 0 on failure
 */
uint8_t INA219_Init(INA219_t *ina219, I2C_HandleTypeDef *i2c, uint8_t Address);

/**
 * @brief Read bus voltage in millivolts
 *
 * Reads BUSVOLTAGE register and converts raw value to mV.
 *
 * @param ina219 Pointer to INA219 instance
 *
 * Requirement traceability:
 *
 * @return uint16_t Bus voltage in millivolts
 */
uint16_t INA219_ReadBusVoltage(INA219_t *ina219);

/**
 * @brief Read calculated current in milliamps
 *
 * Applies current divider and returns the instantaneous current in mA.
 *
 * @param ina219 Pointer to INA219 instance
 *
 * Requirement traceability:
 *
 * @return int16_t Current in mA
 */
int16_t INA219_ReadCurrent(INA219_t *ina219);

/**
 * @brief Read raw current register value
 *
 * Returns the raw 16-bit signed current register content without scaling.
 *
 * @param ina219 Pointer to INA219 instance
 *
 * Requirement traceability:
 *
 * @return int16_t Raw current register value
 */
int16_t INA219_ReadCurrent_raw(INA219_t *ina219);

/**
 * @brief Read shunt voltage in millivolts
 *
 * Reads the SHUNT VOLTAGE register and applies scaling.
 *
 * @param ina219 Pointer to INA219 instance
 *
 * Requirement traceability:
 *
 * @return uint16_t Shunt voltage in millivolts
 */
uint16_t INA219_ReadShuntVolage(INA219_t *ina219);

/**
 * @brief Read power in milliwatts
 *
 * Reads POWER register and multiplies by configured power LSB.
 *
 * @param ina219 Pointer to INA219 instance
 *
 * Requirement traceability:
 *
 * @return uint16_t Power in milliwatts
 */
uint16_t INA219_ReadPower(INA219_t *ina219);

/**
 * @brief Estimate battery state of charge as percentage
 *
 * Converts measured bus voltage into a percentage using provided bounds.
 *
 * @param ina219 Pointer to INA219 instance
 * @param batteryMax Maximum expected battery voltage (mV)
 * @param batteryMin Minimum expected battery voltage (mV)
 *
 * Requirement traceability:
 *
 * @return float Battery life percentage (0..100)
 */
float INA219_GetBatteryLife(INA219_t *ina219,float batteryMax, float batteryMin);

/**
 * @brief Compute averaged milliwatt reading over recent samples
 *
 * @param ina219 Pointer to INA219 instance
 *
 * Requirement traceability:
 *
 * @return float Average milliwatts
 */
float INA219_GetAVGMiliWatt(INA219_t *ina219);

/**
 * @brief Compute energy (mW * delta-time) since last call
 *
 * @param ina219 Pointer to INA219 instance
 *
 * Requirement traceability:
 *
 * @return float Energy sample in mW·s
 */
float INA219_GetMiliWattsDeltaTime(INA219_t *ina219);

/**
 * @brief Get total integrated energy used since boot (approximation)
 *
 * @param ina219 Pointer to INA219 instance
 *
 * Requirement traceability:
 *
 * @return float Total energy (mW·s)
 */
float INA219_GetTotalPowerUsed(INA219_t *ina219);

/**
 * @brief Perform a simple battery health check and return state
 *
 * @param ina219 Pointer to INA219 instance
 * @param batteryPercentageThreshold Threshold percentage to consider low
 * @param batteryPercentage Current battery percentage
 *
 * Requirement traceability:
 *
 * @return enum BatteryState Current battery state
 */
enum BatteryState INA219_HealthCheck(INA219_t *ina219,float batteryPercentageThreshold,float batteryPercentage);

/**
 * @brief Reset INA219 device (write reset bit to config)
 *
 * @param ina219 Pointer to INA219 instance
 *
 * Requirement traceability:
 *
 * @return void
 */
void INA219_Reset(INA219_t *ina219);

/**
 * @brief Set calibration register
 *
 * @param ina219 Pointer to INA219 instance
 * @param CalibrationData Calibration value to write
 *
 * Requirement traceability:
 *
 * @return void
 */
void INA219_setCalibration(INA219_t *ina219, uint16_t CalibrationData);

/**
 * @brief Read configuration register
 *
 * @param ina219 Pointer to INA219 instance
 *
 * Requirement traceability:
 *
 * @return uint16_t Config register value
 */
uint16_t INA219_getConfig(INA219_t *ina219);

/**
 * @brief Write configuration register
 *
 * @param ina219 Pointer to INA219 instance
 * @param Config Config value to write
 *
 * Requirement traceability:
 *
 * @return void
 */
void INA219_setConfig(INA219_t *ina219, uint16_t Config);

/**
 * @brief Convenience calibration: 32V, 2A profile
 *
 * @param ina219 Pointer to INA219 instance
 *
 * Requirement traceability:
 *
 * @return void
 */
void INA219_setCalibration_32V_2A(INA219_t *ina219);

/**
 * @brief Convenience calibration: 32V, 1A profile
 *
 * @param ina219 Pointer to INA219 instance
 *
 * Requirement traceability:
 *
 * @return void
 */
void INA219_setCalibration_32V_1A(INA219_t *ina219);

/**
 * @brief Convenience calibration: 16V, 400mA profile
 *
 * @param ina219 Pointer to INA219 instance
 *
 * Requirement traceability:
 *
 * @return void
 */
void INA219_setCalibration_16V_400mA(INA219_t *ina219);

/**
 * @brief Set operating power mode (powerdown/continuous/etc)
 *
 * @param ina219 Pointer to INA219 instance
 * @param Mode One of INA219_CONFIG_MODE_* values
 *
 * Requirement traceability:
 *
 * @return void
 */
/**
 * @brief Convenience calibration: 16V, 3A profile
 *
 * @param ina219 Pointer to INA219 instance
 *
 * Requirement traceability:
 *
 * @return void
 */
void INA219_setCalibration_16V_3A(INA219_t *ina219);

void INA219_setPowerMode(INA219_t *ina219, uint8_t Mode);

/**
 * @brief Low-level register read helper
 *
 * Reads a 16-bit register from the INA219 using I2C.
 *
 * @param ina219 Pointer to INA219 instance
 * @param Register Register address to read
 *
 * Requirement traceability:
 *
 * @return uint16_t Register value
 */
uint16_t Read16(INA219_t *ina219, uint8_t Register);

/**
 * @brief Low-level register write helper
 *
 * Writes a 16-bit value to the specified INA219 register over I2C.
 *
 * @param ina219 Pointer to INA219 instance
 * @param Register Register address to write
 * @param Value 16-bit value to write
 *
 * Requirement traceability:
 *
 * @return HAL_StatusTypeDef HAL_OK on success, otherwise HAL_ERROR / other HAL status
 */
HAL_StatusTypeDef Write16(INA219_t *ina219, uint8_t Register, uint16_t Value);



#endif /* INC_INA219_H_ */
