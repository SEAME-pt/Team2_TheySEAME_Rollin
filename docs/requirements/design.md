# Design Requirements

## Cluster
```
`dsn~display-battery-real-time~1`

Status: draft

The QT App (cluster) shall display the battery state of the vehicle in real time in percentage (%), as measured, and communicated via CAN, from the STM32 Sensor's module, with a maximum refresh rate of, 0.1 Hz.

Rationale: 0.1 Hz equals 1 time every ten seconds. It was considered that the timescale of changes for this value is smaller than other variables, for instance, the speed measurement.

Covers:
- `feat~report-speed-battery~1`

Needs: impl, itest
```

```
`dsn~display-speed-real-time~1`

Status: draft

The QT App (cluster) shall display the vehicle's ground speed in real time in hectometer per hour (hm/h), as measured, and communicated via CAN, from the STM32 Sensor's module, with a maximum refresh rate of, 10 Hz.

Rationale: If the driver accelerates from 0 to 5 hm/s in 1 second, we would see increments of about 0.5 hm/s in speed, which is a reasonable refresh rate, that also prevents blocking or taking priority over other essential CAN frames.

Covers:
- `feat~report-speed-battery~1`

Needs: impl, itest
```

```
`dsn~gui-readable-font-sizing~1`

Status: draft

The Cluster shall display crucial information with a font and size...

Covers:
- `feat~friendly-gui~1`

Needs: impl, itest
```

```
`dsn~manufacturer-logo-display~1`

Status: draft

The manufacturer (They SEA:ME Rolling Team) shall be displayed on startup for no more than 4 seconds.

Covers:
- `feat~manufacturer-logo-display~1`

Needs: impl, itest
```

```
`dsn~current-weather-display~1`

Status: draft

The Weather information shall be fetched from Open-Meteo API, and displayed on the QT App with a refresh rate equivalent to the API's refresh rate.

Rationale: The weather information refreshes when the actual provided weather changes.

Covers:
- `feat~weather-time-info~1`

Needs: impl, itest
```

```
`dsn~current-time-display~1`

Status: draft

The Time information shall be fetched from System Info, and displayed in HH:MM format on the QT App with, and refreshed every second.

Covers:
- `feat~weather-time-info~1`

Needs: impl, itest
```

```
`dsn~cluster-gear-display~1`

Status: draft

The Cluster shall display the current gear mode as a single letter: P, N, D, or R.

Covers: `feat~gear-selection-modes~1`

Needs: impl, itest
```

## Rasp Remote
```
`dsn~remote-control-functions~1`

Status: draft

The remote control shall implement controls to update the throttle and steering, enable emergency braking, and to alternate betwen gears.

Covers:
- `feat~remote-control~1`

Needs: impl, itest
```
```
`dsn~gear-park-behavior~1`

Status: draft

When in Park (P) mode, the system shall set throttle to 0% and disable all motor control.

Covers: `feat~gear-selection-modes~1`

Needs: impl, itest
```

```
`dsn~gear-neutral-behavior~1`

Status: draft

When in Neutral (N) mode, the system shall set throttle to 0% and disable all motor control.

Covers: `feat~gear-selection-modes~1`

Needs: impl, itest
```

```
`dsn~gear-drive-behavior~1`

Status: draft

When in Drive (D) mode, the system shall enable motor control in both forward and backward directions based on driver input.

Covers: `feat~gear-selection-modes~1`

Needs: impl, itest
```

```
`dsn~gear-reverse-behavior~1`

Status: draft

When in Reverse (R) mode, the system shall enable motor control in backward direction only and ignore forward direction commands.

Covers: `feat~gear-selection-modes~1`

Needs: impl, itest
```

## Rasp Communications
```
`dsn~can-bus-configuration~1`

Status: draft

The system shall use CAN 2.0B with 500 kbps bit rate for all inter-module communication.

Covers: `feat~control-systems~1`

Needs: impl, itest
```

```
`dsn~can-bus-load~1`

Status: draft

The CAN bus utilization shall not exceed 40% under normal operating conditions with all messages transmitting at specified periods.

Rationale: 40% margin ensures bandwidth for error frames, retransmissions, and future expansion.

Covers: `feat~control-systems~1`

Needs: impl, itest
```

```
`dsn~heartbeat-transmission~1`

Status: draft

The Rasp Communications module shall transmit a periodic Heartbeat CAN message to the STM32, at a maximum interval of X ms.

Rationale: Regular heartbeat allows receiving modules to detect communication failures.

Covers: `feat~failsafe-if-no-heartbeat~1`
Needs: impl, itest
```

## STM Drivers

## STM Sensors
```
`dsn~speed-measure-device~1`

Status: draft

The Sensors module shall update the speed value in the QT display.

Covers: `feat~report-speed-battery~1`

Needs: impl, itest
```

```
`dsn~battery-measure-device~1`

Status: draft

The Sensors module shall update the battery value in the QT display.

Covers: `feat~report-speed-battery~1`

Needs: impl, itest
```

## STM Safety
```
`dsn~safety-module-authority~1`

Status: draft

The Safety Module (STM32) shall have final authority over vehicle motion commands and shall override Perception Module outputs when obstacle detected within safety threshold, immediately (< 10 ms) activating the FAILSAFE mode upon object detection, and stopping the vehicle (set throttle 0% and enable safe braking).

Rationale: Provides hardware redundancy. If Camera or RPi5 malfunction, STM32 with distance sensor can still prevent collisions.

Covers: 
- `feat~stop-before-obstacles~1`
- `feat~backup-perception~1`
- `feat~fail-safe-mode~1`

Needs: impl, itest
```

## STM General
`feat~rtos-control~1`


