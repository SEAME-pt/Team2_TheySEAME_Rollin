# STM32 Testing Framework: Unity + Ceedling + CMock

## For normal usage
Go to `apps/stm32-firmware/Core/Tests` and run `./setup.sh` to install Ruby and Ceedling on your machine.

Run `ceedling test:all` to run all unit tests

## To create a new ceedling project
```
ceedling new 'project_name'
```

Then you need to edit the project.yml and add the test files to the directory created by ceedling.
Run `ceedling test:all` to run all unit tests

## Create tests with Unity
First `#include "unity.h"`.

Then you need to create two constructor and destructor functions:
```
void setUp(void) {
  // Initialize before each test
  delta_ticks = 0;
  mock_capture_value = 0;
}

void tearDown(void) {
  // Cleanup after each test
}
```

To create a test, use ASSERTIONS or other verifications methods. Refer to [this](https://docs.unity3d.com/6000.2/Documentation/Manual/test-framework/writing-tests.html) manual to know more about them.

```
void test_Speed_CalculateRPM_ValidInput_ReturnsCorrectRPM(void) {
    // Given: delta_ticks = 2000 (100ms between pulses)
    uint32_t delta_ticks = 2000;
    
    // When: Calculate RPM
    uint32_t result = Speed_CalculateRPM(delta_ticks);
    
    // Then: Expected RPM = (60 * 20000) / (2000 * 20) = 30 RPM
    TEST_ASSERT_EQUAL_UINT32(30, result);
}
```