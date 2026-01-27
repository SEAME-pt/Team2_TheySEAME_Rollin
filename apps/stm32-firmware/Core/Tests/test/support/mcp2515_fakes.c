#include "mcp2515.h"
#include "stm32u5xx_hal.h"
#include "main.h" /* get UART_HandleTypeDef stub and pin macros */
#include <string.h>

static uint8_t read_seq[128];
static int read_seq_len = 0;
static int read_seq_idx = 0;

static uint8_t write_addrs[128];
static uint8_t write_vals[128];
static int write_count = 0;

static uint32_t tick_seq[64];
static int tick_seq_len = 0;
static int tick_seq_idx = 0;

void fake_mcp2515_reset_state(void) {
    read_seq_len = read_seq_idx = 0;
    write_count = 0;
    tick_seq_len = tick_seq_idx = 0;
}

void fake_mcp2515_set_read_sequence(const uint8_t *seq, int len) {
    if (len > (int)sizeof(read_seq)) len = sizeof(read_seq);
    memcpy(read_seq, seq, len);
    read_seq_len = len;
    read_seq_idx = 0;
}

void fake_mcp2515_set_tick_sequence(const uint32_t *seq, int len) {
    if (len > (int)sizeof(tick_seq)/sizeof(uint32_t)) len = sizeof(tick_seq)/sizeof(uint32_t);
    memcpy(tick_seq, seq, len * sizeof(uint32_t));
    tick_seq_len = len;
    tick_seq_idx = 0;
}

void fake_mcp2515_get_writes(uint8_t *addrs, uint8_t *vals, int *count) {
    if (addrs && vals) {
        for (int i = 0; i < write_count; i++) { addrs[i] = write_addrs[i]; vals[i] = write_vals[i]; }
    }
    if (count) *count = write_count;
}

/* Fake implementations to override hardware during unit tests */
uint8_t MCP2515_ReadRegister(uint8_t address) {
    (void)address;
    if (read_seq_idx < read_seq_len) return read_seq[read_seq_idx++];
    return 0;
}

void MCP2515_WriteRegister(uint8_t address, uint8_t value) {
    if (write_count < (int)sizeof(write_addrs)) {
        write_addrs[write_count] = address;
        write_vals[write_count] = value;
        write_count++;
    }
}

void MCP2515_BitModify(uint8_t address, uint8_t mask, uint8_t value) {
    /* Record as a write to the address (value masked for record) */
    uint8_t masked = value & mask;
    if (write_count < (int)sizeof(write_addrs)) {
        write_addrs[write_count] = address;
        write_vals[write_count] = masked;
        write_count++;
    }
}

HAL_StatusTypeDef HAL_UART_Transmit(void *huart, uint8_t *pData, uint16_t Size, uint32_t Timeout) {
    (void)huart; (void)pData; (void)Size; (void)Timeout; return HAL_OK; /* ignore output */
}

uint32_t HAL_GetTick(void) {
    if (tick_seq_idx < tick_seq_len) return tick_seq[tick_seq_idx++];
    return 0;
}

void HAL_Delay(uint32_t ms) {
    (void)ms; /* no-op */
}

/* Provide simple GPIO stubs used by SUT */
void HAL_GPIO_WritePin(void *GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState) {
    (void)GPIOx; (void)GPIO_Pin; (void)PinState; /* no-op */
}

GPIO_PinState HAL_GPIO_ReadPin(void *GPIOx, uint16_t GPIO_Pin) {
    (void)GPIOx; (void)GPIO_Pin; return GPIO_PIN_RESET;
}

/* Provide huart1 instance expected by SUT */
UART_HandleTypeDef huart1;