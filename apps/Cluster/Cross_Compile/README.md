# Docker-Based Qt Cross-Compilation for AGL Linux (ARM64)

During development, we evolved from a Raspberry Pi-focused cross-compilation setup to a more robust solution using the **Automotive Grade Linux (AGL) SDK**. This approach provides a professional-grade build environment specifically designed for embedded automotive applications, with full support for modern toolchains and middleware integration.

This workflow has been customized for our instrument cluster project, providing complete integration with Kuksa vehicle data services, gRPC communication, and protocol buffers. The Docker-based approach ensures reproducibility, eliminates dependency conflicts, and significantly reduces build complexity.

## Overview of the Workflow

The process uses a **single, comprehensive Docker container** that:
1. Sets up the complete AGL cross-compilation toolchain
2. Builds host and target versions of Qt 6.7.3
3. Compiles protobuf and gRPC for both architectures
4. Generates Kuksa protobuf definitions
5. Cross-compiles the final application

This consolidated approach simplifies the build process while providing enterprise-grade tooling.

---

## Build Process

### Step 1 – Base Environment Setup

The container starts by installing all necessary build tools and development libraries:

```dockerfile
FROM debian:bookworm
ENV DEBIAN_FRONTEND=noninteractive

RUN apt update && apt install -y \
    bash sudo xz-utils cpio file git wget tar python3 \
    qemu-user-static locales build-essential cmake ninja-build \
    libgl1-mesa-dev libglu1-mesa-dev libxkbcommon-dev \
    libwayland-dev libfontconfig1-dev libfreetype6-dev \
    libdbus-1-dev libegl1-mesa-dev libinput-dev libssl-dev \
    libgbm-dev unzip
```

### Step 2 – Qt Source Download

Qt 6.7.3 source code is downloaded and extracted:

```dockerfile
WORKDIR /build
RUN wget https://download.qt.io/archive/qt/6.7/6.7.3/single/qt-everywhere-src-6.7.3.tar.xz && \
    tar -xf qt-everywhere-src-6.7.3.tar.xz && rm qt-everywhere-src-6.7.3.tar.xz
```

### Step 3 – AGL Sysroot SDK Installation

The AGL SDK provides a complete cross-compilation environment with all necessary libraries and toolchain:

```dockerfile
WORKDIR /build/agl-sysroot-sdk
COPY scripts/AGL/agl-sysroot-sdk.sh /build/agl-sysroot-sdk/agl-sysroot-sdk.sh

RUN chmod +x agl-sysroot-sdk.sh && ./agl-sysroot-sdk.sh -y -d /build/agl-sysroot-sdk
```

An environment wrapper script is created to simplify cross-compilation commands:

```dockerfile
RUN printf '%s\n' \
    '#!/bin/bash' \
    '. /build/agl-sysroot-sdk/environment-setup-aarch64-agl-linux' \
    'exec "$@"' \
    > /build/agl-sysroot-sdk/env.sh && chmod +x /build/agl-sysroot-sdk/env.sh
```

### Step 4 – Protobuf and gRPC (Host Build)

First, we build native versions for the build machine to generate code:

```dockerfile
# Protobuf for host
WORKDIR /tmp
RUN git clone -b v25.1 --recurse-submodules --depth 1 https://github.com/protocolbuffers/protobuf.git \
    && cd protobuf && mkdir build && cd build \
    && cmake .. -DCMAKE_BUILD_TYPE=Release -Dprotobuf_BUILD_TESTS=OFF \
    && make -j 16 && make install && ldconfig

# gRPC for host
WORKDIR /tmp
RUN git clone -b v1.62.0 --recurse-submodules --depth 1 https://github.com/grpc/grpc \
    && cd grpc && mkdir build && cd build \
    && cmake .. -DgRPC_INSTALL=ON -DgRPC_BUILD_TESTS=OFF \
       -DCMAKE_BUILD_TYPE=Release \
       -DgRPC_ABSL_PROVIDER=package \
       -DgRPC_PROTOBUF_PROVIDER=package \
    && make -j 16 && make install && ldconfig
```

### Step 5 – Protobuf and gRPC (ARM64 Cross-Build)

Next, we cross-compile for the target ARM64 architecture:

```dockerfile
# Protobuf for ARM64
WORKDIR /tmp/protobuf
RUN mkdir build_arm && cd build_arm \
    && /build/agl-sysroot-sdk/env.sh cmake .. \
       -DCMAKE_INSTALL_PREFIX=/build/qt-cross-build \
       -DCMAKE_BUILD_TYPE=Release \
       -Dprotobuf_BUILD_TESTS=OFF \
       -Dprotobuf_BUILD_PROTOC_BINARIES=OFF \
    && make -j 16 && make install

# gRPC for ARM64 (static libraries)
WORKDIR /tmp/grpc
RUN mkdir build_arm && cd build_arm \
    && /build/agl-sysroot-sdk/env.sh cmake .. \
       -DCMAKE_INSTALL_PREFIX=/build/qt-cross-build \
       -DProtobuf_DIR=/build/qt-cross-build/lib/cmake/protobuf \
       -DgRPC_PROTOBUF_PROVIDER=package \
       -DgRPC_CPP_PLUGIN_EXECUTABLE=/usr/local/bin/grpc_cpp_plugin \
    && make -j 16 grpc++ grpc gpr address_sorting re2 upb_* \
    && find . -name "*.a" -exec cp {} /build/qt-cross-build/lib/ \;
```

