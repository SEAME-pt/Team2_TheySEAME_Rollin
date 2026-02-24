# Kuksa Integration (Raspberyr pi 5 + AGL)

This document describes the step-by-step procedure to set up and run the **Kuksa Databroker** together with the **Kuksa CAN Provider**, using SocketCAN and DBC-to-VSS mapping.

## What is Kuksa and why use it?

**Eclipse Kuksa** is an open-source set of components for the Software-Defined Vehicle (SDV) ecosystem. In this setup, Kuksa provides a **standard way to access and distribute vehicle signals** (for example speed, battery state-of-charge, steering angle) through a **gRPC-based broker**.

Why use Kuksa in your project?

- **Standardized signals**: use VSS (Vehicle Signal Specification) paths like `Vehicle.Speed` instead of ad-hoc signal names.
- **Decoupled architecture**: producers (e.g., CAN provider) and consumers (e.g., Qt apps) don’t need to know each other—only the broker.
- **Single source of truth**: Databroker becomes a central place to publish and read signal values.
- **Easier integration**: multiple apps/services can subscribe to the same datapoints without duplicating CAN decoding logic.

---

## Architecture

- **Kuksa Databroker**
  - Central gRPC-based data broker
  - Stores and serves VSS datapoints
  - Kuksa Databroker runs in the background as a systemd service
- **Kuksa CAN Provider**
  - Reads CAN frames via SocketCAN
  - Decodes frames using a DBC file
  - Publishes values to the Databroker using VSS paths
  - Kuksa CAN Provider in the background as a systemd service
- **VSS Specification (VSS JSON)**
  - Defines the datapoint tree (paths, types, units) available in the Databroker
  - Must include the same VSS paths used by the CAN Provider mapping (otherwise you get `Signal <VSS path> is not registered`)
  - Located at : `/etc/kuksa/vss.json`
- **DBC File (CAN database)**
  - Defines CAN messages and signals (IDs, bit layout, scaling, units)
  - Used by the CAN Provider to decode raw CAN frames into signal values
  - Located at : `etc/kuksa/CAN.dbc`
- **Databroker Client**
  - Used to inspect and validate datapoints

### Configuration Files

