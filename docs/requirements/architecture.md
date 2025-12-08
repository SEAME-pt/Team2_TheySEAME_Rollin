# Vehicle Architecture Requirements  

## 📡 Communications

`arch~stm-rpi-can-control~1`
The STM and Rasp Comms modules shall support sending pre-defined control instructions from the Raspberry Pi to the STM32 via CAN, and vice-versa.

Tags:
- STM Comms
- Rasp Comms

Needs: dsn

Covers: feat~manual-auto-mode~1

Status: draft


`arch~stm-rpi-can-telemetry~1`
Shall support sending vehicle state information (speed/RPM/battery) from STM32 to Raspberry Pi via CAN.

Tags:
- STM Comms
- Rasp Comms

Needs: dsn  

Covers: feat~report-speed-battery~1  

Status: draft


`arch~communication-mode-handling~1`
Shall provide a mechanism to transmit mode selection (Test/Manual/Auto) and fail-safe activation between Raspberry Pi and STM32.

Tags:
- STM Comms
- Rasp Comms

Needs: dsn  

Covers: feat~manual-auto-mode~1, feat~fail-safe-mode~1  

Status: draft


## Control
`arch~control-actuation-flow~1`
The STM Control module hall support mapping incoming control instructions to actuator output functions.

Tags: Control  

Needs: dsn  

Covers: feat~manual-auto-mode~1  

Status: draft

`arch~control-failsafe-override~1`
The STM Control module shall support a mechanism for overriding actuation output when fail-safe mode is triggered.

Tags: Control  

Needs: dsn  

Covers: feat~fail-safe-mode~1  

Status: draft


## Drivers
`arch~pwm-driver-layer~1`
The Drivers module shall provide a dedicated motor controller to send throttle and steering instructions via I2C to the PCA9685 .

Tags: Drivers  

Needs: dsn  

Covers: feat~manual-auto-mode~1  

Status: draft


## Sensing
`arch~rpm-battery-sensing~1`
The Sensing moodule shall acquire wheel RPM and battery voltage measurements and expose them to Control and Communications layers.

Tags: Sensing  

Needs: dsn  

Covers: feat~report-speed-battery~1  

Status: draft


## Logging & Diagnostics
`arch~runtime-logging~1`
Shall record operational and debugging data during system use.

Tags: Logging  

Needs: dsn  

Covers: feat~record-debugging-info~1  

Status: draft


## HMI / Graphical User Interface
`arch~display-vehicle-status~1`
Shall display vehicle speed, battery level, and system state via user interface.

Tags: HMI  

Needs: dsn  

Covers: feat~report-speed-battery~1, feat~friendly-gui~1  

Status: draft

`arch~auto-ui-boot~1`
Shall automatically launch the graphical interface at system startup.

Tags: HMI  

Needs: dsn  

Covers: feat~friendly-gui~1  

Status: draft
