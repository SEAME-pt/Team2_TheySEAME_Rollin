#include "ina219.h"

enum BatteryState batteryState;
bool isFirst;
uint16_t ina219_calibrationValue;
int16_t ina219_currentDivider_mA;
int16_t ina219_powerMultiplier_mW;

static int lastTime;
static int deltaTime;
static float totalEnergy;
static float sample[10];
static int idx = 0;

uint16_t Read16(INA219_t *ina219, uint8_t Register)
{
	uint8_t Value[2];
	HAL_I2C_Mem_Read(ina219->ina219_i2c, (INA219_ADDRESS << 1), Register, 1, Value, 2, 1000);
	return ((Value[0] << 8) | Value[1]);
}

HAL_StatusTypeDef Write16(INA219_t *ina219, uint8_t Register, uint16_t Value)
{
	uint8_t addr[2];
	addr[0] = (Value >> 8) & 0xFF;
	addr[1] = (Value >> 0) & 0xFF;
	return HAL_I2C_Mem_Write(ina219->ina219_i2c, (INA219_ADDRESS << 1), Register, 1, addr, 2, 1000);
}

uint16_t INA219_ReadBusVoltage(INA219_t *ina219)
{
	uint16_t result = Read16(ina219, INA219_REG_BUSVOLTAGE);
	return ((result >> 3) * 4);
}

int16_t INA219_ReadCurrent_raw(INA219_t *ina219)
{
	return Read16(ina219, INA219_REG_CURRENT);
}

int16_t INA219_ReadCurrent(INA219_t *ina219)
{
	return INA219_ReadCurrent_raw(ina219) / ina219_currentDivider_mA;
}

uint16_t INA219_ReadShuntVolage(INA219_t *ina219)
{
	uint16_t result = Read16(ina219, INA219_REG_SHUNTVOLTAGE);
	return (result * 0.01);
}

uint16_t INA219_ReadPower(INA219_t *ina219)
{
	uint16_t result = Read16(ina219, INA219_REG_POWER);
	return (result * ina219_powerMultiplier_mW);
}

float INA219_GetBatteryLife(INA219_t *ina219, float batteryMax, float batteryMin)
{
	uint16_t vbus = INA219_ReadBusVoltage(ina219);
	float percentageLife = (vbus - batteryMin) / (batteryMax - batteryMin);

	if (percentageLife >= 0)
		return percentageLife * 100;
	else
		return 0.0f;
}

int INA219_GetDeltaTime_ms()
{
	int now = HAL_GetTick();
	deltaTime = now - lastTime;
	lastTime = now;
	return deltaTime;
}

float INA219_GetTotalPowerUsed(INA219_t *ina219)
{
	float s = INA219_GetMiliWattsDeltaTime(ina219);
	totalEnergy += (s * 1000);
	return totalEnergy;
}

float INA219_GetMiliWattsDeltaTime(INA219_t *ina219)
{
	float energy = INA219_GetDeltaTime_ms() * INA219_ReadPower(ina219);
	return energy / 1000;
}

float INA219_GetAVGMiliWatt(INA219_t *ina219)
{
	float energy = 0;

	if (!isFirst)
	{
		isFirst = true;
		sample[0] = INA219_ReadPower(ina219);
		idx = 1;
	}
	else
	{
		sample[idx] = INA219_ReadPower(ina219);
		idx = (idx + 1) % 10;
	}

	for (int i = 0; i < 10; i++)
		energy += sample[i];

	return energy / 10;
}

enum BatteryState INA219_HealthCheck(INA219_t *ina219, float batteryPercentageThreshold, float batteryPercentage)
{
	switch (batteryState)
	{
		case Battery_START:
			batteryState = Battery_OK;
			break;
		case Battery_OK:
		case Battery_LOW:
			if (batteryPercentage > batteryPercentageThreshold)
				batteryState = Battery_OK;
			else
				batteryState = Battery_LOW;
			break;
		default:
			batteryState = Battery_START;
			break;
	}
	return batteryState;
}

void INA219_Reset(INA219_t *ina219)
{
	Write16(ina219, INA219_REG_CONFIG, INA219_CONFIG_RESET);
	HAL_Delay(1);
}

