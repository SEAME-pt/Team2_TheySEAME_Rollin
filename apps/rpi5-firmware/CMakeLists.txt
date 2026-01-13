cmake_minimum_required(VERSION 3.10)

project(CarControl)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(ExternalProject)

ExternalProject_Add(googletest_add
	PREFIX "${CMAKE_BINARY_DIR}/googletest"
	SOURCE_DIR "${CMAKE_SOURCE_DIR}/../../libs/googletest"
	INSTALL_COMMAND ""
)

include_directories(PUBLIC comms mode_manager/remote)

add_subdirectory(comms)
add_subdirectory(mode_manager)

add_executable(${CMAKE_PROJECT_NAME} main.cpp)
target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC comms mode socketcan)

if(BUILD_TESTS)
	add_subdirectory(tests)
endif()
