#ifndef __TEST_MAIN_H
#define __TEST_MAIN_H

/* Minimal stub of main.h for unit tests to avoid pulling in STM32 HAL headers */

#include <stdint.h>

/* Minimal types used by sensors.h and tests */
typedef unsigned long ULONG;
/* TX types are provided by mocked tx_api.h; do not redefine TX_MUTEX here */

/* Simplified UART handle stub (tests mock UART functions) */
typedef struct {
    int dummy;
} UART_HandleTypeDef;

/* Simple defines used by tests (override in test files if needed) */
#ifndef THREAD_SLEEP_TICKS
#define THREAD_SLEEP_TICKS 10
#endif

#ifndef PULSES_PER_REV
#define PULSES_PER_REV 20
#endif

#endif /* __TEST_MAIN_H */
