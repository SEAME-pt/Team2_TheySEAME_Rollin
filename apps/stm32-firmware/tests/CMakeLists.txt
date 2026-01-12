cmake_minimum_required(VERSION 3.16)

project(STMTests LANGUAGES C CXX)

find_package(GTest REQUIRED)

add_executable(STMTests
    speed_tests.cpp
    ../Core/Src/Sensors/speed.c
)

target_link_libraries(STMTests
    PRIVATE
    GTest::gtest_main
)

target_include_directories(STMTests
    PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../Core/Inc>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../Core/Src>  # Recursive for .c siblings
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../Core/Src/Sensors>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../Drivers/STM32U5xx_HAL_Driver/Inc>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../Drivers/CMSIS/Device/ST/STM32U5xx/Include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../Drivers/CMSIS/Include>
    # Add these common ones:
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../Core/Inc>  # Duplicate ok, for config
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../Drivers/STM32U5xx_HAL_Driver/Inc/Legacy>  # If exists
)
include(GoogleTest)
gtest_discover_tests(STMTests)