cmake_minimum_required(VERSION 3.20)
include_guard(GLOBAL)

# Set the system name and processor for cross-compilation
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# Set the target sysroot and architecture
set(TARGET_SYSROOT /build/agl-sysroot-sdk/sysroots/aarch64-agl-linux) ## update this path accordingly
set(TARGET_ARCHITECTURE aarch64-agl-linux)
set(CMAKE_SYSROOT ${TARGET_SYSROOT})

# Set the C and C++ compilers
set(CMAKE_C_COMPILER /build/agl-sysroot-sdk/sysroots/x86_64-aglsdk-linux/usr/bin/${TARGET_ARCHITECTURE}/${TARGET_ARCHITECTURE}-gcc)
set(CMAKE_CXX_COMPILER /build/agl-sysroot-sdk/sysroots/x86_64-aglsdk-linux/usr/bin/${TARGET_ARCHITECTURE}/${TARGET_ARCHITECTURE}-g++)

set(QT_HOST_PATH /build/qt-host-build)
set(Qt6_DIR /build/qt-cross-build/lib/cmake/Qt6)

set(CMAKE_FIND_ROOT_PATH 
    /build/qt-cross-build
    /build/agl-sysroot-sdk/sysroots/aarch64-agl-linux
)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)