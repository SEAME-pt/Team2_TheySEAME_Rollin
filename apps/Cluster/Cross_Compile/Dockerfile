FROM debian:bookworm

ENV DEBIAN_FRONTEND=noninteractive

RUN apt update && apt install -y \
    bash sudo xz-utils cpio file git wget tar python3 \
    qemu-user-static locales build-essential cmake ninja-build \
    libgl1-mesa-dev libglu1-mesa-dev libxkbcommon-dev libxkbcommon-x11-dev \
    libwayland-dev libwayland-egl1-mesa wayland-protocols \
    libxcb1-dev libxcb-glx0-dev libxcb-icccm4-dev libxcb-image0-dev \
    libxcb-keysyms1-dev libxcb-randr0-dev libxcb-render-util0-dev \
    libxcb-shape0-dev libxcb-shm0-dev libxcb-sync-dev libxcb-util-dev \
    libxcb-xfixes0-dev libxcb-xinerama0-dev libxcb-xkb-dev libx11-xcb-dev \
    libfontconfig1-dev libfreetype6-dev libdbus-1-dev libegl1-mesa-dev \
    libinput-dev libssl-dev libgbm-dev unzip

WORKDIR /build
RUN wget https://download.qt.io/archive/qt/6.7/6.7.3/single/qt-everywhere-src-6.7.3.tar.xz && \
    tar -xf qt-everywhere-src-6.7.3.tar.xz && rm qt-everywhere-src-6.7.3.tar.xz

WORKDIR /build/agl-sysroot-sdk

COPY scripts/AGL/agl-sysroot-sdk.sh /build/agl-sysroot-sdk/agl-sysroot-sdk.sh

RUN chmod +x agl-sysroot-sdk.sh && ./agl-sysroot-sdk.sh -y -d /build/agl-sysroot-sdk

RUN printf '%s\n' \
    '#!/bin/bash' \
    '. /build/agl-sysroot-sdk/environment-setup-aarch64-agl-linux' \
    'exec "$@"' \
    > /build/agl-sysroot-sdk/env.sh && chmod +x /build/agl-sysroot-sdk/env.sh

RUN rm -rf /build/agl-sysroot-sdk/sysroots/aarch64-agl-linux/usr/lib/cmake/utf8_range && \
rm -rf /build/agl-sysroot-sdk/sysroots/aarch64-agl-linux/usr/lib/cmake/protobuf && \
rm -rf /build/agl-sysroot-sdk/sysroots/aarch64-agl-linux/usr/lib/cmake/grpc

WORKDIR /tmp
RUN git clone -b v25.1 --recurse-submodules --depth 1 https://github.com/protocolbuffers/protobuf.git \
    && cd protobuf \
    && mkdir build && cd build \
    && cmake .. \
        -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
        -DCMAKE_BUILD_TYPE=Release \
        -Dprotobuf_BUILD_TESTS=OFF \
    && make -j 16 \
    && make install \
    && ldconfig

WORKDIR /tmp
RUN git clone -b v1.62.0 --recurse-submodules --depth 1 https://github.com/grpc/grpc \
    && cd grpc \
    && mkdir build && cd build \
    && cmake .. \
       -DgRPC_INSTALL=ON \
       -DgRPC_BUILD_TESTS=OFF \
       -DCMAKE_BUILD_TYPE=Release \
       -DgRPC_ABSL_PROVIDER=package \
       -DgRPC_PROTOBUF_PROVIDER=package \
       -DgRPC_SSL_PROVIDER=package \
       -DgRPC_ZLIB_PROVIDER=package \
    && make -j 16 \
    && make install \
    && ldconfig

WORKDIR /tmp/protobuf
RUN rm -rf build_arm && mkdir build_arm && cd build_arm \
    && /build/agl-sysroot-sdk/env.sh cmake .. -DCMAKE_INSTALL_PREFIX=/build/qt-cross-build \
    -DCMAKE_BUILD_TYPE=Release \
    -Dprotobuf_BUILD_TESTS=OFF \
    -Dprotobuf_BUILD_PROTOC_BINARIES=OFF \
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
    && make -j 16 \
    && make install

