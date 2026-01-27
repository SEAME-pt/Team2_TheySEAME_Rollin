#include "mock_tx_api.h"
#include "unity.h"
#include "Sensors/sensors.h"

/* Include SUT directly so the test binary contains the implementation
 * (consistent with existing tests pattern until project.yml sources are
 * fully managed). */
#include "../Src/Sensors/sensors_queue.c"

void setUp(void) {
    // Ensure queue is initialized and drops counter reset before each test
    _txe_queue_create_IgnoreAndReturn(TX_SUCCESS);
    SensorsQueue_Init();
}

void tearDown(void) { }

void test_SensorsQueue_TrySend_Success_NoDrops(void) {
    SensorSample_t s = { .sensor_id = SENSOR_ID_SPEED, .value = 1.23f, .ts = 0 };
    _txe_queue_send_IgnoreAndReturn(TX_SUCCESS);

    int res = SensorsQueue_TrySend(&s);
    TEST_ASSERT_EQUAL_INT(1, res);
    TEST_ASSERT_EQUAL_UINT32(0, SensorsQueue_GetDrops());
}

void test_SensorsQueue_TrySend_Failure_IncrementsDrop(void) {
    SensorSample_t s = { .sensor_id = SENSOR_ID_SPEED, .value = 2.34f, .ts = 0 };
    _txe_queue_send_IgnoreAndReturn(1); // non-TX_SUCCESS to simulate full/failure

    int res = SensorsQueue_TrySend(&s);
    TEST_ASSERT_EQUAL_INT(0, res);
    TEST_ASSERT_EQUAL_UINT32(1, SensorsQueue_GetDrops());
}

void test_SensorsQueue_Send_WaitFailure_IncrementsDrop(void) {
    SensorSample_t s = { .sensor_id = SENSOR_ID_BATTERY, .value = 99.0f, .ts = 0 };
    _txe_queue_send_IgnoreAndReturn(1); // simulate failure even with wait

    int res = SensorsQueue_Send(&s, 10);
    TEST_ASSERT_EQUAL_INT(0, res);
    TEST_ASSERT_EQUAL_UINT32(1, SensorsQueue_GetDrops());
}
