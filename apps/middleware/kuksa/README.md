# Kuksa Integration Guide (Databroker + CAN Provider)

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
- **Kuksa CAN Provider**
  - Reads CAN frames via SocketCAN
  - Decodes frames using a DBC file
  - Publishes values to the Databroker using VSS paths
- **VSS Specification (VSS JSON)**
  - Defines the datapoint tree (paths, types, units) available in the Databroker
  - Must include the same VSS paths used by the CAN Provider mapping (otherwise you get `Signal <VSS path> is not registered`)
- **DBC File (CAN database)**
  - Defines CAN messages and signals (IDs, bit layout, scaling, units)
  - Used by the CAN Provider to decode raw CAN frames into signal values
- **Databroker CLI**
  - Used to inspect and validate datapoints

---

## Prerequisites (AGL/ Debian)
- Raspberry Pi 5 with CAN hardware (controller + transceiver)
- CAN interface available as `can0`
- Docker installed (this guide runs both services using Docker containers)
- `can-utils` installed
- 
---

## 1  . Run Kuksa Databroker

Start the Databroker:

```bash
docker run -it --rm ghcr.io/eclipse-kuksa/kuksa-databroker:latest
```

Our configuration:

```bash
  --insecure   --address 0.0.0.0   --port 55555   --vss /etc/kuksa/vss.json
```

Check if is listening:
```bash
ss -ltnp | grep 55555
```

---

## 2. Run Kuksa CAN Provider (DBC → VSS)

We have the VSS and DBC file at:

- `etc/kuksa//CAN.dbc`
- `etc/kuksa//vss.json`

Run the CAN provider:

```bash
docker run --rm -it --privileged   -v etc/kuksa/:/cfg   kuksa-can-provider:local
```

Our configuration:

```bash
  --server-type kuksa_databroker  --dbcfile /cfg/CAN.dbc  --mapping /cfg/vss.json  --use-socketcan  --dbc2val
```

### Common error

```
Signal <VSS path> is not registered
```

This means the VSS path defined in `vss.json` does not exist in the Databroker metadata.

Validate paths using the CLI before running the provider.

---

## 3.(optional) Validate Databroker with CLI

Open a second terminal and run:

```bash
docker run -it --rm ghcr.io/eclipse-kuksa/kuksa-databroker-cli:latest   --server http://0.0.0.0:55555
```

Inside the CLI prompt, try(example):

```text
get Vehicle.Speed
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

## Troubleshooting

### Databroker CLI cannot connect

```
Failed to connect to http://0.0.0.0:55555/
```

- Databroker is not running
- Wrong port
- Network issue

Check with:

```bash
ss -ltnp | grep 55555
```

---

### CAN Provider exits with "Signal not registered"

- The Databroker does not know the VSS path
- Check available paths using the CLI
- Load the correct VSS specification or update the mapping

---

## Notes
- For production or containerized deployments, consider using a dedicated Docker network.
- Kuksa VAL server (legacy) is **not required** when using the Databroker.

---

## References

- [Kuksa Databroker](https://github.com/eclipse-kuksa/kuksa-databroker)
- [Kuksa CAN Provider](https://github.com/eclipse-kuksa/kuksa-can-provider)
- [Kuksa Databroker CLI](https://github.com/eclipse-kuksa/kuksa-databroker-cli)
- [Vehicle Signal Specification (VSS)](https://github.com/COVESA/vehicle_signal_specification)
