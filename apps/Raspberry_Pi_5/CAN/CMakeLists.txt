cmake_minimum_required(VERSION 3.10)

project(CAN)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SOURCES main.cpp CAN.cpp)

add_executable(${CMAKE_PROJECT_NAME} ${SOURCES})
target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/include)
