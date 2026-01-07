cmake_minimum_required(VERSION 3.10)

project(CarControl)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(comms)
add_subdirectory(mode_manager)

add_executable(${CMAKE_PROJECT_NAME} main.cpp)
target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC comms mode_manager/remote)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE comms mode)

add_subdirectory(tests)
