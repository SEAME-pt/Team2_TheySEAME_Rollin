# Comms CAN RPI5

This module gives and supports CAN communication to the RPI5

---
### Module Type:
- [ ] Module
- [x] Sub-Module

### Interacts with:
What other modules interact with this module?
- Remote Control
- AI Planning

## Specification Items (Requirements)

```
`dsn~comms-can-rpi-frameformat-standard~1`

The Comms CAN RPI5 shall follow the Standard CAN frame format

It needs these fields:
- Arbitration
- Control
- Data (0-8 bytes)
- CRC
- ACK

Needs: impl, test

Covers:
- `arch~stm-rpi-can-control~1`

Status: approved
```

```
`dsn~comms-can-rpi-frameformat-control~1`

The Comms CAN RPI5 shall follow this frame format for any control commands (RPI->STM) communication

Data field:
- 1ºbyte: Mode (AI, Manual, ...)
- 2ºbyte: Throttle percentage (0-100%)
- 3ºbyte: Steering angle (left: -1-0, right: 0-1, neutral: 0)

Needs: impl, test

Covers:
- `arch~stm-rpi-can-control~1`

Status: approved
```

```
`dsn~comms-can-rpi-interface~1`

The Comms CAN RPI5 shall communicate with a bit rate of 500kbps using standard CAN V2.0B

Needs: impl, test

Covers:
- `arch~stm-rpi-can-control~1`

Status: approved
```

```
`dsn~comms-can-rpi-receiveMsg~1`

The Comms CAN RPI5 shall receive CAN frames from the CAN bus in a non-blocking way

Needs: impl, test

Covers:
- `arch~stm-rpi-can-control~1`

Status: approved
```

```
`dsn~comms-can-rpi-sendMsg~1`

The Comms CAN RPI5 shall send a CAN frame whenever any other module wants to send a message via CAN

Needs: impl, test

Covers:
- `arch~stm-rpi-can-control~1`

Status: approved
```

## Further documentation:
Available at: 
- [CAN-Bus](https://en.wikipedia.org/wiki/CAN_bus)
- [CAN-Hardware](../../Hardware/README.md)
- [SocketCAN](https://docs.kernel.org/networking/can.html)
