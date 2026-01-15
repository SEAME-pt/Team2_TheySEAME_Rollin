#include "unity.h"
#include "mcp2515.h"

/*
 * Tests for MCP2515 CAN communication driver
 * - Verifies encoding and transmission behavior for speed and battery messages
 * - Exercises edge cases (clamping), TX busy path, and timeout/error recovery
 * - Uses fakes to control register reads and system ticks and to capture writes
 *
 * ====================== Test Traceability ===========================
 * [test->dsn~can-telemetry-tx~1]
 * ==========================================================================
 */

/* Fakes/Helpers declared in test support */
extern void fake_mcp2515_reset(void);
extern void fake_mcp2515_set_read_sequence(const uint8_t *seq, int len);
extern void fake_mcp2515_get_writes(uint8_t *addrs, uint8_t *vals, int *count);
extern void fake_mcp2515_set_tick_sequence(const uint32_t *seq, int len);
extern x\void fake_mcp2515_reset_state(void);

void setUp(void) {
    fake_mcp2515_reset_state();
}

void tearDown(void) {
}

/* test_MCP2515_SendSpeed_ValidSpeed_Success
 * Purpose: Verify a normal speed value is converted and written correctly.
 * Setup: Provide register read sequence where TX buffer is free and ticks stable.
 * Assert: Function returns HAL_OK and TXB0DATA was written with the expected dm/s byte (12).
 *
 * ====================== Test Traceability ===========================
 * [test->dsn~can-telemetry-tx~1]
 * ==========================================================================
 */
void test_MCP2515_SendSpeed_ValidSpeed_Success(void)
{
    /* Arrange: speed = 1.23 m/s -> 12.3 -> byte=12 */
    const uint8_t reads[] = {
        /* initial TXB0CTRL (not busy) */ 0x00,
        /* SIDH, SIDL, DLC */ 0x08, 0x40, 0x01,
        /* CANSTAT, CANCTRL */ 0x00, 0x00,
        /* TXB0CTRL (poll - done) */ 0x00,
        /* EFLG, CANINTF */ 0x00, 0x00
    };
    fake_mcp2515_set_read_sequence(reads, sizeof(reads));
    const uint32_t ticks[] = {0, 0, 0};
    fake_mcp2515_set_tick_sequence(ticks, 3);

    /* Act */
    HAL_StatusTypeDef rc = MCP2515_SendSpeed(1.23f);

    /* Assert */
    TEST_ASSERT_EQUAL(HAL_OK, rc);

    uint8_t addrs[16]; uint8_t vals[16]; int count = 0;
    fake_mcp2515_get_writes(addrs, vals, &count);

    /* Expect there is a write to TXB0DATA with byte 12 */
    int found = 0;
    for (int i = 0; i < count; i++) {
        if (addrs[i] == MCP2515_REG_TXB0DATA && vals[i] == 12) found = 1;
    }
    TEST_ASSERT_TRUE_MESSAGE(found, "Expected TXB0DATA write with byte 12");
}

/* test_MCP2515_SendSpeed_ClampLowAndHigh
 * Purpose: Ensure speed conversion clamps to [0,255] when out of range.
 * Setup: Run with negative and very high inputs using controlled reads/ticks.
 * Assert: Negative speed writes 0, large speed writes 255, both return HAL_OK.
 *
 * ====================== Test Traceability ===========================
 * [test->dsn~can-telemetry-tx~1]
 * ==========================================================================
 */
void test_MCP2515_SendSpeed_ClampLowAndHigh(void)
{
    /* Arrange: negative -> byte 0 */
    const uint8_t reads1[] = {0x00, 0x08,0x40,0x01, 0x00,0x00, 0x00,0x00,0x00};
    fake_mcp2515_set_read_sequence(reads1, sizeof(reads1));
    const uint32_t ticks1[] = {0,0};
    fake_mcp2515_set_tick_sequence(ticks1, 2);

    HAL_StatusTypeDef rc1 = MCP2515_SendSpeed(-1.0f);
    TEST_ASSERT_EQUAL(HAL_OK, rc1);

    uint8_t addrs[16]; uint8_t vals[16]; int count = 0;
    fake_mcp2515_get_writes(addrs, vals, &count);
    int found0 = 0;
    for (int i = 0; i < count; i++) if (addrs[i] == MCP2515_REG_TXB0DATA && vals[i] == 0) found0 = 1;
    TEST_ASSERT_TRUE(found0);

    /* Arrange: very high -> clamp to 255 */
    fake_mcp2515_reset_state();
    const uint8_t reads2[] = {0x00, 0x08,0x40,0x01, 0x00,0x00, 0x00,0x00,0x00};
    fake_mcp2515_set_read_sequence(reads2, sizeof(reads2));
    const uint32_t ticks2[] = {0,0};
    fake_mcp2515_set_tick_sequence(ticks2, 2);

    HAL_StatusTypeDef rc2 = MCP2515_SendSpeed(100.0f);
    TEST_ASSERT_EQUAL(HAL_OK, rc2);

    fake_mcp2515_get_writes(addrs, vals, &count);
    int found255 = 0;
    for (int i = 0; i < count; i++) if (addrs[i] == MCP2515_REG_TXB0DATA && vals[i] == 255) found255 = 1;
    TEST_ASSERT_TRUE(found255);
}