## No Module:
`feat~software-updates~1`
`feat~recover-from-non-critical-errors~1`
`feat~store-conf-data~1`
`feat~record-debugging-info~1`


# Draft
## Perception
```
`dsn~display-traffic-signs~1`

Status: draft

This requirement aims to identify traffic signs and distringuish them between types.
We need to specify further constraints for this and to set up a communication mechanism between Perception and Cluster, for object dection and display.

Covers:
- `feat~show-signs-obstacles~1`

Needs: impl, itest
```

```
`dsn~perception-camera-input~1`

Status: draft

Specify a refresh rate for the image capture, and the physical connections and layout demanded for this.

Covers: `feat~perception-using-camera~1`

Needs: impl, itest
```

```
`dsn~perception-camera-failure-detection~1`

Status: draft

The Perception Module shall detect camera failures (no signal, corrupted frames) within 100ms and immediately (< 10 ms) activate FAILSAFE Mode through the Communications module.

Covers: `feat~perception-using-camera~1`

Needs: impl, itest
```

```
`dsn~perception-module-output~1`

Status: draft

The Perception Module shall output xxx to the Planning module.

Covers: `feat~perception-using-camera~1`

Needs: impl, itest
```

```
`dsn~perception-module-output~1`

Status: draft

The Perception Module shall detect traffic signs in Portugal and distinguish them between types, communicating with the Cluster to display them accordingly...

Covers: `feat~show-signs-obstacles~1`

Needs: impl, itest
```

```
`dsn~perception-module-output~1`

Status: draft

The Perception Module shall detect pedestrians communicating with the Cluster to display them accordingly.

Covers: `feat~show-signs-obstacles~1`

Needs: impl, itest
```

## Rasp Planning/AI
```
`dsn~planning-module-output~1`

Status: draft

The Planning Module shall output steering and throttle information to send over CAN for direct motor actuation by the STM.

Covers: `feat~navigate-track-autonomously~1`

Needs: impl, itest
```

```
`dsn~planning-obstacle-detection~1`

Status: draft

If an object is detected

Covers: `feat~stop-before-obstacles~1`

Needs: impl, itest
```

```
`dsn~watchdog-enabling~1`

Status: draft

The watchdog shall be innitialized on STM32 startup and shall be reset every 10 ms......

Covers:
- `feat~weather-time-info~1`

Needs: impl, itest
```

## Rasp Mode Manager
```
`dsn~manual-auto-mode~1`

Status: draft

The

Covers:
- `feat~remote-control~1`

Needs: impl, itest
```

## Control Module

```
`dsn~pid-output-clamping~1`

Status: draft

The PID controller output shall be clamped to the range [0%, 100%] before being applied as a throttle command, where 0% represents fully closed throttle and 100% represents fully open throttle.

Covers:
- `feat~cruise-control~1`

Needs: impl, utest
```

```
`dsn~pid-anti-windup~1`

Status: draft

The PID controller shall implement integral anti-windup by suspending the accumulation of the integral term whenever the controller output is saturated (i.e., clamped at 0% or 100%) and the sign of the error would further increase the magnitude of the integrator.

Covers:
- `feat~cruise-control~1`

Needs: impl, utest
```

```
`dsn~cruise-control-activation-command~1`

Status: proposed

The cruise control system shall activate only when the driver actuates the `+` cruise control command and all activation conditions are satisfied.

Covers:
- `feat~cruise-control~1`

Needs: impl, itest
```

```
`dsn~cruise-control-activation-conditions~1`

Status: proposed

The cruise control system shall only allow activation if all of the following conditions are simultaneously satisfied: vehicle speed is within the valid range [15, 130] hm/h, the brake pedal and accelerator pedal are not actuated, and no active fault codes related vehicle speed sensor.

Covers:
- `feat~cruise-control~1`

Needs: impl, itest
```

```
`dsn~cruise-control-initial-target-speed~1`

Status: proposed

Upon cruise control activation, the system shall store the current vehicle speed as the initial target speed.

Covers:
- `feat~cruise-control~1`

Needs: impl, itest
```

```

`dsn~cruise-control-target-speed-increment~1`

Status: proposed

If the driver actuates the `+` cruise control command while cruise control is active, the system shall increase the active target speed by 1 hm/h for each command actuation.

Covers:
- `feat~cruise-control~1`

Needs: impl, itest
```

```
`dsn~cruise-control-target-speed-decrement~1`

Status: proposed

If the driver actuates the `-` cruise control command while cruise control is active, the system shall decrease the active target speed by 1 hm/h for each command actuation.

Covers:
- `feat~cruise-control~1`

Needs: impl, itest
```

```
`dsn~cruise-control-throttle-authority~1`

Status: draft

When cruise control is active, the system shall generate the commanded throttle from the cruise control speed controller rather than from continuous driver throttle input, unless a deactivation condition is present.

Covers:
- `feat~cruise-control~1`

Needs: impl, itest
```

```
`dsn~cruise-control-driver-deactivation~1`

Status: draft

If the driver actuates the accelerator pedal or brake pedal while cruise control is active, the system shall deactivate cruise control and transfer throttle authority back to the driver.

Covers:
- `feat~cruise-control~1`

Needs: impl, itest
```

```
`dsn~cruise-control-fault-deactivation~1`

Status: draft

If a cruise-control communication fault is detected while cruise control is active, the system shall deactivate cruise control and transfer throttle authority back to the driver.

Covers:
- `feat~cruise-control~1`

Needs: impl, itest
```

```
`dsn~cruise-control-speed-regulation-accuracy~1`

Status: draft

When cruise control is active and no deactivation condition is present, the system shall regulate vehicle speed to within ±2 hm/h of the active target speed under steady-state road load conditions.

Covers:
- `feat~cruise-control~1`

Needs: impl, itest
```
