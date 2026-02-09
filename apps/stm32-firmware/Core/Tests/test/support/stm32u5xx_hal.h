#ifndef TEST_STM32U5XX_HAL_H
#define TEST_STM32U5XX_HAL_H

#include <stdint.h>
#include <stddef.h>

/* Minimal stubs for HAL types and functions used in tests */
typedef uint32_t HAL_StatusTypeDef;
#define HAL_OK      ((HAL_StatusTypeDef)0x00U)
#define HAL_ERROR   ((HAL_StatusTypeDef)0x01U)
#define HAL_BUSY    ((HAL_StatusTypeDef)0x02U)
#define HAL_TIMEOUT ((HAL_StatusTypeDef)0x03U)

typedef uint16_t uint16_t;

/* Provide a no-op NOP for SPI delay usage in SUT */
#ifndef __NOP
#define __NOP() ((void)0)
#endif

typedef enum { GPIO_PIN_RESET = 0, GPIO_PIN_SET = 1 } GPIO_PinState;

/* GPIO operations used by mcp2515 (no-op in tests unless overridden) */
void HAL_GPIO_WritePin(void *GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState);
GPIO_PinState HAL_GPIO_ReadPin(void *GPIOx, uint16_t GPIO_Pin);

/* UART - use void* to avoid colliding with test main typedef */
HAL_StatusTypeDef HAL_UART_Transmit(void *huart, uint8_t *pData, uint16_t Size, uint32_t Timeout);

/* Timing */
uint32_t HAL_GetTick(void);
void HAL_Delay(uint32_t ms);

#endif /* TEST_STM32U5XX_HAL_H */