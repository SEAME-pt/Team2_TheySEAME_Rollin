# Vehicle Architecture Requirements  

## STM Communications

```
`arch~stm-rpi-can-control~1`

Status: approved

The STM and Rasp Comms modules shall support sending control instructions from the RPi5 to the STM via CAN.

Tags:
- STM Comms
- Rasp Comms

Needs: dsn

Covers:
- `feat~manual-auto-mode~1`
```

```
`arch~stm-rpi-can-telemetry~1`

Status: approved

Shall support sending vehicle state information (speed/battery) from STM32 to Raspberry Pi via CAN.

Tags:
- STM Comms
- Rasp Comms

Needs: dsn  

Covers:
- `feat~report-speed-battery~1` 
```

```
`arch~communication-mode-handling~1`

Status: proposed

Shall provide a mechanism to transmit mode selection (Test/Manual/Auto) and fail-safe activation between RPi5 and STM.

Tags:
- STM Comms
- Rasp Comms

Needs: dsn  

Covers:
- `feat~manual-auto-mode~1`
- `feat~fail-safe-mode~1`
```

## STM Control
```
`arch~control-actuation-flow~1`

Status: approved

The STM Control module shall support mapping incoming control instructions to driver actuators.

Tags: Control  

Needs: dsn  

Covers:
- `feat~manual-auto-mode~1`
```

```
`arch~control-failsafe-override~1`

Status: proposed

The STM Control module shall support a mechanism for overriding actuation output when fail-safe mode is triggered by the Safety module.

Tags:
- Control
- Safety

Needs: dsn  

Covers:
- `feat~fail-safe-mode~1`
```  

## STM Drivers
```
`arch~pca-driver-layer~1`

Status: approved

The Drivers module shall provide a dedicated motor controller to send throttle and steering instructions via I2C to the PCA9685.

Tags: Drivers  

Needs: dsn  

Covers:
- `feat~manual-auto-mode~1`
```  

## STM Sensing
```
`arch~rpm-battery-sensing~1`

Status: approved

The Sensing moodule shall acquire wheel RPM and battery voltage measurements and expose them to Control and Communications layers.

Tags: Sensing  

Needs: dsn  

Covers:
- `feat~report-speed-battery~1`
```  

```
`arch~rpm-latency~1`

Status: approved

The speed shall be captured at a frequency of at least 2Hz.

Needs: dsn

Covers:
- `feat~report-speed-battery~1` 
```

## RASP Logging
```
`arch~runtime-logging~1`

Status: proposed

Shall record on SD card operational and debugging data during system use.

Tags: Logging

Needs: dsn  

Covers:
- `feat~record-debugging-info~1`
```  

## RASP Cluster
```
`arch~display-vehicle-status~1`

Status: approved

Shall display vehicle speed, battery level, and system state via user interface.

Tags: Cluster  

Needs: dsn  

Covers:
- `feat~report-speed-battery~1`
- `feat~friendly-gui~1`
```

```
`arch~weather-display~1`

Status: approved

Shall fetch the current weather in Porto, from Open-Meteo API, and display it on the user interface.

Tags: Cluster  

Needs: dsn  

Covers:
- `feat~weather-time-info~1`
```

```
`arch~time-display~1`

Status: approved

Shall display the system's date and time utilizing QDateTime class from QT library.

Tags: Cluster  

Needs: dsn  

Covers:
- `feat~weather-time-info~1`
```

```
`arch~auto-ui-boot~1`

Status: approved

Shall automatically launch the graphical interface at system startup.

Tags: Cluster  

Needs: dsn  

Covers:
- `feat~friendly-gui~1`
```  

```
`arch~speed-unit~1`

Status: draft

The Cluster shall display the speed in a user friendly unit (m/s or dm/s).

Tags: Cluster  

Needs: dsn  

Covers:
- `feat~friendly-gui~1`
```  

```
`arch~battery-unit~1`

Status: draft

The Cluster shall display battery in percentage of the voltage level compared to the minimum and maximum operating voltage.

Tags: Cluster  

Needs: dsn  

Covers:
- `feat~friendly-gui~1`
```  

## RASP Remote
```
`arch~remote~control~support~1`

Status: draft

The Remote Module shall communicate with a remote control and transmit it's input to the RASP Data Processing module.

Tags: Remote 

Needs: dsn  

Covers:
- `feat~remote-control~1`
```

## RASP Data Processing
```
`arch~process-raw-data-rasp~1`

Status: draft

The RPi5 shall receive raw input data from the remote and process it in order to send normalized thrust and steering data in degrees and angles to the Rasp Comms module.

Tags: Data Processing 

Needs: dsn

Covers:
- `feat~remote-control~1`
```
