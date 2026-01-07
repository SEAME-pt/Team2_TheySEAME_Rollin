cmake_minimum_required(VERSION 3.10)

project(CarControl)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(can STATIC CAN/CAN.cpp)
target_include_directories(can PRIVATE CAN)

add_library(remote STATIC Evdev/Evdev.cpp Evdev/RemoteControl.cpp)
target_include_directories(remote PRIVATE Evdev)

add_executable(${CMAKE_PROJECT_NAME} main.cpp)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE CAN Evdev)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE can remote)

add_subdirectory(tests)
