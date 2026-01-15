# Communication

This module is responsible for CAN communication using the MCP2515 controller (software SPI), transmission of telemetry (speed, battery), reception and parsing of CAN messages, and handling of CAN-level errors and diagnostics.

### Interacts with:
- Sensors (Speedometer)
- Control (Actuation / Steering)
- Raspberry Pi 5 (telemetry/telecommands via CAN)
- Utilities (Logging / UART for diagnostics)

## Specification Items (Requirements)

```
`dsn~can-init~1`

The Communication module shall initialize the MCP2515 CAN controller to the configured bus speed and set NORMAL operating mode during subsystem startup. Initialization shall be verifiable via the CANSTAT register read and return `HAL_OK` on success.

Needs: impl, test

Covers:
- `arch~stm-rpi-can-telemetry~1`

Status: approved
```

```
`dsn~can-telemetry-tx~1`

The module shall transmit telemetry messages for vehicle speed and battery using the following formats:
- Speed: CAN ID `0x042` (11-bit), payload 1 byte, value in decimeters/second (0..255)
- Battery: CAN ID `0x04D` (11-bit), payload 1 byte, percentage (0..100)

The implementation shall convert and clamp values as required and confirm successful TX via MCP2515 status polling.

Needs: impl, test

Covers:
- `arch~report-speed-battery~1`
- `arch~stm-rpi-can-telemetry~1`

Status: approved
```

```
`dsn~can-error-handling~1`

The module shall detect transmission timeouts and error conditions, clear `EFLG` and `CANINTF` as appropriate, abort stuck transmissions (clear TXREQ) and report `HAL_TIMEOUT` when TX cannot complete within the configured timeout window.

Needs: impl, test

Covers:
- `feat~failsafe-case-miscommunication~1`

Status: approved
```

```
`dsn~can-telemetry-rx~1`

The module shall parse incoming CAN messages for supported IDs (e.g., speed, battery) and update the in-memory vehicle data structure so other modules can consume updated telemetry.

Needs: impl, test

Covers:
- `arch~stm-rpi-can-telemetry~1`

Status: draft
```

```
`dsn~can-logging-policy~1`

Debug and diagnostic UART prints in the Communications module shall be disabled by default for production/develop builds and only enabled with explicit build macros (e.g., `MCP2515_DEBUG`, `COMM_DEBUG`).

Needs: impl, test

Covers:
- `arch~development-guidelines~1`

Status: approved
```

```
`dsn~can-rate-limit~1`

The Communication module shall rate-limit telemetry transmissions (e.g., speed updates no faster than once per 200 ms) to avoid bus congestion; rate-limiting shall be testable via tick/time fakes.

Needs: impl, test

Covers:
- `arch~report-speed-battery~1`

Status: draft
```

## Mapping to existing tests
- `Core/Tests/test/test_mcp2515.c` — covers [test->dsn~can-telemetry-tx~1] and [test->dsn~can-error-handling~1].

## Further documentation:
Available at: [/apps/stm32-firmware/Core/Src/Communication/CAN_README.md](../../apps/stm32-firmware/Core/Src/Communication/CAN_README.md)

---
Edited to follow repository module requirement format and map to new DSN IDs.
