#include "main.h"

void Test_Thread_Entry(ULONG thread_input) {
    testRoutine();
}


void testRoutine() {
    HAL_StatusTypeDef ret;

     // --- THROTTLE MOTOR H-BRIDGE TEST (PCA9685 @ 0x60) ---
       // This test runs both motors forward at 50% duty for 2 seconds, then stops all motors.
       // Channels: 0 = M1 speed, 1 = M1 DIR1, 2 = M1 DIR2, 3 = M2 speed, 4 = M2 speed, 5 = M2 DIR2, 6 = M2 DIR1, 7 = M2 speed
 
       // Set both motors forward, 50% duty
       uint16_t speed_pwm = 4095; // 50% of 4095
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
 
       tx_thread_sleep(500); // 2 seconds
 
       // Stop all motors (set all speed and direction channels to 0)
     for (uint8_t ch = 0; ch < 8; ++ch) {
       PCA9685_SetPWM(&hi2c1, PCA9685_ADDR_THROTTLE, ch, 0, 0);
     }
}
