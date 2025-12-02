cmake_minimum_required(VERSION 3.10)

project(CAN)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "-Wall -Wextra")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SOURCES main.c)

add_executable(${CMAKE_PROJECT_NAME} ${SOURCES})
