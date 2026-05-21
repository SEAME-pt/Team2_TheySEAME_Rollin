#include "control_queue.h"
#include "tx_api.h"
#include <string.h>

static TX_QUEUE control_q;
static UCHAR control_q_mem[CONTROL_QUEUE_SIZE * sizeof(VehicleCommand_t)];
static uint32_t control_q_drops = 0;

UINT ControlQueue_Init(void) {
    memset(&control_q, 0, sizeof(control_q));
    control_q_drops = 0;

    UINT msg_size = (sizeof(VehicleCommand_t) + sizeof(ULONG) - 1) / sizeof(ULONG);

    return tx_queue_create(&control_q, "ControlQ", msg_size,
                           control_q_mem, CONTROL_QUEUE_SIZE);
}

int ControlQueue_TrySend(const VehicleCommand_t *cmd) {
    UINT status = tx_queue_send(&control_q, (void*)cmd, TX_NO_WAIT);
    if (status == TX_SUCCESS) return 1;
    control_q_drops++;
    {
        char buf[64];
        snprintf(buf, sizeof(buf), "[CONTROL_Q] Drop occurred, total=%u\r\n", control_q_drops);
        Debug_Print(buf);
    }
    return 0;
}

int ControlQueue_Send(const VehicleCommand_t *cmd, ULONG wait) {
    UINT status = tx_queue_send(&control_q, (void*)cmd, wait);
    if (status == TX_SUCCESS) return 1;
    control_q_drops++;
    {
        char buf[64];
        snprintf(buf, sizeof(buf), "[CONTROL_Q] Drop occurred on Send, total=%u\r\n", control_q_drops);
        Debug_Print(buf);
    }
    return 0;
}

UINT ControlQueue_Receive(VehicleCommand_t *cmd, ULONG wait) {
    /* Optionally could add diagnostics here */
    return tx_queue_receive(&control_q, (void*)cmd, wait);
}

uint32_t ControlQueue_GetDrops(void) {
    return control_q_drops;
}

UINT ControlQueue_GetOccupancy(UINT *count) {
    /* Returns the current number of messages in the queue via tx_queue_info_get */
    if (count == NULL) return 1; /* invalid param */
    ULONG created_count = 0;
    UINT status = tx_queue_info_get(&control_q, NULL, &created_count, NULL, NULL, NULL, NULL);
    if (status != TX_SUCCESS) return status;
    *count = (UINT)created_count;
    return TX_SUCCESS;
}