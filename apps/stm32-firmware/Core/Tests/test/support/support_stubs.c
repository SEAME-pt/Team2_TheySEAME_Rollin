#include "sensors.h"
#include "../../Core/Inc/sensors_queue.h"
#include <stdio.h>

/* Simple no-op debug function used by SUT in tests */
void Debug_Print(const char *s) {
    (void)s; /* Ignore output in unit tests */
}

/* IRQ stubs */
void __disable_irq(void) { }
void __enable_irq(void) { }

/* Simple ThreadX stubs used in SUT
 * These are minimal and return success so tests can proceed without real RTOS
 */
#ifndef TX_SUCCESS
#define TX_SUCCESS 0
#endif
#ifndef TX_WAIT_FOREVER
#define TX_WAIT_FOREVER ((ULONG)0xffffffffUL)
#endif

/* Define global variables referenced by SUT */
VehicleData_t g_vehicle_data = {0};
VehicleCommand_t g_vehicle_command = {0};

/* tx_mutex types are provided by mocked tx_api, define simple instances */
TX_MUTEX g_vehicle_data_mutex;
TX_MUTEX g_vehicle_command_mutex;

/* Sensors queue stubs for unit tests */
UINT SensorsQueue_Init(void) { return TX_SUCCESS; }
int SensorsQueue_TrySend(const SensorSample_t *samp) { (void)samp; return 1; }
int SensorsQueue_Send(const SensorSample_t *samp, ULONG wait) { (void)samp; (void)wait; return 1; }
UINT SensorsQueue_Receive(SensorSample_t *samp, ULONG wait) { (void)samp; (void)wait; return TX_SUCCESS; }
uint32_t SensorsQueue_GetDrops(void) { return 0; }

/* Mutex API is mocked via CMock (mock_tx_api); do not provide real definitions here to avoid symbol conflicts */

