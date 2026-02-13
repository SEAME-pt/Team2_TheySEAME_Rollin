#include "utils.h"

void Debug_Print(const char *msg) {
    // Implementation: transmit debug string over UART. Public API is documented in `utils.h`.
    HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
} 

