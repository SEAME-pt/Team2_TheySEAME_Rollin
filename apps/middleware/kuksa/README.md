# Kuksa Integration (Raspberry Pi 5 + AGL)

This document describes the step-by-step procedure to set up and run the **Kuksa Databroker** together with the **Kuksa CAN Provider**, using **SocketCAN** and **DBC-to-VSS mapping** for the current vehicle and ADAS signals.

## What is Kuksa and why use it?

**Eclipse Kuksa** is an open-source set of components for the Software-Defined Vehicle (SDV) ecosystem. In this setup, Kuksa provides a **standard way to access and distribute vehicle signals** such as speed, battery state of charge, steering angle, cruise control targets, and traffic sign recognition results through a **gRPC-based broker**.

Why use Kuksa in this project?

- **Standardized signals**: use VSS paths like `Vehicle.Speed` instead of ad-hoc names.
- **Decoupled architecture**: producers such as the CAN provider and consumers such as Qt applications only interact with the broker.
- **Single source of truth**: the Databroker becomes the central place to publish and read signal values.
- **Easier integration**: multiple applications and services can subscribe to the same datapoints without duplicating CAN decoding logic.
- **Scalable ADAS integration**: new features such as LKA, ACC, BSD, TSR, and Parking Assist can be added by extending the VSS tree and DBC file.

---

## Architecture

### Kuksa Databroker
- Central gRPC-based data broker
- Stores and serves VSS datapoints
- Runs in the background as a `systemd` service

### Kuksa CAN Provider
- Reads CAN frames via SocketCAN
- Decodes frames using a DBC file
- Publishes values to the Databroker using VSS paths
- Can also write selected actuator values back to CAN when using `--val2dbc`
- Runs in the background as a `systemd` service

### VSS Specification (`vss.json`)
- Defines the datapoint tree, including paths, types, and units available in the Databroker
- Must include the same VSS paths used by the CAN Provider mapping
- If a path does not exist in the loaded VSS, you get errors such as `Signal <VSS path> is not registered`
- Location: `/etc/kuksa/vss.json`

### DBC File (`CAN.dbc`)
- Defines CAN messages and signals: arbitration IDs, bit layout, scaling, offsets, ranges, and units
- Used by the CAN Provider to decode raw CAN frames into signal values
- Location: `/etc/kuksa/CAN.dbc`

### Databroker Client
- Used to inspect, validate, and debug datapoints exposed by Kuksa

---

## Configuration Files

| File | Purpose |
|---|---|
| `/etc/kuksa-can-provider/config.ini` | Main CAN Provider configuration |
| `/etc/default/kuksa-can-provider` | CAN Provider extra arguments |
| `/etc/default/kuksa-databroker` | Databroker extra arguments |
| `/etc/kuksa/vss.json` | VSS tree and mapping metadata |
| `/etc/kuksa/CAN.dbc` | CAN message and signal database |

---

## Prerequisites

- Raspberry Pi 5 with AGL
- CAN interface available as `can0`
- Kuksa layer included in the AGL build
- Valid VSS file installed at `/etc/kuksa/vss.json`
- Valid DBC file installed at `/etc/kuksa/CAN.dbc`

---

## 1. Kuksa Databroker (System Service)

### Start the Databroker

```bash
systemctl start kuksa-databroker.service
```

### Enable at boot

```bash
systemctl enable kuksa-databroker.service
```

### Check status

```bash
systemctl status kuksa-databroker.service
```

### Databroker configuration

File: `/etc/default/kuksa-databroker`

```bash
EXTRA_ARGS="\
  --address 0.0.0.0 \
  --port 55555 \
  --vss /etc/kuksa/vss.json \
"
```

---

## 2. Kuksa CAN Provider (System Service)

### Start the CAN Provider

```bash
systemctl start kuksa-can-provider.service
```

### Enable at boot

```bash
systemctl enable kuksa-can-provider.service
```

### Check status

```bash
systemctl status kuksa-can-provider.service
```

### CAN Provider extra arguments

File: `/etc/default/kuksa-can-provider`

```bash
EXTRA_ARGS="\
  -server-type kuksa_databroker \
  --dbcfile /etc/kuksa/CAN.dbc \
  --mapping /etc/kuksa/vss.json \
  --use-socketcan \
  --dbc2val \
  --val2dbc \
"
```

### Main configuration

File: `/etc/kuksa-can-provider/config.ini`