void INA219_setCalibration(INA219_t *ina219, uint16_t CalibrationData)
{
	Write16(ina219, INA219_REG_CALIBRATION, CalibrationData);
}

uint16_t INA219_getConfig(INA219_t *ina219)
{
	return Read16(ina219, INA219_REG_CONFIG);
}

void INA219_setConfig(INA219_t *ina219, uint16_t Config)
{
	Write16(ina219, INA219_REG_CONFIG, Config);
}

void INA219_setCalibration_32V_2A(INA219_t *ina219)
{
	uint16_t config = INA219_CONFIG_BVOLTAGERANGE_32V |
	                  INA219_CONFIG_GAIN_8_320MV |
	                  INA219_CONFIG_BADCRES_12BIT |
	                  INA219_CONFIG_SADCRES_12BIT_1S_532US |
	                  INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS;

	ina219_calibrationValue = 4096;
	ina219_currentDivider_mA = 10;
	ina219_powerMultiplier_mW = 2;

	INA219_setCalibration(ina219, ina219_calibrationValue);
	INA219_setConfig(ina219, config);
}

void INA219_setCalibration_32V_1A(INA219_t *ina219)
{
	uint16_t config = INA219_CONFIG_BVOLTAGERANGE_32V |
	                  INA219_CONFIG_GAIN_8_320MV |
	                  INA219_CONFIG_BADCRES_12BIT |
	                  INA219_CONFIG_SADCRES_12BIT_1S_532US |
	                  INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS;

	ina219_calibrationValue = 10240;
	ina219_currentDivider_mA = 25;
	ina219_powerMultiplier_mW = 0.8f;

	INA219_setCalibration(ina219, ina219_calibrationValue);
	INA219_setConfig(ina219, config);
}

void INA219_setCalibration_16V_400mA(INA219_t *ina219)
{
	uint16_t config = INA219_CONFIG_BVOLTAGERANGE_16V |
	                  INA219_CONFIG_GAIN_8_320MV |
	                  INA219_CONFIG_BADCRES_12BIT |
	                  INA219_CONFIG_SADCRES_12BIT_1S_532US |
	                  INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS;

	ina219_calibrationValue = 33600;
	ina219_currentDivider_mA = 11;
	ina219_powerMultiplier_mW = 2.0f;

	INA219_setCalibration(ina219, ina219_calibrationValue);
	INA219_setConfig(ina219, config);
}

void INA219_setCalibration_16V_3A(INA219_t *ina219)
{
	uint16_t config = INA219_CONFIG_BVOLTAGERANGE_16V |
	                  INA219_CONFIG_GAIN_1_40MV |
	                  INA219_CONFIG_BADCRES_12BIT |
	                  INA219_CONFIG_SADCRES_12BIT_1S_532US |
	                  INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS;

	ina219_calibrationValue = 33600;
	ina219_currentDivider_mA = 11;
	ina219_powerMultiplier_mW = 2;

	INA219_setCalibration(ina219, ina219_calibrationValue);
	INA219_setConfig(ina219, config);
}

void INA219_setPowerMode(INA219_t *ina219, uint8_t Mode)
{
	uint16_t config = INA219_getConfig(ina219);
	config = (config & ~INA219_CONFIG_MODE_MASK) | (Mode & INA219_CONFIG_MODE_MASK);
	INA219_setConfig(ina219, config);
}

uint8_t INA219_Init(INA219_t *ina219, I2C_HandleTypeDef *i2c, uint8_t Address)
{
	isFirst = false;
	ina219->ina219_i2c = i2c;
	ina219->Address = Address;
	ina219_currentDivider_mA = 0;
	ina219_powerMultiplier_mW = 0;

	uint8_t ina219_isReady = HAL_I2C_IsDeviceReady(i2c, (Address << 1), 3, 2);

	if (ina219_isReady == HAL_OK)
	{
		batteryState = Battery_START;
		INA219_HealthCheck(ina219, 0.0f, 1.0f);
		INA219_Reset(ina219);
		INA219_setCalibration_16V_3A(ina219);
		return 1;
	}

	return 0;
}
