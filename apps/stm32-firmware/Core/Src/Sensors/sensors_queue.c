#include "sensors_queue.h"
#include "tx_api.h"
#include <string.h>

static TX_QUEUE sensors_q;
static UCHAR sensors_q_mem[SENSORS_QUEUE_SIZE * sizeof(SensorSample_t)];
static uint32_t sensors_q_drops = 0;

UINT SensorsQueue_Init(void) {
    memset(&sensors_q, 0, sizeof(sensors_q));
    sensors_q_drops = 0;
    return tx_queue_create(&sensors_q, "SensorsQ", sizeof(SensorSample_t), sensors_q_mem, SENSORS_QUEUE_SIZE);
}

int SensorsQueue_TrySend(const SensorSample_t *samp) {
    UINT status = tx_queue_send(&sensors_q, (void*)samp, TX_NO_WAIT);
    if (status == TX_SUCCESS) return 1;
    sensors_q_drops++;
    return 0;
}

int SensorsQueue_Send(const SensorSample_t *samp, ULONG wait) {
    UINT status = tx_queue_send(&sensors_q, (void*)samp, wait);
    if (status == TX_SUCCESS) return 1;
    sensors_q_drops++;
    return 0;
}

UINT SensorsQueue_Receive(SensorSample_t *samp, ULONG wait) {
    return tx_queue_receive(&sensors_q, (void*)samp, wait);
}

uint32_t SensorsQueue_GetDrops(void) {
    return sensors_q_drops;
}

UINT SensorsQueue_GetOccupancy(UINT *count) {
    if (count == NULL) return 1;
    ULONG message_count = 0;
    UINT status = tx_queue_info_get(&sensors_q, NULL, &message_count, NULL, NULL, NULL, NULL);
    if (status != TX_SUCCESS) return status;
    *count = (UINT)message_count;
    return TX_SUCCESS;
}