/*
 * INA219.c
 *
 *  Created on: Dec 30, 2020
 *  Updated on: Jan 29, 2022
 *       Author: Piotr Smolen <komuch@gmail.com>
 *       Updated by: Brandon Thibeaux <github: thibeaux>
 */

#include "ina219.h"
enum BatteryState batteryState;

// Variable definitions for extern declarations in header
bool isFirst;
uint16_t ina219_calibrationValue;
int16_t ina219_currentDivider_mA;
int16_t ina219_powerMultiplier_mW;

// Read a 16-bit register from INA219 (internal helper)
uint16_t Read16(INA219_t *ina219, uint8_t Register)
{
	uint8_t Value[2];

	HAL_I2C_Mem_Read(ina219->ina219_i2c, (INA219_ADDRESS<<1), Register, 1, Value, 2, 1000);

	return ((Value[0] << 8) | Value[1]);
}

/*
 * @brief:		Write to a register on the IN219 sensor.
 * @param:		Pointer to the device object that was made from the struct. EX:  (&ina219)
 * @param:		Register address in hexadecimal
 * @param:		16 bit integer in hexadecimal that is the value you want to write to the register.
 * @retval:		HAL_StatusTypeDef, this will include an enum value representing
 * 				if the I2C transmission was successful or not.
 * 				typedef enum
				{
				  HAL_OK       = 0x00U,
				  HAL_ERROR    = 0x01U,
				  HAL_BUSY     = 0x02U,
				  HAL_TIMEOUT  = 0x03U
				} HAL_StatusTypeDef;
 */
HAL_StatusTypeDef Write16(INA219_t *ina219, uint8_t Register, uint16_t Value)
{
	uint8_t addr[2];
	addr[0] = (Value >> 8) & 0xff;  // upper byte
	addr[1] = (Value >> 0) & 0xff; // lower byte
	return HAL_I2C_Mem_Write(ina219->ina219_i2c, (INA219_ADDRESS<<1), Register, 1, (uint8_t*)addr, 2, 1000);
}

// Read bus voltage in millivolts
uint16_t INA219_ReadBusVoltage(INA219_t *ina219)
{
	uint16_t result = Read16(ina219, INA219_REG_BUSVOLTAGE);

	return ((result >> 3  ) * 4);

}

// Get raw current register value (signed)
int16_t INA219_ReadCurrent_raw(INA219_t *ina219)
{
	int16_t result = Read16(ina219, INA219_REG_CURRENT);

	return (result );
}

// Get current in milliamps (scaled)
int16_t INA219_ReadCurrent(INA219_t *ina219)
{
	int16_t result = INA219_ReadCurrent_raw(ina219);

	return (result / ina219_currentDivider_mA );
}

// Read shunt voltage in millivolts
uint16_t INA219_ReadShuntVolage(INA219_t *ina219)
{
	uint16_t result = Read16(ina219, INA219_REG_SHUNTVOLTAGE);

	return (result * 0.01 );
}
// Read instantaneous power in milliwatts
uint16_t INA219_ReadPower(INA219_t *ina219)
{
	uint16_t result = Read16(ina219, INA219_REG_POWER );
	result = result * ina219_powerMultiplier_mW; // power is the power register times the power_LSB (power multiplier)
	return (result);
}

/*
 * @brief:		This takes a minimum and maximum value an turn the current voltage
 * 				level into a percentage to give you a reference on how much battery life is left on your device.
 * 				Uses exponential moving average (EMA) filtering to prevent voltage sag under load
 * 				from causing rapid fluctuations in the battery percentage reading.
 * @param:	<object pointer> <maximum mili-voltage level> <minumum mili-voltage level>
 * @retval: 	Percentage floating point value
 * @example: 	GetBatteryLife(&ina219, 6000, 4000)
 * 				returns 75.02%
 */
static float filteredVoltage = 0.0f;
static bool voltageFilterInitialized = false;
#define BATTERY_EMA_ALPHA 0.05f  // Low alpha = more smoothing (0.05 = ~20 samples to reach 63% of new value)

float INA219_GetBatteryLife(INA219_t *ina219,float batteryMax, float batteryMin)
{
	float  percentageLife = 0.0f;
	uint16_t vbus = INA219_ReadBusVoltage(ina219);
	
	// Apply exponential moving average filter to smooth voltage readings
	// This prevents voltage sag under load from causing rapid battery % fluctuations
	if (!voltageFilterInitialized) {
		filteredVoltage = (float)vbus;
		voltageFilterInitialized = true;
	} else {
		filteredVoltage = BATTERY_EMA_ALPHA * (float)vbus + (1.0f - BATTERY_EMA_ALPHA) * filteredVoltage;
	}
	
	percentageLife = (filteredVoltage - batteryMin) / (batteryMax - batteryMin);
	if(percentageLife >= 0 )
	{
		return percentageLife * 100;
	}
	else
	{
		return 0.0f;
	}
}

