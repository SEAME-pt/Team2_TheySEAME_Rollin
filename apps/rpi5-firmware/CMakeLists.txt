cmake_minimum_required(VERSION 3.10)

project(CarControl)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(PUBLIC comms mode_manager/remote)

add_subdirectory(comms)
add_subdirectory(mode_manager)

add_executable(${CMAKE_PROJECT_NAME} main.cpp)
target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC comms mode)

add_subdirectory(tests)
