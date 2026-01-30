# Use Debian 12 (Bookworm) as the base image
FROM debian:bookworm

# Avoid interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Update and install necessary packages
RUN { \
    set -e && \
    apt-get update && apt-get install -y \
    wget \
    git \
    build-essential \
    make \
    rsync \
    sed \
    libclang-dev \
    ninja-build \
    qt6-base-dev qt6-tools-dev qt6-base-dev-tools qt6-serialbus-dev qt6-positioning-dev \
    gcc \
    bison \
    python3 \
    gperf \
    pkg-config \
    libfontconfig1-dev \
    libfreetype6-dev \
    libx11-6 \
    libxext6  libxrender1 libxfixes3 libxrandr2 \
    libx11-dev \
    libwayland-dev libwayland-client0 libwayland-egl1-mesa \
    libxkbcommon-dev \
    libx11-xcb-dev \
    libxext-dev \
    libxfixes-dev \
    libxi-dev \
    libxrender-dev \
    libxcb1-dev \
    libxcb-glx0-dev \
    libxcb-keysyms1-dev \
    libglvnd0 libgl1-mesa-glx libegl1-mesa \
    libxcb-image0-dev \
    libxcb-shm0-dev \
    libxcb-icccm4-dev \
    libxcb-sync-dev \
    libxcb-xfixes0-dev \
    libxcb-shape0-dev \
    libxcb-randr0-dev \
    libxcb-render-util0-dev \
    libxcb-util-dev \
    libxcb-xinerama0-dev \
    libxcb-xkb-dev \
    libxkbcommon-dev \
    libxkbcommon-x11-dev \
    libatspi2.0-dev \
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    freeglut3-dev \
    libssl-dev \
    libgmp-dev \
    libmpfr-dev \
    libmpc-dev \
    libpq-dev \
    flex \
    pkg-config \
    libgrpc++-dev \
    libgrpc-dev \
    libprotobuf-dev \
    protobuf-compiler \
    gawk \
    texinfo \
    libisl-dev \
    zlib1g-dev \
    libtool \
    autoconf \
    automake \
    libgdbm-dev \
    libdb-dev \
    libbz2-dev \
    libreadline-dev \
    libexpat1-dev \
    liblzma-dev \
    libffi-dev \
    libsqlite3-dev \
    libbsd-dev \
    perl \
    patch \
    m4 \
    libncurses5-dev \
    gettext  \
    gcc-12-aarch64-linux-gnu \
    g++-12-aarch64-linux-gnu \
    binutils-aarch64-linux-gnu \
    libc6-arm64-cross \
    libc6-dev-arm64-cross \
    glibc-source; \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*; \
} 2>&1 | tee -a /build.log

# Set the working directory to /build
WORKDIR /build

# Build and install CMake from source
RUN ( \
    echo "Building CMake from source" && \
    mkdir cmakeBuild && cd cmakeBuild && \
    git clone https://github.com/Kitware/CMake.git && \
    cd CMake && \
    ./bootstrap && make -j$(nproc) && make install && \
    echo "CMake build completed"; \
 ) 2>&1 | tee -a /build.log

# Create sysroot directory
RUN mkdir sysroot sysroot/usr sysroot/opt

#  Raspberry Pi sysroot tarball (if available)
COPY rasp.tar.gz /build/rasp.tar.gz
RUN tar xvfz /build/rasp.tar.gz -C /build/sysroot

# Copy the toolchain file
COPY apps/Cluster/Cross_Compile/opencvToolchain.cmake /build/

RUN set -e && \
    echo "Fix symbolic link" && \
    wget https://raw.githubusercontent.com/riscv/riscv-poky/master/scripts/sysroot-relativelinks.py && \
    chmod +x sysroot-relativelinks.py && \
    python3 sysroot-relativelinks.py /build/sysroot 2>&1 | tee -a /build.log

# Copy the toolchain file
COPY apps/Cluster/Cross_Compile/toolchain.cmake /build/

