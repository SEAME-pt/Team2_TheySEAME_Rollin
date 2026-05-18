# Software Architecture Revision
### Implement Mode Management strategy
Goals:
- Handle switch between Manual and Auto modes
- Handle the Boot, Startup and Fail-safe sequences
- Handle Error handling

Solutions:
- State machine with deterministic authority over frames sent via CAN

## Reccomended High Level architecture:
```
             +------------------+
             |    Supervisor    |
             | mode/fault state |
             +---------+--------+
                       |
                       v
             +------------------+
RC --------> |                  |
AI --------> | Motion Arbiter   | ---> CAN ---> STM32
Failsafe --> |                  |
             +------------------+
```

The idea is that each module publishes it's ```intent``` to the Motion Arbiter. Then the superviser decides who has the authority by determining if the car is allowed to mode, if a failure requires fallback, or whether Auto mode can engage.

## Supervisor (mode/fault switch)
The Mode Supervisor is responsible for:
- Mode transitions
- Startup sequencing
- Watchdogs
- Fault aggregation
- Safe fallback
- Arming/disarming motion
- Selecting command source

Finite State Machine:
``` c++
enum class VehicleState
{
    BOOT, #System Powering up
    INIT, # Waiting subsystems healthy
    STANDBY, # Ready but no actuation
    MANUAL, # Remote control is active
    AUTONOMOUS, # LKA active
    SAFE_STOP, # Failsafe after issue - STOP
    FAULT # Critical Failure
};
```

Then we define mode transitions. For example: 
`MANUAL -> AUTO if STM32 Alive, CAN Healthy, HAILO Healthy, No Sync Problem, ...`

Every subsystem periodically publishes heartbeat to the supervisor to allow for fault switch: 

```
struct HealthStatus
{
    bool alive;
    bool healthy;
    uint64_t timestamp_ms;
    uint32_t fault_flags;
};
```

## Motion Arbitrer
Based on Mode selected by Supervisor, decides which module has the authority.

```
switch(supervisor_mode)
{
    case MANUAL:
        output = rc;
        break;

    case AUTONOMOUS:
        output = lka;
        break;

    case SAFE_STOP:
        output = brake;
        break;
}
```

## Implementation Strategy
### Threads vs Processes
Use processes for major components:
- AI
- Supervisor
- CAN
- RC Input
- Control Modules (LKA, MPC, Manual...)

Why? Processes isolate crashes. If AI fails, the Manual Control Module is still working.

Use threads within processes. For example:
```
AI Process
 ├── Camera thread
 ├── Inference thread
 ├── Postprocess thread
 └── IPC publisher thread

 CAN Process
 ├── RX thread
 ├── TX thread
 └── heartbeat/watchdog thread
```
 ### Reccomended runtime frequencies
|  Module	  | Frequency |
| --------- | --------- |
| Supervisor |	10–20 Hz  |
|Motion Arbiter |	50–100 Hz |
|CAN TX	| 100 Hz        |
|RC Input	| 50–100 Hz |
|LKA	|20–50 Hz|

 ### Inter-process communication
 Either ROS2 or ZeroMQ. ZeroMQ is simpler and allows pub/sub patterns.

