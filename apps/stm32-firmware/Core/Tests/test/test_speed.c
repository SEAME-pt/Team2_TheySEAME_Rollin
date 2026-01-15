#include "mock_tx_api.h"
#include "unity.h"
#include "sensors.h"

// Mock HAL definitions for testing
typedef struct {
    void* Instance;
    uint32_t Channel;
} TIM_HandleTypeDef;

#define TIM1 ((void*)0x40010000UL)
#define HAL_TIM_ACTIVE_CHANNEL_3 (0x04UL)
#define TIM_CHANNEL_3 (0x08UL)

// Mock HAL function
uint32_t mock_capture_value = 0;
uint32_t HAL_TIM_ReadCapturedValue(TIM_HandleTypeDef *htim, uint32_t Channel) {
    (void)htim;
    (void)Channel;
    return mock_capture_value;
}

// External reference to delta_ticks global variable for testing
extern uint32_t delta_ticks;

/* Note: Including the SUT .c file is usually an anti-pattern because it
 * couples the test to the implementation compilation unit. This was the
 * previous behavior and is restored here to ensure the test binary contains
 * the speed implementation while we adjust Ceedling project settings to
 * compile SUT sources per-test. Once project.yml is configured so SUT
 * sources are compiled into the test executable, remove this include and
 * rely on the build system instead.
 */
#include "../Src/Sensors/speed.c"

void setUp(void) {
  // Initialize before each test
  delta_ticks = 0;
  mock_capture_value = 0;
}

void tearDown(void) {
  // Cleanup after each test
}

/**
 * @brief Test Speed_CalculateRPM function with valid input
 * 
 * Tests the RPM calculation with normal input values.
 * Timer frequency is 20kHz (50µs per tick).
 * Expected RPM = (60 * 20000) / (delta_ticks * 10)
 */
void test_Speed_CalculateRPM_ValidInput_ReturnsCorrectRPM(void) {
    // Given: delta_ticks = 2000 (100ms between pulses)
    uint32_t delta_ticks = 2000;
    
    // When: Calculate RPM
    uint32_t result = Speed_CalculateRPM(delta_ticks);
    
    // Then: Expected RPM = (60 * 20000) / (2000 * 20) = 30 RPM
    TEST_ASSERT_EQUAL_UINT32(30, result);
}

/**
 * @brief Test Speed_CalculateRPM function with noise filtering
 * 
 * Tests that the function filters out noise by returning 0 for 
 * delta values below the threshold (20 ticks).
 */
void test_Speed_CalculateRPM_NoiseFiltering_ReturnsZero(void) {
    // Given: delta_ticks below noise threshold
    uint32_t delta_ticks = 15; // Below 20 tick threshold
    
    // When: Calculate RPM
    uint32_t result = Speed_CalculateRPM(delta_ticks);
    
    // Then: Should return 0 (noise filtered)
    TEST_ASSERT_EQUAL_UINT32(0, result);
}

/**
 * @brief Test Speed_CalculateRPM function with edge case (minimum valid input)
 * 
 * Tests the function behavior at the noise filtering threshold boundary.
 */
void test_Speed_CalculateRPM_MinimumValidInput_ReturnsRPM(void) {
    // Given: delta_ticks at threshold boundary
    uint32_t delta_ticks = 20; // Exactly at threshold
    
    // When: Calculate RPM
    uint32_t result = Speed_CalculateRPM(delta_ticks);
    
    // Then: Expected RPM = (60 * 20000) / (20 * 20) = 3000 RPM
    TEST_ASSERT_EQUAL_UINT32(3000, result);
}

/**
 * @brief Test Speed_RPMToMetersPerSecond function with zero RPM
 * 
 * Tests conversion of zero RPM to meters per second.
 */
void test_Speed_RPMToMetersPerSecond_ZeroRPM_ReturnsZero(void) {
    // Given: Zero RPM
    uint32_t rpm = 0;
    
    // When: Convert to m/s
    float result = Speed_RPMToMetersPerSecond(rpm);
    
    // Then: Should return 0.0
    TEST_ASSERT_EQUAL_FLOAT(0.0f, result);
}

/**
 * @brief Test Speed_RPMToMetersPerSecond function with typical RPM value
 * 
 * Tests conversion of a typical RPM value to meters per second.
 * Wheel circumference is 0.21 meters.
 * Formula: m/s = RPM * 0.21 / 60
 */
