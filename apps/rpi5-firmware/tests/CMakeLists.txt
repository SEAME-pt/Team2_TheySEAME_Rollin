cmake_minimum_required(VERSION 3.10)

project(RaspTests)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(SOURCES
	RemoteControlTest.cpp
	CANTest.cpp
)

add_subdirectory(${CMAKE_SOURCE_DIR}/../../libs/googletest ${CMAKE_BINARY_DIR}/googletest)

add_executable(RaspTests ${SOURCES})
target_include_directories(RaspTests PUBLIC
	${CMAKE_SOURCE_DIR}/../../libs/googletest/include
)
target_link_libraries(RaspTests
    PRIVATE
	gtest
	gtest_main
	gmock
	comms
	mode
	socketcan
)
