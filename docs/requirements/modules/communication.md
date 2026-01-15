# Communication Module Requirements

Module: Communication (MCP2515 CAN driver, CAN RX/TX, communication thread)

Overview
--------
This document specifies the functional and non-functional requirements for the Communication module that handles CAN bus interactions (MCP2515 driver), transmission of telemetry (speed, battery), RX message parsing, and error handling.

Requirement identifiers use the DSN convention used across the repository (e.g., `dsn~<short-name>~<revision>`). Each requirement includes acceptance criteria and test traceability pointers.

## Requirements

### dsn~can-init~1 — MCP2515 Initialization
- Description: The system shall initialize the MCP2515 to the configured bus speed and place it in NORMAL mode when the communication subsystem starts.
- Rationale: Ensures the CAN controller is ready for TX/RX operations.
- Acceptance criteria:
  - `MCP2515_Init()` returns `HAL_OK` when initialization succeeds and subsequent reads of CANSTAT show NORMAL mode.
  - Unit test: `test_mcp2515_init_success` (see tests)
- Test Traceability: [test->dsn~can-init~1]

### dsn~can-telemetry-tx~1 — Telemetry TX Message Format and Delivery
- Description: The module shall transmit telemetry messages for speed and battery with the specified CAN IDs and payload formats:
  - Speed: CAN ID `0x042`, 1 byte, decimeters/second (0..255) (dm/s)
  - Battery: CAN ID `0x04D`, 1 byte, percentage (0..100)
- Rationale: Specification required by the cluster and RPi telemetry consumers.
- Acceptance criteria:
  - `MCP2515_SendSpeed()` converts m/s to dm/s correctly, clamps values to [0,255], writes expected TXB data registers and returns `HAL_OK` when transmitted.
  - `MCP2515_SendBattery()` writes the percentage byte and returns `HAL_OK` on success.
- Test Traceability: [test->dsn~can-telemetry-tx~1]

### dsn~can-error-handling~1 — TX Timeout and Error Handling
- Description: The module shall detect TX timeouts, clear error/interrupt flags, and abort stuck transmissions to avoid bus lock conditions.
- Acceptance criteria:
  - On TX timeout, `MCP2515_Send*` returns `HAL_TIMEOUT`, writes `EFLG=0x00` and `CANINTF=0x00`, and clears TXREQ for the stuck buffer.
- Test Traceability: [test->dsn~can-error-handling~1]

### dsn~can-telemetry-rx~1 — RX Message Handling
- Description: The module shall parse incoming CAN messages and update the global vehicle data structure for supported message IDs (speed, battery, others as defined).
- Acceptance criteria:
  - RX messages for known IDs are parsed and reflected in in-memory state.
- Test Traceability: [test->dsn~can-telemetry-rx~1]

### dsn~can-logging-policy~1 — Debug/Diagnostic Prints Policy
- Description: Debug/diagnostic prints in Communication/MCP2515 code shall be disabled in production/develop builds by default and only enabled through explicit build-time macros (e.g., `MCP2515_DEBUG`, `COMM_DEBUG`).
- Acceptance criteria:
  - No runtime UART prints unless debug macros are defined.
- Test Traceability: [test->dsn~can-logging-policy~1]

### dsn~can-rate-limit~1 — Transmission Rate Limiting
- Description: The module shall not transmit speed faster than the configured telemetry interval (e.g., no faster than once per 200 ms by default).
- Acceptance criteria:
  - Calls to send speed are rate-limited (testable via tick/fake sequences).
- Test Traceability: [test->dsn~can-rate-limit~1]

## Mapping to existing tests
- `Core/Tests/test/test_mcp2515.c` covers:
  - [test->dsn~can-telemetry-tx~1] — SendSpeed and SendBattery tests (valid data and clamping)
  - [test->dsn~can-error-handling~1] — Timeout and error-clearing tests

## Notes and next actions
- Add unit tests for `MCP2515_Init()` and RX parsing functions where missing.
- Add test IDs above each relevant unit test and update Doxygen comments in source files to reference the `impl->` traceability IDs.


---
Document created by automated change to address missing module requirements.