```ini
# VSS mapping file
mapping = /etc/kuksa/vss.json

# DBC file used to parse CAN messages
dbcfile = /etc/kuksa/CAN.dbc

# CAN port
port = can0

# IP address for server (Kuksa Databroker)
ip = 0.0.0.0
```

### Notes about `--dbc2val` and `--val2dbc`

- `--dbc2val`: decodes CAN frames from `can0` and publishes them to Kuksa using the VSS paths
- `--val2dbc`: listens for value updates in Kuksa and encodes selected actuator signals back to CAN frames

This is useful in this project because:
- sensors and status signals can come from STM32 into Kuksa
- actuator commands such as throttle, steering, gear, brake, driving mode, LKA target steering angle, and cruise control targets can be written from applications into Kuksa and forwarded back to CAN

---

## 3. Validate Databroker with Kuksa Client

Open a second terminal and run:

```bash
kuksa-client
```

Inside the Kuksa Client prompt, try:

```text
getValue Vehicle.Speed
getValue Vehicle.Powertrain.Battery.StateOfCharge
getValue Vehicle.Control.Mode.DrivingMode
getValue Vehicle.ADAS.LaneKeepAssist.Active
getValue Vehicle.ADAS.TrafficSignRecognition.DetectedSignType
```

To see the list of commands:

```text
help
```

### Note about VSS paths

If you see errors like:

```text
not found
not registered
```

then the Databroker does not have those VSS nodes loaded.

Check the following:
- `/etc/kuksa/vss.json` exists and is valid JSON
- the path exists in the VSS tree
- the CAN Provider mapping uses the exact same VSS path
- signal names in the DBC match the names referenced in `dbc.signal`

---

## 4. Application Dependencies (gRPC + Protobuf)

If you build an application that communicates with Kuksa over gRPC, install the development packages for Protobuf and gRPC and ensure the generated `*.pb.cc`, `*.pb.h`, `*.grpc.pb.cc`, and `*.grpc.pb.h` files are compiled into your target.

### 4.1 Install required packages

```bash
sudo apt-get update
sudo apt-get install -y \
  pkg-config \
  protobuf-compiler \
  libprotobuf-dev \
  protobuf-compiler-grpc \
  libgrpc-dev \
  libgrpc++-dev
```

Quick checks:

```bash
protoc --version
which grpc_cpp_plugin
pkg-config --modversion protobuf grpc++ grpc
```

> On Ubuntu 22.04, CMake often does not provide `gRPCConfig.cmake`. Using `pkg-config` is usually the simplest approach.

### 4.2 Add generated Protobuf/gRPC sources to a Qt target

```cmake
target_sources(qtAppLib PRIVATE
  ${CMAKE_SOURCE_DIR}/path/to/middleware/Kuksa/val/v2/types.pb.cc
  ${CMAKE_SOURCE_DIR}/path/to/middleware/Kuksa/val/v2/val.pb.cc
  ${CMAKE_SOURCE_DIR}/path/to/middleware/Kuksa/val/v2/val.grpc.pb.cc
)
```

Include directories:

```cmake
target_include_directories(qtAppLib PRIVATE
  ${CMAKE_SOURCE_DIR}/path/to/middleware/Kuksa
  ${CMAKE_SOURCE_DIR}/path/to/middleware/Kuksa/val/v2
)
```

### 4.3 Link with gRPC and Protobuf using `pkg-config`

```cmake
find_package(PkgConfig REQUIRED)

pkg_check_modules(GRPC REQUIRED IMPORTED_TARGET grpc++ grpc)
pkg_check_modules(PROTOBUF REQUIRED IMPORTED_TARGET protobuf)

target_link_libraries(qtAppLib PRIVATE
  PkgConfig::GRPC
  PkgConfig::PROTOBUF
)
```

---

# CAN Frame Architecture

## Node roles

- **STM32** transmits most measured vehicle and ADAS status signals
- **RASPI** transmits most control and ADAS command signals

## Message overview

