#include "sensors.h"

char uart_buf[64];

void Battery_Thread_Entry(ULONG thread_input) {
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

    (void) thread_input;
    HAL_StatusTypeDef ret;

    // INA219 constants
    const uint8_t INA219_ADDR = 0x41 << 1;  // INA219 at 0x41
    const uint8_t INA219_REG_CONFIG = 0x00;
    const uint8_t INA219_REG_SHUNT_VOLTAGE = 0x01;
    const uint8_t INA219_REG_BUS_VOLTAGE = 0x02;
    const uint8_t INA219_REG_CURRENT = 0x04;

    Debug_Print("\r\n=== BATTERY MONITOR THREAD STARTED ===\r\n");

    Debug_Print("[BATTERY_THREAD] Resetting I2C bus...\r\n");
    HAL_I2C_DeInit(&hi2c1);
    tx_thread_sleep(10);  // 100ms delay
    MX_I2C1_Init();
    tx_thread_sleep(10);  // 100ms delay
    Debug_Print("[BATTERY_THREAD] I2C bus reset complete\r\n");

    tx_thread_sleep(100);

    Debug_Print("\r\n[BATTERY_THREAD] Scanning I2C bus...\r\n");
    uint8_t found_count = 0;
    for (uint8_t addr = 0x01; addr < 0x7F; addr++) {
        if (HAL_I2C_IsDeviceReady(&hi2c1, addr << 1, 1, 100) == HAL_OK) {
            snprintf(uart_buf, sizeof(uart_buf), "  Device found at 0x%02X\r\n", addr);
            Debug_Print(uart_buf);
            found_count++;
        }
    }
    snprintf(uart_buf, sizeof(uart_buf), "[BATTERY_THREAD] Scan complete: %d devices found\r\n", found_count);
    Debug_Print(uart_buf);

    // Check if INA219 is present

    Debug_Print("\r\n[BATTERY_THREAD] Checking INA219 at 0x41\r\n");
    if (HAL_I2C_IsDeviceReady(&hi2c1, INA219_ADDR, 2, 200) == HAL_OK) {
        Debug_Print("[BATTERY_THREAD] INA219 found!\r\n");

        // Configure INA219: 16V range, 320mV shunt range, continuous mode
        uint8_t config_data[3] = {INA219_REG_CONFIG, 0x39, 0x9F};
        ret = HAL_I2C_Master_Transmit(&hi2c1, INA219_ADDR, config_data, 3, 200);
        if (ret == HAL_OK) {
            Debug_Print("[BATTERY_THREAD] INA219 configured!\r\n");
        } else {
            snprintf(uart_buf, sizeof(uart_buf), "[BATTERY_THREAD] INA219 config failed (status: %d)!\r\n", ret);
            Debug_Print(uart_buf);
        }
    } else {
        Debug_Print("[BATTERY_THREAD] INA219 not found!\r\n");
    }

    // Helper macro for concise init/test
  #define INIT_AND_TEST_PCA9685(addr, label) \
    do { \
      snprintf(uart_buf, sizeof(uart_buf), "[PCA9685] Init %s (0x%02X)\r\n", label, addr); Debug_Print(uart_buf); \
      uint8_t cmd1[2] = {0x00, 0x00}; HAL_I2C_Master_Transmit(&hi2c1, addr, cmd1, 2, 500); tx_thread_sleep(1); \
      uint8_t cmd2[2] = {0x01, 0x04}; HAL_I2C_Master_Transmit(&hi2c1, addr, cmd2, 2, 500); tx_thread_sleep(1); \
      uint8_t cmd3[2] = {0x00, 0x10}; HAL_I2C_Master_Transmit(&hi2c1, addr, cmd3, 2, 500); tx_thread_sleep(1); \
      uint8_t cmd4[2] = {0xFE, 121}; HAL_I2C_Master_Transmit(&hi2c1, addr, cmd4, 2, 500); tx_thread_sleep(1); \
      uint8_t cmd5[2] = {0x00, 0x80}; HAL_I2C_Master_Transmit(&hi2c1, addr, cmd5, 2, 500); tx_thread_sleep(1); \
      uint8_t mode1_reg = 0x00, mode1_val = 0; \
      HAL_I2C_Master_Transmit(&hi2c1, addr, &mode1_reg, 1, 500); \
      HAL_I2C_Master_Receive(&hi2c1, addr, &mode1_val, 1, 500); \
      snprintf(uart_buf, sizeof(uart_buf), "[PCA9685] MODE1: 0x%02X\r\n", mode1_val); Debug_Print(uart_buf); \
      ret = PCA9685_SetPWM(&hi2c1, addr, 0, 0, 307); \
      if (ret == HAL_OK) { \
        uint8_t on_l, on_h, off_l, off_h, reg; \
        reg = 0x06; HAL_I2C_Master_Transmit(&hi2c1, addr, &reg, 1, 500); HAL_I2C_Master_Receive(&hi2c1, addr, &on_l, 1, 500); \
        reg = 0x07; HAL_I2C_Master_Transmit(&hi2c1, addr, &reg, 1, 500); HAL_I2C_Master_Receive(&hi2c1, addr, &on_h, 1, 500); \
        reg = 0x08; HAL_I2C_Master_Transmit(&hi2c1, addr, &reg, 1, 500); HAL_I2C_Master_Receive(&hi2c1, addr, &off_l, 1, 500); \
        reg = 0x09; HAL_I2C_Master_Transmit(&hi2c1, addr, &reg, 1, 500); HAL_I2C_Master_Receive(&hi2c1, addr, &off_h, 1, 500); \
        uint16_t off_value = (off_h << 8) | off_l; \
        snprintf(uart_buf, sizeof(uart_buf), "[PCA9685] %s PWM: ON=%d OFF=%d (readback: %d)\r\n", label, 0, 307, off_value); Debug_Print(uart_buf); \
      } else { Debug_Print("[PCA9685] PWM write failed\r\n"); } \
    } while(0)

    INIT_AND_TEST_PCA9685(PCA9685_ADDR_THROTTLE, "Throttle");
    INIT_AND_TEST_PCA9685(PCA9685_ADDR_STEERING, "Steering");

    Debug_Print("[BATTERY_THREAD] PCA9685 test complete. Starting sweep test...\r\n\r\n");

    // --- STEERING SERVO SWEEP TEST ---
    for (float angle = -30.0f; angle <= 30.0f; angle += 5.0f) {
        PCA9685_SetServoAngle(0, angle); // Channel 0 for steering
        tx_thread_sleep(10); // 100ms per step
    }
    for (float angle = 30.0f; angle >= -30.0f; angle -= 5.0f) {
        PCA9685_SetServoAngle(0, angle);
        tx_thread_sleep(10);
    }
    Debug_Print("[SWEEP] Steering sweep complete\r\n");

    // Wait 1 second before throttle test
    tx_thread_sleep(100); // 1 second

    // --- THROTTLE MOTOR H-BRIDGE TEST (PCA9685 @ 0x60) ---
      // This test runs both motors forward at 50% duty for 2 seconds, then stops all motors.
      // Channels: 0 = M1 speed, 1 = M1 DIR1, 2 = M1 DIR2, 3 = M2 speed, 4 = M2 speed, 5 = M2 DIR2, 6 = M2 DIR1, 7 = M2 speed

      // Set both motors forward, 50% duty
      uint16_t speed_pwm = 2048; // 50% of 4095
      uint16_t dir_high = 4095;
      uint16_t dir_low = 0;

      // Motor 1 (channels 0,1,2,3)
      PCA9685_SetPWM(&hi2c1, PCA9685_ADDR_THROTTLE, 0, 0, speed_pwm); // M1 speed
      PCA9685_SetPWM(&hi2c1, PCA9685_ADDR_THROTTLE, 1, 0, dir_high);  // M1 DIR1 (forward)
      PCA9685_SetPWM(&hi2c1, PCA9685_ADDR_THROTTLE, 2, 0, dir_low);   // M1 DIR2 (reverse)
      PCA9685_SetPWM(&hi2c1, PCA9685_ADDR_THROTTLE, 3, 0, 0);         // M2 speed (off)

      // Motor 2 (channels 4,5,6,7)
      PCA9685_SetPWM(&hi2c1, PCA9685_ADDR_THROTTLE, 4, 0, speed_pwm); // M2 speed
      PCA9685_SetPWM(&hi2c1, PCA9685_ADDR_THROTTLE, 5, 0, dir_low);   // M2 DIR2 (reverse)
      PCA9685_SetPWM(&hi2c1, PCA9685_ADDR_THROTTLE, 6, 0, dir_high);  // M2 DIR1 (forward)
      PCA9685_SetPWM(&hi2c1, PCA9685_ADDR_THROTTLE, 7, 0, speed_pwm); // M2 speed

      Debug_Print("[THROTTLE TEST] Both motors forward 50%\r\n");
      tx_thread_sleep(200); // 2 seconds

      // Stop all motors (set all speed and direction channels to 0)
    for (uint8_t ch = 0; ch < 8; ++ch) {
      PCA9685_SetPWM(&hi2c1, PCA9685_ADDR_THROTTLE, ch, 0, 0);
    }
    Debug_Print("[THROTTLE TEST] Motors stopped\r\n\r\n");

    Debug_Print("[BATTERY_THREAD] Starting monitoring loop\r\n\r\n");

    uint32_t count = 0;
    uint8_t led_state = 0;
    uint8_t error_count = 0;

    while(1) {
        // Toggle green LED (runs independently from other threads)
        led_state = !led_state;
        HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, led_state ? GPIO_PIN_SET : GPIO_PIN_RESET);

        // Read INA219 voltage and current with error recovery
        uint8_t reg_addr;
        uint8_t data[2];

        // Read bus voltage with retry
        reg_addr = INA219_REG_BUS_VOLTAGE;
        ret = HAL_I2C_Master_Transmit(&hi2c1, INA219_ADDR, &reg_addr, 1, 200);

        if (ret != HAL_OK) {
            // I2C error - try to recover
            error_count++;
            if (error_count > 3) {
                // Reset I2C peripheral
                __HAL_I2C_DISABLE(&hi2c1);
                tx_thread_sleep(5);  // 50ms delay
                __HAL_I2C_ENABLE(&hi2c1);
                error_count = 0;
                Debug_Print("[BATTERY_THREAD] I2C bus reset\r\n");
            }
            snprintf(uart_buf, sizeof(uart_buf), "[BATTERY_THREAD][%lu] I2C TX error: %d\r\n", count, ret);
            Debug_Print(uart_buf);
        } else {
            // Small delay before receive
            tx_thread_sleep(1);  // 10ms

            ret = HAL_I2C_Master_Receive(&hi2c1, INA219_ADDR, data, 2, 200);
            if (ret == HAL_OK) {
                error_count = 0;  // Reset error counter on success

                int16_t bus_voltage_raw = (data[0] << 8) | data[1];

                // Debug: show raw data
                if (count < 3) {  // Only for first 3 readings
                    snprintf(uart_buf, sizeof(uart_buf), "[DEBUG] Raw bytes: 0x%02X 0x%02X, raw_value: %d\r\n",
                            data[0], data[1], bus_voltage_raw);
                    Debug_Print(uart_buf);
                }

                bus_voltage_raw >>= 3;  // Remove lower 3 bits
                float voltage = bus_voltage_raw * 0.004f;  // 4mV per bit

                // Calculate battery percentage (9.5V = 0%, 12.6V = 100%)
                float percentage = ((voltage - 9.5f) / (12.6f - 9.5f)) * 100.0f;
                // Clamp percentage between 0% and 100%
                if (percentage > 100.0f) percentage = 100.0f;
                if (percentage < 0.0f) percentage = 0.0f;

                // Read current with delay
                tx_thread_sleep(1);  // 10ms
                reg_addr = INA219_REG_CURRENT;
                ret = HAL_I2C_Master_Transmit(&hi2c1, INA219_ADDR, &reg_addr, 1, 200);
                if (ret == HAL_OK) {
                    tx_thread_sleep(1);  // 10ms
                    ret = HAL_I2C_Master_Receive(&hi2c1, INA219_ADDR, data, 2, 200);
                    if (ret == HAL_OK) {
                        int16_t current_raw = (data[0] << 8) | data[1];
                        float current = current_raw * 0.001f;  // 1mA per bit (depends on calibration)

                        // Convert floats to integers for printf (floating point printf not enabled)
                        int voltage_int = (int)voltage;
                        int voltage_frac = (int)((voltage - voltage_int) * 100);
                        int percentage_int = (int)percentage;
                        int current_int = (int)current;

                        snprintf(uart_buf, sizeof(uart_buf), "[BATTERY_THREAD][%lu] %d.%02dV (%d%%), %dmA\r\n",
                                count, voltage_int, voltage_frac, percentage_int, current_int);
                        Debug_Print(uart_buf);
                    } else {
                        error_count++;
                        snprintf(uart_buf, sizeof(uart_buf), "[BATTERY_THREAD][%lu] Current RX error: %d\r\n", count, ret);
                        Debug_Print(uart_buf);
                    }
                } else {
                    error_count++;
                    snprintf(uart_buf, sizeof(uart_buf), "[BATTERY_THREAD][%lu] Current TX error: %d\r\n", count, ret);
                    Debug_Print(uart_buf);
                }
            } else {
                error_count++;
                snprintf(uart_buf, sizeof(uart_buf), "[BATTERY_THREAD][%lu] Voltage RX error: %d\r\n", count, ret);
                Debug_Print(uart_buf);
            }
        }

        count++;
        tx_thread_sleep(200);  // 2 seconds (context switches to other threads)
    }
} // Properly close Battery_Thread_Entry
