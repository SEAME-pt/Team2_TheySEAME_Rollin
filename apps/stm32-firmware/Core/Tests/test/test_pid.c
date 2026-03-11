#include "unity.h"
#include <math.h>
#include "Control/pid.h"

/* Include SUT directly so the test binary contains the implementation,
 * consistent with the existing test pattern. */
#include "../Src/Control/pid.c"

#define FLOAT_EPS 0.0001f

void setUp(void)
{
    PID_Reset();
}

void tearDown(void)
{
}

void test_clamp_returns_maximum_when_value_above_upper_bound(void)
{
    TEST_ASSERT_FLOAT_WITHIN(FLOAT_EPS, 100.0f, clamp(150.0f));
}

void test_clamp_returns_minimum_when_value_below_lower_bound(void)
{
    TEST_ASSERT_FLOAT_WITHIN(FLOAT_EPS, 0.0f, clamp(-10.0f));
}

void test_clamp_returns_original_value_when_within_bounds(void)
{
    TEST_ASSERT_FLOAT_WITHIN(FLOAT_EPS, 42.5f, clamp(42.5f));
}

void test_pid_output_is_clamped_to_maximum(void)
{
    /*
     * error = 100 - 0 = 100
     * derivative = (100 - 0) / 1 = 100
     * raw output = 9*100 + 0 + 0.05*100 = 905
     * clamped output = 100
     */
    float output = PID(100.0f, 0.0f, 1.0f);

    TEST_ASSERT_FLOAT_WITHIN(FLOAT_EPS, 100.0f, output);
}

void test_pid_output_is_clamped_to_minimum(void)
{
    /*
     * error = 0 - 10 = -10
     * derivative = (-10 - 0) / 1 = -10
     * raw output = 9*(-10) + 0 + 0.05*(-10) = -90.5
     * clamped output = 0
     */
    float output = PID(0.0f, 10.0f, 1.0f);

    TEST_ASSERT_FLOAT_WITHIN(FLOAT_EPS, 0.0f, output);
}

void test_anti_windup_blocks_integral_growth_at_upper_saturation(void)
{
    /*
     * Large positive error causes saturation at 100%.
     * Since error > 0 and output_unsat >= max, integral must not accumulate.
     */
    float out1 = PID(100.0f, 0.0f, 1.0f);
    float i1 = PID_GetIntegral();

    float out2 = PID(100.0f, 0.0f, 1.0f);
    float i2 = PID_GetIntegral();

    TEST_ASSERT_FLOAT_WITHIN(FLOAT_EPS, 100.0f, out1);
    TEST_ASSERT_FLOAT_WITHIN(FLOAT_EPS, 100.0f, out2);
    TEST_ASSERT_FLOAT_WITHIN(FLOAT_EPS, 0.0f, i1);
    TEST_ASSERT_FLOAT_WITHIN(FLOAT_EPS, 0.0f, i2);
}

void test_anti_windup_blocks_integral_growth_at_lower_saturation(void)
{
    /*
     * Large negative error causes saturation at 0%.
     * Since error < 0 and output_unsat <= min, integral must not accumulate.
     */
    float out1 = PID(0.0f, 100.0f, 1.0f);
    float i1 = PID_GetIntegral();

    float out2 = PID(0.0f, 100.0f, 1.0f);
    float i2 = PID_GetIntegral();

    TEST_ASSERT_FLOAT_WITHIN(FLOAT_EPS, 0.0f, out1);
    TEST_ASSERT_FLOAT_WITHIN(FLOAT_EPS, 0.0f, out2);
    TEST_ASSERT_FLOAT_WITHIN(FLOAT_EPS, 0.0f, i1);
    TEST_ASSERT_FLOAT_WITHIN(FLOAT_EPS, 0.0f, i2);
}

void test_integral_accumulates_when_controller_is_not_saturated(void)
{
    /*
     * Small positive error should not saturate, so integral must accumulate.
     */
    PID(10.0f, 9.0f, 1.0f);

    TEST_ASSERT_FLOAT_WITHIN(FLOAT_EPS, 1.0f, PID_GetIntegral());
}

void test_pid_returns_zero_when_dt_is_not_positive(void)
{
    float output = PID(10.0f, 8.0f, 0.0f);

    TEST_ASSERT_FLOAT_WITHIN(FLOAT_EPS, 0.0f, output);
    TEST_ASSERT_FLOAT_WITHIN(FLOAT_EPS, 0.0f, PID_GetIntegral());
}