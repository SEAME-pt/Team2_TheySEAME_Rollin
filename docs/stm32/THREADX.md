# ThreadX RTOS Documentation

## What is ThreadX?

ThreadX is a real-time operating system (RTOS) designed for embedded systems. It provides a lightweight, efficient, and deterministic environment for running multiple threads (tasks) concurrently on microcontrollers and microprocessors. ThreadX is widely used in applications requiring real-time performance, such as IoT devices, medical equipment, consumer electronics, and industrial controllers.

Key features of ThreadX include:
- Preemptive, priority-based scheduling
- Fast context switching
- Deterministic interrupt handling
- Rich set of synchronization primitives (mutexes, semaphores, event flags, queues)
- Small memory footprint
- Easy integration with embedded toolchains

## Main Concepts

- **Thread**: The basic unit of execution. Each thread has its own stack and priority.
- **Scheduler**: Manages which thread runs at any given time based on priority.
- **Synchronization Objects**: Mechanisms like mutexes, semaphores, and event flags to coordinate between threads.
- **Timers**: Allow threads to sleep or perform actions after a delay.

## How to Use ThreadX

### 1. Initialization
Before using ThreadX, you must initialize the RTOS kernel, typically in your `main()` function:

```c
#include "tx_api.h"

int main(void) {
    // Hardware initialization here
    tx_kernel_enter(); // Start ThreadX kernel
}
```

### 2. Creating Threads
Threads are created using `tx_thread_create`. Each thread needs a control block, stack, entry function, and priority.

```c
TX_THREAD my_thread;
ULONG my_thread_stack[1024];

void my_thread_entry(ULONG thread_input) {
    while (1) {
        // Thread code here
    }
}

void tx_application_define(void *first_unused_memory) {
    tx_thread_create(&my_thread, "My Thread", my_thread_entry, 0,
                     my_thread_stack, sizeof(my_thread_stack),
                     1, 1, TX_NO_TIME_SLICE, TX_AUTO_START);
}
```

### 3. Synchronization Primitives

#### Mutexes
```c
TX_MUTEX my_mutex;
tx_mutex_create(&my_mutex, "My Mutex", TX_NO_INHERIT);
tx_mutex_get(&my_mutex, TX_WAIT_FOREVER);
// Critical section
tx_mutex_put(&my_mutex);
```

#### Semaphores
```c
TX_SEMAPHORE my_semaphore;
tx_semaphore_create(&my_semaphore, "My Semaphore", 1);
tx_semaphore_get(&my_semaphore, TX_WAIT_FOREVER);
// Protected section
tx_semaphore_put(&my_semaphore);
```

#### Event Flags
```c
TX_EVENT_FLAGS_GROUP my_events;
tx_event_flags_create(&my_events, "My Events");
tx_event_flags_set(&my_events, 0x01, TX_OR);
tx_event_flags_get(&my_events, 0x01, TX_OR_CLEAR, &actual_flags, TX_WAIT_FOREVER);
```

### 4. Message Queues
```c
TX_QUEUE my_queue;
ULONG queue_buffer[16];
tx_queue_create(&my_queue, "My Queue", TX_1_ULONG, queue_buffer, sizeof(queue_buffer));
tx_queue_send(&my_queue, &message, TX_WAIT_FOREVER);
tx_queue_receive(&my_queue, &received, TX_WAIT_FOREVER);
```

### 5. Timers
```c
TX_TIMER my_timer;
void timer_callback(ULONG input) {
    // Timer action
}
tx_timer_create(&my_timer, "My Timer", timer_callback, 0, 100, 100, TX_AUTO_ACTIVATE);
```

## Useful ThreadX Functions

- `tx_thread_create` — Create a new thread
- `tx_thread_sleep` — Put a thread to sleep for a number of ticks
- `tx_mutex_create`, `tx_mutex_get`, `tx_mutex_put` — Mutex operations
- `tx_semaphore_create`, `tx_semaphore_get`, `tx_semaphore_put` — Semaphore operations
- `tx_event_flags_create`, `tx_event_flags_set`, `tx_event_flags_get` — Event flag operations
- `tx_queue_create`, `tx_queue_send`, `tx_queue_receive` — Queue operations
- `tx_timer_create` — Create a timer

## Project: Thread & Queue Structure (what we implemented) ✅

This section documents the concrete ThreadX design and message-queue patterns added in the current work (feature/message-queues). It explains thread roles, priorities, queue usage, and diagnostics so engineers can reason about realtime behavior and tuning.