/*
 * @brief: get Delta time in mili-seconds which is the difference between the last time you called this function and now
 */
int lastTime,deltaTime,now;
int INA219_GetDeltaTime_ms()
{
	  int now = HAL_GetTick();
	  deltaTime = now -lastTime;
	  lastTime = now;

	  return deltaTime;
}

/*
 * @brief:		The goal is to add up all the power used since the system turned on. WE do this by adding up samples from
 * 				INA219_GetMiliWattsDeltaTime every time this function is called. This returns a total used amount of mW seconds.
 */
float totalEnergy;
float INA219_GetTotalPowerUsed(INA219_t *ina219)
{
	float sample = INA219_GetMiliWattsDeltaTime(ina219); // for some reason it comes out in a really small number so multiply this by 1000 to get real figure.
	totalEnergy += (sample*1000);
	return totalEnergy;
}
/*
 * @brief:		The goal of this function is to give you the amount of mili-watts being
 * 				consumed every time the function is called or every change in time.
 * @param:		Pointer to the device object that was made from the struct. EX:  (&ina219)
 * @retval:		floating point number giving back a value for mili-watts seconds
 */
float INA219_GetMiliWattsDeltaTime(INA219_t *ina219)
{
	float energy = 0;
	energy = INA219_GetDeltaTime_ms() * INA219_ReadPower(ina219);
	return energy/1000; // convert mili-seconds to seconds.
}

/*
 * @brief:		The goal of this function is to give you the average mW amount
 * @param:		Pointer to the device object that was made from the struct. EX:  (&ina219)
 * @retval:		floating point number giving back a value for mili-watts
 */
float sample[10]; // this can be set to any number. REmeber the bigger the sample amount the longer it takes to become accurate.
int idx = 0;
float INA219_GetAVGMiliWatt(INA219_t *ina219)
{
	// Energy = Power * Time
	float energy = 0;
	if (!isFirst)
	{
		isFirst = true;
		sample[0] = INA219_ReadPower(ina219);
		idx = 1;
	}
	else
	{
		if(idx != 10)
		{
			sample[idx] = INA219_ReadPower(ina219);
			idx++;
		}
		else
		{
			idx = 0;
			sample[idx] = INA219_ReadPower(ina219);
		}

	}
	for(int i=0; i< (sizeof(sample)/sizeof(sample[0])); i++)
	{
		energy += sample[i];
	}
	energy = energy / (sizeof(sample)/sizeof(sample[0]));



	return energy;
}

/*
 * @brief:		The goal of this function is to make sure the battery is working as we
 * 				specify it to. If there is a discrepancy that is found in this check we
 * 				update the battery's state to alert the main function to take action.
 * @param:		Pointer to the device object that was made from the struct. EX:  (&ina219)
 * @param:		Floating point value for the percent of battery life we want to
 * 				dedicate as a threshold that indicates a "LOW" state
 * @param:		Floating point value that represents out batteries current life percentage ratio
 * @retval:		We will return an enum state that represents the battery's state. This tells
 * 				the program that called the health check function what state our battery is
 * 				at and whether we have entered a "LOW" state. This way the program can take
 * 				appropriate action.
 */