RUN { \
    set -e && \
    mkdir -p qt6 qt6/host qt6/pi qt6/host-build qt6/pi-build qt6/src && \
    cd qt6/src && \
    echo "=== Downloading Qt 6.7.3 submodules ===" && \
    wget https://download.qt.io/archive/qt/6.7/6.7.3/submodules/qtbase-everywhere-src-6.7.3.tar.xz && \
    wget https://download.qt.io/archive/qt/6.7/6.7.3/submodules/qtwayland-everywhere-src-6.7.3.tar.xz && \
    wget https://download.qt.io/archive/qt/6.7/6.7.3/submodules/qtserialbus-everywhere-src-6.7.3.tar.xz && \
    wget https://download.qt.io/archive/qt/6.7/6.7.3/submodules/qtshadertools-everywhere-src-6.7.3.tar.xz && \
    wget https://download.qt.io/archive/qt/6.7/6.7.3/submodules/qtdeclarative-everywhere-src-6.7.3.tar.xz && \
    wget https://download.qt.io/archive/qt/6.7/6.7.3/submodules/qtpositioning-everywhere-src-6.7.3.tar.xz && \
    \
    echo "=== Extracting Qt sources for host build ===" && \
    cd ../host-build && \
    tar xf ../src/qtbase-everywhere-src-6.7.3.tar.xz && \
    tar xf ../src/qtshadertools-everywhere-src-6.7.3.tar.xz && \
    tar xf ../src/qtserialbus-everywhere-src-6.7.3.tar.xz && \
    tar xf ../src/qtwayland-everywhere-src-6.7.3.tar.xz && \
    tar xf ../src/qtpositioning-everywhere-src-6.7.3.tar.xz && \
    tar xf ../src/qtdeclarative-everywhere-src-6.7.3.tar.xz && \
    \
    echo "=== Building qtbase for host ===" && \
    cd qtbase-everywhere-src-6.7.3 && \
    cmake -GNinja -DCMAKE_BUILD_TYPE=Release \
        -DQT_BUILD_EXAMPLES=OFF \
        -DQT_BUILD_TESTS=OFF \
        -DCMAKE_INSTALL_PREFIX=/build/qt6/host && \
    cmake --build . --parallel $(nproc) && \
    cmake --install . && \
    \
    echo "=== Building qtshadertools for host ===" && \
    cd ../qtshadertools-everywhere-src-6.7.3 && \
    /build/qt6/host/bin/qt-configure-module . && \
    cmake --build . --parallel $(nproc) && \
    cmake --install . && \
    \
    echo "=== Building qtdeclarative for host ===" && \
    cd ../qtdeclarative-everywhere-src-6.7.3 && \
    /build/qt6/host/bin/qt-configure-module . && \
    cmake --build . --parallel $(nproc) && \
    cmake --install . && \
    \
    echo "=== Building qtserialbus for host ===" && \
    cd ../qtserialbus-everywhere-src-6.7.3 && \
    /build/qt6/host/bin/qt-configure-module . && \
    cmake --build . --parallel $(nproc) && \
    cmake --install . && \
    \
    echo "=== Building qtpositioning for host ===" && \
    cd ../qtpositioning-everywhere-src-6.7.3 && \
    /build/qt6/host/bin/qt-configure-module . && \
    cmake --build . --parallel $(nproc) && \
    cmake --install . && \
    \
    echo "=== Building qtwayland for host ===" && \
    cd ../qtwayland-everywhere-src-6.7.3 && \
    /build/qt6/host/bin/qt-configure-module . && \
    cmake --build . --parallel && \
    cmake --install . && \
    \
    echo "=== Extracting Qt sources for Raspberry Pi cross-build ===" && \
    cd ../../pi-build && \
    tar xf ../src/qtbase-everywhere-src-6.7.3.tar.xz && \
    tar xf ../src/qtshadertools-everywhere-src-6.7.3.tar.xz && \
    tar xf ../src/qtserialbus-everywhere-src-6.7.3.tar.xz && \
    tar xf ../src/qtwayland-everywhere-src-6.7.3.tar.xz && \
    tar xf ../src/qtdeclarative-everywhere-src-6.7.3.tar.xz && \
    tar xf ../src/qtpositioning-everywhere-src-6.7.3.tar.xz && \
    \
    echo "=== Building qtbase for Raspberry Pi ===" && \
    cd qtbase-everywhere-src-6.7.3 && \
    /build/qt6/host/bin/qt-cmake \
        -GNinja \
        -DDBUS_IS_SHARED=ON \
        -DCMAKE_BUILD_TYPE=Release \
        -DQT_BUILD_EXAMPLES=OFF \
        -DQT_BUILD_TESTS=OFF \
        -DQT_HOST_PATH=/build/qt6/host \
        -DCMAKE_STAGING_PREFIX=/build/qt6/pi \
        -DCMAKE_INSTALL_PREFIX=/usr/local/qt6 \
        -DCMAKE_TOOLCHAIN_FILE=/build/toolchain.cmake \
        -DQT_FEATURE_xcb=OFF \
        -DQT_FEATURE_xlib=OFF \
        -DQT_FEATURE_wayland=ON \
        -DQT_FEATURE_wayland_compositor=ON \
        -DQT_FEATURE_egl=ON \
        -DFEATURE_sql_psql=ON \
        -DFEATURE_shader_tools=OFF \ 
        . && \
    cmake --build . --parallel $(nproc) && \
    cmake --install . && \
    \
    echo "=== Building qtshadertools for Raspberry Pi ===" && \
    cd ../qtshadertools-everywhere-src-6.7.3 && \
    /build/qt6/host/bin/qt-configure-module . -- \
        -DQT_HOST_PATH=/build/qt6/host \
        -DCMAKE_STAGING_PREFIX=/build/qt6/pi \
        -DCMAKE_TOOLCHAIN_FILE=/build/toolchain.cmake \
        -DCMAKE_INSTALL_PREFIX=/usr/local/qt6 && \
    cmake --build . --parallel $(nproc) && \
    cmake --install . && \
    \
    echo "=== Building qtdeclarative for Raspberry Pi (QML + Quick + Controls2) ===" && \
    cd ../qtdeclarative-everywhere-src-6.7.3 && \
    /build/qt6/host/bin/qt-configure-module . -- \
        -DQT_HOST_PATH=/build/qt6/host \
        -DCMAKE_STAGING_PREFIX=/build/qt6/pi \
        -DCMAKE_TOOLCHAIN_FILE=/build/toolchain.cmake \
        -DCMAKE_INSTALL_PREFIX=/usr/local/qt6 && \
    cmake --build . --parallel $(nproc) && \
    cmake --install . && \
    \
    echo "=== Building qtserialbus for Raspberry Pi ===" && \
    cd ../qtserialbus-everywhere-src-6.7.3 && \
    /build/qt6/host/bin/qt-configure-module . -- \
        -DQT_HOST_PATH=/build/qt6/host \
        -DCMAKE_STAGING_PREFIX=/build/qt6/pi \
        -DCMAKE_TOOLCHAIN_FILE=/build/toolchain.cmake \
        -DCMAKE_INSTALL_PREFIX=/usr/local/qt6 && \
    cmake --build . --parallel $(nproc) && \
    cmake --install . && \
    \
    echo "=== Building qtpositioning for Raspberry Pi ===" && \
    cd ../qtpositioning-everywhere-src-6.7.3 && \
    /build/qt6/host/bin/qt-configure-module . -- \
        -DQT_HOST_PATH=/build/qt6/host \
        -DCMAKE_STAGING_PREFIX=/build/qt6/pi \
        -DCMAKE_TOOLCHAIN_FILE=/build/toolchain.cmake \
        -DCMAKE_INSTALL_PREFIX=/usr/local/qt6 && \
    cmake --build . --parallel $(nproc) && \
    cmake --install . && \
    \
    echo "=== Building qtwayland for Raspberry Pi ===" && \
    cd ../qtwayland-everywhere-src-6.7.3 && \
    /build/qt6/host/bin/qt-configure-module . -- \
        -DQT_HOST_PATH=/build/qt6/host \
        -DCMAKE_STAGING_PREFIX=/build/qt6/pi \
        -DCMAKE_TOOLCHAIN_FILE=/build/toolchain.cmake \
        -DCMAKE_INSTALL_PREFIX=/usr/local/qt6 && \
    cmake --build . --parallel $(nproc) && \
    cmake --install . && \
    echo "=== Qt 6.7.3 cross-compilation completed successfully ==="; \
} 2>&1 | tee -a /build.log