void test_Speed_RPMToMetersPerSecond_TypicalRPM_ReturnsCorrectSpeed(void) {
    // Given: 60 RPM (1 revolution per second)
    uint32_t rpm = 60;
    
    // When: Convert to m/s
    float result = Speed_RPMToMetersPerSecond(rpm);
    
    // Then: Expected speed = 60 * 0.21 / 60 = 0.21 m/s
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.21f, result);
}

/**
 * @brief Test Speed_RPMToMetersPerSecond function with high RPM value
 * 
 * Tests conversion with a higher RPM value to ensure accuracy.
 */
void test_Speed_RPMToMetersPerSecond_HighRPM_ReturnsCorrectSpeed(void) {
    // Given: 1200 RPM
    uint32_t rpm = 1200;
    
    // When: Convert to m/s
    float result = Speed_RPMToMetersPerSecond(rpm);
    
    // Then: Expected speed = 1200 * 0.21 / 60 = 4.2 m/s
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 4.2f, result);
}

/**
 * @brief Test Speed_ProcessDelta function with noise-filtered input
 * 
 * Tests that the function properly handles noise-filtered deltas 
 * (when Speed_CalculateRPM returns 0).
 */
void test_Speed_ProcessDelta_NoiseFiltered_ReturnsZero(void) {
    // Given: Initial state and noise delta
    uint32_t average = 0;
    int counter = 0;
    uint32_t delta_ticks = 10; // Below noise threshold
    
    // When: Process delta
    int result = Speed_ProcessDelta(delta_ticks, &average, &counter);
    
    // Then: Should return 0 (no output), average and counter unchanged
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_UINT32(0, average);
    TEST_ASSERT_EQUAL_INT(0, counter);
}

/**
 * @brief Test Speed_ProcessDelta function accumulating readings
 * 
 * Tests that the function properly accumulates RPM readings
 * without generating output until 5 readings are collected.
 */
void test_Speed_ProcessDelta_AccumulatingReadings_ReturnsZero(void) {
    // Given: Initial state and valid delta
    uint32_t average = 0;
    int counter = 0;
    uint32_t delta_ticks = 2000; // Valid delta (30 RPM)
    
    // When: Process first delta
    int result = Speed_ProcessDelta(delta_ticks, &average, &counter);
    
    // Then: Should return 0 (still accumulating), average and counter updated
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_UINT32(30, average); // RPM added to average (30 for delta=2000)
    TEST_ASSERT_EQUAL_INT(1, counter);
}

/**
 * @brief Test Speed_ProcessDelta function generating output after 5 readings
 * 
 * Tests that the function generates output and resets counters
 * after accumulating 5 valid readings.
 */
void test_Speed_ProcessDelta_FifthReading_ReturnsOne(void) {
    // Given: State with 4 accumulated readings
    uint32_t average = 240; // 4 readings of 60 RPM each
    int counter = 4;
    uint32_t delta_ticks = 2000; // 5th reading (30 RPM)

    // Expect mutex calls during output - ignore them in tests
    _txe_mutex_get_IgnoreAndReturn(TX_SUCCESS);
    _txe_mutex_put_IgnoreAndReturn(TX_SUCCESS);

    // When: Process fifth delta
    int result = Speed_ProcessDelta(delta_ticks, &average, &counter);
    
    // Then: Should return 1 (output generated), counters reset, average calculated
    TEST_ASSERT_EQUAL_INT(1, result);
    TEST_ASSERT_EQUAL_UINT32(0, average); // Reset after output
    TEST_ASSERT_EQUAL_INT(0, counter);    // Reset after output
}

/**
 * @brief Test HAL_TIM_IC_CaptureCallback with correct timer and channel
 * 
 * Tests the capture callback function with the expected TIM1 and Channel 3.
 * Tests normal operation where current capture >= previous capture.
 */
void test_HAL_TIM_IC_CaptureCallback_CorrectTimerChannel_UpdatesDelta(void) {
    // Given: TIM1 handle with Channel 3 active
    TIM_HandleTypeDef htim;
    htim.Instance = TIM1;
    htim.Channel = HAL_TIM_ACTIVE_CHANNEL_3;
    
    // First capture (may set prev_capture depending on static state) - make deterministic by clearing delta
    mock_capture_value = 1000;
    HAL_TIM_IC_CaptureCallback(&htim);
    delta_ticks = 0; // ensure deterministic start for next capture
    
    // When: Second capture with normal increment (simulate second capture at 3000)
    mock_capture_value = 3000;
    HAL_TIM_IC_CaptureCallback(&htim);
    
    // Then: delta_ticks should be updated with difference
    TEST_ASSERT_EQUAL_UINT32(2000, delta_ticks);
}

