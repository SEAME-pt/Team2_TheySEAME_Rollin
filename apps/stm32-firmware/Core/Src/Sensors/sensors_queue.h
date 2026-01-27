#ifndef SENSORS_QUEUE_H
#define SENSORS_QUEUE_H

#include "tx_api.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SENSOR_ID_SPEED = 1,
    SENSOR_ID_BATTERY = 2,
    SENSOR_ID_INA219 = 3,
} SensorId_t;

typedef struct {
    SensorId_t sensor_id;
    float value;      // simple payload: single float value
    uint32_t ts;      // timestamp (HAL_GetTick)
} SensorSample_t;

#define SENSORS_QUEUE_SIZE 64

UINT SensorsQueue_Init(void);
int SensorsQueue_TrySend(const SensorSample_t *samp);
int SensorsQueue_Send(const SensorSample_t *samp, ULONG wait);
UINT SensorsQueue_Receive(SensorSample_t *samp, ULONG wait);
uint32_t SensorsQueue_GetDrops(void);

#ifdef __cplusplus
}
#endif

#endif /* SENSORS_QUEUE_H */
