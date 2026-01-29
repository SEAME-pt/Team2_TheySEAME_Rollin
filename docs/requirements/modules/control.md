# Control

This module is responsible for processing actuation commands (steering, throttle, brake) and safety logic, including safety stop, heartbeat monitoring, and command validation. It receives commands from the Communication module and interacts with Sensors for feedback.

### Interacts with:
- Communication (receives commands via CAN)
- Sensors (receives feedback)
- Cluster (status reporting)

## Specification Items (Requirements)

```
dsn~control-actuation-commands~1

Status: draft

The Control module shall process actuation commands for steering, throttle, and brake, received from the Communication module, and apply them to the vehicle actuators.

Needs: impl, test

Covers:
- arch~vehicle-actuation~1
```

```
dsn~control-safety-stop~1

Status: draft

The Control module shall implement a safety stop mechanism that halts actuation if a heartbeat is missed or a safety condition is triggered.

Needs: impl, test

Covers:
- arch~safety-stop~1
```

```
dsn~control-heartbeat-monitor~1

Status: draft

The Control module shall monitor heartbeat messages and trigger a safety stop if a heartbeat is not received within the configured interval.

Needs: impl, test

Covers:
- arch~heartbeat-monitoring~1
```

```
dsn~control-queue~1

Status: draft

The Control module shall provide a message queue for incoming `VehicleCommand` messages with non-blocking send available for interrupt contexts and a blocking send for task contexts. The queue shall expose runtime diagnostics for drops and occupancy.

Needs: impl, test

Covers:
- arch~control-queue~1
```
