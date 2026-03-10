# Feature/Client Requirements

```
`feat~navigate-track-autonomously~1`

Status: approved

The vehicle shall autonomously follow a designated track without human intervention.

Needs: dsn
```

```
`feat~detect-traffic-signs~1`

Status: approved

The vehicle shall be able to detect relevant traffic control elements (e.g., stop signs, traffic lights, pedestrian crossings).

Needs: dsn
```

```
`feat~stop-before-obstacles~1`

Status: approved

The vehicle shall autonomously take decisions to stop or avoid unsafe interaction with obstacles or pedestrians.

Needs: dsn
```

```
`feat~report-speed-battery~1`

Status: approved

The vehicle shall determine and report it's ground speed and remaining battery percentage.

Needs: dsn
```

```
`feat~software-updates~1`

Status: approved

The vehicle software shall be updateable over-the-air, while implementing safety strategies to ensure the right permissions for such updates.

Needs: dsn
```

```
`feat~perception-using-camera~1`

Status: approved

The vehicle shall use a vision based perception system (camera) as a primary means to detect obstacles, traffic signs, and road contours.

Needs: dsn
```

```
`feat~backup-perception~1`

Status: approved

The system shall have a backup perception system independent from the primary perception system, that overrides it if it predicts a collision course.

Needs: dsn
```

```
`feat~manual-auto-mode~1`

Status: approved

The vehicle shall have four operation modes: TEST, for RPI5 pre-defined control sequence, MANUAL, for remote control, and AUTO, for AI-based perception and decision-making system, and FAILSAFE, for an immediate emergency brake.

Needs: dsn
```

```
`feat~show-signs-obstacles~1`

Status: approved

The vehicle shall display informative traffic signs and road elements on its instrument cluster.

Needs: dsn
```

```
`feat~friendly-gui~1`

Status: approved

The vehicle shall have a friendly user-interface, automatically displaying it when the car turns on.

Needs: dsn
```

```
`feat~manufacturer-logo-display~1`

Status: approved

On startup, the vehicle shall display the Manufacturer's logo on the Instrument Cluster.

Needs: dsn
```

```
`feat~recover-from-non-critical-errors~1`

Status: approved

The system shall recover gracefully from non-critical errors without human intervention.

Needs: dsn
```

```
`feat~store-conf-data~1`

Status: approved

The software shall store configuration data (e.g., speed limit, calibration data, ...)

Needs: dsn
```

```
`feat~record-debugging-info~1`

Status: approved

The system shall record  operation logs for debugging.

Needs: dsn
```

```
`feat~failsafe-if-no-heartbeat~1`

Status: approved

The Control Unit of the car shall enter FAILSAFE mode if it stops receiving periodic information (heartbeat) from the Raspberry Pi.

Needs: dsn
```

```
`feat~remote-control~1`

Status: approved

The system shall be controllable via remote control.

Needs: dsn
```


<!-- ```
`feat~rtos-control~1`

Status: approved

The control system shall be deterministic, applying RTOS behaviour.

Needs: dsn
``` -->

```
`feat~weather-time-info~1`

Status: approved

The system shall display time and weather information on the UI.

Needs: dsn
```

```
`feat~control-systems~1`

Status: approved

The system shall use a Raspberry Pi 5 (RPi5) and a B-U5851-IOT2A STM32 development board (STM32) connected via a CAN bus. The RPi5 shall process camera input and transmit motor and actuator control commands to the STM32 over CAN. The STM32 shall control the vehicle’s motors and actuators and shall transmit sensor data to the RPi5 over CAN.

Needs: dsn
```

```
`feat~gear-selection-modes~1`

Status: draft

The system shall support four gear modes: Park (P), Neutral (N), Drive (D), and Reverse (R), selectable by the driver.
Needs: impl, itest

Needs: dsn
```
# Draft
## Module Control

---

```
`feat~cruise-control~1`

Status: proposed

The system shall provide a cruise control feature that automatically maintains a user-defined target vehicle speed without requiring continuous driver throttle input.

Needs: dsn, impl, itest
```

---

## Notes: 
Safety Critical messages shall be prioritized over other types of CAN messages.