| Message | ID (hex) | ID (dec) | Sender | Signals |
|---|---:|---:|---|---|
| SpeedMsg | `0x200` | 512 | STM32 | `speed` |
| BatteryMsg | `0x201` | 513 | STM32 | `StateOfCharge` |
| ThrottleMsg | `0x100` | 256 | RASPI | `throttle` |
| BrakeMsg | `0x101` | 257 | RASPI | `brake` |
| AngleMsg | `0x102` | 258 | RASPI | `angle` |
| DrivingModeMsg | `0x104` | 260 | RASPI | `DrivingMode` |
| GearMsg | `0x105` | 261 | RASPI | `gear` |
| LaneKeepAssistCmdMsg | `0x208` | 520 | RASPI | `LKA_Enabled`, `LKA_TargetSteeringAngle` |
| LaneKeepAssistStatusMsg | `0x209` | 521 | STM32 | `LKA_Active`, `LKA_LaneOffset` |
| LaneDepartureWarningMsg | `0x20A` | 522 | STM32 | `LDW_Enabled`, `LDW_Warning` |
| CruiseControlCmdMsg | `0x212` | 530 | RASPI | `CC_Enabled`, `CC_TargetSpeed` |
| CruiseControlStatusMsg | `0x213` | 531 | STM32 | `CC_Active` |
| ACCCmdMsg | `0x214` | 532 | RASPI | `ACC_Enabled`, `ACC_TargetSpeed`, `ACC_TimeGap` |
| ACCStatusMsg | `0x215` | 533 | STM32 | `ACC_Active`, `ACC_LeadVehicleDistance` |
| BlindSpotDetectionMsg | `0x21C` | 540 | STM32 | `BSD_Enabled`, `BSD_LeftOccupied`, `BSD_RightOccupied`, `BSD_Warning` |
| TrafficSignRecognitionMsg | `0x226` | 550 | STM32 | `TSR_Enabled`, `TSR_DetectedSpeedLimit`, `TSR_DetectedSignType` |
| ParkingAssistMsg | `0x230` | 560 | STM32 | `PA_Enabled`, `PA_Active`, `PA_ObstacleDistanceFront`, `PA_ObstacleDistanceRear` |

## Signal-level summary

| Message | Signal | Range | Unit | Description |
|---|---|---:|---|---|
| SpeedMsg | `speed` | 0–250 | hm/h | Current vehicle speed |
| BatteryMsg | `StateOfCharge` | 0–100 | % | Battery charge level |
| ThrottleMsg | `throttle` | 0–100 | % | Throttle command |
| BrakeMsg | `brake` | 0–1 | enum | 0 = OFF, 1 = ON |
| AngleMsg | `angle` | -30 to 30 | deg | Steering command |
| DrivingModeMsg | `DrivingMode` | 0–2 | enum | 0 = MANUAL, 1 = AI_ASSIST, 2 = AUTONOMOUS |
| GearMsg | `gear` | 0–3 | enum | 0 = PARK, 1 = NEUTRAL, 2 = REVERSE, 3 = DRIVE |
| LaneKeepAssistCmdMsg | `LKA_Enabled` | 0–1 | bool | Enable LKA |
| LaneKeepAssistCmdMsg | `LKA_TargetSteeringAngle` | -45 to 45 | deg | LKA steering correction |
| LaneKeepAssistStatusMsg | `LKA_Active` | 0–1 | bool | LKA active state |
| LaneKeepAssistStatusMsg | `LKA_LaneOffset` | -10 to 10 | m | Offset from lane center |
| LaneDepartureWarningMsg | `LDW_Enabled` | 0–1 | bool | LDW enabled state |
| LaneDepartureWarningMsg | `LDW_Warning` | 0–1 | bool | Lane departure warning active |
| CruiseControlCmdMsg | `CC_Enabled` | 0–1 | bool | Enable cruise control |
| CruiseControlCmdMsg | `CC_TargetSpeed` | 0–250 | hm/h | Cruise control target speed |
| CruiseControlStatusMsg | `CC_Active` | 0–1 | bool | Cruise control active |
| ACCCmdMsg | `ACC_Enabled` | 0–1 | bool | Enable ACC |
| ACCCmdMsg | `ACC_TargetSpeed` | 0–250 | hm/h | ACC target speed |
| ACCCmdMsg | `ACC_TimeGap` | 0–10 | s | ACC desired time gap |
| ACCStatusMsg | `ACC_Active` | 0–1 | bool | ACC active |
| ACCStatusMsg | `ACC_LeadVehicleDistance` | 0–300 | m | Distance to lead vehicle |
| BlindSpotDetectionMsg | `BSD_Enabled` | 0–1 | bool | BSD enabled state |
| BlindSpotDetectionMsg | `BSD_LeftOccupied` | 0–1 | bool | Left blind spot occupied |
| BlindSpotDetectionMsg | `BSD_RightOccupied` | 0–1 | bool | Right blind spot occupied |
| BlindSpotDetectionMsg | `BSD_Warning` | 0–1 | bool | BSD warning active |
| TrafficSignRecognitionMsg | `TSR_Enabled` | 0–1 | bool | TSR enabled state |
| TrafficSignRecognitionMsg | `TSR_DetectedSpeedLimit` | 0–250 | hm/h | Detected speed limit |
| TrafficSignRecognitionMsg | `TSR_DetectedSignType` | 0–15 | enum | Detected traffic sign type |
| ParkingAssistMsg | `PA_Enabled` | 0–1 | bool | Parking assist enabled |
| ParkingAssistMsg | `PA_Active` | 0–1 | bool | Parking assist active |
| ParkingAssistMsg | `PA_ObstacleDistanceFront` | 0–20 | m | Front obstacle distance |
| ParkingAssistMsg | `PA_ObstacleDistanceRear` | 0–20 | m | Rear obstacle distance |