WORKDIR /tmp/grpc
RUN mkdir -p build_arm && cd build_arm \
    && /build/agl-sysroot-sdk/env.sh cmake .. \
    -DCMAKE_INSTALL_PREFIX=/build/qt-cross-build \
    -DCMAKE_PREFIX_PATH="/build/qt-cross-build" \
    -DCMAKE_BUILD_TYPE=Release \
    -DgRPC_INSTALL=ON \
    -DgRPC_BUILD_TESTS=OFF \
    -DgRPC_BUILD_GRPC_CSHARP_PLUGIN=OFF \
    -DgRPC_BUILD_GRPC_NODE_PLUGIN=OFF \
    -DgRPC_BUILD_GRPC_OBJECTIVE_C_PLUGIN=OFF \
    -DgRPC_BUILD_GRPC_PHP_PLUGIN=OFF \
    -DgRPC_BUILD_GRPC_PYTHON_PLUGIN=OFF \
    -DgRPC_BUILD_GRPC_RUBY_PLUGIN=OFF \
    -DCMAKE_CXX_FLAGS="-Wno-error=deprecated-declarations -Wno-deprecated-declarations" \
    -DCMAKE_C_FLAGS="-Wno-error=deprecated-declarations -Wno-deprecated-declarations" \
    -DProtobuf_DIR=/build/qt-cross-build/lib/cmake/protobuf \
    -Dabsl_DIR=/build/qt-cross-build/lib/cmake/absl \
    -Dutf8_range_DIR=/build/qt-cross-build/lib/cmake/utf8_range \
    -DgRPC_ABSL_PROVIDER=package \
    -DgRPC_PROTOBUF_PROVIDER=package \
    -DgRPC_SSL_PROVIDER=package \
    -DgRPC_ZLIB_PROVIDER=package \
    -DgRPC_RE2_PROVIDER=module \
    -DgRPC_CPP_PLUGIN_EXECUTABLE=/usr/local/bin/grpc_cpp_plugin \
    && make -j 16 grpc++ grpc gpr address_sorting re2 upb_mem_lib upb_message_lib upb_textformat_lib upb_json_lib upb_base_lib utf8_range_lib || true \
    && mkdir -p /build/qt-cross-build/lib \
    && mkdir -p /build/qt-cross-build/include \
    && find . -name "*.a" -exec cp -v {} /build/qt-cross-build/lib/ \; \
    && cp -r ../include/* /build/qt-cross-build/include/

RUN mkdir -p /middleware/kuksa/proto
COPY apps/middleware/kuksa/proto/ /middleware/kuksa/proto/

RUN set -e && \
    mkdir -p /tmp/kuksa_gen && \
    protoc -I /middleware/kuksa/proto --cpp_out=/tmp/kuksa_gen \
      /middleware/kuksa/proto/kuksa/val/v2/types.proto \
      /middleware/kuksa/proto/kuksa/val/v2/val.proto && \
    protoc -I /middleware/kuksa/proto --grpc_out=/tmp/kuksa_gen \
      --plugin=protoc-gen-grpc=/usr/local/bin/grpc_cpp_plugin \
      /middleware/kuksa/proto/kuksa/val/v2/val.proto && \
    mkdir -p /middleware/kuksa/val/v2 && \
    cp -f /tmp/kuksa_gen/kuksa/val/v2/*.pb.* /middleware/kuksa/val/v2/ && \
    rm -rf /tmp/kuksa_gen
    
WORKDIR /build/qt-host-build
RUN cmake /build/qt-everywhere-src-6.7.3 -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/build/qt-host-build \
    -DQT_BUILD_EXAMPLES=OFF \
    -DQT_BUILD_TESTS=OFF \
    -DFEATURE_system_protobuf=ON \
    -DFEATURE_system_grpc=ON
    
RUN ninja -j 16 && ninja install

RUN mkdir -p /toolchain/
COPY apps/Cluster/Cross_Compile/toolchain.cmake /toolchain/

RUN set -e && \
    cd /build/qt-cross-build && \
    /build/agl-sysroot-sdk/env.sh cmake /build/qt-everywhere-src-6.7.3 -GNinja \
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
    -DFEATURE_system_grpc=ON \
 && /build/agl-sysroot-sdk/env.sh ninja -j 16 \
 && ninja install

RUN mkdir -p /build/project

COPY apps/Cluster/qtApp /build/project/

COPY apps/Cluster/qtApp/cross-CMakeLists.txt /build/project/CMakeLists.txt

RUN set -e && \
    cd /build/project && \
    /build/agl-sysroot-sdk/env.sh cmake -S /build/project -GNinja \
    -DCMAKE_TOOLCHAIN_FILE=/toolchain/toolchain.cmake \
    -DQT_HOST_PATH=/build/qt-host-build \
    -DQt6_DIR=/build/qt-cross-build/lib/cmake/Qt6 \
    -DCMAKE_PREFIX_PATH=/build/qt-cross-build \
    -DCMAKE_FIND_ROOT_PATH_MODE_PACKAGE=BOTH \
    -DProtobuf_DIR=/build/qt-cross-build/lib/cmake/protobuf \
    -Dabsl_DIR=/build/qt-cross-build/lib/cmake/absl \
    -Dutf8_range_DIR=/build/qt-cross-build/lib/cmake/utf8_range \
    -DLIB_GRPC_CPP=/build/qt-cross-build/lib/libgrpc++.a\
    -DLIB_GRPC=/build/qt-cross-build/lib/libgrpc.a \
    -DLIB_GPR=/build/qt-cross-build/lib/libgpr.a\
    -DLIB_ADDR_SORTING=/build/qt-cross-build/lib/libaddress_sorting.a \
    && ninja -j $(nproc)