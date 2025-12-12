cmake_minimum_required(VERSION 3.10)

project(CAN)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(libCAN STATIC CAN/src/CAN.cpp)
target_include_directories(libCAN PRIVATE CAN/include)

add_library(libRemote STATIC RemoteControl/src/RemoteControl.cpp)
target_include_directories(libRemote PRIVATE RemoteControl/include)

add_executable(${CMAKE_PROJECT_NAME} main.cpp)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE CAN/include RemoteControl/include)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE libCAN libRemote)
