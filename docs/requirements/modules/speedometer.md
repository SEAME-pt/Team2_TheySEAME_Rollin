# Speedometer
This sub-module is part of the Sensors module and is responsible for measuring the RPM provided by the tachometer sensor in terms of voltage highs and lows, convert it to a speed unit, and send to the Comms module.

---
### Module Type:
- [ ] Module
- [x] Sub-Module

### Interacts with:
What other modules interact with this module?
- Comms/CAN

## Specification Items (Requirements)

```
`dsn~rpm-sensing~1`

The Sensing module shall use the STM32 TIM peripheral in Input Capture mode to measure the period or frequency of the rising/falling edge signals, with a prescaler and clock source appropriate for an RPM between 1 and 857 rev/s, or equivalently, periods between 3 milliseconds and 3 seconds.

Rationale: If a timer is for example 16-bit, it can register up to 65,535 clock ticks. Therefore, the pre-scaler can be configured to increase the time per tick, if the maximum period overflows the register maximum ticks. Furthermore, the timer tick period shall have enough resolution for the minimum period. 

Needs: impl, test

Covers:
- `arch~report-speed-battery~1`
- `feat~maximum-speed~1`

Status: draft
```

```
`dsn~rpm-latency~1`

The speed shall be measured and transmitted to the Comms Module at a frequency of at least 2Hz.

Needs: impl, test

Covers:
- `arch~report-speed-battery~1`

Status: draft
```

```
`dsn~rpm-sensing~1`

The Sensing module shall calculate the vehicle speed in m/s from the RPM measured from the tachometer signal period/frequency and expose the value to the Communications layer via a shared interface (e.g., global variable, struct, or message queue).

Needs: impl, test

Covers:
- `arch~speed-unit~1`

Status: draft
```

```
`dsn~speed-counter-overflow~1`

The Sensing module shall implement overflow handling for the TIM counter to ensure correct RPM calculation even if the timer rolls over between captures.

Needs: impl, test

Covers:
- arch~rpm-battery-sensing~1

Status: draft
```

```
`dsn~rpm-accuracy~1`

RPM accuracy shall be within ±1 RPM for signals in the range 1–857 RPM

Needs: impl, test

Covers:
- arch~rpm-battery-sensing~1

Status: draft
```


## Further documentation:
Available at: []()
