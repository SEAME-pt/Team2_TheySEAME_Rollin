# PID Controller

## 1. Introduction

A **PID controller (Proportional--Integral--Derivative controller)** is
a control mechanism\
It continuously calculates the error between a **desired reference value
(setpoint)** and a **measured process variable**, and applies a
correction to minimize this error.

The controller attempts to adjust the system output so that the measured
value converges to the desired target.

------------------------------------------------------------------------

# 2. PID Control Principle

A PID controller calculates the control signal using three components:

-   **Proportional (P)**
-   **Integral (I)**
-   **Derivative (D)**

The controller output is defined as:

u(t) = Kp·e(t) + Ki∫e(t)dt + Kd·de(t)/dt

Where:

-   u(t) → controller output
-   e(t) → error = setpoint − measured value
-   Kp → proportional gain
-   Ki → integral gain
-   Kd → derivative gain

## Implementation (Code Variables)

```c
error = setpoint - current_value;

integral += error * dt;

derivative = (error - prev_error) / dt;

controller_output = kp * error + ki * integral + kd * derivative;

prev_error = error;
```
------------------------------------------------------------------------

# 3. Error Definition

The error is defined as:

e(t) = r(t) − y(t)

Where:

-   r(t) → target value (setpoint)
-   y(t) → measured system output

The goal of the controller is to drive the error towards **zero**.

------------------------------------------------------------------------

# 4. Proportional Control (P)

The proportional term produces an output proportional to the current
error.

P = Kp · e(t)

Advantages: - Fast response - Simple implementation

Disadvantages: - Cannot eliminate steady‑state error - Too large values
cause oscillations

------------------------------------------------------------------------

# 5. Integral Control (I)

The integral term accumulates past errors and corrects long‑term
deviations.

I = Ki ∫ e(t)dt

Advantages: - Eliminates steady‑state error

Disadvantages: - Can cause overshoot - Slower response - May introduce
instability if too large

------------------------------------------------------------------------

# 6. Derivative Control (D)

The derivative term predicts the future trend of the error by measuring
its rate of change.

D = Kd · de(t)/dt

Advantages: - Improves stability - Reduces overshoot - Dampens
oscillations

Disadvantages: - Sensitive to noise - Not always required

------------------------------------------------------------------------

# 7. PID Tuning

Choosing appropriate values for **Kp**, **Ki**, and **Kd** is essential
for achieving good performance.

Tuning goals:

-   Fast response
-   Minimal overshoot
-   Stability
-   Small steady‑state error

One classical tuning approach is the **Ziegler--Nichols method**.

------------------------------------------------------------------------

# 8 PID Parameters

PID parameters were determined through **empirical tuning** based on step
response tests.

The final gains used in the controller are:

- **Kp = 9.0**
- **Ki = 0.02**
- **Kd = 0.05**

These parameters provide:

- fast rise time
- minimal overshoot
- stable behaviour
- smooth convergence to the reference value

The controller was validated using several reference changes:

- **0 → 40 m/s**
- **40 → 80 m/s**
- **80 → 120 m/s**
- **120 → 20 m/s**

In all tested scenarios, the system converged quickly to the target speed
with negligible steady-state error.