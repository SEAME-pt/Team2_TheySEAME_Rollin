# Comms CAN RPI5

This module gives and supports CAN communication to the RPI5

---
### Module Type:
- [ ] Module
- [x] Sub-Module

### Interacts with:
What other modules interact with this module?
- Interaction 1
- Interaction 2

## Specification Items (Requirements)

```
`dsn~comms-can-rpi-receiveMsg~1`

The Comms CAN RPI5 shall receive CAN frames from the CAN bus in a non-blocking way

Needs: impl, test

Covers:
- `arch~stm-rpi-can-control~1`

Status: draft
```

```
`dsn~comms-can-rpi-sendMsg~1`

The Comms CAN RPI5 shall send a CAN frame whenever any other module wants to send a message via CAN

Needs: impl, test

Covers:
- `arch~stm-rpi-can-control~1`

Status: draft
```

## Further documentation:
Available at: []()