/* test_MCP2515_SendSpeed_TxBufferBusy_ReturnsBusy
 * Purpose: Verify that SendSpeed detects a busy TX buffer and returns HAL_BUSY.
 * Setup: Simulate MCP2515 TXB0CTRL register with TXREQ bit set on first read.
 * Assert: Function returns HAL_BUSY and abort behavior is exercised.
 *
 * ====================== Test Traceability ===========================
 * [test->dsn~can-telemetry-tx~1]
 * ==========================================================================
 */
void test_MCP2515_SendSpeed_TxBufferBusy_ReturnsBusy(void)
{
    /* initial TXB0CTRL returns TXREQ set */
    const uint8_t reads[] = {0x08};
    fake_mcp2515_set_read_sequence(reads, 1);

    HAL_StatusTypeDef rc = MCP2515_SendSpeed(1.0f);
    TEST_ASSERT_EQUAL(HAL_BUSY, rc);
}

/* test_MCP2515_SendSpeed_Timeout_CleansFlags
 * Purpose: Ensure timeout path clears error/interrupt flags and aborts stuck TX.
 * Setup: Simulate TX that never completes (advance ticks past timeout) and set EFLG/CANINTF/TEC/REC values.
 * Assert: Function returns HAL_TIMEOUT and writes were made clearing EFLG and CANINTF.
 *
 * ====================== Test Traceability ===========================
 * [test->dsn~can-error-handling~1]
 * ==========================================================================
 */
void test_MCP2515_SendSpeed_Timeout_CleansFlags(void)
{
    /* Arrange: All reads show TXREQ pending and tick goes past timeout */
    const uint8_t reads[] = {
        0x00, /* initial tx ctrl OK */
        0x08, /* sidh */ 0x08, /* sidl */ 0x40, /* dlc */ 0x01,
        0x00, 0x00,
        /* now the poll will not complete; we simulate timeout by advancing tick */
        /* For timeout handling: EFLG, CANINTF, TEC, REC */ 0xAA, 0xBB, 0x05, 0x03
    };
    fake_mcp2515_set_read_sequence(reads, sizeof(reads));
    const uint32_t ticks[] = {0, 200}; /* start_tick=0, then condition fails -> timeout */
    fake_mcp2515_set_tick_sequence(ticks, 2);

    HAL_StatusTypeDef rc = MCP2515_SendSpeed(1.0f);
    TEST_ASSERT_EQUAL(HAL_TIMEOUT, rc);

    /* Ensure that writes were made to clear EFLG and CANINTF */
    uint8_t addrs[16]; uint8_t vals[16]; int count = 0;
    fake_mcp2515_get_writes(addrs, vals, &count);
    int cleared_eflg = 0, cleared_canintf = 0;
    for (int i = 0; i < count; i++) {
        if (addrs[i] == MCP2515_REG_EFLG && vals[i] == 0x00) cleared_eflg = 1;
        if (addrs[i] == MCP2515_REG_CANINTF && vals[i] == 0x00) cleared_canintf = 1;
    }
    TEST_ASSERT_TRUE(cleared_eflg);
    TEST_ASSERT_TRUE(cleared_canintf);
}

/* test_MCP2515_SendBattery_ValidPercentage_Success
 * Purpose: Verify battery percentage is written to TX payload correctly.
 * Setup: Provide expected register reads and ticks.
 * Assert: Function returns HAL_OK and TXB0DATA contains the percentage (90).
 *
 * ====================== Test Traceability ===========================
 * [test->dsn~can-telemetry-tx~1]
 * ==========================================================================
 */
void test_MCP2515_SendBattery_ValidPercentage_Success(void)
{
    const uint8_t reads[] = {0x00, 0x4d>>3, (uint8_t)(0x4d<<5), 0x01, 0x00, 0x00, 0x00, 0x00, 0x00};
    fake_mcp2515_set_read_sequence(reads, sizeof(reads));
    const uint32_t ticks[] = {0, 0};
    fake_mcp2515_set_tick_sequence(ticks, 2);

    HAL_StatusTypeDef rc = MCP2515_SendBattery(90);
    TEST_ASSERT_EQUAL(HAL_OK, rc);

    uint8_t addrs[16]; uint8_t vals[16]; int count = 0;
    fake_mcp2515_get_writes(addrs, vals, &count);
    int found = 0;
    for (int i = 0; i < count; i++) if (addrs[i] == MCP2515_REG_TXB0DATA && vals[i] == 90) found = 1;
    TEST_ASSERT_TRUE(found);
}