### Step 6 – Kuksa Protocol Buffer Generation

Generate C++ code from Kuksa vehicle data protocol definitions:

```dockerfile
COPY apps/middleware/kuksa/proto/ /middleware/kuksa/proto/

RUN protoc -I /middleware/kuksa/proto --cpp_out=/tmp/kuksa_gen \
      /middleware/kuksa/proto/kuksa/val/v2/types.proto \
      /middleware/kuksa/proto/kuksa/val/v2/val.proto && \
    protoc -I /middleware/kuksa/proto --grpc_out=/tmp/kuksa_gen \
      --plugin=protoc-gen-grpc=/usr/local/bin/grpc_cpp_plugin \
      /middleware/kuksa/proto/kuksa/val/v2/val.proto
```

### Step 7 – Qt Host Build

Build Qt natively for the host machine (required for cross-compilation tools):

```dockerfile
WORKDIR /build/qt-host-build
RUN cmake /build/qt-everywhere-src-6.7.3 -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/build/qt-host-build \
    -DQT_BUILD_EXAMPLES=OFF \
    -DQT_BUILD_TESTS=OFF \
    -DFEATURE_system_protobuf=ON \
    -DFEATURE_system_grpc=ON
    
RUN ninja -j 16 && ninja install
```

### Step 8 – Toolchain Configuration

The toolchain file informs CMake about cross-compilation settings:

```cmake
# Key toolchain settings
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_SYSROOT /build/agl-sysroot-sdk/sysroots/aarch64-agl-linux)
set(CMAKE_C_COMPILER aarch64-agl-linux-gcc)
set(CMAKE_CXX_COMPILER aarch64-agl-linux-g++)
```

### Step 9 – Qt Cross-Build for ARM64

Cross-compile Qt for the target using the AGL toolchain:

```dockerfile
WORKDIR /build/qt-cross-build
RUN /build/agl-sysroot-sdk/env.sh cmake /build/qt-everywhere-src-6.7.3 -GNinja \
    -DCMAKE_TOOLCHAIN_FILE=/toolchain/toolchain.cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/build/qt-cross-build \
    -DQT_HOST_PATH=/build/qt-host-build \
    -DQT_BUILD_EXAMPLES=OFF \
    -DQT_BUILD_TESTS=OFF \
    -DFEATURE_wayland=ON \
    -DFEATURE_eglfs=ON \
    -DSKIP_qtwebengine=ON \
    -DBUILD_qtnetwork=ON \
    -DBUILD_qtnetworkauth=ON \
    -DBUILD_qtpositioning=ON \
    -DFEATURE_system_protobuf=ON \
    -DFEATURE_system_grpc=ON

RUN /build/agl-sysroot-sdk/env.sh ninja -j 16 && ninja install
```

### Step 10 – Application Build

Finally, cross-compile the instrument cluster application:

```dockerfile
COPY apps/Cluster/qtApp /build/project/
COPY apps/Cluster/qtApp/cross-CMakeLists.txt /build/project/CMakeLists.txt

RUN cd /build/project && \
    /build/agl-sysroot-sdk/env.sh cmake -S /build/project -GNinja \
    -DCMAKE_TOOLCHAIN_FILE=/toolchain/toolchain.cmake \
    -DQT_HOST_PATH=/build/qt-host-build \
    -DQt6_DIR=/build/qt-cross-build/lib/cmake/Qt6 \
    -DCMAKE_PREFIX_PATH=/build/qt-cross-build \
    -DProtobuf_DIR=/build/qt-cross-build/lib/cmake/protobuf \
    -DLIB_GRPC_CPP=/build/qt-cross-build/lib/libgrpc++.a \
    -DLIB_GRPC=/build/qt-cross-build/lib/libgrpc.a \
    && ninja -j $(nproc)
```

---

## Build and Deployment Steps

### Build the Docker Image

```bash
docker build -f apps/Cluster/Cross_Compile/Dockerfile -t cross_compile .
```

This single command builds everything: Qt, protobuf, gRPC, and your application.

### Extract the Compiled Application

```bash
# Create temporary container
docker create --name tmpbuild cross_compile

# Extract the ARM64 binary
docker cp tmpbuild:/build/project/qtAppExec ./qtAppExec
```

### Deploy to Target Device

```bash
# Copy to target device
scp qtAppExec user@<TARGET_IP>:/path/

# On target device
ssh user@<TARGET_IP>
./qtAppExec
```
---

## Links
- https://docs.automotivelinux.org/
- https://github.com/PhysicsX/QTonRaspberryPi
- https://www.docker.com/blog/compiling-qt-with-docker-multi-stage-and-multi-platform/
- https://ruvi-d.medium.com/a-master-guide-to-linux-cross-compiling-b894bf909386
- https://doc.qt.io/qt-6.9/cross-compiling-qt.html
- https://wiki.qt.io/Cross-Compile_Qt_6_for_Raspberry_Pi
---

## Conclusion

This Docker-based workflow provides a professional-grade cross-compilation environment for Qt automotive applications. By leveraging the AGL SDK and integrating modern middleware (gRPC, protobuf, Kuksa), we ensure:

- **Reliability**: Consistent builds across development teams
- **Maintainability**: Clear separation of build stages
- **Scalability**: Easy to extend for new features or targets
- **Professionalism**: Industry-standard tools and practices

The single-container approach simplifies the previous multi-stage process while providing more functionality, making it ideal for automotive embedded development.
