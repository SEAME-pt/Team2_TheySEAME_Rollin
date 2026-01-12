#include <gtest/gtest.h>
#include <stdint.h>
#include "sensors.h"
#include "mock_utils.h"      // For Debug_Print mock
#include "mock_tx_api.h"     // For ThreadX function mocks

#define PULSES_PER_REV 10  // Must match speed.c

// Test fixture for speed tests
class SpeedTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Reset all mocks before each test
        mock_utils_Reset();
        mock_tx_api_Reset();
    }
    
    void TearDown() override {
        // Verify all mock expectations after each test
        mock_utils_Verify();
        mock_tx_api_Verify();
    }
};

// Test global variables (these are extern in sensors.h)
VehicleData_t g_vehicle_data;
TX_MUTEX g_vehicle_data_mutex;
VehicleCommand_t g_vehicle_command;
TX_MUTEX g_vehicle_command_mutex;

TEST_F(SpeedTest, CalculateRPM_IgnoresSmallDelta_ReturnsZero) {
    uint32_t delta_ticks = 5; // Below threshold (< 20)
    uint32_t rpm = Speed_CalculateRPM(delta_ticks);
    EXPECT_EQ(rpm, 0);
}

TEST_F(SpeedTest, CalculateRPM_CorrectCalculation_ReturnsExpectedRPM) {
    // Given a delta_ticks that should yield a known RPM
    uint32_t delta_ticks = 1000;
    uint32_t expected_rpm = (60 * 20000) / (delta_ticks * PULSES_PER_REV);
    uint32_t actual_rpm = Speed_CalculateRPM(delta_ticks);
    EXPECT_EQ(actual_rpm, expected_rpm);
}

TEST_F(SpeedTest, CalculateRPM_ValidInput_ReturnsCorrectRPM) {
    // Timer frequency = 160MHz / 8000 = 20kHz
    // For 1000 RPM: RPM = (60 * 20000) / (delta_ticks * 10)
    // Rearranging: delta_ticks = (60 * 20000) / (1000 * 10) = 120
    uint32_t delta_ticks = 120;
    uint32_t expected_rpm = 1000;
    
    uint32_t actual_rpm = Speed_CalculateRPM(delta_ticks);
    
    EXPECT_EQ(actual_rpm, expected_rpm);
}

TEST_F(SpeedTest, Speed_RPMToMetersPerSecond_CorrectConversion) {
    uint32_t rpm = 60;
    float expected_ms = (float)rpm * 0.21f / 60.0f;
    float actual_ms = Speed_RPMToMetersPerSecond(rpm);
    EXPECT_FLOAT_EQ(actual_ms, expected_ms);
}

TEST_F(SpeedTest, Speed_ProcessDelta_ZeroRPM_ReturnsZero) {
    uint32_t average = 0;
    int counter = 0;
    uint32_t delta_ticks = 5; // Below threshold, will return 0 RPM
    
    int result = Speed_ProcessDelta(delta_ticks, &average, &counter);
    
    EXPECT_EQ(result, 0);
    EXPECT_EQ(average, 0);
    EXPECT_EQ(counter, 0);
}

TEST_F(SpeedTest, Speed_ProcessDelta_CounterNotFive_ReturnsZero) {
    uint32_t average = 0;
    int counter = 0;
    uint32_t delta_ticks = 120; // Valid delta that gives 1000 RPM
    
    // Call 4 times (counter should reach 4, not 5)
    for (int i = 0; i < 4; i++) {
        int result = Speed_ProcessDelta(delta_ticks, &average, &counter);
        EXPECT_EQ(result, 0); // Should return 0 until counter reaches 5
    }
    
    EXPECT_EQ(counter, 4);
    EXPECT_EQ(average, 4000); // 4 * 1000 RPM
}

TEST_F(SpeedTest, Speed_ProcessDelta_CounterReachesFive_ReturnsOne) {
    uint32_t average = 0;
    int counter = 0;
    uint32_t delta_ticks = 120; // Valid delta that gives 1000 RPM
    
    // Set up mock expectations
    Debug_Print_Expect("[SPEED THREAD] RPM = 1000, Speed (m/s) = 3.50\r\n");
    tx_mutex_get_ExpectAndReturn(&g_vehicle_command_mutex, TX_WAIT_FOREVER, TX_SUCCESS);
    tx_mutex_put_ExpectAndReturn(&g_vehicle_command_mutex, TX_SUCCESS);
    
    // Call 5 times
    int result = 0;
    for (int i = 0; i < 5; i++) {
        result = Speed_ProcessDelta(delta_ticks, &average, &counter);
    }
    
    EXPECT_EQ(result, 1); // Should return 1 when counter reaches 5
    EXPECT_EQ(counter, 0); // Should be reset
    EXPECT_EQ(average, 0); // Should be reset
    
    // Check that vehicle data was updated (approximately 3.5 m/s for 1000 RPM)
    EXPECT_NEAR(g_vehicle_data.vehicle_speed, 3.5f, 0.1f);
}

TEST_F(SpeedTest, Speed_ProcessDelta_MutexFail_StillResetsCounters) {
    uint32_t average = 0;
    int counter = 0;
    uint32_t delta_ticks = 120; // Valid delta that gives 1000 RPM
    
    // Set up mock expectations - mutex fails
    Debug_Print_Expect("[SPEED THREAD] RPM = 1000, Speed (m/s) = 3.50\r\n");
    tx_mutex_get_ExpectAndReturn(&g_vehicle_command_mutex, TX_WAIT_FOREVER, 1); // Fail
    // No tx_mutex_put expected since mutex_get failed
    
    // Call 5 times
    int result = 0;
    for (int i = 0; i < 5; i++) {
        result = Speed_ProcessDelta(delta_ticks, &average, &counter);
    }
    
    EXPECT_EQ(result, 1); // Should still return 1
    EXPECT_EQ(counter, 0); // Should be reset even if mutex fails
    EXPECT_EQ(average, 0); // Should be reset even if mutex fails
}
