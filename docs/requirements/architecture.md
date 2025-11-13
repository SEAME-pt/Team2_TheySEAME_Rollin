# Software Architecture Modules and Sub-Modules
## Control
- Remote Control
- Auto Control
## ADAS
- Automatic emergency braking (AEB)
- Lane departure warning (LDW),
- Adaptive cruise control (ACC),
- Blind-spot detection,
- Traffic sign recognition (TSR),
- Parking assistance (PA).
## Speed Data
- Speed Measurement
- Speed Reporting
## Instrument Cluster
- Speed Data
- Battery Status
- ADAS Feedback
## Communication
- I2C interface
- CAN Bus interface

`arch~instrument-cluster-speed~1`
The Cluster shall display speed data from the speedometer with a resolution of at least 0.1 m/s.

Component: Instrument Cluster/Speed Data

Covers:
- feat~report-speed-battery~1