### Thread Roles & Priorities 🔧
- **Control thread** (`Control_Thread_Entry`) — priority **8** (high). Consumes commands from the Control queue and commands actuators safely. Implements safety timeout when commands are missing.
- **Communication thread** (`Communication_Thread_Entry`) — priority **10**. Handles MCP2515 CAN, decodes commands, and enqueues commands to the Control queue. Also sends periodic telemetry/status messages and a heartbeat.
- **Sensors processor thread** (`SensorsProcessor_Thread_Entry`) — priority **12**. Drains the Sensors queue, applies filtering/aggregation, and updates the global `g_vehicle_data` under `g_vehicle_data_mutex`.
- **Speed (sensor sampling) thread** (`Speed_Thread_Entry`) — priority **14** (lower). Produces `SensorSample_t` messages (non-blocking enqueue) and detects no-pulse conditions.
- **Battery / Test threads** — priority **10** (typical background tasks).

> Note: Lower numeric value = higher priority in ThreadX. Priorities were chosen to keep control responsive while letting sensor processing run at a lower priority.

### Message Queues & IPC (design) 💡
We implemented two primary queues:

- **Control queue** (`ControlQueue_*` API, defined in `Core/Src/Control/control_queue.h` / `.c`)
  - Purpose: deliver `VehicleCommand_t` from Communication → Control.
  - Size: **32** (temporarily increased during debugging; tunable).
  - API highlights:
    - `ControlQueue_TrySend(cmd)` — **non-blocking**, ISR/high-priority friendly, returns 1 on success, 0 on drop.
    - `ControlQueue_Send(cmd, wait)` — blocking send (task context) with wait.
    - `ControlQueue_Receive(cmd, wait)` — blocking receive for consumer.
    - `ControlQueue_GetDrops()` and `ControlQueue_GetOccupancy(&count)` — diagnostics.
  - Policy: producers use non-blocking sends; consumer blocks waiting for commands. Drop counter increments on enqueue failure.

- **Sensors queue** (`SensorsQueue_*` API, defined in `Core/Src/Sensors/sensors_queue.h` / `.c`)
  - Purpose: buffer `SensorSample_t` (speed, battery, INA219, etc.) from producers (e.g., speed sampler) to the Sensors processor.
  - Size: **64** samples (configurable).
  - API and behavior mirror Control queue (`TrySend`, `Send`, `Receive`, `GetDrops`, `GetOccupancy`).

### Safety & Heartbeat ⚠️
- A **heartbeat** mechanism in the Communication thread re-sends the last valid command every **1s** if no new commands are received. The heartbeat uses the same queue API to keep Control informed and avoid spurious fail-safes during debugging.
- The Control thread enforces a safety timeout (1s default) and issues `Control_StopMotors()` when commands are missing. For short-lived debugging we added a compile-time guard `DISABLE_CONTROL_SAFETY` and a **debug convenience** that defines it automatically for debug/test builds so safety stops do not interfere with investigation.

⚠️ **Important:** Do NOT keep `DISABLE_CONTROL_SAFETY` enabled in production builds.

### Diagnostics & Tests 🔎
- **Drop counters** are exposed via `*_GetDrops()` and are logged periodically by the Communication thread: e.g. `[COMM] Control drops=X occ=Y` every **5s**. This helps identify overflow under load.
- We added occupancy reporting (`*_GetOccupancy(&count)`) to observe current queue fill levels.
- **Unit tests (Ceedling)** were added for queue behavior, including tests validating drop counter increments on failed enqueues (`Core/Tests/test/test_sensors_queue.c` and `test_control_queue.c`). Run tests in `apps/stm32-firmware/Core/Tests` with `ceedling test:all`.

### Rationale & Tuning Tips 💡
- Producers use **non-blocking** `TrySend` to avoid blocking ISRs or high-priority threads — this prevents priority inversion and reduces worst-case latencies.
- Consumer threads block on `Receive()` so they can sleep efficiently and process batches.
- If drops are frequent:
  - Increase queue sizes conservatively, or
  - Re-examine producer rate (debounce or sample less frequently), or
  - Raise consumer priority or reduce work done in consumer (move heavy work to lower-priority task).

### Short-term Changes made during debugging 🔧
- Increased `CONTROL_QUEUE_SIZE` from 8 → **32** (temporary) to reduce immediate drops.
- Added **rate-limited** safety and status messages to make logs readable while investigating.
- Added occupancy diagnostics and queue drop logging.
- Added heartbeat with **default safe command** to keep the system in a known safe state during debugging.

---
## References
- [Azure RTOS ThreadX Documentation](https://learn.microsoft.com/en-us/azure/rtos/threadx/)
- [ThreadX API Reference](https://learn.microsoft.com/en-us/azure/rtos/threadx/threadx-api-reference/)

---
This document provides a high-level overview. For detailed usage, refer to the official documentation and your platform's integration notes.
