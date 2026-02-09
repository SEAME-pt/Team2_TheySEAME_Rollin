#include "mock_tx_api.h"
#include "unity.h"
#include "Sensors/sensors.h"

/* Include SUT directly so the test binary contains the implementation */
#include "../Src/Control/control_queue.c"

void setUp(void) {
    _txe_queue_create_IgnoreAndReturn(TX_SUCCESS);
    ControlQueue_Init();
}

void tearDown(void) { }

void test_ControlQueue_TrySend_Success_NoDrops(void) {
    VehicleCommand_t cmd = { .driving_mode = 1, .throttle = 50, .steering_angle = 0, .command_valid = 1 };
    _txe_queue_send_IgnoreAndReturn(TX_SUCCESS);

    int res = ControlQueue_TrySend(&cmd);
    TEST_ASSERT_EQUAL_INT(1, res);
    TEST_ASSERT_EQUAL_UINT32(0, ControlQueue_GetDrops());
}

void test_ControlQueue_TrySend_Failure_IncrementsDrop(void) {
    VehicleCommand_t cmd = { .driving_mode = 1, .throttle = 50, .steering_angle = 0, .command_valid = 1 };
    _txe_queue_send_IgnoreAndReturn(1); // non-TX_SUCCESS to simulate full/failure

    int res = ControlQueue_TrySend(&cmd);
    TEST_ASSERT_EQUAL_INT(0, res);
    TEST_ASSERT_EQUAL_UINT32(1, ControlQueue_GetDrops());
}

void test_ControlQueue_Send_WaitFailure_IncrementsDrop(void) {
    VehicleCommand_t cmd = { .driving_mode = 2, .throttle = 0, .steering_angle = 0, .command_valid = 0 };
    _txe_queue_send_IgnoreAndReturn(1); // simulate failure even with wait

    int res = ControlQueue_Send(&cmd, 10);
    TEST_ASSERT_EQUAL_INT(0, res);
    TEST_ASSERT_EQUAL_UINT32(1, ControlQueue_GetDrops());
}