| File                                   | Purpose                |
|----------------------------------------|------------------------|
| `/etc/kuksa-can-provider/config.ini`   | [Main  config](#main-config)   |
| `/etc/default/kuksa-can-provider`      | [Can Provider extra arguments](#our-can-provider-configuration)        |
| `/etc/default/kuksa-databroker`        | [Databroker extra arguments](#our-databroker-configuration)        |

---

## Prerequisites
- Raspberry Pi 5 with AGL
- CAN interface available as `can0`
- Kuksa layer in AGL

## 1  . Kuksa Databroker (System Service)

### Start the Databroker:

```bash
systemctl start kuksa-databroker.service
```
### Enable at boot

``` bash
systemctl enable kuksa-databroker.service
```

### Check if Databroker is running

``` bash
systemctl status kuksa-databroker.service
```

### Our Databroker configuration:

```bash
   EXTRA_ARGS="\
      --address 0.0.0.0 \
      --port 55555 \
      --vss /etc/kuksa/vss.json \
    "
```

---

## 2. Run Kuksa CAN Provider (System Service)

### Run the CAN provider:

```bash
systemctl start kuksa-can-provider.service
```
### Enable at boot

``` bash
systemctl enable kuksa-can-provider.service
```

### Check if CAN provider is running

``` bash
systemctl status kuksa-can-provider.service
```
### Our CAN Provider configuration:

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
### Main Config
 We change the next lines in the main configuration file:
  ``` bash
  # VSS mapping file
  mapping = /etc/kuksa/vss.json

  # DBC file used to parse CAN messages
  dbcfile = /etc/kuksa/CAN.dbc

  # CAN port, use elmcan to start the elmcan bridge
  port =can0

  # IP address for server (KUKSA.val Server or Databroker)
  ip = 0.0.0.0
  ```

## 3.(optional) Validate Databroker with kuksa Client

Open a second terminal and run:

```bash
kuksa-client
```

Inside the kuksa Client prompt, try(example):

```text
getValue Vehicle.Speed
```
For a list of available commands inside the kuksa Client prompt try :
```text
help
```
### Note about VSS paths

If you see errors like:

```
not found
not registered
```

it means the Databroker does not have those VSS nodes available.
You must either:

- Load a VSS specification/metadata that defines those paths, or
- Update your CAN mapping file (`vss.json`) to use existing VSS paths.

---
## 4) Applications Dependencies (gRPC + Protobuf)

If you build an application that communicates with Kuksa over gRPC, you must install the **development packages** for Protobuf and gRPC and ensure the generated `*.pb.cc/*.pb.h` and `*.grpc.pb.cc/*.grpc.pb.h` sources are compiled into your target.

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

Quick sanity checks:

```bash
protoc --version
which grpc_cpp_plugin
pkg-config --modversion protobuf grpc++ grpc
```

> Note: On Ubuntu 22.04, CMake typically **does not** provide `gRPCConfig.cmake`.
> Using `pkg-config` (as shown below) is the recommended approach.

### 4.2 Protobuf/gRPC sources in your Qt target

If you already generated the sources and keep them inside your repository (for example under `/apps/middleware/Kuksa/val/v2/`), add them to your library target:

```cmake
target_sources(qtAppLib PRIVATE
  ${CMAKE_SOURCE_DIR}/path/to/middleware/Kuksa/val/v2/types.pb.cc
  ${CMAKE_SOURCE_DIR}/path/to/middleware/Kuksa/val/v2/val.pb.cc
  ${CMAKE_SOURCE_DIR}/path/to/middleware/Kuksa/val/v2/val.grpc.pb.cc
)
```

Make sure the corresponding include directories are available:

```cmake
target_include_directories(qtAppLib PRIVATE
  ${CMAKE_SOURCE_DIR}/path/to/middleware/Kuksa
  ${CMAKE_SOURCE_DIR}/path/to/middleware/Kuksa/val/v2
)
```

### 4.3 Link against gRPC and Protobuf using pkg-config (recommended on Ubuntu)

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

| Message         | ID     | Signal           | Range         | Unit   | Description                                 |
|-----------------|--------|------------------|--------------|--------|---------------------------------------------|
| SpeedMsg        | 0x200   | speed           | 0–200         | hm/h   | Current vehicle speed                       |
| BatteryMsg      | 0x201   | StateOfCharge   | 0–100         | %      | Battery charge level                        |
| ThrottleMsg     | 0x100  | throttle         | 0–100         | %      | Throttle position (acceleration)            |
| Gear            | 0x101  | gear             | 0–3           | enum   | Current Gear(Parking(P),Neutral(N), Reverse(R), Drive(D) |
| AngleMsg        | 0x102  | Angle            | -30-30        | deg    | Steering wheel angle                        |
| Break           | 0x103  | break            | 0–1           | enum   | 0 = DRIVE, 1 = BREAK                        |
| DrivingModeMsg  | 0x104  | DrivingMode      | 0–1           | enum   | 0 = MANUAL, 1 = AI_ASSIST                   |

This architecture makes it simple to add new features or connect other systems, since all important vehicle data is sent in a clear, standard format.

---

# Vehicle Signal Specification (VSS) Mapping

| VSS Path                        | Type      | Data Type | Unit     | Description                        | CAN Signal      |
|----------------------------------|-----------|-----------|----------|------------------------------------|-----------------|
| Vehicle.Speed                    | sensor    | float     | hm/h     | Vehicle speed from STM32           | speed           |
| Vehicle.Powertrain.Battery.StateOfCharge | sensor    | uint8     | percent   | Battery state of charge            | StateOfCharge   |
| Vehicle.Control.Throttle.Value   | actuator  | float     | percent  | Throttle command                   | throttle        |
| Vehicle.Control.Gear.Value       | actuator  | uint8     | enum     | Current Gear(Parking(P),Neutral(N), Reverse(R), Drive(D) | gear    |
| Vehicle.Control.Steering.Angle   | actuator  | float     | deg      | Steering angle command             | Angle           |
| Vehicle.Control.Mode.Break       | actuator  | uint8     | enum     | Break: 0 = DRIVE, 1 = BREAK        | Break           |
| Vehicle.Control.Mode.DrivingMode | actuator  | uint8     | enum     | Driving mode: 0=MANUAL, 1=AI_ASSIST| DrivingMode     |

---

## References

- [Kuksa Databroker](https://github.com/eclipse-kuksa/kuksa-databroker)
- [Kuksa CAN Provider](https://github.com/eclipse-kuksa/kuksa-can-provider)
- [Kuksa Databroker Client](https://github.com/eclipse-kuksa/kuksa-databroker-cli)
- [Vehicle Signal Specification (VSS)](https://github.com/COVESA/vehicle_signal_specification)
