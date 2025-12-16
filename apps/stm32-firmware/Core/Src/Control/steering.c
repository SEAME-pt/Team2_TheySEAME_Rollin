#include "control.h"

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
void PCA9685_SetServoAngle(uint8_t channel, float angle) {

    // Clamp angle to safe range
    if (angle < -30.0f) angle = -30.0f;
    if (angle > 30.0f) angle = 30.0f;

    // Convert angle to pulse width (1ms to 2ms)
    float pulse_center = 307.0f;
    float counts_per_degree = 2.27f;

    uint16_t pulse_width = (uint16_t)(pulse_center + (angle * counts_per_degree));

    // Set PWM: ON at 0, OFF at calculated pulse width
    PCA9685_SetPWM(&hi2c1, PCA9685_ADDR_STEERING, channel, 0, pulse_width);
}
