#include "tx_api.h"
#include <stdio.h>
#include <stdint.h>
#include "pca9685.h"

extern char uart_buf[64];

static HAL_StatusTypeDef PCA9685_SoftwareReset(I2C_HandleTypeDef *hi2c)
{
	uint8_t reset_cmd = 0x06;
	HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(hi2c, 0x00, &reset_cmd, 1, 500);
	tx_thread_sleep(1);
	return status;
}

static HAL_StatusTypeDef PCA9685_WriteReg(I2C_HandleTypeDef *hi2c, uint8_t device_addr, uint8_t reg, uint8_t val)
{
	uint8_t data[2] = {reg, val};
	HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(hi2c, device_addr, data, 2, 500);

	if (status != HAL_OK)
		status = HAL_I2C_Mem_Write(hi2c, device_addr, reg, I2C_MEMADD_SIZE_8BIT, &val, 1, 500);

	tx_thread_sleep(2);
	return status;
}

static HAL_StatusTypeDef PCA9685_ReadReg(I2C_HandleTypeDef *hi2c, uint8_t device_addr, uint8_t reg, uint8_t *val)
{
	HAL_StatusTypeDef status = HAL_I2C_Mem_Read(hi2c, device_addr, reg, I2C_MEMADD_SIZE_8BIT, val, 1, 500);

	if (status != HAL_OK)
	{
		status = HAL_I2C_Master_Transmit(hi2c, device_addr, &reg, 1, 500);
		if (status == HAL_OK)
			status = HAL_I2C_Master_Receive(hi2c, device_addr, val, 1, 500);
	}

	return status;
}

static HAL_StatusTypeDef PCA9685_Init_Device_NoReset(I2C_HandleTypeDef *hi2c, uint8_t addr, const char *name)
{
	HAL_StatusTypeDef ret;

	ret = PCA9685_WriteReg(hi2c, addr, PCA9685_MODE1, 0x31);
	if (ret != HAL_OK)
		return ret;

	ret = PCA9685_WriteReg(hi2c, addr, PCA9685_PRESCALE, 121);
	if (ret != HAL_OK)
		return ret;

	ret = PCA9685_WriteReg(hi2c, addr, PCA9685_MODE1, 0x21);
	if (ret != HAL_OK)
		return ret;

	tx_thread_sleep(5);

	ret = PCA9685_WriteReg(hi2c, addr, PCA9685_MODE2, 0x04);
	if (ret != HAL_OK)
		return HAL_OK;

	return HAL_OK;
}

HAL_StatusTypeDef PCA9685_Init_Multiple(I2C_HandleTypeDef *hi2c, uint8_t addr1, const char *name1, uint8_t addr2, const char *name2)
{
	HAL_StatusTypeDef ret;

	PCA9685_SoftwareReset(hi2c);
	tx_thread_sleep(5);

	ret = PCA9685_Init_Device_NoReset(hi2c, addr1, name1);
	if (ret != HAL_OK)
		return ret;

	ret = PCA9685_Init_Device_NoReset(hi2c, addr2, name2);
	if (ret != HAL_OK)
		return ret;

	return HAL_OK;
}

HAL_StatusTypeDef PCA9685_Init_Device(I2C_HandleTypeDef *hi2c, uint8_t addr, const char *name)
{
	HAL_StatusTypeDef ret;

	PCA9685_SoftwareReset(hi2c);
	tx_thread_sleep(5);

	ret = PCA9685_WriteReg(hi2c, addr, PCA9685_MODE1, 0x31);
	if (ret != HAL_OK)
		return ret;

	ret = PCA9685_WriteReg(hi2c, addr, PCA9685_PRESCALE, 121);
	if (ret != HAL_OK)
		return ret;

	ret = PCA9685_WriteReg(hi2c, addr, PCA9685_MODE1, 0x21);
	if (ret != HAL_OK)
		return ret;

	tx_thread_sleep(5);

	ret = PCA9685_WriteReg(hi2c, addr, PCA9685_MODE2, 0x04);
	if (ret != HAL_OK)
		return HAL_OK;

	return HAL_OK;
}

HAL_StatusTypeDef PCA9685_SetPWM(I2C_HandleTypeDef *hi2c, uint8_t device_addr, uint8_t channel, uint16_t on, uint16_t off)
{
	uint8_t reg_base = PCA9685_LED0_ON_L + 4 * channel;
	uint8_t cmd[2];
	HAL_StatusTypeDef status;

	cmd[0] = reg_base;
	cmd[1] = on & 0xFF;
	status = HAL_I2C_Master_Transmit(hi2c, device_addr, cmd, 2, 500);
	if (status != HAL_OK) return status;

	cmd[0] = reg_base + 1;
	cmd[1] = on >> 8;
	status = HAL_I2C_Master_Transmit(hi2c, device_addr, cmd, 2, 500);
	if (status != HAL_OK) return status;

	cmd[0] = reg_base + 2;
	cmd[1] = off & 0xFF;
	status = HAL_I2C_Master_Transmit(hi2c, device_addr, cmd, 2, 500);
	if (status != HAL_OK) return status;

	cmd[0] = reg_base + 3;
	cmd[1] = off >> 8;
	status = HAL_I2C_Master_Transmit(hi2c, device_addr, cmd, 2, 500);

	return status;
}
