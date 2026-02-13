# STM32 Hardware Abstraction Layer (HAL) Documentation

## What is the STM32 HAL?

The Hardware Abstraction Layer (HAL) for STM32 microcontrollers is a set of C libraries provided by STMicroelectronics. HAL provides a high-level, hardware-independent API to interact with the microcontroller's peripherals (GPIO, UART, I2C, SPI, timers, etc.), making embedded development easier and more portable across STM32 devices.

Key features of the STM32 HAL include:
- Simplified peripheral initialization and configuration
- Consistent API across STM32 families
- Support for interrupt and DMA-based operations
- Extensive set of example code and documentation
- Integration with STM32CubeMX for code generation

## Main Concepts

- **Peripheral Handle**: Each peripheral (e.g., UART, I2C) is managed via a handle structure containing configuration and state.
- **Initialization Functions**: Functions to configure and enable peripherals.
- **Interrupts and Callbacks**: HAL provides weak callback functions for handling events (e.g., data received, transfer complete).
- **Blocking and Non-blocking APIs**: Most operations can be performed in polling (blocking), interrupt, or DMA (non-blocking) modes.

## How to Use the HAL

### 1. Initialization
Before using any peripheral, the HAL library and the system clock must be initialized:

```c
#include "stm32u5xx_hal.h"

int main(void) {
    HAL_Init(); // Initialize the HAL Library
    SystemClock_Config(); // Configure the system clock
    // Peripheral initialization here
    while (1) {
        // Main loop
    }
}
```

### 2. GPIO Example
Configure and use a GPIO pin:

```c
GPIO_InitTypeDef GPIO_InitStruct = {0};
__HAL_RCC_GPIOA_CLK_ENABLE();
GPIO_InitStruct.Pin = GPIO_PIN_5;
GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
GPIO_InitStruct.Pull = GPIO_NOPULL;
GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET); // Set pin high
```

### 3. UART Example
Initialize and use UART for serial communication:

```c
UART_HandleTypeDef huart2;

huart2.Instance = USART2;
huart2.Init.BaudRate = 115200;
huart2.Init.WordLength = UART_WORDLENGTH_8B;
huart2.Init.StopBits = UART_STOPBITS_1;
huart2.Init.Parity = UART_PARITY_NONE;
huart2.Init.Mode = UART_MODE_TX_RX;
huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
huart2.Init.OverSampling = UART_OVERSAMPLING_16;
HAL_UART_Init(&huart2);

char msg[] = "Hello, UART!\r\n";
HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
```

### 4. I2C Example
Initialize and use I2C:

```c
I2C_HandleTypeDef hi2c1;

hi2c1.Instance = I2C1;
hi2c1.Init.Timing = 0x00707CBB;
hi2c1.Init.OwnAddress1 = 0;
hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
hi2c1.Init.OwnAddress2 = 0;
hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
HAL_I2C_Init(&hi2c1);

uint8_t data = 0x55;
HAL_I2C_Master_Transmit(&hi2c1, (0x40 << 1), &data, 1, HAL_MAX_DELAY);
```

### 5. Using Interrupts and Callbacks
Enable interrupts and implement callback functions:

```c
HAL_UART_Receive_IT(&huart2, rx_buffer, RX_LEN);

// Callback (override weak function)
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    // Handle received data
}
```

### 6. Common HAL Functions
- `HAL_Init()` — Initialize the HAL library
- `HAL_GPIO_Init()` — Initialize GPIO pins
- `HAL_GPIO_WritePin()` / `HAL_GPIO_ReadPin()` — Set or read GPIO pin state
- `HAL_UART_Init()` — Initialize UART peripheral
- `HAL_UART_Transmit()` / `HAL_UART_Receive()` — Send/receive data over UART
- `HAL_I2C_Init()` — Initialize I2C peripheral
- `HAL_I2C_Master_Transmit()` / `HAL_I2C_Master_Receive()` — I2C communication
- `HAL_Delay(ms)` — Delay in milliseconds

## References
- [STM32 HAL API Reference](https://www.st.com/en/embedded-software/stm32cube-mcu-package.html)
- [STM32Cube HAL User Manual](https://www.st.com/resource/en/user_manual/dm00244518.pdf)
- [STM32CubeMX Tool](https://www.st.com/en/development-tools/stm32cubemx.html)

---
This document provides a high-level overview. For detailed usage, refer to the official STM32 documentation and your device's reference manual.
