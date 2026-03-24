/**
 * @file test_cruise_control.c
 * @brief Unit tests for cruise_control.c
 *
 * Covers:  
 *   dsn~pid-output-clamping~1
 *   dsn~pid-anti-windup~1
 *
 * Build alongside cruise_control.c (with stubs below) and link Unity.
 */

#include "../../Src/Control/cruise_control.h"

/* -----------------------------------------------------------------------
 * Minimal stubs – replace platform calls that are unavailable in host
 * test environment.
 * --------------------------------------------------------------------- */

/* HAL_GetTick: return a monotonically increasing value so dt is stable */
static uint32_t fake_tick = 0;
uint32_t HAL_GetTick(void) { return fake_tick; }

/* Debug_Print: swallow trace strings */
void Debug_Print(const char *s) { (void)s; }

/* Control_SetThrottle: capture the last throttle value set */
static float last_throttle = -1.0f;
void Control_SetThrottle(float throttle, int channel)
{
    (void)channel;
    last_throttle = throttle;
}

/* -----------------------------------------------------------------------
 * Helpers
 * --------------------------------------------------------------------- */

/** Advance the fake HAL tick by @p ms milliseconds. */
static void tick_advance(uint32_t ms) { fake_tick += ms; }

/* -----------------------------------------------------------------------
 * setUp / tearDown
 * --------------------------------------------------------------------- */

void setUp(void)
{
    fake_tick   = 0;
    last_throttle = -1.0f;
    PID_Reset();
}

void tearDown(void) { }

/* =======================================================================
 * dsn~pid-output-clamping~1
 *
 * "The PID controller output shall be clamped to the range [0%, 100%]
 *  before being applied as a throttle command."
 * ===================================================================== */

/**
 * @covers dsn~pid-output-clamping~1
 *
 * When the set-point is far above the current value the raw PID output
 * would exceed 100 %.  The returned value must be exactly 100 %.
 */
void test_PID_OutputClampedToMax(void)
{
    /* Huge error → raw output >> 100 before clamping */
    float out = PID(100.0f, 0.0f, 0.1f);
    TEST_ASSERT_EQUAL_FLOAT(PID_OUTPUT_MAX, out);
}

/**
 * @covers dsn~pid-output-clamping~1
 *
 * clamp() with a value above PID_OUTPUT_MAX must return PID_OUTPUT_MAX.
 */
void test_Clamp_AboveMax_ReturnsMax(void)
{
    TEST_ASSERT_EQUAL_FLOAT(PID_OUTPUT_MAX, clamp(150.0f));
    TEST_ASSERT_EQUAL_FLOAT(PID_OUTPUT_MAX, clamp(PID_OUTPUT_MAX + 0.001f));
}

/**
 * @covers dsn~pid-output-clamping~1
 *
 * clamp() with a value below PID_OUTPUT_MIN must return PID_OUTPUT_MIN.
 */
void test_Clamp_BelowMin_ReturnsMin(void)
{
    TEST_ASSERT_EQUAL_FLOAT(PID_OUTPUT_MIN, clamp(-50.0f));
    TEST_ASSERT_EQUAL_FLOAT(PID_OUTPUT_MIN, clamp(PID_OUTPUT_MIN - 0.001f));
}

/**
 * @covers dsn~pid-output-clamping~1
 *
 * clamp() with a value inside [0, 100] must return the value unchanged.
 */
void test_Clamp_InsideRange_ReturnsUnchanged(void)
{
    TEST_ASSERT_EQUAL_FLOAT(0.0f,   clamp(0.0f));
    TEST_ASSERT_EQUAL_FLOAT(50.0f,  clamp(50.0f));
    TEST_ASSERT_EQUAL_FLOAT(100.0f, clamp(100.0f));
}

/**
 * @covers dsn~pid-output-clamping~1
 *
 * PID output is never negative even when set-point < current value.
 */
void test_PID_OutputNeverNegative(void)
{
    /* current_speed far above set_point → negative raw output */
    float out = PID(0.0f, 100.0f, 0.1f);
    TEST_ASSERT_GREATER_OR_EQUAL_FLOAT(PID_OUTPUT_MIN, out);
}

/* =======================================================================
 * dsn~pid-anti-windup~1
 *
 * "The PID controller shall implement integral anti-windup by suspending
 *  the accumulation of the integral term whenever the controller output
 *  is saturated and the sign of the error would further increase the
 *  magnitude of the integrator."
 * ===================================================================== */

/**
 * @covers dsn~pid-anti-windup~1
 *
 * When the output is clamped at the maximum and the error is positive
 * (which would wind the integral up further), the integral must NOT grow.
 */
void test_AntiWindup_IntegralFrozenAtUpperSaturation(void)
{
    /* Drive output to upper saturation with a large positive error */
    PID(100.0f, 0.0f, 0.1f);
    float integral_after_first = PID_GetIntegral();

    /* Another call with the same conditions – integral must not grow */
    PID(100.0f, 0.0f, 0.1f);
    float integral_after_second = PID_GetIntegral();

    TEST_ASSERT_EQUAL_FLOAT(integral_after_first, integral_after_second);
}

/**
 * @covers dsn~pid-anti-windup~1
 *
 * When the output is clamped at the minimum and the error is negative
 * (which would wind the integral down further), the integral must NOT decrease.
 */