---

# Vehicle Signal Specification (VSS) Mapping

## Base vehicle signals

| VSS Path | Type | Data Type | Unit | Description | CAN Signal |
|---|---|---|---|---|---|
| `Vehicle.Speed` | sensor | float | hm/h | Vehicle speed from STM32 | `speed` |
| `Vehicle.Powertrain.Battery.StateOfCharge` | sensor | uint8 | percent | Battery state of charge | `StateOfCharge` |
| `Vehicle.Control.Throttle.Value` | actuator | float | percent | Throttle command | `throttle` |
| `Vehicle.Control.Gear.Value` | actuator | uint8 | enum | Current gear | `gear` |
| `Vehicle.Control.Steering.Angle` | actuator | float | deg | Steering angle command | `angle` |
| `Vehicle.Control.Brake.Value` | actuator | uint8 | enum | Brake command: 0 = OFF, 1 = ON | `brake` |
| `Vehicle.Control.Mode.DrivingMode` | actuator | uint8 | enum | Driving mode: 0 = MANUAL, 1 = AI_ASSIST, 2 = AUTONOMOUS | `DrivingMode` |

## ADAS signals

| VSS Path | Type | Data Type | Unit | Description | CAN Signal |
|---|---|---|---|---|---|
| `Vehicle.ADAS.LaneKeepAssist.Enabled` | actuator | boolean | - | Enable or disable LKA | `LKA_Enabled` |
| `Vehicle.ADAS.LaneKeepAssist.Active` | sensor | boolean | - | LKA currently active | `LKA_Active` |
| `Vehicle.ADAS.LaneKeepAssist.TargetSteeringAngle` | actuator | float | deg | Steering correction command from LKA | `LKA_TargetSteeringAngle` |
| `Vehicle.ADAS.LaneKeepAssist.LaneOffset` | sensor | float | m | Vehicle lateral offset from lane center | `LKA_LaneOffset` |
| `Vehicle.ADAS.LaneDepartureWarning.Enabled` | actuator | boolean | - | Enable or disable LDW | `LDW_Enabled` |
| `Vehicle.ADAS.LaneDepartureWarning.Warning` | sensor | boolean | - | Lane departure warning active | `LDW_Warning` |
| `Vehicle.ADAS.CruiseControl.Enabled` | actuator | boolean | - | Enable or disable cruise control | `CC_Enabled` |
| `Vehicle.ADAS.CruiseControl.Active` | sensor | boolean | - | Cruise control active | `CC_Active` |
| `Vehicle.ADAS.CruiseControl.TargetSpeed` | actuator | float | hm/h | Target speed for cruise control | `CC_TargetSpeed` |
| `Vehicle.ADAS.AdaptiveCruiseControl.Enabled` | actuator | boolean | - | Enable or disable ACC | `ACC_Enabled` |
| `Vehicle.ADAS.AdaptiveCruiseControl.Active` | sensor | boolean | - | ACC active | `ACC_Active` |
| `Vehicle.ADAS.AdaptiveCruiseControl.TargetSpeed` | actuator | float | hm/h | Target speed for ACC | `ACC_TargetSpeed` |
| `Vehicle.ADAS.AdaptiveCruiseControl.TimeGap` | actuator | float | s | Desired time gap to the front vehicle | `ACC_TimeGap` |
| `Vehicle.ADAS.AdaptiveCruiseControl.LeadVehicleDistance` | sensor | float | m | Distance to lead vehicle | `ACC_LeadVehicleDistance` |
| `Vehicle.ADAS.BlindSpotDetection.Enabled` | actuator | boolean | - | Enable or disable BSD | `BSD_Enabled` |
| `Vehicle.ADAS.BlindSpotDetection.LeftOccupied` | sensor | boolean | - | Object detected in left blind spot | `BSD_LeftOccupied` |
| `Vehicle.ADAS.BlindSpotDetection.RightOccupied` | sensor | boolean | - | Object detected in right blind spot | `BSD_RightOccupied` |
| `Vehicle.ADAS.BlindSpotDetection.Warning` | sensor | boolean | - | Blind spot warning active | `BSD_Warning` |
| `Vehicle.ADAS.TrafficSignRecognition.Enabled` | actuator | boolean | - | Enable or disable TSR | `TSR_Enabled` |
| `Vehicle.ADAS.TrafficSignRecognition.DetectedSpeedLimit` | sensor | float | hm/h | Detected speed limit from traffic sign | `TSR_DetectedSpeedLimit` |
| `Vehicle.ADAS.TrafficSignRecognition.DetectedSignType` | sensor | enum | enum | Detected traffic sign type | `TSR_DetectedSignType` |
| `Vehicle.ADAS.ParkingAssist.Enabled` | actuator | boolean | - | Enable or disable parking assist | `PA_Enabled` |
| `Vehicle.ADAS.ParkingAssist.Active` | sensor | boolean | - | Parking assist active | `PA_Active` |
| `Vehicle.ADAS.ParkingAssist.ObstacleDistanceFront` | sensor | float | m | Distance to front obstacle | `PA_ObstacleDistanceFront` |
| `Vehicle.ADAS.ParkingAssist.ObstacleDistanceRear` | sensor | float | m | Distance to rear obstacle | `PA_ObstacleDistanceRear` |

