## Cluster
The Cluster Sub-Module is responsible for presenting real-time vehicle and system information, including speed, battery level, time, date, and weather.
It collects data from sensors via SystemInfo and InfoProvider, which internally communicate over the CAN bus with STM32 and other microcontrollers.
The Cluster module exposes this information to the QML interface for visual representation.

---

### Module Type:
- [ ] Module
- [x] Sub-Module

### Interacts with:
- CAN bus (via CanManager inside SystemInfo): receives messages from STM32 and other microcontrollers.

---

## Specification Items (Requirements)

```
`dsn~design-requirement-cluster-speed~1`

The Cluster Sub-Module shall display the current vehicle speed in real time, refreshing at least 10 times per second.
It shall obtain this data from SystemInfo, which receives it from STM32 or other microcontrollers via CAN bus.

Needs: impl, test

Covers:
- `arch~architecture-requirement-cluster-speed~1`

Status: draft
```

```
`dsn~design-requirement-cluster-battery~1`

The Cluster Sub-Module shall display the battery state-of-charge in real time, updating whenever new data arrives.
It shall obtain this data from SystemInfo, which receives it from STM32 or other microcontrollers via CAN bus.

Needs: impl, test

Covers:
- `arch~display-vehicle-status~1`

Status: draft
```

```
`dsn~design-requirement-cluster-weather~1`

The Cluster Sub-Module shall display the current weather information obtained from GeneralInfo.
Updates shall occur at least every minute.

Needs: impl, test

Covers:
- `arch~weather-display~1`

Status: draft
```

```
`dsn~design-requirement-cluster-datetime~1`

The Cluster Sub-Module shall display the current date and time information obtained from GeneralInfo.
Updates shall occur at least every second.

Needs: impl, test

Covers:
- `arch~time-display~1`

Status: draft
```
---

## Further documentation
- Available at: [Cluster doc](../../../apps/Cluster/README.md)