/**
 * @brief Test HAL_TIM_IC_CaptureCallback with timer overflow condition
 * 
 * Tests the capture callback function when timer overflow occurs
 * (current capture < previous capture).
 */
void test_HAL_TIM_IC_CaptureCallback_TimerOverflow_HandlesCorrectly(void) {
    // Given: TIM1 handle with Channel 3 active
    TIM_HandleTypeDef htim;
    htim.Instance = TIM1;
    htim.Channel = HAL_TIM_ACTIVE_CHANNEL_3;
    
    // First call to establish prev_capture near end of timer range
    mock_capture_value = 0xFFFE; // Near maximum 16-bit value
    HAL_TIM_IC_CaptureCallback(&htim);
    
    // When: Timer overflows and captures at beginning of range
    mock_capture_value = 0x0010; // Small value after overflow
    HAL_TIM_IC_CaptureCallback(&htim);
    
    // Then: delta_ticks should handle overflow correctly
    // Expected: (0xFFFF - 0xFFFE + 1) + 0x0010 = 2 + 16 = 18
    TEST_ASSERT_EQUAL_UINT32(18, delta_ticks);
}

/**
 * @brief Test HAL_TIM_IC_CaptureCallback with wrong timer instance
 * 
 * Tests that the callback function ignores calls from wrong timer instances.
 */
void test_HAL_TIM_IC_CaptureCallback_WrongTimer_IgnoresCall(void) {
    // Given: Wrong timer instance (not TIM1)
    TIM_HandleTypeDef htim;
    htim.Instance = (void*)0x40000000UL; // Different timer
    htim.Channel = HAL_TIM_ACTIVE_CHANNEL_3;
    mock_capture_value = 1000;
    
    // When: Callback is called
    HAL_TIM_IC_CaptureCallback(&htim);
    
    // Then: delta_ticks should remain unchanged
    TEST_ASSERT_EQUAL_UINT32(0, delta_ticks);
}

/**
 * @brief Test HAL_TIM_IC_CaptureCallback with wrong channel
 * 
 * Tests that the callback function ignores calls from wrong channels.
 */
void test_HAL_TIM_IC_CaptureCallback_WrongChannel_IgnoresCall(void) {
    // Given: Correct timer but wrong channel
    TIM_HandleTypeDef htim;
    htim.Instance = TIM1;
    htim.Channel = 0x01UL; // Wrong channel (not Channel 3)
    mock_capture_value = 1000;
    
    // When: Callback is called
    HAL_TIM_IC_CaptureCallback(&htim);
    
    // Then: delta_ticks should remain unchanged
    TEST_ASSERT_EQUAL_UINT32(0, delta_ticks);
}

/**
 * @brief Test HAL_TIM_IC_CaptureCallback with consecutive calls
 * 
 * Tests multiple consecutive calls to verify static variable behavior
 * and proper delta calculation between captures.
 */
void test_HAL_TIM_IC_CaptureCallback_ConsecutiveCalls_UpdatesCorrectly(void) {
    // Given: TIM1 handle with Channel 3 active
    TIM_HandleTypeDef htim;
    htim.Instance = TIM1;
    htim.Channel = HAL_TIM_ACTIVE_CHANNEL_3;
    
    // First capture - make deterministic by clearing delta after initial call
    mock_capture_value = 500;
    HAL_TIM_IC_CaptureCallback(&htim);
    delta_ticks = 0;

    // Second capture
    mock_capture_value = 1500;
    HAL_TIM_IC_CaptureCallback(&htim);
    TEST_ASSERT_EQUAL_UINT32(1000, delta_ticks); // 1500 - 500
    
    // Third capture
    mock_capture_value = 3000;
    HAL_TIM_IC_CaptureCallback(&htim);
    TEST_ASSERT_EQUAL_UINT32(1500, delta_ticks); // 3000 - 1500
    
    // Fourth capture
    mock_capture_value = 3200;
    HAL_TIM_IC_CaptureCallback(&htim);
    TEST_ASSERT_EQUAL_UINT32(200, delta_ticks); // 3200 - 3000
}