RUN tar -czvf qt-host-binaries.tar.gz -C /build/qt6/host .
RUN tar -czvf qt-pi-binaries.tar.gz -C /build/qt6/pi .

# Kuksa protos (font)
RUN mkdir -p /middleware/kuksa/proto
COPY apps/middleware/kuksa/proto/ /middleware/kuksa/proto/

# ---- Install gRPC/Protobuf for target (arm64) and copy into sysroot + generate Kuksa pb/grpc ----
RUN set -e && \
    dpkg --add-architecture arm64 && \
    apt-get update && \
    apt-get install -y --no-install-recommends \
        rsync \
        pkgconf \
        protobuf-compiler \
        protobuf-compiler-grpc \
        libgrpc-dev:arm64 libgrpc++-dev:arm64 \
        libprotobuf-dev:arm64 \
        zlib1g-dev:arm64 \
        libc-ares-dev:arm64 \
        libssl-dev:arm64 \
        libre2-dev:arm64 && \
    rm -rf /var/lib/apt/lists/* && \
    \
    mkdir -p /build/sysroot/usr/lib/aarch64-linux-gnu && \
    mkdir -p /build/sysroot/usr/include && \
    mkdir -p /build/sysroot/usr/share/pkgconfig && \
    \
    rsync -a /usr/lib/aarch64-linux-gnu/ /build/sysroot/usr/lib/aarch64-linux-gnu/ && \
    rsync -a /usr/share/pkgconfig/ /build/sysroot/usr/share/pkgconfig/ 2>/dev/null || true && \
    rsync -a /usr/include/ /build/sysroot/usr/include/ && \
    \
    # -------- generate kuksa VAL v2 C++ sources (into fixed path /middleware/kuksa/val/v2) --------
    mkdir -p /tmp/kuksa_gen && \
    protoc -I /middleware/kuksa/proto --cpp_out=/tmp/kuksa_gen \
      /middleware/kuksa/proto/kuksa/val/v2/types.proto \
      /middleware/kuksa/proto/kuksa/val/v2/val.proto && \
    protoc -I /middleware/kuksa/proto --grpc_out=/tmp/kuksa_gen \
      --plugin=protoc-gen-grpc=/usr/bin/grpc_cpp_plugin \
      /middleware/kuksa/proto/kuksa/val/v2/val.proto && \
    mkdir -p /middleware/kuksa/val/v2 && \
    cp -f /tmp/kuksa_gen/kuksa/val/v2/*.pb.* /middleware/kuksa/val/v2/ && \
    rm -rf /tmp/kuksa_gen

# Set up project directory
RUN mkdir /build/project

COPY     apps/Cluster/qtApp /build/project


# Build the project using Qt for Raspberry Pi
RUN { \
    set -e && \
    cd /build/project && \
    export PKG_CONFIG_SYSROOT_DIR=/build/sysroot && \
    export PKG_CONFIG_LIBDIR=/build/sysroot/usr/lib/aarch64-linux-gnu/pkgconfig:/build/sysroot/usr/share/pkgconfig && \
    export PKG_CONFIG_PATH= && \
    /build/qt6/pi/bin/qt-cmake . && \
    cmake --build .; \
} 2>&1 | tee -a /build.log