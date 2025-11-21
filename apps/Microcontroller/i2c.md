# I2C Communication in BatteryMonitor Project

This document explains how the project communicates with the INA219 current/voltage sensor and the two PCA9685 PWM controllers over the I2C bus, including code structure, protocol details, and the STM32 HAL I2C functions used.

---

## I2C Bus Overview
- **Bus Used:** I2C1 (STM32 HAL, pins PB8/PB9)
- **Speed:** 100 kHz (standard mode)
- **API:** STM32 HAL I2C functions

### STM32 HAL I2C Functions Used

#### `HAL_I2C_Master_Transmit()`
- **Prototype:**
  ```c
  HAL_StatusTypeDef HAL_I2C_Master_Transmit(I2C_HandleTypeDef *hi2c,
                                            uint16_t DevAddress,
                                            uint8_t *pData,
                                            uint16_t Size,
                                            uint32_t Timeout);
  ```
- **Inputs:**
  - `hi2c`: Pointer to the I2C peripheral handle (e.g., `&hi2c1`)
  - `DevAddress`: 8-bit I2C address of the target device (left-shifted 7-bit address)
  - `pData`: Pointer to the data buffer to send (e.g., register address or data)
  - `Size`: Number of bytes to send
  - `Timeout`: Timeout duration in milliseconds
- **Usage:**
  - Used to send register addresses or data to I2C devices (INA219, PCA9685)

#### `HAL_I2C_Master_Receive()`
- **Prototype:**
  ```c
  HAL_StatusTypeDef HAL_I2C_Master_Receive(I2C_HandleTypeDef *hi2c,
                                           uint16_t DevAddress,
                                           uint8_t *pData,
                                           uint16_t Size,
                                           uint32_t Timeout);
  ```
- **Inputs:**
  - `hi2c`: Pointer to the I2C peripheral handle
  - `DevAddress`: 8-bit I2C address of the target device
  - `pData`: Pointer to the buffer to store received data
  - `Size`: Number of bytes to read
  - `Timeout`: Timeout duration in milliseconds
- **Usage:**
  - Used to read data from I2C devices after sending a register address

#### `HAL_I2C_Mem_Write()` and `HAL_I2C_Mem_Read()`
- **Purpose:**
  - Used as a fallback for register-level access (write/read a value to/from a specific register address in one call)
- **Inputs:**
  - `hi2c`: Pointer to the I2C peripheral handle
  - `DevAddress`: 8-bit I2C address
  - `MemAddress`: Register address to access
  - `MemAddSize`: Register address size (usually `I2C_MEMADD_SIZE_8BIT`)
  - `pData`: Pointer to data buffer
  - `Size`: Number of bytes to write/read
  - `Timeout`: Timeout duration

---

## INA219 (Current/Voltage Sensor)
- **I2C Address:** 0x41 (7-bit, shifted to 0x82 for 8-bit operations)
- **Purpose:** Measures battery voltage and current.

### Reading from INA219
1. **Set Register Address:**
   - Use `HAL_I2C_Master_Transmit` to send the register address (e.g., 0x02 for bus voltage, 0x04 for current).
2. **Read Data:**
   - Use `HAL_I2C_Master_Receive` to read 2 bytes of data from the sensor.
3. **Convert Data:**
   - For voltage: Combine bytes, shift as needed, and scale (e.g., 4mV per bit).
   - For current: Combine bytes and scale (e.g., 1mA per bit, calibration dependent).

#### Example (from `main.c`):
```c
uint8_t reg_addr = INA219_REG_BUS_VOLTAGE;
ret = HAL_I2C_Master_Transmit(&hi2c1, INA219_ADDR, &reg_addr, 1, 200); // Send register address
ret = HAL_I2C_Master_Receive(&hi2c1, INA219_ADDR, data, 2, 200);       // Read 2 bytes
int16_t bus_voltage_raw = (data[0] << 8) | data[1];
bus_voltage_raw >>= 3;
float voltage = bus_voltage_raw * 0.004f;
```

---

## PCA9685 (PWM Controllers)
- **Devices:**
  - Steering: 0x40 (7-bit, 0x80 8-bit)
  - Throttle: 0x60 (7-bit, 0xC0 8-bit)
- **Purpose:** Generate PWM signals for servos and H-Bridge motor drivers.

### Communicating with PCA9685
1. **Initialization:**
   - Write to MODE1, MODE2, and PRESCALE registers to configure frequency and mode.
   - Sequence: Normal mode → Sleep → Set prescale → Wake up.
   - Each register write uses `HAL_I2C_Master_Transmit` with a 2-byte buffer: `[register, value]`.
2. **Set PWM Output:**
   - For each channel, write 4 registers: ON_L, ON_H, OFF_L, OFF_H.
   - Each write uses `HAL_I2C_Master_Transmit` with a 2-byte buffer: `[register, value]`.

#### Example (from `pca9685.c`):
```c
uint8_t reg_base = 0x06 + 4 * channel;
cmd[0] = reg_base;     cmd[1] = on & 0xFF;   // ON_L
HAL_I2C_Master_Transmit(hi2c, device_addr, cmd, 2, 500);
cmd[0] = reg_base + 1; cmd[1] = on >> 8;     // ON_H
HAL_I2C_Master_Transmit(hi2c, device_addr, cmd, 2, 500);
cmd[0] = reg_base + 2; cmd[1] = off & 0xFF;  // OFF_L
HAL_I2C_Master_Transmit(hi2c, device_addr, cmd, 2, 500);
cmd[0] = reg_base + 3; cmd[1] = off >> 8;    // OFF_H
HAL_I2C_Master_Transmit(hi2c, device_addr, cmd, 2, 500);
```

---

## I2C Communication Details
- **All communication is blocking** (waits for each transaction to complete).
- **Error Handling:**
  - Return values are checked; errors are logged via UART.
  - I2C bus is reset if repeated errors occur.
- **Timing:**
  - Small delays (`tx_thread_sleep`) are used between I2C operations for reliability.

---

## Summary Table
| Device   | 7-bit Addr | 8-bit Addr | Purpose         | Channels Used |
|----------|------------|------------|----------------|--------------|
| INA219   | 0x41       | 0x82       | Voltage/Current| N/A          |
| PCA9685 (Steering) | 0x40 | 0x80 | Servo PWM      | 0            |
| PCA9685 (Throttle) | 0x60 | 0xC0 | Motor PWM/H-Bridge | 0-7      |

---

For more details, see `Core/Src/main.c` and `Core/Src/pca9685.c`.