---

## Traffic Sign Recognition enum

The `TSR_DetectedSignType` signal is encoded as an enum in the DBC and should match the VSS `allowed` values.

| Value | Meaning |
|---:|---|
| 0 | UNKNOWN |
| 1 | STOP |
| 2 | YIELD |
| 3 | NO_ENTRY |
| 4 | TURN_LEFT |
| 5 | TURN_RIGHT |
| 6 | PEDESTRIAN |
| 7 | TRAFFIC_LIGHT |
| 8 | ONE_WAY |
| 9 | NO_PARKING |
| 10 | NO_OVERTAKING |

---

## Common issues and fixes

### 1. Signal exists in DBC but not in Kuksa
Cause:
- the path is missing in `/etc/kuksa/vss.json`

Fix:
- add the corresponding VSS node
- restart the Databroker
- restart the CAN Provider

### 2. Signal exists in VSS but never updates
Cause:
- wrong `dbc.signal` name
- wrong CAN ID in the DBC
- no message being published on `can0`

Fix:
- verify the signal name in the DBC
- verify the message ID and sender
- inspect live CAN traffic with:

```bash
candump can0
```

### 3. Enum values do not match
Cause:
- VSS `allowed` values and DBC `VAL_` definitions are out of sync

Fix:
- update both files together whenever adding enum states

### 4. Bidirectional signals do not go back to CAN
Cause:
- `--val2dbc` missing
- signal does not have a reverse mapping
- the actuator path is not being updated in the Databroker

Fix:
- enable `--val2dbc`
- confirm the signal exists in the VSS mapping
- test by writing values from a client application

---

## Recommended validation workflow

1. Start `kuksa-databroker.service`
2. Start `kuksa-can-provider.service`
3. Verify service status with `systemctl status`
4. Inspect CAN traffic with `candump can0`
5. Open `kuksa-client`
6. Read known signals such as:
   - `Vehicle.Speed`
   - `Vehicle.Control.Throttle.Value`
   - `Vehicle.ADAS.CruiseControl.TargetSpeed`
   - `Vehicle.ADAS.TrafficSignRecognition.DetectedSignType`
7. If actuator writeback is enabled, update a writable datapoint and verify the corresponding CAN frame is emitted

---

## References

- [Kuksa Databroker](https://github.com/eclipse-kuksa/kuksa-databroker)
- [Kuksa CAN Provider](https://github.com/eclipse-kuksa/kuksa-can-provider)
- [Kuksa Databroker CLI](https://github.com/eclipse-kuksa/kuksa-databroker-cli)
- [Vehicle Signal Specification (VSS)](https://github.com/COVESA/vehicle_signal_specification)
