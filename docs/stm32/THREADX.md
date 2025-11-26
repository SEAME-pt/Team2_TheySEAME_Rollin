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

## References
- [Azure RTOS ThreadX Documentation](https://learn.microsoft.com/en-us/azure/rtos/threadx/)
- [ThreadX API Reference](https://learn.microsoft.com/en-us/azure/rtos/threadx/threadx-api-reference/)

---
This document provides a high-level overview. For detailed usage, refer to the official documentation and your platform's integration notes.
