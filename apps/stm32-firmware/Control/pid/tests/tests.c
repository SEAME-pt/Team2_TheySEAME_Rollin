#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "../pid.h"

static int tests_run = 0;
static int tests_failed = 0;

#define FLOAT_EPS 0.0001f

#define ASSERT_FLOAT_EQ(expected, actual)                                          \
    do {                                                                           \
        float exp__ = (expected);                                                  \
        float act__ = (actual);                                                    \
        if (fabsf(exp__ - act__) > FLOAT_EPS) {                                    \
            printf("[FAIL] %s:%d: expected %.6f, got %.6f\n",                      \
                   __FILE__, __LINE__, exp__, act__);                              \
            tests_failed++;                                                        \
            return;                                                                \
        }                                                                          \
    } while (0)

#define ASSERT_TRUE(condition)                                                     \
    do {                                                                           \
        if (!(condition)) {                                                        \
            printf("[FAIL] %s:%d: assertion failed: %s\n",                         \
                   __FILE__, __LINE__, #condition);                                \
            tests_failed++;                                                        \
            return;                                                                \
        }                                                                          \
    } while (0)

#define RUN_TEST(test_fn)                                                          \
    do {                                                                           \
        int failed_before = tests_failed;                                          \
        tests_run++;                                                               \
        PID_Reset();                                                               \
        test_fn();                                                                 \
        if (tests_failed == failed_before) {                                       \
            printf("[PASS] %s\n", #test_fn);                                       \
        }                                                                          \
    } while (0)

/* -------------------------------------------------------------------------- */
/* dsn~pid-output-clamping~1                                                   */
/* -------------------------------------------------------------------------- */

static void test_clamp_returns_maximum_when_value_above_upper_bound(void)
{
    ASSERT_FLOAT_EQ(100.0f, clamp(150.0f));
}

static void test_clamp_returns_minimum_when_value_below_lower_bound(void)
{
    ASSERT_FLOAT_EQ(0.0f, clamp(-10.0f));
}

static void test_clamp_returns_original_value_when_within_bounds(void)
{
    ASSERT_FLOAT_EQ(42.5f, clamp(42.5f));
}

static void test_pid_output_is_clamped_to_maximum(void)
{
    /*
     * error = 100, derivative = 100
     * raw output = 9*100 + 0 + 0.05*100 = 905
     * clamped output = 100
     */
    float output = PID(100.0f, 0.0f, 1.0f);

    ASSERT_FLOAT_EQ(100.0f, output);
}

static void test_pid_output_is_clamped_to_minimum(void)
{
    /*
     * error = 0 - 10 = -10
     * derivative = -10
     * raw output = 9*(-10) + 0 + 0.05*(-10) = -90.5
     * clamped output = 0
     */
    float output = PID(0.0f, 10.0f, 1.0f);

    ASSERT_FLOAT_EQ(0.0f, output);
}


/* -------------------------------------------------------------------------- */
/* dsn~pid-anti-windup~1                                                       */
/* -------------------------------------------------------------------------- */

static void test_anti_windup_blocks_integral_growth_at_upper_saturation(void)
{
    /*
     * Large positive error causes saturation at 100%.
     * Since error > 0 and output_unsat >= max, integral must not accumulate.
     */
    float out1 = PID(100.0f, 0.0f, 1.0f);
    float i1 = PID_GetIntegral();

    float out2 = PID(100.0f, 0.0f, 1.0f);
    float i2 = PID_GetIntegral();

    ASSERT_FLOAT_EQ(100.0f, out1);
    ASSERT_FLOAT_EQ(100.0f, out2);
    ASSERT_FLOAT_EQ(0.0f, i1);
    ASSERT_FLOAT_EQ(0.0f, i2);
}

static void test_anti_windup_blocks_integral_growth_at_lower_saturation(void)
{
    /*
     * Large negative error causes saturation at 0%.
     * Since error < 0 and output_unsat <= min, integral must not accumulate.
     */
    float out1 = PID(0.0f, 100.0f, 1.0f);
    float i1 = PID_GetIntegral();

    float out2 = PID(0.0f, 100.0f, 1.0f);
    float i2 = PID_GetIntegral();

    ASSERT_FLOAT_EQ(0.0f, out1);
    ASSERT_FLOAT_EQ(0.0f, out2);
    ASSERT_FLOAT_EQ(0.0f, i1);
    ASSERT_FLOAT_EQ(0.0f, i2);
}

static void test_integral_accumulates_when_controller_is_not_saturated(void)
{
    /*
     * Small positive error should not saturate, so integral must accumulate.
     */
    PID(10.0f, 9.0f, 1.0f);

    ASSERT_FLOAT_EQ(1.0f, PID_GetIntegral());
}


/* -------------------------------------------------------------------------- */
/* Main                                                                       */
/* -------------------------------------------------------------------------- */

int main(void)
{
    RUN_TEST(test_clamp_returns_maximum_when_value_above_upper_bound);
    RUN_TEST(test_clamp_returns_minimum_when_value_below_lower_bound);
    RUN_TEST(test_clamp_returns_original_value_when_within_bounds);
    RUN_TEST(test_pid_output_is_clamped_to_maximum);
    RUN_TEST(test_pid_output_is_clamped_to_minimum);

    RUN_TEST(test_anti_windup_blocks_integral_growth_at_upper_saturation);
    RUN_TEST(test_anti_windup_blocks_integral_growth_at_lower_saturation);
    RUN_TEST(test_integral_accumulates_when_controller_is_not_saturated);

    printf("\nTests run: %d\n", tests_run);
    printf("Tests failed: %d\n", tests_failed);

    return (tests_failed == 0) ? 0 : 1;
}