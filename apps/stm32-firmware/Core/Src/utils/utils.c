#include "utils.h"

void Debug_Print(const char *msg) {
	/*
	 * @brief
	 *
	 * Description
	 *
	 * ====================== Requirement Traceability ===========================
	 *
	 * ==========================================================================
	 *
	 * @param name         Function
	 *
	 * @return HAL_StatusTypeDef
	 *         - HAL_OK     : Write successful
	 *         - HAL_ERROR  : Transmission failed
	 *         - HAL_BUSY   : I2C peripheral is busy
	 *         - HAL_TIMEOUT: Communication timeout
	 *
	 */

    HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
}

