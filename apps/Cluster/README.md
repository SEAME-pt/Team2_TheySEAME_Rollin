
# Docker-Based Qt Cross-Compilation for AGL Linux

During development, we faced repeated failures when attempting a standard cross-compilation of Qt for ARM64. To overcome these issues, we transitioned to a Docker-based workflow, inspired by PhysicsX’s methodology, which allows for a fully controlled and reproducible build environment.

This approach has been customized for our instrument cluster project on the Raspberry Pi, providing a clean separation between host and target environments and ensuring future scalability. Key references include:
By using Docker, we avoid dependency conflicts and drastically reduce build time compared to compiling directly on the device.

## Overview of the Workflow

The process uses **two dedicated Docker containers**:
1. **Target Environment Container:** Simulates the Raspberry Pi OS and generates the sysroot.
2. **Cross-Compilation Container:** Uses the sysroot to build Qt and the project on the host.

### Step 1 – Target Environment Container

This container replicates the Raspberry Pi operating system and collects all required headers and libraries.

**Tasks performed:**
- Use `arm64v8/debian:bookworm` as the base image.
- Install essential development packages (graphics, input, SSL, multimedia).
- Archive system directories into a compressed sysroot (`rasp.tar.gz`).

```dockerfile
FROM arm64v8/debian:bookworm
RUN apt-get update && apt-get install -y \
    libboost-all-dev libinput-dev libxkbcommon-dev libegl1-mesa-dev \
    libgles2-mesa-dev libglib2.0-dev libssl-dev libdbus-1-dev ...
WORKDIR /build
RUN tar czf rasp.tar.gz -C / lib usr/include usr/lib etc/alternatives
```

This step produces a snapshot of the target filesystem to be used in cross-compilation.

### Step 2 – Cross-Compilation Container

The second container sets up the host environment to compile Qt and the application using the ARM64 sysroot.

**Key actions:**
- Install compilers and build tools (GCC, Ninja, Python, Git, etc.).
- Build a compatible version of CMake from source.
- Extract the sysroot from `rasp.tar.gz`.
- Correct symlinks inside the sysroot to ensure portability.

```dockerfile
FROM debian:bookworm
RUN apt-get update && apt-get install -y git build-essential ninja-build python3 ...
RUN git clone https://github.com/Kitware/CMake.git && ./bootstrap && make -j$(nproc) && make install
COPY rasp.tar.gz /build/rasp.tar.gz
RUN tar xvfz /build/rasp.tar.gz -C /build/sysroot
RUN wget https://raw.githubusercontent.com/riscv/riscv-poky/master/scripts/sysroot-relativelinks.py \
 && python3 sysroot-relativelinks.py /build/sysroot
```

After this step, we have a fully prepared cross-compilation environment with all necessary tools and a clean sysroot.

### Step 3 – Toolchain Configuration

The toolchain file (`toolchain.cmake`) informs CMake about:
- The ARM64 compiler binaries.
- Target architecture and system name.
- Sysroot location.
- Paths to Qt modules and system libraries.

### Step 4 – Compiling Qt for ARM64

Before building our application, Qt itself must be cross-compiled for the target architecture.

**Workflow:**
- Fetch Qt modules (qtbase, qtdeclarative, qtshaderstools).
- Configure Qt build using the toolchain.
- Compile and install Qt into a dedicated directory (`/build/qt6/pi`).

```dockerfile
RUN { \
    mkdir -p qt6 && \
    wget ...qtbase...qtdeclarative...qtshadertools... && \
    cmake -DCMAKE_TOOLCHAIN_FILE=/build/toolchain.cmake ... && \
    cmake --build . && cmake --install .; \
}
```

### Step 5 – Building the Application

Finally, the project is compiled against the cross-compiled Qt libraries.

**Steps:**
- Copy source code to `/build/project`.
- Configure with the Qt toolchain.
- Build the final ARM64 executable.

```dockerfile
COPY project /build/project
RUN { \
    cd /build/project && \
    /build/qt6/pi/bin/qt-cmake . && \
    cmake --build .; \
}
```

The resulting executable is fully compatible with the Raspberry Pi target.


# Steps

##### **1 - Creates the Target Image (DockerfileRasp)**

```bash
$ docker buildx build --platform linux/arm64 --load -f Dockerfile-sysroot -t raspimage .
```


##### **2 - Extract the Sysroot Archive**

```bash
$ docker create --name temp-arm raspimage
$ docker cp temp-arm:/build/rasp.tar.gz ./rasp.tar.gz
```
##### **3 - Build the Cross-Compiler and Qt (Dockerfile)**

```bash
$ docker build -t qtcrossbuild -f Dockerfile .
```

##### **4 - Extract the project binaries**

```bash
$ docker create --name tmpbuild qtcrossbuild
$ docker cp tmpbuild:/build/project/appDashboardproject ./qtApp
$ docker cp tmpbuild:/build/qt-pi-binaries.tar.gz ./qt-pi-binaries.tar.gz
```

### Links
- 🐐 https://github.com/PhysicsX/QTonRaspberryPi
- https://www.docker.com/blog/compiling-qt-with-docker-multi-stage-and-multi-platform/
- https://ruvi-d.medium.com/a-master-guide-to-linux-cross-compiling-b894bf909386
- https://doc.qt.io/qt-6.9/cross-compiling-qt.html
- https://wiki.qt.io/Cross-Compile_Qt_6_for_Raspberry_Pi

### Conclusion

Using a Dockerized workflow allows us to bypass the errors encountered with traditional cross-compilation. It provides a consistent, isolated environment, reduces build time, and creates a reusable framework for future ARM64 Qt projects.