enum BatteryState INA219_HealthCheck(INA219_t *ina219,float batteryPercentageThreshold,float batteryPercentage)
{
	switch(batteryState)
	{
		case (Battery_START):
			/* Enter your start up functionality here */
			batteryState = Battery_OK;
			break;
		case (Battery_OK):
			/* Enter your battery OK state functionality here */
			if(batteryPercentage > batteryPercentageThreshold) // is battery life below given threshold?
			{
				batteryState = Battery_OK;
			}
			else
			{
				 batteryState = Battery_LOW;
			}
			break;
		case (Battery_LOW):
			/* Enter your battery LOW state functionality here */
			if(batteryPercentage > batteryPercentageThreshold) // is battery life below given threshold?
			{
				batteryState = Battery_OK;
			}
			else
			{
				 batteryState = Battery_LOW;
			}
			break;
		default:
			/*
			 * If program encounters a bug or a value outside what is expected we go here.
			 * Feel free to add functionality if needed.
			*/
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
	uint16_t result = Read16(ina219, INA219_REG_CONFIG);
	return result;
}

void INA219_setConfig(INA219_t *ina219, uint16_t Config)
{
	Write16(ina219, INA219_REG_CONFIG, Config);
}

void INA219_setCalibration_32V_2A(INA219_t *ina219)
{
	uint16_t config = INA219_CONFIG_BVOLTAGERANGE_32V |
	             INA219_CONFIG_GAIN_8_320MV | INA219_CONFIG_BADCRES_12BIT |
	             INA219_CONFIG_SADCRES_12BIT_1S_532US |
	             INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS;

	ina219_calibrationValue = 4096;
	ina219_currentDivider_mA = 10; // Current LSB = 100uA per bit (1000/100 = 10)
	ina219_powerMultiplier_mW = 2; // Power LSB = 1mW per bit (2/1)

	INA219_setCalibration(ina219, ina219_calibrationValue);
	INA219_setConfig(ina219, config);
}

void INA219_setCalibration_32V_1A(INA219_t *ina219)
{
	uint16_t config = INA219_CONFIG_BVOLTAGERANGE_32V |
	                    INA219_CONFIG_GAIN_8_320MV | INA219_CONFIG_BADCRES_12BIT |
	                    INA219_CONFIG_SADCRES_12BIT_1S_532US |
	                    INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS;

	ina219_calibrationValue = 10240;
	ina219_currentDivider_mA = 25;    // Current LSB = 40uA per bit (1000/40 = 25)
	ina219_powerMultiplier_mW = 0.8f; // Power LSB = 800uW per bit

	INA219_setCalibration(ina219, ina219_calibrationValue);
	INA219_setConfig(ina219, config);
}

void INA219_setCalibration_16V_400mA(INA219_t *ina219)
{
    // Changed from INA219_CONFIG_GAIN_1_40MV to INA219_CONFIG_GAIN_8_320MV to match Hugo work
	uint16_t config = INA219_CONFIG_BVOLTAGERANGE_16V |
	                    INA219_CONFIG_GAIN_8_320MV | INA219_CONFIG_BADCRES_12BIT |
	                    INA219_CONFIG_SADCRES_12BIT_1S_532US |
	                    INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS;

	ina219_calibrationValue = 33600; //Previous: 10240
	ina219_currentDivider_mA = 11;    // Current LSB = 91uA per bit (1000/91 = 11)
	ina219_powerMultiplier_mW = 2.0f; // Power LSB = 4mW per bit

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

    ina219_calibrationValue = 33600;       // Calibration register
    ina219_currentDivider_mA = 11;        // Current LSB = 91 uA/bit
    ina219_powerMultiplier_mW = 2;        // Power multiplier

    INA219_setCalibration(ina219, ina219_calibrationValue);
    INA219_setConfig(ina219, config);
}

void INA219_setPowerMode(INA219_t *ina219, uint8_t Mode)
{
	uint16_t config = INA219_getConfig(ina219);

	switch (Mode) {
		case INA219_CONFIG_MODE_POWERDOWN:
			config = (config & ~INA219_CONFIG_MODE_MASK) | (INA219_CONFIG_MODE_POWERDOWN & INA219_CONFIG_MODE_MASK);
			INA219_setConfig(ina219, config);
			break;

		case INA219_CONFIG_MODE_SANDBVOLT_TRIGGERED:
			config = (config & ~INA219_CONFIG_MODE_MASK) | (INA219_CONFIG_MODE_SANDBVOLT_TRIGGERED & INA219_CONFIG_MODE_MASK);
			INA219_setConfig(ina219, config);
			break;

		case INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS:
			config = (config & ~INA219_CONFIG_MODE_MASK) | (INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS & INA219_CONFIG_MODE_MASK);
			INA219_setConfig(ina219, config);
			break;

		case INA219_CONFIG_MODE_ADCOFF:
			config = (config & ~INA219_CONFIG_MODE_MASK) | (INA219_CONFIG_MODE_ADCOFF & INA219_CONFIG_MODE_MASK);
			INA219_setConfig(ina219, config);
			break;
	}
}

uint8_t INA219_Init(INA219_t *ina219, I2C_HandleTypeDef *i2c, uint8_t Address)
{
	isFirst = false; // set global var used by INA219_GetMiliWattMinutes
	ina219->ina219_i2c = i2c;
	ina219->Address = Address;

	ina219_currentDivider_mA = 0;
	ina219_powerMultiplier_mW = 0;

	uint8_t ina219_isReady = HAL_I2C_IsDeviceReady(i2c, (Address << 1), 3, 2);

	if(ina219_isReady == HAL_OK)
	{
		// just to initialize our state machine.
		//The numbers 0.0f and 1.0f is just to call the healthcheck function.
		//Feel free to change this if you want. This function should be called in your main function to be polled.
		batteryState = Battery_START; // go to starting position.
		INA219_HealthCheck(ina219,0.0f,1.0f );
		INA219_Reset(ina219);
		INA219_setCalibration_16V_3A(ina219);

		return 1;
	}

	else
	{
		return 0;
	}
}