void test_AntiWindup_IntegralFrozenAtLowerSaturation(void)
{
    PID(0.0f, 100.0f, 0.1f);
    float integral_after_first = PID_GetIntegral();

    PID(0.0f, 100.0f, 0.1f);
    float integral_after_second = PID_GetIntegral();

    TEST_ASSERT_EQUAL_FLOAT(integral_after_first, integral_after_second);
}

/**
 * @covers dsn~pid-anti-windup~1
 *
 * When the output is NOT saturated the integral must accumulate normally
 * (i.e. its magnitude must grow with each call while an error is present).
 */
void test_AntiWindup_IntegralAccumulatesWhenUnsaturated(void)
{
    /* Small positive error that keeps output inside (0, 100) */
    float sp = 1.0f;
    float pv = 0.99f; /* error = 0.01, output will be small */

    PID(sp, pv, 0.1f);
    float integral_first = PID_GetIntegral();

    PID(sp, pv, 0.1f);
    float integral_second = PID_GetIntegral();

    /* Integral must have grown */
    TEST_ASSERT_GREATER_THAN_FLOAT(integral_first, integral_second);
}

/**
 * @covers dsn~pid-anti-windup~1
 *
 * When the output is saturated high but the error is NEGATIVE (error
 * would unwind the integrator), accumulation must be PERMITTED so the
 * controller can recover.
 */
void test_AntiWindup_IntegralAllowedToDecreaseWhenOutputHighButErrorNegative(void)
{
    /* First push the integral positive */
    PID(100.0f, 0.0f, 0.1f);
    float integral_before = PID_GetIntegral();

    /* Now the plant overshoots: current > set_point, error is negative.
     * Output is still ≥ max due to FF, but anti-windup must allow
     * the integral to decrease (unwind). */
    PID(1.0f, 200.0f, 0.1f); /* large negative error, output saturated low */
    float integral_after = PID_GetIntegral();

    TEST_ASSERT_LESS_THAN_FLOAT(integral_before, integral_after);
}

/**
 * @covers dsn~pid-anti-windup~1
 *
 * PID_Reset clears the integral so repeated enable/disable cycles
 * (as used by cruise_control()) do not carry stale windup between
 * activation sessions.
 */
void test_PID_Reset_ClearsIntegral(void)
{
    PID(100.0f, 0.0f, 0.1f); /* accumulate some integral */
    PID_Reset();
    TEST_ASSERT_EQUAL_FLOAT(0.0f, PID_GetIntegral());
}

/**
 * @covers dsn~pid-anti-windup~1
 *
 * The integral is bounded by PID_INTEGRAL_MAX even when anti-windup is
 * NOT active (e.g. unsaturated region with persistent error).
 */
void test_AntiWindup_IntegralBoundedByMax(void)
{
    /* Tiny error, unsaturated output – run many iterations */
    for (int i = 0; i < 10000; i++) {
        PID(0.5f, 0.49f, 0.1f);
    }
    TEST_ASSERT_LESS_OR_EQUAL_FLOAT(PID_INTEGRAL_MAX, PID_GetIntegral());
}

/**
 * @covers dsn~pid-anti-windup~1
 *
 * The integral is bounded by PID_INTEGRAL_MIN (negative direction).
 */
void test_AntiWindup_IntegralBoundedByMin(void)
{
    for (int i = 0; i < 10000; i++) {
        PID(0.49f, 0.5f, 0.1f);
    }
    TEST_ASSERT_GREATER_OR_EQUAL_FLOAT(PID_INTEGRAL_MIN, PID_GetIntegral());
}

/* =======================================================================
 * PID_Reset / dt guard
 * ===================================================================== */

/**
 * dt <= 0 must return 0 immediately without touching state.
 */
void test_PID_ZeroDt_ReturnsZero(void)
{
    float out = PID(1.0f, 0.5f, 0.0f);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, out);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, PID_GetIntegral());
}

void test_PID_NegativeDt_ReturnsZero(void)
{
    float out = PID(1.0f, 0.5f, -0.05f);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, out);
}

/* =======================================================================
 * main
 * ===================================================================== */

int main(void)
{
    UNITY_BEGIN();

    /* dsn~pid-output-clamping~1 */
    RUN_TEST(test_PID_OutputClampedToMax);
    RUN_TEST(test_Clamp_AboveMax_ReturnsMax);
    RUN_TEST(test_Clamp_BelowMin_ReturnsMin);
    RUN_TEST(test_Clamp_InsideRange_ReturnsUnchanged);
    RUN_TEST(test_PID_OutputNeverNegative);

    /* dsn~pid-anti-windup~1 */
    RUN_TEST(test_AntiWindup_IntegralFrozenAtUpperSaturation);
    RUN_TEST(test_AntiWindup_IntegralFrozenAtLowerSaturation);
    RUN_TEST(test_AntiWindup_IntegralAccumulatesWhenUnsaturated);
    RUN_TEST(test_AntiWindup_IntegralAllowedToDecreaseWhenOutputHighButErrorNegative);
    RUN_TEST(test_PID_Reset_ClearsIntegral);
    RUN_TEST(test_AntiWindup_IntegralBoundedByMax);
    RUN_TEST(test_AntiWindup_IntegralBoundedByMin);

    /* dt guard */
    RUN_TEST(test_PID_ZeroDt_ReturnsZero);
    RUN_TEST(test_PID_NegativeDt_ReturnsZero);

    return UNITY_END();
}