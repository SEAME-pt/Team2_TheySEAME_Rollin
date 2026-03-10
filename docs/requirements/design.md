# Design Requirements

## Cluster
```
`dsn~display-battery-real-time~1`

Status: proposed

The QT App (cluster) shall display the battery state of the vehicle in real time in percentage (%), as measured, and communicated via CAN, from the STM32 Sensor's module, with a maximum refresh rate of, 0.1 Hz.

Rationale: 0.1 Hz equals 1 time every ten seconds. It was considered that the timescale of changes for this value is smaller than other variables, for instance, the speed measurement.

Covers:
- `feat~report-speed-battery~1`

Needs: impl, itest
```

```
`dsn~display-speed-real-time~1`

Status: proposed

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

Status: proposed

The manufacturer (They SEA:ME Rolling Team) shall be displayed on startup for no more than 4 seconds.

Covers:
- `feat~manufacturer-logo-display~1`

Needs: impl, itest
```

```
`dsn~current-weather-display~1`

Status: proposed

The Weather information shall be fetched from Open-Meteo API, and displayed on the QT App with a refresh rate equivalent to the API's refresh rate.

Rationale: The weather information refreshes when the actual provided weather changes.

Covers:
- `feat~weather-time-info~1`

Needs: impl, itest
```

```
`dsn~current-time-display~1`

Status: proposed

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

Status: proposed

The remote control shall implement controls to update the throttle and steering, enable emergency braking, and to alternate betwen gears.

Covers:
- `feat~remote-control~1`

Needs: impl, itest
```
```
`dsn~gear-park-behavior~1`

Status: proposed

When in Park (P) mode, the system shall set throttle to 0% and disable all motor control.

Covers: `feat~gear-selection-modes~1`

Needs: impl, itest
```

```
`dsn~gear-neutral-behavior~1`

Status: proposed

When in Neutral (N) mode, the system shall set throttle to 0% and disable all motor control.

Covers: `feat~gear-selection-modes~1`

Needs: impl, itest
```

```
`dsn~gear-drive-behavior~1`

Status: proposed

When in Drive (D) mode, the system shall enable motor control in both forward and backward directions based on driver input.

Covers: `feat~gear-selection-modes~1`

Needs: impl, itest
```

```
`dsn~gear-reverse-behavior~1`

Status: proposed

When in Reverse (R) mode, the system shall enable motor control in backward direction only and ignore forward direction commands.

Covers: `feat~gear-selection-modes~1`

Needs: impl, itest
```

## Rasp Communications
```
`dsn~can-bus-configuration~1`

Status: proposed

The system shall use CAN 2.0B with 500 kbps bit rate for all inter-module communication.

Covers: `feat~control-systems~1`

Needs: impl, itest
```

```
`dsn~can-bus-load~1`

Status: proposed

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

Status: proposed

The Sensors module shall update the speed value in the QT display.

Covers: `feat~report-speed-battery~1`

Needs: impl, itest
```

```
`dsn~battery-measure-device~1`

Status: proposed

The Sensors module shall update the battery value in the QT display.

Covers: `feat~report-speed-battery~1`

Needs: impl, itest
```

## STM Safety
```
`dsn~safety-module-authority~1`

Status: proposed

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
`dsn~pid-proportional-term~1`

Status: proposed

The PID controller shall compute a proportional output term as the product of the proportional gain `Kp` and the instantaneous speed error `e(t) = v_target - v_measured`, expressed in km/h.

Rationale: The proportional term provides an immediate corrective response proportional to the magnitude of the current error, forming the primary driver of the control action.

Covers:
- `feat~pid-speed-control~1`

Needs: impl, utest
```

---

```
`dsn~pid-integral-term~1`

Status: proposed

The PID controller shall compute an integral output term as the product of the integral gain `Ki` and the discrete-time accumulation of speed error over time, using the update period `dt` as the integration step, in accordance with `I(t) = I(t-1) + Ki * e(t) * dt`.

Rationale: The integral term eliminates steady-state error that the proportional term alone cannot correct, such as the constant throttle offset needed to maintain speed on an incline.

Covers:
- `feat~pid-speed-control~1`

Needs: impl, utest
```

---

```
`dsn~pid-derivative-term~1`

Status: proposed

The PID controller shall compute a derivative output term as the product of the derivative gain `Kd` and the rate of change of speed error between consecutive control cycles, in accordance with `D(t) = Kd * (e(t) - e(t-1)) / dt`.

Rationale: The derivative term anticipates future error trends and damps oscillatory behaviour, preventing overshoot during rapid speed changes such as downhill acceleration.

Covers:
- `feat~pid-speed-control~1`

Needs: impl, utest
```

---

```
`dsn~pid-output-clamping~1`

Status: proposed

The PID controller output shall be clamped to the range [0%, 100%] before being applied as a throttle command, where 0% represents fully closed throttle and 100% represents fully open throttle.

Rationale: Clamping prevents the controller from issuing physically unrealisable throttle commands and protects the actuator from saturation-induced instability (integrator wind-up mitigation).

Covers:
- `feat~pid-speed-control~1`

Needs: impl, utest
```

---

```
`dsn~pid-anti-windup~1`

Status: proposed

The PID controller shall implement integral anti-windup by suspending the accumulation of the integral term whenever the controller output is saturated (i.e., clamped at 0% or 100%) and the sign of the error would further increase the magnitude of the integrator.

Rationale: Without anti-windup, the integrator continues to accumulate during saturation, causing a large overshoot or delayed response when the system leaves saturation.

Covers:
- `feat~pid-speed-control~1`

Needs: impl, utest
```

---

```
`dsn~pid-update-rate~1`

Status: proposed

The PID controller shall execute its control loop at a fixed update rate of 100 Hz (i.e., `dt = 10 ms`), synchronised with the vehicle speed measurement input.

Rationale: 100 Hz provides a control bandwidth well above the mechanical response bandwidth of the throttle actuator (typically < 10 Hz), ensuring the discrete approximation of the continuous-time PID is accurate and stable.

Covers:
- `feat~pid-speed-control~1`

Needs: impl, itest
```

---

```
`dsn~cruise-control-activation-conditions~1`

Status: proposed

The cruise control system shall only allow activation if all of the following conditions are simultaneously satisfied: vehicle speed is within the valid range [30, 180] km/h, the brake pedal is not actuated, the engine is running, and no active fault codes related to the throttle, speed sensor, or CAN bus are present.

Rationale: Gating activation on these conditions prevents engagement in unsafe or degraded system states.

Covers:
- `feat~cruise-control-speed-regulation~1`
- `feat~cruise-control-set-target-speed~1`

Needs: impl, itest
```
---

```
`dsn~cruise-control-deactivation-response-time~1`

Status: proposed

Upon detection of a deactivation event, the cruise control system shall transfer throttle authority back to the driver within a maximum latency of 50 ms from the moment the trigger signal is asserted.

Rationale: A 50 ms response time is within the range imperceptible to the driver and well below the threshold that could cause a safety-relevant delay in throttle handover.

Covers:
- `feat~cruise-control-deactivation~1`

Needs: impl, itest
```

---
